# Define additional MSDOS specific settings.
#

# --- Set Wrapper command ---
# Provide a macro that can be used to access the wrapper and to avoid
# hardcoding the program name everywhere
GUWCMD*=guw.exe

# This is a no-op for native W32 dmake
.WINPATH !:= yes

# Directory cache configuration.
.DIRCACHE  *:= no

# Applicable suffix definitions
E *:= .exe	# Executables

# Now set the remaining arguments depending on which SHELL we
# are going to use.
.IF $(USE_SHELL) == 4nt

.IMPORT : COMSPEC

SHELL *:= $(COMSPEC)

   SHELLFLAGS       *:= $(SWITCHAR)c
   GROUPFLAGS       *:= $(SHELLFLAGS)
   SHELLMETAS       *:= "<>|&%^
# Fix syntax highlighting: "
   GROUPSUFFIX      *:= .bat
   DIVFILE          *=  $(TMPFILE:s,/,\,)
   RM               *=  +del
   RMFLAGS          *= /y /E
   MV	            *=  +rename
   __.DIVSEP-sh-yes *:= \\
   __.DIVSEP-sh-no  *:= \\
   DIRSEPSTR        := \\

.ELSE	# Non 4nt case

.IF $(USE_SHELL) == bash
   SHELLFLAGS       *:= -c
.ELSE
   # See iz50689 why -f is needed.
   SHELLFLAGS       *:= -fc
.ENDIF # $(USE_SHELL) == bash

   GROUPFLAGS       *:=
   SHELLMETAS       *:= *";?<>|()&][$$\#`'
# Fix syntax highlighting: "
   GROUPSUFFIX      *:= .csh
# This is hopefully not used. Only in: dmake/msdos/spawn.c  
#  .MKSARGS         *:= yes
   DIVFILE          *=  $(TMPFILE:s,/,${__.DIVSEP-sh-${USESHELL}})
   RM               *=  $/bin$/rm
   RMFLAGS          *=  -f
   MV	            *=  $/bin$/mv
   __.DIVSEP-sh-yes *:= \\\
   __.DIVSEP-sh-no  *:= \\
   DIRSEPSTR :=/

.ENDIF

.EXPORT : GUWCMD

.IF $(USE_SHELL) == 4nt

my4ver:=$(shell @echo %_4ver)

.IF "$(my4ver:s/.//:s/,//)" >= "400"
.ELSE			# "$(my4ver:s/.//:s/,//)" >= "400"
.ERROR : ; @echo Forced error: 4NT version too old! Version 4.0 or newer is required.
4NT_version_too_old
.ENDIF			# "$(my4ver:s/.//:s/,//)" >= "400"

.ELSE # $(USE_SHELL) == 4nt

.IF $(SHELL) == $(NULL)
# SHELL has to be set in (non-4NT) OOobuild environment
.ERROR : ; @echo Forced error: Environment variable SHELL has to be set for OOo build!
SHELL_variable_needed
.END

.END # $(USE_SHELL) == 4nt

GROUPSHELL *:= $(SHELL)

# Does not respect case of filenames.
.DIRCACHERESPCASE := no
NAMEMAX	=	256

