/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: htmlkywd.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _HTMLKYWD_HXX
#define _HTMLKYWD_HXX

#include "svtools/svtdllapi.h"
#include "sal/types.h"

#ifndef SVTOOLS_CONSTASCII_DECL
#define SVTOOLS_CONSTASCII_DECL( n, s ) n[sizeof(s)]
#endif
#ifndef SVTOOLS_CONSTASCII_DEF
#define SVTOOLS_CONSTASCII_DEF( n, s ) n[sizeof(s)] = s
#endif


extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_doctype32,
    "HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_doctype40,
    "HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"" );


// diese werden nur eingeschaltet
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_area, "AREA" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_base, "BASE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_comment, "!--" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_doctype, "!DOCTYPE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_embed, "EMBED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_figureoverlay, "OVERLAY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_horzrule, "HR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_horztab, "TAB" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_image, "IMG" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_image2, "IMAGE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_input, "INPUT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_isindex, "ISINDEX" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_linebreak, "BR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_li, "LI" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_link, "LINK" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_meta, "META" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_nextid, "NEXTID" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_nobr, "NOBR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_object, "OBJECT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_of, "OF" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_option, "OPTION" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_param, "PARAM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_range, "RANGE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_spacer, "SPACER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_wbr, "WBR" );

// diese werden wieder abgeschaltet
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_abbreviation, "ABBREV" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_above, "ABOVE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_acronym, "ACRONYM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_address, "ADDRESS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_anchor, "A" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_applet, "APPLET" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_array, "ARRAY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_author, "AU" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_banner, "BANNER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_bar, "BAR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_basefont, "BASEFONT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_below, "BELOW" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_bigprint, "BIG" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_blink, "BLINK" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_blockquote, "BLOCKQUOTE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_blockquote30, "BQ" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_body, "BODY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_bold, "B" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_boldtext, "BT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_box, "BOX" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_caption, "CAPTION" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_center, "CENTER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_citiation, "CITE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_code, "CODE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_col, "COL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_colgroup, "COLGROUP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_credit, "CREDIT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_dd, "DD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_deflist, "DL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_deletedtext, "DEL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_dirlist, "DIR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_division, "DIV" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_dot, "DOT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_doubledot, "DDOT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_dt, "DT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_emphasis, "EM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_figure, "FIG" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_font, "FONT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_footnote, "FN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_form, "FORM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_frame, "FRAME" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_frameset, "FRAMESET" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_hat, "HAT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_head1, "H1" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_head2, "H2" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_head3, "H3" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_head4, "H4" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_head5, "H5" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_head6, "H6" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_head, "HEAD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_html, "HTML" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_iframe, "IFRAME" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_ilayer, "ILAYER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_insertedtext, "INS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_italic, "I" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_item, "ITEM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_keyboard, "KBD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_language, "LANG" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_layer, "LAYER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_listheader, "LH" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_map, "MAP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_math, "MATH" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_menulist, "MENU" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_multicol, "MULTICOL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_noembed, "NOEMBED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_noframe, "NOFRAME" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_noframes, "NOFRAMES" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_noscript, "NOSCRIPT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_note, "NOTE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_orderlist, "OL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_parabreak, "P" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_person, "PERSON" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_plaintext, "T" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_preformtxt, "PRE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_root, "ROOT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_row, "ROW" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_sample, "SAMP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_script, "SCRIPT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_select, "SELECT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_shortquote, "Q" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_smallprint, "SMALL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_span, "SPAN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_squareroot, "AQRT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_strikethrough, "S" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_strong, "STRONG" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_style, "STYLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_subscript, "SUB" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_superscript, "SUP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_table, "TABLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_tablerow, "TR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_tabledata, "TD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_tableheader, "TH" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_tbody, "TBODY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_teletype, "TT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_text, "TEXT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_textarea, "TEXTAREA" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_textflow, "TEXTFLOW" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_tfoot, "TFOOT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_thead, "THEAD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_tilde, "TILDE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_title, "TITLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_underline, "U" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_unorderlist, "UL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_variable, "VAR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_vector, "VEC" );

// obsolete features
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_xmp, "XMP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_listing, "LISTING" );

// proposed features
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_definstance, "DFN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_strike, "STRIKE" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_bgsound, "BGSOUND" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_comment2, "COMMENT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_marquee, "MARQUEE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_plaintext2, "PLAINTEXT" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_sdfield, "SDFIELD" );

