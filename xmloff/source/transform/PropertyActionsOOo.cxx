/*************************************************************************
 *
 *  $RCSfile: PropertyActionsOOo.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:51:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_PROPTYPE_HXX
#include "PropType.hxx"
#endif
#ifndef _XMLOFF_PROPERTYACTIONSOOO_HXX
#include "PropertyActionsOOo.hxx"
#endif

using namespace ::xmloff::token;

#define NO_PARAMS 0, 0, 0

XMLTransformerActionInit aGraphicPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_DRAW, XML_STROKE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_STROKE_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_STROKE_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_MARKER_START_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_MARKER_END_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_MARKER_START_CENTER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_MARKER_END_CENTER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_STROKE_OPACITY, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_STROKE_LINEJOIN, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_DRAW,
                                                 XML_STROKE_LINEJOIN), 0 },
    { XML_NAMESPACE_TEXT, XML_ANIMATION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_ANIMATION_DIRECTION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_ANIMATION_START_INSIDE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_ANIMATION_STOP_INSIDE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_ANIMATION_REPEAT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_ANIMATION_DELAY, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_AUTO_GROW_WIDTH, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_AUTO_GROW_HEIGHT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FIT_TO_SIZE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_TEXTAREA_VERTICAL_ALIGN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_TEXTAREA_HORIZONTAL_ALIGN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_COLOR_MODE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_COLOR_INVERSION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_LUMINANCE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CONTRAST, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_GAMMA, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_RED, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_GREEN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_BLUE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHADOW, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_X, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_Y, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHADOW_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHADOW_TRANSPARENCY,
                XML_ATACTION_RENAME_NEG_PERCENT,
                XMLTransformerActionInit::QNameParam( XML_NAMESPACE_DRAW,
                                                 XML_SHADOW_OPACITY), 0 },
    { XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_HORIZONTAL, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_VERTICAL, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_HORIZONTAL, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_VERTICAL, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_LINE_DISTANCE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_GUIDE_OVERHANG, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_GUIDE_DISTANCE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_START_GUIDE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_END_GUIDE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_PLACING, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_PARALLEL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_MEASURE_ALIGN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_MEASURE_VERTICAL_ALIGN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_UNIT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHOW_UNIT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_DECIMAL_PLACES, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_TYPE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE_TYPE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_GAP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE_DIRECTION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_LINE_LENGTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_FIT_LINE_LENGTH, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_HORIZONTAL_SEGMENTS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_VERTICAL_SEGMENTS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_EDGE_ROUNDING, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_DR3D, XML_EDGE_ROUNDING_MODE, XML_ATACTION_COPY,
//      NO_PARAMS }, /* TODO: does not exist! */
    { XML_NAMESPACE_DR3D, XML_BACK_SCALE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_DEPTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_BACKFACE_CULLING, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_LIGHTING_MODE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_NORMALS_KIND, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_NORMALS_DIRECTION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_DR3D, XML_TEXTURE_GENERATION_MODE_X, XML_ATACTION_COPY,
//      NO_PARAMS }, /* TODO: does not exist! */
//  { XML_NAMESPACE_DR3D, XML_TEXTURE_GENERATION_MODE_Y, XML_ATACTION_COPY,
//      NO_PARAMS }, /* TODO: does not exist! */
//  { XML_NAMESPACE_DR3D, XML_TEXTURE_KIND, XML_ATACTION_COPY,
//      NO_PARAMS }, /* TODO: does not exist! */
//  { XML_NAMESPACE_DR3D, XML_TEXTURE_FILTER, XML_ATACTION_COPY,
//      NO_PARAMS }, /* TODO: does not exist! */
//  { XML_NAMESPACE_DR3D, XML_TEXTURE_MODE, XML_ATACTION_COPY,
//      NO_PARAMS }, /* TODO: does not exist! */
    { XML_NAMESPACE_DR3D, XML_AMBIENT_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_EMISSIVE_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_SPECULAR_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_DIFFUSE_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_SHININESS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_SHADOW, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_REL_WIDTH, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_REL_HEIGHT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MIN_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MIN_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MAX_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MAX_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_INCH2IN_DUPLICATE,
          XML_PROP_TYPE_PARAGRAPH, 0, 0 }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_INCH2IN_DUPLICATE,
          XML_PROP_TYPE_PARAGRAPH, 0, 0 }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_INCH2IN_DUPLICATE,
          XML_PROP_TYPE_PARAGRAPH, 0, 0 }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_INCH2IN_DUPLICATE,
          XML_PROP_TYPE_PARAGRAPH, 0, 0 }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BACKGROUND_TRANSPARENCY, XML_ATACTION_COPY,
          NO_PARAMS },
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INCHS2INS,
          NO_PARAMS },
    { XML_NAMESPACE_STYLE, XML_PRINT_CONTENT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_PROTECT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_HORIZONTAL_POS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_X, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_HORIZONTAL_REL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_VERTICAL_POS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_Y, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_VERTICAL_REL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_ANCHOR_PAGE_NUMBER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_EDITABLE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_WRAP, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_STYLE, XML_WRAP_DYNAMIC_TRESHOLD, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute */
    { XML_NAMESPACE_STYLE, XML_NUMBER_WRAPPED_PARAGRAPHS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_WRAP_CONTOUR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_WRAP_CONTOUR_MODE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_RUN_THROUGH, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FLOW_WITH_TEXT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_STYLE, XML_OVERFLOW_BEHAVIOR, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute */
    { XML_NAMESPACE_STYLE, XML_MIRROR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_CLIP, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FRAME_DISPLAY_SCROLLBAR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FRAME_DISPLAY_BORDER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FRAME_MARGIN_HORIZONTAL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FRAME_MARGIN_VERTICAL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_KEEP_TOGETHER, XML_ATACTION_COPY,
          NO_PARAMS }, /* TODO: This is new on OOo, but not in the OASIS format */
    { XML_NAMESPACE_DRAW, XML_STROKE_DASH, XML_ATACTION_ENCODE_STYLE_NAME_REF,
        NO_PARAMS  },
    { XML_NAMESPACE_DRAW, XML_MARKER_START, XML_ATACTION_ENCODE_STYLE_NAME_REF,
        NO_PARAMS  },
    { XML_NAMESPACE_DRAW, XML_MARKER_END, XML_ATACTION_ENCODE_STYLE_NAME_REF,
        NO_PARAMS  },
    // style-graphic-fill-properties-attlist
    { XML_NAMESPACE_DRAW, XML_FILL, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_COLOR, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME, XML_ATACTION_ENCODE_STYLE_NAME_REF,
        NO_PARAMS  },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME, XML_ATACTION_ENCODE_STYLE_NAME, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME, XML_ATACTION_ENCODE_STYLE_NAME_REF,
        NO_PARAMS  },
    { XML_NAMESPACE_STYLE, XML_REPEAT, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH, XML_ATACTION_INCH2IN, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT, XML_ATACTION_INCH2IN, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */

    // #i25616#
    { XML_NAMESPACE_DRAW, XML_TRANSPARENCY, XML_PTACTION_TRANSPARENCY, NO_PARAMS },

    { XML_NAMESPACE_DRAW, XML_TRANSPARENCY_NAME,
                XML_ATACTION_RENAME_ENCODE_STYLE_NAME_REF,
                XMLTransformerActionInit::QNameParam( XML_NAMESPACE_DRAW,
                                                 XML_OPACITY_NAME), 0 },

    { XML_NAMESPACE_DRAW, XML_MOVE_PROTECT, XML_ATACTION_COPY, NO_PARAMS },
    { XML_NAMESPACE_DRAW, XML_SIZE_PROTECT, XML_ATACTION_COPY, NO_PARAMS },
    { XML_NAMESPACE_DRAW, XML_FIT_TO_CONTOUR, XML_ATACTION_COPY, NO_PARAMS },
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aGraphicPropertyOOoElemActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_BACKGROUND_IMAGE, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_COLUMNS, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_LIST_STYLE, XML_ATACTION_COPY, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aDrawingPagePropertyOOoAttrActionTable[] =
{
    // style-graphic-fill-properties-attlist
    { XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME, XML_ATACTION_ENCODE_STYLE_NAME_REF,
        NO_PARAMS  },
    { XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME, XML_ATACTION_ENCODE_STYLE_NAME, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID, XML_ATACTION_ENCODE_STYLE_NAME, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME, XML_ATACTION_ENCODE_STYLE_NAME_REF,
        NO_PARAMS  },
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH, XML_ATACTION_INCH2IN, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT, XML_ATACTION_INCH2IN, NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_DRAW, XML_OPACITY_NAME, XML_ATACTION_ENCODE_STYLE_NAME_REF,
//      NO_PARAMS  },

    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aPageLayoutPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_PAGE_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PAGE_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INCHS2INS,
          NO_PARAMS },
    { XML_NAMESPACE_STYLE, XML_FOOTNOTE_MAX_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_BASE_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_REGISTER_TRUTH_REF_STYLE_NAME, XML_ATACTION_ENCODE_STYLE_NAME_REF, NO_PARAMS },
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aHeaderFooterPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_SVG, XML_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MIN_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INCHS2INS,
          NO_PARAMS },
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTextPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_FONT_VARIANT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_TEXT_TRANSFORM, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_USE_WINDOW_FONT_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_OUTLINE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_CROSSING_OUT, XML_PTACTION_LINETHROUGH,
          NO_PARAMS }, /* TODO: rename */
    { XML_NAMESPACE_STYLE, XML_TEXT_POSITION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_NAME, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_NAME_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_NAME_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_FONT_FAMILY, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_FAMILY_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_FAMILY_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_FAMILY_GENERIC, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_FAMILY_GENERIC_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_FAMILY_GENERIC_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_STYLE_NAME, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_STYLE_NAME_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_STYLE_NAME_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_PITCH, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_PITCH_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_PITCH_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_CHARSET, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_FONT_SIZE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_ASIAN, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_COMPLEX, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_REL, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_REL_ASIAN, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_REL_COMPLEX, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_STYLE, XML_SCRIPT_TYPE, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute*/
    { XML_NAMESPACE_FO, XML_LETTER_SPACING, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_LANGUAGE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LANGUAGE_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LANGUAGE_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_COUNTRY, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_COUNTRY_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_COUNTRY_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_FONT_STYLE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_STYLE_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_STYLE_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_RELIEF, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_TEXT_SHADOW, XML_ATACTION_INCHS2INS,
          NO_PARAMS },
    { XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE, XML_PTACTION_UNDERLINE,
          NO_PARAMS }, /* TODO: rename */
    { XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_FONT_WEIGHT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_WEIGHT_ASIAN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_WEIGHT_COMPLEX, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_SCORE_SPACES,
        XML_PTACTION_LINE_MODE, NO_PARAMS },
    { XML_NAMESPACE_STYLE, XML_LETTER_KERNING, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_BLINKING, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_BACKGROUND_COLOR, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_FO,
                                                 XML_BACKGROUND_COLOR   ), 0 },
    { XML_NAMESPACE_STYLE, XML_TEXT_COMBINE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_COMBINE_START_CHAR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_COMBINE_END_CHAR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_EMPHASIZE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_SCALE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_ROTATION_ANGLE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_ROTATION_SCALE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */

    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTextPropertyOOoElemActionTable[] =
{
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};



XMLTransformerActionInit aParagraphPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_LINE_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LINE_HEIGHT_AT_LEAST, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LINE_SPACING, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_TEXT_ALIGN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_TEXT_ALIGN_LAST, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_JUSTIFY_SINGLE_WORD, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BREAK_INSIDE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_WIDOWS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_ORPHANS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TAB_STOP_DISTANCE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_HYPHENATE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_HYPHENATION_KEEP, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_HYPHENATION_REMAIN_CHAR_COUNT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_HYPHENATION_PUSH_CHAR_COUNT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_HYPHENATION_LADDER_COUNT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_REGISTER_TRUE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_TEXT_INDENT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_AUTO_TEXT_INDENT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BREAK_BEFORE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BREAK_AFTER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INCHS2INS,
          NO_PARAMS },
    { XML_NAMESPACE_FO, XML_KEEP_WITH_NEXT, XML_PTACTION_KEEP_WITH_NEXT,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_NUMBER_LINES, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_LINE_NUMBER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_VERTICAL_ALIGN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_WRITING_MODE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_STYLE, XML_WRITING_MODE_AUTOMATIC, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute */
    { XML_NAMESPACE_STYLE, XML_SNAP_TO_LAYOUT_GRID, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_PAGE_NUMBER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BACKGROUND_TRANSPARENCY, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_KEEP_TOGETHER, XML_ATACTION_COPY,
          NO_PARAMS }, /* TODO: This is a real bug */
    { XML_NAMESPACE_STYLE, XML_TEXT_AUTOSPACE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_PUNCTUATION_WRAP, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LINE_BREAK, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_ENABLE_NUMBERING, XML_ATACTION_COPY,
          NO_PARAMS }, /* TODO: undocumented*/
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aParagraphPropertyOOoElemActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_TAB_STOPS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DROP_CAP, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BACKGROUND_IMAGE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aSectionPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTablePropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTableColumnPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_COLUMN_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTableRowPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_ROW_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_MIN_ROW_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTableCellPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_VERTICAL_ALIGN, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_STYLE,
                                                 XML_VERTICAL_ALIGN ), 0 },
    { XML_NAMESPACE_FO, XML_TEXT_ALIGN, XML_ATACTION_COPY,
          XML_PROP_TYPE_PARAGRAPH, 0, 0 }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TEXT_ALIGN_SOURCE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_DIRECTION, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_STYLE,
                                                 XML_DIRECTION), 0 },
    { XML_NAMESPACE_STYLE, XML_GLYPH_ORIENTATION_VERTICAL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INCHS2INS,
          NO_PARAMS },
    { XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIAGONAL_BLTR, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIAGONAL_BLTR_WIDTH, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIAGONAL_TLBR, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIAGONAL_TLBR_WIDTH, XML_ATACTION_INCHS2INS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_WRAP_OPTION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_REPEAT_CONTENT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_ROTATION_ANGLE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_ROTATION_ALIGN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_CELL_PROTECT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_PRINT_CONTENT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DECIMAL_PLACES, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTableCellPropertyOOoElemActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_BACKGROUND_IMAGE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aListLevelPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_TEXT, XML_SPACE_BEFORE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_MIN_LABEL_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_MIN_LABEL_DISTANCE, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aChartPropertyOOoAttrActionTable[] =
{
    { XML_NAMESPACE_CHART, XML_SCALE_TEXT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_THREE_DIMENSIONAL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_DEEP, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_CHART, XML_SYMBOL_TYPE, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute */
//  { XML_NAMESPACE_CHART, XML_SYMBOL_TYPE, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute */
//  { XML_NAMESPACE_CHART, XML_SYMBOL_TYPE, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute */
//  { XML_NAMESPACE_CHART, XML_SYMBOL_NAME, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute */
//  { XML_NAMESPACE_CHART, XML_SYMBOL_TYPE, XML_ATACTION_COPY,
//      NO_PARAMS }, /* new attribute */
    { XML_NAMESPACE_XLINK, XML_HREF, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_SYMBOL_WIDTH, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_SYMBOL_HEIGHT, XML_ATACTION_INCH2IN,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_VERTICAL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_CONNECT_BARS, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_GAP_WIDTH, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_OVERLAP, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
//  { XML_NAMESPACE_CHART, XML_JAPANESE_CANDLE_STICK, XML_ATACTION_COPY,
//      NO_PARAMS }, /* TODO: renamed? */
//  { XML_NAMESPACE_CHART, XML_INTERPOLATION, XML_ATACTION_COPY,
//      NO_PARAMS }, /* TODO: renamed? */
    { XML_NAMESPACE_CHART, XML_SPLINE_ORDER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_SPLINE_RESOLUTION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_PIE_OFFSET, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_LINES, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_SOLID_TYPE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_STACKED, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_PERCENTAGE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_LINK_DATA_STYLE_TO_SOURCE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_VISIBLE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_LOGARITHMIC, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_MAXIMUM, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_MINIMUM, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_ORIGIN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_TICK_MARKS_MAJOR_INNER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_TICK_MARKS_MAJOR_OUTER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_TICK_MARKS_MINOR_INNER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_TICK_MARKS_MINOR_OUTER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_DISPLAY_LABEL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_TEXT_OVERLAP, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_LINE_BREAK, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_LABEL_ARRANGEMENT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIRECTION, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_ROTATION_ANGLE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_DATA_LABEL_NUMBER, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_DATA_LABEL_TEXT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_DATA_LABEL_SYMBOL, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_MEAN_VALUE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_ERROR_CATEGORY, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_ERROR_PERCENTAGE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_ERROR_MARGIN, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_ERROR_LOWER_LIMIT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_ERROR_UPPER_LIMIT, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_ERROR_UPPER_INDICATOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_ERROR_LOWER_INDICATOR, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_SERIES_SOURCE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_REGRESSION_TYPE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_DIRECTION, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_STYLE,
                                                 XML_DIRECTION ), 0 },
    { XML_NAMESPACE_CHART, XML_SPLINES, XML_PTACTION_SPLINES, NO_PARAMS },
    { XML_NAMESPACE_TEXT, XML_ROTATION_ANGLE, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_STYLE,
                                                 XML_ROTATION_ANGLE ), 0 },
    { XML_NAMESPACE_CHART, XML_INTERVAL_MAJOR, XML_PTACTION_INTERVAL_MAJOR, NO_PARAMS },
    { XML_NAMESPACE_CHART, XML_INTERVAL_MINOR, XML_PTACTION_INTERVAL_MINOR, NO_PARAMS },

    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aChartPropertyOOoElemActionTable[] =
{
    { XML_NAMESPACE_CHART, XML_SYMBOL_IMAGE, XML_ATACTION_COPY,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

