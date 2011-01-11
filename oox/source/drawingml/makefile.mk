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

PRJNAME=oox
TARGET=drawingml
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/clrscheme.obj\
        $(SLO)$/clrschemecontext.obj\
        $(SLO)$/color.obj\
        $(SLO)$/colorchoicecontext.obj\
        $(SLO)$/connectorshapecontext.obj\
        $(SLO)$/customshapegeometry.obj\
        $(SLO)$/customshapeproperties.obj\
        $(SLO)$/drawingmltypes.obj\
        $(SLO)$/embeddedwavaudiofile.obj\
        $(SLO)$/fillproperties.obj\
        $(SLO)$/fillpropertiesgroupcontext.obj\
        $(SLO)$/graphicshapecontext.obj\
        $(SLO)$/guidcontext.obj\
        $(SLO)$/hyperlinkcontext.obj\
        $(SLO)$/lineproperties.obj\
        $(SLO)$/linepropertiescontext.obj\
        $(SLO)$/objectdefaultcontext.obj\
        $(SLO)$/shape.obj\
        $(SLO)$/shapecontext.obj\
        $(SLO)$/shapegroupcontext.obj\
        $(SLO)$/shapepropertiescontext.obj\
        $(SLO)$/shapepropertymap.obj\
        $(SLO)$/shapestylecontext.obj\
        $(SLO)$/spdefcontext.obj\
        $(SLO)$/textbody.obj\
        $(SLO)$/textbodycontext.obj\
        $(SLO)$/textbodyproperties.obj\
        $(SLO)$/textbodypropertiescontext.obj\
        $(SLO)$/textcharacterproperties.obj\
        $(SLO)$/textcharacterpropertiescontext.obj\
        $(SLO)$/textfield.obj\
        $(SLO)$/textfieldcontext.obj\
        $(SLO)$/textfont.obj\
        $(SLO)$/textliststyle.obj \
        $(SLO)$/textliststylecontext.obj\
        $(SLO)$/textparagraph.obj\
        $(SLO)$/textparagraphproperties.obj\
        $(SLO)$/textparagraphpropertiescontext.obj\
        $(SLO)$/textrun.obj\
        $(SLO)$/textspacingcontext.obj\
        $(SLO)$/texttabstoplistcontext.obj\
        $(SLO)$/theme.obj\
        $(SLO)$/themeelementscontext.obj\
        $(SLO)$/themefragmenthandler.obj\
        $(SLO)$/transform2dcontext.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
