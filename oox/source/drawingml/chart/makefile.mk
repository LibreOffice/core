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
# $Revision: 1.3 $
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
    $(SLO)$/axismodel.obj				\
    $(SLO)$/chartformatinfo.obj			\
    $(SLO)$/chartfragment.obj			\
    $(SLO)$/chartmodel.obj				\
    $(SLO)$/chartobjecttable.obj		\
    $(SLO)$/charttypeinfo.obj			\
    $(SLO)$/layoutcontext.obj			\
    $(SLO)$/layoutmodel.obj				\
    $(SLO)$/legendcontext.obj			\
    $(SLO)$/legendmodel.obj				\
    $(SLO)$/plotareacontext.obj			\
    $(SLO)$/plotareamodel.obj			\
    $(SLO)$/seriescontext.obj			\
    $(SLO)$/seriesmodel.obj				\
    $(SLO)$/seriessourcecontext.obj		\
    $(SLO)$/seriessourcemodel.obj		\
    $(SLO)$/typegroupcontext.obj		\
    $(SLO)$/typegroupmodel.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
