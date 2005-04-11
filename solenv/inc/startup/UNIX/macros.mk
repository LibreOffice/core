# This file is used for all $(OS)==UNX during startup
#

SHELL *:= /usr/bin/csh
SHELLFLAGS	 *:=
GROUPSHELL *:= $(SHELL)

   SHELLFLAGS       := -fc
   GROUPFLAGS       *:= $(SHELLFLAGS)
   SHELLMETAS       *:= "<>|/
   RM               *=  rm
   RMFLAGS          *=  -f
   MV	            *=  mv