// die Namen fuer alle Zeichen
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_lt, "lt" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_gt, "gt" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_amp, "amp" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_quot, "quot" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Aacute, "Aacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Agrave, "Agrave" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Acirc, "Acirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Atilde, "Atilde" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Aring, "Aring" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Auml, "Auml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_AElig, "AElig" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Ccedil, "Ccedil" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Eacute, "Eacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Egrave, "Egrave" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Ecirc, "Ecirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Euml, "Euml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Iacute, "Iacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Igrave, "Igrave" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Icirc, "Icirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Iuml, "Iuml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_ETH, "ETH" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Ntilde, "Ntilde" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Oacute, "Oacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Ograve, "Ograve" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Ocirc, "Ocirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Otilde, "Otilde" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Ouml, "Ouml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Oslash, "Oslash" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Uacute, "Uacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Ugrave, "Ugrave" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Ucirc, "Ucirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Uuml, "Uuml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_Yacute, "Yacute" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_THORN, "THORN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_szlig, "szlig" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_aacute, "aacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_agrave, "agrave" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_acirc, "acirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_atilde, "atilde" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_aring, "aring" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_auml, "auml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_aelig, "aelig" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ccedil, "ccedil" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_eacute, "eacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_egrave, "egrave" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ecirc, "ecirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_euml, "euml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_iacute, "iacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_igrave, "igrave" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_icirc, "icirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_iuml, "iuml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_eth, "eth" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ntilde, "ntilde" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_oacute, "oacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ograve, "ograve" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ocirc, "ocirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_otilde, "otilde" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ouml, "ouml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_oslash, "oslash" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_uacute, "uacute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ugrave, "ugrave" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ucirc, "ucirc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_uuml, "uuml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_yacute, "yacute" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_thorn, "thorn" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_yuml, "yuml" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_acute, "acute" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_brvbar, "brvbar" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_cedil, "cedil" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_cent, "cent" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_copy, "copy" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_curren, "curren" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_deg, "deg" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_divide, "divide" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_frac12, "frac12" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_frac14, "frac14" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_frac34, "frac34" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_iexcl, "iexcl" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_iquest, "iquest" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_laquo, "laquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_macr, "macr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_micro, "micro" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_middot, "middot" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_nbsp, "nbsp" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_not, "not" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ordf, "ordf" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ordm, "ordm" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_para, "para" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_plusmn, "plusmn" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_pound, "pound" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_raquo, "raquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_reg, "reg" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sect, "sect" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_shy, "shy" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sup1, "sup1" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sup2, "sup2" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sup3, "sup3" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_times, "times" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_uml, "uml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_yen, "yen" );

// Netscape kennt noch ein paar in Grossbuchstaben ...
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_LT, "LT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_GT, "GT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_AMP, "AMP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_C_QUOT, "QUOT" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_COPY, "COPY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_REG, "REG" );

