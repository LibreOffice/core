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

PRJNAME=	sal
TARGET=		unloading
TARGET1=samplelib1
TARGET2=samplelib2
TARGETTYPE=CUI
COMP1TYPELIST=$(TARGET1)

NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE

USE_DEFFILE=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

CPPUMAKERFLAGS =

.IF "$(BOOTSTRAP_SERVICE)" == "TRUE"
UNOUCROUT=	$(OUT)$/inc$/bootstrap
INCPRE+=	$(OUT)$/inc$/bootstrap
.ELSE
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF
UNOUCROUT=	$(OUT)$/inc$/light
INCPRE+=	$(OUT)$/inc$/light
.ENDIF


LIB1TARGET=$(SLB)$/$(TARGET1).lib
LIB1OBJFILES= \
        $(SLO)$/samplelib1.obj

SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1DEPN=
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1EXPORTFILE=	exports.dxp
DEF1NAME=	$(SHL1TARGET)

#-------------------------------------------------------

LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2OBJFILES= \
        $(SLO)$/samplelib2.obj

SHL2TARGET=	$(TARGET2)

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2DEPN=
SHL2LIBS=	$(SLB)$/$(TARGET2).lib
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2EXPORTFILE=	exports.dxp
DEF2NAME=	$(SHL2TARGET)

# ------------------------------------------------------------------

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/unloadTest.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

