/*************************************************************************
 *
 *  $RCSfile: txtprmap.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-29 14:56:55 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_TXTPRHDL_HXX
#include "txtprhdl.hxx"
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include "txtprmap.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define M_E( a, p, l, t, c ) \
    { a, XML_NAMESPACE_##p, sXML_##l, t, c }

#define M_ED( a, p, l, t, c ) \
    { a, XML_NAMESPACE_##p, sXML_##l, (t) | MID_FLAG_DEFAULT_ITEM_EXPORT, c }


XMLPropertyMapEntry aXMLParaPropMap[] =
{
    // RES_CHRATR_CASEMAP
    M_E( "CharCaseMap",     FO,     font_variant,       XML_TYPE_TEXT_CASEMAP_VAR,  0 ),
    M_E( "CharCaseMap",     FO,     text_transform,     XML_TYPE_TEXT_CASEMAP,  0 ),
    // RES_CHRATR_COLOR
    M_ED( "CharColor",      FO,     color,              XML_TYPE_COLORAUTO|MID_FLAG_MERGE_PROPERTY, 0 ),
    M_ED( "CharColor",      STYLE,  use_window_font_color,  XML_TYPE_ISAUTOCOLOR|MID_FLAG_MERGE_PROPERTY,   0 ),
    // RES_CHRATR_CONTOUR
    M_E( "CharContoured",   STYLE,  text_outline,       XML_TYPE_BOOL,  0 ),
    // RES_CHRATR_CROSSEDOUT
    M_E( "CharStrikeout",   STYLE,  text_crossing_out,  XML_TYPE_TEXT_CROSSEDOUT,   0),
    // RES_CHRATR_ESCAPEMENT
    M_E( "CharEscapement",       STYLE, text_position,  XML_TYPE_TEXT_ESCAPEMENT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "CharEscapementHeight", STYLE, text_position,  XML_TYPE_TEXT_ESCAPEMENT_HEIGHT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    // RES_CHRATR_FONT
    M_ED( "CharFontName",   STYLE,  font_name,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME ),
    M_ED( "CharFontName",   FO,     font_family,        XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME ),
    M_ED( "CharFontStyleName",STYLE,    font_style_name,    XML_TYPE_STRING, CTF_FONTSTYLENAME ),
    M_ED( "CharFontFamily", STYLE,  font_family_generic,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY ),
    M_ED( "CharFontPitch",  STYLE,  font_pitch,         XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH ),
    M_ED( "CharFontCharSet",    STYLE,  font_charset,       XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET ),
    // RES_CHRATR_FONTSIZE
    M_ED( "CharHeight",       FO,   font_size,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT ),
    M_ED( "CharPropHeight",FO,  font_size,          XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL ),
    M_ED( "CharDiffHeight",STYLE,font_size_rel,     XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF ),
    // RES_CHRATR_KERNING
    M_E( "CharKerning",     FO,     letter_spacing,     XML_TYPE_TEXT_KERNING, 0 ),
    // RES_CHRATR_LANGUAGE
    M_ED( "CharLocale",     FO,     language,           XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    M_ED( "CharLocale",     FO,     country,            XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_POSTURE
    M_E( "CharPosture",     FO,     font_style,         XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_PROPORTIONALFONTSIZE
    // TODO: not used?
    // RES_CHRATR_SHADOWED
    M_E( "CharShadowed",    FO,     text_shadow,        XML_TYPE_TEXT_SHADOWED, 0 ),
    // RES_CHRATR_UNDERLINE
    M_E( "CharUnderline",   STYLE,  text_underline,     XML_TYPE_TEXT_UNDERLINE, CTF_UNDERLINE ),
    M_E( "CharUnderlineColor",  STYLE,  text_underline_color,       XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, CTF_UNDERLINE_COLOR  ),
    M_E( "CharUnderlineHasColor",   STYLE,  text_underline_color,       XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, CTF_UNDERLINE_HASCOLOR   ),
    // RES_CHRATR_WEIGHT
    M_E( "CharWeight",      FO,     font_weight,        XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_WORDLINEMODE
    M_E( "WordMode",        FO,     score_spaces,       XML_TYPE_NBOOL, 0 ),
    // RES_CHRATR_AUTOKERN
    M_E( "CharAutoKerning", STYLE,  letter_kerning,     XML_TYPE_BOOL, 0 ),
    // RES_CHRATR_BLINK
    M_E( "CharFlash",       STYLE,  text_blinking,      XML_TYPE_BOOL, 0 ),
    // RES_CHRATR_NOHYPHEN
    // TODO: not used?
    // RES_CHRATR_NOLINEBREAK
    // TODO: not used?
    // RES_CHRATR_BACKGROUND
    M_E( "CharBackColor",   STYLE,  text_background_color, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "CharBackTransparent", STYLE,  text_background_color, XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    M_E( "CharBackColor",   FO, text_background_color, XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM_EXPORT, CTF_OLDTEXTBACKGROUND ),
    // RES_CHRATR_CJK_FONT
    M_ED( "CharFontNameAsian",  STYLE,  font_name_asian,            XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CJK ),
    M_ED( "CharFontNameAsian",  STYLE,      font_family_asian,      XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME_CJK ),
    M_ED( "CharFontStyleNameAsian",STYLE,   font_style_name_asian,  XML_TYPE_STRING, CTF_FONTSTYLENAME_CJK ),
    M_ED( "CharFontFamilyAsian",    STYLE,  font_family_generic_asian,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CJK ),
    M_ED( "CharFontPitchAsian", STYLE,  font_pitch_asian,           XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CJK ),
    M_ED( "CharFontCharSetAsian",   STYLE,  font_charset_asian,     XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CJK ),
    // RES_CHRATR_CJK_FONTSIZE
    M_ED( "CharHeightAsian",          STYLE,    font_size_asian,            XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CJK ),
    M_ED( "CharPropHeightAsian",STYLE,  font_size_asian,            XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL_CJK ),
    M_ED( "CharDiffHeightAsian",STYLE,font_size_rel_asian,      XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF_CJK ),
    // RES_CHRATR_CJK_LANGUAGE
    M_ED( "CharLocaleAsian",        STYLE,      language_asian,             XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    M_ED( "CharLocaleAsian",        STYLE,      country_asian,          XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_CJK_POSTURE
    M_E( "CharPostureAsian",        STYLE,      font_style_asian,           XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_CJK_WEIGHT
    M_E( "CharWeightAsian",     STYLE,      font_weight_asian,      XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_CTL_FONT
    M_ED( "CharFontNameComplex",    STYLE,  font_name_complex,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CTL ),
    M_ED( "CharFontNameComplex",    STYLE,      font_family_complex,        XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME_CTL ),
    M_ED( "CharFontStyleNameComplex",STYLE, font_style_name_complex,    XML_TYPE_STRING, CTF_FONTSTYLENAME_CTL ),
    M_ED( "CharFontFamilyComplex",  STYLE,  font_family_generic_complex,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CTL ),
    M_ED( "CharFontPitchComplex",   STYLE,  font_pitch_complex,         XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CTL ),
    M_ED( "CharFontCharSetComplex", STYLE,  font_charset_complex,       XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CTL ),
    // RES_CHRATR_CTL_FONTSIZE
    M_ED( "CharHeightComplex",        STYLE,    font_size_complex,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CTL ),
    M_ED( "CharPropHeightComplex",STYLE,    font_size_complex,          XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL_CTL ),
    M_ED( "CharDiffHeightComplex",STYLE,font_size_rel_complex,      XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF_CTL ),
    // RES_CHRATR_CTL_LANGUAGE
    M_ED( "CharLocaleComplex",      STYLE,      language_complex,           XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    M_ED( "CharLocaleComplex",      STYLE,      country_complex,            XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_CTL_POSTURE
    M_E( "CharPostureComplex",      STYLE,      font_style_complex,         XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_CTL_WEIGHT
    M_E( "CharWeightComplex",       STYLE,      font_weight_complex,        XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_ROTATE
    M_E( "CharRotation",            STYLE,      text_rotation_angle,        XML_TYPE_TEXT_ROTATION_ANGLE, 0 ),
    M_E( "CharRotationIsFitToLine", STYLE,      text_rotation_scale,        XML_TYPE_TEXT_ROTATION_SCALE, 0 ),
    // RES_CHRATR_EMPHASIS_MARK
    M_E( "CharEmphasis",            STYLE,      text_emphasize,             XML_TYPE_TEXT_EMPHASIZE, 0 ),
    // RES_CHRATR_TWO_LINES
    M_E( "CharCombineIsOn",         STYLE,      text_combine,               XML_TYPE_TEXT_COMBINE, 0 ),
    M_E( "CharCombinePrefix",       STYLE,      text_combine_start_char,    XML_TYPE_TEXT_COMBINECHAR, 0 ),
    M_E( "CharCombineSuffix",       STYLE,      text_combine_end_char,      XML_TYPE_TEXT_COMBINECHAR, 0 ),
    // RES_CHRATR_SCALEW
    M_E( "CharScaleWidth",          STYLE,      text_scale,                 XML_TYPE_PERCENT16, 0 ),
    //RES_CHRATR_RELIEF
    M_E( "CharRelief",              STYLE,      font_relief,                XML_TYPE_TEXT_FONT_RELIEF, 0 ),
    // RES_TXTATR_INETFMT
    // TODO
    // RES_TXTATR_REFMARK
    // TODO
    // RES_TXTATR_TOXMARK
    // TODO
    // RES_TXTATR_CHARFMT
//  M_E_SI( TEXT,   style_name,         RES_TXTATR_CHARFMT, 0 ),
    // RES_TXTATR_CJK_RUBY
    // TODO
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
    M_E( "ParaLineSpacing",     FO,     line_height,            XML_TYPE_LINE_SPACE_FIXED, 0 ),
    M_E( "ParaLineSpacing",     STYLE,  line_height_at_least,   XML_TYPE_LINE_SPACE_MINIMUM, 0 ),
    M_E( "ParaLineSpacing",     STYLE,  line_spacing,           XML_TYPE_LINE_SPACE_DISTANCE, 0 ),
    // RES_PARATR_ADJUST
    M_E( "ParaAdjust",          FO,     text_align,         XML_TYPE_TEXT_ADJUST, 0 ),
    M_E( "ParaLastLineAdjust",  FO,     text_align_last,    XML_TYPE_TEXT_ADJUSTLAST, 0 ),
    M_E( "ParaExpandSingleWord",STYLE,  justify_single_word,XML_TYPE_BOOL, 0 ),
    // RES_PARATR_SPLIT
    M_E( "ParaSplit",           STYLE,  break_inside,       XML_TYPE_TEXT_SPLIT, 0 ),
    // RES_PARATR_ORPHANS
    M_E( "ParaOrphans",         FO,     orphans,            XML_TYPE_NUMBER8, 0 ),
    // RES_PARATR_WIDOWS
    M_E( "ParaWidows",          FO,     widows,             XML_TYPE_NUMBER8, 0 ),
    // RES_PARATR_TABSTOP
    M_ED( "ParaTabStops",       STYLE,  tab_stops,          MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_TABSTOP, CTF_TABSTOP ), // this is not realy a string!
    // RES_PARATR_HYPHENZONE
    M_E( "ParaIsHyphenation",   FO,     hyphenate,          XML_TYPE_BOOL, 0 ),
    M_E( "ParaHyphenationMaxLeadingChars", FO, hyphenation_remain_char_count, XML_TYPE_NUMBER, 0 ),
    M_E( "ParaHyphenationMaxTrailingChars",FO, hyphenation_push_char_count, XML_TYPE_NUMBER, 0 ),
    M_E( "ParaHyphenationMaxHyphens",   FO, hyphenation_ladder_count, XML_TYPE_NUMBER_NONE, 0 ),
    // RES_PARATR_DROP
    M_E( "DropCapWholeWord",    STYLE,  length,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL, CTF_DROPCAPWHOLEWORD ),
    M_E( "DropCapCharStyleName",    STYLE,  style_name, MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_DROPCAPCHARSTYLE ),
    M_E( "DropCapFormat",       STYLE,  drop_cap,   MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_DROPCAP, CTF_DROPCAPFORMAT ),
    // RES_PARATR_REGISTER
    M_E( "ParaRegisterModeActive",  STYLE,  register_true,  XML_TYPE_BOOL, 0 ),
    // RES_PARATR_NUMRULE
    M_E( "NumberingStyleName",  STYLE,  list_style_name,    MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_NUMBERINGSTYLENAME ),
    M_E( "IsNumbering",         TEXT,   enable_numbering,   XML_TYPE_BOOL, 0 ),

    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    // not required
    // RES_PAPER_BIN
    // not required
    // RES_LR_SPACE

    M_E( "ParaLeftMargin",          FO, margin_left,        XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARALEFTMARGIN ),
    M_E( "ParaLeftMarginRelative",  FO, margin_left,        XML_TYPE_PERCENT, CTF_PARALEFTMARGIN_REL ),
    M_E( "ParaRightMargin",         FO, margin_right,       XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARARIGHTMARGIN ),
    M_E( "ParaRightMarginRelative", FO, margin_right,       XML_TYPE_PERCENT, CTF_PARARIGHTMARGIN_REL ),
    M_E( "ParaFirstLineIndent",     FO, text_indent,        XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARAFIRSTLINE ),
    M_E( "ParaFirstLineIndentRelative", FO, text_indent,    XML_TYPE_PERCENT, CTF_PARAFIRSTLINE_REL ),
    M_E( "ParaLastLineAdjust",  STYLE, auto_text_indent,    XML_TYPE_BOOL, 0 ),
    // RES_UL_SPACE
    M_E( "ParaTopMargin",           FO, margin_top,         XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARATOPMARGIN ),
    M_E( "ParaTopMarginRelative",   FO, margin_top,         XML_TYPE_PERCENT, CTF_PARATOPMARGIN_REL ),
    M_E( "ParaBottomMargin",        FO, margin_bottom,      XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARABOTTOMMARGIN ),
    M_E( "ParaBottomMarginRelative",FO, margin_bottom,      XML_TYPE_PERCENT, CTF_PARABOTTOMMARGIN_REL ),
    // RES_PAGEDESC
    M_E( "PageDescName",            STYLE,  master_page_name,           MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_PAGEDESCNAME ),
    M_E( "PageNumberOffset",        STYLE,  page_number,            XML_TYPE_NUMBER16, 0 ),
    // RES_BREAK : TODO: does this work?
    M_E( "BreakType",       FO, break_before,       XML_TYPE_TEXT_BREAKBEFORE|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "BreakType",       FO, break_after,        XML_TYPE_TEXT_BREAKAFTER, 0 ),
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
    M_E( "ParaBackColor",   FO, background_color,       XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "ParaBackTransparent", FO, background_color,       XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    M_E( "ParaBackGraphicLocation", STYLE,  position,   MID_FLAG_SPECIAL_ITEM|XML_TYPE_BUILDIN_CMP_ONLY, CTF_BACKGROUND_POS  ),
    M_E( "ParaBackGraphicFilter",STYLE, filter_name,    MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_FILTER ),
    M_E( "ParaBackGraphicURL",  STYLE,  background_image,   MID_FLAG_ELEMENT_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_URL ),
    // RES_BOX
    M_E( "LeftBorder",          STYLE,  border_line_width,        XML_TYPE_BORDER_WIDTH, CTF_ALLBORDERWIDTH ),
    M_E( "LeftBorder",          STYLE,  border_line_width_left,   XML_TYPE_BORDER_WIDTH, CTF_LEFTBORDERWIDTH ),
    M_E( "RightBorder",         STYLE,  border_line_width_right,  XML_TYPE_BORDER_WIDTH, CTF_RIGHTBORDERWIDTH ),
    M_E( "TopBorder",           STYLE,  border_line_width_top,    XML_TYPE_BORDER_WIDTH, CTF_TOPBORDERWIDTH ),
    M_E( "BottomBorder",        STYLE,  border_line_width_bottom, XML_TYPE_BORDER_WIDTH, CTF_BOTTOMBORDERWIDTH ),

    M_E( "LeftBorderDistance",  FO,     padding,                  XML_TYPE_MEASURE, CTF_ALLBORDERDISTANCE ), // need special import filtering
    M_E( "LeftBorderDistance",  FO,     padding_left,             XML_TYPE_MEASURE, CTF_LEFTBORDERDISTANCE ),
    M_E( "RightBorderDistance", FO,     padding_right,            XML_TYPE_MEASURE, CTF_RIGHTBORDERDISTANCE ),
    M_E( "TopBorderDistance",   FO,     padding_top,              XML_TYPE_MEASURE, CTF_TOPBORDERDISTANCE ),
    M_E( "BottomBorderDistance",FO,     padding_bottom,           XML_TYPE_MEASURE, CTF_BOTTOMBORDERDISTANCE ),

    M_E( "LeftBorder",          FO,     border,                   XML_TYPE_BORDER, CTF_ALLBORDER ),
    M_E( "LeftBorder",          FO,     border_left,              XML_TYPE_BORDER, CTF_LEFTBORDER ),
    M_E( "RightBorder",         FO,     border_right,             XML_TYPE_BORDER, CTF_RIGHTBORDER ),
    M_E( "TopBorder",           FO,     border_top,               XML_TYPE_BORDER, CTF_TOPBORDER ),
    M_E( "BottomBorder",        FO,     border_bottom,            XML_TYPE_BORDER, CTF_BOTTOMBORDER ),
    // RES_SHADOW
    M_E( "ParaShadowFormat",    STYLE,  shadow,     XML_TYPE_TEXT_SHADOW, 0 ),
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    M_E( "ParaKeepTogether",    FO, keep_with_next,     XML_TYPE_BOOL, 0 ),
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required

    // RES_LINENUMBER
    M_E( "ParaLineNumberCount", TEXT,   number_lines,           XML_TYPE_BOOL, 0 ),
    M_E( "ParaLineNumberStartValue", TEXT, line_number,         XML_TYPE_NUMBER, 0 ),

    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E( "UserDefinedAttributes", TEXT, xmlns, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    M_ED( "ParaIsCharacterDistance", STYLE, text_autospace, XML_TYPE_TEXT_AUTOSPACE, 0 ),
    M_ED( "ParaIsHangingPunctuation", STYLE, punctuation_wrap, XML_TYPE_TEXT_PUNCTUATION_WRAP, 0 ),
    M_ED( "ParaIsForbiddenRules", STYLE, line_break, XML_TYPE_TEXT_LINE_BREAK, 0 ),
    M_ED( "TabStopDistance", STYLE, tab_stop_distance, XML_TYPE_MEASURE, 0 ),
    { 0, 0, 0, 0 }
};

XMLPropertyMapEntry aXMLTextPropMap[] =
{
    // RES_CHRATR_CASEMAP
    M_E( "CharCaseMap",     FO,     font_variant,       XML_TYPE_TEXT_CASEMAP_VAR,  0 ),
    M_E( "CharCaseMap",     FO,     text_transform,     XML_TYPE_TEXT_CASEMAP,  0 ),
    // RES_CHRATR_COLOR
    M_ED( "CharColor",      FO,     color,              XML_TYPE_COLORAUTO|MID_FLAG_MERGE_PROPERTY, 0 ),
    M_ED( "CharColor",      STYLE,  use_window_font_color,  XML_TYPE_ISAUTOCOLOR|MID_FLAG_MERGE_PROPERTY,   0 ),
    // RES_CHRATR_CONTOUR
    M_E( "CharContoured",   STYLE,  text_outline,       XML_TYPE_BOOL,  0 ),
    // RES_CHRATR_CROSSEDOUT
    M_E( "CharStrikeout",   STYLE,  text_crossing_out,  XML_TYPE_TEXT_CROSSEDOUT,   0),
    // RES_CHRATR_ESCAPEMENT
    M_E( "CharEscapement",       STYLE, text_position,  XML_TYPE_TEXT_ESCAPEMENT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "CharEscapementHeight", STYLE, text_position,  XML_TYPE_TEXT_ESCAPEMENT_HEIGHT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    // RES_CHRATR_FONT
    M_ED( "CharFontName",   STYLE,  font_name,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME ),
    M_ED( "CharFontName",   FO,     font_family,        XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME ),
    M_ED( "CharFontStyleName",STYLE,    font_style_name,    XML_TYPE_STRING, CTF_FONTSTYLENAME ),
    M_ED( "CharFontFamily", STYLE,  font_family_generic,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY ),
    M_ED( "CharFontPitch",  STYLE,  font_pitch,         XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH ),
    M_ED( "CharFontCharSet",    STYLE,  font_charset,       XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET ),
    // RES_CHRATR_FONTSIZE
    M_ED( "CharHeight",       FO,   font_size,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT ),
    M_ED( "CharPropHeight",FO,  font_size,          XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL ),
    M_ED( "CharDiffHeight",STYLE,font_size_rel,     XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF ),
    // RES_CHRATR_KERNING
    M_E( "CharKerning",     FO,     letter_spacing,     XML_TYPE_TEXT_KERNING, 0 ),
    // RES_CHRATR_LANGUAGE
    M_ED( "CharLocale",     FO,     language,           XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    M_ED( "CharLocale",     FO,     country,            XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_POSTURE
    M_E( "CharPosture",     FO,     font_style,         XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_PROPORTIONALFONTSIZE
    // TODO: not used?
    // RES_CHRATR_SHADOWED
    M_E( "CharShadowed",    FO,     text_shadow,        XML_TYPE_TEXT_SHADOWED, 0 ),
    // BIS HIER GEPRUEFT!
    // RES_CHRATR_UNDERLINE
    M_E( "CharUnderline",   STYLE,  text_underline,     XML_TYPE_TEXT_UNDERLINE, CTF_UNDERLINE ),
    M_E( "CharUnderlineColor",  STYLE,  text_underline_color,       XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, CTF_UNDERLINE_COLOR  ),
    M_E( "CharUnderlineHasColor",   STYLE,  text_underline_color,       XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, CTF_UNDERLINE_HASCOLOR   ),
    // RES_CHRATR_WEIGHT
    M_E( "CharWeight",      FO,     font_weight,        XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_WORDLINEMODE
    M_E( "WordMode",        FO,     score_spaces,       XML_TYPE_NBOOL, 0 ),
    // RES_CHRATR_AUTOKERN
    M_E( "CharAutoKerning", STYLE,  letter_kerning,     XML_TYPE_BOOL, 0 ),
    // RES_CHRATR_BLINK
    M_E( "CharFlash",       STYLE,  text_blinking,      XML_TYPE_BOOL, 0 ),
    // RES_CHRATR_NOHYPHEN
    // TODO: not used?
    // RES_CHRATR_NOLINEBREAK
    // TODO: not used?
    // RES_CHRATR_BACKGROUND
    M_E( "CharBackColor",   STYLE,  text_background_color, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "CharBackTransparent", STYLE,  text_background_color, XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    M_E( "CharBackColor",   FO, text_background_color, XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM_EXPORT, CTF_OLDTEXTBACKGROUND ),
    // RES_CHRATR_CJK_FONT
    M_ED( "CharFontNameAsian",  STYLE,  font_name_asian,            XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CJK ),
    M_ED( "CharFontNameAsian",  STYLE,      font_family_asian,      XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME_CJK ),
    M_ED( "CharFontStyleNameAsian",STYLE,   font_style_name_asian,  XML_TYPE_STRING, CTF_FONTSTYLENAME_CJK ),
    M_ED( "CharFontFamilyAsian",    STYLE,  font_family_generic_asian,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CJK ),
    M_ED( "CharFontPitchAsian", STYLE,  font_pitch_asian,           XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CJK ),
    M_ED( "CharFontCharSetAsian",   STYLE,  font_charset_asian,     XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CJK ),
    // RES_CHRATR_CJK_FONTSIZE
    M_ED( "CharHeightAsian",          STYLE,    font_size_asian,            XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CJK ),
    M_ED( "CharPropHeightAsian",STYLE,  font_size_asian,            XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL_CJK ),
    M_ED( "CharDiffHeightAsian",STYLE,font_size_rel_asian,      XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF_CJK ),
    // RES_CHRATR_CJK_LANGUAGE
    M_ED( "CharLocaleAsian",        STYLE,      language_asian,             XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    M_ED( "CharLocaleAsian",        STYLE,      country_asian,          XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_CJK_POSTURE
    M_E( "CharPostureAsian",        STYLE,      font_style_asian,           XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_CJK_WEIGHT
    M_E( "CharWeightAsian",     STYLE,      font_weight_asian,      XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_CTL_FONT
    M_ED( "CharFontNameComplex",    STYLE,  font_name_complex,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CTL ),
    M_ED( "CharFontNameComplex",    STYLE,      font_family_complex,        XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME_CTL ),
    M_ED( "CharFontStyleNameComplex",STYLE, font_style_name_complex,    XML_TYPE_STRING, CTF_FONTSTYLENAME_CTL ),
    M_ED( "CharFontFamilyComplex",  STYLE,  font_family_generic_complex,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CTL ),
    M_ED( "CharFontPitchComplex",   STYLE,  font_pitch_complex,         XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CTL ),
    M_ED( "CharFontCharSetComplex", STYLE,  font_charset_complex,       XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CTL ),
    // RES_CHRATR_CTL_FONTSIZE
    M_ED( "CharHeightComplex",        STYLE,    font_size_complex,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CTL ),
    M_ED( "CharPropHeightComplex",STYLE,    font_size_complex,          XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL_CTL ),
    M_ED( "CharDiffHeightComplex",STYLE,font_size_rel_complex,      XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF_CTL ),
    // RES_CHRATR_CTL_LANGUAGE
    M_ED( "CharLocaleComplex",      STYLE,      language_complex,           XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    M_ED( "CharLocaleComplex",      STYLE,      country_complex,            XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_CTL_POSTURE
    M_E( "CharPostureComplex",      STYLE,      font_style_complex,         XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_CTL_WEIGHT
    M_E( "CharWeightComplex",       STYLE,      font_weight_complex,        XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_ROTATE
    M_E( "CharRotation",            STYLE,      text_rotation_angle,        XML_TYPE_TEXT_ROTATION_ANGLE, 0 ),
    M_E( "CharRotationIsFitToLine", STYLE,      text_rotation_scale,        XML_TYPE_TEXT_ROTATION_SCALE, 0 ),
    // RES_CHRATR_EMPHASIS_MARK
    M_E( "CharEmphasis",            STYLE,      text_emphasize,             XML_TYPE_TEXT_EMPHASIZE, 0 ),
    // RES_CHRATR_TWO_LINES
    M_E( "CharCombineIsOn",         STYLE,      text_combine,               XML_TYPE_TEXT_COMBINE|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "CharCombinePrefix",       STYLE,      text_combine_start_char,    XML_TYPE_TEXT_COMBINECHAR, 0 ),
    M_E( "CharCombineSuffix",       STYLE,      text_combine_end_char,      XML_TYPE_TEXT_COMBINECHAR, 0 ),
    // RES_CHRATR_SCALEW
    M_E( "CharScaleWidth",          STYLE,      text_scale,                 XML_TYPE_PERCENT16, 0 ),
    // combined characters field, does not correspond to a property
    M_E( "",                        STYLE,      text_combine,               XML_TYPE_TEXT_COMBINE_CHARACTERS|MID_FLAG_NO_PROPERTY, CTF_COMBINED_CHARACTERS_FIELD ),
    //RES_CHRATR_RELIEF
    M_E( "CharRelief",              STYLE,      font_relief,                XML_TYPE_TEXT_FONT_RELIEF, 0 ),

    // RES_TXTATR_INETFMT
    // TODO
    // RES_TXTATR_REFMARK
    // TODO
    // RES_TXTATR_TOXMARK
    // TODO
    // RES_TXTATR_CHARFMT
//  M_E_SI( TEXT,   style_name,         RES_TXTATR_CHARFMT, 0 ),
    // RES_TXTATR_CJK_RUBY
    // TODO
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
    // RES_UNKNOWNATR_CONTAINER
    M_E( "UserDefinedAttributes", TEXT, xmlns, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    M_ED( "ParaIsCharacterDistance", STYLE, text_autospace, XML_TYPE_TEXT_AUTOSPACE, 0 ),
    M_ED( "ParaIsHangingPunctuation", STYLE, punctuation_wrap, XML_TYPE_TEXT_PUNCTUATION_WRAP, 0 ),
    M_ED( "ParaIsForbiddenRules", STYLE, line_break, XML_TYPE_TEXT_LINE_BREAK, 0 ),
    M_E( "TabStopDistance", STYLE, tab_stop_distance, XML_TYPE_MEASURE, 0 ),
    { 0, 0, 0, 0 }
};

XMLPropertyMapEntry aXMLFramePropMap[] =
{
    // RES_FILL_ORDER
    // TODO: not required???
    // RES_FRM_SIZE
    M_ED( "Width",          SVG, width,             XML_TYPE_MEASURE, 0 ),
    M_ED( "RelativeWidth",  STYLE, rel_width,       XML_TYPE_TEXT_REL_WIDTH_HEIGHT, 0 ),
//  M_ED( "RelativeWidth",  STYLE, rel_width,       XML_TYPE_TEXT_REL_WIDTH_HEIGHT|MID_FLAG_MULTI_PROPERTY, 0 ),
//  M_ED( "IsSyncWidthToHeight",STYLE, rel_width,   XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT|MID_FLAG_MULTI_PROPERTY, 0 ),

    M_ED( "Height",         SVG, height,            XML_TYPE_MEASURE, CTF_FRAMEHEIGHT_ABS ),
    M_ED( "Height",         FO, min_height,         XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_FRAMEHEIGHT_MIN_ABS ),
    M_ED( "RelativeHeight", FO, min_height,         XML_TYPE_TEXT_REL_WIDTH_HEIGHT, CTF_FRAMEHEIGHT_MIN_REL ),
    M_ED( "RelativeHeight", STYLE, rel_height,      XML_TYPE_TEXT_REL_WIDTH_HEIGHT, CTF_FRAMEHEIGHT_REL ),
//  M_ED( "RelativeHeight", STYLE, rel_height,      XML_TYPE_TEXT_REL_WIDTH_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_FRAMEHEIGHT_REL ),
//  M_ED( "IsSyncHeightToWidth",STYLE, rel_height,  XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_SYNCHEIGHT ),
//  M_ED( "IsSyncHeightToWidth",STYLE, rel_height,  XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT_MIN, CTF_SYNCHEIGHT_MIN ),
    M_ED( "SizeType",       FO, text_box,           XML_TYPE_NUMBER16|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_SIZETYPE ),
    // RES_PAPER_BIN
    // not required
    // RES_ANCHOR
    // moved to here because it is not used for automatic styles
    M_ED( "AnchorType",         TEXT,   anchor_type,            XML_TYPE_TEXT_ANCHOR_TYPE, CTF_ANCHORTYPE ),
    M_ED( "AnchorPageNo",       TEXT,   anchor_page_number,     XML_TYPE_NUMBER16, CTF_ANCHORPAGENUMBER ),
    M_ED( "HoriOrientPosition", SVG,    x,        XML_TYPE_MEASURE, 0 ),
    M_ED( "VertOrientPosition", SVG,    y,        XML_TYPE_MEASURE, 0 ),
    // ***** The map for automatic styles starts here *****
    // RES_LR_SPACE
    M_E( "LeftMargin",              FO, margin_left,        XML_TYPE_MEASURE,  0),
    M_E( "RightMargin",             FO, margin_right,       XML_TYPE_MEASURE, 0 ),
    // RES_UL_SPACE
    M_E( "TopMargin",               FO, margin_top,         XML_TYPE_MEASURE, 0 ),
    M_E( "BottomMargin",            FO, margin_bottom,      XML_TYPE_MEASURE, 0 ),
    // RES_PAGEDESC
    // not required
    // RES_BREAK
    // not required
    // RES_CNTNT
    // not required (accessed using API)
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    M_E( "Print",                   STYLE,  print_content,  XML_TYPE_BOOL, 0 ),
    // RES_OPAQUE
    M_ED( "Opaque",                 STYLE,  run_through,    XML_TYPE_TEXT_OPAQUE, 0 ),
    // RES_PROTECT
    M_E( "ContentProtected",        STYLE,  protect,        XML_TYPE_TEXT_PROTECT_CONTENT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "SizeProtected",       STYLE,  protect,        XML_TYPE_TEXT_PROTECT_SIZE|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "PositionProtected",       STYLE,  protect,        XML_TYPE_TEXT_PROTECT_POSITION|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    // RES_SURROUND
    M_ED( "TextWrap",               STYLE,  wrap,   XML_TYPE_TEXT_WRAP, CTF_WRAP ),
    M_ED( "SurroundAnchorOnly",     STYLE,  number_wrapped_paragraphs,  XML_TYPE_TEXT_PARAGRAPH_ONLY, CTF_WRAP_PARAGRAPH_ONLY ),
    M_E( "SurroundContour",         STYLE,  wrap_contour,   XML_TYPE_BOOL, CTF_WRAP_CONTOUR ),
    M_E( "ContourOutside",          STYLE,  wrap_contour_mode,  XML_TYPE_TEXT_WRAP_OUTSIDE, CTF_WRAP_CONTOUR_MODE ),
    // RES_VERT_ORIENT
    M_ED( "VertOrient",             STYLE,  vertical_pos,         XML_TYPE_TEXT_VERTICAL_POS, CTF_VERTICALPOS ),
    M_ED( "VertOrient",             STYLE,  vertical_rel,         XML_TYPE_TEXT_VERTICAL_REL_AS_CHAR|MID_FLAG_MULTI_PROPERTY, CTF_VERTICALREL_ASCHAR ),
    M_ED( "VertOrientRelation",     STYLE,  vertical_rel,         XML_TYPE_TEXT_VERTICAL_REL, CTF_VERTICALREL ),
    M_ED( "VertOrientRelation",     STYLE,  vertical_rel,         XML_TYPE_TEXT_VERTICAL_REL_PAGE|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_VERTICALREL_PAGE ),
    M_ED( "VertOrientRelation",     STYLE,  vertical_rel,         XML_TYPE_TEXT_VERTICAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_VERTICALREL_FRAME ),
    // RES_HORI_ORIENT
    M_ED( "HoriOrient",             STYLE,  horizontal_pos,       XML_TYPE_TEXT_HORIZONTAL_POS|MID_FLAG_MULTI_PROPERTY, CTF_HORIZONTALPOS ),
    M_ED( "PageToggle",     STYLE,  horizontal_pos,       XML_TYPE_TEXT_HORIZONTAL_MIRROR, CTF_HORIZONTALMIRROR ),
    M_ED( "HoriOrient",             STYLE,  horizontal_pos,       XML_TYPE_TEXT_HORIZONTAL_POS_MIRRORED|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_HORIZONTALPOS_MIRRORED ),
    M_ED( "HoriOrientRelation",     STYLE,  horizontal_rel,       XML_TYPE_TEXT_HORIZONTAL_REL, CTF_HORIZONTALREL ),
    M_ED( "HoriOrientRelation",     STYLE,  horizontal_rel,       XML_TYPE_TEXT_HORIZONTAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_HORIZONTALREL_FRAME ),
    // RES_ANCHOR
    // see above
    // RES_BACKGROUND
    M_ED( "BackColor",  FO, background_color,       XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_ED( "BackTransparent",    FO, background_color,       XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    M_E( "BackGraphicLocation", STYLE,  position,   MID_FLAG_SPECIAL_ITEM|XML_TYPE_BUILDIN_CMP_ONLY, CTF_BACKGROUND_POS  ),
    M_E( "BackGraphicFilter",STYLE, filter_name,    MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_FILTER ),
    M_E( "BackGraphicURL",  STYLE,  background_image,   MID_FLAG_ELEMENT_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_URL ),
    // RES_BOX
    M_ED( "LeftBorder",         STYLE,  border_line_width,        XML_TYPE_BORDER_WIDTH, CTF_ALLBORDERWIDTH ),
    M_ED( "LeftBorder",         STYLE,  border_line_width_left,   XML_TYPE_BORDER_WIDTH, CTF_LEFTBORDERWIDTH ),
    M_ED( "RightBorder",            STYLE,  border_line_width_right,  XML_TYPE_BORDER_WIDTH, CTF_RIGHTBORDERWIDTH ),
    M_ED( "TopBorder",          STYLE,  border_line_width_top,    XML_TYPE_BORDER_WIDTH, CTF_TOPBORDERWIDTH ),
    M_ED( "BottomBorder",       STYLE,  border_line_width_bottom, XML_TYPE_BORDER_WIDTH, CTF_BOTTOMBORDERWIDTH ),

    M_ED( "LeftBorderDistance", FO,     padding,                  XML_TYPE_MEASURE, CTF_ALLBORDERDISTANCE ), // need special import filtering
    M_ED( "LeftBorderDistance", FO,     padding_left,             XML_TYPE_MEASURE, CTF_LEFTBORDERDISTANCE ),
    M_ED( "RightBorderDistance",    FO,     padding_right,            XML_TYPE_MEASURE, CTF_RIGHTBORDERDISTANCE ),
    M_ED( "TopBorderDistance",  FO,     padding_top,              XML_TYPE_MEASURE, CTF_TOPBORDERDISTANCE ),
    M_ED( "BottomBorderDistance",FO,    padding_bottom,           XML_TYPE_MEASURE, CTF_BOTTOMBORDERDISTANCE ),

    M_ED( "LeftBorder",         FO,     border,                   XML_TYPE_BORDER, CTF_ALLBORDER ),
    M_ED( "LeftBorder",         FO,     border_left,              XML_TYPE_BORDER, CTF_LEFTBORDER ),
    M_ED( "RightBorder",            FO,     border_right,             XML_TYPE_BORDER, CTF_RIGHTBORDER ),
    M_ED( "TopBorder",          FO,     border_top,               XML_TYPE_BORDER, CTF_TOPBORDER ),
    M_ED( "BottomBorder",       FO,     border_bottom,            XML_TYPE_BORDER, CTF_BOTTOMBORDER ),
    // RES_SHADOW
    M_E( "ShadowFormat",        STYLE,  shadow,                 XML_TYPE_TEXT_SHADOW, 0 ),
    // RES_FRMMACRO
    // TODO
    // RES_COL
    M_E( "TextColumns",         STYLE,  columns,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_TEXTCOLUMNS ),
    // RES_KEEP
    // not required
    // RES_URL
    // not required (exprted as draw:a element)
    // RES_EDIT_IN_READONLY
    M_ED( "EditInReadonly",     STYLE,  editable,                   XML_TYPE_BOOL, 0 ),
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required (exported at text:text-box element)
    // RES_LINENUMBER
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_COLUMNBALANCE
    // TODO
    // RES_UNKNOWNATR_CONTAINER
//  M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    // RES_GRFATR_MIRRORGRF (vertical MUST be processed after horizontal!)
    M_E( "HoriMirroredOnEvenPages",     STYLE,  mirror,     XML_TYPE_TEXT_MIRROR_HORIZONTAL_LEFT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "HoriMirroredOnOddPages",      STYLE,  mirror,     XML_TYPE_TEXT_MIRROR_HORIZONTAL_RIGHT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "VertMirrored",        STYLE,  mirror,     XML_TYPE_TEXT_MIRROR_VERTICAL|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    // RES_GRFATR_CROPGRF
    M_E( "GraphicCrop",         FO,     clip,       XML_TYPE_TEXT_CLIP, 0 ),
    // RES_GRFATR_ROTATION
    // not required (exported as svg:transform attribute)
    // RES_GRFATR_LUMINANCE
    { "AdjustLuminance",  XML_NAMESPACE_DRAW, sXML_luminance,           XML_TYPE_PERCENT16, 0 },        // signed?
    // RES_GRFATR_CONTRAST
    { "AdjustContrast", XML_NAMESPACE_DRAW, sXML_contrast,              XML_TYPE_PERCENT16, 0 },        // signed?
    // RES_GRFATR_CHANNELR
    { "AdjustRed",      XML_NAMESPACE_DRAW, sXML_red,                   XML_TYPE_PERCENT16, 0 },        // signed?
    // RES_GRFATR_CHANNELG
    { "AdjustGreen",    XML_NAMESPACE_DRAW, sXML_green,                 XML_TYPE_PERCENT16, 0 },        // signed?
    // RES_GRFATR_CHANNELB
    { "AdjustBlue",     XML_NAMESPACE_DRAW, sXML_blue,                  XML_TYPE_PERCENT16, 0 },        // signed?
    // RES_GRFATR_GAMMA
    { "Gamma",          XML_NAMESPACE_DRAW, sXML_gamma,                 XML_TYPE_DOUBLE, 0 },           // signed?
    // RES_GRFATR_INVERT
    { "GraphicIsInverted", XML_NAMESPACE_DRAW, sXML_color_inversion,        XML_TYPE_BOOL, 0 },
    // RES_GRFATR_TRANSPARENCY
    { "Transparency",   XML_NAMESPACE_DRAW, sXML_transparency,      XML_TYPE_PERCENT16, 0 },
    // RES_GRFATR_DRAWMODE
    { "GraphicColorMode", XML_NAMESPACE_DRAW, sXML_color_mode,          XML_TYPE_COLOR_MODE, 0 },
    // special entries for floating frames
    M_E( "",            DRAW,   frame_display_scrollbar,    XML_TYPE_BOOL|MID_FLAG_NO_PROPERTY, CTF_FRAME_DISPLAY_SCROLLBAR ),
    M_E( "",            DRAW,   frame_display_border,   XML_TYPE_BOOL|MID_FLAG_NO_PROPERTY, CTF_FRAME_DISPLAY_BORDER ),
    M_E( "",            DRAW,   frame_margin_horizontal,    XML_TYPE_MEASURE_PX|MID_FLAG_NO_PROPERTY,   CTF_FRAME_MARGIN_HORI ),
    M_E( "",            DRAW,   frame_margin_vertical,  XML_TYPE_MEASURE_PX|MID_FLAG_NO_PROPERTY,   CTF_FRAME_MARGIN_VERT ),
    M_E( "",            DRAW,   visible_area_left,      XML_TYPE_MEASURE|MID_FLAG_NO_PROPERTY,  CTF_OLE_VIS_AREA_LEFT ),
    M_E( "",            DRAW,   visible_area_top,       XML_TYPE_MEASURE|MID_FLAG_NO_PROPERTY,  CTF_OLE_VIS_AREA_TOP ),
    M_E( "",            DRAW,   visible_area_width,     XML_TYPE_MEASURE|MID_FLAG_NO_PROPERTY,  CTF_OLE_VIS_AREA_WIDTH ),
    M_E( "",            DRAW,   visible_area_height,    XML_TYPE_MEASURE|MID_FLAG_NO_PROPERTY,  CTF_OLE_VIS_AREA_HEIGHT ),
    M_E( "",            DRAW,   draw_aspect,            XML_TYPE_NUMBER16|MID_FLAG_NO_PROPERTY, CTF_OLE_DRAW_ASPECT ),
    M_E( "UserDefinedAttributes", TEXT, xmlns, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    { 0, 0, 0, 0 }
};

XMLPropertyMapEntry aXMLShapePropMap[] =
{
    // RES_LR_SPACE
    M_E( "LeftMargin",              FO, margin_left,        XML_TYPE_MEASURE,  0),
    M_E( "RightMargin",             FO, margin_right,       XML_TYPE_MEASURE, 0 ),
    // RES_UL_SPACE
    M_E( "TopMargin",               FO, margin_top,         XML_TYPE_MEASURE, 0 ),
    M_E( "BottomMargin",            FO, margin_bottom,      XML_TYPE_MEASURE, 0 ),
    // RES_SURROUND
    M_E( "TextWrap",                STYLE,  wrap,   XML_TYPE_TEXT_WRAP, CTF_WRAP ),
    M_E( "SurroundAnchorOnly",      STYLE,  number_wrapped_paragraphs,  XML_TYPE_TEXT_PARAGRAPH_ONLY, CTF_WRAP_PARAGRAPH_ONLY ),
    M_E( "SurroundContour",         STYLE,  wrap_contour,   XML_TYPE_BOOL, CTF_WRAP_CONTOUR ),
    M_E( "ContourOutside",          STYLE,  wrap_contour_mode,  XML_TYPE_TEXT_WRAP_OUTSIDE, CTF_WRAP_CONTOUR_MODE ),
    // RES_VERT_ORIENT
    M_E( "VertOrient",              STYLE,  vertical_pos,         XML_TYPE_TEXT_VERTICAL_POS, CTF_VERTICALPOS ),
    M_E( "VertOrient",              STYLE,  vertical_rel,         XML_TYPE_TEXT_VERTICAL_REL_AS_CHAR|MID_FLAG_MULTI_PROPERTY, CTF_VERTICALREL_ASCHAR ),
    M_E( "VertOrientRelation",      STYLE,  vertical_rel,         XML_TYPE_TEXT_VERTICAL_REL, CTF_VERTICALREL ),
    M_E( "VertOrientRelation",      STYLE,  vertical_rel,         XML_TYPE_TEXT_VERTICAL_REL_PAGE|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_VERTICALREL_PAGE ),
    M_E( "VertOrientRelation",      STYLE,  vertical_rel,         XML_TYPE_TEXT_VERTICAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_VERTICALREL_FRAME ),
    // RES_HORI_ORIENT
    M_E( "HoriOrient",              STYLE,  horizontal_pos,       XML_TYPE_TEXT_HORIZONTAL_POS|MID_FLAG_MULTI_PROPERTY, CTF_HORIZONTALPOS ),
    M_E( "PageToggle",      STYLE,  horizontal_pos,       XML_TYPE_TEXT_HORIZONTAL_MIRROR, CTF_HORIZONTALMIRROR ),
    M_E( "HoriOrient",              STYLE,  horizontal_pos,       XML_TYPE_TEXT_HORIZONTAL_POS_MIRRORED|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_HORIZONTALPOS_MIRRORED ),
    M_E( "HoriOrientRelation",      STYLE,  horizontal_rel,       XML_TYPE_TEXT_HORIZONTAL_REL, CTF_HORIZONTALREL ),
    M_E( "HoriOrientRelation",      STYLE,  horizontal_rel,       XML_TYPE_TEXT_HORIZONTAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_HORIZONTALREL_FRAME ),
    M_E( "UserDefinedAttributes", TEXT, xmlns, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    { 0, 0, 0, 0 }
};

XMLPropertyMapEntry aXMLSectionPropMap[] =
{
    // RES_COL
    M_E( "TextColumns",         STYLE,  columns,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_TEXTCOLUMNS ),

    // RES_BACKGROUND
    M_E( "BackColor",   FO, background_color,       XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    M_E( "BackTransparent", FO, background_color,       XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    M_E( "BackGraphicLocation", STYLE,  position,   MID_FLAG_SPECIAL_ITEM|XML_TYPE_BUILDIN_CMP_ONLY, CTF_BACKGROUND_POS  ),
    M_E( "BackGraphicFilter",STYLE, filter_name,    MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_FILTER ),
    M_E( "BackGraphicURL",  STYLE,  background_image,   MID_FLAG_ELEMENT_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_URL ),

    // move protect-flag into section element
//  M_E( "IsProtected",         STYLE,  protect,    XML_TYPE_BOOL, 0 ),

    // section footnote settings
    M_E( "FootnoteIsOwnNumbering",      TEXT,   _empty, MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL,    CTF_SECTION_FOOTNOTE_NUM_OWN ),
    M_E( "FootnoteIsRestartNumbering",  TEXT,   _empty, MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL,    CTF_SECTION_FOOTNOTE_NUM_RESTART ),
    M_E( "FootnoteRestartNumberingAt",  TEXT,   _empty, MID_FLAG_SPECIAL_ITEM|XML_TYPE_NUMBER16,CTF_SECTION_FOOTNOTE_NUM_RESTART_AT ),
    M_E( "FootnoteNumberingType",       TEXT,   _empty, MID_FLAG_SPECIAL_ITEM|XML_TYPE_NUMBER16,CTF_SECTION_FOOTNOTE_NUM_TYPE ),
    M_E( "FootnoteNumberingPrefix",     TEXT,   _empty, MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING,  CTF_SECTION_FOOTNOTE_NUM_PREFIX ),
    M_E( "FootnoteNumberingSuffix",     TEXT,   _empty, MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING,  CTF_SECTION_FOOTNOTE_NUM_SUFFIX ),
    M_E( "FootnoteIsCollectAtTextEnd",  TEXT,   footnotes_configuration,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_BOOL,    CTF_SECTION_FOOTNOTE_END ),

    // section footnote settings
    M_E( "EndnoteIsOwnNumbering",       TEXT,   _empty,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL,    CTF_SECTION_ENDNOTE_NUM_OWN ),
    M_E( "EndnoteIsRestartNumbering",   TEXT,   _empty,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL,    CTF_SECTION_ENDNOTE_NUM_RESTART ),
    M_E( "EndnoteRestartNumberingAt",   TEXT,   _empty,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_NUMBER16,CTF_SECTION_ENDNOTE_NUM_RESTART_AT ),
    M_E( "EndnoteNumberingType",        TEXT,   _empty,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_NUMBER16,CTF_SECTION_ENDNOTE_NUM_TYPE ),
    M_E( "EndnoteNumberingPrefix",      TEXT,   _empty,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING,  CTF_SECTION_ENDNOTE_NUM_PREFIX ),
    M_E( "EndnoteNumberingSuffix",      TEXT,   _empty,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING,  CTF_SECTION_ENDNOTE_NUM_SUFFIX ),
    M_E( "EndnoteIsCollectAtTextEnd",   TEXT,   endnotes_configuration,     MID_FLAG_ELEMENT_ITEM|XML_TYPE_BOOL,    CTF_SECTION_ENDNOTE_END ),

    { 0, 0, 0, 0 }
};

XMLPropertyMapEntry aXMLRubyPropMap[] =
{
    M_E( "RubyAdjust",  STYLE, ruby_align, XML_TYPE_TEXT_RUBY_ADJUST, 0 ),
    { 0, 0, 0, 0 }
};

XMLPropertyMapEntry *lcl_txtprmap_getMap( sal_uInt16 nType )
{
    XMLPropertyMapEntry *pMap = 0;
    switch( nType )
    {
    case TEXT_PROP_MAP_TEXT:
        pMap = aXMLTextPropMap;
        break;
    case TEXT_PROP_MAP_PARA:
        pMap = aXMLParaPropMap;
        break;
    case TEXT_PROP_MAP_FRAME:
        pMap = aXMLFramePropMap;
        break;
    case TEXT_PROP_MAP_AUTO_FRAME:
        pMap = &(aXMLFramePropMap[11]);
        DBG_ASSERT( pMap->msXMLName == sXML_margin_left, "frame map changed" );
        break;
    case TEXT_PROP_MAP_SHAPE:
        pMap = aXMLShapePropMap;
        break;
    case TEXT_PROP_MAP_SECTION:
        pMap = aXMLSectionPropMap;
        break;
    case TEXT_PROP_MAP_RUBY:
        pMap = aXMLRubyPropMap;
        break;
    }
    DBG_ASSERT( pMap, "illegal map type" );
    return pMap;
}

XMLTextPropertySetMapper::XMLTextPropertySetMapper( sal_uInt16 nType ) :
    XMLPropertySetMapper( lcl_txtprmap_getMap( nType ),
                          new XMLTextPropertyHandlerFactory )
{
}

XMLTextPropertySetMapper::~XMLTextPropertySetMapper()
{
}




