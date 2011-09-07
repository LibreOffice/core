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

PRJNAME=salhelper
TARGET=salhelper

ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
USE_DEFFILE=TRUE

.IF "$(COM)" != "MSC"
UNIXVERSIONNAMES=UDK
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=	\
    $(SLO)$/condition.obj \
    $(SLO)$/dynload.obj \
    $(SLO)$/simplereferenceobject.obj \
    $(SLO)$/timer.obj

.IF "$(COM)" == "MSC"
SHL1TARGET=	$(TARGET)$(UDK_MAJOR)$(COMID)
SHL1IMPLIB=	i$(TARGET)
.ELSE
SHL1TARGET=	uno_$(TARGET)$(COMID)
SHL1IMPLIB=	uno_$(TARGET)$(COMID)
.ENDIF

SHL1STDLIBS=$(SALLIB)

SHL1DEPN=
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=  URELIB

DEF1NAME=	$(SHL1TARGET)

.IF "$(COMNAME)"=="msci"
SHL1VERSIONMAP=msci.map
.ELIF "$(COMNAME)"=="mscx"
SHL1VERSIONMAP=mscx.map
.ELIF "$(COMNAME)"=="sunpro5"
SHL1VERSIONMAP=sols.map
.ELIF "$(COMNAME)"=="gcc3"
SHL1VERSIONMAP=gcc3.map
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

