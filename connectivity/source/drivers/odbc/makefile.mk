#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 06:36:23 $
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
TARGET=odbcbase
TARGET2=odbc

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LDUMP=ldump2.exe

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
        $(SLO)$/OPreparedStatement.obj			\
        $(SLO)$/OStatement.obj					\
        $(SLO)$/OResultSetMetaData.obj			\
        $(SLO)$/OResultSet.obj					\
        $(SLO)$/OTools.obj						\
        $(SLO)$/ODatabaseMetaDataResultSet.obj	\
        $(SLO)$/ODatabaseMetaData.obj			\
        $(SLO)$/ODriver.obj						\
        $(SLO)$/OConnection.obj

# --- ODBC BASE Library -----------------------------------

SHL1TARGET=	$(ODBC2_TARGET)$(ODBC2_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)			\
    $(CPPUHELPERLIB)			\
    $(CPPULIB)					\
    $(VOSLIB)					\
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(ODBC2_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(SLB)$/$(TARGET).lib
DEFLIB1NAME=$(TARGET)

# --- ODBC Library -----------------------------------
# --- Files -------------------------------------

SLO2FILES=\
        $(SLO)$/oservices.obj	\
        $(SLO)$/ORealDriver.obj	\
        $(SLO)$/OFunctions.obj

# --- ODBC Library -----------------------------------

SHL2TARGET=	$(ODBC_TARGET)$(ODBC_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(ODBCBASELIB)				\
    $(CPPUHELPERLIB)			\
    $(CPPULIB)					\
    $(SALLIB)

.IF "$(ODBCBASELIB)" == ""
SHL2STDLIBS+= iodbcbase.lib
.ENDIF

SHL2DEPN=$(SHL1TARGETN)
SHL2IMPLIB=	i$(ODBC_TARGET)

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
SHL2VERSIONMAP=odbc.map

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- filter file ------------------------------

.IF "$(depend)"==""

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo _TI				>>$@
    @echo _real				>>$@
.ENDIF

