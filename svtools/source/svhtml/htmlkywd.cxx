/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <algorithm>
#include <limits.h>
#include <sal/types.h>
#include <stdlib.h>
#include <string.h>
#include <rtl/ustring.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <sal/log.hxx>

template<typename T>
struct TokenEntry
{
    OUStringLiteral sToken;
    T nToken;
};
template<typename T>
static bool sortCompare(const TokenEntry<T> & lhs, const TokenEntry<T> & rhs)
{
    return strcmp(lhs.sToken.data, rhs.sToken.data) < 0;
}
template<typename T>
static bool findCompare(const TokenEntry<T> & lhs, const OUString & rhs)
{
    return rhs.compareToAscii(lhs.sToken.data) > 0;
}
template<typename T, size_t LEN>
static T search(TokenEntry<T> const (&dataTable)[LEN], const OUString & key, T notFoundValue)
{
    auto findIt = std::lower_bound( std::begin(dataTable), std::end(dataTable),
                                     key, findCompare<T> );
    if (findIt != std::end(dataTable) && key.compareToAscii(findIt->sToken.data)==0)
        return findIt->nToken;
    return notFoundValue;
}

using HTML_TokenEntry = TokenEntry<HtmlTokenId>;

// this array is sorted by the name (even if it doesn't look like it from the constant names)
static HTML_TokenEntry const aHTMLTokenTab[] = {
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_comment),         HtmlTokenId::COMMENT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_doctype),         HtmlTokenId::DOCTYPE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_anchor),          HtmlTokenId::ANCHOR_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_abbreviation),    HtmlTokenId::ABBREVIATION_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_acronym),         HtmlTokenId::ACRONYM_ON},   // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_address),         HtmlTokenId::ADDRESS_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_applet),          HtmlTokenId::APPLET_ON},    // HotJava
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_area),            HtmlTokenId::AREA}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_author),          HtmlTokenId::AUTHOR_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_bold),            HtmlTokenId::BOLD_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_banner),          HtmlTokenId::BANNER_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_base),            HtmlTokenId::BASE}, // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_basefont),        HtmlTokenId::BASEFONT_ON},  // Netscape
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_bigprint),        HtmlTokenId::BIGPRINT_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_blink),           HtmlTokenId::BLINK_ON}, // Netscape
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_blockquote),      HtmlTokenId::BLOCKQUOTE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_body),            HtmlTokenId::BODY_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_blockquote30),    HtmlTokenId::BLOCKQUOTE30_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_linebreak),       HtmlTokenId::LINEBREAK},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_caption),         HtmlTokenId::CAPTION_ON},   // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_center),          HtmlTokenId::CENTER_ON},    // Netscape
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_citiation),       HtmlTokenId::CITIATION_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_code),            HtmlTokenId::CODE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_col),             HtmlTokenId::COL_ON}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_colgroup),        HtmlTokenId::COLGROUP_ON}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_comment2),        HtmlTokenId::COMMENT2_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_credit),          HtmlTokenId::CREDIT_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_dd),              HtmlTokenId::DD_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_deletedtext),     HtmlTokenId::DELETEDTEXT_ON},   // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_definstance),     HtmlTokenId::DEFINSTANCE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_dirlist),         HtmlTokenId::DIRLIST_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_division),        HtmlTokenId::DIVISION_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_deflist),         HtmlTokenId::DEFLIST_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_dt),              HtmlTokenId::DT_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_emphasis),        HtmlTokenId::EMPHASIS_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_embed),           HtmlTokenId::EMBED},    // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_figure),          HtmlTokenId::FIGURE_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_footnote),        HtmlTokenId::FOOTNOTE_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_font),            HtmlTokenId::FONT_ON}, // Netscape
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_form),            HtmlTokenId::FORM_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_frame),           HtmlTokenId::FRAME_ON}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_frameset),        HtmlTokenId::FRAMESET_ON},  // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_head1),           HtmlTokenId::HEAD1_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_head2),           HtmlTokenId::HEAD2_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_head3),           HtmlTokenId::HEAD3_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_head4),           HtmlTokenId::HEAD4_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_head5),           HtmlTokenId::HEAD5_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_head6),           HtmlTokenId::HEAD6_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_head),            HtmlTokenId::HEAD_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_horzrule),        HtmlTokenId::HORZRULE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_html),            HtmlTokenId::HTML_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_italic),          HtmlTokenId::ITALIC_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_iframe),          HtmlTokenId::IFRAME_ON},    // IE 3.0b2
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_image),           HtmlTokenId::IMAGE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_input),           HtmlTokenId::INPUT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_insertedtext),    HtmlTokenId::INSERTEDTEXT_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_keyboard),        HtmlTokenId::KEYBOARD_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_language),        HtmlTokenId::LANGUAGE_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_listheader),      HtmlTokenId::LISTHEADER_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_li),              HtmlTokenId::LI_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_link),            HtmlTokenId::LINK}, // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_listing),         HtmlTokenId::LISTING_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_map),             HtmlTokenId::MAP_ON},   // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_marquee),         HtmlTokenId::MARQUEE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_menulist),        HtmlTokenId::MENULIST_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_meta),            HtmlTokenId::META}, // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_multicol),        HtmlTokenId::MULTICOL_ON},  // Netscape 3.0b5
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_nobr),            HtmlTokenId::NOBR_ON},  // Netscape
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_noembed),         HtmlTokenId::NOEMBED_ON},   // Netscape 2.0 ???
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_noframe),         HtmlTokenId::NOFRAMES_ON},  // Netscape 2.0 ???
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_noframes),        HtmlTokenId::NOFRAMES_ON},  // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_noscript),        HtmlTokenId::NOSCRIPT_ON},  // Netscape 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_note),            HtmlTokenId::NOTE_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_object),          HtmlTokenId::OBJECT_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_orderlist),       HtmlTokenId::ORDERLIST_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_option),          HtmlTokenId::OPTION},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_parabreak),       HtmlTokenId::PARABREAK_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_param),           HtmlTokenId::PARAM},    // HotJava
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_person),          HtmlTokenId::PERSON_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_plaintext2),      HtmlTokenId::PLAINTEXT2_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_preformtxt),      HtmlTokenId::PREFORMTXT_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_shortquote),      HtmlTokenId::SHORTQUOTE_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_strikethrough),   HtmlTokenId::STRIKETHROUGH_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_sample),          HtmlTokenId::SAMPLE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_script),          HtmlTokenId::SCRIPT_ON}, // HTML 3.2
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_sdfield),         HtmlTokenId::SDFIELD_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_select),          HtmlTokenId::SELECT_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_smallprint),      HtmlTokenId::SMALLPRINT_ON},    // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_spacer),          HtmlTokenId::SPACER},   // Netscape 3.0b5
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_span),            HtmlTokenId::SPAN_ON},  // Style Sheets
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_strike),          HtmlTokenId::STRIKE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_strong),          HtmlTokenId::STRONG_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_style),           HtmlTokenId::STYLE_ON}, // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_subscript),       HtmlTokenId::SUBSCRIPT_ON}, // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_superscript),     HtmlTokenId::SUPERSCRIPT_ON},   // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_plaintext),       HtmlTokenId::PLAINTEXT_ON}, // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_table),           HtmlTokenId::TABLE_ON}, // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_tbody),           HtmlTokenId::TBODY_ON}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_tabledata),       HtmlTokenId::TABLEDATA_ON}, // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_textarea),        HtmlTokenId::TEXTAREA_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_tfoot),           HtmlTokenId::TFOOT_ON}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_tableheader),     HtmlTokenId::TABLEHEADER_ON},   // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_thead),           HtmlTokenId::THEAD_ON}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_title),           HtmlTokenId::TITLE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_tablerow),        HtmlTokenId::TABLEROW_ON},  // HTML 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_teletype),        HtmlTokenId::TELETYPE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_underline),       HtmlTokenId::UNDERLINE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_unorderlist),     HtmlTokenId::UNORDERLIST_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_variable),        HtmlTokenId::VARIABLE_ON},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_xmp),             HtmlTokenId::XMP_ON},
};


