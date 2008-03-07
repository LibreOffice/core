#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 11:20:26 $
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

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=uicontrols

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        TableGrantCtrl.src	\
        undosqledit.src		\
        tabletree.src		

# ... exception files .........................

EXCEPTIONSFILES=\
        $(SLO)$/statusbarontroller.obj	\
        $(SLO)$/RelationControl.obj		\
        $(SLO)$/toolboxcontroller.obj	\
        $(SLO)$/tabletree.obj			\
        $(SLO)$/TableGrantCtrl.obj		\
        $(SLO)$/dbtreelistbox.obj       \
        $(SLO)$/sqledit.obj				\
        $(SLO)$/ColumnControlWindow.obj	\
        $(SLO)$/FieldDescControl.obj    \
        $(SLO)$/opendoccontrols.obj

# ... object files ............................

SLOFILES=	\
        $(EXCEPTIONSFILES)				\
        $(SLO)$/ScrollHelper.obj		\
        $(SLO)$/VertSplitView.obj		\
        $(SLO)$/SqlNameEdit.obj			\
        $(SLO)$/listviewitems.obj		\
        $(SLO)$/undosqledit.obj			\
        $(SLO)$/marktree.obj			\
        $(SLO)$/curledit.obj            \
        $(SLO)$/charsetlistbox.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

