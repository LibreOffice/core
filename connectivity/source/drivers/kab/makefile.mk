#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2006-01-10 14:31:04 $
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
TARGET=kab

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(GUI)" == "UNX"
.IF "$(ENABLE_KAB)" == "TRUE"
# --- Files -------------------------------------

EXCEPTIONSFILES=\
    $(SLO)$/KColumns.obj			\
    $(SLO)$/KTable.obj				\
    $(SLO)$/KTables.obj				\
    $(SLO)$/KCatalog.obj			\
    $(SLO)$/KResultSet.obj			\
    $(SLO)$/KStatement.obj			\
    $(SLO)$/KPreparedStatement.obj	\
    $(SLO)$/KDatabaseMetaData.obj	\
    $(SLO)$/KConnection.obj			\
    $(SLO)$/KServices.obj			\
    $(SLO)$/KResultSetMetaData.obj	\
    $(SLO)$/KDriver.obj				\
    $(SLO)$/kcondition.obj			\
    $(SLO)$/korder.obj				\
    $(SLO)$/kfields.obj				\
 
CFLAGS+=$(KDE_CFLAGS)

SLOFILES=\
    $(EXCEPTIONSFILES)

KAB_LIB=$(KDE_LIBS) -lkabc

SHL1VERSIONMAP=$(TARGET).map


# --- Library -----------------------------------

SHL1TARGET=	$(KAB_TARGET)$(KAB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SVLLIB)					\
    $(SVLIB)					\
    $(TOOLSLIB)					\
    $(SVTOOLLIB)				\
    $(SVTLIB)					\
    $(UNOTOOLSLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)			\
    $(KAB_LIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)


# --- Targets -----------------------------------
.ELSE		# "$(ENABLE_KAB)" == "TRUE"
dummy:
    @echo KDE Addressbook disabled - nothing to build
.ENDIF

.ELSE		# "$(GUI)" == "UNX"
dummy:
    @echo "Nothing to build for GUI $(GUI)"
.ENDIF

.INCLUDE : target.mk

