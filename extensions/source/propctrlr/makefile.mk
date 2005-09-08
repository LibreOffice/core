#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 20:18:16 $
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

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=pcr
CDEFS+=-DCOMPMOD_RESPREFIX=pcr

.IF $(DVO_XFORMS) != ""
CDEFS+=-DDVO_XFORMS
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=    $(SLO)$/controlfontdialog.obj	\
                    $(SLO)$/fontdialog.obj	\
                    $(SLO)$/pcrservices.obj	\
                    $(SLO)$/selectlabeldialog.obj	\
                    $(SLO)$/formcontroller.obj	\
                    $(SLO)$/propcontroller.obj	\
                    $(SLO)$/modulepcr.obj   \
                    $(SLO)$/cellbindinghelper.obj   \
                    $(SLO)$/taborder.obj   \
                    $(SLO)$/pcrunodialogs.obj   \
                    $(SLO)$/unourl.obj   \
                    $(SLO)$/formlinkdialog.obj   \
                    $(SLO)$/listselectiondlg.obj   \
                    $(SLO)$/propertyhandler.obj   \
                    $(SLO)$/cellbindinghandler.obj   \
                    $(SLO)$/stringrepresentation.obj   \
                    $(SLO)$/editpropertyhandler.obj   \
                    $(SLO)$/eformspropertyhandler.obj   \
                    $(SLO)$/eformshelper.obj   \
                    $(SLO)$/xsddatatypes.obj   \
                    $(SLO)$/xsdvalidationhelper.obj \
                    $(SLO)$/pushbuttonnavigation.obj \
                    $(SLO)$/submissionhandler.obj

SLOFILES=			$(EXCEPTIONSFILES) \
                    $(SLO)$/browserline.obj	\
                    $(SLO)$/formmetadata.obj	\
                    $(SLO)$/formbrowsertools.obj	\
                    $(SLO)$/standardcontrol.obj	\
                    $(SLO)$/usercontrol.obj	\
                    $(SLO)$/commoncontrol.obj	\
                    $(SLO)$/browserpage.obj	\
                    $(SLO)$/browserlistbox.obj	\
                    $(SLO)$/propertyeditor.obj	\
                    $(SLO)$/formstrings.obj	\
                    $(SLO)$/pcrstrings.obj	\
                    $(SLO)$/browserview.obj	\
                    $(SLO)$/xsdvalidationpropertyhandler.obj \
                    $(SLO)$/pcrcommon.obj	\
                    $(SLO)$/newdatatype.obj \
                    $(SLO)$/buttonnavigationhandler.obj \
                    $(SLO)$/propertycomposer.obj \


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

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1VERSIONMAP= $(TARGET).map

SHL1STDLIBS= \
        $(SVXLIB)	\
        $(SFXLIB)	\
        $(SVTOOLLIB)\
        $(TKLIB)	\
        $(VCLLIB)	\
        $(SVLLIB)	\
        $(TOOLSLIB) \
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

