#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 19:02:41 $
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
TARGET=xls
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =										\
        $(SLO)$/addressconverter.obj			\
        $(SLO)$/autofiltercontext.obj			\
        $(SLO)$/biffcodec.obj					\
        $(SLO)$/biffdetector.obj				\
        $(SLO)$/bifffragmenthandler.obj			\
        $(SLO)$/biffhelper.obj					\
        $(SLO)$/biffinputstream.obj				\
        $(SLO)$/biffoutputstream.obj			\
        $(SLO)$/chartsheetfragment.obj			\
        $(SLO)$/condformatbuffer.obj			\
        $(SLO)$/condformatcontext.obj			\
        $(SLO)$/connectionsfragment.obj			\
        $(SLO)$/defnamesbuffer.obj				\
        $(SLO)$/drawingfragment.obj				\
        $(SLO)$/excelfilter.obj					\
        $(SLO)$/excelhandlers.obj				\
        $(SLO)$/externallinkbuffer.obj			\
        $(SLO)$/externallinkfragment.obj		\
        $(SLO)$/formulabase.obj					\
        $(SLO)$/formulaparser.obj				\
        $(SLO)$/headerfooterparser.obj			\
        $(SLO)$/numberformatsbuffer.obj			\
        $(SLO)$/pagesettings.obj				\
        $(SLO)$/pivotcachefragment.obj			\
        $(SLO)$/pivottablebuffer.obj			\
        $(SLO)$/pivottablefragment.obj			\
        $(SLO)$/querytablefragment.obj			\
        $(SLO)$/richstring.obj					\
        $(SLO)$/richstringcontext.obj			\
        $(SLO)$/sharedformulabuffer.obj			\
        $(SLO)$/sharedstringsbuffer.obj			\
        $(SLO)$/sharedstringsfragment.obj		\
        $(SLO)$/sheetcellrangemap.obj			\
        $(SLO)$/sheetdatacontext.obj			\
        $(SLO)$/stylesbuffer.obj				\
        $(SLO)$/stylesfragment.obj				\
        $(SLO)$/stylespropertyhelper.obj		\
        $(SLO)$/tablebuffer.obj					\
        $(SLO)$/tablefragment.obj				\
        $(SLO)$/themebuffer.obj					\
        $(SLO)$/unitconverter.obj				\
        $(SLO)$/validationpropertyhelper.obj	\
        $(SLO)$/viewsettings.obj				\
        $(SLO)$/webquerybuffer.obj				\
        $(SLO)$/workbookfragment.obj			\
        $(SLO)$/workbookhelper.obj				\
        $(SLO)$/workbooksettings.obj			\
        $(SLO)$/worksheetbuffer.obj				\
        $(SLO)$/worksheetfragment.obj			\
        $(SLO)$/worksheethelper.obj             \
        $(SLO)$/worksheetsettings.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
