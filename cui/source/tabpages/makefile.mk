#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.65 $
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
        grfpage.src \
        macroass.src \
        measure.src \
        numfmt.src \
        numpages.src \
        page.src \
        paragrph.src \
        swpossizetabpage.src \
        tabstpge.src \
        textanim.src \
        textattr.src \
        labdlg.src \
        tabarea.src \
        tabline.src \
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
