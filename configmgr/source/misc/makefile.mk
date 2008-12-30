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
# $Revision: 1.28 $
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
PRJINC=$(PRJ)$/source
PRJNAME=configmgr
TARGET=misc

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE : $(PRJ)$/version.mk

# --- Files -------------------------------------

SLOFILES=	\
        $(SLO)$/bootstrap.obj	\
        $(SLO)$/providerfactory.obj	\
        $(SLO)$/providerwrapper.obj	\
        $(SLO)$/logger.obj	\
        $(SLO)$/tracer.obj	\
        $(SLO)$/configunoreg.obj	\
        $(SLO)$/serviceinfohelper.obj \
        $(SLO)$/bootstrapcontext.obj	\
        $(SLO)$/anypair.obj	\
        $(SLO)$/strimpl.obj	\
        $(SLO)$/mergechange.obj \
        $(SLO)$/oslstream.obj	\
        $(SLO)$/filehelper.obj	\
        $(SLO)$/bufferedfile.obj \
        $(SLO)$/requestoptions.obj	\
        $(SLO)$/interactionrequest.obj	\
        $(SLO)$/configinteractionhandler.obj	\
        $(SLO)$/simpleinteractionrequest.obj	\
        $(SLO)$/propertysethelper.obj	\
        
OBJFILES=	\
        $(OBJ)$/oslstream.obj	\
        $(OBJ)$/filehelper.obj	\
        $(OBJ)$/bufferedfile.obj \


# --- Targets ----------------------------------

.INCLUDE : target.mk

