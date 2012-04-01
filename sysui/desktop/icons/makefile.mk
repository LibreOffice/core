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

PRJ=..$/..

PRJNAME=sysui
TARGET=icons

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Targets -------------------------------------------------------------

all: \
    $(MISC)$/oasis-database.ico \
    $(MISC)$/oasis-drawing-template.ico \
    $(MISC)$/oasis-drawing.ico \
    $(MISC)$/oasis-empty-template.ico \
    $(MISC)$/oasis-empty-document.ico \
    $(MISC)$/oasis-formula.ico \
    $(MISC)$/oasis-master-document.ico \
    $(MISC)$/oasis-presentation-template.ico \
    $(MISC)$/oasis-presentation.ico \
    $(MISC)$/oasis-spreadsheet-template.ico \
    $(MISC)$/oasis-spreadsheet.ico \
    $(MISC)$/oasis-text-template.ico \
    $(MISC)$/oasis-text.ico \
    $(MISC)$/oasis-web-template.ico \
    $(MISC)$/database.ico \
    $(MISC)$/drawing-template.ico \
    $(MISC)$/drawing.ico \
    $(MISC)$/empty-template.ico \
    $(MISC)$/empty-document.ico \
    $(MISC)$/formula.ico \
    $(MISC)$/master-document.ico \
    $(MISC)$/presentation-template.ico \
    $(MISC)$/presentation.ico \
    $(MISC)$/spreadsheet-template.ico \
    $(MISC)$/spreadsheet.ico \
    $(MISC)$/text-template.ico \
    $(MISC)$/text.ico \
    $(MISC)$/impress_app.ico \
    $(MISC)$/calc_app.ico \
    $(MISC)$/draw_app.ico \
    $(MISC)$/base_app.ico \
    $(MISC)$/writer_app.ico \
    $(MISC)$/math_app.ico \
    $(MISC)$/main_app.ico \
    $(MISC)$/open.ico \
    $(MISC)$/oxt-extension.ico

$(MISC)$/%.ico: %.ico
    $(COPY) $< $@

.INCLUDE :  target.mk
