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



PRJ=..$/..$/..

PRJNAME=sc
TARGET=vbaobj
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

.IF "$(L10N_framework)"==""
INCPRE=$(INCCOM)$/$(TARGET)

# ------------------------------------------------------------------

SLOFILES= \
        $(SLO)$/excelvbahelper.obj \
        $(SLO)$/service.obj \
        $(SLO)$/vbaapplication.obj \
        $(SLO)$/vbaassistant.obj \
        $(SLO)$/vbaaxes.obj \
        $(SLO)$/vbaaxis.obj \
        $(SLO)$/vbaaxistitle.obj \
        $(SLO)$/vbaborders.obj \
        $(SLO)$/vbacharacters.obj \
        $(SLO)$/vbachart.obj \
        $(SLO)$/vbachartobject.obj \
        $(SLO)$/vbachartobjects.obj \
        $(SLO)$/vbacharts.obj \
        $(SLO)$/vbacharttitle.obj \
        $(SLO)$/vbacomment.obj \
        $(SLO)$/vbacomments.obj \
        $(SLO)$/vbacondition.obj \
        $(SLO)$/vbadialog.obj \
        $(SLO)$/vbadialogs.obj \
        $(SLO)$/vbaeventshelper.obj \
        $(SLO)$/vbafont.obj \
        $(SLO)$/vbaformat.obj \
        $(SLO)$/vbaformatcondition.obj \
        $(SLO)$/vbaformatconditions.obj \
        $(SLO)$/vbaglobals.obj \
        $(SLO)$/vbahyperlink.obj \
        $(SLO)$/vbahyperlinks.obj \
        $(SLO)$/vbainterior.obj \
        $(SLO)$/vbaname.obj \
        $(SLO)$/vbanames.obj \
        $(SLO)$/vbaoleobject.obj \
        $(SLO)$/vbaoleobjects.obj \
        $(SLO)$/vbaoutline.obj \
        $(SLO)$/vbapagebreak.obj \
        $(SLO)$/vbapagebreaks.obj \
        $(SLO)$/vbapagesetup.obj \
        $(SLO)$/vbapalette.obj \
        $(SLO)$/vbapane.obj \
        $(SLO)$/vbapivotcache.obj \
        $(SLO)$/vbapivottable.obj \
        $(SLO)$/vbapivottables.obj \
        $(SLO)$/vbarange.obj \
        $(SLO)$/vbaseriescollection.obj \
        $(SLO)$/vbasheetobject.obj \
        $(SLO)$/vbasheetobjects.obj \
        $(SLO)$/vbastyle.obj \
        $(SLO)$/vbastyles.obj \
        $(SLO)$/vbatextboxshape.obj \
        $(SLO)$/vbatextframe.obj \
        $(SLO)$/vbavalidation.obj \
        $(SLO)$/vbawindow.obj \
        $(SLO)$/vbawindows.obj \
        $(SLO)$/vbaworkbook.obj \
        $(SLO)$/vbaworkbooks.obj \
        $(SLO)$/vbaworksheet.obj \
        $(SLO)$/vbaworksheets.obj \
        $(SLO)$/vbawsfunction.obj

.ENDIF
# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

