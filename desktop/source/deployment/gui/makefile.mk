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

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = deploymentgui
ENABLE_EXCEPTIONS = TRUE
NO_BSYMBOLIC = TRUE
USE_PCH :=
ENABLE_PCH :=
PRJINC:=..$/..

.IF "$(GUI)"=="OS2"
TARGET = deplgui
.ENDIF

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk
DLLPRE =

SLOFILES = \
        $(SLO)$/dp_gui_service.obj \
        $(SLO)$/dp_gui_extlistbox.obj \
        $(SLO)$/dp_gui_dialog2.obj \
        $(SLO)$/dp_gui_theextmgr.obj \
        $(SLO)$/license_dialog.obj \
        $(SLO)$/dp_gui_dependencydialog.obj \
        $(SLO)$/dp_gui_thread.obj \
        $(SLO)$/dp_gui_updatedialog.obj \
        $(SLO)$/dp_gui_updateinstalldialog.obj \
        $(SLO)$/dp_gui_autoscrolledit.obj \
        $(SLO)$/dp_gui_extensioncmdqueue.obj \
        $(SLO)$/descedit.obj

SHL1TARGET = $(TARGET)$(DLLPOSTFIX).uno
SHL1VERSIONMAP = $(SOLARENV)/src/component.map

SHL1STDLIBS = \
        $(SALLIB) \
        $(SALHELPERLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(UCBHELPERLIB) \
        $(COMPHELPERLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(I18NISOLANGLIB) \
        $(TKLIB) \
        $(VCLLIB) \
        $(SVTOOLLIB)	\
        $(SVLLIB)  \
        $(SVXLIB) \
        $(SVXCORELIB) \
        $(SFXLIB) \
        $(DEPLOYMENTMISCLIB) \
        $(OLE32LIB)

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def

DEF1NAME = $(SHL1TARGET)

SRS1NAME = $(TARGET)
SRC1FILES = \
        dp_gui_dialog.src \
        dp_gui_dialog2.src \
        dp_gui_backend.src \
        dp_gui_dependencydialog.src \
        dp_gui_updatedialog.src \
        dp_gui_versionboxes.src \
        dp_gui_updateinstalldialog.src

RESLIB1NAME = $(TARGET)
RESLIB1SRSFILES = $(SRS)$/$(TARGET).srs
RESLIB1IMAGES=	$(PRJ)$/res

.INCLUDE : target.mk


ALLTAR : $(MISC)/deploymentgui.component

$(MISC)/deploymentgui.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt deploymentgui.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt deploymentgui.component