//HTML4
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_alefsym, "alefsym" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Alpha, "Alpha" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_alpha, "alpha" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_and, "and" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ang, "ang" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_asymp, "asymp" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_bdquo, "bdquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Beta, "Beta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_beta, "beta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_bull, "bull" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_cap, "cap" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_chi, "chi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Chi, "Chi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_circ, "circ" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_clubs, "clubs" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_cong, "cong" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_crarr, "crarr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_cup, "cup" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_dagger, "dagger" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Dagger, "Dagger" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_darr, "darr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_dArr, "dArr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Delta, "Delta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_delta, "delta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_diams, "diams" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_empty, "empty" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_emsp, "emsp" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ensp, "ensp" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Epsilon, "Epsilon" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_epsilon, "epsilon" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_equiv, "equiv" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Eta, "Eta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_eta, "eta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_euro, "euro" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_exist, "exist" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_fnof, "fnof" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_forall, "forall" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_frasl, "frasl" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Gamma, "Gamma" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_gamma, "gamma" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ge, "ge" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_harr, "harr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_hArr, "hArr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_hearts, "hearts" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_hellip, "hellip" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_image, "image" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_infin, "infin" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_int, "int" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Iota, "Iota" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_iota, "iota" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_isin, "isin" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Kappa, "Kappa" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_kappa, "kappa" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Lambda, "Lambda" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lambda, "lambda" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lang, "lang" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_larr, "larr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lArr, "lArr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lceil, "lceil" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ldquo, "ldquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_le, "le" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lfloor, "lfloor" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lowast, "lowast" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_loz, "loz" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lrm, "lrm" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lsaquo, "lsaquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_lsquo, "lsquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_mdash, "mdash" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_minus, "minus" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Mu, "Mu" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_mu, "mu" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_nabla, "nabla" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ndash, "ndash" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ne, "ne" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_ni, "ni" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_notin, "notin" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_nsub, "nsub" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Nu, "Nu" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_nu, "nu" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_OElig, "OElig" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_oelig, "oelig" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_oline, "oline" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Omega, "Omega" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_omega, "omega" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Omicron, "Omicron" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_omicron, "omicron" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_oplus, "oplus" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_or, "or" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_otimes, "otimes" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_part, "part" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_permil, "permil" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_perp, "perp" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Phi, "Phi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_phi, "phi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Pi, "Pi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_pi, "pi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_piv, "piv" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_prime, "prime" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Prime, "Prime" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_prod, "prod" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_prop, "prop" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Psi, "Psi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_psi, "psi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_radic, "radic" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rang, "rang" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rarr, "rarr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rArr, "rArr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rceil, "rceil" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rdquo, "rdquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_real, "real" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rfloor, "rfloor" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Rho, "Rho" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rho, "rho" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rlm, "rlm" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rsaquo, "rsaquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_rsquo, "rsquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sbquo, "sbquo" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Scaron, "Scaron" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_scaron, "scaron" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sdot, "sdot" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Sigma, "Sigma" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sigma, "sigma" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sigmaf, "sigmaf" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sim, "sim" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_spades, "spades" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sub, "sub" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sube, "sube" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sum, "sum" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_sup, "sup" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_supe, "supe" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Tau, "Tau" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_tau, "tau" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_there4, "there4" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Theta, "Theta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_theta, "theta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_thetasym, "thetasym" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_thinsp, "thinsp" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_tilde, "tilde" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_trade, "trade" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_uarr, "uarr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_uArr, "uArr" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_upsih, "upsih" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Upsilon, "Upsilon" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_upsilon, "upsilon" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_weierp, "weierp" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Xi, "Xi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_xi, "xi" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Yuml, "Yuml" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_Zeta, "Zeta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_zeta, "zeta" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_zwj, "zwj" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_S_zwnj, "zwnj" );

// HTML Attribut-Token (=Optionen)

// Attribute ohne Wert
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_box, "BOX" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_checked, "CHECKED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_compact, "COMPACT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_continue, "CONTINUE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_controls, "CONTROLS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_declare, "DECLARE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_disabled, "DISABLED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_folded, "FOLDED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_ismap, "ISMAP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_mayscript, "MAYSCRIPT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_multiple, "MULTIPLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_noflow, "NOFLOW" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_nohref, "NOHREF" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_noresize, "NORESIZE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_noshade, "NOSHADE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_nowrap, "NOWRAP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_plain, "PLAIN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_sdfixed, "SDFIXED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_selected, "SELECTED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_shapes, "SHAPES" );

// Attribute mit einem String als Wert
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_above, "ABOVE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_accesskey, "ACCESSKEY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_accept, "ACCEPT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_add_date, "ADD_DATE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_alt, "ALT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_axes, "AXES" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_axis, "AXIS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_below, "BELOW" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_char, "CHAR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_class, "CLASS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_clip, "CLIP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_code, "CODE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_codetype, "CODETYPE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_colspec, "COLSPEC" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_content, "CONTENT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_coords, "COORDS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_dp, "DP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_enctype, "ENCTYPE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_error, "ERROR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_face, "FACE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_frameborder, "FRAMEBORDER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_httpequiv, "HTTP-EQUIV" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_language, "LANGUAGE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_last_modified, "LAST_MODIFIED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_last_visit, "LAST_VISIT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_md, "MD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_n, "N" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_name, "NAME" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_notation, "NOTATION" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_prompt, "PROMPT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_shape, "SHAPE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_standby, "STANDBY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_style, "STYLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_title, "TITLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_value, "VALUE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDval, "SDVAL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDnum, "SDNUM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_sdlibrary, "SDLIBRARY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_sdmodule, "SDMODULE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_sdevent, "SDEVENT-" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_sdaddparam, "SDADDPARAM-" );

// Attribute mit einem SGML-Identifier als Wert
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_from, "FROM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_id, "ID" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_target, "TARGET" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_to, "TO" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_until, "UNTIL" );

// Attribute mit einem URI als Wert
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_action, "ACTION" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_archive, "ARCHIVE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_background, "BACKGROUND" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_classid, "CLASSID" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_codebase, "CODEBASE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_data, "DATA" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_dynsrc, "DYNSRC" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_dynsync, "DYNSYNC" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_imagemap, "IMAGEMAP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_href, "HREF" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_lowsrc, "LOWSRC" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_script, "SCRIPT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_src, "SRC" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_usemap, "USEMAP" );

