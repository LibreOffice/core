/*************************************************************************
 *
 *  $RCSfile: htmlkywd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:05 $
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <segmentc.hxx>

#include "svparser.hxx"
#include "htmlkywd.hxx"
#include "htmltokn.h"

// die Tabelle muss noch sortiert werden
struct HTML_TokenEntry
{
    union
    {
        const sal_Char *sToken;
        const String *pUToken;
    };
    int nToken;
};

// die Farbnamen werden nicht exportiert
// Sie stammen aus "http://www.uio.no/~mnbjerke/colors_w.html"
// und scheinen im Gegensatz zu denen aus
// "http://www.infi.net/wwwimages/colorindex.html"
// zu stimmen
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_aliceblue, "ALICEBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_antiquewhite, "ANTIQUEWHITE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_aqua, "AQUA" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_aquamarine, "AQUAMARINE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_azure, "AZURE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_beige, "BEIGE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_bisque, "BISQUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_black, "BLACK" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_blanchedalmond, "BLANCHEDALMOND" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_blue, "BLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_blueviolet, "BLUEVIOLET" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_brown, "BROWN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_burlywood, "BURLYWOOD" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_cadetblue, "CADETBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_chartreuse, "CHARTREUSE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_chocolate, "CHOCOLATE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_coral, "CORAL" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_cornflowerblue, "CORNFLOWERBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_cornsilk, "CORNSILK" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_crimson, "CRIMSON" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_cyan, "CYAN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkblue, "DARKBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkcyan, "DARKCYAN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkgoldenrod, "DARKGOLDENROD" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkgray, "DARKGRAY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkgreen, "DARKGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkkhaki, "DARKKHAKI" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkmagenta, "DARKMAGENTA" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkolivegreen, "DARKOLIVEGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkorange, "DARKORANGE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkorchid, "DARKORCHID" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkred, "DARKRED" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darksalmon, "DARKSALMON" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkseagreen, "DARKSEAGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkslateblue, "DARKSLATEBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkslategray, "DARKSLATEGRAY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkturquoise, "DARKTURQUOISE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_darkviolet, "DARKVIOLET" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_deeppink, "DEEPPINK" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_deepskyblue, "DEEPSKYBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_dimgray, "DIMGRAY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_dodgerblue, "DODGERBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_firebrick, "FIREBRICK" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_floralwhite, "FLORALWHITE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_forestgreen, "FORESTGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_fuchsia, "FUCHSIA" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_gainsboro, "GAINSBORO" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_ghostwhite, "GHOSTWHITE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_gold, "GOLD" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_goldenrod, "GOLDENROD" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_gray, "GRAY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_green, "GREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_greenyellow, "GREENYELLOW" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_honeydew, "HONEYDEW" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_hotpink, "HOTPINK" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_indianred, "INDIANRED" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_indigo, "INDIGO" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_ivory, "IVORY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_khaki, "KHAKI" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lavender, "LAVENDER" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lavenderblush, "LAVENDERBLUSH" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lawngreen, "LAWNGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lemonchiffon, "LEMONCHIFFON" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightblue, "LIGHTBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightcoral, "LIGHTCORAL" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightcyan, "LIGHTCYAN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightgoldenrodyellow, "LIGHTGOLDENRODYELLOW" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightgreen, "LIGHTGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightgrey, "LIGHTGREY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightpink, "LIGHTPINK" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightsalmon, "LIGHTSALMON" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightseagreen, "LIGHTSEAGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightskyblue, "LIGHTSKYBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightslategray, "LIGHTSLATEGRAY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightsteelblue, "LIGHTSTEELBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lightyellow, "LIGHTYELLOW" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_lime, "LIME" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_limegreen, "LIMEGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_linen, "LINEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_magenta, "MAGENTA" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_maroon, "MAROON" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumaquamarine, "MEDIUMAQUAMARINE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumblue, "MEDIUMBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumorchid, "MEDIUMORCHID" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumpurple, "MEDIUMPURPLE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumseagreen, "MEDIUMSEAGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumslateblue, "MEDIUMSLATEBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumspringgreen, "MEDIUMSPRINGGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumturquoise, "MEDIUMTURQUOISE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mediumvioletred, "MEDIUMVIOLETRED" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_midnightblue, "MIDNIGHTBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mintcream, "MINTCREAM" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_mistyrose, "MISTYROSE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_moccasin, "MOCCASIN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_navajowhite, "NAVAJOWHITE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_navy, "NAVY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_oldlace, "OLDLACE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_olive, "OLIVE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_olivedrab, "OLIVEDRAB" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_orange, "ORANGE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_orangered, "ORANGERED" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_orchid, "ORCHID" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_palegoldenrod, "PALEGOLDENROD" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_palegreen, "PALEGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_paleturquoise, "PALETURQUOISE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_palevioletred, "PALEVIOLETRED" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_papayawhip, "PAPAYAWHIP" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_peachpuff, "PEACHPUFF" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_peru, "PERU" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_pink, "PINK" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_plum, "PLUM" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_powderblue, "POWDERBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_purple, "PURPLE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_red, "RED" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_rosybrown, "ROSYBROWN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_royalblue, "ROYALBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_saddlebrown, "SADDLEBROWN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_salmon, "SALMON" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_sandybrown, "SANDYBROWN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_seagreen, "SEAGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_seashell, "SEASHELL" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_sienna, "SIENNA" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_silver, "SILVER" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_skyblue, "SKYBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_slateblue, "SLATEBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_slategray, "SLATEGRAY" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_snow, "SNOW" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_springgreen, "SPRINGGREEN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_steelblue, "STEELBLUE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_tan, "TAN" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_teal, "TEAL" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_thistle, "THISTLE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_tomato, "TOMATO" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_turquoise, "TURQUOISE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_violet, "VIOLET" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_wheat, "WHEAT" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_white, "WHITE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_whitesmoke, "WHITESMOKE" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_yellow, "YELLOW" );
sal_Char __READONLY_DATA SVTOOLS_CONSTASCII_DEF( sHTML_COL_yellowgreen, "YELLOWGREEN" );


// Flag: RTF-Token Tabelle wurde schon sortiert
static int __FAR_DATA bSortKeyWords = FALSE;

static HTML_TokenEntry __FAR_DATA aHTMLTokenTab[] = {
sHTML_area,         HTML_AREA, // Netscape 2.0
sHTML_base,         HTML_BASE, // HTML 3.0
sHTML_comment,      HTML_COMMENT,
sHTML_doctype,      HTML_DOCTYPE,
sHTML_embed,        HTML_EMBED, // Netscape 2.0
sHTML_figureoverlay,HTML_FIGUREOVERLAY, // HTML 3.0
sHTML_horzrule,     HTML_HORZRULE,
sHTML_horztab,      HTML_HORZTAB, // HTML 3.0
sHTML_image,        HTML_IMAGE,
sHTML_image2,        HTML_IMAGE,
sHTML_input,        HTML_INPUT,
sHTML_isindex,      HTML_ISINDEX, // HTML 3.0
sHTML_li,           HTML_LI_ON,
sHTML_linebreak,    HTML_LINEBREAK,
sHTML_link,         HTML_LINK, // HTML 3.0
sHTML_meta,         HTML_META, // HTML 3.0
sHTML_nextid,       HTML_NEXTID, // HTML 3.0
sHTML_of,           HTML_OF, // HTML 3.0
sHTML_option,       HTML_OPTION,
sHTML_param,        HTML_PARAM, // HotJava
sHTML_range,        HTML_RANGE, // HTML 3.0
sHTML_spacer,       HTML_SPACER, // Netscape 3.0b5
sHTML_wbr,          HTML_WBR, // Netscape

sHTML_abbreviation, HTML_ABBREVIATION_ON, // HTML 3.0
sHTML_above,        HTML_ABOVE_ON, // HTML 3.0
sHTML_acronym,      HTML_ACRONYM_ON, // HTML 3.0
sHTML_address,      HTML_ADDRESS_ON,
sHTML_anchor,       HTML_ANCHOR_ON,
sHTML_applet,       HTML_APPLET_ON, // HotJava
sHTML_array,        HTML_ARRAY_ON, // HTML 3.0
sHTML_author,       HTML_AUTHOR_ON, // HTML 3.0
sHTML_banner,       HTML_BANNER_ON, // HTML 3.0
sHTML_bar,          HTML_BAR_ON, // HTML 3.0
sHTML_basefont,     HTML_BASEFONT_ON, // Netscape
sHTML_below,        HTML_BELOW_ON, // HTML 3.0
sHTML_bigprint,     HTML_BIGPRINT_ON, // HTML 3.0
sHTML_blink,        HTML_BLINK_ON, // Netscape
sHTML_blockquote,   HTML_BLOCKQUOTE_ON,
sHTML_blockquote30, HTML_BLOCKQUOTE30_ON, // HTML 3.0
sHTML_body,         HTML_BODY_ON,
sHTML_bold,         HTML_BOLD_ON,
sHTML_boldtext,     HTML_BOLDTEXT_ON, // HTML 3.0
sHTML_box,          HTML_BOX_ON, // HTML 3.0
sHTML_caption,      HTML_CAPTION_ON, // HTML 3.0
sHTML_center,       HTML_CENTER_ON, // Netscape
sHTML_citiation,    HTML_CITIATION_ON,
sHTML_col,          HTML_COL_ON, // HTML 3 Table Model Draft
sHTML_colgroup,     HTML_COLGROUP_ON, // HTML 3 Table Model Draft
sHTML_code,         HTML_CODE_ON,
sHTML_credit,       HTML_CREDIT_ON, // HTML 3.0
sHTML_dd,           HTML_DD_ON,
sHTML_deflist,      HTML_DEFLIST_ON,
sHTML_deletedtext,  HTML_DELETEDTEXT_ON, // HTML 3.0
sHTML_dirlist,      HTML_DIRLIST_ON,
sHTML_division,     HTML_DIVISION_ON, // HTML 3.0
sHTML_dot,          HTML_DOT_ON, // HTML 3.0
sHTML_doubledot,    HTML_DOUBLEDOT_ON, // HTML 3.0
sHTML_dt,           HTML_DT_ON,
sHTML_emphasis,     HTML_EMPHASIS_ON,
sHTML_figure,       HTML_FIGURE_ON, // HTML 3.0
sHTML_font,         HTML_FONT_ON, // Netscpe
sHTML_footnote,     HTML_FOOTNOTE_ON, // HTML 3.0
sHTML_form,         HTML_FORM_ON,
sHTML_frame,        HTML_FRAME_ON, // Netscape 2.0
sHTML_frameset,     HTML_FRAMESET_ON, // Netscape 2.0
sHTML_hat,          HTML_HAT_ON, // HTML 3.0
sHTML_head,         HTML_HEAD_ON,
sHTML_head1,        HTML_HEAD1_ON,
sHTML_head2,        HTML_HEAD2_ON,
sHTML_head3,        HTML_HEAD3_ON,
sHTML_head4,        HTML_HEAD4_ON,
sHTML_head5,        HTML_HEAD5_ON,
sHTML_head6,        HTML_HEAD6_ON,
sHTML_html,         HTML_HTML_ON,
sHTML_iframe,       HTML_IFRAME_ON, // IE 3.0b2
sHTML_ilayer,       HTML_ILAYER_ON,
sHTML_insertedtext, HTML_INSERTEDTEXT_ON, // HTML 3.0
sHTML_italic,       HTML_ITALIC_ON,
sHTML_item,         HTML_ITEM_ON, // HTML 3.0
sHTML_keyboard,     HTML_KEYBOARD_ON,
sHTML_language,     HTML_LANGUAGE_ON, // HTML 3.0
sHTML_layer,        HTML_LAYER_ON,
sHTML_listheader,   HTML_LISTHEADER_ON, // HTML 3.0
sHTML_map,          HTML_MAP_ON, // Netscape 2.0
sHTML_math,         HTML_MATH_ON, // HTML 3.0
sHTML_menulist,     HTML_MENULIST_ON,
sHTML_multicol,     HTML_MULTICOL_ON, // Netscape 3.0b5
sHTML_nobr,         HTML_NOBR_ON, // Netscape
sHTML_noembed,      HTML_NOEMBED_ON, // Netscape 2.0 ???
sHTML_noframe,      HTML_NOFRAMES_ON, // Netscape 2.0 ???
sHTML_noframes,     HTML_NOFRAMES_ON, // Netscape 2.0
sHTML_noscript,     HTML_NOSCRIPT_ON, // Netscape 3.0
sHTML_note,         HTML_NOTE_ON, // HTML 3.0
sHTML_orderlist,    HTML_ORDERLIST_ON,
sHTML_parabreak,    HTML_PARABREAK_ON,
sHTML_person,       HTML_PERSON_ON, // HTML 3.0
sHTML_plaintext,    HTML_PLAINTEXT_ON, // HTML 3.0
sHTML_preformtxt,   HTML_PREFORMTXT_ON,
sHTML_root,         HTML_ROOT_ON, // HTML 3.0
sHTML_row,          HTML_ROW_ON, // HTML 3.0
sHTML_sample,       HTML_SAMPLE_ON,
sHTML_script,       HTML_SCRIPT_ON, // HTML 3.2
sHTML_select,       HTML_SELECT_ON,
sHTML_shortquote,   HTML_SHORTQUOTE_ON, // HTML 3.0
sHTML_smallprint,   HTML_SMALLPRINT_ON, // HTML 3.0
sHTML_span,         HTML_SPAN_ON, // Style Sheets
sHTML_squareroot,   HTML_SQUAREROOT_ON, // HTML 3.0
sHTML_strikethrough,HTML_STRIKETHROUGH_ON, // HTML 3.0
sHTML_strong,       HTML_STRONG_ON,
sHTML_style,        HTML_STYLE_ON, // HTML 3.0
sHTML_subscript,    HTML_SUBSCRIPT_ON, // HTML 3.0
sHTML_superscript,  HTML_SUPERSCRIPT_ON, // HTML 3.0
sHTML_table,        HTML_TABLE_ON, // HTML 3.0
sHTML_tabledata,    HTML_TABLEDATA_ON, // HTML 3.0
sHTML_tableheader,  HTML_TABLEHEADER_ON, // HTML 3.0
sHTML_tablerow,     HTML_TABLEROW_ON, // HTML 3.0
sHTML_tbody,        HTML_TBODY_ON, // HTML 3 Table Model Draft
sHTML_teletype,     HTML_TELETYPE_ON,
sHTML_text,         HTML_TEXT_ON, // HTML 3.0
sHTML_textarea,     HTML_TEXTAREA_ON,
sHTML_textflow,     HTML_TEXTFLOW_ON,
sHTML_tfoot,        HTML_TFOOT_ON, // HTML 3 Table Model Draft
sHTML_thead,        HTML_THEAD_ON, // HTML 3 Table Model Draft
sHTML_tilde,        HTML_TILDE_ON, // HTML 3.0
sHTML_title,        HTML_TITLE_ON,
sHTML_underline,    HTML_UNDERLINE_ON,
sHTML_unorderlist,  HTML_UNORDERLIST_ON,
sHTML_variable,     HTML_VARIABLE_ON,
sHTML_vector,       HTML_VECTOR_ON, // HTML 3.0

sHTML_xmp,          HTML_XMP_ON,
sHTML_listing,      HTML_LISTING_ON,

sHTML_definstance,  HTML_DEFINSTANCE_ON,
sHTML_strike,       HTML_STRIKE_ON,

sHTML_bgsound,      HTML_BGSOUND,
sHTML_comment2,     HTML_COMMENT2_ON,
sHTML_marquee,      HTML_MARQUEE_ON,
sHTML_plaintext2,   HTML_PLAINTEXT2_ON,

sHTML_sdfield,      HTML_SDFIELD_ON,
};


extern "C"
{

static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
 _Optlink
#endif
    HTMLKeyCompare( const void *pFirst, const void *pSecond)
{
    int nRet = 0;
    if( -1 == ((HTML_TokenEntry*)pFirst)->nToken )
    {
        if( -1 == ((HTML_TokenEntry*)pSecond)->nToken )
            nRet = ((HTML_TokenEntry*)pFirst)->pUToken->CompareTo(
                            *((HTML_TokenEntry*)pSecond)->pUToken );
        else
            nRet = ((HTML_TokenEntry*)pFirst)->pUToken->CompareToAscii(
                            ((HTML_TokenEntry*)pSecond)->sToken );
    }
    else
    {
        if( -1 == ((HTML_TokenEntry*)pSecond)->nToken )
            nRet = -1 * ((HTML_TokenEntry*)pSecond)->pUToken->CompareToAscii(
                            ((HTML_TokenEntry*)pFirst)->sToken );
        else
            nRet = strcmp( ((HTML_TokenEntry*)pFirst)->sToken,
                            ((HTML_TokenEntry*)pSecond)->sToken );
    }

    return nRet;
}

}

int GetHTMLToken( const String& rName )
{
    if( !bSortKeyWords )
    {
        qsort( (void*) aHTMLTokenTab,
                sizeof( aHTMLTokenTab ) / sizeof( HTML_TokenEntry ),
                sizeof( HTML_TokenEntry ),
                HTMLKeyCompare );
        bSortKeyWords = TRUE;
    }

    int nRet = 0;

    if( !rName.CompareToAscii( sHTML_comment, 3UL) )
        return HTML_COMMENT;

    void* pFound;
    HTML_TokenEntry aSrch;
    aSrch.pUToken = &rName;
    aSrch.nToken = -1;

    if( 0 != ( pFound = bsearch( (sal_Char *) &aSrch,
                        (void*) aHTMLTokenTab,
                        sizeof( aHTMLTokenTab ) / sizeof( HTML_TokenEntry ),
                        sizeof( HTML_TokenEntry ),
                        HTMLKeyCompare )))
        nRet = ((HTML_TokenEntry*)pFound)->nToken;
    return nRet;
}

/**/

