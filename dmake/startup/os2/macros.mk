# Define additional OS/2 specific macros.
#

# Process release-specific refinements, if any.
.INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSRELEASE)$/macros.mk

# Execution environment configuration.
# Grab the current setting of COMSPEC.
#
.IMPORT .IGNORE : COMSPEC

# First check if SHELL is defined to be something other than COMSPEC.
# If it is assume that SHELL is a Korn compatible shell like MKS's
.IF $(SHELL) == $(NULL)
   .IF $(COMSPEC) == $(NULL)
      SHELL *:= $(ROOTDIR)$/bin$/sh$E
   .ELSE
      SHELL *:= $(COMSPEC)
   .END
.END
GROUPSHELL *:= $(SHELL)

# Directory entries are case incensitive
.DIRCACHERESPCASE *:= no

# Applicable suffix definitions
A *:= .lib	# Libraries
E *:= .exe	# Executables
F *:= .for	# Fortran
O *:= .obj	# Objects
P *:= .pas	# Pascal
S *:= .asm	# Assembler sources
V *:= 		# RCS suffix

# Now set the remaining arguments depending on which SHELL we
# are going to use.  COMSPEC (assumed to be command.com) or
# MKS Korn shell.
.IF $(SHELL) == $(COMSPEC)
   SHELLFLAGS       *:= $(SWITCHAR)c
   GROUPFLAGS       *:= $(SHELLFLAGS)
   SHELLMETAS       *:= *"?<>
   GROUPSUFFIX      *:= .bat
   DIRSEPSTR        *:= \\\
   DIVFILE          *=  $(TMPFILE:s,/,\)
   RM               *=  del
   RMFLAGS          *=
   MV	            *=  rename
.ELSE
   SHELLFLAGS       *:= -c
   GROUPFLAGS       *:= 
   SHELLMETAS       *:= *"?<>|()&][$$\#`'
   GROUPSUFFIX      *:= .ksh
   .MKSARGS         *:= yes
   RM               *=  $(ROOTDIR)$/bin$/rm
   RMFLAGS          *=  -f
   MV	            *=  $(ROOTDIR)$/bin$/mv
   DIVFILE          *=  $(TMPFILE:s,/,${__.DIVSEP-sh-${USESHELL}})
   __.DIVSEP-sh-yes !:= \\\
   __.DIVSEP-sh-no  !:= \\
.ENDIF
