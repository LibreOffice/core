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
TARGET=table
LIBTARGET=no
# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        colwd.src \
        convert.src \
        chartins.src \
        instable.src \
        mergetbl.src \
        rowht.src \
        splittbl.src \
        table.src \
        tabledlg.src \
        tautofmt.src

EXCEPTIONSFILES=   \
        $(SLO)$/chartins.obj \
        $(SLO)$/colwd.obj \
        $(SLO)$/convert.obj \
        $(SLO)$/instable.obj \
        $(SLO)$/mergetbl.obj \
        $(SLO)$/rowht.obj \
        $(SLO)$/splittbl.obj \
        $(SLO)$/swtablerep.obj \
        $(SLO)$/tabledlg.obj \
        $(SLO)$/tablemgr.obj \
        $(SLO)$/tautofmt.obj

SLOFILES =  \
        $(EXCEPTIONSFILES)

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
        $(SLO)$/chartins.obj \
        $(SLO)$/swtablerep.obj \
        $(SLO)$/tablemgr.obj 
        
        
# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

