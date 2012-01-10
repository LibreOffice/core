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
TARGET=fps_office
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE
GEN_HID=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE=

# --- Files --------------------------------------------------------

SLOFILES=\
    $(SLO)$/asyncfilepicker.obj \
    $(SLO)$/commonpicker.obj \
    $(SLO)$/OfficeControlAccess.obj \
    $(SLO)$/OfficeFilePicker.obj \
    $(SLO)$/OfficeFolderPicker.obj \
    $(SLO)$/fpinteraction.obj \
    $(SLO)$/fpsmartcontent.obj \
    $(SLO)$/fps_office.obj \
    $(SLO)$/iodlg.obj \
    $(SLO)$/iodlgimp.obj

SHL1TARGET=	$(TARGET).uno
SHL1IMPLIB=	i$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1STDLIBS=\
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)

SRS1NAME=	$(TARGET)
SRC1FILES=	\
    OfficeFilePicker.src \
    iodlg.src

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=\
    $(SRS)$/fps_office.srs

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/fps_office.component

$(MISC)/fps_office.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fps_office.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fps_office.component
