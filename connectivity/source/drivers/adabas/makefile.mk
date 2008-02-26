#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.27 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 15:25:55 $
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
TARGET=adabas

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(SYSTEM_ODBC_HEADERS)" == "YES"
CFLAGS+=-DSYSTEM_ODBC_HEADERS
.ENDIF

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/BFunctions.obj					\
        $(SLO)$/BConnection.obj					\
        $(SLO)$/BDriver.obj						\
        $(SLO)$/BCatalog.obj					\
        $(SLO)$/BGroups.obj						\
        $(SLO)$/BGroup.obj						\
        $(SLO)$/BUser.obj						\
        $(SLO)$/BUsers.obj						\
        $(SLO)$/BKeyColumns.obj					\
        $(SLO)$/BKey.obj						\
        $(SLO)$/BKeys.obj						\
        $(SLO)$/BColumns.obj					\
        $(SLO)$/BIndex.obj						\
        $(SLO)$/BIndexColumns.obj				\
        $(SLO)$/BIndexes.obj					\
        $(SLO)$/BTable.obj						\
        $(SLO)$/BTables.obj						\
        $(SLO)$/BViews.obj						\
        $(SLO)$/Bservices.obj					\
        $(SLO)$/BDatabaseMetaData.obj			\
        $(SLO)$/BPreparedStatement.obj          \
        $(SLO)$/BStatement.obj                  \
        $(SLO)$/BResultSetMetaData.obj			\
        $(SLO)$/BResultSet.obj

SHL1VERSIONMAP=$(TARGET).map

# --- Library -----------------------------------

SHL1TARGET=	$(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(TOOLSLIB)					\
    $(ODBCBASELIB)				\
    $(UCBHELPERLIB) 			\
    $(UNOTOOLSLIB)				\
    $(COMPHELPERLIB)

.IF "$(ODBCBASELIB)" == ""
SHL1STDLIBS+=$(ODBCBASELIB)
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ----------------------------------

.INCLUDE : target.mk


