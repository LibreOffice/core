# Define additional Windows-specific settings.
#

# This is a no-op for native Windows dmake
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

# Does not respect case of filenames.
.DIRCACHERESPCASE := no
NAMEMAX	=	256
