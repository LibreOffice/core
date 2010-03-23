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
PRJINC=$(PRJ)$/source
PRJNAME=reportdesign
TARGET=api

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/dll.pmk

# --- Files -------------------------------------
SLOFILES=	\
        $(SLO)$/FormattedField.obj		\
        $(SLO)$/Groups.obj				\
        $(SLO)$/Group.obj				\
        $(SLO)$/Section.obj				\
        $(SLO)$/services.obj			\
        $(SLO)$/FixedText.obj			\
        $(SLO)$/ImageControl.obj		\
        $(SLO)$/ReportDefinition.obj	\
        $(SLO)$/ReportComponent.obj		\
        $(SLO)$/ReportEngineJFree.obj	\
        $(SLO)$/Tools.obj				\
        $(SLO)$/Function.obj			\
        $(SLO)$/Functions.obj			\
        $(SLO)$/ReportControlModel.obj	\
        $(SLO)$/FormatCondition.obj		\
        $(SLO)$/ReportVisitor.obj		\
        $(SLO)$/Shape.obj				\
        $(SLO)$/FixedLine.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

