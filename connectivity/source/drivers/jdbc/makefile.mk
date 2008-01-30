#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: rt $ $Date: 2008-01-30 07:55:28 $
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
TARGET=jdbc

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/version.mk


.IF "$(SOLAR_JAVA)" != ""

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/Array.obj							\
        $(SLO)$/Blob.obj							\
        $(SLO)$/Boolean.obj							\
        $(SLO)$/CallableStatement.obj				\
        $(SLO)$/Class.obj							\
        $(SLO)$/Clob.obj							\
        $(SLO)$/ConnectionLog.obj                   \
        $(SLO)$/DatabaseMetaData.obj				\
        $(SLO)$/Date.obj							\
        $(SLO)$/DriverManager.obj					\
        $(SLO)$/DriverPropertyInfo.obj				\
        $(SLO)$/Exception.obj						\
        $(SLO)$/InputStream.obj						\
        $(SLO)$/JConnection.obj                     \
        $(SLO)$/JDriver.obj							\
        $(SLO)$/JStatement.obj                      \
        $(SLO)$/Object.obj							\
        $(SLO)$/PreparedStatement.obj				\
        $(SLO)$/Reader.obj							\
        $(SLO)$/Ref.obj								\
        $(SLO)$/ResultSet.obj						\
        $(SLO)$/ResultSetMetaData.obj				\
        $(SLO)$/SQLException.obj					\
        $(SLO)$/SQLWarning.obj						\
        $(SLO)$/String.obj							\
        $(SLO)$/Throwable.obj						\
        $(SLO)$/Timestamp.obj						\
        $(SLO)$/jservices.obj						\
        $(SLO)$/JBigDecimal.obj						\
        $(SLO)$/tools.obj                           \
        $(SLO)$/ContextClassLoader.obj

SHL1VERSIONMAP=$(JDBC_TARGET).map

# --- Library -----------------------------------

SHL1TARGET=	$(JDBC_TARGET)$(JDBC_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(JVMACCESSLIB)				\
    $(DBTOOLSLIB)				\
    $(JVMFWKLIB)				\
    $(COMPHELPERLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(JDBC_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF # SOLAR_JAVA

# --- Targets ----------------------------------

.INCLUDE : target.mk