struct HTML_CharEntry
{
    union
    {
        const sal_Char *sName;
        const String *pUName;
    };
    sal_Unicode cChar;
};

// Flag: RTF-Token Tabelle wurde schon sortiert
static int __FAR_DATA bSortCharKeyWords = FALSE;

static HTML_CharEntry __FAR_DATA aHTMLCharNameTab[] = {
sHTML_C_lt,          60,
sHTML_C_gt,          62,
sHTML_C_amp,         38,
sHTML_C_quot,        34,

sHTML_C_Agrave,     192,
sHTML_C_Aacute,     193,
sHTML_C_Acirc,      194,
sHTML_C_Atilde,     195,
sHTML_C_Auml,       196,
sHTML_C_Aring,      197,
sHTML_C_AElig,      198,
sHTML_C_Ccedil,     199,
sHTML_C_Egrave,     200,
sHTML_C_Eacute,     201,
sHTML_C_Ecirc,      202,
sHTML_C_Euml,       203,
sHTML_C_Igrave,     204,
sHTML_C_Iacute,     205,
sHTML_C_Icirc,      206,
sHTML_C_Iuml,       207,
sHTML_C_ETH,        208,
sHTML_C_Ntilde,     209,
sHTML_C_Ograve,     210,
sHTML_C_Oacute,     211,
sHTML_C_Ocirc,      212,
sHTML_C_Otilde,     213,
sHTML_C_Ouml,       214,
sHTML_C_Oslash,     216,
sHTML_C_Ugrave,     217,
sHTML_C_Uacute,     218,
sHTML_C_Ucirc,      219,
sHTML_C_Uuml,       220,
sHTML_C_Yacute,     221,

sHTML_C_THORN,      222,
sHTML_C_szlig,      223,

sHTML_S_agrave,     224,
sHTML_S_aacute,     225,
sHTML_S_acirc,      226,
sHTML_S_atilde,     227,
sHTML_S_auml,       228,
sHTML_S_aring,      229,
sHTML_S_aelig,      230,
sHTML_S_ccedil,     231,
sHTML_S_egrave,     232,
sHTML_S_eacute,     233,
sHTML_S_ecirc,      234,
sHTML_S_euml,       235,
sHTML_S_igrave,     236,
sHTML_S_iacute,     237,
sHTML_S_icirc,      238,
sHTML_S_iuml,       239,
sHTML_S_eth,        240,
sHTML_S_ntilde,     241,
sHTML_S_ograve,     242,
sHTML_S_oacute,     243,
sHTML_S_ocirc,      244,
sHTML_S_otilde,     245,
sHTML_S_ouml,       246,
sHTML_S_oslash,     248,
sHTML_S_ugrave,     249,
sHTML_S_uacute,     250,
sHTML_S_ucirc,      251,
sHTML_S_uuml,       252,
sHTML_S_yacute,     253,
sHTML_S_thorn,      254,
sHTML_S_yuml,       255,

// Sonderzeichen
sHTML_S_acute,      180,
sHTML_S_brvbar,     166,
sHTML_S_cedil,      184,
sHTML_S_cent,       162,
sHTML_S_copy,       169,
sHTML_S_curren,     164,
sHTML_S_deg,        176,
sHTML_S_divide,     247,
sHTML_S_frac12,     189,
sHTML_S_frac14,     188,
sHTML_S_frac34,     190,
sHTML_S_iexcl,      161,
sHTML_S_iquest,     191,
sHTML_S_laquo,      171,
sHTML_S_macr,       175,
sHTML_S_micro,      181,
sHTML_S_middot,     183,
sHTML_S_not,        172,
sHTML_S_ordf,       170,
sHTML_S_ordm,       186,
sHTML_S_para,       182,
sHTML_S_plusmn,     177,
sHTML_S_pound,      163,
sHTML_S_raquo,      187,
sHTML_S_reg,        174,
sHTML_S_sect,       167,
sHTML_S_sup1,       185,
sHTML_S_sup2,       178,
sHTML_S_sup3,       179,
sHTML_S_times,      215,
sHTML_S_uml,        168,
sHTML_S_yen,        165,

// Netscape kennt noch ein paar in Grossbuchstaben ...
sHTML_C_LT,          60,
sHTML_C_GT,          62,
sHTML_C_AMP,         38,
sHTML_C_QUOT,        34,
sHTML_S_COPY,       169,
sHTML_S_REG,        174,

// Sonderzeichen, die zu Tokens konvertiert werden !!!
sHTML_S_nbsp,           1,
sHTML_S_shy,            2,
};

