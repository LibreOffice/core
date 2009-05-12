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

PRJ=..

PRJNAME=embeddedobj
TARGET=embobj
#LIBTARGET=NO
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

LIB1OBJFILES= \
        $(SLO)$/miscobj.obj\
        $(SLO)$/specialobject.obj\
        $(SLO)$/persistence.obj\
        $(SLO)$/embedobj.obj\
        $(SLO)$/inplaceobj.obj\
        $(SLO)$/visobj.obj\
        $(SLO)$/dummyobject.obj\
        $(SLO)$/xcreator.obj\
        $(SLO)$/xfactory.obj\
        $(SLO)$/register.obj\
        $(SLO)$/docholder.obj\
        $(SLO)$/intercept.obj

LIB1TARGET=$(SLB)$/$(TARGET).lib

SHL1TARGET= $(TARGET)

SHL1STDLIBS=\
    $(SALLIB)\
    $(CPPULIB)\
    $(COMPHELPERLIB)\
    $(CPPUHELPERLIB)

SHL1DEPN=
SHL1IMPLIB= i$(TARGET)

SHL1LIBS=	$(LIB1TARGET)

SHL1DEF= $(MISC)$/$(SHL1TARGET).def

DEF1NAME= $(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(LIB1TARGET)

DEFLIB1NAME=$(TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo __CT				>>$@

