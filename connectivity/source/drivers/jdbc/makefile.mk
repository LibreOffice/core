#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    $(VOSLIB)					\
    $(SALLIB)					\
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
