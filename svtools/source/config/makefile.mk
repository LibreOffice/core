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
# $Revision: 1.61 $
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

PRJNAME=svtools
TARGET=config
TARGET1=heavyconfig
LIBTARGET=NO

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svl.pmk

# --- Files --------------------------------------------------------

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

# --- config.lib Files --------------------------------------------------------
LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1OBJFILES=  \
    $(SLO)$/accelcfg.obj				\
    $(SLO)$/fltrcfg.obj				\
    $(SLO)$/xmlaccelcfg.obj				\
    $(SLO)$/moduleoptions.obj           \
    $(SLO)$/pathoptions.obj             \
    $(SLO)$/saveopt.obj					\
    $(SLO)$/lingucfg.obj                \
    $(SLO)$/javaoptions.obj             \
    $(SLO)$/securityoptions.obj			\
    $(SLO)$/localisationoptions.obj		\
    $(SLO)$/workingsetoptions.obj		\
    $(SLO)$/viewoptions.obj				\
    $(SLO)$/internaloptions.obj			\
    $(SLO)$/startoptions.obj			\
    $(SLO)$/historyoptions.obj			\
    $(SLO)$/inetoptions.obj				\
    $(SLO)$/menuoptions.obj				\
    $(SLO)$/dynamicmenuoptions.obj		\
    $(SLO)$/optionsdrawinglayer.obj		\
    $(SLO)$/fontoptions.obj				\
    $(SLO)$/addxmltostorageoptions.obj	\
    $(SLO)$/defaultoptions.obj			\
    $(SLO)$/searchopt.obj				\
    $(SLO)$/printwarningoptions.obj		\
    $(SLO)$/cacheoptions.obj            \
    $(SLO)$/regoptions.obj				\
    $(SLO)$/cmdoptions.obj				\
    $(SLO)$/extendedsecurityoptions.obj \
    $(SLO)$/sourceviewconfig.obj		\
    $(SLO)$/compatibility.obj           \
    $(SLO)$/eventcfg.obj                \
    $(SLO)$/optionsdlg.obj              \
    $(SLO)$/itemholder1.obj \
    $(SLO)$/options.obj

#	$(SLO)$/miscopt.obj					\
# --- heavyconfig.lib Files --------------------------------------------------------
LIB2TARGET= $(SLB)$/$(TARGET1).lib

LIB2OBJFILES=  \
    $(SLO)$/accessibilityoptions.obj	\
    $(SLO)$/fontsubstconfig.obj				\
    $(SLO)$/misccfg.obj				\
    $(SLO)$/apearcfg.obj				\
    $(SLO)$/helpopt.obj                 \
    $(SLO)$/printoptions.obj			\
    $(SLO)$/languageoptions.obj     \
    $(SLO)$/ctloptions.obj          \
    $(SLO)$/cjkoptions.obj          \
    $(SLO)$/colorcfg.obj            \
    $(SLO)$/extcolorcfg.obj            \
    $(SLO)$/undoopt.obj                 \
    $(SLO)$/useroptions.obj      \
    $(SLO)$/syslocaleoptions.obj \
    $(SLO)$/itemholder2.obj \
    $(SLO)$/miscopt.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

