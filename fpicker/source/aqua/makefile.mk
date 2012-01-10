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

PRJNAME=fpicker
TARGET=fps_aqua.uno
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

DLLPRE=

# ------------------------------------------------------------------

.IF "$(GUIBASE)" != "aqua"

dummy:
    @echo "Nothing to build. GUIBASE == $(GUIBASE)"

.ELSE # we build for aqua
CFLAGSCXX+=$(OBJCXXFLAGS)

# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/resourceprovider.obj       \
        $(SLO)$/FPentry.obj                \
        $(SLO)$/SalAquaPicker.obj          \
        $(SLO)$/SalAquaFilePicker.obj      \
        $(SLO)$/SalAquaFolderPicker.obj    \
        $(SLO)$/CFStringUtilities.obj      \
        $(SLO)$/FilterHelper.obj           \
        $(SLO)$/ControlHelper.obj          \
        $(SLO)$/NSString_OOoAdditions.obj  \
        $(SLO)$/NSURL_OOoAdditions.obj     \
        $(SLO)$/AquaFilePickerDelegate.obj

SHL1NOCHECK=TRUE
SHL1TARGET= $(TARGET)
SHL1OBJS=   $(SLOFILES)
SHL1STDLIBS=\
    $(VCLLIB) \
    $(TOOLSLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF # "$(GUIBASE)" != "aqua"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/fps_aqua.component

$(MISC)/fps_aqua.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fps_aqua.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fps_aqua.component
