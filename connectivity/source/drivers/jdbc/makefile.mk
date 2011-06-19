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

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=jdbc

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : $(PRJ)$/makefile.pmk
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

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Library -----------------------------------

SHL1TARGET=	$(JDBC_TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SALLIB)					\
    $(SALHELPERLIB)					\
    $(JVMACCESSLIB)				\
    $(DBTOOLSLIB)				\
    $(UNOTOOLSLIB)				\
    $(JVMFWKLIB)				\
    $(COMPHELPERLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(JDBC_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF # SOLAR_JAVA

# --- Targets ----------------------------------

.INCLUDE : $(PRJ)$/target.pmk



ALLTAR : $(MISC)/jdbc.component

$(MISC)/jdbc.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        jdbc.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt jdbc.component
