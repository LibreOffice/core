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



PRJ=..

PRJNAME=cppcanvas
TARGET=cppcanvas
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
    $(SLB)$/canvaswrapper.lib	\
    $(SLB)$/metafilerenderer.lib	\
    $(SLB)$/cppcanvastools.lib

SHL1TARGET= 	$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= 	i$(TARGET)
SHL1STDLIBS=	$(TOOLSLIB) $(CPPULIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(CANVASTOOLSLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(I18NISOLANGLIB)

.IF "$(debug)$(dbgutil)"!=""
SHL1STDLIBS += $(CPPUHELPERLIB)
.ENDIF # "$(debug)$(dbgutil)"!=""

SHL1LIBS=		$(SLB)$/$(TARGET).lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=CPPCanvas
DEFLIB1NAME	=$(TARGET)

# ==========================================================================

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk
    @$(TYPE) $(TARGET).flt > $@
