
bin_PROGRAMS = dmake
dmake_SOURCES = \
	infer.c make.c stat.c expand.c dmstring.c hash.c dag.c dmake.c\
	path.c imacs.c sysintf.c parse.c getinp.c quit.c state.c\
	dmdump.c macparse.c rulparse.c percent.c function.c
dmake_LDADD   = unix/libunix.a

SUBDIRS = unix startup

EXTRA_DIST = make.c 

bin_PROGRAMS = dmake

# set the include path found by configure
INCLUDES = -I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/unix/linux -I$(top_srcdir)/unix -I$(top_srcdir)/unix/linux/gnu 

                                                                                
