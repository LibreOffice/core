#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.26 $
#
#   last change: $Author: rt $ $Date: 2005-10-19 12:17:21 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..
PRJINC=$(PRJ)$/source
PRJNAME=configmgr
TARGET=misc

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
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
        $(SLO)$/attributelist.obj	\
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

