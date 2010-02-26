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

DIR_FILTERCFGOUT := $(MISC)$/filters
.IF "$(WITH_LANG)"!=""
DIR_LOCFRAG      := $(DIR_FILTERCFGOUT)$/ui$/fragments
.ELSE			# "$(WITH_LANG)"!=""
DIR_LOCFRAG      := $(PRJ)$/source$/config$/fragments
.ENDIF			# "$(WITH_LANG)"!=""
DIR_LOCMERGE     := $(DIR_FILTERCFGOUT)$/ui$/merge
DIR_LANGPACK     := $(DIR_FILTERCFGOUT)$/ui$/langpacks
DIR_MODPACKS     := $(DIR_FILTERCFGOUT)$/modulepacks

INCLUDE_FRAGMENTS = \
     $(PRJ)$/source$/config$/fragments$/fcfg_base.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_database.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_writer.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_web.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_global.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_w4w.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_calc.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_draw.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_impress.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_database.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_chart.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_math.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_xslt.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_palm.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_pocketword.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_pocketexcel.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_drawgraphics.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_impressgraphics.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_internalgraphics.mk

.IF "$(WITH_BINFILTER)" != "NO"
INCLUDE_FRAGMENTS += \
     $(PRJ)$/source$/config$/fragments$/fcfg_writer_bf.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_web_bf.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_global_bf.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_calc_bf.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_draw_bf.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_impress_bf.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_chart_bf.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_math_bf.mk
.ENDIF

.INCLUDE: $(INCLUDE_FRAGMENTS)
