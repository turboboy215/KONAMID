# KONAMID
Konami (GB/GBC) to MIDI converter

This tool converts music from Game Boy and Game Boy Color games using Konami's original sound engine to MIDI format.
It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex).
Two more optional parameters can also be used for when auto-detection fails: format (slightly different variations of the sequence format) and multi-bank method. For format, options 0-9 are available, with examples below:
* 0 = Castlevania 1, Quarth, Skate or Die
* 1 = Castlevania 2, Nemesis 1, TMNT 1
* 2 = Motocross Maniacs
* 3 = Nemesis 2, Hyper Dunk, Track & Field
* 4 = TMNT 2
* 5 = Goemon 1/2, Castlevania Legends, most GBC games
* 6 = TMNT 3, TTA 2/Wacky Sports, Batman
* 7 = TTA 1, Kid Dracula
* 8 = Zen
* 9 = Mummy, Grinch, Woody Woodpecker Racing
For multi-bank, this should be handled automatically, except for International Superstar Soccer '99 where option 3 must be used for its second bank containing the music.
For games that contain multiple banks of music, you must run the program multiple times specifying where each different bank is located. However, in order to prevent files from being overwritten, the MIDI files from the previous bank must either be moved to a separate folder or renamed.

Examples:
* KONAMID "Castlevania Adventure, The (E) [!].gb" 3
* KONAMID "Nemesis II - The Return of the Hero (E) [!].gb" E
* KONAMID "Nemesis II - The Return of the Hero (E) [!].gb" F
* KONAMID "Nemesis II - The Return of the Hero (E) [!].gb" 10
* KONAMID "Mummy, The (E) (M3) [C][!].gbc" 9

This tool was based on my own reverse-engineering of the sound engine. Differences of the sequence format in various games were accounted for to ensure that all games using the driver are compatible.

Also included, as usual, is another program, KONA2TXT, a "prototype" program used to debug games, although support is very incomplete compared to the MIDI converter.

Supported games:
  * Azure Dreams
  * Batman: The Animated Series
  * Battle Fishers
  * Blades of Steel
  * Castlevania II: Belmont's Revenge
  * The Castlevania Adventure
  * Castlevania Legends
  * Cave Noire
  * Cross Country Racing
  * Cyborg Kuro-chan: Devil Fukkatsu
  * Ganbare Goemon: Sarawareta Ebisumaru!
  * God Medicine
  * God Medicine: Hukkoku Han
  * The Grinch
  * Hyper Dunk
  * International Superstar Soccer
  * International Superstar Soccer '99
  * International Track & Field
  * Kid Dracula
  * Kinnuku Banzuke GB: Chousensha wa Kimida!
  * Konami Golf
  * Motocross Maniacs
  * Mystical Ninja Starring Goemon
  * Nemesis
  * Nemesis II: The Return of the Hero
  * NFL Football
  * Parodius
  * Pocket GI Stable
  * Pop'n TwinBee
  * Power Pro GB
  * Quarth
  * Raging Fighter
  * Skate or Die: Bad 'n Rad
  * The Spirit of F-1
  * Stranded Kids
  * Teenage Mutant Ninja/Hero Turtles: Fall of the Foot Clan
  * Teenage Mutant Ninja/Hero Turtles II: Back from the Sewers
  * Teenage Mutant Ninja/Hero Turtles III: Radical Rescue
  * Tiny Toon Adventures: Babs' Big Break
  * Tiny Toon Adventures: Wacky Sports
  * Tiny Toon Adventures 2: Montana's Movie Madness
  * Track & Field
  * Woody Woodpecker Racing
  * Zen: Intergalactic Ninja

## To do:
  * Panning support
  * Support for other Konami sound engines (e.g. NES)
  * GBS file support
