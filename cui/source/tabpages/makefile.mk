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
TARGET=tabpages
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"
CFLAGS+= -DENABLE_LAYOUT=1 -I../$(PRJ)/layout/inc -I../$(PRJ)/layout/$(INPATH)/inc
.ENDIF # ENABLE_LAYOUT == TRUE

# --- Files --------------------------------------------------------

SRS1NAME=tabpages
SRC1FILES =  \
        align.src \
        autocdlg.src \
        backgrnd.src \
        bbdlg.src \
        border.src \
        chardlg.src \
        connect.src \
        dstribut.src \
        frmdirlbox.src \
        grfpage.src \
        labdlg.src \
        macroass.src \
        measure.src \
        numfmt.src \
        numpages.src \
        page.src \
        paragrph.src \
        strings.src \
        swpossizetabpage.src \
        tabarea.src \
        tabline.src \
        tabstpge.src \
        textanim.src \
        textattr.src \
        transfrm.src


SLOFILES+=\
        $(SLO)$/align.obj \
        $(SLO)$/autocdlg.obj \
        $(SLO)$/backgrnd.obj \
        $(SLO)$/bbdlg.obj \
        $(SLO)$/border.obj \
        $(SLO)$/borderconn.obj \
        $(SLO)$/chardlg.obj \
        $(SLO)$/connect.obj \
        $(SLO)$/dstribut.obj \
        $(SLO)$/grfpage.obj \
        $(SLO)$/labdlg.obj \
        $(SLO)$/macroass.obj \
        $(SLO)$/measure.obj \
        $(SLO)$/numfmt.obj \
        $(SLO)$/numpages.obj \
        $(SLO)$/page.obj \
        $(SLO)$/paragrph.obj \
        $(SLO)$/swpossizetabpage.obj \
        $(SLO)$/tabarea.obj \
        $(SLO)$/tabline.obj \
        $(SLO)$/tabstpge.obj \
        $(SLO)$/textanim.obj \
        $(SLO)$/textattr.obj \
        $(SLO)$/tparea.obj \
        $(SLO)$/tpbitmap.obj \
        $(SLO)$/tpcolor.obj \
        $(SLO)$/tpgradnt.obj \
        $(SLO)$/tphatch.obj \
        $(SLO)$/tpline.obj \
        $(SLO)$/tplnedef.obj \
        $(SLO)$/tplneend.obj \
        $(SLO)$/tpshadow.obj \
        $(SLO)$/transfrm.obj \


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
