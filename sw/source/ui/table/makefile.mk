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
# $Revision: 1.8 $
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

EXCEPTIONSFILES=   \
        $(SLO)$/chartins.obj\

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

SLOFILES =  \
        $(SLO)$/colwd.obj \
        $(SLO)$/convert.obj \
        $(SLO)$/chartins.obj \
        $(SLO)$/instable.obj \
        $(SLO)$/mergetbl.obj \
        $(SLO)$/rowht.obj \
        $(SLO)$/splittbl.obj \
        $(SLO)$/tabledlg.obj \
        $(SLO)$/tablemgr.obj \
        $(SLO)$/swtablerep.obj \
        $(SLO)$/tautofmt.obj

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
        $(SLO)$/chartins.obj \
        $(SLO)$/swtablerep.obj \
        $(SLO)$/tablemgr.obj 
        
        
# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

