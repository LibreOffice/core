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
TARGET=xmloff_draw
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

INC+= -I$(PRJ)$/inc$/bf_xmloff

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/xmloff_XMLNumberStyles.obj\
        $(SLO)$/xmloff_XMLGraphicsDefaultStyle.obj\
        $(SLO)$/xmloff_viewcontext.obj\
        $(SLO)$/xmloff_eventimp.obj\
        $(SLO)$/xmloff_layerexp.obj\
        $(SLO)$/xmloff_layerimp.obj\
        $(SLO)$/xmloff_ximpshow.obj\
        $(SLO)$/xmloff_animimp.obj\
        $(SLO)$/xmloff_animexp.obj\
        $(SLO)$/xmloff_numithdl.obj\
        $(SLO)$/xmloff_sdxmlimp.obj \
        $(SLO)$/xmloff_sdxmlexp.obj \
        $(SLO)$/xmloff_ximpstyl.obj \
        $(SLO)$/xmloff_ximpbody.obj \
        $(SLO)$/xmloff_ximpshap.obj \
        $(SLO)$/xmloff_ximpgrp.obj \
        $(SLO)$/xmloff_ximp3dscene.obj \
        $(SLO)$/xmloff_ximp3dobject.obj \
        $(SLO)$/xmloff_ximpnote.obj \
        $(SLO)$/xmloff_sdpropls.obj \
        $(SLO)$/xmloff_propimp0.obj \
        $(SLO)$/xmloff_xexptran.obj \
        $(SLO)$/xmloff_shapeexport.obj \
        $(SLO)$/xmloff_shapeexport2.obj \
        $(SLO)$/xmloff_shapeexport3.obj \
        $(SLO)$/xmloff_shapeimport.obj \
        $(SLO)$/xmloff_XMLImageMapExport.obj \
        $(SLO)$/xmloff_XMLImageMapContext.obj \
        $(SLO)$/xmloff_XMLShapePropertySetContext.obj \
        $(SLO)$/xmloff_XMLShapeStyleContext.obj \
        $(SLO)$/xmloff_ximppage.obj

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

