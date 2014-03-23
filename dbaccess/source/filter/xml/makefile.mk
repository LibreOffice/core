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
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=dbaxml

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

SLOFILES=	$(SLO)$/xmlfilter.obj				\
            $(SLO)$/dbloader2.obj				\
            $(SLO)$/xmlDatabase.obj				\
            $(SLO)$/xmlDataSource.obj			\
            $(SLO)$/xmlTableFilterPattern.obj	\
            $(SLO)$/xmlTableFilterList.obj		\
            $(SLO)$/xmlDataSourceInfo.obj		\
            $(SLO)$/xmlDataSourceSetting.obj	\
            $(SLO)$/xmlDataSourceSettings.obj	\
            $(SLO)$/xmlDocuments.obj			\
            $(SLO)$/xmlComponent.obj			\
            $(SLO)$/xmlHierarchyCollection.obj	\
            $(SLO)$/xmlLogin.obj				\
            $(SLO)$/xmlExport.obj				\
            $(SLO)$/xmlQuery.obj				\
            $(SLO)$/xmlTable.obj				\
            $(SLO)$/xmlColumn.obj				\
            $(SLO)$/xmlStyleImport.obj			\
            $(SLO)$/xmlHelper.obj				\
            $(SLO)$/xmlAutoStyle.obj			\
            $(SLO)$/xmlConnectionData.obj		\
            $(SLO)$/xmlDatabaseDescription.obj	\
            $(SLO)$/xmlFileBasedDatabase.obj	\
            $(SLO)$/xmlServerDatabase.obj		\
            $(SLO)$/xmlConnectionResource.obj	\
            $(SLO)$/xmlservices.obj

# --- Library -----------------------------------
LIB1TARGET=$(SLB)$/dba_flt.lib
LIB1FILES=\
        $(SLB)$/fltshared.lib	\
        $(SLB)$/$(TARGET).lib
        
SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS=\
    $(XMLOFFLIB)		\
    $(VCLLIB)			\
    $(TOOLSLIB)			\
    $(TKLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SFXLIB)			\
    $(SVLLIB)			\
    $(DBTOOLSLIB)		\
    $(UNOTOOLSLIB)		\
    $(SO2LIB)			\
    $(SALLIB)

.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC" || "$(GUI)"=="OS2"
SHL1STDLIBS+= \
        -ldba$(DLLPOSTFIX)
.ELSE
SHL1STDLIBS+= \
        idba.lib
.ENDIF


SHL1DEPN=
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=	i$(SHL1TARGET)
# SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/dbaxml.component

$(MISC)/dbaxml.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dbaxml.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dbaxml.component
