#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.29 $
#
#   last change: $Author: ihi $ $Date: 2006-12-20 13:52:19 $
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

PRJNAME=toolkit
TARGET=awt

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=   \
            $(SLO)$/accessiblemenubasecomponent.obj		\
            $(SLO)$/accessiblemenucomponent.obj			\
            $(SLO)$/accessiblemenuitemcomponent.obj		\
            $(SLO)$/vclxaccessiblebox.obj				\
            $(SLO)$/vclxaccessiblebutton.obj			\
            $(SLO)$/vclxaccessiblecheckbox.obj			\
            $(SLO)$/vclxaccessiblecombobox.obj			\
            $(SLO)$/vclxaccessiblecomponent.obj 		\
            $(SLO)$/vclxaccessibledropdowncombobox.obj	\
            $(SLO)$/vclxaccessibledropdownlistbox.obj	\
            $(SLO)$/vclxaccessibleedit.obj				\
            $(SLO)$/vclxaccessiblefixedtext.obj			\
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
            $(SLO)$/vclxaccessibletoolboxitem.obj		\
            $(SLO)$/vclxbitmap.obj 						\
            $(SLO)$/vclxcontainer.obj 					\
            $(SLO)$/vclxdevice.obj 						\
            $(SLO)$/vclxfont.obj 						\
            $(SLO)$/vclxgraphics.obj 					\
            $(SLO)$/vclxmenu.obj 						\
            $(SLO)$/vclxpointer.obj 					\
            $(SLO)$/vclxprinter.obj 					\
            $(SLO)$/vclxregion.obj 						\
            $(SLO)$/vclxsystemdependentwindow.obj		\
            $(SLO)$/vclxtoolkit.obj 					\
            $(SLO)$/vclxtopwindow.obj 					\
            $(SLO)$/vclxwindow.obj 						\
            $(SLO)$/vclxwindow1.obj 					\
            $(SLO)$/vclxwindows.obj                     \
            $(SLO)$/vclxspinbutton.obj                  \
            $(SLO)$/xsimpleanimation.obj                \
            $(SLO)$/xthrobber.obj

SRS1NAME=$(TARGET)
SRC1FILES=\
            accessiblestrings.src        \
            xthrobber.src

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