extern "C"
{

static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
 _Optlink
#endif
    HTMLCharNameCompare( const void *pFirst, const void *pSecond)
{
    int nRet = 0;
    if( USHRT_MAX == ((HTML_CharEntry*)pFirst)->cChar )
    {
        if( USHRT_MAX == ((HTML_CharEntry*)pSecond)->cChar )
            nRet = ((HTML_CharEntry*)pFirst)->pUName->CompareTo(
                            *((HTML_CharEntry*)pSecond)->pUName );
        else
            nRet = ((HTML_CharEntry*)pFirst)->pUName->CompareToAscii(
                            ((HTML_CharEntry*)pSecond)->sName );
    }
    else
    {
        if( USHRT_MAX == ((HTML_CharEntry*)pSecond)->cChar )
            nRet = -1 * ((HTML_CharEntry*)pSecond)->pUName->CompareToAscii(
                            ((HTML_CharEntry*)pFirst)->sName );
        else
            nRet = strcmp( ((HTML_CharEntry*)pFirst)->sName,
                            ((HTML_CharEntry*)pSecond)->sName );
    }

    return nRet;
}

}

sal_Unicode GetHTMLCharName( const String& rName )
{
    if( !bSortCharKeyWords )
    {
        qsort( (void*) aHTMLCharNameTab,
                sizeof( aHTMLCharNameTab ) / sizeof( HTML_CharEntry ),
                sizeof( HTML_CharEntry ),
                HTMLCharNameCompare );
        bSortCharKeyWords = TRUE;
    }

    sal_Unicode cRet = 0;
    void* pFound;
    HTML_CharEntry aSrch;
    aSrch.pUName = &rName;
    aSrch.cChar = USHRT_MAX;

    if( 0 != ( pFound = bsearch( (sal_Char *) &aSrch,
                        (void*) aHTMLCharNameTab,
                        sizeof( aHTMLCharNameTab) / sizeof( HTML_CharEntry ),
                        sizeof( HTML_CharEntry ),
                        HTMLCharNameCompare )))
        cRet = ((HTML_CharEntry*)pFound)->cChar;
    return cRet;
}

