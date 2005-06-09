#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: hr $ $Date: 2005-06-09 14:57:11 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

