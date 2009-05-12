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
# $Revision: 1.8 $
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
PRJNAME=agg
TARGET=agg
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# don't link default libraries from sal
UWINAPILIB=
LIBSALCPPRT=
# --- Files -------------------------------------

SLOFILES= \
        $(SLO)$/agg_arc.obj						\
        $(SLO)$/agg_arrowhead.obj				\
        $(SLO)$/agg_bezier_arc.obj				\
        $(SLO)$/agg_bspline.obj					\
        $(SLO)$/agg_curves.obj					\
        $(SLO)$/agg_embedded_raster_fonts.obj	\
        $(SLO)$/agg_gsv_text.obj				\
        $(SLO)$/agg_image_filters.obj			\
        $(SLO)$/agg_line_aa_basics.obj			\
        $(SLO)$/agg_line_profile_aa.obj			\
        $(SLO)$/agg_path_storage.obj			\
        $(SLO)$/agg_rasterizer_scanline_aa.obj	\
        $(SLO)$/agg_rounded_rect.obj			\
        $(SLO)$/agg_sqrt_tables.obj				\
        $(SLO)$/agg_trans_affine.obj			\
        $(SLO)$/agg_trans_double_path.obj		\
        $(SLO)$/agg_trans_single_path.obj		\
        $(SLO)$/agg_trans_warp_magnifier.obj	\
        $(SLO)$/agg_vcgen_bspline.obj			\
        $(SLO)$/agg_vcgen_contour.obj			\
        $(SLO)$/agg_vcgen_dash.obj				\
        $(SLO)$/agg_vcgen_markers_term.obj		\
        $(SLO)$/agg_vcgen_smooth_poly1.obj		\
        $(SLO)$/agg_vcgen_stroke.obj			\
        $(SLO)$/agg_vpgen_clip_polygon.obj		\
        $(SLO)$/agg_vpgen_clip_polyline.obj		\
        $(SLO)$/agg_vpgen_segmentator.obj

SHL1TARGET = $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=agg
DEFLIB1NAME	=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk $(TARGET).flt
    @$(TYPE) $(TARGET).flt > $@