/**/

// Flag: Optionen-Tabelle wurde schon sortiert
static int __FAR_DATA bSortOptionKeyWords = FALSE;

static HTML_TokenEntry __FAR_DATA aHTMLOptionTab[] = {

// Attribute ohne Wert
sHTML_O_box,        HTML_O_BOX,
sHTML_O_checked,    HTML_O_CHECKED,
sHTML_O_compact,    HTML_O_COMPACT,
sHTML_O_continue,   HTML_O_CONTINUE,
sHTML_O_controls,   HTML_O_CONTROLS, // IExplorer 2.0
sHTML_O_declare,    HTML_O_DECLARE,
sHTML_O_disabled,   HTML_O_DISABLED,
sHTML_O_folded,     HTML_O_FOLDED, // Netscape internal
sHTML_O_ismap,      HTML_O_ISMAP,
sHTML_O_mayscript,  HTML_O_MAYSCRIPT,
sHTML_O_multiple,   HTML_O_MULTIPLE,
sHTML_O_noflow,     HTML_O_NOFLOW,
sHTML_O_nohref,     HTML_O_NOHREF, // Netscape 2.0
sHTML_O_noresize,   HTML_O_NORESIZE, // Netscape 2.0
sHTML_O_noshade,    HTML_O_NOSHADE, // Netscape 2.0
sHTML_O_nowrap,     HTML_O_NOWRAP,
sHTML_O_plain,      HTML_O_PLAIN,
sHTML_O_sdfixed,    HTML_O_SDFIXED,
sHTML_O_selected,   HTML_O_SELECTED,
sHTML_O_shapes,     HTML_O_SHAPES,

// Attribute mit einem String als Wert
sHTML_O_above,      HTML_O_ABOVE,
sHTML_O_accept,     HTML_O_ACCEPT,
sHTML_O_accesskey,  HTML_O_ACCESSKEY,
sHTML_O_add_date,   HTML_O_ADD_DATE, // Netscape internal
sHTML_O_alt,        HTML_O_ALT,
sHTML_O_axes,       HTML_O_AXES,
sHTML_O_axis,       HTML_O_AXIS,
sHTML_O_below,      HTML_O_BELOW,
sHTML_O_char,       HTML_O_CHAR, // HTML 3 Table Model Draft
sHTML_O_class,      HTML_O_CLASS,
sHTML_O_clip,       HTML_O_CLIP,
sHTML_O_code,       HTML_O_CODE, // HotJava
sHTML_O_codetype,   HTML_O_CODETYPE,
sHTML_O_colspec,    HTML_O_COLSPEC,
sHTML_O_content,    HTML_O_CONTENT,
sHTML_O_coords,     HTML_O_COORDS, // Netscape 2.0
sHTML_O_dp,         HTML_O_DP,
sHTML_O_enctype,    HTML_O_ENCTYPE,
sHTML_O_error,      HTML_O_ERROR,
sHTML_O_face,       HTML_O_FACE, // IExplorer 2.0
sHTML_O_frameborder, HTML_O_FRAMEBORDER, // IExplorer 3.0
sHTML_O_httpequiv,  HTML_O_HTTPEQUIV,
sHTML_O_language,   HTML_O_LANGUAGE, // JavaScript
sHTML_O_last_modified,  HTML_O_LAST_MODIFIED, // Netscape internal
sHTML_O_last_visit, HTML_O_LAST_VISIT, // Netscape internal
sHTML_O_md,         HTML_O_MD,
sHTML_O_n,          HTML_O_N,
sHTML_O_name,       HTML_O_NAME,
sHTML_O_notation,   HTML_O_NOTATION,
sHTML_O_prompt,     HTML_O_PROMPT,
sHTML_O_shape,      HTML_O_SHAPE,
sHTML_O_standby,    HTML_O_STANDBY,
sHTML_O_style,      HTML_O_STYLE,
sHTML_O_title,      HTML_O_TITLE,
sHTML_O_value,      HTML_O_VALUE,
sHTML_O_SDval,      HTML_O_SDVAL, // StarDiv NumberValue
sHTML_O_SDnum,      HTML_O_SDNUM, // StarDiv NumberFormat
sHTML_O_sdlibrary,  HTML_O_SDLIBRARY,
sHTML_O_sdmodule,   HTML_O_SDMODULE,

// Attribute mit einem SGML-Identifier als Wert
sHTML_O_from,       HTML_O_FROM,
sHTML_O_id,         HTML_O_ID,
sHTML_O_target,     HTML_O_TARGET, // Netscape 2.0
sHTML_O_to,         HTML_O_TO,
sHTML_O_until,      HTML_O_UNTIL,

// Attribute mit einem URI als Wert
sHTML_O_action,     HTML_O_ACTION,
sHTML_O_background, HTML_O_BACKGROUND,
sHTML_O_classid,    HTML_O_CLASSID,
sHTML_O_codebase,   HTML_O_CODEBASE, // HotJava
sHTML_O_data,       HTML_O_DATA,
sHTML_O_dynsrc,     HTML_O_DYNSRC, // IExplorer 3.0b1
sHTML_O_dynsync,    HTML_O_DYNSYNC, // IExplorer 2.0
sHTML_O_imagemap,   HTML_O_IMAGEMAP,
sHTML_O_href,       HTML_O_HREF,
sHTML_O_lowsrc,     HTML_O_LOWSRC, // Netscape 3.0
sHTML_O_script,     HTML_O_SCRIPT,
sHTML_O_src,        HTML_O_SRC,
sHTML_O_usemap,     HTML_O_USEMAP, // Netscape 2.0

// Attribute mit Entity-Namen als Wert
sHTML_O_dingbat,    HTML_O_DINGBAT,
sHTML_O_sym,        HTML_O_SYM,

// Attribute mit einer Farbe als Wert (alle Netscape)
sHTML_O_alink,      HTML_O_ALINK,
sHTML_O_bgcolor,    HTML_O_BGCOLOR,
sHTML_O_bordercolor, HTML_O_BORDERCOLOR, // IExplorer 2.0
sHTML_O_bordercolorlight, HTML_O_BORDERCOLORLIGHT, // IExplorer 2.0
sHTML_O_bordercolordark, HTML_O_BORDERCOLORDARK, // IExplorer 2.0
sHTML_O_color,      HTML_O_COLOR,
sHTML_O_link,       HTML_O_LINK,
sHTML_O_text,       HTML_O_TEXT,
sHTML_O_vlink,      HTML_O_VLINK,

// Attribute mit einem numerischen Wert
sHTML_O_border,     HTML_O_BORDER,
sHTML_O_cellspacing,HTML_O_CELLSPACING, // HTML 3 Table Model Draft
sHTML_O_cellpadding,HTML_O_CELLPADDING, // HTML 3 Table Model Draft
sHTML_O_charoff,    HTML_O_CHAROFF, // HTML 3 Table Model Draft
sHTML_O_colspan,    HTML_O_COLSPAN,
sHTML_O_framespacing, HTML_O_FRAMESPACING, // IExplorer 3.0
sHTML_O_gutter,     HTML_O_GUTTER, // Netscape 3.0b5
sHTML_O_indent,     HTML_O_INDENT,
sHTML_O_height,     HTML_O_HEIGHT,
sHTML_O_hspace,     HTML_O_HSPACE, // Netscape
sHTML_O_left,       HTML_O_LEFT,
sHTML_O_leftmargin, HTML_O_LEFTMARGIN, // IExplorer 2.0
sHTML_O_loop,       HTML_O_LOOP, // IExplorer 2.0
sHTML_O_marginheight,HTML_O_MARGINHEIGHT, // Netscape 2.0
sHTML_O_marginwidth,HTML_O_MARGINWIDTH, // Netscape 2.0
sHTML_O_max,        HTML_O_MAX,
sHTML_O_maxlength,  HTML_O_MAXLENGTH,
sHTML_O_min,        HTML_O_MIN,
sHTML_O_pagex,      HTML_O_PAGEX,
sHTML_O_pagey,      HTML_O_PAGEY,
sHTML_O_pointsize,  HTML_O_POINTSIZE,
sHTML_O_rowspan,    HTML_O_ROWSPAN,
sHTML_O_scrollamount, HTML_O_SCROLLAMOUNT, // IExplorer 2.0
sHTML_O_scrolldelay, HTML_O_SCROLLDELAY, // IExplorer 2.0
sHTML_O_seqnum,     HTML_O_SEQNUM,
sHTML_O_skip,       HTML_O_SKIP,
sHTML_O_span,       HTML_O_SPAN, // HTML 3 Table Model Draft
sHTML_O_tabindex,   HTML_O_TABINDEX,
sHTML_O_top,        HTML_O_TOP,
sHTML_O_topmargin,  HTML_O_TOPMARGIN, // IExplorer 2.0
sHTML_O_vspace,     HTML_O_VSPACE, // Netscape
sHTML_O_weight,     HTML_O_WEIGHT,
sHTML_O_width,      HTML_O_WIDTH,
sHTML_O_x,          HTML_O_X,
sHTML_O_y,          HTML_O_Y,
sHTML_O_zindex,     HTML_O_ZINDEX,

// Attribute mit Enum-Werten
sHTML_O_bgproperties, HTML_O_BGPROPERTIES, // IExplorer 2.0
sHTML_O_behavior,   HTML_O_BEHAVIOR, // IExplorer 2.0
sHTML_O_clear,      HTML_O_CLEAR,
sHTML_O_dir,        HTML_O_DIR, // HTML 3 Table Model Draft
sHTML_O_direction,  HTML_O_DIRECTION, // IExplorer 2.0
sHTML_O_format,     HTML_O_FORMAT,
sHTML_O_frame,      HTML_O_FRAME, // HTML 3 Table Model Draft
sHTML_O_lang,       HTML_O_LANG,
sHTML_O_method,     HTML_O_METHOD,
sHTML_O_palette,    HTML_O_PALETTE,
sHTML_O_rel,        HTML_O_REL,
sHTML_O_rev,        HTML_O_REV,
sHTML_O_rules,      HTML_O_RULES, // HTML 3 Table Model Draft
sHTML_O_scrolling,  HTML_O_SCROLLING, // Netscape 2.0
sHTML_O_sdreadonly, HTML_O_SDREADONLY,
sHTML_O_subtype,    HTML_O_SUBTYPE,
sHTML_O_type,       HTML_O_TYPE,
sHTML_O_valign,     HTML_O_VALIGN,
sHTML_O_valuetype,  HTML_O_VALUETYPE,
sHTML_O_wrap,       HTML_O_WRAP,
sHTML_O_visibility, HTML_O_VISIBILITY,

// Attribute mit Script-Code als Wert
sHTML_O_onblur,     HTML_O_ONBLUR, // JavaScript
sHTML_O_onchange,   HTML_O_ONCHANGE, // JavaScript
sHTML_O_onclick,    HTML_O_ONCLICK, // JavaScript
sHTML_O_onfocus,    HTML_O_ONFOCUS, // JavaScript
sHTML_O_onload,     HTML_O_ONLOAD, // JavaScript
sHTML_O_onmouseover, HTML_O_ONMOUSEOVER, // JavaScript
sHTML_O_onreset,    HTML_O_ONRESET, // JavaScript
sHTML_O_onselect,   HTML_O_ONSELECT, // JavaScript
sHTML_O_onsubmit,   HTML_O_ONSUBMIT, // JavaScript
sHTML_O_onunload,   HTML_O_ONUNLOAD, // JavaScript
sHTML_O_onabort,    HTML_O_ONABORT, // JavaScript
sHTML_O_onerror,    HTML_O_ONERROR, // JavaScript
sHTML_O_onmouseout, HTML_O_ONMOUSEOUT, // JavaScript

sHTML_O_SDonblur,       HTML_O_SDONBLUR, // StarBasic
sHTML_O_SDonchange,     HTML_O_SDONCHANGE, // StarBasic
sHTML_O_SDonclick,      HTML_O_SDONCLICK, // StarBasic
sHTML_O_SDonfocus,      HTML_O_SDONFOCUS, // StarBasic
sHTML_O_SDonload,       HTML_O_SDONLOAD, // StarBasic
sHTML_O_SDonmouseover,  HTML_O_SDONMOUSEOVER, // StarBasic
sHTML_O_SDonreset,      HTML_O_SDONRESET, // StarBasic
sHTML_O_SDonselect,     HTML_O_SDONSELECT, // StarBasic
sHTML_O_SDonsubmit,     HTML_O_SDONSUBMIT, // StarBasic
sHTML_O_SDonunload,     HTML_O_SDONUNLOAD, // StarBasic
sHTML_O_SDonabort,      HTML_O_SDONABORT, // StarBasic
sHTML_O_SDonerror,      HTML_O_SDONERROR, // StarBasic
sHTML_O_SDonmouseout,   HTML_O_SDONMOUSEOUT, // StarBasic

// Attribute mit Kontext-abhaengigen Werten
sHTML_O_align,      HTML_O_ALIGN,
sHTML_O_cols,       HTML_O_COLS, // Netscape 2.0 vs HTML 2.0
sHTML_O_rows,       HTML_O_ROWS, // Netscape 2.0 vs HTML 2.0
sHTML_O_size,       HTML_O_SIZE,
sHTML_O_start,      HTML_O_START, // Netscape 2.0 vs IExplorer 2.0
sHTML_O_units,      HTML_O_UNITS
};

