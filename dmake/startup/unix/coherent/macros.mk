# We hang off the standard BSD'isms
#
__.incdir !:= $(INCFILENAME:d:d:d:d)/bsd43

# Process environment-specific refinements, if any.
.INCLUDE .SETDIR=$(__.incdir) .NOINFER .IGNORE : macros.mk