HtmlTokenId GetHTMLToken( const OUString& rName )
{
    static bool bSortKeyWords = false;
    if( !bSortKeyWords )
    {
        assert( std::is_sorted( std::begin(aHTMLTokenTab), std::end(aHTMLTokenTab), sortCompare<HtmlTokenId> ) );
        bSortKeyWords = true;
    }

    if( rName.startsWith( OOO_STRING_SVTOOLS_HTML_comment ))
        return HtmlTokenId::COMMENT;

    return search( aHTMLTokenTab, rName, HtmlTokenId::NONE);
}

using HTML_CharEntry = TokenEntry<sal_Unicode>;

// Flag: RTF token table has already been sorted
static bool bSortCharKeyWords = false;

static HTML_CharEntry aHTMLCharNameTab[] = {
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_lt),             60},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_gt),             62},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_amp),        38},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_quot),       34},

    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Agrave),        192},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Aacute),        193},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Acirc),     194},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Atilde),        195},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Auml),      196},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Aring),     197},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_AElig),     198},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Ccedil),        199},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Egrave),        200},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Eacute),        201},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Ecirc),     202},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Euml),      203},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Igrave),        204},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Iacute),        205},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Icirc),     206},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Iuml),      207},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_ETH),       208},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Ntilde),        209},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Ograve),        210},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Oacute),        211},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Ocirc),     212},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Otilde),        213},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Ouml),      214},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Oslash),        216},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Ugrave),        217},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Uacute),        218},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Ucirc),     219},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Uuml),      220},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_Yacute),        221},

    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_THORN),     222},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_C_szlig),     223},

    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_agrave),        224},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_aacute),        225},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_acirc),     226},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_atilde),        227},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_auml),      228},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_aring),     229},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_aelig),     230},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ccedil),        231},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_egrave),        232},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_eacute),        233},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ecirc),     234},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_euml),      235},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_igrave),        236},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_iacute),        237},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_icirc),     238},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_iuml),      239},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_eth),       240},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ntilde),        241},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ograve),        242},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_oacute),        243},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ocirc),     244},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_otilde),        245},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ouml),      246},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_oslash),        248},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ugrave),        249},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_uacute),        250},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ucirc),     251},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_uuml),      252},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_yacute),        253},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_thorn),     254},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_yuml),      255},