int GetHTMLOption( const String& rName )
{
    if( !bSortOptionKeyWords )
    {
        qsort( (void*) aHTMLOptionTab,
                sizeof( aHTMLOptionTab ) / sizeof( HTML_TokenEntry ),
                sizeof( HTML_TokenEntry ),
                HTMLKeyCompare );
        bSortOptionKeyWords = TRUE;
    }

    int nRet = HTML_O_UNKNOWN;
    void* pFound;
    HTML_TokenEntry aSrch;
    aSrch.pUToken = &rName;
    aSrch.nToken = -1;

    if( 0 != ( pFound = bsearch( (sal_Char *) &aSrch,
                        (void*) aHTMLOptionTab,
                        sizeof( aHTMLOptionTab ) / sizeof( HTML_TokenEntry ),
                        sizeof( HTML_TokenEntry ),
                        HTMLKeyCompare )))
        nRet = ((HTML_TokenEntry*)pFound)->nToken;
    return nRet;
}

/**/

// Flag: Farb-Tabelle wurde schon sortiert
struct HTML_ColorEntry
{
    union
    {
        const sal_Char* sName;
        const String *pUName;
    };
    ULONG nColor;
};

static int __FAR_DATA bSortColorKeyWords = FALSE;

#define HTML_NO_COLOR 0xffffffffUL

