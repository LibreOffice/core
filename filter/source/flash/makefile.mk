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
PRJNAME=filter
TARGET=flash

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES =	impswfdialog.src				

SLOFILES=	$(SLO)$/swffilter.obj								\
            $(SLO)$/swfwriter.obj								\
            $(SLO)$/swfwriter1.obj								\
            $(SLO)$/swfwriter2.obj								\
            $(SLO)$/swfuno.obj									\
            $(SLO)$/swfexporter.obj							\
            $(SLO)$/swfdialog.obj							\
            $(SLO)$/impswfdialog.obj

# --- Library -----------------------------------

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES= $(SRS)$/$(TARGET).srs

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

# static libraries must come at the end for MACOSX
.IF "$(OS)" != "MACOSX"
SHL1STDLIBS+=\
    $(ZLIB3RDLIB)
.ENDIF


# dynamic libraries
SHL1STDLIBS+=\
    $(SVTOOLLIB)		\
    $(BASEGFXLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB) 

# static libraries must come at the end for MACOSX
.IF "$(OS)" == "MACOSX"
SHL1STDLIBS+=\
    $(ZLIB3RDLIB)
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/flash.component

$(MISC)/flash.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        flash.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt flash.component
