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
TARGET=xsltdlg

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
#GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	xmlfiltersettingsdialog.src							\
            xmlfiltertabdialog.src								\
            xmlfiltertabpagebasic.src							\
            xmlfiltertabpagexslt.src							\
            xmlfiltertestdialog.src								\
            xmlfileview.src										\
            xmlfilterdialogstrings.src

SLOFILES=	$(SLO)$/typedetectionimport.obj						\
            $(SLO)$/typedetectionexport.obj						\
            $(SLO)$/xmlfilterjar.obj							\
            $(SLO)$/xmlfilterdialogcomponent.obj				\
            $(SLO)$/xmlfiltersettingsdialog.obj					\
            $(SLO)$/xmlfiltertabdialog.obj						\
            $(SLO)$/xmlfiltertabpagebasic.obj					\
            $(SLO)$/xmlfiltertabpagexslt.obj					\
            $(SLO)$/xmlfiltertestdialog.obj						\
            $(SLO)$/xmlfileview.obj

# --- Library -----------------------------------

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES= $(SRS)$/$(TARGET).srs

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
    
SHL1STDLIBS=\
    $(SFX2LIB)			\
    $(SVTOOLLIB)		\
    $(SVLLIB)			\
    $(TKLIB)			\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(SALLIB)			\
    $(TOOLSLIB)			\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(ONELIB)

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/xsltdlg.component

$(MISC)/xsltdlg.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        xsltdlg.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xsltdlg.component
