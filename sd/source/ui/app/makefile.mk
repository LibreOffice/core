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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=app

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

NO_HID_FILES=app.src
.EXPORT : NO_HID_FILES

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	\
        app.src 			\
        toolbox.src			\
        strings.src 		\
        res_bmp.src 		\
        popup.src			\
        sdstring.src

#OBJFILES = \
#		$(OBJ)$/sdlib.obj	\
#		$(OBJ)$/sdresid.obj	\

SLOFILES =	\
        $(SLO)$/sdmod.obj		\
        $(SLO)$/sdmod1.obj		\
        $(SLO)$/sdmod2.obj		\
        $(SLO)$/sddll.obj		\
        $(SLO)$/sddll1.obj      \
        $(SLO)$/sddll2.obj      \
        $(SLO)$/tbxww.obj		\
        $(SLO)$/optsitem.obj	\
        $(SLO)$/sdresid.obj		\
        $(SLO)$/sdpopup.obj		\
        $(SLO)$/sdxfer.obj		\
        $(SLO)$/tmplctrl.obj

EXCEPTIONSFILES= \
        $(SLO)$/sddll1.obj      \
        $(SLO)$/sddll2.obj      \
        $(SLO)$/sdxfer.obj		\
        $(SLO)$/sdmod1.obj		\
        $(SLO)$/tbxww.obj		\
        $(SLO)$/tmplctrl.obj

.IF "$(GUI)" == "WNT"

NOOPTFILES=\
    $(SLO)$/optsitem.obj

.ENDIF

# --- Tagets -------------------------------------------------------

all: \
    $(INCCOM)$/sddll0.hxx   \
    ALLTAR

$(INCCOM)$/sddll0.hxx: makefile.mk
    @echo $@
.IF "$(GUI)"=="UNX"
    echo \#define DLL_NAME \"libsd$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE			# "$(GUI)"=="UNX"
    echo \#define DLL_NAME \"sd$(DLLPOSTFIX).DLL\" >$@
.ENDIF			# "$(GUI)"=="UNX"

LOCALIZE_ME =  tbxids_tmpl.src menuids2_tmpl.src menu_tmpl.src menuids_tmpl.src menuids4_tmpl.src popup2_tmpl.src toolbox2_tmpl.src menuportal_tmpl.src menuids3_tmpl.src


.INCLUDE :  target.mk

$(SRS)$/app.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


$(SRS)$/$(SRS1NAME).srs  : $(LOCALIZE_ME_DEST)

