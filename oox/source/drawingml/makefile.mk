#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 18:24:46 $
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
        $(SLO)$/hyperlinkcontext.obj\
        $(SLO)$/lineproperties.obj\
        $(SLO)$/linepropertiescontext.obj\
        $(SLO)$/objectdefaultcontext.obj\
        $(SLO)$/shape.obj\
        $(SLO)$/shapecontext.obj\
        $(SLO)$/shapegroupcontext.obj\
        $(SLO)$/shapepropertiescontext.obj\
        $(SLO)$/shapestylecontext.obj\
        $(SLO)$/spdefcontext.obj\
        $(SLO)$/textbody.obj\
        $(SLO)$/textbodycontext.obj\
        $(SLO)$/textbodypropertiescontext.obj\
        $(SLO)$/textcharacterproperties.obj\
        $(SLO)$/textcharacterpropertiescontext.obj\
        $(SLO)$/textfield.obj\
        $(SLO)$/textfieldcontext.obj\
        $(SLO)$/textfontcontext.obj\
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
        $(SLO)$/themefragmenthandler.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
