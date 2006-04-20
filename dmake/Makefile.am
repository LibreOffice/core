#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: Makefile.am,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: hr $ $Date: 2006-04-20 11:56:21 $
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

# require "new" automake >= 1.7
AUTOMAKE_OPTIONS = 1.7 foreign



bin_PROGRAMS = dmake
dmake_SOURCES = \
	infer.c make.c stat.c expand.c dmstring.c hash.c dag.c dmake.c\
	path.c imacs.c sysintf.c parse.c getinp.c quit.c state.c\
	dmdump.c macparse.c rulparse.c percent.c function.c
if DBUG
dmake_SOURCES += dbug/dbug/dbug.c
endif

#dmake_LDADD   = unix/libunix.a
#dmake_LDADD   = win95/libwin95.a msdos/libmsdos.a win95/microsft/libmsft.a
dmake_LDADD = @OS_LIBRARY@
dmake_DEPENDENCIES = @OS_LIBRARY@

DIST_SUBDIRS = unix startup win95 msdos tests
SUBDIRS = $(OS_SUBDIRS) tests

EXTRA_DIST = NEWS ChangeLog

# set the include path found by configure
INCLUDES = -I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/@OS_TYPE@/@OS_VERSION@ -I$(top_srcdir)/@OS_TYPE@ -I$(top_srcdir)/@OS_TYPE@/@OS_VERSION@/gnu 
