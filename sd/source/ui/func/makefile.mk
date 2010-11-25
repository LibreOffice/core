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
        $(SLO)$/sdundo.obj		\
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
        $(SLO)$/sdundo.obj		\
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
