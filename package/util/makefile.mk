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



# 2 == Unicode
MAJOR_VERSION=2

PRJ=..
PRJNAME=package
TARGET=package

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(L10N_framework)"==""

# --- General ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	\
    $(SLB)$/zipapi.lib \
    $(SLB)$/zippackage.lib \
    $(SLB)$/manifest.lib

# --- Shared-Library -----------------------------------------------

SHL1TARGET=$(TARGET)$(MAJOR_VERSION)
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=$(SOLARENV)$/src$/component.map

SHL1STDLIBS=\
    $(CPPULIB)		\
    $(UCBHELPERLIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)		\
    $(SALLIB)		\
    $(ZLIB3RDLIB)	\
    $(SAXLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
DEF1NAME=$(SHL1TARGET)

.ENDIF # L10N_framework

# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk


ALLTAR : $(MISC)/package2.component

$(MISC)/package2.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        package2.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt package2.component
