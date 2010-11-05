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

PRJNAME=sc
TARGET=pagedlg
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        areasdlg.cxx	\
        hfedtdlg.cxx	\
        tptable.cxx	\
        tphf.cxx 	\
        tphfedit.cxx

            

SLOFILES = \
        $(EXCEPTIONSFILES)

EXCEPTIONSFILES = \
        $(SLO)$/areasdlg.obj		\
        $(SLO)$/hfedtdlg.obj		\
        $(SLO)$/scuitphfedit.obj	\
        $(SLO)$/tphf.obj		\
        $(SLO)$/tphfedit.obj		\
        $(SLO)$/tptable.obj		\

SRS1NAME=$(TARGET)
SRC1FILES =  pagedlg.src     	\
            hfedtdlg.src    	\
            tphf.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
        $(SLO)$/areasdlg.obj	\
        $(SLO)$/tphfedit.obj
# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

