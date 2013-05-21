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
PRJNAME=cui
TARGET=dialogs
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"
CFLAGS+= -DENABLE_LAYOUT=1 -I../$(PRJ)/layout/inc -I../$(PRJ)/layout/$(INPATH)/inc
.ENDIF # ENABLE_LAYOUT == TRUE

.IF "$(BUILD_VER_STRING)" != ""
CFLAGS+= -DBUILD_VER_STRING="$(BUILD_VER_STRING)"
.ENDIF

# compile date (year) = copyright year used in About dialog
CFLAGS+= -DCOPYRIGHT_YEAR="$(shell date +"%Y")"


# --- Files --------------------------------------------------------

SRS1NAME=dialogs
SRC1FILES =  \
        about.src \
        colorpicker.src \
        charmap.src \
        commonlingui.src \
        cuiimapdlg.src \
        dlgname.src \
        fmsearch.src \
        gallery.src \
        grfflt.src \
        hangulhanjadlg.src \
        hyphen.src \
        hlmarkwn.src \
        hyperdlg.src \
        iconcdlg.src \
        insrc.src \
        multipat.src \
        newtabledlg.src \
        passwdomdlg.src \
        postdlg.src \
        scriptdlg.src \
        sdrcelldlg.src \
        showcols.src \
        SpellDialog.src \
        splitcelldlg.src \
        srchxtra.src \
        svuidlg.src \
        tbxform.src \
        thesdlg.src \
        zoom.src \


SLOFILES+=\
        $(SLO)$/about.obj \
        $(SLO)$/colorpicker.obj \
        $(SLO)$/commonlingui.obj \
        $(SLO)$/cuicharmap.obj \
        $(SLO)$/cuifmsearch.obj \
        $(SLO)$/cuigaldlg.obj \
        $(SLO)$/cuigrfflt.obj \
        $(SLO)$/cuihyperdlg.obj \
        $(SLO)$/cuiimapwnd.obj \
        $(SLO)$/cuitbxform.obj \
        $(SLO)$/dlgname.obj \
        $(SLO)$/hangulhanjadlg.obj \
        $(SLO)$/hldocntp.obj \
        $(SLO)$/hldoctp.obj \
        $(SLO)$/hlinettp.obj \
        $(SLO)$/hlmailtp.obj \
        $(SLO)$/hlmarkwn.obj \
        $(SLO)$/hltpbase.obj \
        $(SLO)$/hyphen.obj \
        $(SLO)$/iconcdlg.obj \
        $(SLO)$/insdlg.obj \
        $(SLO)$/insrc.obj \
        $(SLO)$/linkdlg.obj \
        $(SLO)$/multifil.obj \
        $(SLO)$/multipat.obj \
        $(SLO)$/newtabledlg.obj \
        $(SLO)$/passwdomdlg.obj \
        $(SLO)$/pastedlg.obj \
        $(SLO)$/plfilter.obj \
        $(SLO)$/postdlg.obj \
        $(SLO)$/scriptdlg.obj \
        $(SLO)$/sdrcelldlg.obj \
        $(SLO)$/showcols.obj \
        $(SLO)$/SpellAttrib.obj \
        $(SLO)$/SpellDialog.obj \
        $(SLO)$/splitcelldlg.obj \
        $(SLO)$/srchxtra.obj \
        $(SLO)$/thesdlg.obj \
        $(SLO)$/zoom.obj \


.IF "$(GUI)"=="WNT"
SLOFILES+=$(SLO)$/winpluginlib.obj
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
