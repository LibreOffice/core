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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..

PRJNAME=binfilter
TARGET=xmloff_forms

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_xmloff

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/xmloff_formstyles.obj \
        $(SLO)$/xmloff_officeforms.obj \
        $(SLO)$/xmloff_formevents.obj \
        $(SLO)$/xmloff_eventimport.obj \
        $(SLO)$/xmloff_eventexport.obj \
        $(SLO)$/xmloff_controlpropertyhdl.obj \
        $(SLO)$/xmloff_controlpropertymap.obj \
        $(SLO)$/xmloff_valueproperties.obj \
        $(SLO)$/xmloff_attriblistmerge.obj \
        $(SLO)$/xmloff_controlelement.obj \
        $(SLO)$/xmloff_formlayerexport.obj \
        $(SLO)$/xmloff_layerexport.obj \
        $(SLO)$/xmloff_elementexport.obj \
        $(SLO)$/xmloff_propertyexport.obj \
        $(SLO)$/xmloff_elementimport.obj \
        $(SLO)$/xmloff_layerimport.obj \
        $(SLO)$/xmloff_propertyimport.obj \
        $(SLO)$/xmloff_formlayerimport.obj \
        $(SLO)$/xmloff_formattributes.obj \
        $(SLO)$/xmloff_formenums.obj \
        $(SLO)$/xmloff_strings.obj \
        $(SLO)$/xmloff_logging.obj \
        $(SLO)$/xmloff_formcellbinding.obj \

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
