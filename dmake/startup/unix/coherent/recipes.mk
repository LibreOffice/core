# Use the same file as 
#
__.incdir !:= $(INCFILENAME:d:d:d:d)/bsd43

# Process environment-specific refinements, if any.
.INCLUDE .SETDIR=$(__.incdir) .NOINFER .IGNORE : recipes.mk
