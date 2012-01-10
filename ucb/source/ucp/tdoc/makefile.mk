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
PRJNAME=ucb
# Version
UCPTDOC_MAJOR=1
TARGET=ucptdoc$(UCPTDOC_MAJOR).uno
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------
.IF "$(L10N_framework)"==""

# no "lib" prefix
DLLPRE =

SLOFILES=\
    $(SLO)$/tdoc_provider.obj	\
    $(SLO)$/tdoc_services.obj	\
    $(SLO)$/tdoc_uri.obj		\
    $(SLO)$/tdoc_content.obj	\
    $(SLO)$/tdoc_contentcaps.obj \
    $(SLO)$/tdoc_storage.obj	\
    $(SLO)$/tdoc_docmgr.obj		\
    $(SLO)$/tdoc_datasupplier.obj \
    $(SLO)$/tdoc_resultset.obj  \
    $(SLO)$/tdoc_documentcontentfactory.obj \
    $(SLO)$/tdoc_passwordrequest.obj \
    $(SLO)$/tdoc_stgelems.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS=\
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# Make symbol renaming match library name for Mac OS X
.IF "$(OS)"=="MACOSX"
SYMBOLPREFIX=$(TARGET)
.ENDIF

DEF1NAME=$(SHL1TARGET)

.ENDIF # L10N_framework

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk


ALLTAR : $(MISC)/ucptdoc1.component

$(MISC)/ucptdoc1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucptdoc1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucptdoc1.component
