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
TARGET=preload
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=preload
CDEFS+=-DCOMPMOD_RESPREFIX=preload

# --- Files --------------------------------------------------------

SLOFILES=           $(SLO)$/services.obj\
                    $(SLO)$/modulepreload.obj\
                    $(SLO)$/preloadservices.obj\
                    $(SLO)$/oemwiz.obj

SRS1NAME=$(TARGET)
SRC1FILES=           preload.src

RESLIB1NAME=preload
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= $(SRS)$/preload.srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
.IF "$(GUI)" == "OS2"
# YD preload is not a valid library name...
SHL1TARGET= $(TARGET)g
.ENDIF
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(SFXLIB)   \
        $(SVTOOLLIB)\
        $(VCLLIB)    \
        $(SVLLIB)	\
        $(TOOLSLIB)  \
        $(UNOTOOLSLIB)                     \
        $(COMPHELPERLIB)     \
        $(CPPUHELPERLIB) 	 \
        $(CPPULIB)   \
        $(SALLIB)


SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


ALLTAR : $(MISC)/preload.component

$(MISC)/preload.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        preload.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt preload.component