// special characters
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_acute),     180},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_brvbar),    166},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_cedil),      184},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_cent),       162},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_copy),      169},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_curren),     164},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_deg),        176},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_divide),     247},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_frac12),     189},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_frac14),     188},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_frac34),     190},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_iexcl),      161},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_iquest),     191},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_laquo),      171},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_macr),       175},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_micro),      181},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_middot),     183},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_not),        172},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ordf),       170},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ordm),       186},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_para),       182},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_plusmn),     177},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_pound),      163},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_raquo),      187},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_reg),       174},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sect),       167},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sup1),       185},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sup2),       178},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sup3),       179},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_times),      215},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_uml),        168},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_yen),        165},

// special characters), which will be converted to tokens !!!
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_nbsp),      1},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_shy),       2},


// HTML4
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_OElig),     338},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_oelig),     339},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Scaron),        352},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_scaron),        353},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Yuml),      376},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_fnof),      402},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_circ),      710},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_tilde),     732},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Alpha),     913},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Beta),      914},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Gamma),     915},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Delta),     916},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Epsilon),   917},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Zeta),      918},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Eta),       919},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Theta),     920},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Iota),      921},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Kappa),     922},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Lambda),        923},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Mu),            924},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Nu),            925},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Xi),            926},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Omicron),   927},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Pi),            928},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Rho),       929},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Sigma),     931},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Tau),       932},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Upsilon),   933},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Phi),       934},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Chi),       935},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Psi),       936},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Omega),     937},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_alpha),     945},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_beta),      946},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_gamma),     947},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_delta),     948},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_epsilon),   949},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_zeta),      950},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_eta),       951},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_theta),     952},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_iota),      953},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_kappa),     954},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lambda),        955},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_mu),            956},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_nu),            957},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_xi),            958},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_omicron),   959},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_pi),            960},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rho),       961},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sigmaf),        962},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sigma),     963},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_tau),       964},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_upsilon),   965},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_phi),       966},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_chi),       967},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_psi),       968},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_omega),     969},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_thetasym),  977},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_upsih),     978},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_piv),       982},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ensp),      8194},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_emsp),      8195},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_thinsp),        8201},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_zwnj),      8204},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_zwj),       8205},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lrm),       8206},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rlm),       8207},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ndash),     8211},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_mdash),     8212},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lsquo),     8216},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rsquo),     8217},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sbquo),     8218},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ldquo),     8220},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rdquo),     8221},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_bdquo),     8222},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_dagger),        8224},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Dagger),        8225},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_bull),      8226},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_hellip),        8230},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_permil),        8240},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_prime),     8242},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_Prime),     8243},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lsaquo),        8249},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rsaquo),        8250},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_oline),     8254},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_frasl),     8260},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_euro),      8364},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_image),     8465},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_weierp),        8472},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_real),      8476},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_trade),     8482},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_alefsym),   8501},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_larr),      8592},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_uarr),      8593},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rarr),      8594},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_darr),      8595},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_harr),      8596},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_crarr),     8629},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lArr),      8656},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_uArr),      8657},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rArr),      8658},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_dArr),      8659},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_hArr),      8660},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_forall),        8704},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_part),      8706},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_exist),     8707},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_empty),     8709},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_nabla),     8711},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_isin),      8712},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_notin),     8713},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ni),            8715},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_prod),      8719},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sum),       8721},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_minus),     8722},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lowast),        8727},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_radic),     8730},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_prop),      8733},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_infin),     8734},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ang),       8736},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_and),       8743},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_or),            8744},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_cap),       8745},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_cup),       8746},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_int),       8747},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_there4),        8756},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sim),       8764},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_cong),      8773},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_asymp),     8776},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ne),            8800},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_equiv),     8801},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_le),            8804},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_ge),            8805},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sub),       8834},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sup),       8835},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_nsub),      8836},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sube),      8838},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_supe),      8839},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_oplus),     8853},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_otimes),        8855},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_perp),      8869},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_sdot),      8901},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lceil),     8968},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rceil),     8969},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lfloor),        8970},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rfloor),        8971},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_lang),      9001},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_rang),      9002},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_loz),       9674},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_spades),        9824},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_clubs),     9827},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_hearts),        9829},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_S_diams),     9830}
};

