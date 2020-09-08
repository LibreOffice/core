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
#include <string_view>

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>

namespace {

template<typename T>
struct TokenEntry
{
    std::u16string_view sToken;
    T nToken;
};

}

template<typename T>
static bool sortCompare(const TokenEntry<T> & lhs, const TokenEntry<T> & rhs)
{
    return lhs.sToken < rhs.sToken;
}
template<typename T>
static bool findCompare(const TokenEntry<T> & lhs, const OUString & rhs)
{
    return lhs.sToken < rhs;
}
template<typename T, size_t LEN>
static T search(TokenEntry<T> const (&dataTable)[LEN], const OUString & key, T notFoundValue)
{
    auto findIt = std::lower_bound( std::begin(dataTable), std::end(dataTable),
                                     key, findCompare<T> );
    if (findIt != std::end(dataTable) && key == findIt->sToken)
        return findIt->nToken;
    return notFoundValue;
}

using HTML_TokenEntry = TokenEntry<HtmlTokenId>;

// this array is sorted by the name (even if it doesn't look like it from the constant names)
HTML_TokenEntry const aHTMLTokenTab[] = {
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_comment),         HtmlTokenId::COMMENT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_doctype),         HtmlTokenId::DOCTYPE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_anchor),          HtmlTokenId::ANCHOR_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_abbreviation),    HtmlTokenId::ABBREVIATION_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_acronym),         HtmlTokenId::ACRONYM_ON},   // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_address),         HtmlTokenId::ADDRESS_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_applet),          HtmlTokenId::APPLET_ON},    // HotJava
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_area),            HtmlTokenId::AREA}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_author),          HtmlTokenId::AUTHOR_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_bold),            HtmlTokenId::BOLD_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_banner),          HtmlTokenId::BANNER_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_base),            HtmlTokenId::BASE}, // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_basefont),        HtmlTokenId::BASEFONT_ON},  // Netscape
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_bigprint),        HtmlTokenId::BIGPRINT_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_blink),           HtmlTokenId::BLINK_ON}, // Netscape
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_blockquote),      HtmlTokenId::BLOCKQUOTE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_body),            HtmlTokenId::BODY_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_blockquote30),    HtmlTokenId::BLOCKQUOTE30_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_linebreak),       HtmlTokenId::LINEBREAK},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_caption),         HtmlTokenId::CAPTION_ON},   // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_center),          HtmlTokenId::CENTER_ON},    // Netscape
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_citiation),       HtmlTokenId::CITIATION_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_code),            HtmlTokenId::CODE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_col),             HtmlTokenId::COL_ON}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_colgroup),        HtmlTokenId::COLGROUP_ON}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_comment2),        HtmlTokenId::COMMENT2_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_credit),          HtmlTokenId::CREDIT_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_dd),              HtmlTokenId::DD_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_deletedtext),     HtmlTokenId::DELETEDTEXT_ON},   // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_definstance),     HtmlTokenId::DEFINSTANCE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_dirlist),         HtmlTokenId::DIRLIST_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_division),        HtmlTokenId::DIVISION_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_deflist),         HtmlTokenId::DEFLIST_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_dt),              HtmlTokenId::DT_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_emphasis),        HtmlTokenId::EMPHASIS_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_embed),           HtmlTokenId::EMBED},    // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_figure),          HtmlTokenId::FIGURE_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_footnote),        HtmlTokenId::FOOTNOTE_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_font),            HtmlTokenId::FONT_ON}, // Netscape
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_form),            HtmlTokenId::FORM_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_frame),           HtmlTokenId::FRAME_ON}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_frameset),        HtmlTokenId::FRAMESET_ON},  // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_head1),           HtmlTokenId::HEAD1_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_head2),           HtmlTokenId::HEAD2_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_head3),           HtmlTokenId::HEAD3_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_head4),           HtmlTokenId::HEAD4_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_head5),           HtmlTokenId::HEAD5_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_head6),           HtmlTokenId::HEAD6_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_head),            HtmlTokenId::HEAD_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_horzrule),        HtmlTokenId::HORZRULE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_html),            HtmlTokenId::HTML_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_italic),          HtmlTokenId::ITALIC_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_iframe),          HtmlTokenId::IFRAME_ON},    // IE 3.0b2
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_image),           HtmlTokenId::IMAGE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_input),           HtmlTokenId::INPUT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_insertedtext),    HtmlTokenId::INSERTEDTEXT_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_keyboard),        HtmlTokenId::KEYBOARD_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_language),        HtmlTokenId::LANGUAGE_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_listheader),      HtmlTokenId::LISTHEADER_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_li),              HtmlTokenId::LI_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_link),            HtmlTokenId::LINK}, // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_listing),         HtmlTokenId::LISTING_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_map),             HtmlTokenId::MAP_ON},   // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_marquee),         HtmlTokenId::MARQUEE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_menulist),        HtmlTokenId::MENULIST_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_meta),            HtmlTokenId::META}, // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_multicol),        HtmlTokenId::MULTICOL_ON},  // Netscape 3.0b5
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_nobr),            HtmlTokenId::NOBR_ON},  // Netscape
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_noembed),         HtmlTokenId::NOEMBED_ON},   // Netscape 2.0 ???
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_noframe),         HtmlTokenId::NOFRAMES_ON},  // Netscape 2.0 ???
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_noframes),        HtmlTokenId::NOFRAMES_ON},  // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_noscript),        HtmlTokenId::NOSCRIPT_ON},  // Netscape 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_note),            HtmlTokenId::NOTE_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_object),          HtmlTokenId::OBJECT_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_orderlist),       HtmlTokenId::ORDERLIST_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_option),          HtmlTokenId::OPTION},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_parabreak),       HtmlTokenId::PARABREAK_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_param),           HtmlTokenId::PARAM},    // HotJava
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_person),          HtmlTokenId::PERSON_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_plaintext2),      HtmlTokenId::PLAINTEXT2_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_preformtxt),      HtmlTokenId::PREFORMTXT_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_shortquote),      HtmlTokenId::SHORTQUOTE_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_strikethrough),   HtmlTokenId::STRIKETHROUGH_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_sample),          HtmlTokenId::SAMPLE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_script),          HtmlTokenId::SCRIPT_ON}, // HTML 3.2
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_sdfield),         HtmlTokenId::SDFIELD_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_select),          HtmlTokenId::SELECT_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_smallprint),      HtmlTokenId::SMALLPRINT_ON},    // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_spacer),          HtmlTokenId::SPACER},   // Netscape 3.0b5
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_span),            HtmlTokenId::SPAN_ON},  // Style Sheets
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_strike),          HtmlTokenId::STRIKE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_strong),          HtmlTokenId::STRONG_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_style),           HtmlTokenId::STYLE_ON}, // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_subscript),       HtmlTokenId::SUBSCRIPT_ON}, // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_superscript),     HtmlTokenId::SUPERSCRIPT_ON},   // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_plaintext),       HtmlTokenId::PLAINTEXT_ON}, // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_table),           HtmlTokenId::TABLE_ON}, // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_tbody),           HtmlTokenId::TBODY_ON}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_tabledata),       HtmlTokenId::TABLEDATA_ON}, // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_textarea),        HtmlTokenId::TEXTAREA_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_tfoot),           HtmlTokenId::TFOOT_ON}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_tableheader),     HtmlTokenId::TABLEHEADER_ON},   // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_thead),           HtmlTokenId::THEAD_ON}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_title),           HtmlTokenId::TITLE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_tablerow),        HtmlTokenId::TABLEROW_ON},  // HTML 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_teletype),        HtmlTokenId::TELETYPE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_underline),       HtmlTokenId::UNDERLINE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_unorderlist),     HtmlTokenId::UNORDERLIST_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_variable),        HtmlTokenId::VARIABLE_ON},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_xmp),             HtmlTokenId::XMP_ON},
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
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_lt),             60},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_gt),             62},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_amp),        38},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_quot),       34},

    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Agrave),        192},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Aacute),        193},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Acirc),     194},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Atilde),        195},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Auml),      196},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Aring),     197},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_AElig),     198},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Ccedil),        199},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Egrave),        200},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Eacute),        201},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Ecirc),     202},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Euml),      203},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Igrave),        204},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Iacute),        205},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Icirc),     206},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Iuml),      207},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_ETH),       208},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Ntilde),        209},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Ograve),        210},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Oacute),        211},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Ocirc),     212},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Otilde),        213},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Ouml),      214},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Oslash),        216},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Ugrave),        217},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Uacute),        218},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Ucirc),     219},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Uuml),      220},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_Yacute),        221},

    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_THORN),     222},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_C_szlig),     223},

    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_agrave),        224},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_aacute),        225},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_acirc),     226},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_atilde),        227},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_auml),      228},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_aring),     229},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_aelig),     230},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ccedil),        231},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_egrave),        232},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_eacute),        233},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ecirc),     234},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_euml),      235},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_igrave),        236},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_iacute),        237},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_icirc),     238},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_iuml),      239},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_eth),       240},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ntilde),        241},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ograve),        242},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_oacute),        243},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ocirc),     244},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_otilde),        245},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ouml),      246},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_oslash),        248},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ugrave),        249},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_uacute),        250},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ucirc),     251},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_uuml),      252},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_yacute),        253},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_thorn),     254},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_yuml),      255},

