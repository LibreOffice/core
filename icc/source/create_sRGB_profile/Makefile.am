## Process this file with automake to produce Makefile.in

bin_PROGRAMS = \
	create_display_profile

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/IccProfLib \
	-I$(top_srcdir)/Contrib/ICC_utils

LDADD = -L$(top_builddir)/IccProfLib -lSampleICC \
	-L$(top_builddir)/Contrib/ICC_utils -lICC_utils

create_display_profile_SOURCES = \
	create_display_profile.cpp
