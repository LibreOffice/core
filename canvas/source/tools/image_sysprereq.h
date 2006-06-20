/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: image_sysprereq.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:17:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#if defined __GNUC__
#pragma GCC system_header
#elif defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#endif

#include <agg2/agg_rendering_buffer.h>

#ifdef CANVAS_IMAGE_CXX

//////////////////////////////////////////////////////////////////////////////////
// includes from antigrain [i would love to write this stuff by myself]
//////////////////////////////////////////////////////////////////////////////////

#include <agg2/agg_rendering_buffer.h>
#include <agg2/agg_pixfmt_rgb.h>
#include <agg2/agg_pixfmt_rgba.h>
#include <agg2/agg_renderer_base.h>
#include <agg2/agg_color_rgba.h>
#include <agg2/agg_rasterizer_outline_aa.h>
#include <agg2/agg_rasterizer_scanline_aa.h>
#include <agg2/agg_scanline_p.h>
#include <agg2/agg_scanline_u.h>
#include <agg2/agg_renderer_scanline.h>
#include <agg2/agg_renderer_outline_aa.h>
#include <agg2/agg_renderer_primitives.h>
#include <agg2/agg_path_storage.h>
#include <agg2/agg_span_pattern.h>
#include <agg2/agg_span_pattern_rgba.h>
#include <agg2/agg_span_pattern_resample_rgb.h>
#include <agg2/agg_span_pattern_resample_rgba.h>
#include <agg2/agg_span_interpolator_linear.h>
#include <agg2/agg_span_gradient.h>
#include <agg2/agg_span_image_resample_rgb.h>
#include <agg2/agg_span_image_resample_rgba.h>
#include <agg2/agg_image_filters.h>
#include <agg2/agg_dda_line.h>
#include <agg2/agg_scanline_storage_aa.h>
#include <agg2/agg_scanline_storage_bin.h>
#include <agg2/agg_scanline_bin.h>
#include <agg2/agg_path_storage_integer.h>
#include <agg2/agg_conv_contour.h>
#include <agg2/agg_conv_curve.h>
#include <agg2/agg_conv_stroke.h>
#include <agg2/agg_conv_transform.h>
#include <agg2/agg_trans_affine.h>
#include <agg2/agg_font_cache_manager.h>
#include <agg2/agg_bitset_iterator.h>
#include <agg2/agg_path_storage.h>

#endif  // CANVAS_IMAGE_CXX

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif

