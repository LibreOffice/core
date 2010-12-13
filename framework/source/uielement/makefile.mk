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

PRJNAME=            framework
TARGET=             fwk_uielement
USE_DEFFILE=        TRUE
ENABLE_EXCEPTIONS=  TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :          settings.mk

# --- Generate -----------------------------------------------------

SLOFILES=           \
                    $(SLO)$/addonstoolbarmanager.obj            \
                    $(SLO)$/addonstoolbarwrapper.obj            \
                    $(SLO)$/buttontoolbarcontroller.obj         \
                    $(SLO)$/comboboxtoolbarcontroller.obj       \
                    $(SLO)$/complextoolbarcontroller.obj        \
                    $(SLO)$/constitemcontainer.obj              \
                    $(SLO)$/controlmenucontroller.obj           \
                    $(SLO)$/dropdownboxtoolbarcontroller.obj    \
                    $(SLO)$/edittoolbarcontroller.obj           \
                    $(SLO)$/fontmenucontroller.obj              \
                    $(SLO)$/fontsizemenucontroller.obj          \
                    $(SLO)$/footermenucontroller.obj            \
                    $(SLO)$/generictoolbarcontroller.obj        \
                    $(SLO)$/headermenucontroller.obj            \
                    $(SLO)$/imagebuttontoolbarcontroller.obj    \
                    $(SLO)$/itemcontainer.obj                   \
                    $(SLO)$/langselectionmenucontroller.obj     \
                    $(SLO)$/langselectionstatusbarcontroller.obj \
                    $(SLO)$/logoimagestatusbarcontroller.obj    \
                    $(SLO)$/logotextstatusbarcontroller.obj     \
                    $(SLO)$/macrosmenucontroller.obj            \
                    $(SLO)$/menubarmanager.obj                  \
                    $(SLO)$/menubarmerger.obj                   \
                    $(SLO)$/menubarwrapper.obj                  \
                    $(SLO)$/newmenucontroller.obj               \
                    $(SLO)$/objectmenucontroller.obj            \
                    $(SLO)$/progressbarwrapper.obj              \
                    $(SLO)$/recentfilesmenucontroller.obj       \
                    $(SLO)$/rootitemcontainer.obj               \
                    $(SLO)$/simpletextstatusbarcontroller.obj   \
                    $(SLO)$/spinfieldtoolbarcontroller.obj      \
                    $(SLO)$/statusbar.obj                       \
                    $(SLO)$/statusbarmanager.obj                \
                    $(SLO)$/statusbarwrapper.obj                \
                    $(SLO)$/statusindicatorinterfacewrapper.obj \
                    $(SLO)$/togglebuttontoolbarcontroller.obj   \
                    $(SLO)$/toolbar.obj                         \
                    $(SLO)$/toolbarmanager.obj                  \
                    $(SLO)$/toolbarmerger.obj                   \
                    $(SLO)$/toolbarsmenucontroller.obj          \
                    $(SLO)$/toolbarwrapper.obj                  \
                    $(SLO)$/popupmenucontroller.obj				\
                    $(SLO)$/uicommanddescription.obj            \
                    $(SLO)$/panelwrapper.obj					\
                    $(SLO)$/panelwindow.obj

# --- Targets ------------------------------------------------------

.INCLUDE :          target.mk
