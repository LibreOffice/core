#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
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
#***********************************************************************/

PRJ = ..
PRJNAME = binaryurp
TARGET = qa

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

DLLPRE =

SLOFILES = $(SLO)/test-cache.obj $(SLO)/test-unmarshal.obj

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLO)/test-cache.obj
SHL1RPATH = NONE
SHL1STDLIBS = $(CPPUNITLIB) $(SALLIB)
SHL1TARGET = test-cache
SHL1VERSIONMAP = version.map
DEF1NAME = $(SHL1TARGET)

SHL2IMPLIB = i$(SHL2TARGET)
SHL2OBJS = \
    $(SLO)/test-unmarshal.obj \
    $(SLO)/binaryany.obj \
    $(SLO)/bridge.obj \
    $(SLO)/bridgefactory.obj \
    $(SLO)/currentcontext.obj \
    $(SLO)/incomingrequest.obj \
    $(SLO)/lessoperators.obj \
    $(SLO)/marshal.obj \
    $(SLO)/outgoingrequests.obj \
    $(SLO)/proxy.obj \
    $(SLO)/reader.obj \
    $(SLO)/unmarshal.obj \
    $(SLO)/writer.obj
SHL2RPATH = NONE
SHL2STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(CPPUNITLIB) \
    $(SALHELPERLIB) \
    $(SALLIB)
SHL2TARGET = test-unmarshal
SHL2VERSIONMAP = version.map
DEF2NAME = $(SHL2TARGET)

.INCLUDE: target.mk
.INCLUDE: _cppunit.mk
