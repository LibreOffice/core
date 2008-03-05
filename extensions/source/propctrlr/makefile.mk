#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 17:11:42 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************
PRJ=..$/..
PRJINC=..$/inc

PRJNAME=extensions
TARGET=pcr
USE_DEFFILE=TRUE

ENABLE_EXCEPTIONS=TRUE

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
                    $(SLO)$/inspectormodelbase.obj

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
SHL1VERSIONMAP= $(TARGET).map

SHL1STDLIBS= \
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

