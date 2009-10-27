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
# $Revision: 1.31 $
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

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(PRJ)$/util\svxpch

PRJNAME=svx
TARGET=options
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES=\
    optaccessibility.src    \
    optasian.src    \
    optcolor.src    \
    optjsearch.src  \
    optgenrl.src    \
    optdict.src     \
    optsave.src     \
    optpath.src     \
    optlingu.src    \
    optgrid.src     \
    optinet2.src    \
    multipat.src    \
    optctl.src      \
    optchart.src    \
    optimprove.src

EXCEPTIONSFILES=\
    $(SLO)$/optlingu.obj

LIB1TARGET= $(SLB)$/$(TARGET)-core.lib
LIB1OBJFILES= \
    $(SLO)$/asiancfg.obj

LIB2TARGET= $(SLB)$/$(TARGET).lib
LIB2OBJFILES= \
    $(SLO)$/optlingu.obj \
    $(SLO)$/htmlcfg.obj \
    $(SLO)$/optitems.obj    \
    $(SLO)$/optgrid.obj \
    $(SLO)$/srchcfg.obj

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

