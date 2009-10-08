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
# $Revision: 1.10 $
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
PRJINC=$(PRJ)$/source$/inc
PRJNAME=configmgr
TARGET=treemgr

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/makefile.pmk

# --- Files -------------------------------------

SLOFILES=	\
        $(SLO)$/collectchanges.obj \
        $(SLO)$/configdefaultprovider.obj	\
        $(SLO)$/configexcept.obj	\
        $(SLO)$/configgroup.obj	\
        $(SLO)$/configpath.obj	\
        $(SLO)$/configset.obj	\
        $(SLO)$/defaultproviderproxy.obj	\
        $(SLO)$/valuemembernode.obj	\
        $(SLO)$/nodechange.obj	\
        $(SLO)$/nodechangeimpl.obj	\
        $(SLO)$/nodechangeinfo.obj	\
        $(SLO)$/nodefactory.obj	\
        $(SLO)$/nodeimpl.obj	\
        $(SLO)$/nodeimplobj.obj	\
        $(SLO)$/noderef.obj	\
        $(SLO)$/roottree.obj	\
        $(SLO)$/setnodeimpl.obj	\
        $(SLO)$/template.obj	\
        $(SLO)$/templateimpl.obj	\
        $(SLO)$/treeimpl.obj	\
        $(SLO)$/viewaccess.obj \
        $(SLO)$/viewstrategy.obj \
        $(SLO)$/viewnode.obj \
        $(SLO)$/deferredview.obj \
        $(SLO)$/directview.obj \
        $(SLO)$/readonlyview.obj \


# --- Targets ----------------------------------

.INCLUDE : target.mk

