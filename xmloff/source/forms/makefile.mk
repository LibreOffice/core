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

PRJ=..$/..

PRJNAME=xmloff
TARGET=forms

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/officeforms.obj \
        $(SLO)$/formevents.obj \
        $(SLO)$/eventimport.obj \
        $(SLO)$/eventexport.obj \
        $(SLO)$/controlpropertyhdl.obj \
        $(SLO)$/controlpropertymap.obj \
        $(SLO)$/valueproperties.obj \
        $(SLO)$/attriblistmerge.obj \
        $(SLO)$/controlelement.obj \
        $(SLO)$/formlayerexport.obj \
        $(SLO)$/layerexport.obj \
        $(SLO)$/elementexport.obj \
        $(SLO)$/propertyexport.obj \
        $(SLO)$/elementimport.obj \
        $(SLO)$/layerimport.obj \
        $(SLO)$/propertyimport.obj \
        $(SLO)$/formlayerimport.obj \
        $(SLO)$/formattributes.obj \
        $(SLO)$/formenums.obj \
        $(SLO)$/formsimp.obj \
        $(SLO)$/strings.obj \
        $(SLO)$/logging.obj \
        $(SLO)$/formcellbinding.obj \
        $(SLO)$/gridcolumnproptranslator.obj \
        \
        $(SLO)/property_meta_data.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
