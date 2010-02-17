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

PRJ=.

PRJNAME=DUMP8
TARGET=dump8

# --- Settings ---------------------------------------------------

.INCLUDE :	settings.mk

CXXFILES=	dump8.cxx dump8a.cxx ww8dout.cxx ww8darr.cxx ww8scan.cxx msvbasic.cxx

OBJFILES=	$(OBJ)$/dump8.obj $(OBJ)$/dump8a.obj $(OBJ)$/ww8dout.obj \
               $(OBJ)$/ww8darr.obj $(OBJ)$/ww8scan.obj $(OBJ)$/msvbasic.obj


.IF "$(depend)" == ""

CFLAGS+= -DDUMP

# --- DUMP8 EXE --------------------------------------------------------

# IENV=$(IENV);..\RES

APP1TARGET=$(PRJNAME)

# RESLIB1NAME=vu
# RESLIB1SRSFILES=$(SRS)$/main.srs

APP1LIBS=$(LIB1TARGET)

.IF "$(GUI)" == "WNT"
APP1STDLIBS= $(SVTOOLLIB) $(SVLLIB) $(TOOLSLIB) $(SVLIB) $(SVMEMLIB) $(SO2LIB)
# irtl.lib
.IF "$(SO3)" == ""
APP1STDLIBS+= sdstor.lib
.ELSE
APP1STDLIBS+= $(SOTLIB)
.ENDIF
.IF "$(COMEX)"=="10"
APP1STDLIBS+= $(OLE32LIB) $(OLEAUT32LIB) $(UUIDLIB) $(SHELL32LIB) $(ADVAPI32LIB) libci.lib
.ELSE
APP1STDLIBS+= $(OLE32LIB) $(OLEAUT32LIB) $(UUIDLIB) $(SHELL32LIB) $(ADVAPI32LIB) libci.lib msvcirt.lib
.ENDIF
.ELSE
.IF "$(GUI)" == "OS2"
APP1STDLIBS=sdstor.lib $(SO2LIB) $(SVTOOLLIB) $(SVLIB) tools.lib $(SVMEMLIB) $(SVXLIB)
.ELSE
APP1STDLIBS=$(SVTOOLLIB) tools.lib $(SVLIB) $(SVMEMLIB) sdstor.lib $(SO2LIB) $(SVXLIB)
APP1STDLIBS+= ole2 compobj storage shell
.ENDIF
.ENDIF
.IF "$(GUI)"=="WNT"
APP1DEPN=	$(L)$/svtool.lib $(L)$/itools.lib $(SVLIBDEPEND) $(L)$/svmem.lib $(L)$/so2.lib
.ELSE
APP1DEPN=	$(L)$/svtool.lib $(L)$/tools.lib $(SVLIBDEPEND) $(L)$/svmem.lib $(L)$/so2.lib
.ENDIF
APP1OBJS=	$(OBJ)$/dump8.obj $(OBJ)$/dump8a.obj $(OBJ)$/ww8dout.obj \
               $(OBJ)$/ww8darr.obj $(OBJ)$/ww8scan.obj $(OBJ)/msvbasic.obj

APP1DEF=	$(MISC)$/$(PRJNAME).def


#-------------------------------------------------------------------------
#										OS/2
#-------------------------------------------------------------------------

.IF "$(GUI)" == "OS2"

ALL: \
    $(SRS)$/main.srs \
    ALLTAR

$(MISC)$/$(PRJNAME).def: makefile
    echo NAME		 VIEWER WINDOWAPI								   >$@
    echo DESCRIPTION 'Storage Viewer (C)1995 STAR DIVISION GmbH'      >>$@
    echo EXETYPE	 OS2											  >>$@
    echo PROTMODE													  >>$@
    echo STUB		 'OS2STUB.EXE'                                    >>$@
    echo CODE		 LOADONCALL 									  >>$@
    echo DATA		 PRELOAD MULTIPLE								  >>$@
    echo HEAPSIZE	 4096											  >>$@
    echo STACKSIZE	 30000											  >>$@

.ENDIF # GUI == OS2

#-------------------------------------------------------------------------
#								Windows NT
#-------------------------------------------------------------------------

# --- Targets -----------------------------------------------------------

.ENDIF

.INCLUDE :	target.mk

