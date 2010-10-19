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
        $(SLO)$/biffhelper.obj					\
        $(SLO)$/biffinputstream.obj				\
        $(SLO)$/biffoutputstream.obj			\
        $(SLO)$/chartsheetfragment.obj			\
        $(SLO)$/commentsbuffer.obj				\
        $(SLO)$/commentsfragment.obj			\
        $(SLO)$/condformatbuffer.obj			\
        $(SLO)$/condformatcontext.obj			\
        $(SLO)$/connectionsbuffer.obj			\
        $(SLO)$/connectionsfragment.obj			\
        $(SLO)$/defnamesbuffer.obj				\
        $(SLO)$/drawingfragment.obj				\
        $(SLO)$/excelchartconverter.obj			\
        $(SLO)$/excelfilter.obj					\
        $(SLO)$/excelhandlers.obj				\
        $(SLO)$/excelvbaproject.obj				\
        $(SLO)$/externallinkbuffer.obj			\
        $(SLO)$/externallinkfragment.obj		\
        $(SLO)$/formulabase.obj					\
        $(SLO)$/formulaparser.obj				\
        $(SLO)$/numberformatsbuffer.obj			\
        $(SLO)$/ooxformulaparser.obj			\
        $(SLO)$/pagesettings.obj				\
        $(SLO)$/pivotcachebuffer.obj			\
        $(SLO)$/pivotcachefragment.obj			\
        $(SLO)$/pivottablebuffer.obj			\
        $(SLO)$/pivottablefragment.obj			\
        $(SLO)$/querytablebuffer.obj			\
        $(SLO)$/querytablefragment.obj			\
        $(SLO)$/richstring.obj					\
        $(SLO)$/richstringcontext.obj			\
        $(SLO)$/scenariobuffer.obj				\
        $(SLO)$/scenariocontext.obj				\
        $(SLO)$/sharedformulabuffer.obj			\
        $(SLO)$/sharedstringsbuffer.obj			\
        $(SLO)$/sharedstringsfragment.obj		\
        $(SLO)$/sheetdatacontext.obj			\
        $(SLO)$/stylesbuffer.obj				\
        $(SLO)$/stylesfragment.obj				\
        $(SLO)$/tablebuffer.obj					\
        $(SLO)$/tablefragment.obj				\
        $(SLO)$/themebuffer.obj					\
        $(SLO)$/unitconverter.obj				\
        $(SLO)$/viewsettings.obj				\
        $(SLO)$/workbookfragment.obj			\
        $(SLO)$/workbookhelper.obj				\
        $(SLO)$/workbooksettings.obj			\
        $(SLO)$/worksheetbuffer.obj				\
        $(SLO)$/worksheetfragment.obj			\
        $(SLO)$/worksheethelper.obj             \
        $(SLO)$/worksheetsettings.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
