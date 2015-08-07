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
TARGET=dbase

VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : $(PRJ)$/makefile.pmk
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
# [Don Lewis] clang 3.4 -O2 on amd64 tries to use R_X86_64_PC32 relocation with
#             symbol _ZThn192_N12connectivity4file10OFileTable7acquireEvj
.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCPOWERPC" || ("$(OS)$(COM)$(CPUNAME)"=="FREEBSDCLANGX86_64" && $(CCNUMVER)>=000300040000 && $(CCNUMVER)<000300050000)
EXCEPTIONSNOOPTFILES= \
    $(SLO)$/DTable.obj
.ELSE
EXCEPTIONSFILES +=\
            $(SLO)$/DTable.obj
.ENDIF


SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SVLLIB)					\
    $(TOOLSLIB)					\
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

.INCLUDE : $(PRJ)$/target.pmk



ALLTAR : $(MISC)/dbase.component

$(MISC)/dbase.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dbase.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dbase.component
