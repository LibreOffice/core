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

PRJ=..$/..

PRJNAME=setup_native
TARGET=getuid

NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# no 'lib' prefix
DLLPRE=

# no _version.o
VERSIONOBJ=

.IF "$(OS)" == "LINUX" || "$(OS)" == "SOLARIS"

.IF "$(OS)" == "LINUX" 
CFLAGS+=-D_GNU_SOURCE
.ENDIF

.IF "$(OS)" == "SOLARIS"
.IF "$(CPU)" == "I"
.IF "$(COM)" != "GCC"
CFLAGS+=-D_KERNEL
.ENDIF
.ENDIF
.ENDIF

# this object must not be a Ultra SPARC binary, this does not
# work with /usr/bin/sort and such. Needs to be 32 bit even in
# 64 bit builds
.IF "$(OS)" == "SOLARIS"
.IF "$(COM)" != "GCC"
.IF "$(CPU)" == "S"
ENVCFLAGS=-xarch=v8
.ENDIF
.IF "$(CPU)"=="U"
CFLAGSAPPEND+=-m32
LINKFLAGSSHL+=-m32
.ENDIF
.ENDIF
.ENDIF

SLOFILES = $(SLO)$/getuid.obj

SHL1OBJS=$(SLOFILES)
SHL1TARGET=$(TARGET)
SHL1STDLIBS=-ldl
SHL1CODETYPE=C

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

