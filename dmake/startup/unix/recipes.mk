# Define additional UNIX specific build recipes.
#

# Define additional build targets.
%$E : %.sh; cp $< $@; chmod 0777 $@

# This rule tells how to make a non-suffixed executable from its single
# file source.
% : %$O; $(CC) $(LDFLAGS) -o $@ $< $(LDLIBS)

# This rule tells how to make a.out from it's immediate list of prerequisites.
%.out :; $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Process release-specific refinements, if any.
.INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSRELEASE)$/recipes.mk
