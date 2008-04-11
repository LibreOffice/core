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
# $Revision: 1.4 $
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

PRJNAME=accessibility
TARGET=standard

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

SLOFILES=   \
            $(SLO)$/accessiblemenubasecomponent.obj		\
            $(SLO)$/accessiblemenucomponent.obj			\
            $(SLO)$/accessiblemenuitemcomponent.obj		\
            $(SLO)$/vclxaccessiblebox.obj				\
            $(SLO)$/vclxaccessiblebutton.obj			\
            $(SLO)$/vclxaccessiblecheckbox.obj			\
            $(SLO)$/vclxaccessiblecombobox.obj			\
            $(SLO)$/vclxaccessibledropdowncombobox.obj	\
            $(SLO)$/vclxaccessibledropdownlistbox.obj	\
            $(SLO)$/vclxaccessibleedit.obj				\
            $(SLO)$/vclxaccessiblefixedhyperlink.obj    \
            $(SLO)$/vclxaccessiblefixedtext.obj         \
            $(SLO)$/vclxaccessiblelist.obj				\
            $(SLO)$/vclxaccessiblelistbox.obj			\
            $(SLO)$/vclxaccessiblelistitem.obj			\
            $(SLO)$/vclxaccessiblemenu.obj				\
            $(SLO)$/vclxaccessiblemenubar.obj			\
            $(SLO)$/vclxaccessiblemenuitem.obj			\
            $(SLO)$/vclxaccessiblemenuseparator.obj		\
            $(SLO)$/vclxaccessiblepopupmenu.obj			\
            $(SLO)$/vclxaccessibleradiobutton.obj		\
            $(SLO)$/vclxaccessiblescrollbar.obj			\
            $(SLO)$/vclxaccessiblestatusbar.obj			\
            $(SLO)$/vclxaccessiblestatusbaritem.obj		\
            $(SLO)$/vclxaccessibletabcontrol.obj		\
            $(SLO)$/vclxaccessibletabpage.obj			\
            $(SLO)$/vclxaccessibletabpagewindow.obj		\
            $(SLO)$/vclxaccessibletextcomponent.obj		\
            $(SLO)$/vclxaccessibletextfield.obj			\
            $(SLO)$/vclxaccessibletoolbox.obj			\
            $(SLO)$/vclxaccessibletoolboxitem.obj       \

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

