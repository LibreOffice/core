# This is the Linux gnu configuration file for DMAKE
#	It makes sure we include from the right place.
#
CFLAGS += -I$(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)
