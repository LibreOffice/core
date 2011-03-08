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

PRJNAME=sc
TARGET=vbaobj
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF

CDEFS+=-DVBA_OOBUILD_HACK

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
        $(SLO)$/vbacharttitle.obj \
        $(SLO)$/vbacomment.obj \
        $(SLO)$/vbacomments.obj \
        $(SLO)$/vbacommentshape.obj \
        $(SLO)$/vbacondition.obj \
        $(SLO)$/vbadialog.obj \
        $(SLO)$/vbadialogs.obj \
        $(SLO)$/vbaeventshelper.obj \
        $(SLO)$/vbafiledialog.obj \
        $(SLO)$/vbafiledialogselecteditems.obj \
        $(SLO)$/vbafilesearch.obj \
        $(SLO)$/vbafont.obj \
        $(SLO)$/vbaformat.obj \
        $(SLO)$/vbaformatcondition.obj \
        $(SLO)$/vbaformatconditions.obj \
        $(SLO)$/vbafoundfiles.obj \
        $(SLO)$/vbaglobals.obj \
        $(SLO)$/vbahyperlink.obj \
        $(SLO)$/vbahyperlinks.obj \
        $(SLO)$/vbainterior.obj \
        $(SLO)$/vbamenubar.obj  \
        $(SLO)$/vbamenubars.obj \
        $(SLO)$/vbamenu.obj \
        $(SLO)$/vbamenus.obj \
        $(SLO)$/vbamenuitem.obj \
        $(SLO)$/vbamenuitems.obj \
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
        $(SLO)$/vbaquerytable.obj \
        $(SLO)$/vbarange.obj \
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

.IF "$(L10N_framework)"==""

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

.ENDIF
