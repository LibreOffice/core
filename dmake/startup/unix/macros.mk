# Define additional UNIX specific macros.
#

# Process release-specific refinements, if any.
.INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSRELEASE)$/macros.mk
