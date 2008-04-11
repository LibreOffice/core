#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.16 $
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

PRJNAME=			framework
TARGET=				fwk_uielement
USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  		settings.mk

# --- Generate -----------------------------------------------------

SLOFILES=			\
                    $(SLO)$/menubarwrapper.obj              \
                    $(SLO)$/menubarmanager.obj              \
                    $(SLO)$/fontmenucontroller.obj          \
                    $(SLO)$/fontsizemenucontroller.obj      \
                    $(SLO)$/objectmenucontroller.obj        \
                    $(SLO)$/headermenucontroller.obj        \
                    $(SLO)$/footermenucontroller.obj        \
                    $(SLO)$/controlmenucontroller.obj       \
                    $(SLO)$/macrosmenucontroller.obj   		\
                    $(SLO)$/uicommanddescription.obj        \
                    $(SLO)$/itemcontainer.obj               \
                    $(SLO)$/constitemcontainer.obj          \
                    $(SLO)$/rootitemcontainer.obj           \
                    $(SLO)$/toolbarwrapper.obj              \
                    $(SLO)$/toolbar.obj                     \
                    $(SLO)$/toolbarmanager.obj              \
                    $(SLO)$/addonstoolbarwrapper.obj        \
                    $(SLO)$/addonstoolbarmanager.obj        \
                    $(SLO)$/generictoolbarcontroller.obj    \
                    $(SLO)$/toolbarsmenucontroller.obj      \
                    $(SLO)$/statusbarwrapper.obj            \
                    $(SLO)$/statusbar.obj                   \
                    $(SLO)$/statusbarmanager.obj			\
                    $(SLO)$/recentfilesmenucontroller.obj   \
                    $(SLO)$/progressbarwrapper.obj          \
                    $(SLO)$/statusindicatorinterfacewrapper.obj \
                    $(SLO)$/logoimagestatusbarcontroller.obj \
                    $(SLO)$/logotextstatusbarcontroller.obj \
                    $(SLO)$/newmenucontroller.obj           \
                    $(SLO)$/simpletextstatusbarcontroller.obj\
                    $(SLO)$/complextoolbarcontroller.obj     \
                    $(SLO)$/comboboxtoolbarcontroller.obj    \
                    $(SLO)$/imagebuttontoolbarcontroller.obj \
                    $(SLO)$/togglebuttontoolbarcontroller.obj \
                    $(SLO)$/buttontoolbarcontroller.obj       \
                    $(SLO)$/spinfieldtoolbarcontroller.obj    \
                    $(SLO)$/edittoolbarcontroller.obj         \
                    $(SLO)$/dropdownboxtoolbarcontroller.obj \
                    $(SLO)$/menubarmerger.obj \
                    $(SLO)$/toolbarmerger.obj \
                    $(SLO)$/langselectionstatusbarcontroller.obj \
                    $(SLO)$/langselectionmenucontroller.obj

# --- Targets ------------------------------------------------------

.INCLUDE :			target.mk
