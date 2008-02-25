#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:48:15 $
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

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = deploymentgui
ENABLE_EXCEPTIONS = TRUE
#USE_DEFFILE = TRUE
NO_BSYMBOLIC = TRUE
USE_PCH :=
ENABLE_PCH :=

.IF "$(GUI)"=="OS2"
TARGET = deplgui
.ENDIF

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk
DLLPRE =

SLOFILES = \
        $(SLO)$/dp_gui_service.obj \
        $(SLO)$/dp_gui_dialog.obj \
        $(SLO)$/dp_gui_treelb.obj \
        $(SLO)$/dp_gui_cmdenv.obj \
    $(SLO)$/license_dialog.obj \
        $(SLO)$/dp_gui_dependencydialog.obj \
        $(SLO)$/dp_gui_thread.obj \
        $(SLO)$/dp_gui_updatability.obj \
        $(SLO)$/dp_gui_updatedialog.obj \
        $(SLO)$/dp_gui_updateinstalldialog.obj \
        $(SLO)$/dp_gui_autoscrolledit.obj \
        $(SLO)$/dp_gui_system.obj \
        $(SLO)$/dp_gui_addextensionqueue.obj \
        $(SLO)$/descedit.obj

SHL1TARGET = $(TARGET)$(DLLPOSTFIX).uno
SHL1VERSIONMAP = ..$/deployment.map

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
        $(SFXLIB) \
        $(DEPLOYMENTMISCLIB)

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def

DEF1NAME = $(SHL1TARGET)
#DEFLIB1NAME = $(TARGET)
#DEF1DEPN =

SRS1NAME = $(TARGET)
SRC1FILES = \
        dp_gui_dialog.src \
        dp_gui_backend.src \
        dp_gui_dependencydialog.src \
        dp_gui_updatedialog.src \
        dp_gui_versionboxes.src \
        dp_gui_updateinstalldialog.src

RESLIB1NAME = $(TARGET)
RESLIB1SRSFILES = $(SRS)$/$(TARGET).srs
RESLIB1IMAGES=	$(PRJ)$/res

.INCLUDE : target.mk

