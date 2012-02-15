#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
        $(SLO)$/autofilterbuffer.obj			\
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
        $(SLO)$/drawingbase.obj					\
        $(SLO)$/drawingfragment.obj				\
        $(SLO)$/drawingmanager.obj				\
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
        $(SLO)$/sharedstringsbuffer.obj			\
        $(SLO)$/sharedstringsfragment.obj		\
        $(SLO)$/sheetdatabuffer.obj				\
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
        $(SLO)$/worksheethelper.obj				\
        $(SLO)$/worksheetsettings.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
