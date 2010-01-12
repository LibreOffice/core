# Define additional OS/2 specific macros.
#

# Process release-specific refinements, if any.
# YD .INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSRELEASE)$/macros.mk

# Directory entries are case incensitive
.DIRCACHERESPCASE *:= no

# tmp is already imported - use it!
TMPDIR *= $(TMP)

# Applicable suffix definitions
A *:= .lib	# Libraries
E *:= .exe	# Executables
F *:= .for	# Fortran
O *:= .obj	# Objects
P *:= .pas	# Pascal
S *:= .asm	# Assembler sources
V *:= 		# RCS suffix

.MKSARGS         *:= yes
RM               *=  $(ROOTDIR)$/bin$/rm
MV	            *=  $(ROOTDIR)$/bin$/mv
DIVFILE          *=  $(TMPFILE:s,/,${__.DIVSEP-sh-${USESHELL}})
__.DIVSEP-sh-yes !:= \\\
__.DIVSEP-sh-no  !:= \\

NAMEMAX		=	256

