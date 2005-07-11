# Define additional MSDOS specific settings.
#

# Execution environment configuration.
# Grab the current setting of COMSPEC.
#
.IMPORT .IGNORE : COMSPEC

.IF $(SHELL) == $(NULL)
   .IF $(COMSPEC) == $(NULL)
# SHELL has to be set in OOo build environment
        .ERROR : ; @echo Forced error: Environment variable SHELL has to be set for OOo build!
        SHELL_variable_needed
   .ELSE
      SHELL *:= $(COMSPEC)
   .END
.END

GROUPSHELL *:= $(SHELL)

# Standard C-language command names and flags
CC         *:= cl		# C   compiler 

# Directory cache configuration.
.DIRCACHE  *:= no

# Applicable suffix definitions
E *:= .exe	# Executables

# Now set the remaining arguments depending on which SHELL we
# are going to use.
.IF $(SHELL) == $(COMSPEC)
   SHELLFLAGS       *:= $(SWITCHAR)c
   GROUPFLAGS       *:= $(SHELLFLAGS)
   SHELLMETAS       *:= "<>|
   GROUPSUFFIX      *:= .bat
   DIVFILE          *=  $(TMPFILE:s,/,\,)
   RM               *=  del
   RMFLAGS          *= /y
   MV	            *=  rename
   __.DIVSEP-sh-yes *:= \\
   __.DIVSEP-sh-no  *:= \\
   DIRSEPSTR        := \\
   PWD:=$(shell +echo %_cwd)
.EXPORT : PWD
   
.ELSE	# Non 4nt case
# See iz50689 why -f is needed.
   SHELLFLAGS       *:= -fc
   GROUPFLAGS       *:=
   SHELLMETAS       *:= *";?<>|()&][$$\#`'
   GROUPSUFFIX      *:= .csh
# This is hopefully not used. Only in: dmake/msdos/spawn.c  
#  .MKSARGS         *:= yes
   DIVFILE          *=  $(TMPFILE:s,/,${__.DIVSEP-sh-${USESHELL}})
   RM               *=  $(ROOTDIR)$/bin$/rm
   RMFLAGS          *=  -f
   MV	            *=  $(ROOTDIR)$/bin$/mv
   __.DIVSEP-sh-yes *:= \\\
   __.DIVSEP-sh-no  *:= \\
   DIRSEPSTR :=/
# See IZ 11093 why PWD has to be overwritten
   PWD:=$(shell +pwd)
.EXPORT : PWD

.ENDIF

.IF "$(GUI)$(USE_SHELL)"=="WNT4nt"
my4ver:=$(shell +echo %_4ver)
.ENDIF			# "$(GUI)$(USE_SHELL)"=="WNT4nt"

.IF "$(my4ver:s/.//:s/,//)" >= "400"
   RMFLAGS          +=/E
.ELSE			# "$(my4ver:s/.//:s/,//)" >= "400"
UNSUPPORTED_4nt_VERSION=TRUE
.ENDIF			# "$(my4ver:s/.//:s/,//)" >= "400"

# Does not respect case of filenames.
.DIRCACHERESPCASE := no
NAMEMAX	=	256

