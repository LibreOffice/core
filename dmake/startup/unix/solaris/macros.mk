# We hang off the standard sysvr3'isms
#
__.incdir !:= $(INCFILENAME:d:d:d:d)/sysvr3

# Process environment-specific refinements, if any.
.INCLUDE .SETDIR=$(__.incdir) .NOINFER .IGNORE : macros.mk
