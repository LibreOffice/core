#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: ihi $ $Date: 2006-12-19 17:25:51 $
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
PRJNAME=xmloff
TARGET=draw
AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : svpre.mk
.INCLUDE : settings.mk
.INCLUDE : sv.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/animationexport.obj\
        $(SLO)$/animationimport.obj\
        $(SLO)$/XMLNumberStyles.obj\
        $(SLO)$/XMLGraphicsDefaultStyle.obj\
        $(SLO)$/viewcontext.obj\
        $(SLO)$/eventimp.obj\
        $(SLO)$/layerexp.obj\
        $(SLO)$/layerimp.obj\
        $(SLO)$/ximpshow.obj\
        $(SLO)$/animimp.obj\
        $(SLO)$/animexp.obj\
        $(SLO)$/numithdl.obj\
        $(SLO)$/sdxmlimp.obj \
        $(SLO)$/sdxmlexp.obj \
        $(SLO)$/ximpstyl.obj \
        $(SLO)$/ximpbody.obj \
        $(SLO)$/ximpshap.obj \
        $(SLO)$/ximpgrp.obj \
        $(SLO)$/ximp3dscene.obj \
        $(SLO)$/ximp3dobject.obj \
        $(SLO)$/ximpnote.obj \
        $(SLO)$/sdpropls.obj \
        $(SLO)$/propimp0.obj \
        $(SLO)$/xexptran.obj \
        $(SLO)$/shapeexport.obj \
        $(SLO)$/shapeexport2.obj \
        $(SLO)$/shapeexport3.obj \
        $(SLO)$/shapeexport4.obj \
        $(SLO)$/shapeimport.obj \
        $(SLO)$/XMLImageMapExport.obj \
        $(SLO)$/XMLImageMapContext.obj \
        $(SLO)$/XMLShapePropertySetContext.obj \
        $(SLO)$/XMLShapeStyleContext.obj \
        $(SLO)$/ximppage.obj \
        $(SLO)$/ximpcustomshape.obj \
        $(SLO)$/EnhancedCustomShapeToken.obj \
        $(SLO)$/XMLReplacementImageContext.obj \
        $(SLO)$/descriptionimp.obj

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

