#
# $Id: make_mac.sh,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $
#
# This MPW script builds the dmake executable using the Metroworks PPC compiler
# and linker. The resulting dmake binary will only run on the PPC platform.
#
# To execute this script, you will need the following tools:
# - MacOS 8 or later
# - CodeWarrior for MacOS Release 5 or later
#
# To run this script, you need to open the MPW Shell that is bundled with CodeWarrior.
# If you have another instance of MPW installed other than the one bundled with
# CodeWarrior, you may encounter problems. Once you open the MPW Shell that is bundled
# with CodeWarrior, you will need to set MPW's working directory to the "dmake"
# directory (the directory above the directory that this script is in). Then, execute
# this script from the MPW Worksheet by typing the following command:
#
#   :mac:make_mac.sh
#
set Exit 1
if ( "{0}" != ':mac:make_mac.sh' )
  Echo "You cannot run this script from the current directory."
  Echo "To run this script, you need to be in the ¶"dmake¶" directory and then"
  Echo "execute the following command: ¶":mac:make_mac.sh¶""
  Exit
end
if ( ! `Exists -d objects` )
  NewFolder objects
end
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle infer.c
Move -y infer.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle make.c
Move -y make.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle stat.c
Move -y stat.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle expand.c
Move -y expand.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle dmstring.c
Move -y dmstring.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle hash.c
Move -y hash.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle dag.c
Move -y dag.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle dmake.c
Move -y dmake.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle path.c
Move -y path.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle imacs.c
Move -y imacs.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle sysintf.c
Move -y sysintf.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle parse.c
Move -y parse.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle getinp.c
Move -y getinp.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle quit.c
Move -y quit.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle state.c
Move -y state.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle dmdump.c
Move -y dmdump.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle macparse.c
Move -y macparse.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle rulparse.c
Move -y rulparse.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle percent.c
Move -y percent.c.o objects
# Note that function.c needs to have __useAppleExts__ defined. Otherwise, it won't link.
MWCPPC -o : -ansi off -I :mac -d _MPW -d __useAppleExts__ -sym off -proto ignoreoldstyle function.c
Move -y function.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:arlib.c
Move -y arlib.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:bogus.c
Move -y bogus.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:dirbrk.c
Move -y dirbrk.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:directry.c
Move -y directry.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:environ.c
Move -y environ.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:main.c
Move -y main.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:rmprq.c
Move -y rmprq.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:ruletab.c
Move -y ruletab.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:tempnam.c
Move -y tempnam.c.o objects
MWCPPC -o : -ansi off -I :mac -d _MPW -sym off -proto ignoreoldstyle :mac:tomacfil.c
Move -y tomacfil.c.o objects
MWLinkPPC -w -c 'MPS ' -t MPST -sym off -xm m -o dmake :objects:infer.c.o :objects:make.c.o ¶
  :objects:stat.c.o :objects:expand.c.o :objects:dmstring.c.o :objects:hash.c.o ¶
  :objects:dag.c.o :objects:dmake.c.o :objects:path.c.o :objects:imacs.c.o ¶
  :objects:sysintf.c.o :objects:parse.c.o :objects:getinp.c.o :objects:quit.c.o ¶
  :objects:state.c.o :objects:dmdump.c.o :objects:macparse.c.o :objects:rulparse.c.o ¶
  :objects:percent.c.o :objects:function.c.o :objects:arlib.c.o :objects:bogus.c.o ¶
  :objects:dirbrk.c.o :objects:directry.c.o :objects:environ.c.o :objects:main.c.o ¶
  :objects:rmprq.c.o :objects:ruletab.c.o :objects:tempnam.c.o :objects:tomacfil.c.o ¶
  "{SharedLibraries}StdCLib" ¶
  "{SharedLibraries}InterfaceLib" ¶
  "{PPCLibraries}StdCRuntime.o" ¶
  "{PPCLibraries}PPCCRuntime.o" ¶
  "{PPCLibraries}PPCToolLibs.o"
SetFile -a l :startup:config.mk
Duplicate -y :mac:template.mk :startup:config.mk
