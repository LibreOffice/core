/*************************************************************************
 *
 *  $RCSfile: htmlkywd.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:51 $
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

#ifndef _HTMLKYWD_HXX
#define _HTMLKYWD_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef SVTOOLS_CONSTASCII_DECL
#define SVTOOLS_CONSTASCII_DECL( n, s ) n[sizeof(s)]
#endif
#ifndef SVTOOLS_CONSTASCII_DEF
#define SVTOOLS_CONSTASCII_DEF( n, s ) n[sizeof(s)] = s
#endif


extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_doctype32,
    "HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"" );


// diese werden nur eingeschaltet
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_area, "AREA" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_base, "BASE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_comment, "!--" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_doctype, "!DOCTYPE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_embed, "EMBED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_figureoverlay, "OVERLAY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_horzrule, "HR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_horztab, "TAB" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_image, "IMG" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_image2, "IMAGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_input, "INPUT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_isindex, "ISINDEX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_linebreak, "BR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_li, "LI" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_link, "LINK" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_meta, "META" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_nextid, "NEXTID" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_nobr, "NOBR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_of, "OF" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_option, "OPTION" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_param, "PARAM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_range, "RANGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_spacer, "SPACER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_wbr, "WBR" );

// diese werden wieder abgeschaltet
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_abbreviation, "ABBREV" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_above, "ABOVE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_acronym, "ACRONYM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_address, "ADDRESS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_anchor, "A" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_applet, "APPLET" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_array, "ARRAY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_author, "AU" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_banner, "BANNER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_bar, "BAR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_basefont, "BASEFONT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_below, "BELOW" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_bigprint, "BIG" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_blink, "BLINK" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_blockquote, "BLOCKQUOTE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_blockquote30, "BQ" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_body, "BODY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_bold, "B" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_boldtext, "BT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_box, "BOX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_caption, "CAPTION" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_center, "CENTER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_citiation, "CITE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_code, "CODE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_col, "COL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_colgroup, "COLGROUP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_credit, "CREDIT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_dd, "DD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_deflist, "DL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_deletedtext, "DEL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_dirlist, "DIR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_division, "DIV" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_dot, "DOT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_doubledot, "DDOT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_dt, "DT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_emphasis, "EM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_figure, "FIG" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_font, "FONT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_footnote, "FN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_form, "FORM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_frame, "FRAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_frameset, "FRAMESET" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_hat, "HAT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_head1, "H1" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_head2, "H2" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_head3, "H3" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_head4, "H4" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_head5, "H5" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_head6, "H6" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_head, "HEAD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_html, "HTML" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_iframe, "IFRAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_ilayer, "ILAYER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_insertedtext, "INS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_italic, "I" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_item, "ITEM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_keyboard, "KBD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_language, "LANG" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_layer, "LAYER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_listheader, "LH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_map, "MAP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_math, "MATH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_menulist, "MENU" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_multicol, "MULTICOL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_noembed, "NOEMBED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_noframe, "NOFRAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_noframes, "NOFRAMES" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_noscript, "NOSCRIPT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_note, "NOTE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_orderlist, "OL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_parabreak, "P" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_person, "PERSON" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_plaintext, "T" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_preformtxt, "PRE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_root, "ROOT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_row, "ROW" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_sample, "SAMP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_script, "SCRIPT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_select, "SELECT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_shortquote, "Q" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_smallprint, "SMALL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_span, "SPAN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_squareroot, "AQRT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_strikethrough, "S" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_strong, "STRONG" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_style, "STYLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_subscript, "SUB" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_superscript, "SUP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_table, "TABLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_tablerow, "TR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_tabledata, "TD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_tableheader, "TH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_tbody, "TBODY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_teletype, "TT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_text, "TEXT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_textarea, "TEXTAREA" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_textflow, "TEXTFLOW" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_tfoot, "TFOOT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_thead, "THEAD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_tilde, "TILDE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_title, "TITLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_underline, "U" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_unorderlist, "UL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_variable, "VAR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_vector, "VEC" );

// obsolete features
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_xmp, "XMP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_listing, "LISTING" );

// proposed features
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_definstance, "DFN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_strike, "STRIKE" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_bgsound, "BGSOUND" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_comment2, "COMMENT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_marquee, "MARQUEE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_plaintext2, "PLAINTEXT" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_sdfield, "SDFIELD" );

// die Namen fuer alle Zeichen
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_lt, "lt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_gt, "gt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_amp, "amp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_quot, "quot" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Aacute, "Aacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Agrave, "Agrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Acirc, "Acirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Atilde, "Atilde" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Aring, "Aring" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Auml, "Auml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_AElig, "AElig" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Ccedil, "Ccedil" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Eacute, "Eacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Egrave, "Egrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Ecirc, "Ecirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Euml, "Euml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Iacute, "Iacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Igrave, "Igrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Icirc, "Icirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Iuml, "Iuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_ETH, "ETH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Ntilde, "Ntilde" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Oacute, "Oacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Ograve, "Ograve" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Ocirc, "Ocirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Otilde, "Otilde" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Ouml, "Ouml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Oslash, "Oslash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Uacute, "Uacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Ugrave, "Ugrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Ucirc, "Ucirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Uuml, "Uuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_Yacute, "Yacute" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_THORN, "THORN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_szlig, "szlig" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_aacute, "aacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_agrave, "agrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_acirc, "acirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_atilde, "atilde" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_aring, "aring" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_auml, "auml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_aelig, "aelig" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ccedil, "ccedil" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_eacute, "eacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_egrave, "egrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ecirc, "ecirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_euml, "euml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_iacute, "iacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_igrave, "igrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_icirc, "icirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_iuml, "iuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_eth, "eth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ntilde, "ntilde" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_oacute, "oacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ograve, "ograve" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ocirc, "ocirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_otilde, "otilde" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ouml, "ouml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_oslash, "oslash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_uacute, "uacute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ugrave, "ugrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ucirc, "ucirc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_uuml, "uuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_yacute, "yacute" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_thorn, "thorn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_yuml, "yuml" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_acute, "acute" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_brvbar, "brvbar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_cedil, "cedil" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_cent, "cent" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_copy, "copy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_curren, "curren" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_deg, "deg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_divide, "divide" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_frac12, "frac12" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_frac14, "frac14" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_frac34, "frac34" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_iexcl, "iexcl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_iquest, "iquest" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_laquo, "laquo" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_macr, "macr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_micro, "micro" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_middot, "middot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_nbsp, "nbsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_not, "not" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ordf, "ordf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_ordm, "ordm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_para, "para" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_plusmn, "plusmn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_pound, "pound" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_raquo, "raquo" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_reg, "reg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_sect, "sect" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_shy, "shy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_sup1, "sup1" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_sup2, "sup2" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_sup3, "sup3" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_times, "times" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_uml, "uml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_yen, "yen" );

// Netscape kennt noch ein paar in Grossbuchstaben ...
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_LT, "LT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_GT, "GT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_AMP, "AMP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_C_QUOT, "QUOT" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_COPY, "COPY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_S_REG, "REG" );

// HTML Attribut-Token (=Optionen)

// Attribute ohne Wert
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_box, "BOX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_checked, "CHECKED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_compact, "COMPACT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_continue, "CONTINUE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_controls, "CONTROLS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_declare, "DECLARE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_disabled, "DISABLED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_folded, "FOLDED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_ismap, "ISMAP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_mayscript, "MAYSCRIPT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_multiple, "MULTIPLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_noflow, "NOFLOW" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_nohref, "NOHREF" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_noresize, "NORESIZE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_noshade, "NOSHADE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_nowrap, "NOWRAP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_plain, "PLAIN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_sdfixed, "SDFIXED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_selected, "SELECTED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_shapes, "SHAPES" );

// Attribute mit einem String als Wert
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_above, "ABOVE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_accesskey, "ACCESSKEY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_accept, "ACCEPT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_add_date, "ADD_DATE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_alt, "ALT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_axes, "AXES" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_axis, "AXIS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_below, "BELOW" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_char, "CHAR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_class, "CLASS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_clip, "CLIP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_code, "CODE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_codetype, "CODETYPE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_colspec, "COLSPEC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_content, "CONTENT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_coords, "COORDS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_dp, "DP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_enctype, "ENCTYPE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_error, "ERROR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_face, "FACE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_frameborder, "FRAMEBORDER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_httpequiv, "HTTP-EQUIV" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_language, "LANGUAGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_last_modified, "LAST_MODIFIED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_last_visit, "LAST_VISIT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_md, "MD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_n, "N" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_name, "NAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_notation, "NOTATION" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_prompt, "PROMPT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_shape, "SHAPE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_standby, "STANDBY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_style, "STYLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_title, "TITLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_value, "VALUE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDval, "SDVAL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDnum, "SDNUM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_sdlibrary, "SDLIBRARY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_sdmodule, "SDMODULE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_sdevent, "SDEVENT-" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_sdaddparam, "SDADDPARAM-" );

// Attribute mit einem SGML-Identifier als Wert
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_from, "FROM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_id, "ID" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_target, "TARGET" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_to, "TO" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_until, "UNTIL" );

// Attribute mit einem URI als Wert
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_action, "ACTION" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_background, "BACKGROUND" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_classid, "CLASSID" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_codebase, "CODEBASE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_data, "DATA" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_dynsrc, "DYNSRC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_dynsync, "DYNSYNC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_imagemap, "IMAGEMAP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_href, "HREF" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_lowsrc, "LOWSRC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_script, "SCRIPT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_src, "SRC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_usemap, "USEMAP" );

// Attribute mit Entity-Namen als Wert
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_dingbat, "DINGBAT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_sym, "SYM" );

// Attribute mit einer Farbe als Wert (alle Netscape)
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_alink, "ALINK" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_bgcolor, "BGCOLOR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_bordercolor, "BORDERCOLOR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_bordercolorlight, "BORDERCOLORLIGHT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_bordercolordark, "BORDERCOLORDARK" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_color, "COLOR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_link, "LINK" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_text, "TEXT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_vlink, "VLINK" );

// Attribute mit einem numerischen Wert
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_border, "BORDER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_cellspacing, "CELLSPACING" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_cellpadding, "CELLPADDING" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_charoff, "CHAROFF" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_colspan, "COLSPAN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_framespacing, "FRAMESPACING" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_gutter, "GUTTER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_indent, "INDENT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_height, "HEIGHT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_hspace, "HSPACE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_left, "LEFT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_leftmargin, "LEFTMARGIN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_loop, "LOOP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_marginheight, "MARGINHEIGHT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_marginwidth, "MARGINWIDTH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_max, "MAX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_maxlength, "MAXLENGTH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_min, "MIN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_pagex, "PAGEX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_pagey, "PAGEY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_pointsize, "POINT-SIZE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_rowspan, "ROWSPAN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_scrollamount, "SCROLLAMOUNT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_scrolldelay, "SCROLLDELAY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_seqnum, "SEQNUM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_skip, "SKIP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_span, "SPAN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_tabindex, "TABINDEX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_top, "TOP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_topmargin, "TOPMARGIN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_vspace, "VSPACE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_weight, "WEIGHT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_width, "WIDTH" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_x, "X" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_y, "Y" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_zindex, "Z-INDEX" );

// Attribute mit Enum-Werten
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_behavior, "BEHAVIOR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_bgproperties, "BGPROPERTIES" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_clear, "CLEAR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_dir, "DIR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_direction, "DIRECTION" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_format, "FORMAT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_frame, "FRAME" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_lang, "LANG" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_method, "METHOD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_palette, "PALETTE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_rel, "REL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_rev, "REV" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_rules, "RULES" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_scrolling, "SCROLLING" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_sdreadonly, "READONLY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_subtype, "SUBTYPE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_type, "TYPE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_valign, "VALIGN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_valuetype, "VALUETYPE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_visibility, "VISIBILITY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_wrap, "WRAP" );

// Attribute mit Script-Code als Wert
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onblur, "ONBLUR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onchange, "ONCHANGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onclick, "ONCLICK" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onfocus, "ONFOCUS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onload, "ONLOAD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onmouseover, "ONMOUSEOVER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onreset, "ONRESET" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onselect, "ONSELECT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onsubmit, "ONSUBMIT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onunload, "ONUNLOAD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onabort, "ONABORT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onerror, "ONERROR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_onmouseout, "ONMOUSEOUT" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonblur, "SDONBLUR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonchange, "SDONCHANGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonclick, "SDONCLICK" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonfocus, "SDONFOCUS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonload, "SDONLOAD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonmouseover, "SDONMOUSEOVER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonreset, "SDONRESET" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonselect, "SDONSELECT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonsubmit, "SDONSUBMIT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonunload, "SDONUNLOAD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonabort, "SDONABORT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonerror, "SDONERROR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_SDonmouseout, "SDONMOUSEOUT" );

// Attribute mit Kontext-abhaengigen Werten
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_align, "ALIGN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_cols, "COLS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_rows, "ROWS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_start, "START" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_size, "SIZE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_units, "UNITS" );


// Werte von <INPUT TYPE=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_text, "TEXT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_password, "PASSWORD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_checkbox, "CHECKBOX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_radio, "RADIO" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_range, "RANGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_scribble, "SCRIBBLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_file, "FILE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_hidden, "HIDDEN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_submit, "SUBMIT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_image, "IMAGE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_reset, "RESET" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_IT_button, "BUTTON" );

// Werte von <TABLE FRAME=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_void, "VOID" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_above, "ABOVE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_below, "BELOW" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_hsides, "HSIDES" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_lhs, "LHS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_rhs, "RHS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_vsides, "VSIDES" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_box, "BOX" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TF_border, "BORDER" );

// Werte von <TABLE RULES=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TR_none, "NONE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TR_groups, "GROUPS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TR_rows, "ROWS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TR_cols, "COLS" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_TR_all, "ALL" );

// Werte von <P, H?, TR, TH, TD ALIGN=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_AL_left, "LEFT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_AL_center, "CENTER" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_AL_middle, "MIDDLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_AL_right, "RIGHT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_AL_justify, "JUSTIFY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_AL_char, "CHAR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_AL_all, "ALL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_AL_none, "NONE" );

// Werte von <TR VALIGN=...>, <IMG ALIGN=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_VA_top, "TOP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_VA_middle, "MIDDLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_VA_bottom, "BOTTOM" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_VA_baseline, "BASELINE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_VA_texttop, "TEXTTOP" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_VA_absmiddle, "ABSMIDDLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_VA_absbottom, "ABSBOTTOM" );

// Werte von <AREA SHAPE=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SH_rect, "RECT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SH_rectangle, "RECTANGLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SH_circ, "CIRC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SH_circle, "CIRCLE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SH_poly, "POLY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SH_polygon, "POLYGON" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SH_default, "DEFAULT" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_LG_starbasic, "STARBASIC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_LG_javascript, "JAVASCRIPT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_LG_javascript11, "JAVASCRIPT1.1" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_LG_livescript, "LIVESCRIPT" );
//extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_LG_vbscript, "VBSCRIPT" );
//extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_LG_unused_javascript, "UNUSED JAVASCRIPT" );
//extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_LG_starone, "StarScript" );

// ein par Werte fuer unser StarBASIC-Support
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SB_library, "$LIBRARY:" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SB_module, "$MODULE:" );


// Werte von <FORM METHOD=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_METHOD_get, "GET" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_METHOD_post, "POST" );

// Werte von <META CONTENT/HTTP-EQUIV=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_refresh, "REFRESH" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_generator, "GENERATOR" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_author, "AUTHOR" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_classification, "CLASSIFICATION" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_description, "DESCRIPTION" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_keywords, "KEYWORDS" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_changed, "CHANGED" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_changedby, "CHANGEDBY" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_created, "CREATED" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_content_type, "CONTENT-TYPE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_content_script_type, "CONTENT-SCRIPT-TYPE" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_sdendnote, "SDENDNOTE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_META_sdfootnote, "SDFOOTNOTE" );

// Werte von <UL TYPE=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_ULTYPE_disc, "DISC" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_ULTYPE_square, "SQUARE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_ULTYPE_circle, "CIRCLE" );

// Werte von <FRAMESET SCROLLING=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SCROLL_yes, "YES" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SCROLL_no, "NO" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SCROLL_auto, "AUTO" );

// Werte von <MULTICOL TYPE=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_MCTYPE_horizontal, "HORIZONTAL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_MCTYPE_vertical, "VERTICAL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_MCTYPE_box, "BOX" );

// Werte von <MARQUEE BEHAVIOUR=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_BEHAV_scroll, "SCROLL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_BEHAV_slide, "SLIDE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_BEHAV_alternate, "ALTERNATE" );

// Werte von <MARQUEE LOOP=...>
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_LOOP_infinite, "INFINITE" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SPTYPE_block, "BLOCK" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SPTYPE_horizontal, "HORIZONTAL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_SPTYPE_vertical, "VERTICAL" );


// interne Grafik-Namen
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_private_image, "private:image/" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_internal_gopher, "internal-gopher-" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_internal_icon, "internal-icon-" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_binary, "binary" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_image, "image" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_index, "index" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_menu, "menu" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_movie, "movie" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_sound, "sound" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_telnet, "telnet" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_text, "text" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_GOPHER_unknown, "unknown" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_baddata, "baddata" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_delayed, "delayed" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_embed, "embed" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_insecure, "insecure" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_INT_ICON_notfound, "notfound" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_sdendnote, "sdendnote" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_sdendnote_anc, "sdendnoteanc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_sdendnote_sym, "sdendnotesym" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_sdfootnote, "sdfootnote" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_sdfootnote_anc, "sdfootnoteanc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_sdfootnote_sym, "sdfootnotesym" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FTN_anchor, "anc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_FTN_symbol, "sym" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_WW_off, "OFF" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_WW_hard, "HARD" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_WW_soft, "SOFT" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_WW_virtual, "VIRTUAL" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_WW_physical, "PHYSICAL" );

extern sal_Char  __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_on, "on" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_ET_url, "application/x-www-form-urlencoded" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_ET_multipart, "multipart/form-data" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_ET_text, "text/plain" );


#endif
