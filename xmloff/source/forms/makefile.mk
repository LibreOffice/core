#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 14:14:30 $
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
TARGET=forms

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

# UNO header files
UNOUCROUT=$(OUT)$/inc
INCPRE+=$(UNOUCROUT)

UNOTYPES=\
    com.sun.star.form.binding.XBindableValue \
    com.sun.star.form.binding.XListEntrySource \
    com.sun.star.form.binding.XListEntrySink

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

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