// Attribute mit Entity-Namen als Wert
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_dingbat, "DINGBAT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_sym, "SYM" );

// Attribute mit einer Farbe als Wert (alle Netscape)
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_alink, "ALINK" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_bgcolor, "BGCOLOR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_bordercolor, "BORDERCOLOR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_bordercolorlight, "BORDERCOLORLIGHT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_bordercolordark, "BORDERCOLORDARK" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_color, "COLOR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_link, "LINK" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_text, "TEXT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_vlink, "VLINK" );

// Attribute mit einem numerischen Wert
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_border, "BORDER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_cellspacing, "CELLSPACING" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_cellpadding, "CELLPADDING" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_charoff, "CHAROFF" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_colspan, "COLSPAN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_framespacing, "FRAMESPACING" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_gutter, "GUTTER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_indent, "INDENT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_height, "HEIGHT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_hspace, "HSPACE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_left, "LEFT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_leftmargin, "LEFTMARGIN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_loop, "LOOP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_marginheight, "MARGINHEIGHT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_marginwidth, "MARGINWIDTH" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_max, "MAX" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_maxlength, "MAXLENGTH" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_min, "MIN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_pagex, "PAGEX" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_pagey, "PAGEY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_pointsize, "POINT-SIZE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_rowspan, "ROWSPAN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_scrollamount, "SCROLLAMOUNT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_scrolldelay, "SCROLLDELAY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_seqnum, "SEQNUM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_skip, "SKIP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_span, "SPAN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_tabindex, "TABINDEX" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_top, "TOP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_topmargin, "TOPMARGIN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_vspace, "VSPACE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_weight, "WEIGHT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_width, "WIDTH" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_x, "X" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_y, "Y" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_zindex, "Z-INDEX" );

// Attribute mit Enum-Werten
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_behavior, "BEHAVIOR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_bgproperties, "BGPROPERTIES" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_clear, "CLEAR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_dir, "DIR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_direction, "DIRECTION" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_format, "FORMAT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_frame, "FRAME" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_lang, "LANG" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_method, "METHOD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_palette, "PALETTE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_rel, "REL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_rev, "REV" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_rules, "RULES" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_scrolling, "SCROLLING" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_sdreadonly, "READONLY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_subtype, "SUBTYPE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_type, "TYPE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_valign, "VALIGN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_valuetype, "VALUETYPE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_visibility, "VISIBILITY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_wrap, "WRAP" );

// Attribute mit Script-Code als Wert
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onblur, "ONBLUR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onchange, "ONCHANGE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onclick, "ONCLICK" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onfocus, "ONFOCUS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onload, "ONLOAD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onmouseover, "ONMOUSEOVER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onreset, "ONRESET" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onselect, "ONSELECT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onsubmit, "ONSUBMIT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onunload, "ONUNLOAD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onabort, "ONABORT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onerror, "ONERROR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_onmouseout, "ONMOUSEOUT" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonblur, "SDONBLUR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonchange, "SDONCHANGE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonclick, "SDONCLICK" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonfocus, "SDONFOCUS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonload, "SDONLOAD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonmouseover, "SDONMOUSEOVER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonreset, "SDONRESET" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonselect, "SDONSELECT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonsubmit, "SDONSUBMIT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonunload, "SDONUNLOAD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonabort, "SDONABORT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonerror, "SDONERROR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonmouseout, "SDONMOUSEOUT" );

// Attribute mit Kontext-abhaengigen Werten
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_align, "ALIGN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_cols, "COLS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_rows, "ROWS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_start, "START" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_size, "SIZE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_units, "UNITS" );


// Werte von <INPUT TYPE=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_text, "TEXT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_password, "PASSWORD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_checkbox, "CHECKBOX" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_radio, "RADIO" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_range, "RANGE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_scribble, "SCRIBBLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_file, "FILE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_hidden, "HIDDEN" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_submit, "SUBMIT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_image, "IMAGE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_reset, "RESET" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_IT_button, "BUTTON" );

// Werte von <TABLE FRAME=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_void, "VOID" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_above, "ABOVE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_below, "BELOW" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_hsides, "HSIDES" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_lhs, "LHS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_rhs, "RHS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_vsides, "VSIDES" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_box, "BOX" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TF_border, "BORDER" );

