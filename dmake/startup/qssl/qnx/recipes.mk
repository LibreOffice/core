# Define additional QNX specific build recipes.
#

# Recipe to make archive files.
# --Figure out what to do about the librarian--
%$A .GROUP :
    $(AR) $(ARFLAGS) $@ $?
    $(RM) $(RMFLAGS) $?
