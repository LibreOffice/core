/*************************************************************************
 *
 *  $RCSfile: xmlitemm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:15:00 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <hintids.hxx>

#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLITMAP_HXX
#include <xmloff/xmlitmap.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _UNOMID_H
#include <unomid.h>
#endif

#define M_E( p, l, w, m ) \
    { XML_NAMESPACE_##p, sXML_##l, w, m }
#define M_E_SI( p, l, w, m ) \
    { XML_NAMESPACE_##p, sXML_##l, w, MID_FLAG_SPECIAL_ITEM_IMPORT|m }
#define M_E_SE( p, l, w, m ) \
    { XML_NAMESPACE_##p, sXML_##l, w, MID_FLAG_SPECIAL_ITEM_EXPORT|m }

#ifdef XML_CORE_API
SvXMLItemMapEntry aXMLParaItemMap[] =
{
    // RES_CHRATR_CASEMAP
    M_E( FO,    font_variant,       RES_CHRATR_CASEMAP, MID_CASEMAP_VARIANT ),
    M_E( FO,    text_transform,     RES_CHRATR_CASEMAP, MID_CASEMAP_TRANS ),
    // RES_CHRATR_COLOR
    M_E( FO,    color,              RES_CHRATR_COLOR,   0 ),
    // RES_CHRATR_CONTOUR
    M_E( STYLE, text_outline,       RES_CHRATR_CONTOUR, 0 ),
    // RES_CHRATR_CROSSEDOUT
    M_E( STYLE, text_crossing_out,  RES_CHRATR_CROSSEDOUT, 0 ),
    // RES_CHRATR_ESCAPEMENT
    M_E( STYLE, text_position,      RES_CHRATR_ESCAPEMENT, 0 ),
    // RES_CHRATR_FONT
    M_E( FO,    font_family,        RES_CHRATR_FONT,    MID_FONT_FAMILY_NAME ),
    M_E( STYLE, font_style_name,    RES_CHRATR_FONT,    MID_FONT_STYLE_NAME ),
    M_E( STYLE, font_family_generic,RES_CHRATR_FONT,    MID_FONT_FAMILY ),
    M_E( STYLE, font_pitch,         RES_CHRATR_FONT,    MID_FONT_PITCH ),
    M_E( STYLE, font_charset,       RES_CHRATR_FONT,    MID_FONT_CHAR_SET ),
    // RES_CHRATR_FONTSIZE
    M_E( FO,    font_size,          RES_CHRATR_FONTSIZE, 0 ),
    // RES_CHRATR_KERNING
    M_E( FO,    letter_spacing,     RES_CHRATR_KERNING, 0 ),
    // RES_CHRATR_LANGUAGE
    M_E_SI( FO, language,           RES_CHRATR_LANGUAGE,    MID_LANGUAGE ),
    M_E_SI( FO, country,            RES_CHRATR_LANGUAGE,    MID_COUNTRY ),
    // RES_CHRATR_POSTURE
    M_E( FO,    font_style,         RES_CHRATR_POSTURE, 0 ),
    // RES_CHRATR_PROPORTIONALFONTSIZE
    // TODO: not used?
    // RES_CHRATR_SHADOWED
    M_E( FO,    text_shadow,        RES_CHRATR_SHADOWED, 0 ),
    // RES_CHRATR_UNDERLINE
    M_E( STYLE, text_underline,     RES_CHRATR_UNDERLINE, 0 ),
    // RES_CHRATR_WEIGHT
    M_E( FO,    font_weight,        RES_CHRATR_WEIGHT,  0 ),
    // RES_CHRATR_WORDLINEMODE
    M_E( FO,    score_spaces,       RES_CHRATR_WORDLINEMODE, 0 ),
    // RES_CHRATR_AUTOKERN
    M_E( STYLE, letter_kerning,     RES_CHRATR_AUTOKERN, 0 ),
    // RES_CHRATR_BLINK
    M_E( STYLE, text_blinking,      RES_CHRATR_BLINK,   0 ),
    // RES_CHRATR_NOHYPHEN
    // TODO: not used?
    // RES_CHRATR_NOLINEBREAK
    // TODO: not used?
    // RES_CHRATR_BACKGROUND
    M_E( FO,    text_background_color, RES_CHRATR_BACKGROUND, MID_BACK_COLOR ),
    // RES_TXTATR_INETFMT
    // TODO
    // RES_TXTATR_REFMARK
    // TODO
    // RES_TXTATR_TOXMARK
    // TODO
    // RES_TXTATR_CHARFMT
    M_E_SI( TEXT,   style_name,         RES_TXTATR_CHARFMT, 0 ),
    // RES_TXTATR_FIELD
    // TODO
    // RES_TXTATR_FLYCNT
    // TODO
    // RES_TXTATR_FTN
    // TODO
    // RES_TXTATR_SOFTHYPH
    // TODO
    // RES_TXTATR_HARDBLANK
    // TODO

    // RES_PARATR_LINESPACING
    M_E( FO,    line_height,            RES_PARATR_LINESPACING, MID_LS_FIXED ),
    M_E( STYLE, line_height_at_least,   RES_PARATR_LINESPACING, MID_LS_MINIMUM ),
    M_E( STYLE, line_spacing,           RES_PARATR_LINESPACING, MID_LS_DISTANCE ),
    // RES_PARATR_ADJUST
    M_E( FO,    text_align,         RES_PARATR_ADJUST,  MID_PARA_ADJUST ),
    M_E( FO,    text_align_last,    RES_PARATR_ADJUST,  MID_LAST_LINE_ADJUST ),
    M_E( STYLE, justify_single_word,RES_PARATR_ADJUST,  MID_EXPAND_SINGLE ),
    // RES_PARATR_SPLIT
    M_E( STYLE, break_inside,       RES_PARATR_SPLIT,   0 ),
    // RES_PARATR_ORPHANS
    M_E( FO,    orphans,            RES_PARATR_ORPHANS, 0 ),
    // RES_PARATR_WIDOWS
    M_E( FO,    widows,             RES_PARATR_WIDOWS,  0 ),
    // RES_PARATR_TABSTOP
    M_E( STYLE, tabstop_tab_stops,  RES_PARATR_TABSTOP, MID_FLAG_ELEMENT_ITEM ),
    // RES_PARATR_HYPHENZONE
    M_E( FO,    hyphenate,                      RES_PARATR_HYPHENZONE,
                                                MID_IS_HYPHEN ),
    M_E( FO,    hyphenation_remain_char_count,  RES_PARATR_HYPHENZONE,
                                                MID_HYPHEN_MIN_LEAD ),
    M_E( FO,    hyphenation_push_char_count,    RES_PARATR_HYPHENZONE,
                                                MID_HYPHEN_MIN_TRAIL ),
    M_E( FO,    hyphenation_ladder_count,       RES_PARATR_HYPHENZONE,
                                                MID_HYPHEN_MAX_HYPHENS ),
    // RES_PARATR_DROP
    M_E( STYLE, drop_cap,           RES_PARATR_DROP,    MID_FLAG_ELEMENT_ITEM ),
    // RES_PARATR_REGISTER
    M_E( STYLE, register_true,      RES_PARATR_REGISTER,    0 ),
    // RES_PARATR_NUMRULE
    // not required

    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    // not required
    // RES_PAPER_BIN
    // not required
    // RES_LR_SPACE
    M_E( FO,    margin_left,        RES_LR_SPACE,   MID_L_MARGIN ),
    M_E( FO,    margin_right,       RES_LR_SPACE,   MID_R_MARGIN ),
    M_E( FO,    text_indent,        RES_LR_SPACE,   MID_FIRST_LINE_INDENT ),
    M_E( STYLE, auto_text_indent,   RES_LR_SPACE,   MID_FIRST_AUTO ),
    // RES_UL_SPACE
    M_E( FO,    margin_top,         RES_UL_SPACE,   MID_UP_MARGIN ),
    M_E( FO,    margin_bottom,      RES_UL_SPACE,   MID_LO_MARGIN ),
    // RES_PAGEDESC
    // TODO
    // RES_BREAK
    M_E( FO,    break_before,       RES_BREAK,  MID_BREAK_BEFORE ),
    M_E( FO,    break_after,        RES_BREAK,  MID_BREAK_AFTER ),
    // RES_CNTNT
    // not required
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    // not required
    // RES_OPAQUE
    // not required
    // RES_PROTECT
    // not required
    // RES_SURROUND
    // not required
    // RES_VERT_ORIENT
    // not required
    // RES_HORI_ORIENT
    // not required
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,    background_color,       RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE, background_image,       RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    M_E( STYLE, border_line_width,        RES_BOX, ALL_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_left,   RES_BOX, LEFT_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_right,  RES_BOX, RIGHT_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_top,    RES_BOX, TOP_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_bottom, RES_BOX, BOTTOM_BORDER_LINE_WIDTH ),
    M_E( FO,    padding,                  RES_BOX, ALL_BORDER_PADDING ),
    M_E( FO,    padding_left,             RES_BOX, LEFT_BORDER_PADDING ),
    M_E( FO,    padding_right,            RES_BOX, RIGHT_BORDER_PADDING ),
    M_E( FO,    padding_top,              RES_BOX, TOP_BORDER_PADDING ),
    M_E( FO,    padding_bottom,           RES_BOX, BOTTOM_BORDER_PADDING ),
    M_E( FO,    border,                   RES_BOX, ALL_BORDER ),
    M_E( FO,    border_left,              RES_BOX, LEFT_BORDER ),
    M_E( FO,    border_right,             RES_BOX, RIGHT_BORDER ),
    M_E( FO,    border_top,               RES_BOX, TOP_BORDER ),
    M_E( FO,    border_bottom,            RES_BOX, BOTTOM_BORDER ),
    // RES_SHADOW
    M_E( STYLE, shadow,             RES_SHADOW, 0 ),
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    M_E( FO, keep_with_next,        RES_KEEP, 0 ),
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required
    // RES_LINENUMBER
    // TODO
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    {0,0,0,0 }
};
#endif

SvXMLItemMapEntry aXMLTableItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    M_E_SE( STYLE,  width,          RES_FRM_SIZE,   MID_FRMSIZE_WIDTH ),
    M_E_SE( STYLE,  rel_width,      RES_FRM_SIZE,   MID_FRMSIZE_REL_WIDTH ),
    // RES_PAPER_BIN
    // not required
    // TODO: RES_LR_SPACE
    M_E_SE( FO, margin_left,        RES_LR_SPACE,   MID_L_MARGIN ),
    M_E_SE( FO, margin_right,       RES_LR_SPACE,   MID_R_MARGIN ),
    // RES_UL_SPACE
    M_E( FO,    margin_top,         RES_UL_SPACE,   MID_UP_MARGIN ),
    M_E( FO,    margin_bottom,      RES_UL_SPACE,   MID_LO_MARGIN ),
    // RES_PAGEDESC
    // TODO
    // RES_BREAK
    M_E( FO,    break_before,       RES_BREAK,  MID_BREAK_BEFORE ),
    M_E( FO,    break_after,        RES_BREAK,  MID_BREAK_AFTER ),
    // RES_CNTNT
    // not required
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    // not required
    // RES_OPAQUE
    // not required
    // RES_PROTECT
    // not required
    // RES_SURROUND
    // not required
    // RES_VERT_ORIENT
    // not required
    // RES_HORI_ORIENT
    M_E( TABLE, align,                  RES_HORI_ORIENT, 0 ),
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,    background_color,       RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE, background_image,       RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    // not required
    // RES_SHADOW
    M_E( STYLE, shadow,             RES_SHADOW, 0 ),
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    M_E( FO, keep_with_next,        RES_KEEP, 0 ),
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    M_E( STYLE, may_break_between_rows, RES_LAYOUT_SPLIT, 0 ),
    // RES_CHAIN
    // not required
    // RES_LINENUMBER
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    {0,0,0,0 }
};

SvXMLItemMapEntry aXMLTableColItemMap[] =
{
    M_E_SI( STYLE,  column_width,     RES_FRM_SIZE, MID_FRMSIZE_COL_WIDTH ),
    M_E( STYLE,     rel_column_width, RES_FRM_SIZE, MID_FRMSIZE_REL_COL_WIDTH ),
    {0,0,0,0 }
};

SvXMLItemMapEntry aXMLTableRowItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    M_E( STYLE, row_height,         RES_FRM_SIZE,   MID_FRMSIZE_FIX_HEIGHT ),
    M_E( STYLE, min_row_height,     RES_FRM_SIZE,   MID_FRMSIZE_MIN_HEIGHT ),
    // RES_PAPER_BIN
    // not required
    // RES_LR_SPACE
    // not required
    // RES_UL_SPACE
    // not required
    // RES_PAGEDESC
    // not required
    // RES_BREAK
    // not required
    // RES_CNTNT
    // not required
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    // not required
    // RES_OPAQUE
    // not required
    // RES_PROTECT
    // not required
    // RES_SURROUND
    // not required
    // RES_VERT_ORIENT
    // not required
    // RES_HORI_ORIENT
    // not required
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,    background_color,       RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE, background_image,       RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    // not required
    // RES_ANCHOR
    // not required
    // RES_SHADOW
    // not required
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    // not required
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required
    // RES_LINENUMBER
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    {0,0,0,0 }
};

SvXMLItemMapEntry aXMLTableCellItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    // not required
    // RES_PAPER_BIN
    // not required
    // RES_LR_SPACE
    // not required
    // RES_UL_SPACE
    // not required
    // RES_PAGEDESC
    // not required
    // RES_BREAK
    // not required
    // RES_CNTNT
    // not required
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    // not required
    // RES_OPAQUE
    // not required
    // RES_PROTECT
    // not required
    // RES_SURROUND
    // not required
    // RES_VERT_ORIENT
    M_E( TABLE, vertical_align,                 RES_VERT_ORIENT, 0 ),
    // RES_HORI_ORIENT
    // not required
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,    background_color,       RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE, background_image,       RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    M_E( STYLE, border_line_width,        RES_BOX, ALL_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_left,   RES_BOX, LEFT_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_right,  RES_BOX, RIGHT_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_top,    RES_BOX, TOP_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_bottom, RES_BOX, BOTTOM_BORDER_LINE_WIDTH ),
    M_E( FO,    padding,                  RES_BOX, ALL_BORDER_PADDING ),
    M_E( FO,    padding_left,             RES_BOX, LEFT_BORDER_PADDING ),
    M_E( FO,    padding_right,            RES_BOX, RIGHT_BORDER_PADDING ),
    M_E( FO,    padding_top,              RES_BOX, TOP_BORDER_PADDING ),
    M_E( FO,    padding_bottom,           RES_BOX, BOTTOM_BORDER_PADDING ),
    M_E( FO,    border,                   RES_BOX, ALL_BORDER ),
    M_E( FO,    border_left,              RES_BOX, LEFT_BORDER ),
    M_E( FO,    border_right,             RES_BOX, RIGHT_BORDER ),
    M_E( FO,    border_top,               RES_BOX, TOP_BORDER ),
    M_E( FO,    border_bottom,            RES_BOX, BOTTOM_BORDER ),
    // RES_SHADOW
    // not required
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    // not required
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required
    // RES_LINENUMBER
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    {0,0,0,0 }
};

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmlitemm.cxx,v 1.1.1.1 2000-09-18 17:15:00 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.8  2000/09/18 16:05:07  willem.vandorp
      OpenOffice header added.

      Revision 1.7  2000/08/24 11:16:41  mib
      text import continued

      Revision 1.6  2000/08/10 10:22:16  mib
      #74404#: Adeptions to new XSL/XLink working draft

      Revision 1.5  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.4  2000/02/17 14:40:30  mib
      #70271#: XML table import

      Revision 1.3  2000/02/07 10:03:28  mib
      #70271#: tables

      Revision 1.2  2000/01/12 15:00:23  mib
      #70271#: lists

      Revision 1.1  1999/12/14 07:33:08  mib
      #70271#: XML import/export of drop cap/register/language item, splitted swxmlat


*************************************************************************/