sal_Unicode GetHTMLCharName( const OUString& rName )
{
    if( !bSortCharKeyWords )
    {
        std::sort( std::begin(aHTMLCharNameTab), std::end(aHTMLCharNameTab),
                              sortCompare<sal_Unicode> );
        bSortCharKeyWords = true;
    }

    return search<sal_Unicode>( aHTMLCharNameTab, rName, 0);
}

// Flag: Options table has already been sorted
static bool bSortOptionKeyWords = false;

using HTML_OptionEntry = TokenEntry<HtmlOptionId>;

static HTML_OptionEntry aHTMLOptionTab[] = {

// Attributes without value
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_checked),   HtmlOptionId::CHECKED},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_compact),   HtmlOptionId::COMPACT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_declare),   HtmlOptionId::DECLARE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_disabled),  HtmlOptionId::DISABLED},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_ismap),     HtmlOptionId::ISMAP},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_mayscript), HtmlOptionId::MAYSCRIPT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_multiple),  HtmlOptionId::MULTIPLE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_nohref),        HtmlOptionId::NOHREF}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_noresize),  HtmlOptionId::NORESIZE}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_noshade),   HtmlOptionId::NOSHADE}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_nowrap),        HtmlOptionId::NOWRAP},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_sdfixed),   HtmlOptionId::SDFIXED},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_selected),      HtmlOptionId::SELECTED},

// Attributes with a string value
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_accept),        HtmlOptionId::ACCEPT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_accesskey), HtmlOptionId::ACCESSKEY},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_alt),       HtmlOptionId::ALT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_axis),      HtmlOptionId::AXIS},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_char),      HtmlOptionId::CHAR}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_charset),   HtmlOptionId::CHARSET},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_class),     HtmlOptionId::CLASS},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_code),      HtmlOptionId::CODE}, // HotJava
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_codetype),  HtmlOptionId::CODETYPE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_content),   HtmlOptionId::CONTENT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_coords),        HtmlOptionId::COORDS}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_enctype),   HtmlOptionId::ENCTYPE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_face),      HtmlOptionId::FACE}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_frameborder), HtmlOptionId::FRAMEBORDER}, // IExplorer 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_httpequiv), HtmlOptionId::HTTPEQUIV},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_language),  HtmlOptionId::LANGUAGE}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_name),      HtmlOptionId::NAME},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_prompt),        HtmlOptionId::PROMPT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_shape),     HtmlOptionId::SHAPE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_standby),   HtmlOptionId::STANDBY},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_style),     HtmlOptionId::STYLE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_title),     HtmlOptionId::TITLE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_value),     HtmlOptionId::VALUE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDval),     HtmlOptionId::SDVAL}, // StarDiv NumberValue
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDnum),     HtmlOptionId::SDNUM}, // StarDiv NumberFormat
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_sdlibrary), HtmlOptionId::SDLIBRARY},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_sdmodule),  HtmlOptionId::SDMODULE},

