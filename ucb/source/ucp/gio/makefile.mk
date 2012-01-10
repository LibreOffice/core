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
UCPGIO_MAJOR=1
TARGET=ucpgio
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

.IF "$(ENABLE_GIO)"!=""
COMPILER_WARN_ALL=TRUE
PKGCONFIG_MODULES=gio-2.0
.INCLUDE: pkg_config.mk

.IF "$(OS)" == "SOLARIS"
LINKFLAGS+=-z nodefs
.ENDIF          # "$(OS)" == "SOLARIS"

# no "lib" prefix
DLLPRE =

SLOFILES=$(SLO)$/gio_provider.obj\
         $(SLO)$/gio_content.obj\
         $(SLO)$/gio_resultset.obj\
         $(SLO)$/gio_datasupplier.obj\
         $(SLO)$/gio_seekable.obj\
         $(SLO)$/gio_inputstream.obj\
         $(SLO)$/gio_outputstream.obj\
         $(SLO)$/gio_mount.obj

SHL1TARGET=$(TARGET)$(UCPGIO_MAJOR).uno
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB)

SHL1STDLIBS+=$(PKGCONFIG_LIBS)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

.ENDIF          # "$(ENABLE_GIO)"!=""
.ENDIF # L10N_framework

.INCLUDE: target.mk

ALLTAR : $(MISC)/ucpgio.component

$(MISC)/ucpgio.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpgio.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpgio.component
