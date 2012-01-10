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
PRJINC=..$/inc

PRJNAME=extensions
TARGET=pcr
USE_DEFFILE=TRUE

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=           $(SLO)$/controlfontdialog.obj \
                    $(SLO)$/fontdialog.obj	\
                    $(SLO)$/pcrservices.obj	\
                    $(SLO)$/selectlabeldialog.obj	\
                    $(SLO)$/propcontroller.obj	\
                    $(SLO)$/formcontroller.obj	\
                    $(SLO)$/modulepcr.obj  \
                    $(SLO)$/cellbindinghelper.obj   \
                    $(SLO)$/taborder.obj   \
                    $(SLO)$/pcrunodialogs.obj   \
                    $(SLO)$/unourl.obj   \
                    $(SLO)$/formlinkdialog.obj   \
                    $(SLO)$/listselectiondlg.obj   \
                    $(SLO)$/browserlistbox.obj	\
                    $(SLO)$/stringrepresentation.obj   \
                    $(SLO)$/eformshelper.obj   \
                    $(SLO)$/xsddatatypes.obj   \
                    $(SLO)$/xsdvalidationhelper.obj \
                    $(SLO)$/pushbuttonnavigation.obj \
                    $(SLO)$/sqlcommanddesign.obj \
                    $(SLO)$/composeduiupdate.obj \
                    $(SLO)$/propertycomposer.obj \
                    $(SLO)$/pcrcomponentcontext.obj \
                    $(SLO)$/commoncontrol.obj \
                    $(SLO)$/standardcontrol.obj \
                    $(SLO)$/usercontrol.obj \
                    $(SLO)$/browserline.obj \
                    $(SLO)$/genericpropertyhandler.obj \
                    $(SLO)$/buttonnavigationhandler.obj \
                    $(SLO)$/handlerhelper.obj \
                    $(SLO)$/propertyhandler.obj \
                    $(SLO)$/eventhandler.obj \
                    $(SLO)$/editpropertyhandler.obj \
                    $(SLO)$/eformspropertyhandler.obj \
                    $(SLO)$/cellbindinghandler.obj \
                    $(SLO)$/submissionhandler.obj \
                    $(SLO)$/xsdvalidationpropertyhandler.obj \
                    $(SLO)$/formcomponenthandler.obj \
                    $(SLO)$/formmetadata.obj \
                    $(SLO)$/defaultforminspection.obj \
                    $(SLO)$/formbrowsertools.obj \
                    $(SLO)$/browserpage.obj	\
                    $(SLO)$/propertyeditor.obj	\
                    $(SLO)$/formstrings.obj	\
                    $(SLO)$/pcrstrings.obj	\
                    $(SLO)$/browserview.obj	\
                    $(SLO)$/pcrcommon.obj	\
                    $(SLO)$/newdatatype.obj \
                    $(SLO)$/propeventtranslation.obj \
                    $(SLO)$/objectinspectormodel.obj \
                    $(SLO)$/inspectorhelpwindow.obj \
                    $(SLO)$/defaulthelpprovider.obj \
                    $(SLO)$/MasterDetailLinkDialog.obj \
                    $(SLO)$/inspectormodelbase.obj \
                    $(SLO)$/propertycontrolextender.obj \
                    $(SLO)$/formgeometryhandler.obj

SRS1NAME=$(TARGET)
SRC1FILES=			propres.src	 \
                    formres.src \
                    pcrmiscres.src \
                    taborder.src \
                    fontdialog.src \
                    selectlabeldialog.src \
                    formlinkdialog.src \
                    listselectiondlg.src \
                    newdatatype.src

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= $(SRS)$/$(TARGET).srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(EDITENGLIB)	\
        $(SVXCORELIB)	\
        $(SVXLIB)	\
        $(SFXLIB)	\
        $(SVTOOLLIB)\
        $(TKLIB)	\
        $(VCLLIB)	\
        $(SVLLIB)	\
        $(TOOLSLIB) \
        $(I18NISOLANGLIB)   \
        $(DBTOOLSLIB)	\
        $(COMPHELPERLIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(CPPULIB) 	\
        $(UNOTOOLSLIB) 	\
        $(SALLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


ALLTAR : $(MISC)/pcr.component

$(MISC)/pcr.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        pcr.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt pcr.component
