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
# $Revision: 1.6 $
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

PRJNAME=oox
TARGET=chart
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =								\
    $(SLO)$/axiscontext.obj				\
    $(SLO)$/axisconverter.obj			\
    $(SLO)$/axismodel.obj				\
    $(SLO)$/chartcontextbase.obj		\
    $(SLO)$/chartconverter.obj			\
    $(SLO)$/chartdrawingfragment.obj	\
    $(SLO)$/chartspaceconverter.obj		\
    $(SLO)$/chartspacefragment.obj		\
    $(SLO)$/chartspacemodel.obj			\
    $(SLO)$/converterbase.obj			\
    $(SLO)$/datasourcecontext.obj		\
    $(SLO)$/datasourceconverter.obj		\
    $(SLO)$/datasourcemodel.obj			\
    $(SLO)$/modelbase.obj				\
    $(SLO)$/objectformatter.obj			\
    $(SLO)$/plotareacontext.obj			\
    $(SLO)$/plotareaconverter.obj		\
    $(SLO)$/plotareamodel.obj			\
    $(SLO)$/seriescontext.obj			\
    $(SLO)$/seriesconverter.obj			\
    $(SLO)$/seriesmodel.obj				\
    $(SLO)$/titlecontext.obj			\
    $(SLO)$/titleconverter.obj			\
    $(SLO)$/titlemodel.obj				\
    $(SLO)$/typegroupcontext.obj		\
    $(SLO)$/typegroupconverter.obj		\
    $(SLO)$/typegroupmodel.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
