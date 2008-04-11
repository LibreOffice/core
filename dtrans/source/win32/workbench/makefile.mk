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
# $Revision: 1.11 $
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

PRJ=..$/..$/..

PRJNAME=		dtrans
TARGET=			testwincb
TARGET1=		testmshl
LIBTARGET=		NO
TARGETTYPE=		CUI
USE_BOUNDCHK=
TESTCB=TRUE

.IF "$(USE_BOUNDCHK)"=="TR"
bndchk=tr
stoponerror=tr
.ENDIF

# --- Settings -----------------------------------------------------
#.INCLUDE :		$(PRJ)$/util$/makefile.pmk

.INCLUDE :  settings.mk

.IF "$(TESTCB)"=="TRUE"

CFLAGS+=-D_WIN32_DCOM -EHsc -Ob0

# --- Files --------------------------------------------------------

OBJFILES=   	$(OBJ)$/test_wincb.obj
APP1TARGET=		$(TARGET)
APP1OBJS=		$(OBJ)$/test_wincb.obj
                
                
APP1STDLIBS=	$(SALLIB) \
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SOLARLIBDIR)$/uwinapi.lib\
                $(USER32LIB) \
                $(OLE32LIB)\
                $(COMDLG32LIB)

APP1LIBS= $(SLB)$/dtutils.lib
          

APP1NOSAL=  TRUE

.ENDIF

.IF "$(TESTCB)"==""

CFLAGS+=/D_WIN32_DCOM /EHsc /Ob0

OBJFILES=   	$(OBJ)$/testmarshal.obj
APP1TARGET=		$(TARGET1)
APP1OBJS=		$(OBJ)$/testmarshal.obj
                
APP1STDLIBS=	$(SALLIB)\
                $(USER32LIB)\
                $(OLE32LIB)\
                comsupp.lib\
                $(OLEAUT32LIB)

APP1LIBS= 

APP1NOSAL=  TRUE

.ENDIF

# --- Targets ------------------------------------------------------
.INCLUDE :		target.mk
#.INCLUDE :		$(PRJ)$/util$/target.pmk
