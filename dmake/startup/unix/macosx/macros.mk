# Define additional Mac OS X specific macros.
#

# Process environment-specific refinements, if any.
.IF $(OSENVIRONMENT)
   .INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSENVIRONMENT)$/macros.mk
.ENDIF

# Make OS-release-specific settings
PRINT *:= lpr
