#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: obo $ $Date: 2008-03-26 12:37:38 $
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
.IF "$(GUI)"=="WNT"

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=ado

#	ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

# --- Files -------------------------------------
        
EXCEPTIONSFILES= \
        $(SLO)$/ADatabaseMetaDataImpl.obj				\
        $(SLO)$/Aolevariant.obj							\
        $(SLO)$/ADatabaseMetaData.obj					\
        $(SLO)$/AColumn.obj								\
        $(SLO)$/AColumns.obj							\
        $(SLO)$/AIndex.obj								\
        $(SLO)$/AIndexes.obj							\
        $(SLO)$/AKey.obj								\
        $(SLO)$/AKeys.obj								\
        $(SLO)$/AUser.obj								\
        $(SLO)$/AUsers.obj								\
        $(SLO)$/AGroup.obj								\
        $(SLO)$/AGroups.obj								\
        $(SLO)$/ACatalog.obj							\
        $(SLO)$/AView.obj								\
        $(SLO)$/AViews.obj								\
        $(SLO)$/ATable.obj								\
        $(SLO)$/ATables.obj								\
        $(SLO)$/ACallableStatement.obj					\
        $(SLO)$/ADatabaseMetaDataResultSetMetaData.obj	\
        $(SLO)$/ADatabaseMetaDataResultSet.obj			\
        $(SLO)$/AResultSet.obj							\
        $(SLO)$/AConnection.obj							\
        $(SLO)$/AStatement.obj							\
        $(SLO)$/APreparedStatement.obj					\
        $(SLO)$/AResultSetMetaData.obj					\
        $(SLO)$/ADriver.obj								\
        $(SLO)$/Aservices.obj

SLOFILES=\
        $(EXCEPTIONSFILES)								\
        $(SLO)$/Awrapado.obj							\
        $(SLO)$/adoimp.obj
# --- Library -----------------------------------

SHL1TARGET=	$(ADO_TARGET)$(ADO_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(OLE32LIB)					\
    $(OLEAUT32LIB)				\
    $(UUIDLIB)					\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)

.IF "$(COMPHELPERLIB)" == ""
SHL1STDLIBS+= icomphelp2.lib
.ENDIF

#SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk

.ENDIF # "$(GUI)"=="WNT"

