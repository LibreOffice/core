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
PRJ=..

PRJNAME=xmlscript
TARGET=imexp
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CXXFILES= \
        imexp.cxx

OBJFILES= \
        $(OBJ)$/imexp.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)
APP1STDLIBS= \
        $(TOOLSLIB)			\
        $(SOTLIB)			\
        $(SVTOOLLIB)			\
        $(COMPHELPERLIB)		\
        $(VCLLIB)			\
        $(CPPULIB)			\
        $(CPPUHELPERLIB)		\
        $(SALLIB)			\
        $(XMLSCRIPTLIB)

APP1DEF=	$(MISC)$/imexp.def

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


# ------------------------------------------------------------------
# Windows
# ------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

$(MISC)$/imexp.def: makefile.mk
    echo  NAME			imexp			>$@
    echo  DESCRIPTION	'XML dialog im-/ exporter'     	>>$@
    echo  EXETYPE		WINDOWS 			>>$@
    echo  STUB		'winSTUB.EXE'           	>>$@
    echo  PROTMODE						>>$@
    echo  CODE		PRELOAD MOVEABLE DISCARDABLE	>>$@
    echo  DATA		PRELOAD MOVEABLE MULTIPLE	>>$@
    echo  HEAPSIZE		8192				>>$@
    echo  STACKSIZE 	32768				>>$@

.ENDIF
