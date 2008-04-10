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
# $Revision: 1.20 $
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
PRJINC=..$/..
PRJNAME=connectivity
TARGET=flat

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk


# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/EResultSet.obj			\
    $(SLO)$/EStatement.obj			\
    $(SLO)$/EPreparedStatement.obj	\
    $(SLO)$/ETable.obj				\
    $(SLO)$/EDatabaseMetaData.obj	\
    $(SLO)$/ECatalog.obj			\
    $(SLO)$/EColumns.obj			\
    $(SLO)$/ETables.obj				\
    $(SLO)$/EConnection.obj			\
    $(SLO)$/Eservices.obj			\
    $(SLO)$/EDriver.obj

EXCEPTIONSFILES=\
    $(SLO)$/EResultSet.obj			\
    $(SLO)$/EStatement.obj			\
    $(SLO)$/EPreparedStatement.obj	\
    $(SLO)$/ETable.obj				\
    $(SLO)$/EDatabaseMetaData.obj	\
    $(SLO)$/ECatalog.obj			\
    $(SLO)$/EColumns.obj			\
    $(SLO)$/ETables.obj				\
    $(SLO)$/EConnection.obj			\
    $(SLO)$/Eservices.obj			\
    $(SLO)$/EDriver.obj


SHL1VERSIONMAP=$(TARGET).map

# --- Library -----------------------------------
SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SVLLIB)					\
    $(VCLLIB)					\
    $(TOOLSLIB)					\
    $(I18NISOLANGLIB)			\
    $(SVTOOLLIB)				\
    $(UNOTOOLSLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(COMPHELPERLIB)

.IF "$(DBFILELIB)" == ""
SHL1STDLIBS+= ifile.lib
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ----------------------------------

.INCLUDE : target.mk


