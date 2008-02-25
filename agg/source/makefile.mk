#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:50:55 $
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
