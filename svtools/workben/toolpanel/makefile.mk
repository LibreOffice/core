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
# $Revision: 1.16 $
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

PRJNAME=svtools
TARGET=toolpaneltest
TARGETTYPE=GUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CXXFILES=   toolpaneltest.cxx

OBJFILES=   $(OBJ)$/toolpaneltest.obj

APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJFILES)
APP1STDLIBS=\
            $(VCLLIB)           \
            $(UCBHELPERLIB)     \
            $(SALLIB)           \
            $(TOOLSLIB)         \
            $(COMPHELPERLIB)    \
            $(CPPUHELPERLIB)    \
            $(CPPULIB)          \
            $(BASEGFXLIB)       \
            $(SVTOOLLIB)        \

APP1RAPTH=BRAND

.IF "$(GUI)"!="UNX"
APP1DEF=    $(MISC)$/$(TARGET).def
.ENDIF

.IF "$(COM)"=="GCC"
ADDOPTFILES=$(OBJ)$/toolpaneltest.obj
add_cflagscxx="-frtti -fexceptions"
.ENDIF


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


# ------------------------------------------------------------------
# MAC
# ------------------------------------------------------------------

.IF "$(GUI)" == "MAC"

$(MISC)$/$(TARGET).def: makefile
    echo Kein Def-File fuer Applikationen auf Mac
.ENDIF


# ------------------------------------------------------------------
# Windows
# ------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

$(MISC)$/$(TARGET).def: makefile
    echo  NAME          $(TARGET)                           >$@
    echo  DESCRIPTION   'ToolPanel - Testprogramm'         >>$@
    echo  EXETYPE       WINDOWS                            >>$@
    echo  STUB          'winSTUB.EXE'                      >>$@
    echo  PROTMODE                                         >>$@
    echo  CODE          PRELOAD MOVEABLE DISCARDABLE       >>$@
    echo  DATA          PRELOAD MOVEABLE MULTIPLE          >>$@
    echo  HEAPSIZE      8192                               >>$@
    echo  STACKSIZE     32768                              >>$@

.ENDIF

