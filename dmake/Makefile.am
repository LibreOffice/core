#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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
