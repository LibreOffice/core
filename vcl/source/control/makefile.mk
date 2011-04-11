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

EXCEPTIONSFILES=					\
            $(SLO)$/button.obj		\
            $(SLO)$/ctrl.obj		\
            $(SLO)$/edit.obj		\
            $(SLO)$/field2.obj		\
            $(SLO)$/ilstbox.obj		\
            $(SLO)$/tabctrl.obj		\
            $(SLO)$/throbber.obj

SLOFILES=	$(EXCEPTIONSFILES)      \
            $(SLO)$/combobox.obj	\
            $(SLO)$/field.obj		\
            $(SLO)$/fixbrd.obj		\
            $(SLO)$/fixed.obj		\
            $(SLO)$/group.obj		\
            $(SLO)$/imgctrl.obj 	\
            $(SLO)$/longcurr.obj	\
            $(SLO)$/lstbox.obj		\
            $(SLO)$/morebtn.obj 	\
            $(SLO)$/menubtn.obj 	\
            $(SLO)$/scrbar.obj		\
            $(SLO)$/slider.obj		\
            $(SLO)$/spinfld.obj 	\
            $(SLO)$/spinbtn.obj 	\
            $(SLO)$/quickselectionengine.obj


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.INCLUDE :	$(PRJ)$/util$/target.pmk

