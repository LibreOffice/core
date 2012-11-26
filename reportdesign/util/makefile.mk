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
PRJNAME=reportdesign
TARGET=rpt
TARGET2=$(TARGET)ui
# USE_DEFFILE=TRUE
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings ----------------------------------
.INCLUDE :  settings.mk

.IF "$(L10N_framework)"==""

# ------------------------------------------------------------------
# --- reportdesign core (rpt) -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
        $(SLB)$/api.lib				\
        $(SLB)$/coreshared.lib		\
        $(SLB)$/core_resource.lib	\
        $(SLB)$/core_sdr.lib        \
        $(SLB)$/core_misc.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
    $(EDITENGLIB)			\
        $(SVXCORELIB)			\
        $(BASEGFXLIB)			\
        $(SVXLIB)				\
        $(FWELIB)				\
        $(SFXLIB)				\
        $(TOOLSLIB) 			\
        $(I18NISOLANGLIB) \
        $(SVLLIB)				\
        $(SVTOOLLIB)			\
        $(UNOTOOLSLIB)			\
        $(DBTOOLSLIB)			\
        $(COMPHELPERLIB) 		\
        $(CPPUHELPERLIB) 		\
        $(CPPULIB)				\
        $(VCLLIB)				\
        $(TKLIB)				\
        $(VOSLIB)				\
        $(SALLIB)

.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC" || "$(GUI)"=="OS2"
SHL1STDLIBS+= \
        -ldbu$(DLLPOSTFIX)
.ELSE
SHL1STDLIBS+= \
        idbu.lib
.ENDIF

SHL1IMPLIB=i$(TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)
.ENDIF
# --- .res file ----------------------------------------------------------

RES1FILELIST=\
    $(SRS)$/core_strings.srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)

.IF "$(L10N_framework)"==""
# --- reportdesign UI core (rptui) -----------------------------------
LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=\
        $(SLB)$/uimisc.lib			\
        $(SLB)$/uidlg.lib			\
        $(SLB)$/uishared.lib		\
        $(SLB)$/ui_inspection.lib	\
        $(SLB)$/report.lib

SHL2TARGET=$(TARGET2)$(DLLPOSTFIX)

SHL2STDLIBS= \
    $(EDITENGLIB)			\
        $(BASEGFXLIB)			\
        $(SVXCORELIB)			\
        $(SVXLIB)				\
        $(SFXLIB)				\
        $(SVTOOLLIB)			\
        $(TKLIB)				\
        $(VCLLIB)				\
        $(SVLLIB)				\
        $(SOTLIB)				\
        $(UNOTOOLSLIB)			\
        $(TOOLSLIB)				\
        $(DBTOOLSLIB)			\
        $(COMPHELPERLIB)		\
        $(CPPUHELPERLIB)		\
        $(CPPULIB)				\
        $(FWELIB)				\
        $(SO2LIB)				\
        $(I18NISOLANGLIB)		\
        $(SALLIB)
.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC" || "$(GUI)"=="OS2"
SHL2STDLIBS+= \
        -ldbu$(DLLPOSTFIX) \
        -l$(TARGET)$(DLLPOSTFIX)
.ELSE
SHL2STDLIBS+= \
        idbu.lib				\
        i$(TARGET).lib
.ENDIF

.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC" || "$(GUI)"=="OS2"
SHL2STDLIBS+= \
        -lfor$(DLLPOSTFIX) \
        -lforui$(DLLPOSTFIX)
.ELSE
SHL2STDLIBS+= \
        ifor.lib \
        iforui.lib
.ENDIF


SHL2DEPN=$(SHL1TARGETN)
SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
SHL2VERSIONMAP=$(SOLARENV)/src/component.map
.ENDIF
# --- .res file ----------------------------------------------------------
RES2FILELIST=\
    $(SRS)$/uidlg.srs				\
    $(SRS)$/ui_inspection.srs		\
    $(SRS)$/report.srs

RESLIB2NAME=$(TARGET2)
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES=$(RES2FILELIST)
.IF "$(L10N_framework)"==""

# ------------------- rptxml -------------------
TARGET3=rptxml
# --- Library -----------------------------------
LIB3TARGET=$(SLB)$/rpt_flt.lib
LIB3FILES=\
        $(SLB)$/xmlshared.lib	\
        $(SLB)$/$(TARGET3).lib

SHL3TARGET=$(TARGET3)$(DLLPOSTFIX)

SHL3STDLIBS=\
    $(EDITENGLIB)			\
    $(SVXCORELIB)			\
    $(XMLOFFLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(DBTOOLSLIB)		\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(UCBHELPERLIB)		\
    $(SFXLIB)			\
    $(SVLLIB)			\
    $(SOTLIB)			\
    $(SO2LIB)			\
    $(SALLIB)
.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC" || "$(GUI)"=="OS2"
SHL3STDLIBS+= \
        -l$(TARGET)$(DLLPOSTFIX)
.ELSE
SHL3STDLIBS+= \
    irpt.lib
.ENDIF


SHL3DEPN=$(SHL1TARGETN)
SHL3LIBS=$(LIB3TARGET)
SHL3IMPLIB=	i$(SHL3TARGET)
SHL3VERSIONMAP=$(SOLARENV)/src/component.map
SHL3DEF=	$(MISC)$/$(SHL3TARGET).def

DEF3NAME=$(SHL3TARGET)

.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk



ALLTAR : $(MISC)/rpt.component $(MISC)/rptui.component $(MISC)/rptxml.component

$(MISC)/rpt.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        rpt.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt rpt.component

$(MISC)/rptui.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        rptui.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt rptui.component

$(MISC)/rptxml.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        rptxml.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL3TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt rptxml.component
