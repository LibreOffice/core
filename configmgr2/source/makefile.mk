#*************************************************************************
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile,v $
#
# $Revision: 1.4 $
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
#***********************************************************************/

PRJ = ..
PRJNAME = configmgr
TARGET = configmgr

ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

CDEFS += -DOOO_DLLIMPLEMENTATION_CONFIGMGR

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS += $(LIBXML_CFLAGS)
.ENDIF

SLOFILES = \
    $(SLO)$/access.obj \
    $(SLO)$/childaccess.obj \
    $(SLO)$/components.obj \
    $(SLO)$/configurationprovider.obj \
    $(SLO)$/data.obj \
    $(SLO)$/defaultprovider.obj \
    $(SLO)$/groupnode.obj \
    $(SLO)$/localizedpropertynode.obj \
    $(SLO)$/localizedvaluenode.obj \
    $(SLO)$/lock.obj \
    $(SLO)$/node.obj \
    $(SLO)$/nodemap.obj \
    $(SLO)$/pad.obj \
    $(SLO)$/propertynode.obj \
    $(SLO)$/rootaccess.obj \
    $(SLO)$/services.obj \
    $(SLO)$/setnode.obj \
    $(SLO)$/type.obj \
    $(SLO)$/update.obj \
    $(SLO)$/xml.obj \
    $(SLO)$/xmlreader.obj

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(LIBXML2LIB) \
    $(SALHELPERLIB) \
    $(SALLIB)
SHL1TARGET = configmgr
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
