#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2008-01-29 15:27:12 $
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

