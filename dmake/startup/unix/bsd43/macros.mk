# Define additional Berkely UNIX specific macros.
#

# Process environment-specific refinements, if any.
.IF $(OSENVIRONMENT)
   .INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSENVIRONMENT)$/macros.mk
.ENDIF

# Set defaults for local OS release
RANLIB *:= ranlib
PRINT  *:= lpr
