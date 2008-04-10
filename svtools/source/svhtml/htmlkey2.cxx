/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: htmlkey2.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "htmlkywd.hxx"

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_doctype32,
    "HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_doctype40,
    "HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"" );


// diese werden nur eingeschaltet
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_area, "AREA" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_base, "BASE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_comment, "!--" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_doctype, "!DOCTYPE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_embed, "EMBED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_figureoverlay, "OVERLAY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_horzrule, "HR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_horztab, "TAB" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_image, "IMG" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_image2, "IMAGE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_input, "INPUT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_isindex, "ISINDEX" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_linebreak, "BR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_li, "LI" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_link, "LINK" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_meta, "META" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_nextid, "NEXTID" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_nobr, "NOBR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_of, "OF" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_option, "OPTION" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_param, "PARAM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_range, "RANGE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_spacer, "SPACER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_wbr, "WBR" );

// diese werden wieder abgeschaltet
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_abbreviation, "ABBREV" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_above, "ABOVE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_acronym, "ACRONYM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_address, "ADDRESS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_anchor, "A" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_applet, "APPLET" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_array, "ARRAY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_author, "AU" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_banner, "BANNER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_bar, "BAR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_basefont, "BASEFONT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_below, "BELOW" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_bigprint, "BIG" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_blink, "BLINK" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_blockquote, "BLOCKQUOTE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_blockquote30, "BQ" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_body, "BODY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_bold, "B" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_boldtext, "BT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_box, "BOX" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_caption, "CAPTION" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_center, "CENTER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_citiation, "CITE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_code, "CODE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_col, "COL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_colgroup, "COLGROUP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_credit, "CREDIT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_dd, "DD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_deflist, "DL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_deletedtext, "DEL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_dirlist, "DIR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_division, "DIV" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_dot, "DOT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_doubledot, "DDOT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_dt, "DT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_emphasis, "EM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_figure, "FIG" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_font, "FONT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_footnote, "FN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_form, "FORM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_frame, "FRAME" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_frameset, "FRAMESET" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_hat, "HAT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_head1, "H1" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_head2, "H2" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_head3, "H3" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_head4, "H4" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_head5, "H5" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_head6, "H6" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_head, "HEAD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_html, "HTML" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_iframe, "IFRAME" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_ilayer, "ILAYER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_insertedtext, "INS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_italic, "I" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_item, "ITEM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_keyboard, "KBD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_language, "LANG" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_layer, "LAYER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_listheader, "LH" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_map, "MAP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_math, "MATH" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_menulist, "MENU" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_multicol, "MULTICOL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_noembed, "NOEMBED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_noframe, "NOFRAME" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_noframes, "NOFRAMES" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_noscript, "NOSCRIPT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_note, "NOTE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_object, "OBJECT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_orderlist, "OL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_parabreak, "P" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_person, "PERSON" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_plaintext, "T" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_preformtxt, "PRE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_root, "ROOT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_row, "ROW" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_sample, "SAMP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_script, "SCRIPT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_select, "SELECT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_shortquote, "Q" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_smallprint, "SMALL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_span, "SPAN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_squareroot, "AQRT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_strikethrough, "S" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_strong, "STRONG" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_style, "STYLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_subscript, "SUB" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_superscript, "SUP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_table, "TABLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_tablerow, "TR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_tabledata, "TD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_tableheader, "TH" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_tbody, "TBODY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_teletype, "TT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_text, "TEXT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_textarea, "TEXTAREA" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_textflow, "TEXTFLOW" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_tfoot, "TFOOT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_thead, "THEAD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_tilde, "TILDE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_title, "TITLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_underline, "U" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_unorderlist, "UL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_variable, "VAR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_vector, "VEC" );

// obsolete features
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_xmp, "XMP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_listing, "LISTING" );

// proposed features
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_definstance, "DFN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_strike, "STRIKE" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_bgsound, "BGSOUND" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_comment2, "COMMENT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_marquee, "MARQUEE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_plaintext2, "PLAINTEXT" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_sdfield, "SDFIELD" );

