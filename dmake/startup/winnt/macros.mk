# Define additional MSDOS specific settings.
#

# Execution environment configuration.
# Grab the current setting of COMSPEC.
#
.IMPORT .IGNORE : COMSPEC

# First check if SHELL is defined to be something other than COMSPEC.
# If it is, then assume that SHELL is a Korn compatible shell like MKS's
.IF $(SHELL) == $(NULL)
   .IF $(COMSPEC) == $(NULL)
      SHELL *:= $(ROOTDIR)$/bin$/sh$E
   .ELSE
      SHELL *:= $(COMSPEC)
   .END
.END
GROUPSHELL *:= $(SHELL)

# Process release-specific refinements, if any.
.INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSRELEASE)$/macros.mk

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
   SHELLMETAS       *:= "<>|
   GROUPSUFFIX      *:= .cmd
   DIVFILE          *=  $(TMPFILE:s,/,\)
   RM               *=  del
   RMFLAGS          *=
   MV	            *=  rename
   __.DIVSEP-sh-yes *:= \\
   __.DIVSEP-sh-no  *:= \\
.ELSE
   SHELL	    !:= $(SHELL:s,/,\,)
   COMMAND          *=  $(CMNDNAME:s,/,\,) $(CMNDARGS)
   SHELLFLAGS       *:= -c
   GROUPFLAGS       *:= 
   SHELLMETAS       *:= *";?<>|()&][$$\#`'
   GROUPSUFFIX      *:= .ksh
   .MKSARGS         *:= yes
   RM               *=  $(ROOTDIR)$/bin$/rm
   RMFLAGS          *=  -f
   MV	            *=  $(ROOTDIR)$/bin$/mv
   DIVFILE          *=  $(TMPFILE:s,/,${__.DIVSEP-sh-${USESHELL}})
   __.DIVSEP-sh-yes *:= \\\
   __.DIVSEP-sh-no  *:= \\
.ENDIF


# Does not respect case of filenames.
.DIRCACHERESPCASE := no
