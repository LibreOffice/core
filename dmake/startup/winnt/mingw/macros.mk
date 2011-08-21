# Define additional MinGW specific macros.
#

# Process environment-specific refinements, if any.
.IF $(OSENVIRONMENT)
   .INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSENVIRONMENT)$/macros.mk
.ENDIF

# Make OS-release-specific settings
#PRINT *:= lpr
