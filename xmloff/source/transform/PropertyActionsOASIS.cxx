/*************************************************************************
 *
 *  $RCSfile: PropertyActionsOASIS.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:34:07 $
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
#ifndef _XMLOFF_FAMILYTYPE_HXX
#include "FamilyType.hxx"
#endif
#ifndef _XMLOFF_PROPERTYACTIONSOASIS_HXX
#include "PropertyActionsOASIS.hxx"
#endif

using namespace ::xmloff::token;

#define NO_PARAMS 0, 0, 0

XMLTransformerActionInit aGraphicPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_SVG, XML_STROKE_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_MARKER_START_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_MARKER_END_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_X, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_Y, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_SHADOW_OPACITY,
                XML_ATACTION_RENAME_NEG_PERCENT,
                XMLTransformerActionInit::QNameParam( XML_NAMESPACE_DRAW,
                                                 XML_SHADOW_TRANSPARENCY), 0 },
    { XML_NAMESPACE_DRAW, XML_GUIDE_OVERHANG, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_START_GUIDE, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_END_GUIDE, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_LINE_LENGTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DR3D, XML_DEPTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MAX_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MAX_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INS2INCHS,
          NO_PARAMS },
//  { XML_NAMESPACE_STYLE, XML_WRAP_DYNAMIC_TRESHOLD, XML_ATACTION_REMOVE,
//      NO_PARAMS }, /* generated entry */ // TODO
    { XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_HORIZONTAL, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_VERTICAL, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_HORIZONTAL, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_VERTICAL, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_LINE_DISTANCE, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_GUIDE_DISTANCE, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_CAPTION_GAP, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_X, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_SVG, XML_Y, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_STROKE_DASH, XML_ATACTION_DECODE_STYLE_NAME_REF,
        XML_FAMILY_TYPE_STROKE_DASH, 0 },
    { XML_NAMESPACE_DRAW, XML_MARKER_START, XML_ATACTION_DECODE_STYLE_NAME_REF,
        XML_FAMILY_TYPE_MARKER, 0 },
    { XML_NAMESPACE_DRAW, XML_MARKER_END, XML_ATACTION_DECODE_STYLE_NAME_REF,
        XML_FAMILY_TYPE_MARKER, 0 },
    { XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME, XML_ATACTION_DECODE_STYLE_NAME_REF,
        XML_FAMILY_TYPE_GRADIENT, 0 },

    // #i25616#
    { XML_NAMESPACE_DRAW, XML_OPACITY, XML_OPTACTION_OPACITY, NO_PARAMS },
    { XML_NAMESPACE_DRAW, XML_IMAGE_OPACITY, XML_OPTACTION_IMAGE_OPACITY, NO_PARAMS },

    { XML_NAMESPACE_DRAW, XML_STROKE_LINEJOIN, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_SVG,
                                                 XML_STROKE_LINEJOIN), 0 },
    { XML_NAMESPACE_DRAW, XML_OPACITY_NAME,
                XML_ATACTION_RENAME_DECODE_STYLE_NAME_REF,
                XMLTransformerActionInit::QNameParam( XML_NAMESPACE_DRAW,
                                                 XML_TRANSPARENCY_NAME),
                XML_FAMILY_TYPE_GRADIENT },
    { XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME, XML_ATACTION_DECODE_STYLE_NAME_REF,
        XML_FAMILY_TYPE_HATCH, 0 },
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME, XML_ATACTION_DECODE_STYLE_NAME_REF,
        XML_FAMILY_TYPE_FILL_IMAGE, 0 },
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aDrawingPagePropertyOASISAttrActionTable[] =
{
    // style-graphic-fill-properties-attlist
    { XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME, XML_ATACTION_DECODE_STYLE_NAME_REF,
        NO_PARAMS  },
    { XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME, XML_ATACTION_DECODE_STYLE_NAME, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID, XML_ATACTION_DECODE_STYLE_NAME, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME, XML_ATACTION_DECODE_STYLE_NAME_REF,
        NO_PARAMS  },
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT, XML_ATACTION_IN2INCH, NO_PARAMS }, /* generated entry */

    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aPageLayoutPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_PAGE_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PAGE_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INS2INCHS,
          NO_PARAMS },
    { XML_NAMESPACE_STYLE, XML_FOOTNOTE_MAX_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_BASE_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_REGISTER_TRUTH_REF_STYLE_NAME, XML_ATACTION_DECODE_STYLE_NAME_REF, XML_FAMILY_TYPE_PARAGRAPH, 0 },
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aHeaderFooterPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_SVG, XML_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MIN_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INS2INCHS,
          NO_PARAMS },
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTextPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_FONT_SIZE, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_ASIAN, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_COMPLEX, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_REL, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_REL_ASIAN, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_FONT_SIZE_REL_COMPLEX, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_LETTER_SPACING, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_TEXT_SHADOW, XML_ATACTION_INS2INCHS,
          NO_PARAMS },
    { XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_TYPE,
        XML_OPTACTION_UNDERLINE_TYPE, NO_PARAMS }, /* new attribute */
    { XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_STYLE,
        XML_OPTACTION_UNDERLINE_STYLE, NO_PARAMS }, /* new attribute */
    { XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_WIDTH,
        XML_OPTACTION_UNDERLINE_WIDTH, NO_PARAMS }, /* new attribute */
    { XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_MODE,
        XML_OPTACTION_LINE_MODE, NO_PARAMS },
    { XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_TYPE,
        XML_OPTACTION_LINETHROUGH_TYPE, NO_PARAMS }, /* new entry*/
    { XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_STYLE,
        XML_OPTACTION_LINETHROUGH_STYLE, NO_PARAMS }, /* new entry*/
    { XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_WIDTH,
        XML_OPTACTION_LINETHROUGH_WIDTH, NO_PARAMS }, /* new entry*/
    { XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_COLOR,
        XML_ATACTION_REMOVE, NO_PARAMS }, /* new entry*/
    { XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_TEXT,
        XML_OPTACTION_LINETHROUGH_TEXT, NO_PARAMS }, /* new entry*/
    { XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_TEXT_STYLE,
        XML_ATACTION_REMOVE, NO_PARAMS }, /* new entry*/
    { XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_MODE,
        XML_OPTACTION_LINE_MODE, NO_PARAMS },
    { XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_STYLE,
                                                 XML_TEXT_BACKGROUND_COLOR  ), 0 },
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aParagraphPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_LINE_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LINE_HEIGHT_AT_LEAST, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_LINE_SPACING, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_TAB_STOP_DISTANCE, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_TEXT_INDENT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INS2INCHS,
          NO_PARAMS },
    { XML_NAMESPACE_FO, XML_KEEP_WITH_NEXT, XML_OPTACTION_KEEP_WITH_NEXT,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aSectionPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTablePropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTableColumnPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_COLUMN_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTableRowPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_ROW_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_MIN_ROW_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aTableCellPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_STYLE, XML_VERTICAL_ALIGN, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_FO,
                                                 XML_VERTICAL_ALIGN ), 0 },
    { XML_NAMESPACE_FO, XML_BORDER, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIAGONAL_BLTR, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIAGONAL_BLTR_WIDTH, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIAGONAL_TLBR, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIAGONAL_TLBR_WIDTH, XML_ATACTION_INS2INCHS,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_TOP, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIRECTION, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_FO,
                                                 XML_DIRECTION), 0 },
    { XML_NAMESPACE_STYLE, XML_SHADOW, XML_ATACTION_INS2INCHS,
          NO_PARAMS },
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aListLevelPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_TEXT, XML_SPACE_BEFORE, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_MIN_LABEL_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_TEXT, XML_MIN_LABEL_DISTANCE, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_FO, XML_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};