// special characters
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_acute),     180},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_brvbar),    166},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_cedil),      184},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_cent),       162},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_copy),      169},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_curren),     164},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_deg),        176},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_divide),     247},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_frac12),     189},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_frac14),     188},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_frac34),     190},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_iexcl),      161},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_iquest),     191},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_laquo),      171},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_macr),       175},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_micro),      181},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_middot),     183},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_not),        172},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ordf),       170},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ordm),       186},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_para),       182},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_plusmn),     177},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_pound),      163},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_raquo),      187},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_reg),       174},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sect),       167},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sup1),       185},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sup2),       178},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sup3),       179},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_times),      215},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_uml),        168},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_yen),        165},

// special characters), which will be converted to tokens !!!
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_nbsp),      1},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_shy),       2},


// HTML4
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_OElig),     338},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_oelig),     339},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Scaron),        352},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_scaron),        353},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Yuml),      376},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_fnof),      402},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_circ),      710},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_tilde),     732},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Alpha),     913},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Beta),      914},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Gamma),     915},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Delta),     916},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Epsilon),   917},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Zeta),      918},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Eta),       919},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Theta),     920},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Iota),      921},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Kappa),     922},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Lambda),        923},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Mu),            924},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Nu),            925},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Xi),            926},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Omicron),   927},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Pi),            928},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Rho),       929},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Sigma),     931},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Tau),       932},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Upsilon),   933},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Phi),       934},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Chi),       935},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Psi),       936},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Omega),     937},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_alpha),     945},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_beta),      946},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_gamma),     947},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_delta),     948},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_epsilon),   949},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_zeta),      950},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_eta),       951},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_theta),     952},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_iota),      953},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_kappa),     954},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lambda),        955},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_mu),            956},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_nu),            957},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_xi),            958},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_omicron),   959},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_pi),            960},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rho),       961},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sigmaf),        962},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sigma),     963},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_tau),       964},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_upsilon),   965},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_phi),       966},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_chi),       967},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_psi),       968},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_omega),     969},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_thetasym),  977},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_upsih),     978},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_piv),       982},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ensp),      8194},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_emsp),      8195},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_thinsp),        8201},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_zwnj),      8204},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_zwj),       8205},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lrm),       8206},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rlm),       8207},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ndash),     8211},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_mdash),     8212},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lsquo),     8216},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rsquo),     8217},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sbquo),     8218},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ldquo),     8220},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rdquo),     8221},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_bdquo),     8222},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_dagger),        8224},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Dagger),        8225},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_bull),      8226},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_hellip),        8230},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_permil),        8240},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_prime),     8242},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_Prime),     8243},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lsaquo),        8249},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rsaquo),        8250},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_oline),     8254},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_frasl),     8260},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_euro),      8364},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_image),     8465},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_weierp),        8472},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_real),      8476},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_trade),     8482},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_alefsym),   8501},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_larr),      8592},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_uarr),      8593},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rarr),      8594},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_darr),      8595},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_harr),      8596},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_crarr),     8629},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lArr),      8656},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_uArr),      8657},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rArr),      8658},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_dArr),      8659},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_hArr),      8660},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_forall),        8704},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_part),      8706},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_exist),     8707},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_empty),     8709},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_nabla),     8711},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_isin),      8712},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_notin),     8713},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ni),            8715},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_prod),      8719},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sum),       8721},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_minus),     8722},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lowast),        8727},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_radic),     8730},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_prop),      8733},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_infin),     8734},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ang),       8736},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_and),       8743},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_or),            8744},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_cap),       8745},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_cup),       8746},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_int),       8747},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_there4),        8756},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sim),       8764},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_cong),      8773},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_asymp),     8776},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ne),            8800},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_equiv),     8801},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_le),            8804},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_ge),            8805},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sub),       8834},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sup),       8835},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_nsub),      8836},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sube),      8838},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_supe),      8839},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_oplus),     8853},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_otimes),        8855},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_perp),      8869},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_sdot),      8901},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lceil),     8968},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rceil),     8969},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lfloor),        8970},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rfloor),        8971},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_lang),      9001},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_rang),      9002},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_loz),       9674},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_spades),        9824},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_clubs),     9827},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_hearts),        9829},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_S_diams),     9830}
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
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_checked),   HtmlOptionId::CHECKED},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_compact),   HtmlOptionId::COMPACT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_declare),   HtmlOptionId::DECLARE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_disabled),  HtmlOptionId::DISABLED},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_ismap),     HtmlOptionId::ISMAP},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_mayscript), HtmlOptionId::MAYSCRIPT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_multiple),  HtmlOptionId::MULTIPLE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_nohref),        HtmlOptionId::NOHREF}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_noresize),  HtmlOptionId::NORESIZE}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_noshade),   HtmlOptionId::NOSHADE}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_nowrap),        HtmlOptionId::NOWRAP},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_sdfixed),   HtmlOptionId::SDFIXED},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_selected),      HtmlOptionId::SELECTED},