static HTML_ColorEntry __FAR_DATA aHTMLColorNameTab[] = {
sHTML_COL_aliceblue, 0x00f0f8ffUL,
sHTML_COL_antiquewhite, 0x00faebd7UL,
sHTML_COL_aqua, 0x0000ffffUL,
sHTML_COL_aquamarine, 0x007fffd4UL,
sHTML_COL_azure, 0x00f0ffffUL,
sHTML_COL_beige, 0x00f5f5dcUL,
sHTML_COL_bisque, 0x00ffe4c4UL,
sHTML_COL_black, 0x00000000UL,
sHTML_COL_blanchedalmond, 0x00ffebcdUL,
sHTML_COL_blue, 0x000000ffUL,
sHTML_COL_blueviolet, 0x008a2be2UL,
sHTML_COL_brown, 0x00a52a2aUL,
sHTML_COL_burlywood, 0x00deb887UL,
sHTML_COL_cadetblue, 0x005f9ea0UL,
sHTML_COL_chartreuse, 0x007fff00UL,
sHTML_COL_chocolate, 0x00d2691eUL,
sHTML_COL_coral, 0x00ff7f50UL,
sHTML_COL_cornflowerblue, 0x006495edUL,
sHTML_COL_cornsilk, 0x00fff8dcUL,
sHTML_COL_crimson, 0x00dc143cUL,
sHTML_COL_cyan, 0x0000ffffUL,
sHTML_COL_darkblue, 0x0000008bUL,
sHTML_COL_darkcyan, 0x00008b8bUL,
sHTML_COL_darkgoldenrod, 0x00b8860bUL,
sHTML_COL_darkgray, 0x00a9a9a9UL,
sHTML_COL_darkgreen, 0x00006400UL,
sHTML_COL_darkkhaki, 0x00bdb76bUL,
sHTML_COL_darkmagenta, 0x008b008bUL,
sHTML_COL_darkolivegreen, 0x00556b2fUL,
sHTML_COL_darkorange, 0x00ff8c00UL,
sHTML_COL_darkorchid, 0x009932ccUL,
sHTML_COL_darkred, 0x008b0000UL,
sHTML_COL_darksalmon, 0x00e9967aUL,
sHTML_COL_darkseagreen, 0x008fbc8fUL,
sHTML_COL_darkslateblue, 0x00483d8bUL,
sHTML_COL_darkslategray, 0x002f4f4fUL,
sHTML_COL_darkturquoise, 0x0000ced1UL,
sHTML_COL_darkviolet, 0x009400d3UL,
sHTML_COL_deeppink, 0x00ff1493UL,
sHTML_COL_deepskyblue, 0x0000bfffUL,
sHTML_COL_dimgray, 0x00696969UL,
sHTML_COL_dodgerblue, 0x001e90ffUL,
sHTML_COL_firebrick, 0x00b22222UL,
sHTML_COL_floralwhite, 0x00fffaf0UL,
sHTML_COL_forestgreen, 0x00228b22UL,
sHTML_COL_fuchsia, 0x00ff00ffUL,
sHTML_COL_gainsboro, 0x00dcdcdcUL,
sHTML_COL_ghostwhite, 0x00f8f8ffUL,
sHTML_COL_gold, 0x00ffd700UL,
sHTML_COL_goldenrod, 0x00daa520UL,
sHTML_COL_gray, 0x00808080UL,
sHTML_COL_green, 0x00008000UL,
sHTML_COL_greenyellow, 0x00adff2fUL,
sHTML_COL_honeydew, 0x00f0fff0UL,
sHTML_COL_hotpink, 0x00ff69b4UL,
sHTML_COL_indianred, 0x00cd5c5cUL,
sHTML_COL_indigo, 0x004b0082UL,
sHTML_COL_ivory, 0x00fffff0UL,
sHTML_COL_khaki, 0x00f0e68cUL,
sHTML_COL_lavender, 0x00e6e6faUL,
sHTML_COL_lavenderblush, 0x00fff0f5UL,
sHTML_COL_lawngreen, 0x007cfc00UL,
sHTML_COL_lemonchiffon, 0x00fffacdUL,
sHTML_COL_lightblue, 0x00add8e6UL,
sHTML_COL_lightcoral, 0x00f08080UL,
sHTML_COL_lightcyan, 0x00e0ffffUL,
sHTML_COL_lightgoldenrodyellow, 0x00fafad2UL,
sHTML_COL_lightgreen, 0x0090ee90UL,
sHTML_COL_lightgrey, 0x00d3d3d3UL,
sHTML_COL_lightpink, 0x00ffb6c1UL,
sHTML_COL_lightsalmon, 0x00ffa07aUL,
sHTML_COL_lightseagreen, 0x0020b2aaUL,
sHTML_COL_lightskyblue, 0x0087cefaUL,
sHTML_COL_lightslategray, 0x00778899UL,
sHTML_COL_lightsteelblue, 0x00b0c4deUL,
sHTML_COL_lightyellow, 0x00ffffe0UL,
sHTML_COL_lime, 0x0000ff00UL,
sHTML_COL_limegreen, 0x0032cd32UL,
sHTML_COL_linen, 0x00faf0e6UL,
sHTML_COL_magenta, 0x00ff00ffUL,
sHTML_COL_maroon, 0x00800000UL,
sHTML_COL_mediumaquamarine, 0x0066cdaaUL,
sHTML_COL_mediumblue, 0x000000cdUL,
sHTML_COL_mediumorchid, 0x00ba55d3UL,
sHTML_COL_mediumpurple, 0x009370dbUL,
sHTML_COL_mediumseagreen, 0x003cb371UL,
sHTML_COL_mediumslateblue, 0x007b68eeUL,
sHTML_COL_mediumspringgreen, 0x0000fa9aUL,
sHTML_COL_mediumturquoise, 0x0048d1ccUL,
sHTML_COL_mediumvioletred, 0x00c71585UL,
sHTML_COL_midnightblue, 0x00191970UL,
sHTML_COL_mintcream, 0x00f5fffaUL,
sHTML_COL_mistyrose, 0x00ffe4e1UL,
sHTML_COL_moccasin, 0x00ffe4b5UL,
sHTML_COL_navajowhite, 0x00ffdeadUL,
sHTML_COL_navy, 0x00000080UL,
sHTML_COL_oldlace, 0x00fdf5e6UL,
sHTML_COL_olive, 0x00808000UL,
sHTML_COL_olivedrab, 0x006b8e23UL,
sHTML_COL_orange, 0x00ffa500UL,
sHTML_COL_orangered, 0x00ff4500UL,
sHTML_COL_orchid, 0x00da70d6UL,
sHTML_COL_palegoldenrod, 0x00eee8aaUL,
sHTML_COL_palegreen, 0x0098fb98UL,
sHTML_COL_paleturquoise, 0x00afeeeeUL,
sHTML_COL_palevioletred, 0x00db7093UL,
sHTML_COL_papayawhip, 0x00ffefd5UL,
sHTML_COL_peachpuff, 0x00ffdab9UL,
sHTML_COL_peru, 0x00cd853fUL,
sHTML_COL_pink, 0x00ffc0cbUL,
sHTML_COL_plum, 0x00dda0ddUL,
sHTML_COL_powderblue, 0x00b0e0e6UL,
sHTML_COL_purple, 0x00800080UL,
sHTML_COL_red, 0x00ff0000UL,
sHTML_COL_rosybrown, 0x00bc8f8fUL,
sHTML_COL_royalblue, 0x004169e1UL,
sHTML_COL_saddlebrown, 0x008b4513UL,
sHTML_COL_salmon, 0x00fa8072UL,
sHTML_COL_sandybrown, 0x00f4a460UL,
sHTML_COL_seagreen, 0x002e8b57UL,
sHTML_COL_seashell, 0x00fff5eeUL,
sHTML_COL_sienna, 0x00a0522dUL,
sHTML_COL_silver, 0x00c0c0c0UL,
sHTML_COL_skyblue, 0x0087ceebUL,
sHTML_COL_slateblue, 0x006a5acdUL,
sHTML_COL_slategray, 0x00708090UL,
sHTML_COL_snow, 0x00fffafaUL,
sHTML_COL_springgreen, 0x0000ff7fUL,
sHTML_COL_steelblue, 0x004682b4UL,
sHTML_COL_tan, 0x00d2b48cUL,
sHTML_COL_teal, 0x00008080UL,
sHTML_COL_thistle, 0x00d8bfd8UL,
sHTML_COL_tomato, 0x00ff6347UL,
sHTML_COL_turquoise, 0x0040e0d0UL,
sHTML_COL_violet, 0x00ee82eeUL,
sHTML_COL_wheat, 0x00f5deb3UL,
sHTML_COL_white, 0x00ffffffUL,
sHTML_COL_whitesmoke, 0x00f5f5f5UL,
sHTML_COL_yellow, 0x00ffff00UL,
sHTML_COL_yellowgreen, 0x009acd32UL
};