// Attributes with a SGML identifier value
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_id),            HtmlOptionId::ID},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_target),        HtmlOptionId::TARGET}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_to),            HtmlOptionId::TO},

// Attributes with an URI value
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_action),        HtmlOptionId::ACTION},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_archive),       HtmlOptionId::ARCHIVE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_background),    HtmlOptionId::BACKGROUND},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_classid),   HtmlOptionId::CLASSID},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_codebase),  HtmlOptionId::CODEBASE}, // HotJava
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_data),      HtmlOptionId::DATA},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_href),      HtmlOptionId::HREF},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_script),        HtmlOptionId::SCRIPT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_src),       HtmlOptionId::SRC},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_usemap),        HtmlOptionId::USEMAP}, // Netscape 2.0

// Attributes with a color value (all Netscape versions)
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_alink),     HtmlOptionId::ALINK},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_bgcolor),   HtmlOptionId::BGCOLOR},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_bordercolor), HtmlOptionId::BORDERCOLOR}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_bordercolorlight), HtmlOptionId::BORDERCOLORLIGHT}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_bordercolordark), HtmlOptionId::BORDERCOLORDARK}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_color),     HtmlOptionId::COLOR},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_link),      HtmlOptionId::LINK},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_text),      HtmlOptionId::TEXT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_vlink),     HtmlOptionId::VLINK},

// Attributes with a numerical value
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_border),        HtmlOptionId::BORDER},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_cellspacing),HtmlOptionId::CELLSPACING}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_cellpadding),HtmlOptionId::CELLPADDING}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_charoff),   HtmlOptionId::CHAROFF}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_colspan),   HtmlOptionId::COLSPAN},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_framespacing), HtmlOptionId::FRAMESPACING}, // IExplorer 3.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_gutter),        HtmlOptionId::GUTTER}, // Netscape 3.0b5
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_height),        HtmlOptionId::HEIGHT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_hspace),        HtmlOptionId::HSPACE}, // Netscape
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_left),      HtmlOptionId::LEFT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_loop),      HtmlOptionId::LOOP}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_marginheight),HtmlOptionId::MARGINHEIGHT}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_marginwidth),HtmlOptionId::MARGINWIDTH}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_maxlength), HtmlOptionId::MAXLENGTH},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_rowspan),   HtmlOptionId::ROWSPAN},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_scrollamount), HtmlOptionId::SCROLLAMOUNT}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_scrolldelay), HtmlOptionId::SCROLLDELAY}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_span),      HtmlOptionId::SPAN}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_tabindex),  HtmlOptionId::TABINDEX},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_vspace),        HtmlOptionId::VSPACE}, // Netscape
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_width),     HtmlOptionId::WIDTH},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_zindex),        HtmlOptionId::ZINDEX},

// Attributes with enum values
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_behavior),  HtmlOptionId::BEHAVIOR}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_clear),     HtmlOptionId::CLEAR},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_dir),       HtmlOptionId::DIR}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_direction),     HtmlOptionId::DIRECTION}, // IExplorer 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_format),        HtmlOptionId::FORMAT},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_frame),     HtmlOptionId::FRAME}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_lang),      HtmlOptionId::LANG},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_method),        HtmlOptionId::METHOD},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_rel),       HtmlOptionId::REL},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_rev),       HtmlOptionId::REV},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_rules),     HtmlOptionId::RULES}, // HTML 3 Table Model Draft
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_scrolling), HtmlOptionId::SCROLLING}, // Netscape 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_sdreadonly),    HtmlOptionId::SDREADONLY},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_subtype),   HtmlOptionId::SUBTYPE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_type),      HtmlOptionId::TYPE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_valign),        HtmlOptionId::VALIGN},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_valuetype), HtmlOptionId::VALUETYPE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_wrap),      HtmlOptionId::WRAP},

