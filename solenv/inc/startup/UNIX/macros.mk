# We hang off the standard sysvr3'isms
#
__.incdir !:= $(INCFILENAME:d:d:d:d)/sysvr3

# Process environment-specific refinements, if any.
.INCLUDE .SETDIR=$(__.incdir) .NOINFER .IGNORE : macros.mk

# Process release-specific refinements, if any.
.INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSRELEASE)$/macros.mk

# Execution environment configuration.
# Grab the current setting of COMSPEC.
#
.IMPORT .IGNORE : COMSPEC

# First check if SHELL is defined to be something other than COMSPEC.
# If it is, then assume that SHELL is a Korn compatible shell like MKS's
#SHELL *:= $(ROOTDIR)$/bin$/csh$E
SHELL *:= /usr/bin/csh
SHELLFLAGS	 *:=
GROUPSHELL *:= $(SHELL)

# Applicable suffix definitions
#	A *:= .lib	# Libraries
#	E *:= .exe	# Executables
#	F *:= .for	# Fortran
#	O *:= .obj	# Objects
#	P *:= .pas	# Pascal
#	S *:= .asm	# Assembler sources
#	V *:= 		# RCS suffix

#   SHELLFLAGS       := -f c		// Original mit falscher Option
   SHELLFLAGS       := -fc
   GROUPFLAGS       *:= $(SHELLFLAGS)
   SHELLMETAS       *:= "<>|/
   RM               *=  rm
   RMFLAGS          *=  -f
   MV	            *=  mv
