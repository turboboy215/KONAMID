/*Konami (GB/GBC) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long offset;
long tablePtrLoc;
long tableOffset;
int i, j;
char outfile[1000000];
int songNum;
long seqPtrs[4];
long songPtr;
int chanMask;
long bankAmt;
int foundTable = 0;
int format = 0;
int mb = 0;
int isSFX = 0;

unsigned static char* romData;
unsigned static char* exBankData;

const char MagicBytes1[3] = { 0x40, 0x2A, 0xEA };
const char MagicBytes2[3] = { 0x40, 0x7E, 0xEA };
const char MagicBytes3[5] = { 0x16, 0x00, 0x19, 0x5E, 0x23 };
const char MagicBytes4[3] = { 0xCF, 0x5D, 0x54 };
const char MagicBytes5[3] = { 0xCF, 0x2A, 0xEA };
const char MagicBytes6[4] = { 0xCD, 0x81, 0x4C, 0x2A };
const char MagicBytes6b[4] = { 0xCD, 0x8B, 0x4C, 0x2A };
const char MagicBytes6c[4] = { 0xCD, 0x81, 0x03, 0x2A };
const char MagicBytes7[3] = { 0x61, 0x7E, 0xEA };
const char MagicBytes8[4] = { 0xEA, 0xCF, 0xC0, 0x21 };
const char MagicBytes9[3] = { 0x48, 0x2A, 0xEA };
const char MagicBytes10[3] = { 0x55, 0x2A, 0xEA };
const char MagicBytes11[3] = { 0xF5, 0xEF, 0xF1 };
const char MagicBytes12[3] = { 0xCB, 0x7F, 0x28 };
const char MagicBytes13[3] = { 0x49, 0x2A, 0xEA };

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptr);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("Konami (GB/GBC) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: KONA2TXT <rom> <bank>\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}
			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);

			/*Try to search the bank for song table loader - Method 1: Castlevania 1/2*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes1, 3)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);

					if (romData[i - 1] != 0x33)
					{
						format = 1;
						if (romData[i - 8] != 0xEA)
						{
							format = 3;
						}
					}
					foundTable = 1;
				}
			}

			/*Method 2: Nemesis*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes2, 3)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 1;
				}
			}

			/*Method 3: Motocross Maniacs*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes3, 5)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 2;
				}
			}

			/*Method 4: NFL Football*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes2, 3)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 1;
				}
			}

			/*Method 5: Skate or Die*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes5, 3)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 0;

				}
			}

			/*Method 6: Konami Golf (E)*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes6, 4)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 1;

				}
			}

			/*Method 6b: Konami Golf (J)*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes6b, 4)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 1;

				}
			}

			/*Method 6c: Ultra Golf (U)*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes6c, 4)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 1;

				}
			}

			/*Method 7: TMNT2*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes8, 4)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i + 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 3;

				}
			}

			/*Method 8: Batman*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes9, 3)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 3;
				}
			}

			/*Method 9: Tiny Toon Adventures 2*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes10, 3)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 3;
				}
			}

			/*Method 10: Zen*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes12, 3)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2 && romData[i - 5] == 0x21)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 3;
				}
			}

			/*Method 11: TTA Wacky Sports*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes13, 3)) && foundTable != 1 && ReadLE16(&romData[i - 4]) < bankSize * 2)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 3;
				}
			}


			/*Multi-banked method 1: Nemesis 2*/
			if (ReadLE16(&romData[0]) >= bankAmt && ReadLE16(&romData[0]) < bankSize * 2 && foundTable != 1)
			{
				tableOffset = 0x4000;
				printf("Song table starts at 0x%04x...\n", tableOffset);
				foundTable = 1;
				format = 3;
				if (ReadLE16(&romData[0]) == 0x4000 || ReadLE16(&romData[0]) == 0x4002)
				{
					mb = 2;
				}
			}
			
			/*Multi-banked method 2b: Goemon 1*/
			if (ReadLE16(&romData[0x3000]) > bankAmt && ReadLE16(&romData[0x3000]) < bankSize * 2 && foundTable != 1)
			{
				tableOffset = 0x7000;
				printf("Song table starts at 0x%04x...\n", tableOffset);
				foundTable = 1;
				format = 3;
			}

			/*Multi-banked method 2: Parodius*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes7, 3)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 1;
					mb = 1;

				}
			}

			/*Multi-banked method 3: Kid Dracula*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes11, 3)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
					format = 1;
					mb = 1;

				}
			}

			if (foundTable == 1)
			{
				i = tableOffset - bankAmt;
				if (ReadLE16(&romData[i]) > bankSize * 2 || ReadLE16(&romData[i]) < bankSize || format == 1 || format == 3)
				{
					i += 2;
				}
				songNum = 1;
				while (ReadLE16(&romData[i]) > bankAmt)
				{
					songPtr = ReadLE16(&romData[i]);
					if (tableOffset == 0x4000 && songPtr > 0x7000)
					{
						break;
					}

					if (songPtr == 0x7840)
					{
						break;
					}
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					if (songPtr != 0)
					{
						song2txt(songNum, songPtr);
					}
					i += 2;
					songNum++;
				}
			}
			else
			{
				printf("ERROR: Magic bytes not found!\n");
				exit(-1);
			}


			printf("The operation was successfully completed!\n");
			return 0;
		}
	}
}

void song2txt(int songNum, long ptr)
{
	int activeChan[4];
	int maskArray[4];
	unsigned char mask = 0;
	long romPos = 0;
	long seqPos = 0;
	int curTrack = 0;
	int seqEnd = 0;
	int curNote = 0;
	int curNoteLen = 0;
	int curIns = 0;
	int chanSpeed = 0;
	int octave = 0;
	int transpose = 0;
	int repeat1 = 0;
	long repeat1Pt = 0;
	int repeat2 = 0;
	long repeat2Pt = 0;
	int tempo = 0;
	int k = 0;
	int curVol = 0;
	long jumpPos = 0;
	int setRep1 = 0;
	int setRep2 = 0;
	int setLoop = 0;
	int tempoVal = 0;
	unsigned char command[4];
	unsigned char lowNibble = 0;
	unsigned char highNibble = 0;
	long loop = 0;
	int songBank = 0;
	int romBank = bank - 1;
	sprintf(outfile, "song%i.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%i.txt!\n", songNum);
		exit(2);
	}
	else
	{
		romPos = ptr - bankAmt;
		if (mb == 0)
		{
			mask = romData[romPos + 1];
		}
		else if (mb == 1)
		{
			/*Get the current bank*/
			songBank = romData[romPos + 1];
			fseek(rom, ((songBank)*bankSize), SEEK_SET);
			exBankData = (unsigned char*)malloc(bankSize);
			fread(exBankData, 1, bankSize, rom);

			mask = romData[romPos + 2];
		}
		else if (mb == 2)
		{
			/*Get the current bank*/
			lowNibble = (romData[romPos] >> 4);
			highNibble = (romData[romPos] & 15);

			songBank = romBank + lowNibble;
			fseek(rom, ((songBank)*bankSize), SEEK_SET);
			exBankData = (unsigned char*)malloc(bankSize);
			fread(exBankData, 1, bankSize, rom);

			mask = romData[romPos + 1];
		}


		/*Format 1 (Castlevania 1)*/
		if (format == 0)
		{
			for (k = 0; k < 4; k++)
			{
				maskArray[k] = 0;
			}


			/*Try to get active channels for sound effects*/
			maskArray[3] = mask >> 1 & 1;
			maskArray[0] = mask & 1;

			if (maskArray[0] == 0 && maskArray[3] == 0)
			{
				fprintf(txt, "Song type: Music\n");
				maskArray[3] = mask >> 5 & 1;
				maskArray[2] = mask >> 4 & 1;
				maskArray[1] = mask >> 3 & 1;
				maskArray[0] = mask >> 2 & 1;
			}

			else
			{
				fprintf(txt, "Song type: Sound effect\n");
			}
		}

		/*Format 2 (Castlevania 2/Nemesis)*/
		else if (format == 1 || format == 3)
		{
			for (k = 0; k < 4; k++)
			{
				maskArray[k] = 0;
			}


			/*Try to get active channels for sound effects*/
			maskArray[3] = mask >> 1 & 1;
			maskArray[1] = mask >> 2 & 1;
			maskArray[0] = mask & 1;

			if (maskArray[0] == 0 && maskArray[1] == 0 && maskArray[3] == 0)
			{
				fprintf(txt, "Song type: Music\n");
				maskArray[3] = mask >> 6 & 1;
				maskArray[2] = mask >> 5 & 1;
				maskArray[1] = mask >> 4 & 1;
				maskArray[0] = mask >> 3 & 1;
			}

			else
			{
				fprintf(txt, "Song type: Sound effect\n");
			}
		}

		/*Format 3 (Motocross Maniacs)*/
		else if (format == 2)
		{
			for (k = 0; k < 4; k++)
			{
				maskArray[k] = 0;
			}


			/*Try to get active channels for sound effects*/
			maskArray[3] = mask >> 2 & 1;
			maskArray[0] = mask >> 1 & 1;

			if (maskArray[0] == 0 && maskArray[3] == 0)
			{
				fprintf(txt, "Song type: Music\n");
				maskArray[3] = mask >> 6 & 1;
				maskArray[2] = mask >> 5 & 1;
				maskArray[1] = mask >> 4 & 1;
				maskArray[0] = mask >> 3 & 1;
			}

			else
			{
				fprintf(txt, "Song type: Sound effect\n");
			}
		}

		if (mb == 0 || mb == 2)
		{
			romPos += 2;
		}
		else if (mb == 1)
		{
			romPos += 3;

		}

		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			if (maskArray[curTrack] != 0)
			{
				seqEnd = 0;
				seqPos = ReadLE16(&romData[romPos]);
				setRep1 = 0;
				setRep2 = 0;
				setLoop = 0;
				fprintf(txt, "Channel %i: 0x%04X\n", curTrack + 1, seqPos);
				fprintf(txt, "\n");

				while (seqEnd == 0 && seqPos < bankSize * 2)
				{
					if (mb == 0)
					{
						command[0] = romData[seqPos - bankAmt];
						command[1] = romData[seqPos + 1 - bankAmt];
						command[2] = romData[seqPos + 2 - bankAmt];
						command[3] = romData[seqPos + 3 - bankAmt];
					}
					else if (mb == 1 || mb == 2)
					{
						command[0] = exBankData[seqPos - bankAmt];
						command[1] = exBankData[seqPos + 1 - bankAmt];
						command[2] = exBankData[seqPos + 2 - bankAmt];
						command[3] = exBankData[seqPos + 3 - bankAmt];
					}

					/*Play note*/
					if (command[0] < 0xE0)
					{
						lowNibble = (command[0] >> 4);
						highNibble = (command[0] & 15);
						curNote = lowNibble;
						curNoteLen = highNibble;

						/*Rest*/
						if (lowNibble == 0)
						{
							fprintf(txt, "Rest, length: %i\n", curNoteLen);
							seqPos++;
						}
						else
						{
							fprintf(txt, "Play note: %01X, length: %i\n", curNote, curNoteLen);
							seqPos++;
						}

					}

					/*Set parameters*/
					else if (command[0] == 0xE0)
					{
						if (curTrack < 3)
						{
							chanSpeed = command[1];
							fprintf(txt, "Set channel parameters: speed: %i, duty: %01X, fade out: %01X\n", chanSpeed, command[2], command[3]);
							seqPos += 4;
						}
						else if (curTrack == 3)
						{
							chanSpeed = command[1];
							fprintf(txt, "Set channel parameters: speed: %i\n", chanSpeed);
							seqPos += 2;
						}

					}

					/*Set octave*/
					else if (command[0] > 0xE0 && command[0] < 0xE7)
					{
						lowNibble = (command[0] >> 4);
						highNibble = (command[0] & 15);
						octave = highNibble;
						fprintf(txt, "Set octave: %i\n", octave);
						seqPos++;
					}

					/*Set channel speed*/
					else if (command[0] == 0xE7)
					{
						curNoteLen = command[1];
						fprintf(txt, "Set channel speed: %i\n", curNoteLen);
						seqPos += 2;
					}

					/*Set note decay?*/
					else if (command[0] == 0xE8)
					{
						fprintf(txt, "Set note decay?: %01X\n", command[1]);
						seqPos += 2;
					}

					/*Set channel pan?*/
					else if (command[0] == 0xE9)
					{
						fprintf(txt, "Set channel pan?: %01x\n", command[1]);
						seqPos += 2;
					}

					/*Change instrument or transpose*/
					else if (command[0] == 0xEA)
					{
						lowNibble = (command[1] >> 4);
						highNibble = (command[1] & 15);

						/*Transpose up*/
						if (lowNibble == 0)
						{
							transpose = highNibble;
							fprintf(txt, "Transpose up: %i\n", transpose);
						}

						/*Transpose down*/
						else if (lowNibble == 1)
						{
							transpose = highNibble * -1;
							fprintf(txt, "Transpose down: %i\n", transpose);
						}

						else if (lowNibble > 1)
						{
							curIns = command[1] - 0x0F;
							fprintf(txt, "Change instrument: %i\n", curIns);
						}

						seqPos += 2;
					}

					/*Effect/portamento*/
					else if (command[0] == 0xEB)
					{
						if (command[1] != 0x00)
						{
							fprintf(txt, "Effect/portamento: %i, %i\n", command[1], command[2]);
							seqPos += 3;
						}

						else if (command[1] == 0x00)
						{
							fprintf(txt, "Effect/portamento: %i\n", command[1]);
							seqPos += 2;
						}
						
					}

					/*Effect 2?*/
					else if (command[0] == 0xEC)
					{
						fprintf(txt, "Effect 2?: %i\n", (command[1] * -1));
						seqPos += 2;
					}

					/*Set envelope?*/
					else if (command[0] == 0xED)
					{
						fprintf(txt, "Set envelope?: %01X\n", command[1]);
						seqPos += 2;
					}

					/*Set tempo*/
					else if (command[0] == 0xEE)
					{
						tempo = command[1];
						fprintf(txt, "Set tempo: %01X\n", tempo);
						seqPos += 2;
					}

					/*Set panning?*/
					else if (command[0] == 0xEF)
					{
						fprintf(txt, "Set panning?: %01X\n", command[1]);
						seqPos += 2;
					}

					/*Set note volume?*/
					else if (command[0] >= 0xF0 && command[0] < 0xFB)
					{
						curVol = command[0] - 0xF0;
						fprintf(txt, "Set note volume?: %01X\n", curVol);
						seqPos++;
					}

					/*Start of repeat point OR repeat 1 (later format)*/
					else if (command[0] == 0xFB)
					{
						if (format < 3)
						{
							repeat1Pt = seqPos + 1;
							fprintf(txt, "Start of repeat point\n");
							seqPos++;
						}
						else if (format == 3)
						{
							if (setRep1 == 0)
							{
								setRep1 = 1;
								repeat1Pt = seqPos + 1;
								fprintf(txt, "Start of repeat 1 point\n");
								seqPos++;
							}
							else if (setRep1 == 1)
							{
								repeat1 = command[1];
								fprintf(txt, "Repeat 1 section: %i times\n", repeat1);
								seqPos += 2;
								setRep1 = 0;
							}

						}

					}

					/*Return from jump OR for later format, repeat 2*/
					else if (command[0] == 0xFC)
					{
						if (format < 3)
						{
							fprintf(txt, "Return from jump\n");
							seqPos++;
						}
						else if (format == 3)
						{
							if (setRep2 == 0)
							{
								setRep2 = 1;
								repeat2Pt = seqPos + 1;
								fprintf(txt, "Start of repeat 2 point\n");
								seqPos++;
							}
							else if (setRep2 == 1)
							{
								repeat2 = command[1];
								fprintf(txt, "Repeat 2 section: %i times\n", repeat2);
								seqPos += 2;
								setRep2 = 0;
							}
						}

					}

					/*Go to position*/
					else if (command[0] == 0xFD)
					{
						if (mb == 0)
						{
							jumpPos = ReadLE16(&romData[seqPos - bankAmt + 1]);
						}
						else if (mb == 1 || mb == 2)
						{
							jumpPos = ReadLE16(&exBankData[seqPos - bankAmt + 1]);
						}

						fprintf(txt, "Go to position: 0x%04X\n", jumpPos);
						seqEnd = 1;
					}

					/*Repeat section starting from FB value OR start/end of loop point (later format)*/
					else if (command[0] == 0xFE)
					{
						if (format < 3)
						{
							if (command[1] != 0xFF)
							{
								repeat1 = command[1];
								fprintf(txt, "Repeat section: %i times\n", repeat1);
								seqPos += 2;
							}

							/*Go to loop point?*/
							else if (command[1] == 0xFF)
							{
								if (mb == 0)
								{
									loop = ReadLE16(&romData[seqPos - bankAmt + 2]);
								}
								else if (mb == 1 || mb == 2)
								{
									loop = ReadLE16(&exBankData[seqPos - bankAmt + 2]);
								}

								fprintf(txt, "Go to loop point?: 0x%04X\n", loop);
								seqEnd = 1;
							}
						}

						else if (format == 3)
						{
							if (setLoop == 0)
							{
								loop = seqPos + 1;
								fprintf(txt, "Start of loop point\n");
								setLoop = 1;
								seqPos++;
							}
							else if (setLoop == 1)
							{
								fprintf(txt, "End of loop point\n");
								seqEnd = 1;
							}

						}

					}

					/*End of sequence*/
					else if (command[0] == 0xFF)
					{
						fprintf(txt, "End of sequence\n");
						seqEnd = 1;
					}

					/*Unknown command*/
					else
					{
						fprintf(txt, "Unknown command: %01X\n", command[0]);
						seqPos += 2;
					}
				}
				fprintf(txt, "\n");
				romPos += 2;


			}
		}
		fclose(txt);
	}
}