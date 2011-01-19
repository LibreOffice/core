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

PRJNAME=unoxml
TARGET=domimpl

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =	\
    $(SLO)$/attr.obj \
    $(SLO)$/cdatasection.obj \
    $(SLO)$/characterdata.obj \
    $(SLO)$/comment.obj \
    $(SLO)$/document.obj \
    $(SLO)$/documentbuilder.obj \
    $(SLO)$/documentfragment.obj \
    $(SLO)$/documenttype.obj \
    $(SLO)$/element.obj \
    $(SLO)$/entity.obj \
    $(SLO)$/entityreference.obj \
    $(SLO)$/node.obj \
    $(SLO)$/notation.obj \
    $(SLO)$/processinginstruction.obj \
    $(SLO)$/text.obj \
    $(SLO)$/domimplementation.obj \
    $(SLO)$/elementlist.obj \
    $(SLO)$/childlist.obj \
    $(SLO)$/notationsmap.obj \
    $(SLO)$/entitiesmap.obj \
    $(SLO)$/attributesmap.obj \
    $(SLO)$/saxbuilder.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


