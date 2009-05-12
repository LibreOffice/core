#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.9 $
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

TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

PRJNAME=rsc
TARGET=rscpp

.IF "$(cpp)" != ""
PRJNAME=CPP
TARGET=cpp
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

CDEFS+=-DSOLAR

.IF "$(cpp)" != ""
CDEFS+=-DNOMAIN
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES=   \
            $(OBJ)$/cpp1.obj \
            $(OBJ)$/cpp2.obj \
            $(OBJ)$/cpp3.obj \
            $(OBJ)$/cpp4.obj \
            $(OBJ)$/cpp5.obj \
            $(OBJ)$/cpp6.obj	\

.IF "$(cpp)" == ""
LIBSALCPPRT=$(0)
APP1TARGET= $(TARGET)
APP1LIBS=$(LB)$/$(TARGET).lib
.IF "$(GUI)" != "OS2"
APP1STACK=32768
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
CDEFS+=-Dunix
.ENDIF

.IF "$(OS)$(CPU)"=="SOLARISI"
# cc: Sun C 5.5 Patch 112761-10 2004/08/10
# Solaris x86 compiler ICE
# "cpp6.c", [get]:ube: internal error
# remove after compiler upgrade
NOOPTFILES=$(OBJ)$/cpp6.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

cpp1.c: cppdef.h cpp.h
cpp2.c: cppdef.h cpp.h
cpp3.c: cppdef.h cpp.h
cpp4.c: cppdef.h cpp.h
cpp5.c: cppdef.h cpp.h
cpp6.c: cppdef.h cpp.h 