XMLTransformerActionInit aChartPropertyOASISAttrActionTable[] =
{
    { XML_NAMESPACE_CHART, XML_SYMBOL_WIDTH, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_CHART, XML_SYMBOL_HEIGHT, XML_ATACTION_IN2INCH,
          NO_PARAMS }, /* generated entry */
    { XML_NAMESPACE_STYLE, XML_DIRECTION, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_FO,
                                                 XML_DIRECTION ), 0 },
    { XML_NAMESPACE_CHART, XML_INTERPOLATION, XML_OPTACTION_INTERPOLATION, NO_PARAMS },
    { XML_NAMESPACE_STYLE, XML_ROTATION_ANGLE, XML_ATACTION_RENAME,
        XMLTransformerActionInit::QNameParam( XML_NAMESPACE_TEXT,
                                                 XML_ROTATION_ANGLE ), 0 },
    { XML_NAMESPACE_CHART, XML_INTERVAL_MAJOR, XML_OPTACTION_INTERVAL_MAJOR, NO_PARAMS },
    { XML_NAMESPACE_CHART, XML_INTERVAL_MINOR_DIVISOR, XML_OPTACTION_INTERVAL_MINOR_DIVISOR,
      NO_PARAMS },

    { XML_NAMESPACE_OFFICE, XML_TOKEN_INVALID, XML_ATACTION_EOT, NO_PARAMS }
};
