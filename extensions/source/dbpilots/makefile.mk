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


PRJ=..$/..
PRJINC=..$/inc

PRJNAME=extensions
TARGET=dbp
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=dbp

# --- Files --------------------------------------------------------

CXXFILES=			dbptools.cxx	\
                    gridwizard.cxx	\
                    listcombowizard.cxx	\
                    optiongrouplayouter.cxx	\
                    commonpagesdbp.cxx	\
                    groupboxwiz.cxx	\
                    wizardservices.cxx	\
                    controlwizard.cxx	\
                    moduledbp.cxx	\
                    dbpservices.cxx

SLOFILES=			$(SLO)$/dbptools.obj	\
                    $(SLO)$/gridwizard.obj	\
                    $(SLO)$/listcombowizard.obj	\
                    $(SLO)$/optiongrouplayouter.obj	\
                    $(SLO)$/commonpagesdbp.obj	\
                    $(SLO)$/groupboxwiz.obj	\
                    $(SLO)$/wizardservices.obj	\
                    $(SLO)$/controlwizard.obj	\
                    $(SLO)$/moduledbp.obj	\
                    $(SLO)$/dbpservices.obj


SRS1NAME=$(TARGET)
SRC1FILES=			gridpages.src	\
                    commonpagesdbp.src	\
                    dbpilots.src	\
                    groupboxpages.src	\
                    listcombopages.src

RESLIB1NAME=dbp
RESLIB1SRSFILES= $(SRS)$/dbp.srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(SVTOOLLIB) \
        $(VCLLIB) \
        $(TOOLSLIB) \
        $(SVLLIB) \
        $(SFXLIB)		\
        $(DBTOOLSLIB) \
                $(UNOTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/dbp.component

$(MISC)/dbp.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dbp.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dbp.component