extern "C"
{

static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
 _Optlink
#endif
    HTMLColorNameCompare( const void *pFirst, const void *pSecond)
{
    int nRet = 0;
    if( HTML_NO_COLOR == ((HTML_ColorEntry*)pFirst)->nColor )
    {
        if( HTML_NO_COLOR == ((HTML_ColorEntry*)pSecond)->nColor )
            nRet = ((HTML_ColorEntry*)pFirst)->pUName->CompareTo(
                            *((HTML_ColorEntry*)pSecond)->pUName );
        else
            nRet = ((HTML_ColorEntry*)pFirst)->pUName->CompareToAscii(
                            ((HTML_ColorEntry*)pSecond)->sName );
    }
    else
    {
        if( HTML_NO_COLOR  == ((HTML_ColorEntry*)pSecond)->nColor )
            nRet = -1 * ((HTML_ColorEntry*)pSecond)->pUName->CompareToAscii(
                            ((HTML_ColorEntry*)pFirst)->sName );
        else
            nRet = strcmp( ((HTML_ColorEntry*)pFirst)->sName,
                            ((HTML_ColorEntry*)pSecond)->sName );
    }

    return nRet;
}

}

ULONG GetHTMLColor( const String& rName )
{
    if( !bSortColorKeyWords )
    {
        qsort( (void*) aHTMLColorNameTab,
                sizeof( aHTMLColorNameTab ) / sizeof( HTML_ColorEntry ),
                sizeof( HTML_ColorEntry ),
                HTMLColorNameCompare );
        bSortColorKeyWords = TRUE;
    }

    ULONG nRet = ULONG_MAX;
    void* pFound;
    HTML_ColorEntry aSrch;
    aSrch.pUName = &rName;
    aSrch.nColor = HTML_NO_COLOR;

    if( 0 != ( pFound = bsearch( (sal_Char *) &aSrch,
                        (void*) aHTMLColorNameTab,
                        sizeof( aHTMLColorNameTab) / sizeof( HTML_ColorEntry ),
                        sizeof( HTML_ColorEntry ),
                        HTMLColorNameCompare )))
        nRet = ((HTML_ColorEntry*)pFound)->nColor;

    return nRet;
}

