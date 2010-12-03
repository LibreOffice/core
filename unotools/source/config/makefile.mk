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
PRJINC=..$/..$/inc
PRJNAME=unotools
TARGET=config

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_BROFFICE)"=="TRUE"
CDEFS+=-DENABLE_BROFFICE
.ENDIF

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/syslocaleoptions.obj \
        $(SLO)$/configvaluecontainer.obj \
        $(SLO)$/confignode.obj \
        $(SLO)$/configitem.obj \
        $(SLO)$/configmgr.obj  \
        $(SLO)$/configpathes.obj  \
        $(SLO)$/docinfohelper.obj  \
                $(SLO)$/bootstrap.obj \
        $(SLO)$/accelcfg.obj                                \
        $(SLO)$/cacheoptions.obj            \
    $(SLO)$/cmdoptions.obj				\
    $(SLO)$/compatibility.obj           \
    $(SLO)$/defaultoptions.obj			\
    $(SLO)$/dynamicmenuoptions.obj		\
    $(SLO)$/eventcfg.obj                \
    $(SLO)$/extendedsecurityoptions.obj \
        $(SLO)$/fltrcfg.obj                             \
    $(SLO)$/fontcfg.obj \
    $(SLO)$/fontoptions.obj				\
        $(SLO)$/historyoptions.obj                         \
    $(SLO)$/inetoptions.obj				\
    $(SLO)$/internaloptions.obj			\
        $(SLO)$/itemholder1.obj \
        $(SLO)$/javaoptions.obj             \
        $(SLO)$/lingucfg.obj                \
    $(SLO)$/localisationoptions.obj		\
        $(SLO)$/misccfg.obj                             \
        $(SLO)$/moduleoptions.obj           \
        $(SLO)$/options.obj                     \
        $(SLO)$/optionsdlg.obj              \
        $(SLO)$/pathoptions.obj             \
    $(SLO)$/printwarningoptions.obj		\
    $(SLO)$/regoptions.obj				\
    $(SLO)$/saveopt.obj					\
    $(SLO)$/searchopt.obj				\
    $(SLO)$/securityoptions.obj			\
    $(SLO)$/sourceviewconfig.obj		\
    $(SLO)$/startoptions.obj			\
        $(SLO)$/undoopt.obj                 \
        $(SLO)$/useroptions.obj      \
    $(SLO)$/viewoptions.obj				\
    $(SLO)$/workingsetoptions.obj		\
        $(SLO)$/xmlaccelcfg.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