// Werte von <TABLE RULES=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TR_none, "NONE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TR_groups, "GROUPS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TR_rows, "ROWS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TR_cols, "COLS" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_TR_all, "ALL" );

// Werte von <P, H?, TR, TH, TD ALIGN=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_AL_left, "LEFT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_AL_center, "CENTER" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_AL_middle, "MIDDLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_AL_right, "RIGHT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_AL_justify, "JUSTIFY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_AL_char, "CHAR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_AL_all, "ALL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_AL_none, "NONE" );

// Werte von <TR VALIGN=...>, <IMG ALIGN=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_VA_top, "TOP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_VA_middle, "MIDDLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_VA_bottom, "BOTTOM" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_VA_baseline, "BASELINE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_VA_texttop, "TEXTTOP" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_VA_absmiddle, "ABSMIDDLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_VA_absbottom, "ABSBOTTOM" );

// Werte von <AREA SHAPE=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SH_rect, "RECT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SH_rectangle, "RECTANGLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SH_circ, "CIRC" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SH_circle, "CIRCLE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SH_poly, "POLY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SH_polygon, "POLYGON" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SH_default, "DEFAULT" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_LG_starbasic, "STARBASIC" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_LG_javascript, "JAVASCRIPT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_LG_javascript11, "JAVASCRIPT1.1" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_LG_livescript, "LIVESCRIPT" );
//extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_LG_vbscript, "VBSCRIPT" );
//extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_LG_unused_javascript, "UNUSED JAVASCRIPT" );
//extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_LG_starone, "StarScript" );

// ein par Werte fuer unser StarBASIC-Support
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SB_library, "$LIBRARY:" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SB_module, "$MODULE:" );


// Werte von <FORM METHOD=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_METHOD_get, "GET" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_METHOD_post, "POST" );

// Werte von <META CONTENT/HTTP-EQUIV=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_refresh, "REFRESH" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_generator, "GENERATOR" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_author, "AUTHOR" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_classification, "CLASSIFICATION" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_description, "DESCRIPTION" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_keywords, "KEYWORDS" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_changed, "CHANGED" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_changedby, "CHANGEDBY" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_created, "CREATED" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_content_type, "CONTENT-TYPE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_content_script_type, "CONTENT-SCRIPT-TYPE" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_sdendnote, "SDENDNOTE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_META_sdfootnote, "SDFOOTNOTE" );

// Werte von <UL TYPE=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_ULTYPE_disc, "DISC" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_ULTYPE_square, "SQUARE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_ULTYPE_circle, "CIRCLE" );

// Werte von <FRAMESET SCROLLING=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SCROLL_yes, "YES" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SCROLL_no, "NO" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SCROLL_auto, "AUTO" );

// Werte von <MULTICOL TYPE=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_MCTYPE_horizontal, "HORIZONTAL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_MCTYPE_vertical, "VERTICAL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_MCTYPE_box, "BOX" );

// Werte von <MARQUEE BEHAVIOUR=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_BEHAV_scroll, "SCROLL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_BEHAV_slide, "SLIDE" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_BEHAV_alternate, "ALTERNATE" );

// Werte von <MARQUEE LOOP=...>
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_LOOP_infinite, "INFINITE" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SPTYPE_block, "BLOCK" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SPTYPE_horizontal, "HORIZONTAL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_SPTYPE_vertical, "VERTICAL" );


// interne Grafik-Namen
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_private_image, "private:image/" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_internal_gopher, "internal-gopher-" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_internal_icon, "internal-icon-" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_binary, "binary" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_image, "image" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_index, "index" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_menu, "menu" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_movie, "movie" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_sound, "sound" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_telnet, "telnet" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_text, "text" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_unknown, "unknown" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_baddata, "baddata" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_delayed, "delayed" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_embed, "embed" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_insecure, "insecure" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_notfound, "notfound" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_sdendnote, "sdendnote" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_sdendnote_anc, "sdendnoteanc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_sdendnote_sym, "sdendnotesym" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_sdfootnote, "sdfootnote" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_sdfootnote_anc, "sdfootnoteanc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_sdfootnote_sym, "sdfootnotesym" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_FTN_anchor, "anc" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_FTN_symbol, "sym" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_WW_off, "OFF" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_WW_hard, "HARD" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_WW_soft, "SOFT" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_WW_virtual, "VIRTUAL" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_WW_physical, "PHYSICAL" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_on, "on" );

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_ET_url, "application/x-www-form-urlencoded" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_ET_multipart, "multipart/form-data" );
extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_ET_text, "text/plain" );


#endif