// Attributes with a string value
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_accept),        HtmlOptionId::ACCEPT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_accesskey), HtmlOptionId::ACCESSKEY},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_alt),       HtmlOptionId::ALT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_axis),      HtmlOptionId::AXIS},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_char),      HtmlOptionId::CHAR}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_charset),   HtmlOptionId::CHARSET},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_class),     HtmlOptionId::CLASS},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_code),      HtmlOptionId::CODE}, // HotJava
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_codetype),  HtmlOptionId::CODETYPE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_content),   HtmlOptionId::CONTENT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_coords),        HtmlOptionId::COORDS}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_enctype),   HtmlOptionId::ENCTYPE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_face),      HtmlOptionId::FACE}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_frameborder), HtmlOptionId::FRAMEBORDER}, // IExplorer 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_httpequiv), HtmlOptionId::HTTPEQUIV},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_language),  HtmlOptionId::LANGUAGE}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_name),      HtmlOptionId::NAME},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_prompt),        HtmlOptionId::PROMPT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_shape),     HtmlOptionId::SHAPE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_standby),   HtmlOptionId::STANDBY},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_style),     HtmlOptionId::STYLE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_title),     HtmlOptionId::TITLE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_value),     HtmlOptionId::VALUE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDval),     HtmlOptionId::SDVAL}, // StarDiv NumberValue
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDnum),     HtmlOptionId::SDNUM}, // StarDiv NumberFormat
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_sdlibrary), HtmlOptionId::SDLIBRARY},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_sdmodule),  HtmlOptionId::SDMODULE},

