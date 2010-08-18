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

# This is hopefully not used. Only in: dmake/msdos/spawn.c  
#  .MKSARGS         *:= yes
   DIVFILE          *=  $(TMPFILE:s,/,${__.DIVSEP-sh-${USESHELL}})
   RM               *=  $/bin$/rm
   MV	            *=  $/bin$/mv
   __.DIVSEP-sh-yes *:= \\\
   __.DIVSEP-sh-no  *:= \\
   DIRSEPSTR :=/

.EXPORT : GUWCMD

# Does not respect case of filenames.
.DIRCACHERESPCASE := no
NAMEMAX	=	256

