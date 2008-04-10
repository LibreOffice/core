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
# $Revision: 1.23 $
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


