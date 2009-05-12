/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: image_sysprereq.h,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
#if AGG_VERSION == 2300
#include <agg2/agg_span_pattern.h>
#endif
#include <agg2/agg_span_pattern_rgba.h>
#if AGG_VERSION >= 2400
#include <agg2/agg_span_image_filter_rgb.h>
#include <agg2/agg_span_image_filter_rgba.h>
#else
#include <agg2/agg_span_pattern_resample_rgb.h>
#include <agg2/agg_span_pattern_resample_rgba.h>
#endif
#include <agg2/agg_span_interpolator_linear.h>
#include <agg2/agg_span_gradient.h>
#if AGG_VERSION == 2300
#include <agg2/agg_span_image_resample_rgb.h>
#include <agg2/agg_span_image_resample_rgba.h>
#endif
#if AGG_VERSION >= 2400
#include <agg2/agg_span_allocator.h>
#endif
#include <agg2/agg_image_filters.h>
#if AGG_VERSION >= 2400
#include <agg2/agg_image_accessors.h>
#endif
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