// die Namen fuer alle Zeichen
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_lt, "lt" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_gt, "gt" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_amp, "amp" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_quot, "quot" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Aacute, "Aacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Agrave, "Agrave" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Acirc, "Acirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Atilde, "Atilde" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Aring, "Aring" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Auml, "Auml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_AElig, "AElig" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Ccedil, "Ccedil" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Eacute, "Eacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Egrave, "Egrave" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Ecirc, "Ecirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Euml, "Euml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Iacute, "Iacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Igrave, "Igrave" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Icirc, "Icirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Iuml, "Iuml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_ETH, "ETH" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Ntilde, "Ntilde" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Oacute, "Oacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Ograve, "Ograve" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Ocirc, "Ocirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Otilde, "Otilde" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Ouml, "Ouml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Oslash, "Oslash" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Uacute, "Uacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Ugrave, "Ugrave" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Ucirc, "Ucirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Uuml, "Uuml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_Yacute, "Yacute" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_THORN, "THORN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_szlig, "szlig" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_aacute, "aacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_agrave, "agrave" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_acirc, "acirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_atilde, "atilde" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_aring, "aring" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_auml, "auml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_aelig, "aelig" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ccedil, "ccedil" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_eacute, "eacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_egrave, "egrave" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ecirc, "ecirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_euml, "euml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_iacute, "iacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_igrave, "igrave" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_icirc, "icirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_iuml, "iuml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_eth, "eth" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ntilde, "ntilde" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_oacute, "oacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ograve, "ograve" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ocirc, "ocirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_otilde, "otilde" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ouml, "ouml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_oslash, "oslash" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_uacute, "uacute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ugrave, "ugrave" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ucirc, "ucirc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_uuml, "uuml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_yacute, "yacute" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_thorn, "thorn" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_yuml, "yuml" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_acute, "acute" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_brvbar, "brvbar" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_cedil, "cedil" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_cent, "cent" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_copy, "copy" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_curren, "curren" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_deg, "deg" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_divide, "divide" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_frac12, "frac12" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_frac14, "frac14" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_frac34, "frac34" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_iexcl, "iexcl" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_iquest, "iquest" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_laquo, "laquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_macr, "macr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_micro, "micro" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_middot, "middot" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_nbsp, "nbsp" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_not, "not" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ordf, "ordf" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ordm, "ordm" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_para, "para" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_plusmn, "plusmn" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_pound, "pound" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_raquo, "raquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_reg, "reg" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sect, "sect" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_shy, "shy" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sup1, "sup1" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sup2, "sup2" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sup3, "sup3" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_times, "times" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_uml, "uml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_yen, "yen" );

// Netscape kennt noch ein paar in Grossbuchstaben ...
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_LT, "LT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_GT, "GT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_AMP, "AMP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_C_QUOT, "QUOT" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_COPY, "COPY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_REG, "REG" );

// HTML4
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_alefsym, "alefsym" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Alpha, "Alpha" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_alpha, "alpha" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_and, "and" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ang, "ang" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_asymp, "asymp" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_bdquo, "bdquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Beta, "Beta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_beta, "beta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_bull, "bull" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_cap, "cap" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_chi, "chi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Chi, "Chi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_circ, "circ" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_clubs, "clubs" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_cong, "cong" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_crarr, "crarr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_cup, "cup" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_dagger, "dagger" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Dagger, "Dagger" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_darr, "darr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_dArr, "dArr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Delta, "Delta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_delta, "delta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_diams, "diams" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_empty, "empty" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_emsp, "emsp" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ensp, "ensp" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Epsilon, "Epsilon" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_epsilon, "epsilon" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_equiv, "equiv" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Eta, "Eta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_eta, "eta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_euro, "euro" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_exist, "exist" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_fnof, "fnof" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_forall, "forall" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_frasl, "frasl" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Gamma, "Gamma" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_gamma, "gamma" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ge, "ge" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_harr, "harr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_hArr, "hArr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_hearts, "hearts" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_hellip, "hellip" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_image, "image" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_infin, "infin" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_int, "int" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Iota, "Iota" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_iota, "iota" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_isin, "isin" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Kappa, "Kappa" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_kappa, "kappa" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Lambda, "Lambda" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lambda, "lambda" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lang, "lang" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_larr, "larr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lArr, "lArr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lceil, "lceil" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ldquo, "ldquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_le, "le" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lfloor, "lfloor" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lowast, "lowast" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_loz, "loz" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lrm, "lrm" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lsaquo, "lsaquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_lsquo, "lsquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_mdash, "mdash" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_minus, "minus" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Mu, "Mu" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_mu, "mu" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_nabla, "nabla" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ndash, "ndash" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ne, "ne" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_ni, "ni" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_notin, "notin" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_nsub, "nsub" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Nu, "Nu" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_nu, "nu" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_OElig, "OElig" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_oelig, "oelig" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_oline, "oline" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Omega, "Omega" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_omega, "omega" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Omicron, "Omicron" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_omicron, "omicron" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_oplus, "oplus" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_or, "or" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_otimes, "otimes" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_part, "part" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_permil, "permil" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_perp, "perp" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Phi, "Phi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_phi, "phi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Pi, "Pi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_pi, "pi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_piv, "piv" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_prime, "prime" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Prime, "Prime" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_prod, "prod" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_prop, "prop" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Psi, "Psi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_psi, "psi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_radic, "radic" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rang, "rang" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rarr, "rarr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rArr, "rArr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rceil, "rceil" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rdquo, "rdquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_real, "real" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rfloor, "rfloor" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Rho, "Rho" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rho, "rho" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rlm, "rlm" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rsaquo, "rsaquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_rsquo, "rsquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sbquo, "sbquo" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Scaron, "Scaron" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_scaron, "scaron" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sdot, "sdot" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Sigma, "Sigma" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sigma, "sigma" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sigmaf, "sigmaf" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sim, "sim" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_spades, "spades" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sub, "sub" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sube, "sube" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sum, "sum" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_sup, "sup" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_supe, "supe" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Tau, "Tau" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_tau, "tau" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_there4, "there4" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Theta, "Theta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_theta, "theta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_thetasym, "thetasym" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_thinsp, "thinsp" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_tilde, "tilde" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_trade, "trade" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_uarr, "uarr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_uArr, "uArr" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_upsih, "upsih" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Upsilon, "Upsilon" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_upsilon, "upsilon" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_weierp, "weierp" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Xi, "Xi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_xi, "xi" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Yuml, "Yuml" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_Zeta, "Zeta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_zeta, "zeta" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_zwj, "zwj" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_S_zwnj, "zwnj" );