// Attributes with script code value
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onblur),        HtmlOptionId::ONBLUR}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onchange),  HtmlOptionId::ONCHANGE}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onclick),   HtmlOptionId::ONCLICK}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onfocus),   HtmlOptionId::ONFOCUS}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onload),    HtmlOptionId::ONLOAD}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onmouseover), HtmlOptionId::ONMOUSEOVER}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onreset),   HtmlOptionId::ONRESET}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onselect),  HtmlOptionId::ONSELECT}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onsubmit),  HtmlOptionId::ONSUBMIT}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onunload),  HtmlOptionId::ONUNLOAD}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onabort),   HtmlOptionId::ONABORT}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onerror),   HtmlOptionId::ONERROR}, // JavaScript
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_onmouseout),    HtmlOptionId::ONMOUSEOUT}, // JavaScript

    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonblur),      HtmlOptionId::SDONBLUR}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonchange),    HtmlOptionId::SDONCHANGE}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonclick),         HtmlOptionId::SDONCLICK}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonfocus),         HtmlOptionId::SDONFOCUS}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonload),      HtmlOptionId::SDONLOAD}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonmouseover),     HtmlOptionId::SDONMOUSEOVER}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonreset),     HtmlOptionId::SDONRESET}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonselect),        HtmlOptionId::SDONSELECT}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonsubmit),        HtmlOptionId::SDONSUBMIT}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonunload),        HtmlOptionId::SDONUNLOAD}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonabort),     HtmlOptionId::SDONABORT}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonerror),         HtmlOptionId::SDONERROR}, // StarBasic
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_SDonmouseout),  HtmlOptionId::SDONMOUSEOUT}, // StarBasic

// Attributes with context sensitive values
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_align),     HtmlOptionId::ALIGN},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_cols),      HtmlOptionId::COLS}, // Netscape 2.0 vs HTML 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_rows),      HtmlOptionId::ROWS}, // Netscape 2.0 vs HTML 2.0
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_size),      HtmlOptionId::SIZE},
    {OUStringLiteral(OOO_STRING_SVTOOLS_HTML_O_start),     HtmlOptionId::START}, // Netscape 2.0 vs IExplorer 2.0
};

HtmlOptionId GetHTMLOption( const OUString& rName )
{
    if( !bSortOptionKeyWords )
    {
        std::sort( std::begin(aHTMLOptionTab), std::end(aHTMLOptionTab), sortCompare<HtmlOptionId> );
        bSortOptionKeyWords = true;
    }

    return search( aHTMLOptionTab, rName, HtmlOptionId::UNKNOWN);
}


using HTML_ColorEntry = TokenEntry<sal_uInt32>;

// Flag: color table has already been sorted
static bool bSortColorKeyWords = false;

#define HTML_NO_COLOR 0xffffffffUL