// Attributes with a SGML identifier value
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_id),            HtmlOptionId::ID},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_target),        HtmlOptionId::TARGET}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_to),            HtmlOptionId::TO},

// Attributes with a URI value
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_action),        HtmlOptionId::ACTION},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_archive),       HtmlOptionId::ARCHIVE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_background),    HtmlOptionId::BACKGROUND},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_classid),   HtmlOptionId::CLASSID},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_codebase),  HtmlOptionId::CODEBASE}, // HotJava
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_data),      HtmlOptionId::DATA},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_href),      HtmlOptionId::HREF},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_script),        HtmlOptionId::SCRIPT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_src),       HtmlOptionId::SRC},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_usemap),        HtmlOptionId::USEMAP}, // Netscape 2.0

// Attributes with a color value (all Netscape versions)
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_alink),     HtmlOptionId::ALINK},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_bgcolor),   HtmlOptionId::BGCOLOR},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_bordercolor), HtmlOptionId::BORDERCOLOR}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_bordercolorlight), HtmlOptionId::BORDERCOLORLIGHT}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_bordercolordark), HtmlOptionId::BORDERCOLORDARK}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_color),     HtmlOptionId::COLOR},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_link),      HtmlOptionId::LINK},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_text),      HtmlOptionId::TEXT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_vlink),     HtmlOptionId::VLINK},