// HTML Attribut-Token (=Optionen)

// Attribute ohne Wert
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_box, "BOX" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_checked, "CHECKED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_compact, "COMPACT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_continue, "CONTINUE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_controls, "CONTROLS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_declare, "DECLARE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_disabled, "DISABLED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_folded, "FOLDED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_ismap, "ISMAP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_mayscript, "MAYSCRIPT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_multiple, "MULTIPLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_noflow, "NOFLOW" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_nohref, "NOHREF" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_noresize, "NORESIZE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_noshade, "NOSHADE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_nowrap, "NOWRAP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_plain, "PLAIN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_sdfixed, "SDFIXED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_selected, "SELECTED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_shapes, "SHAPES" );

// Attribute mit einem String als Wert
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_above, "ABOVE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_accesskey, "ACCESSKEY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_accept, "ACCEPT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_add_date, "ADD_DATE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_alt, "ALT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_axes, "AXES" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_axis, "AXIS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_below, "BELOW" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_char, "CHAR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_class, "CLASS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_clip, "CLIP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_code, "CODE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_codetype, "CODETYPE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_colspec, "COLSPEC" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_content, "CONTENT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_coords, "COORDS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_dp, "DP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_enctype, "ENCTYPE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_error, "ERROR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_face, "FACE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_frameborder, "FRAMEBORDER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_httpequiv, "HTTP-EQUIV" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_language, "LANGUAGE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_last_modified, "LAST_MODIFIED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_last_visit, "LAST_VISIT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_md, "MD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_n, "N" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_name, "NAME" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_notation, "NOTATION" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_prompt, "PROMPT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_shape, "SHAPE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_standby, "STANDBY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_style, "STYLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_title, "TITLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_value, "VALUE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDval, "SDVAL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDnum, "SDNUM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_sdlibrary, "SDLIBRARY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_sdmodule, "SDMODULE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_sdevent, "SDEVENT-" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_sdaddparam, "SDADDPARAM-" );

// Attribute mit einem SGML-Identifier als Wert
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_from, "FROM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_id, "ID" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_target, "TARGET" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_to, "TO" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_until, "UNTIL" );

// Attribute mit einem URI als Wert
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_action, "ACTION" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_archive, "ARCHIVE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_background, "BACKGROUND" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_classid, "CLASSID" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_codebase, "CODEBASE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_data, "DATA" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_dynsrc, "DYNSRC" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_dynsync, "DYNSYNC" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_imagemap, "IMAGEMAP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_href, "HREF" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_lowsrc, "LOWSRC" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_script, "SCRIPT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_src, "SRC" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_usemap, "USEMAP" );

