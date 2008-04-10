#*************************************************************************
#*
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.12 $
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
#************************************************************************/

PRJ=..$/..


PRJNAME=i18npool
TARGET=locale

TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)" == "SOLARIS"
# the xmlparser build breaks in this header file
# 
#      /opt/SUNWspro/WS6U1/include/CC/Cstd/./limits
#
# which defines a class with member functions called min() and max().
# the build breaks because in solar.h, there is something like this
# 
#      #define max(x,y) ((x) < (y) ? (y) : (x))
#      #define min(x,y) ((x) < (y) ? (x) : (y))
#
# so the only choice is to prevent "CC/Cstd/./limits" from being 
# included:
CDEFS+=-D__STD_LIMITS
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=   $(SLO)$/localedata.obj

OBJFILES   = \
        $(OBJ)$/saxparser.obj \
        $(OBJ)$/LocaleNode.obj \
        $(OBJ)$/filewriter.obj

APP1TARGET = saxparser

APP1OBJS   = $(OBJFILES)

APP1STDLIBS = \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB)

DEPOBJFILES = \
        $(OBJ)$/saxparser.obj \
        $(OBJ)$/LocaleNode.obj \
        $(OBJ)$/filewriter.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

