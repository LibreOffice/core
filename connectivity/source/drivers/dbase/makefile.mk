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
# $Revision: 1.28 $
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
TARGET=dbase

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/DCode.obj				\
    $(SLO)$/DResultSet.obj			\
    $(SLO)$/DStatement.obj			\
    $(SLO)$/DPreparedStatement.obj	\
    $(SLO)$/dindexnode.obj			\
    $(SLO)$/DIndexIter.obj			\
    $(SLO)$/DDatabaseMetaData.obj	\
    $(SLO)$/DCatalog.obj			\
    $(SLO)$/DColumns.obj			\
    $(SLO)$/DIndexColumns.obj		\
    $(SLO)$/DIndex.obj				\
    $(SLO)$/DIndexes.obj			\
    $(SLO)$/DTable.obj				\
    $(SLO)$/DTables.obj				\
    $(SLO)$/DConnection.obj			\
    $(SLO)$/Dservices.obj			\
    $(SLO)$/DDriver.obj

EXCEPTIONSFILES=\
    $(SLO)$/DCode.obj				\
    $(SLO)$/DResultSet.obj			\
    $(SLO)$/DStatement.obj			\
    $(SLO)$/DPreparedStatement.obj	\
    $(SLO)$/dindexnode.obj			\
    $(SLO)$/DIndexIter.obj			\
    $(SLO)$/DDatabaseMetaData.obj	\
    $(SLO)$/DCatalog.obj			\
    $(SLO)$/DColumns.obj			\
    $(SLO)$/DIndexColumns.obj		\
    $(SLO)$/DIndex.obj				\
    $(SLO)$/DIndexes.obj			\
    $(SLO)$/DTables.obj				\
    $(SLO)$/DConnection.obj			\
    $(SLO)$/Dservices.obj			\
    $(SLO)$/DDriver.obj

# [kh] ppc linux gcc compiler problem
.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCPOWERPC"
EXCEPTIONSNOOPTFILES= \
    $(SLO)$/DTable.obj
.ELSE
EXCEPTIONSFILES +=\
            $(SLO)$/DTable.obj
.ENDIF


SHL1VERSIONMAP=$(TARGET).map

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SVLLIB)					\
    $(TOOLSLIB)					\
    $(SVTOOLLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(UNOTOOLSLIB)				\
    $(COMPHELPERLIB)

.IF "$(DBFILELIB)" == ""
SHL1STDLIBS+= ifile.lib
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(DBASE_TARGET)


SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ----------------------------------

.INCLUDE : target.mk


