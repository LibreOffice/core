#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2008-01-17 08:06:00 $
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

PRJNAME=oox
TARGET=ppt
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/pptimport.obj\
        $(SLO)$/presentationfragmenthandler.obj\
        $(SLO)$/slidefragmenthandler.obj\
        $(SLO)$/layoutfragmenthandler.obj\
        $(SLO)$/backgroundproperties.obj\
        $(SLO)$/slidetransitioncontext.obj\
        $(SLO)$/slidetransition.obj\
        $(SLO)$/slidetimingcontext.obj\
        $(SLO)$/slidepersist.obj\
        $(SLO)$/slidemastertextstylescontext.obj \
        $(SLO)$/timenode.obj\
        $(SLO)$/pptfilterhelpers.obj\
        $(SLO)$/soundactioncontext.obj \
        $(SLO)$/commontimenodecontext.obj \
        $(SLO)$/commonbehaviorcontext.obj \
        $(SLO)$/conditioncontext.obj \
        $(SLO)$/timetargetelementcontext.obj \
        $(SLO)$/timenodelistcontext.obj \
        $(SLO)$/animationspersist.obj \
        $(SLO)$/animvariantcontext.obj \
        $(SLO)$/timeanimvaluecontext.obj \
        $(SLO)$/pptshape.obj \
        $(SLO)$/pptshapegroupcontext.obj \
        $(SLO)$/pptshapecontext.obj \
        $(SLO)$/pptshapepropertiescontext.obj \
        $(SLO)$/buildlistcontext.obj \
        $(SLO)$/animationtypes.obj \
        $(SLO)$/customshowlistcontext.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
