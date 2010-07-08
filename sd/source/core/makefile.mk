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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=core
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

IENV+=-I..\ui\inc

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = glob.src

SLOFILES = $(SLO)$/stlsheet.obj  \
           $(SLO)$/stlfamily.obj \
           $(SLO)$/stlpool.obj	\
           $(SLO)$/drawdoc.obj \
           $(SLO)$/drawdoc2.obj \
           $(SLO)$/drawdoc3.obj \
           $(SLO)$/drawdoc4.obj \
           $(SLO)$/drawdoc_animations.obj\
           $(SLO)$/sdpage.obj \
           $(SLO)$/sdpage2.obj	\
           $(SLO)$/sdattr.obj \
           $(SLO)$/sdobjfac.obj \
           $(SLO)$/anminfo.obj	\
           $(SLO)$/sdiocmpt.obj	\
           $(SLO)$/typemap.obj	\
           $(SLO)$/pglink.obj   \
           $(SLO)$/cusshow.obj  \
           $(SLO)$/PageListWatcher.obj  \
           $(SLO)$/sdpage_animations.obj\
           $(SLO)$/CustomAnimationPreset.obj\
           $(SLO)$/CustomAnimationEffect.obj\
           $(SLO)$/TransitionPreset.obj\
           $(SLO)$/undoanim.obj\
           $(SLO)$/EffectMigration.obj\
           $(SLO)$/CustomAnimationCloner.obj\
           $(SLO)$/shapelist.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

