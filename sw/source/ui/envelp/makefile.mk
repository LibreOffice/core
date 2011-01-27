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

PRJ=..$/..$/..

PRJNAME=sw
TARGET=envelp
LIBTARGET=no
# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =\
        envelp.src \
        envfmt.src \
        envlop.src \
        envprt.src \
        label.src  \
        labfmt.src \
        labprt.src \
        mailmrge.src

SLOFILES =  \
        $(EXCEPTIONSFILES) \
        $(SLO)$/labimg.obj

EXCEPTIONSFILES= \
        $(SLO)$/envfmt.obj \
        $(SLO)$/envimg.obj \
        $(SLO)$/envlop1.obj \
        $(SLO)$/envprt.obj \
        $(SLO)$/label1.obj \
        $(SLO)$/labelcfg.obj \
        $(SLO)$/labelexp.obj \
        $(SLO)$/labfmt.obj \
        $(SLO)$/labprt.obj \
        $(SLO)$/mailmrge.obj \
        $(SLO)$/syncbtn.obj

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
     $(SLO)$/envimg.obj  \
        $(SLO)$/labelcfg.obj  \
        $(SLO)$/labimg.obj  \
        $(SLO)$/syncbtn.obj
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

