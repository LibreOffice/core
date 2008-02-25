#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:06:40 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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