// Attribute mit Entity-Namen als Wert
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_dingbat, "DINGBAT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_sym, "SYM" );

// Attribute mit einer Farbe als Wert (alle Netscape)
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_alink, "ALINK" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_bgcolor, "BGCOLOR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_bordercolor, "BORDERCOLOR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_bordercolorlight, "BORDERCOLORLIGHT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_bordercolordark, "BORDERCOLORDARK" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_color, "COLOR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_link, "LINK" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_text, "TEXT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_vlink, "VLINK" );

// Attribute mit einem numerischen Wert
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_border, "BORDER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_cellspacing, "CELLSPACING" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_cellpadding, "CELLPADDING" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_charoff, "CHAROFF" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_colspan, "COLSPAN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_framespacing, "FRAMESPACING" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_gutter, "GUTTER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_indent, "INDENT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_height, "HEIGHT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_hspace, "HSPACE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_left, "LEFT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_leftmargin, "LEFTMARGIN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_loop, "LOOP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_marginheight, "MARGINHEIGHT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_marginwidth, "MARGINWIDTH" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_max, "MAX" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_maxlength, "MAXLENGTH" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_min, "MIN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_pagex, "PAGEX" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_pagey, "PAGEY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_pointsize, "POINT-SIZE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_rowspan, "ROWSPAN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_scrollamount, "SCROLLAMOUNT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_scrolldelay, "SCROLLDELAY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_seqnum, "SEQNUM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_skip, "SKIP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_span, "SPAN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_tabindex, "TABINDEX" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_top, "TOP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_topmargin, "TOPMARGIN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_vspace, "VSPACE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_weight, "WEIGHT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_width, "WIDTH" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_x, "X" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_y, "Y" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_zindex, "Z-INDEX" );

// Attribute mit Enum-Werten
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_behavior, "BEHAVIOR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_bgproperties, "BGPROPERTIES" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_clear, "CLEAR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_dir, "DIR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_direction, "DIRECTION" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_format, "FORMAT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_frame, "FRAME" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_lang, "LANG" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_method, "METHOD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_palette, "PALETTE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_rel, "REL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_rev, "REV" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_rules, "RULES" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_scrolling, "SCROLLING" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_sdreadonly, "READONLY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_subtype, "SUBTYPE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_type, "TYPE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_valign, "VALIGN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_valuetype, "VALUETYPE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_visibility, "VISIBILITY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_wrap, "WRAP" );

// Attribute mit Script-Code als Wert
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onblur, "ONBLUR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onchange, "ONCHANGE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onclick, "ONCLICK" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onfocus, "ONFOCUS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onload, "ONLOAD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onmouseover, "ONMOUSEOVER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onreset, "ONRESET" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onselect, "ONSELECT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onsubmit, "ONSUBMIT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onunload, "ONUNLOAD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onabort, "ONABORT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onerror, "ONERROR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_onmouseout, "ONMOUSEOUT" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonblur, "SDONBLUR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonchange, "SDONCHANGE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonclick, "SDONCLICK" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonfocus, "SDONFOCUS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonload, "SDONLOAD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonmouseover, "SDONMOUSEOVER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonreset, "SDONRESET" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonselect, "SDONSELECT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonsubmit, "SDONSUBMIT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonunload, "SDONUNLOAD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonabort, "SDONABORT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonerror, "SDONERROR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonmouseout, "SDONMOUSEOUT" );

// Attribute mit Kontext-abhaengigen Werten
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_align, "ALIGN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_cols, "COLS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_rows, "ROWS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_start, "START" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_size, "SIZE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_units, "UNITS" );


// Werte von <INPUT TYPE=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_text, "TEXT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_password, "PASSWORD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_checkbox, "CHECKBOX" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_radio, "RADIO" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_range, "RANGE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_scribble, "SCRIBBLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_file, "FILE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_hidden, "HIDDEN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_submit, "SUBMIT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_image, "IMAGE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_reset, "RESET" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_IT_button, "BUTTON" );

// Werte von <TABLE FRAME=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_void, "VOID" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_above, "ABOVE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_below, "BELOW" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_hsides, "HSIDES" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_lhs, "LHS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_rhs, "RHS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_vsides, "VSIDES" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_box, "BOX" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TF_border, "BORDER" );

// Werte von <TABLE RULES=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TR_none, "NONE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TR_groups, "GROUPS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TR_rows, "ROWS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TR_cols, "COLS" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_TR_all, "ALL" );

