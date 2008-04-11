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
# $Revision: 1.30 $
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
    optchart.src

#EXCEPTIONSFILES=\
#	$(SLO)$/optasian.obj		\
#	$(SLO)$/optlingu.obj		\
#	$(SLO)$/optaccessibility.obj	\
#	$(SLO)$/optsave.obj		\
#	$(SLO)$/optpath.obj

#SLOFILES=\
#	$(EXCEPTIONSFILES)	\
#	$(SLO)$/asiancfg.obj	\
#	$(SLO)$/htmlcfg.obj	\
#	$(SLO)$/optcolor.obj    \
#	$(SLO)$/optjsearch.obj  \
#	$(SLO)$/optitems.obj	\
#	$(SLO)$/optgenrl.obj	\
#	$(SLO)$/adritem.obj	\
#	$(SLO)$/optgrid.obj	\
#	$(SLO)$/optinet2.obj	\
#	$(SLO)$/optextbr.obj    \
#	$(SLO)$/srchcfg.obj	\
#	$(SLO)$/optctl.obj

EXCEPTIONSFILES=\
    $(SLO)$/optlingu.obj

SLOFILES=\
    $(EXCEPTIONSFILES)	\
    $(SLO)$/asiancfg.obj    \
    $(SLO)$/htmlcfg.obj \
    $(SLO)$/optitems.obj    \
    $(SLO)$/optgrid.obj \
    $(SLO)$/srchcfg.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

