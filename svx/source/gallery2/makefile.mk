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
PRJNAME=svx
TARGET=gal

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	\
            gallery.src				\
            galtheme.src			

SLOFILES =	\
            $(EXCEPTIONSFILES)		\
            $(SLO)$/codec.obj

EXCEPTIONSFILES =	\
            $(SLO)$/galbrws.obj		\
            $(SLO)$/galbrws1.obj	\
            $(SLO)$/galbrws2.obj	\
            $(SLO)$/galctrl.obj		\
            $(SLO)$/galexpl.obj		\
            $(SLO)$/gallery1.obj	\
            $(SLO)$/galmisc.obj		\
            $(SLO)$/galobj.obj		\
            $(SLO)$/galtheme.obj

RESLIB1NAME = $(TARGET)
RESLIB1IMAGES = $(PRJ)$/res
RESLIB1SRSFILES = $(SRS)$/$(TARGET).srs

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk
