#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: Makefile.am,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 18:26:19 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

AUTOMAKE_OPTIONS = foreign subdir-objects 1.9


bin_PROGRAMS = dmake
dmake_SOURCES = \
	infer.c make.c stat.c expand.c dmstring.c hash.c dag.c dmake.c\
	path.c imacs.c sysintf.c parse.c getinp.c quit.c state.c\
	dmdump.c macparse.c rulparse.c percent.c function.c

# set the include path found by configure
INCLUDES = -I$(top_srcdir) -I$(top_builddir) \
           -I$(top_srcdir)/@OS_TYPE@/@OS_VERSION@ -I$(top_srcdir)/@OS_TYPE@

if DBUG
dmake_SOURCES += dbug/dbug/dbug.c
endif

if OSTYPEUNIX
dmake_SOURCES += unix/arlib.c unix/dcache.c unix/dirbrk.c unix/rmprq.c\
                 unix/ruletab.c unix/runargv.c unix/tempnam.c
INCLUDES += -I$(top_srcdir)/@OS_TYPE@/@OS_VERSION@/gnu \
            -I$(top_srcdir)/@OS_TYPE@/@OS_VERSION@ \
            -I$(top_srcdir)/@OS_TYPE@ 
endif

if OSTYPEWIN32
dmake_SOURCES += win95/dchdir.c win95/switchar.c \
                 msdos/dstrlwr.c msdos/arlib.c \
                 msdos/dirbrk.c unix/runargv.c \
                 unix/rmprq.c \
                 win95/microsft/ruletab.c
INCLUDES += -I$(top_srcdir)/@OS_TYPE@/@OS_VERSION@ -I$(top_srcdir)/@OS_TYPE@
endif

SUBDIRS = startup tests

EXTRA_DIST = NEWS ChangeLog
