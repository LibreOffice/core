#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: packagedef.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2005-11-11 14:08:09 $
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

DIR_FILTERCFGOUT := $(MISC)$/filters 
DIR_LOCFRAG      := $(DIR_FILTERCFGOUT)$/ui$/fragments
DIR_LOCMERGE     := $(DIR_FILTERCFGOUT)$/ui$/merge
DIR_LANGPACK     := $(DIR_FILTERCFGOUT)$/ui$/langpacks
DIR_MODPACKS     := $(DIR_FILTERCFGOUT)$/modulepacks

.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_base.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_writer.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_web.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_global.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_w4w.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_calc.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_draw.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_impress.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_database.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_chart.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_math.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_xslt.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_palm.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_pocketword.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_pocketexcel.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_drawgraphics.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_impressgraphics.mk
.INCLUDE: $(PRJ)$/source$/config$/fragments$/fcfg_internalgraphics.mk

