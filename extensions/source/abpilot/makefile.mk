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
TARGET=abp
USE_DEFFILE=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=abp
CDEFS+=-DCOMPMOD_RESPREFIX=abp

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=	\
        $(SLO)$/fieldmappingimpl.obj	\
        $(SLO)$/datasourcehandling.obj	\
        $(SLO)$/admininvokationimpl.obj	\
        $(SLO)$/unodialogabp.obj	\
        $(SLO)$/moduleabp.obj	\
        $(SLO)$/abpservices.obj \
        $(SLO)$/typeselectionpage.obj	\

SLOFILES=	\
        $(SLO)$/fieldmappingpage.obj	\
        $(SLO)$/abpfinalpage.obj	\
        $(SLO)$/tableselectionpage.obj	\
        $(SLO)$/admininvokationpage.obj	\
        $(SLO)$/abspage.obj	\
        $(SLO)$/abspilot.obj \
        $(EXCEPTIONSFILES)


.IF "$(WITH_MOZILLA)" != "NO"
.IF "$(SYSTEM_MOZILLA)" != "YES"
CDEFS+=-DWITH_MOZILLA
.ENDIF
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=			abspilot.src

RESLIB1NAME=abp
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= $(SRS)$/abp.srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(VCLLIB)			\
        $(SALLIB)			\
        $(TOOLSLIB)			\
        $(CPPULIB) 			\
        $(SVTOOLLIB)		\
        $(SVLLIB)			\
        $(SFXLIB)			\
        $(CPPUHELPERLIB)	\
        $(COMPHELPERLIB)	\
        $(TKLIB)			\
        $(UNOTOOLSLIB)		\
        $(SVXCORELIB)		\
        $(SVXLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/abp.component

$(MISC)/abp.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        abp.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt abp.component
