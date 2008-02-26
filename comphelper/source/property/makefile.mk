#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 15:14:38 $
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
PRJINC=..$/..$/inc
PRJNAME=comphelper
TARGET=property

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

SLOFILES= 	\
            $(SLO)$/MasterPropertySetInfo.obj \
            $(SLO)$/MasterPropertySet.obj \
            $(SLO)$/ChainablePropertySetInfo.obj \
            $(SLO)$/ChainablePropertySet.obj \
            $(SLO)$/TypeGeneration.obj \
            $(SLO)$/genericpropertyset.obj\
            $(SLO)$/propertysethelper.obj \
            $(SLO)$/propertysetinfo.obj \
            $(SLO)$/composedprops.obj \
            $(SLO)$/propagg.obj \
            $(SLO)$/property.obj \
            $(SLO)$/propmultiplex.obj \
            $(SLO)$/propstate.obj \
            $(SLO)$/propertystatecontainer.obj \
            $(SLO)$/propertycontainer.obj \
            $(SLO)$/propertycontainerhelper.obj \
            $(SLO)$/propertybag.obj \
            $(SLO)$/opropertybag.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

