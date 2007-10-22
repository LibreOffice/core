#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: vg $ $Date: 2007-10-22 16:34:38 $
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

PRJ					= ..$/..
PRJNAME				= xmloff
TARGET				= chart
AUTOSEG				= true
ENABLE_EXCEPTIONS	= TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/ColorPropertySet.obj \
            $(SLO)$/SchXMLTools.obj \
            $(SLO)$/SchXMLExport.obj \
            $(SLO)$/SchXMLImport.obj \
            $(SLO)$/contexts.obj \
            $(SLO)$/SchXMLTableContext.obj \
            $(SLO)$/SchXMLChartContext.obj \
            $(SLO)$/SchXMLPlotAreaContext.obj \
            $(SLO)$/SchXMLParagraphContext.obj \
            $(SLO)$/SchXMLSeriesHelper.obj \
            $(SLO)$/SchXMLSeries2Context.obj \
            $(SLO)$/PropertyMaps.obj \
            $(SLO)$/XMLChartStyleContext.obj \
            $(SLO)$/XMLErrorIndicatorPropertyHdl.obj \
            $(SLO)$/SchXMLAutoStylePoolP.obj \
            $(SLO)$/XMLChartPropertyContext.obj \
            $(SLO)$/XMLSymbolImageContext.obj \
            $(SLO)$/XMLLabelSeparatorContext.obj \
            $(SLO)$/XMLTextOrientationHdl.obj \
            $(SLO)$/XMLSymbolTypePropertyHdl.obj \
            $(SLO)$/transporttypes.obj

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

