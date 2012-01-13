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
PRJNAME=avmedia
TARGET=avmediawin

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(ENABLE_DIRECTX)" != ""

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

.IF "$(GUI)" == "WNT"

SLOFILES= \
        $(SLO)$/winuno.obj      \
        $(SLO)$/manager.obj     \
        $(SLO)$/window.obj      \
        $(SLO)$/player.obj      \
        $(SLO)$/framegrabber.obj    


EXCEPTIONSFILES= \
        $(SLO)$/winuno.obj      \
        $(SLO)$/framegrabber.obj    

SHL1TARGET=$(TARGET)
SHL1STDLIBS= $(CPPULIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(UNOTOOLSLIB) $(TOOLSLIB) $(VCLLIB) 
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

SHL1STDLIBS += $(OLE32LIB)
SHL1STDLIBS += $(OLEAUT32LIB)
SHL1STDLIBS += $(GDI32LIB)

.IF "$(COM)"=="GCC"
SHL1STDLIBS += $(PSDK_HOME)$/lib$/strmiids.lib
.ELSE
SHL1STDLIBS += strmiids.lib
.ENDIF

.ENDIF
.ENDIF

.INCLUDE :  	target.mk

ALLTAR : $(MISC)/avmediawin.component

$(MISC)/avmediawin.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        avmediawin.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmediawin.component
