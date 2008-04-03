#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 14:31:49 $
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

