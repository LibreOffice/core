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
PROJECTPCHSOURCE=$(PRJ)$/util\sd
PRJNAME=sd
TARGET=func
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES = \
        $(SLO)$/fuformatpaintbrush.obj	\
        $(SLO)$/fuhhconv.obj	\
        $(SLO)$/undoheaderfooter.obj    \
        $(SLO)$/undolayer.obj	\
        $(SLO)$/fupoor.obj		\
        $(SLO)$/fudraw.obj		\
        $(SLO)$/fuzoom.obj		\
        $(SLO)$/fusel.obj		\
        $(SLO)$/fuconstr.obj 	\
        $(SLO)$/fuconrec.obj 	\
        $(SLO)$/fuconuno.obj 	\
        $(SLO)$/fuconbez.obj 	\
        $(SLO)$/fuediglu.obj 	\
        $(SLO)$/fusldlg.obj 	\
        $(SLO)$/fuscale.obj		\
        $(SLO)$/futransf.obj 	\
        $(SLO)$/futext.obj		\
        $(SLO)$/fuline.obj		\
        $(SLO)$/sdundogr.obj 	\
        $(SLO)$/fuoaprms.obj 	\
        $(SLO)$/unoaprms.obj 	\
        $(SLO)$/fuarea.obj		\
        $(SLO)$/fuchar.obj		\
        $(SLO)$/fuconarc.obj 	\
        $(SLO)$/fuparagr.obj 	\
        $(SLO)$/fubullet.obj 	\
        $(SLO)$/futempl.obj		\
        $(SLO)$/fuoutl.obj		\
        $(SLO)$/fuoltext.obj 	\
        $(SLO)$/fuinsert.obj 	\
        $(SLO)$/fupage.obj		\
        $(SLO)$/undopage.obj 	\
        $(SLO)$/fuprlout.obj 	\
        $(SLO)$/fuprobjs.obj 	\
        $(SLO)$/fulinend.obj 	\
        $(SLO)$/fusnapln.obj 	\
        $(SLO)$/fuolbull.obj 	\
        $(SLO)$/fucopy.obj		\
        $(SLO)$/fulink.obj		\
        $(SLO)$/futhes.obj		\
        $(SLO)$/fusearch.obj 	\
        $(SLO)$/fuinsfil.obj 	\
        $(SLO)$/futxtatt.obj 	\
        $(SLO)$/fumeasur.obj 	\
        $(SLO)$/fuconnct.obj 	\
        $(SLO)$/unprlout.obj 	\
        $(SLO)$/fudspord.obj 	\
        $(SLO)$/unmovss.obj		\
        $(SLO)$/fucon3d.obj		\
        $(SLO)$/fumorph.obj		\
        $(SLO)$/fuexpand.obj    \
        $(SLO)$/fusumry.obj     \
        $(SLO)$/fucushow.obj	\
        $(SLO)$/fuvect.obj		\
        $(SLO)$/bulmaper.obj    \
        $(SLO)$/undoback.obj    \
        $(SLO)$/fuconcs.obj		\
        $(SLO)$/smarttag.obj

.IF "$(GUI)" == "WNT"

NOOPTFILES=\
    $(SLO)$/futext.obj

.ENDIF # wnt

LIB1TARGET=	$(SLB)$/$(TARGET).lib
LIB1OBJFILES= \
        $(SLO)$/fuformatpaintbrush.obj	\
        $(SLO)$/fuhhconv.obj	\
        $(SLO)$/undoheaderfooter.obj    \
        $(SLO)$/undolayer.obj	\
        $(SLO)$/fupoor.obj		\
        $(SLO)$/fudraw.obj		\
        $(SLO)$/fuzoom.obj		\
        $(SLO)$/fusel.obj		\
        $(SLO)$/fuconstr.obj 	\
        $(SLO)$/fuconrec.obj 	\
        $(SLO)$/fuconuno.obj 	\
        $(SLO)$/fuconbez.obj 	\
        $(SLO)$/fuediglu.obj 	\
        $(SLO)$/fusldlg.obj 	\
        $(SLO)$/fuscale.obj		\
        $(SLO)$/futransf.obj 	\
        $(SLO)$/futext.obj		\
        $(SLO)$/fuline.obj		\
        $(SLO)$/sdundogr.obj 	\
        $(SLO)$/fuoaprms.obj 	\
        $(SLO)$/unoaprms.obj 	\
        $(SLO)$/fuarea.obj		\
        $(SLO)$/fuchar.obj		\
        $(SLO)$/fuconarc.obj 	\
        $(SLO)$/fuparagr.obj 	\
        $(SLO)$/fubullet.obj 	\
        $(SLO)$/futempl.obj		\
        $(SLO)$/fuoutl.obj		\
        $(SLO)$/fuoltext.obj	\
        $(SLO)$/smarttag.obj
            
LIB3TARGET=	$(SLB)$/$(TARGET)_2.lib
LIB3OBJFILES= \
        $(SLO)$/fuinsert.obj 	\
        $(SLO)$/fupage.obj		\
        $(SLO)$/undopage.obj 	\
        $(SLO)$/fuprlout.obj 	\
        $(SLO)$/fuprobjs.obj 	\
        $(SLO)$/fulinend.obj 	\
        $(SLO)$/fusnapln.obj 	\
        $(SLO)$/fuolbull.obj 	\
        $(SLO)$/fucopy.obj		\
        $(SLO)$/fulink.obj		\
        $(SLO)$/futhes.obj		\
        $(SLO)$/fusearch.obj 	\
        $(SLO)$/fuinsfil.obj 	\
        $(SLO)$/futxtatt.obj 	\
        $(SLO)$/fumeasur.obj 	\
        $(SLO)$/fuconnct.obj 	\
        $(SLO)$/unprlout.obj 	\
        $(SLO)$/fudspord.obj 	\
        $(SLO)$/unmovss.obj		\
        $(SLO)$/fucon3d.obj		\
        $(SLO)$/fumorph.obj		\
        $(SLO)$/fuexpand.obj    \
        $(SLO)$/fusumry.obj     \
        $(SLO)$/fucushow.obj	\
        $(SLO)$/fuvect.obj		\
        $(SLO)$/bulmaper.obj    \
        $(SLO)$/undoback.obj    \
        $(SLO)$/fuconcs.obj

LIB2TARGET=	$(SLB)$/$(TARGET)_ui.lib
LIB2OBJFILES= \
        $(SLO)$/bulmaper.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