// Color names are not exported (source:
// "http://www.uio.no/~mnbjerke/colors_w.html")
// "http://www.infi.net/wwwimages/colorindex.html" seem to be buggy.
static HTML_ColorEntry const aHTMLColorNameTab[] = {
    { OUStringLiteral("aliceblue"), 0x00f0f8ffUL },
    { OUStringLiteral("antiquewhite"), 0x00faebd7UL },
    { OUStringLiteral("aqua"), 0x0000ffffUL },
    { OUStringLiteral("aquamarine"), 0x007fffd4UL },
    { OUStringLiteral("azure"), 0x00f0ffffUL },
    { OUStringLiteral("beige"), 0x00f5f5dcUL },
    { OUStringLiteral("bisque"), 0x00ffe4c4UL },
    { OUStringLiteral("black"), 0x00000000UL },
    { OUStringLiteral("blanchedalmond"), 0x00ffebcdUL },
    { OUStringLiteral("blue"), 0x000000ffUL },
    { OUStringLiteral("blueviolet"), 0x008a2be2UL },
    { OUStringLiteral("brown"), 0x00a52a2aUL },
    { OUStringLiteral("burlywood"), 0x00deb887UL },
    { OUStringLiteral("cadetblue"), 0x005f9ea0UL },
    { OUStringLiteral("chartreuse"), 0x007fff00UL },
    { OUStringLiteral("chocolate"), 0x00d2691eUL },
    { OUStringLiteral("coral"), 0x00ff7f50UL },
    { OUStringLiteral("cornflowerblue"), 0x006495edUL },
    { OUStringLiteral("cornsilk"), 0x00fff8dcUL },
    { OUStringLiteral("crimson"), 0x00dc143cUL },
    { OUStringLiteral("cyan"), 0x0000ffffUL },
    { OUStringLiteral("darkblue"), 0x0000008bUL },
    { OUStringLiteral("darkcyan"), 0x00008b8bUL },
    { OUStringLiteral("darkgoldenrod"), 0x00b8860bUL },
    { OUStringLiteral("darkgray"), 0x00a9a9a9UL },
    { OUStringLiteral("darkgreen"), 0x00006400UL },
    { OUStringLiteral("darkkhaki"), 0x00bdb76bUL },
    { OUStringLiteral("darkmagenta"), 0x008b008bUL },
    { OUStringLiteral("darkolivegreen"), 0x00556b2fUL },
    { OUStringLiteral("darkorange"), 0x00ff8c00UL },
    { OUStringLiteral("darkorchid"), 0x009932ccUL },
    { OUStringLiteral("darkred"), 0x008b0000UL },
    { OUStringLiteral("darksalmon"), 0x00e9967aUL },
    { OUStringLiteral("darkseagreen"), 0x008fbc8fUL },
    { OUStringLiteral("darkslateblue"), 0x00483d8bUL },
    { OUStringLiteral("darkslategray"), 0x002f4f4fUL },
    { OUStringLiteral("darkturquoise"), 0x0000ced1UL },
    { OUStringLiteral("darkviolet"), 0x009400d3UL },
    { OUStringLiteral("deeppink"), 0x00ff1493UL },
    { OUStringLiteral("deepskyblue"), 0x0000bfffUL },
    { OUStringLiteral("dimgray"), 0x00696969UL },
    { OUStringLiteral("dodgerblue"), 0x001e90ffUL },
    { OUStringLiteral("firebrick"), 0x00b22222UL },
    { OUStringLiteral("floralwhite"), 0x00fffaf0UL },
    { OUStringLiteral("forestgreen"), 0x00228b22UL },
    { OUStringLiteral("fuchsia"), 0x00ff00ffUL },
    { OUStringLiteral("gainsboro"), 0x00dcdcdcUL },
    { OUStringLiteral("ghostwhite"), 0x00f8f8ffUL },
    { OUStringLiteral("gold"), 0x00ffd700UL },
    { OUStringLiteral("goldenrod"), 0x00daa520UL },
    { OUStringLiteral("gray"), 0x00808080UL },
    { OUStringLiteral("green"), 0x00008000UL },
    { OUStringLiteral("greenyellow"), 0x00adff2fUL },
    { OUStringLiteral("honeydew"), 0x00f0fff0UL },
    { OUStringLiteral("hotpink"), 0x00ff69b4UL },
    { OUStringLiteral("indianred"), 0x00cd5c5cUL },
    { OUStringLiteral("indigo"), 0x004b0082UL },
    { OUStringLiteral("ivory"), 0x00fffff0UL },
    { OUStringLiteral("khaki"), 0x00f0e68cUL },
    { OUStringLiteral("lavender"), 0x00e6e6faUL },
    { OUStringLiteral("lavenderblush"), 0x00fff0f5UL },
    { OUStringLiteral("lawngreen"), 0x007cfc00UL },
    { OUStringLiteral("lemonchiffon"), 0x00fffacdUL },
    { OUStringLiteral("lightblue"), 0x00add8e6UL },
    { OUStringLiteral("lightcoral"), 0x00f08080UL },
    { OUStringLiteral("lightcyan"), 0x00e0ffffUL },
    { OUStringLiteral("lightgoldenrodyellow"), 0x00fafad2UL },
    { OUStringLiteral("lightgreen"), 0x0090ee90UL },
    { OUStringLiteral("lightgrey"), 0x00d3d3d3UL },
    { OUStringLiteral("lightpink"), 0x00ffb6c1UL },
    { OUStringLiteral("lightsalmon"), 0x00ffa07aUL },
    { OUStringLiteral("lightseagreen"), 0x0020b2aaUL },
    { OUStringLiteral("lightskyblue"), 0x0087cefaUL },
    { OUStringLiteral("lightslategray"), 0x00778899UL },
    { OUStringLiteral("lightsteelblue"), 0x00b0c4deUL },
    { OUStringLiteral("lightyellow"), 0x00ffffe0UL },
    { OUStringLiteral("lime"), 0x0000ff00UL },
    { OUStringLiteral("limegreen"), 0x0032cd32UL },
    { OUStringLiteral("linen"), 0x00faf0e6UL },
    { OUStringLiteral("magenta"), 0x00ff00ffUL },
    { OUStringLiteral("maroon"), 0x00800000UL },
    { OUStringLiteral("mediumaquamarine"), 0x0066cdaaUL },
    { OUStringLiteral("mediumblue"), 0x000000cdUL },
    { OUStringLiteral("mediumorchid"), 0x00ba55d3UL },
    { OUStringLiteral("mediumpurple"), 0x009370dbUL },
    { OUStringLiteral("mediumseagreen"), 0x003cb371UL },
    { OUStringLiteral("mediumslateblue"), 0x007b68eeUL },
    { OUStringLiteral("mediumspringgreen"), 0x0000fa9aUL },
    { OUStringLiteral("mediumturquoise"), 0x0048d1ccUL },
    { OUStringLiteral("mediumvioletred"), 0x00c71585UL },
    { OUStringLiteral("midnightblue"), 0x00191970UL },
    { OUStringLiteral("mintcream"), 0x00f5fffaUL },
    { OUStringLiteral("mistyrose"), 0x00ffe4e1UL },
    { OUStringLiteral("moccasin"), 0x00ffe4b5UL },
    { OUStringLiteral("navajowhite"), 0x00ffdeadUL },
    { OUStringLiteral("navy"), 0x00000080UL },
    { OUStringLiteral("oldlace"), 0x00fdf5e6UL },
    { OUStringLiteral("olive"), 0x00808000UL },
    { OUStringLiteral("olivedrab"), 0x006b8e23UL },
    { OUStringLiteral("orange"), 0x00ffa500UL },
    { OUStringLiteral("orangered"), 0x00ff4500UL },
    { OUStringLiteral("orchid"), 0x00da70d6UL },
    { OUStringLiteral("palegoldenrod"), 0x00eee8aaUL },
    { OUStringLiteral("palegreen"), 0x0098fb98UL },
    { OUStringLiteral("paleturquoise"), 0x00afeeeeUL },
    { OUStringLiteral("palevioletred"), 0x00db7093UL },
    { OUStringLiteral("papayawhip"), 0x00ffefd5UL },
    { OUStringLiteral("peachpuff"), 0x00ffdab9UL },
    { OUStringLiteral("peru"), 0x00cd853fUL },
    { OUStringLiteral("pink"), 0x00ffc0cbUL },
    { OUStringLiteral("plum"), 0x00dda0ddUL },
    { OUStringLiteral("powderblue"), 0x00b0e0e6UL },
    { OUStringLiteral("purple"), 0x00800080UL },
    { OUStringLiteral("red"), 0x00ff0000UL },
    { OUStringLiteral("rosybrown"), 0x00bc8f8fUL },
    { OUStringLiteral("royalblue"), 0x004169e1UL },
    { OUStringLiteral("saddlebrown"), 0x008b4513UL },
    { OUStringLiteral("salmon"), 0x00fa8072UL },
    { OUStringLiteral("sandybrown"), 0x00f4a460UL },
    { OUStringLiteral("seagreen"), 0x002e8b57UL },
    { OUStringLiteral("seashell"), 0x00fff5eeUL },
    { OUStringLiteral("sienna"), 0x00a0522dUL },
    { OUStringLiteral("silver"), 0x00c0c0c0UL },
    { OUStringLiteral("skyblue"), 0x0087ceebUL },
    { OUStringLiteral("slateblue"), 0x006a5acdUL },
    { OUStringLiteral("slategray"), 0x00708090UL },
    { OUStringLiteral("snow"), 0x00fffafaUL },
    { OUStringLiteral("springgreen"), 0x0000ff7fUL },
    { OUStringLiteral("steelblue"), 0x004682b4UL },
    { OUStringLiteral("tan"), 0x00d2b48cUL },
    { OUStringLiteral("teal"), 0x00008080UL },
    { OUStringLiteral("thistle"), 0x00d8bfd8UL },
    { OUStringLiteral("tomato"), 0x00ff6347UL },
    { OUStringLiteral("turquoise"), 0x0040e0d0UL },
    { OUStringLiteral("violet"), 0x00ee82eeUL },
    { OUStringLiteral("wheat"), 0x00f5deb3UL },
    { OUStringLiteral("white"), 0x00ffffffUL },
    { OUStringLiteral("whitesmoke"), 0x00f5f5f5UL },
    { OUStringLiteral("yellow"), 0x00ffff00UL },
    { OUStringLiteral("yellowgreen"), 0x009acd32UL }
};

sal_uInt32 GetHTMLColor( const OUString& rName )
{
    if( !bSortColorKeyWords )
    {
        assert( std::is_sorted( std::begin(aHTMLColorNameTab), std::end(aHTMLColorNameTab),
                                sortCompare<sal_uInt32> ) );
        bSortColorKeyWords = true;
    }

    OUString aLowerCase(rName.toAsciiLowerCase());

    return search<sal_uInt32>( aHTMLColorNameTab, aLowerCase, HTML_NO_COLOR);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
