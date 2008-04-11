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
# $Revision: 1.5 $
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

PRJ=..

PRJNAME=desktop
TARGET=zipintro
# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

DEFAULT_FLAVOURS=dev dev_nologo nologo broffice dev_broffice nologo_broffice nologo_dev_broffice intro

ZIP1LIST= \
    $(null,$(INTRO_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/dev$/introabout$/intro.bmp $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(SOLARSRC)$/$(RSCDEFIMG)$/introabout$/about.bmp $(ABOUT_BITMAPS))
ZIP2LIST= \
    $(null,$(INTRO_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/dev_nologo$/introabout$/intro.bmp $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(SOLARSRC)$/$(RSCDEFIMG)$/introabout$/about.bmp $(ABOUT_BITMAPS))
ZIP3LIST= \
    $(null,$(INTRO_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/nologo$/introabout$/intro.bmp $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(SOLARSRC)$/$(RSCDEFIMG)$/introabout$/about.bmp $(ABOUT_BITMAPS))
ZIP4LIST= \
    $(null,$(INTRO_BITMAPS) $(SOLARSRC)$/$(RSCDEFIMG)$/introabout$/intro.bmp $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(SOLARSRC)$/$(RSCDEFIMG)$/introabout$/about.bmp $(ABOUT_BITMAPS))
ZIP5LIST= \
    $(null,$(INTRO_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/dev_broffice$/introabout$/intro.bmp $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/broffice$/introabout$/about.bmp $(ABOUT_BITMAPS))
ZIP6LIST= \
    $(null,$(INTRO_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/broffice$/introabout$/intro.bmp $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/broffice$/introabout$/about.bmp $(ABOUT_BITMAPS))
ZIP7LIST= \
        $(null,$(INTRO_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/nologo_broffice$/introabout$/intro.bmp $(INTRO_BITMAPS)) \
        $(null,$(ABOUT_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/broffice$/introabout$/about.bmp $(ABOUT_BITMAPS))
ZIP8LIST= \
        $(null,$(INTRO_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/dev_nologo_broffice$/introabout$/intro.bmp $(INTRO_BITMAPS)) \
        $(null,$(ABOUT_BITMAPS) $(SOLARSRC)$/ooo_custom_images$/broffice$/introabout$/about.bmp $(ABOUT_BITMAPS))

ZIP1TARGET=dev_intro

ZIP2TARGET=dev_nologo_intro

ZIP3TARGET=nologo_intro

ZIP4TARGET=intro_intro

ZIP5TARGET=dev_broffice_intro

ZIP6TARGET=broffice_intro

ZIP7TARGET=nologo_broffice_intro

ZIP8TARGET=nologo_dev_broffice_intro

.INCLUDE :  target.mk

ALLTAR : $(foreach,i,$(DEFAULT_FLAVOURS) $(COMMONBIN)$/$i$/intro.zip)

# now duplicate for deliver...
# Because of issue 78837 we cannot use a % rule here (Commented out below)
# but have to write individual rules.
#$(COMMONBIN)$/%$/intro.zip : $(COMMONBIN)$/%_intro.zip

$(COMMONBIN)$/dev$/intro.zip : $(COMMONBIN)$/dev_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/dev_nologo$/intro.zip : $(COMMONBIN)$/dev_nologo_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/nologo$/intro.zip : $(COMMONBIN)$/nologo_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/broffice$/intro.zip : $(COMMONBIN)$/broffice_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/dev_broffice$/intro.zip : $(COMMONBIN)$/dev_broffice_intro.zip
        @@-$(MKDIR) $(@:d)
        @$(COPY) $< $@

$(COMMONBIN)$/nologo_broffice$/intro.zip : $(COMMONBIN)$/nologo_broffice_intro.zip
        @@-$(MKDIR) $(@:d)
        @$(COPY) $< $@

$(COMMONBIN)$/nologo_dev_broffice$/intro.zip : $(COMMONBIN)$/nologo_dev_broffice_intro.zip
        @@-$(MKDIR) $(@:d)
        @$(COPY) $< $@

$(COMMONBIN)$/intro$/intro.zip : $(COMMONBIN)$/intro_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

