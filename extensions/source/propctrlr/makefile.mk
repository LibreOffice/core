#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
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
