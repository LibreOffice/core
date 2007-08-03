#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2007-08-03 11:24:14 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

