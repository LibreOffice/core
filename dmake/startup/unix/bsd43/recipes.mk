# Define additional Berkeley UNIX specific build rules and recipes.
#

# Recipe to make archive files.
%$A .GROUP :
    $(AR) $(ARFLAGS) $@ $?
    $(RM) $(RMFLAGS) $?
    $(RANLIB) $@

# Process environment-specific refinements, if any.
.IF $(OSENVIRONMENT)
   .INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSENVIRONMENT)$/recipes.mk
.ENDIF
