/*************************************************************************
 *
 *  $RCSfile: htmlkey2.cxx,v $
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

#include "htmlkywd.hxx"

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_doctype32,
    "HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"" );


// diese werden nur eingeschaltet
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_area, "AREA" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_base, "BASE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_comment, "!--" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_doctype, "!DOCTYPE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_embed, "EMBED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_figureoverlay, "OVERLAY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_horzrule, "HR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_horztab, "TAB" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_image, "IMG" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_image2, "IMAGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_input, "INPUT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_isindex, "ISINDEX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_linebreak, "BR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_li, "LI" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_link, "LINK" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_meta, "META" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_nextid, "NEXTID" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_nobr, "NOBR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_of, "OF" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_option, "OPTION" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_param, "PARAM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_range, "RANGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_spacer, "SPACER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_wbr, "WBR" );

// diese werden wieder abgeschaltet
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_abbreviation, "ABBREV" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_above, "ABOVE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_acronym, "ACRONYM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_address, "ADDRESS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_anchor, "A" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_applet, "APPLET" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_array, "ARRAY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_author, "AU" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_banner, "BANNER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_bar, "BAR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_basefont, "BASEFONT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_below, "BELOW" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_bigprint, "BIG" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_blink, "BLINK" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_blockquote, "BLOCKQUOTE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_blockquote30, "BQ" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_body, "BODY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_bold, "B" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_boldtext, "BT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_box, "BOX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_caption, "CAPTION" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_center, "CENTER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_citiation, "CITE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_code, "CODE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_col, "COL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_colgroup, "COLGROUP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_credit, "CREDIT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_dd, "DD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_deflist, "DL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_deletedtext, "DEL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_dirlist, "DIR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_division, "DIV" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_dot, "DOT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_doubledot, "DDOT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_dt, "DT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_emphasis, "EM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_figure, "FIG" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_font, "FONT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_footnote, "FN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_form, "FORM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_frame, "FRAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_frameset, "FRAMESET" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_hat, "HAT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_head1, "H1" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_head2, "H2" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_head3, "H3" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_head4, "H4" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_head5, "H5" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_head6, "H6" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_head, "HEAD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_html, "HTML" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_iframe, "IFRAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_ilayer, "ILAYER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_insertedtext, "INS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_italic, "I" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_item, "ITEM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_keyboard, "KBD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_language, "LANG" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_layer, "LAYER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_listheader, "LH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_map, "MAP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_math, "MATH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_menulist, "MENU" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_multicol, "MULTICOL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_noembed, "NOEMBED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_noframe, "NOFRAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_noframes, "NOFRAMES" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_noscript, "NOSCRIPT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_note, "NOTE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_orderlist, "OL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_parabreak, "P" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_person, "PERSON" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_plaintext, "T" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_preformtxt, "PRE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_root, "ROOT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_row, "ROW" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_sample, "SAMP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_script, "SCRIPT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_select, "SELECT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_shortquote, "Q" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_smallprint, "SMALL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_span, "SPAN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_squareroot, "AQRT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_strikethrough, "S" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_strong, "STRONG" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_style, "STYLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_subscript, "SUB" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_superscript, "SUP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_table, "TABLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_tablerow, "TR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_tabledata, "TD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_tableheader, "TH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_tbody, "TBODY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_teletype, "TT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_text, "TEXT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_textarea, "TEXTAREA" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_textflow, "TEXTFLOW" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_tfoot, "TFOOT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_thead, "THEAD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_tilde, "TILDE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_title, "TITLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_underline, "U" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_unorderlist, "UL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_variable, "VAR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_vector, "VEC" );

// obsolete features
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_xmp, "XMP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_listing, "LISTING" );

// proposed features
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_definstance, "DFN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_strike, "STRIKE" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_bgsound, "BGSOUND" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_comment2, "COMMENT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_marquee, "MARQUEE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_plaintext2, "PLAINTEXT" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_sdfield, "SDFIELD" );

// die Namen fuer alle Zeichen
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_lt, "lt" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_gt, "gt" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_amp, "amp" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_quot, "quot" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Aacute, "Aacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Agrave, "Agrave" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Acirc, "Acirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Atilde, "Atilde" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Aring, "Aring" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Auml, "Auml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_AElig, "AElig" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Ccedil, "Ccedil" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Eacute, "Eacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Egrave, "Egrave" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Ecirc, "Ecirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Euml, "Euml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Iacute, "Iacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Igrave, "Igrave" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Icirc, "Icirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Iuml, "Iuml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_ETH, "ETH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Ntilde, "Ntilde" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Oacute, "Oacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Ograve, "Ograve" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Ocirc, "Ocirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Otilde, "Otilde" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Ouml, "Ouml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Oslash, "Oslash" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Uacute, "Uacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Ugrave, "Ugrave" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Ucirc, "Ucirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Uuml, "Uuml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_Yacute, "Yacute" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_THORN, "THORN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_szlig, "szlig" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_aacute, "aacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_agrave, "agrave" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_acirc, "acirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_atilde, "atilde" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_aring, "aring" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_auml, "auml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_aelig, "aelig" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ccedil, "ccedil" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_eacute, "eacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_egrave, "egrave" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ecirc, "ecirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_euml, "euml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_iacute, "iacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_igrave, "igrave" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_icirc, "icirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_iuml, "iuml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_eth, "eth" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ntilde, "ntilde" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_oacute, "oacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ograve, "ograve" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ocirc, "ocirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_otilde, "otilde" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ouml, "ouml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_oslash, "oslash" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_uacute, "uacute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ugrave, "ugrave" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ucirc, "ucirc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_uuml, "uuml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_yacute, "yacute" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_thorn, "thorn" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_yuml, "yuml" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_acute, "acute" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_brvbar, "brvbar" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_cedil, "cedil" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_cent, "cent" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_copy, "copy" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_curren, "curren" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_deg, "deg" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_divide, "divide" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_frac12, "frac12" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_frac14, "frac14" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_frac34, "frac34" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_iexcl, "iexcl" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_iquest, "iquest" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_laquo, "laquo" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_macr, "macr" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_micro, "micro" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_middot, "middot" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_nbsp, "nbsp" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_not, "not" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ordf, "ordf" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_ordm, "ordm" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_para, "para" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_plusmn, "plusmn" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_pound, "pound" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_raquo, "raquo" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_reg, "reg" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_sect, "sect" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_shy, "shy" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_sup1, "sup1" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_sup2, "sup2" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_sup3, "sup3" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_times, "times" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_uml, "uml" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_yen, "yen" );

// Netscape kennt noch ein paar in Grossbuchstaben ...
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_LT, "LT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_GT, "GT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_AMP, "AMP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_C_QUOT, "QUOT" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_COPY, "COPY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_S_REG, "REG" );

// HTML Attribut-Token (=Optionen)

// Attribute ohne Wert
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_box, "BOX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_checked, "CHECKED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_compact, "COMPACT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_continue, "CONTINUE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_controls, "CONTROLS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_declare, "DECLARE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_disabled, "DISABLED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_folded, "FOLDED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_ismap, "ISMAP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_mayscript, "MAYSCRIPT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_multiple, "MULTIPLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_noflow, "NOFLOW" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_nohref, "NOHREF" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_noresize, "NORESIZE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_noshade, "NOSHADE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_nowrap, "NOWRAP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_plain, "PLAIN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_sdfixed, "SDFIXED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_selected, "SELECTED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_shapes, "SHAPES" );

// Attribute mit einem String als Wert
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_above, "ABOVE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_accesskey, "ACCESSKEY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_accept, "ACCEPT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_add_date, "ADD_DATE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_alt, "ALT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_axes, "AXES" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_axis, "AXIS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_below, "BELOW" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_char, "CHAR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_class, "CLASS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_clip, "CLIP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_code, "CODE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_codetype, "CODETYPE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_colspec, "COLSPEC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_content, "CONTENT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_coords, "COORDS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_dp, "DP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_enctype, "ENCTYPE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_error, "ERROR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_face, "FACE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_frameborder, "FRAMEBORDER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_httpequiv, "HTTP-EQUIV" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_language, "LANGUAGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_last_modified, "LAST_MODIFIED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_last_visit, "LAST_VISIT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_md, "MD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_n, "N" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_name, "NAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_notation, "NOTATION" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_prompt, "PROMPT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_shape, "SHAPE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_standby, "STANDBY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_style, "STYLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_title, "TITLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_value, "VALUE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDval, "SDVAL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDnum, "SDNUM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_sdlibrary, "SDLIBRARY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_sdmodule, "SDMODULE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_sdevent, "SDEVENT-" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_sdaddparam, "SDADDPARAM-" );

// Attribute mit einem SGML-Identifier als Wert
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_from, "FROM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_id, "ID" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_target, "TARGET" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_to, "TO" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_until, "UNTIL" );

// Attribute mit einem URI als Wert
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_action, "ACTION" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_background, "BACKGROUND" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_classid, "CLASSID" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_codebase, "CODEBASE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_data, "DATA" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_dynsrc, "DYNSRC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_dynsync, "DYNSYNC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_imagemap, "IMAGEMAP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_href, "HREF" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_lowsrc, "LOWSRC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_script, "SCRIPT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_src, "SRC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_usemap, "USEMAP" );

// Attribute mit Entity-Namen als Wert
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_dingbat, "DINGBAT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_sym, "SYM" );

// Attribute mit einer Farbe als Wert (alle Netscape)
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_alink, "ALINK" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_bgcolor, "BGCOLOR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_bordercolor, "BORDERCOLOR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_bordercolorlight, "BORDERCOLORLIGHT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_bordercolordark, "BORDERCOLORDARK" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_color, "COLOR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_link, "LINK" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_text, "TEXT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_vlink, "VLINK" );

// Attribute mit einem numerischen Wert
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_border, "BORDER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_cellspacing, "CELLSPACING" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_cellpadding, "CELLPADDING" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_charoff, "CHAROFF" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_colspan, "COLSPAN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_framespacing, "FRAMESPACING" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_gutter, "GUTTER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_indent, "INDENT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_height, "HEIGHT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_hspace, "HSPACE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_left, "LEFT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_leftmargin, "LEFTMARGIN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_loop, "LOOP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_marginheight, "MARGINHEIGHT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_marginwidth, "MARGINWIDTH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_max, "MAX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_maxlength, "MAXLENGTH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_min, "MIN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_pagex, "PAGEX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_pagey, "PAGEY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_pointsize, "POINT-SIZE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_rowspan, "ROWSPAN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_scrollamount, "SCROLLAMOUNT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_scrolldelay, "SCROLLDELAY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_seqnum, "SEQNUM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_skip, "SKIP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_span, "SPAN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_tabindex, "TABINDEX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_top, "TOP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_topmargin, "TOPMARGIN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_vspace, "VSPACE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_weight, "WEIGHT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_width, "WIDTH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_x, "X" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_y, "Y" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_zindex, "Z-INDEX" );

// Attribute mit Enum-Werten
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_behavior, "BEHAVIOR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_bgproperties, "BGPROPERTIES" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_clear, "CLEAR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_dir, "DIR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_direction, "DIRECTION" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_format, "FORMAT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_frame, "FRAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_lang, "LANG" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_method, "METHOD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_palette, "PALETTE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_rel, "REL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_rev, "REV" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_rules, "RULES" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_scrolling, "SCROLLING" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_sdreadonly, "READONLY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_subtype, "SUBTYPE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_type, "TYPE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_valign, "VALIGN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_valuetype, "VALUETYPE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_visibility, "VISIBILITY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_wrap, "WRAP" );

// Attribute mit Script-Code als Wert
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onblur, "ONBLUR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onchange, "ONCHANGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onclick, "ONCLICK" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onfocus, "ONFOCUS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onload, "ONLOAD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onmouseover, "ONMOUSEOVER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onreset, "ONRESET" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onselect, "ONSELECT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onsubmit, "ONSUBMIT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onunload, "ONUNLOAD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onabort, "ONABORT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onerror, "ONERROR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_onmouseout, "ONMOUSEOUT" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonblur, "SDONBLUR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonchange, "SDONCHANGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonclick, "SDONCLICK" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonfocus, "SDONFOCUS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonload, "SDONLOAD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonmouseover, "SDONMOUSEOVER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonreset, "SDONRESET" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonselect, "SDONSELECT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonsubmit, "SDONSUBMIT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonunload, "SDONUNLOAD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonabort, "SDONABORT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonerror, "SDONERROR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_SDonmouseout, "SDONMOUSEOUT" );

// Attribute mit Kontext-abhaengigen Werten
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_align, "ALIGN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_cols, "COLS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_rows, "ROWS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_start, "START" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_size, "SIZE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_units, "UNITS" );


// Werte von <INPUT TYPE=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_text, "TEXT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_password, "PASSWORD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_checkbox, "CHECKBOX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_radio, "RADIO" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_range, "RANGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_scribble, "SCRIBBLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_file, "FILE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_hidden, "HIDDEN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_submit, "SUBMIT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_image, "IMAGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_reset, "RESET" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_IT_button, "BUTTON" );

// Werte von <TABLE FRAME=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_void, "VOID" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_above, "ABOVE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_below, "BELOW" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_hsides, "HSIDES" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_lhs, "LHS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_rhs, "RHS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_vsides, "VSIDES" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_box, "BOX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TF_border, "BORDER" );

// Werte von <TABLE RULES=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TR_none, "NONE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TR_groups, "GROUPS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TR_rows, "ROWS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TR_cols, "COLS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_TR_all, "ALL" );

// Werte von <P, H?, TR, TH, TD ALIGN=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_AL_left, "LEFT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_AL_center, "CENTER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_AL_middle, "MIDDLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_AL_right, "RIGHT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_AL_justify, "JUSTIFY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_AL_char, "CHAR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_AL_all, "ALL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_AL_none, "NONE" );

// Werte von <TR VALIGN=...>, <IMG ALIGN=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_VA_top, "TOP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_VA_middle, "MIDDLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_VA_bottom, "BOTTOM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_VA_baseline, "BASELINE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_VA_texttop, "TEXTTOP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_VA_absmiddle, "ABSMIDDLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_VA_absbottom, "ABSBOTTOM" );

// Werte von <AREA SHAPE=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SH_rect, "RECT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SH_rectangle, "RECTANGLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SH_circ, "CIRC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SH_circle, "CIRCLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SH_poly, "POLY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SH_polygon, "POLYGON" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SH_default, "DEFAULT" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_LG_starbasic, "STARBASIC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_LG_javascript, "JAVASCRIPT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_LG_javascript11, "JAVASCRIPT1.1" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_LG_livescript, "LIVESCRIPT" );
//sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_LG_vbscript, "VBSCRIPT" );
//sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_LG_unused_javascript, "UNUSED JAVASCRIPT" );
//sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_LG_starone, "StarScript" );

// ein par Werte fuer unser StarBASIC-Support
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SB_library, "$LIBRARY:" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SB_module, "$MODULE:" );


// Werte von <FORM METHOD=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_METHOD_get, "GET" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_METHOD_post, "POST" );

// Werte von <META CONTENT/HTTP-EQUIV=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_refresh, "REFRESH" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_generator, "GENERATOR" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_author, "AUTHOR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_classification, "CLASSIFICATION" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_description, "DESCRIPTION" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_keywords, "KEYWORDS" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_changed, "CHANGED" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_changedby, "CHANGEDBY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_created, "CREATED" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_content_type, "CONTENT-TYPE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_content_script_type, "CONTENT-SCRIPT-TYPE" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_sdendnote, "SDENDNOTE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_META_sdfootnote, "SDFOOTNOTE" );

// Werte von <UL TYPE=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_ULTYPE_disc, "DISC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_ULTYPE_square, "SQUARE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_ULTYPE_circle, "CIRCLE" );

// Werte von <FRAMESET SCROLLING=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SCROLL_yes, "YES" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SCROLL_no, "NO" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SCROLL_auto, "AUTO" );

// Werte von <MULTICOL TYPE=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_MCTYPE_horizontal, "HORIZONTAL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_MCTYPE_vertical, "VERTICAL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_MCTYPE_box, "BOX" );

// Werte von <MARQUEE BEHAVIOUR=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_BEHAV_scroll, "SCROLL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_BEHAV_slide, "SLIDE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_BEHAV_alternate, "ALTERNATE" );

// Werte von <MARQUEE LOOP=...>
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_LOOP_infinite, "INFINITE" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SPTYPE_block, "BLOCK" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SPTYPE_horizontal, "HORIZONTAL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_SPTYPE_vertical, "VERTICAL" );


// interne Grafik-Namen
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_private_image, "private:image/" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_internal_gopher, "internal-gopher-" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_internal_icon, "internal-icon-" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_binary, "binary" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_image, "image" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_index, "index" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_menu, "menu" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_movie, "movie" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_sound, "sound" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_telnet, "telnet" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_text, "text" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_GOPHER_unknown, "unknown" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_baddata, "baddata" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_delayed, "delayed" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_embed, "embed" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_insecure, "insecure" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_INT_ICON_notfound, "notfound" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_sdendnote, "sdendnote" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_sdendnote_anc, "sdendnoteanc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_sdendnote_sym, "sdendnotesym" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_sdfootnote, "sdfootnote" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_sdfootnote_anc, "sdfootnoteanc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_sdfootnote_sym, "sdfootnotesym" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FTN_anchor, "anc" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FTN_symbol, "sym" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_WW_off, "OFF" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_WW_hard, "HARD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_WW_soft, "SOFT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_WW_virtual, "VIRTUAL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_WW_physical, "PHYSICAL" );

sal_Char  __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_on, "on" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_ET_url, "application/x-www-form-urlencoded" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_ET_multipart, "multipart/form-data" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_ET_text, "text/plain" );
