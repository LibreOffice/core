# Define additional MSDOS specific build recipes.
#

# Executables
   %$E       : %$O ; $(CC) $(LDFLAGS) -o$@ $< $(LDLIBS)
   %$O       : %$S ; $(AS) $(ASFLAGS) $(<:s,/,\)

# Process release-specific refinements, if any.
.INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSRELEASE)$/recipes.mk