// Attributes with a numerical value
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_border),        HtmlOptionId::BORDER},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_cellspacing),HtmlOptionId::CELLSPACING}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_cellpadding),HtmlOptionId::CELLPADDING}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_charoff),   HtmlOptionId::CHAROFF}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_colspan),   HtmlOptionId::COLSPAN},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_framespacing), HtmlOptionId::FRAMESPACING}, // IExplorer 3.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_gutter),        HtmlOptionId::GUTTER}, // Netscape 3.0b5
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_height),        HtmlOptionId::HEIGHT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_hspace),        HtmlOptionId::HSPACE}, // Netscape
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_left),      HtmlOptionId::LEFT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_loop),      HtmlOptionId::LOOP}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_marginheight),HtmlOptionId::MARGINHEIGHT}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_marginwidth),HtmlOptionId::MARGINWIDTH}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_maxlength), HtmlOptionId::MAXLENGTH},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_rowspan),   HtmlOptionId::ROWSPAN},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_scrollamount), HtmlOptionId::SCROLLAMOUNT}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_scrolldelay), HtmlOptionId::SCROLLDELAY}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_span),      HtmlOptionId::SPAN}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_tabindex),  HtmlOptionId::TABINDEX},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_vspace),        HtmlOptionId::VSPACE}, // Netscape
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_width),     HtmlOptionId::WIDTH},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_zindex),        HtmlOptionId::ZINDEX},

// Attributes with enum values
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_behavior),  HtmlOptionId::BEHAVIOR}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_clear),     HtmlOptionId::CLEAR},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_dir),       HtmlOptionId::DIR}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_direction),     HtmlOptionId::DIRECTION}, // IExplorer 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_format),        HtmlOptionId::FORMAT},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_frame),     HtmlOptionId::FRAME}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_lang),      HtmlOptionId::LANG},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_method),        HtmlOptionId::METHOD},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_rel),       HtmlOptionId::REL},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_rev),       HtmlOptionId::REV},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_rules),     HtmlOptionId::RULES}, // HTML 3 Table Model Draft
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_scrolling), HtmlOptionId::SCROLLING}, // Netscape 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_sdreadonly),    HtmlOptionId::SDREADONLY},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_subtype),   HtmlOptionId::SUBTYPE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_type),      HtmlOptionId::TYPE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_valign),        HtmlOptionId::VALIGN},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_valuetype), HtmlOptionId::VALUETYPE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_wrap),      HtmlOptionId::WRAP},

