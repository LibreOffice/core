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

PRJ=..

PRJNAME=desktop
TARGET=zipintro

.INCLUDE :  settings.mk

ZIP1LIST= \
    $(MISC)$/$(RSCDEFIMG)$/brand$/intro.png \
    $(MISC)$/$(RSCDEFIMG)$/brand$/about.png
ZIP2LIST= \
    $(MISC)$/$(RSCDEFIMG)$/brand_dev$/intro.png \
    $(MISC)$/$(RSCDEFIMG)$/brand_dev$/about.png
ZIP3LIST= \
    $(MISC)$/$(RSCDEFIMG)$/brand$/shell$/backing_left.png \
    $(MISC)$/$(RSCDEFIMG)$/brand$/shell$/backing_right.png \
    $(MISC)$/$(RSCDEFIMG)$/brand$/shell$/backing_space.png \
    $(MISC)$/$(RSCDEFIMG)$/brand$/shell$/backing_rtl_left.png \
    $(MISC)$/$(RSCDEFIMG)$/brand$/shell$/backing_rtl_right.png

.IF "$(ENABLE_BROFFICE)"=="TRUE"
ZIP1LIST+= \
    $(MISC)$/$(RSCDEFIMG)$/brand$/intro-pt_BR.png  \
    $(MISC)$/$(RSCDEFIMG)$/brand$/about-pt_BR.png
ZIP2LIST+= \
    $(MISC)$/$(RSCDEFIMG)$/brand_dev$/intro-pt_BR.png  \
    $(MISC)$/$(RSCDEFIMG)$/brand_dev$/about-pt_BR.png
ZIP3LIST+= \
    $(MISC)$/$(RSCDEFIMG)$/brand$/shell$/backing_left-pt_BR.png \
    $(MISC)$/$(RSCDEFIMG)$/brand$/shell$/backing_right-pt_BR.png
.ENDIF # ENABLE_BROFFICE

ZIP1TARGET=brand
ZIP1DEPS=$(ZIP1LIST)
ZIP2TARGET=brand_dev
ZIP2DEPS=$(ZIP2LIST)
ZIP3TARGET=shell
ZIP3DEPS=$(ZIP3LIST)

.INCLUDE :  target.mk

ALLTAR : \
    $(COMMONBIN)$/brand$/intro.zip \
    $(COMMONBIN)$/brand_dev$/intro.zip \
    $(COMMONBIN)$/shell/shell.zip

$(COMMONBIN)$/brand_dev$/intro.zip : $(COMMONBIN)$/brand_dev.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/brand$/intro.zip : $(COMMONBIN)$/brand.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/shell$/shell.zip : $(COMMONBIN)$/shell.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(MISC)$/%.bmp : $(SOLARSRC)$/%.bmp
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(MISC)$/%.png : $(SOLARSRC)$/%.png
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

.IF "$(INTRO_BITMAP)" != ""
$(MISC)$/$(RSCDEFIMG)$/brand$/intro.png : $(INTRO_BITMAP)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
.ENDIF

.IF "$(ABOUT_BITMAP)" != ""
$(MISC)$/$(RSCDEFIMG)$/brand$/about.png : $(ABOUT_BITMAP)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
.ENDIF
