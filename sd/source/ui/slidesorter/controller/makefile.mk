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
# $Revision: 1.9 $
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

PRJ=..$/..$/..$/..

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=slscontroller
ENABLE_EXCEPTIONS=TRUE
AUTOSEG=true
PRJINC=..$/..

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =      							\
    $(SLO)$/SlideSorterController.obj		\
    $(SLO)$/SlsAnimator.obj	    			\
    $(SLO)$/SlsClipboard.obj	    		\
    $(SLO)$/SlsCurrentSlideManager.obj	    \
    $(SLO)$/SlsFocusManager.obj				\
    $(SLO)$/SlsListener.obj					\
    $(SLO)$/SlsPageObjectFactory.obj		\
    $(SLO)$/SlsPageSelector.obj				\
    $(SLO)$/SlsProperties.obj				\
    $(SLO)$/SlsScrollBarManager.obj			\
    $(SLO)$/SlsSelectionCommand.obj	    	\
    $(SLO)$/SlsSelectionManager.obj	    	\
    $(SLO)$/SlsSlotManager.obj				\
    $(SLO)$/SlsTransferable.obj				\
                                            \
    $(SLO)$/SlsHideSlideFunction.obj		\
    $(SLO)$/SlsSelectionFunction.obj		\
    $(SLO)$/SlsSlideFunction.obj		

EXCEPTIONSFILES=							\
    $(SLO)$/SlideSorterController.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

