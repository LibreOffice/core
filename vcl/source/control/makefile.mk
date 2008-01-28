#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: vg $ $Date: 2008-01-28 14:16:35 $
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

PRJNAME=vcl
TARGET=ctrl

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

.IF "$(COM)"=="ICC"
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/button.obj		\
            $(SLO)$/ctrl.obj		\
            $(SLO)$/combobox.obj	\
            $(SLO)$/edit.obj		\
            $(SLO)$/field.obj		\
            $(SLO)$/field2.obj		\
            $(SLO)$/fixbrd.obj		\
            $(SLO)$/fixed.obj		\
            $(SLO)$/group.obj		\
            $(SLO)$/ilstbox.obj 	\
            $(SLO)$/imgctrl.obj 	\
            $(SLO)$/longcurr.obj	\
            $(SLO)$/lstbox.obj		\
            $(SLO)$/morebtn.obj 	\
            $(SLO)$/menubtn.obj 	\
            $(SLO)$/scrbar.obj		\
            $(SLO)$/slider.obj		\
            $(SLO)$/spinfld.obj 	\
            $(SLO)$/spinbtn.obj 	\
            $(SLO)$/tabctrl.obj

EXCEPTIONSFILES=					\
            $(SLO)$/button.obj		\
            $(SLO)$/ctrl.obj		\
            $(SLO)$/edit.obj		\
            $(SLO)$/field2.obj		\
            $(SLO)$/ilstbox.obj		\
            $(SLO)$/tabctrl.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.INCLUDE :	$(PRJ)$/util$/target.pmk