// Attributes with script code value
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onblur),        HtmlOptionId::ONBLUR}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onchange),  HtmlOptionId::ONCHANGE}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onclick),   HtmlOptionId::ONCLICK}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onfocus),   HtmlOptionId::ONFOCUS}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onload),    HtmlOptionId::ONLOAD}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onmouseover), HtmlOptionId::ONMOUSEOVER}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onreset),   HtmlOptionId::ONRESET}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onselect),  HtmlOptionId::ONSELECT}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onsubmit),  HtmlOptionId::ONSUBMIT}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onunload),  HtmlOptionId::ONUNLOAD}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onabort),   HtmlOptionId::ONABORT}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onerror),   HtmlOptionId::ONERROR}, // JavaScript
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_onmouseout),    HtmlOptionId::ONMOUSEOUT}, // JavaScript

    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonblur),      HtmlOptionId::SDONBLUR}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonchange),    HtmlOptionId::SDONCHANGE}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonclick),         HtmlOptionId::SDONCLICK}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonfocus),         HtmlOptionId::SDONFOCUS}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonload),      HtmlOptionId::SDONLOAD}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonmouseover),     HtmlOptionId::SDONMOUSEOVER}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonreset),     HtmlOptionId::SDONRESET}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonselect),        HtmlOptionId::SDONSELECT}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonsubmit),        HtmlOptionId::SDONSUBMIT}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonunload),        HtmlOptionId::SDONUNLOAD}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonabort),     HtmlOptionId::SDONABORT}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonerror),         HtmlOptionId::SDONERROR}, // StarBasic
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_SDonmouseout),  HtmlOptionId::SDONMOUSEOUT}, // StarBasic