// Werte von <P, H?, TR, TH, TD ALIGN=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_AL_left, "LEFT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_AL_center, "CENTER" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_AL_middle, "MIDDLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_AL_right, "RIGHT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_AL_justify, "JUSTIFY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_AL_char, "CHAR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_AL_all, "ALL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_AL_none, "NONE" );

// Werte von <TR VALIGN=...>, <IMG ALIGN=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_VA_top, "TOP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_VA_middle, "MIDDLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_VA_bottom, "BOTTOM" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_VA_baseline, "BASELINE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_VA_texttop, "TEXTTOP" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_VA_absmiddle, "ABSMIDDLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_VA_absbottom, "ABSBOTTOM" );

// Werte von <AREA SHAPE=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SH_rect, "RECT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SH_rectangle, "RECTANGLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SH_circ, "CIRC" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SH_circle, "CIRCLE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SH_poly, "POLY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SH_polygon, "POLYGON" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SH_default, "DEFAULT" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_LG_starbasic, "STARBASIC" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_LG_javascript, "JAVASCRIPT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_LG_javascript11, "JAVASCRIPT1.1" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_LG_livescript, "LIVESCRIPT" );
//sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_LG_vbscript, "VBSCRIPT" );
//sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_LG_unused_javascript, "UNUSED JAVASCRIPT" );
//sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_LG_starone, "StarScript" );

// ein par Werte fuer unser StarBASIC-Support
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SB_library, "$LIBRARY:" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SB_module, "$MODULE:" );


// Werte von <FORM METHOD=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_METHOD_get, "GET" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_METHOD_post, "POST" );

// Werte von <META CONTENT/HTTP-EQUIV=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_refresh, "REFRESH" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_generator, "GENERATOR" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_author, "AUTHOR" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_classification, "CLASSIFICATION" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_description, "DESCRIPTION" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_keywords, "KEYWORDS" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_changed, "CHANGED" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_changedby, "CHANGEDBY" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_created, "CREATED" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_content_type, "CONTENT-TYPE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_content_script_type, "CONTENT-SCRIPT-TYPE" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_sdendnote, "SDENDNOTE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_META_sdfootnote, "SDFOOTNOTE" );

// Werte von <UL TYPE=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_ULTYPE_disc, "DISC" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_ULTYPE_square, "SQUARE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_ULTYPE_circle, "CIRCLE" );

// Werte von <FRAMESET SCROLLING=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SCROLL_yes, "YES" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SCROLL_no, "NO" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SCROLL_auto, "AUTO" );

// Werte von <MULTICOL TYPE=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_MCTYPE_horizontal, "HORIZONTAL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_MCTYPE_vertical, "VERTICAL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_MCTYPE_box, "BOX" );

// Werte von <MARQUEE BEHAVIOUR=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_BEHAV_scroll, "SCROLL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_BEHAV_slide, "SLIDE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_BEHAV_alternate, "ALTERNATE" );

// Werte von <MARQUEE LOOP=...>
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_LOOP_infinite, "INFINITE" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SPTYPE_block, "BLOCK" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SPTYPE_horizontal, "HORIZONTAL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_SPTYPE_vertical, "VERTICAL" );


// interne Grafik-Namen
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_private_image, "private:image/" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_internal_gopher, "internal-gopher-" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_internal_icon, "internal-icon-" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_binary, "binary" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_image, "image" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_index, "index" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_menu, "menu" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_movie, "movie" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_sound, "sound" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_telnet, "telnet" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_text, "text" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_unknown, "unknown" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_baddata, "baddata" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_delayed, "delayed" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_embed, "embed" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_insecure, "insecure" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_notfound, "notfound" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_sdendnote, "sdendnote" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_sdendnote_anc, "sdendnoteanc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_sdendnote_sym, "sdendnotesym" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_sdfootnote, "sdfootnote" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_sdfootnote_anc, "sdfootnoteanc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_sdfootnote_sym, "sdfootnotesym" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_FTN_anchor, "anc" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_FTN_symbol, "sym" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_WW_off, "OFF" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_WW_hard, "HARD" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_WW_soft, "SOFT" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_WW_virtual, "VIRTUAL" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_WW_physical, "PHYSICAL" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_on, "on" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_ET_url, "application/x-www-form-urlencoded" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_ET_multipart, "multipart/form-data" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_ET_text, "text/plain" );
