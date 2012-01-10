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

PRJNAME=fileaccess
TARGET=fileacc
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
COMP1TYPELIST=$(TARGET)
COMPRDB=$(SOLARBINDIR)$/types.rdb

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
# ------------------------------------------------------------------

SLOFILES= \
    $(SLO)$/FileAccess.obj 
#	$(SLO)$/$(COMP1TYPELIST)_description.obj

SHL1TARGET= $(TARGET)
SHL1IMPLIB=	i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(UCBHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

SHL1DEPN=
SHL1LIBS=$(SLB)$/$(TARGET).lib

# --- Targets ------------------------------------------------------
.ENDIF # L10N_framework

.INCLUDE :	target.mk


ALLTAR : $(MISC)/fileacc.component

$(MISC)/fileacc.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fileacc.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fileacc.component