// Attributes with context sensitive values
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_align),     HtmlOptionId::ALIGN},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_cols),      HtmlOptionId::COLS}, // Netscape 2.0 vs HTML 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_rows),      HtmlOptionId::ROWS}, // Netscape 2.0 vs HTML 2.0
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_size),      HtmlOptionId::SIZE},
    {std::u16string_view(u"" OOO_STRING_SVTOOLS_HTML_O_start),     HtmlOptionId::START}, // Netscape 2.0 vs IExplorer 2.0
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
HTML_ColorEntry const aHTMLColorNameTab[] = {
    { std::u16string_view(u"aliceblue"), 0x00f0f8ffUL },
    { std::u16string_view(u"antiquewhite"), 0x00faebd7UL },
    { std::u16string_view(u"aqua"), 0x0000ffffUL },
    { std::u16string_view(u"aquamarine"), 0x007fffd4UL },
    { std::u16string_view(u"azure"), 0x00f0ffffUL },
    { std::u16string_view(u"beige"), 0x00f5f5dcUL },
    { std::u16string_view(u"bisque"), 0x00ffe4c4UL },
    { std::u16string_view(u"black"), 0x00000000UL },
    { std::u16string_view(u"blanchedalmond"), 0x00ffebcdUL },
    { std::u16string_view(u"blue"), 0x000000ffUL },
    { std::u16string_view(u"blueviolet"), 0x008a2be2UL },
    { std::u16string_view(u"brown"), 0x00a52a2aUL },
    { std::u16string_view(u"burlywood"), 0x00deb887UL },
    { std::u16string_view(u"cadetblue"), 0x005f9ea0UL },
    { std::u16string_view(u"chartreuse"), 0x007fff00UL },
    { std::u16string_view(u"chocolate"), 0x00d2691eUL },
    { std::u16string_view(u"coral"), 0x00ff7f50UL },
    { std::u16string_view(u"cornflowerblue"), 0x006495edUL },
    { std::u16string_view(u"cornsilk"), 0x00fff8dcUL },
    { std::u16string_view(u"crimson"), 0x00dc143cUL },
    { std::u16string_view(u"cyan"), 0x0000ffffUL },
    { std::u16string_view(u"darkblue"), 0x0000008bUL },
    { std::u16string_view(u"darkcyan"), 0x00008b8bUL },
    { std::u16string_view(u"darkgoldenrod"), 0x00b8860bUL },
    { std::u16string_view(u"darkgray"), 0x00a9a9a9UL },
    { std::u16string_view(u"darkgreen"), 0x00006400UL },
    { std::u16string_view(u"darkkhaki"), 0x00bdb76bUL },
    { std::u16string_view(u"darkmagenta"), 0x008b008bUL },
    { std::u16string_view(u"darkolivegreen"), 0x00556b2fUL },
    { std::u16string_view(u"darkorange"), 0x00ff8c00UL },
    { std::u16string_view(u"darkorchid"), 0x009932ccUL },
    { std::u16string_view(u"darkred"), 0x008b0000UL },
    { std::u16string_view(u"darksalmon"), 0x00e9967aUL },
    { std::u16string_view(u"darkseagreen"), 0x008fbc8fUL },
    { std::u16string_view(u"darkslateblue"), 0x00483d8bUL },
    { std::u16string_view(u"darkslategray"), 0x002f4f4fUL },
    { std::u16string_view(u"darkturquoise"), 0x0000ced1UL },
    { std::u16string_view(u"darkviolet"), 0x009400d3UL },
    { std::u16string_view(u"deeppink"), 0x00ff1493UL },
    { std::u16string_view(u"deepskyblue"), 0x0000bfffUL },
    { std::u16string_view(u"dimgray"), 0x00696969UL },
    { std::u16string_view(u"dodgerblue"), 0x001e90ffUL },
    { std::u16string_view(u"firebrick"), 0x00b22222UL },
    { std::u16string_view(u"floralwhite"), 0x00fffaf0UL },
    { std::u16string_view(u"forestgreen"), 0x00228b22UL },
    { std::u16string_view(u"fuchsia"), 0x00ff00ffUL },
    { std::u16string_view(u"gainsboro"), 0x00dcdcdcUL },
    { std::u16string_view(u"ghostwhite"), 0x00f8f8ffUL },
    { std::u16string_view(u"gold"), 0x00ffd700UL },
    { std::u16string_view(u"goldenrod"), 0x00daa520UL },
    { std::u16string_view(u"gray"), 0x00808080UL },
    { std::u16string_view(u"green"), 0x00008000UL },
    { std::u16string_view(u"greenyellow"), 0x00adff2fUL },
    { std::u16string_view(u"honeydew"), 0x00f0fff0UL },
    { std::u16string_view(u"hotpink"), 0x00ff69b4UL },
    { std::u16string_view(u"indianred"), 0x00cd5c5cUL },
    { std::u16string_view(u"indigo"), 0x004b0082UL },
    { std::u16string_view(u"ivory"), 0x00fffff0UL },
    { std::u16string_view(u"khaki"), 0x00f0e68cUL },
    { std::u16string_view(u"lavender"), 0x00e6e6faUL },
    { std::u16string_view(u"lavenderblush"), 0x00fff0f5UL },
    { std::u16string_view(u"lawngreen"), 0x007cfc00UL },
    { std::u16string_view(u"lemonchiffon"), 0x00fffacdUL },
    { std::u16string_view(u"lightblue"), 0x00add8e6UL },
    { std::u16string_view(u"lightcoral"), 0x00f08080UL },
    { std::u16string_view(u"lightcyan"), 0x00e0ffffUL },
    { std::u16string_view(u"lightgoldenrodyellow"), 0x00fafad2UL },
    { std::u16string_view(u"lightgreen"), 0x0090ee90UL },
    { std::u16string_view(u"lightgrey"), 0x00d3d3d3UL },
    { std::u16string_view(u"lightpink"), 0x00ffb6c1UL },
    { std::u16string_view(u"lightsalmon"), 0x00ffa07aUL },
    { std::u16string_view(u"lightseagreen"), 0x0020b2aaUL },
    { std::u16string_view(u"lightskyblue"), 0x0087cefaUL },
    { std::u16string_view(u"lightslategray"), 0x00778899UL },
    { std::u16string_view(u"lightsteelblue"), 0x00b0c4deUL },
    { std::u16string_view(u"lightyellow"), 0x00ffffe0UL },
    { std::u16string_view(u"lime"), 0x0000ff00UL },
    { std::u16string_view(u"limegreen"), 0x0032cd32UL },
    { std::u16string_view(u"linen"), 0x00faf0e6UL },
    { std::u16string_view(u"magenta"), 0x00ff00ffUL },
    { std::u16string_view(u"maroon"), 0x00800000UL },
    { std::u16string_view(u"mediumaquamarine"), 0x0066cdaaUL },
    { std::u16string_view(u"mediumblue"), 0x000000cdUL },
    { std::u16string_view(u"mediumorchid"), 0x00ba55d3UL },
    { std::u16string_view(u"mediumpurple"), 0x009370dbUL },
    { std::u16string_view(u"mediumseagreen"), 0x003cb371UL },
    { std::u16string_view(u"mediumslateblue"), 0x007b68eeUL },
    { std::u16string_view(u"mediumspringgreen"), 0x0000fa9aUL },
    { std::u16string_view(u"mediumturquoise"), 0x0048d1ccUL },
    { std::u16string_view(u"mediumvioletred"), 0x00c71585UL },
    { std::u16string_view(u"midnightblue"), 0x00191970UL },
    { std::u16string_view(u"mintcream"), 0x00f5fffaUL },
    { std::u16string_view(u"mistyrose"), 0x00ffe4e1UL },
    { std::u16string_view(u"moccasin"), 0x00ffe4b5UL },
    { std::u16string_view(u"navajowhite"), 0x00ffdeadUL },
    { std::u16string_view(u"navy"), 0x00000080UL },
    { std::u16string_view(u"oldlace"), 0x00fdf5e6UL },
    { std::u16string_view(u"olive"), 0x00808000UL },
    { std::u16string_view(u"olivedrab"), 0x006b8e23UL },
    { std::u16string_view(u"orange"), 0x00ffa500UL },
    { std::u16string_view(u"orangered"), 0x00ff4500UL },
    { std::u16string_view(u"orchid"), 0x00da70d6UL },
    { std::u16string_view(u"palegoldenrod"), 0x00eee8aaUL },
    { std::u16string_view(u"palegreen"), 0x0098fb98UL },
    { std::u16string_view(u"paleturquoise"), 0x00afeeeeUL },
    { std::u16string_view(u"palevioletred"), 0x00db7093UL },
    { std::u16string_view(u"papayawhip"), 0x00ffefd5UL },
    { std::u16string_view(u"peachpuff"), 0x00ffdab9UL },
    { std::u16string_view(u"peru"), 0x00cd853fUL },
    { std::u16string_view(u"pink"), 0x00ffc0cbUL },
    { std::u16string_view(u"plum"), 0x00dda0ddUL },
    { std::u16string_view(u"powderblue"), 0x00b0e0e6UL },
    { std::u16string_view(u"purple"), 0x00800080UL },
    { std::u16string_view(u"red"), 0x00ff0000UL },
    { std::u16string_view(u"rosybrown"), 0x00bc8f8fUL },
    { std::u16string_view(u"royalblue"), 0x004169e1UL },
    { std::u16string_view(u"saddlebrown"), 0x008b4513UL },
    { std::u16string_view(u"salmon"), 0x00fa8072UL },
    { std::u16string_view(u"sandybrown"), 0x00f4a460UL },
    { std::u16string_view(u"seagreen"), 0x002e8b57UL },
    { std::u16string_view(u"seashell"), 0x00fff5eeUL },
    { std::u16string_view(u"sienna"), 0x00a0522dUL },
    { std::u16string_view(u"silver"), 0x00c0c0c0UL },
    { std::u16string_view(u"skyblue"), 0x0087ceebUL },
    { std::u16string_view(u"slateblue"), 0x006a5acdUL },
    { std::u16string_view(u"slategray"), 0x00708090UL },
    { std::u16string_view(u"snow"), 0x00fffafaUL },
    { std::u16string_view(u"springgreen"), 0x0000ff7fUL },
    { std::u16string_view(u"steelblue"), 0x004682b4UL },
    { std::u16string_view(u"tan"), 0x00d2b48cUL },
    { std::u16string_view(u"teal"), 0x00008080UL },
    { std::u16string_view(u"thistle"), 0x00d8bfd8UL },
    { std::u16string_view(u"tomato"), 0x00ff6347UL },
    { std::u16string_view(u"turquoise"), 0x0040e0d0UL },
    { std::u16string_view(u"violet"), 0x00ee82eeUL },
    { std::u16string_view(u"wheat"), 0x00f5deb3UL },
    { std::u16string_view(u"white"), 0x00ffffffUL },
    { std::u16string_view(u"whitesmoke"), 0x00f5f5f5UL },
    { std::u16string_view(u"yellow"), 0x00ffff00UL },
    { std::u16string_view(u"yellowgreen"), 0x009acd32UL }
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
