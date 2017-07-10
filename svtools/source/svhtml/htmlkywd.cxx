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


#include <limits.h>
#include <sal/types.h>
#include <stdlib.h>
#include <string.h>
#include <rtl/ustring.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>

// Table has still to be sorted
struct HTML_TokenEntry
{
    union
    {
        const sal_Char *sToken;
        const OUString *pUToken;
    };
    HtmlTokenId nToken;
};

// Flag: RTF token table has already been sorted
static bool bSortKeyWords = false;

static HTML_TokenEntry aHTMLTokenTab[] = {
    {{OOO_STRING_SVTOOLS_HTML_area},            HtmlTokenId::AREA}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_base},            HtmlTokenId::BASE}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_comment},     HtmlTokenId::COMMENT},
    {{OOO_STRING_SVTOOLS_HTML_doctype},      HtmlTokenId::DOCTYPE},
    {{OOO_STRING_SVTOOLS_HTML_embed},       HtmlTokenId::EMBED},    // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_horzrule},        HtmlTokenId::HORZRULE},
    {{OOO_STRING_SVTOOLS_HTML_image},          HtmlTokenId::IMAGE},
    {{OOO_STRING_SVTOOLS_HTML_input},          HtmlTokenId::INPUT},
    {{OOO_STRING_SVTOOLS_HTML_linebreak},      HtmlTokenId::LINEBREAK},
    {{OOO_STRING_SVTOOLS_HTML_link},            HtmlTokenId::LINK}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_meta},            HtmlTokenId::META}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_option},        HtmlTokenId::OPTION},
    {{OOO_STRING_SVTOOLS_HTML_param},       HtmlTokenId::PARAM},    // HotJava
    {{OOO_STRING_SVTOOLS_HTML_spacer},      HtmlTokenId::SPACER},   // Netscape 3.0b5

    {{OOO_STRING_SVTOOLS_HTML_abbreviation},    HtmlTokenId::ABBREVIATION_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_acronym},     HtmlTokenId::ACRONYM_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_address},      HtmlTokenId::ADDRESS_ON},
    {{OOO_STRING_SVTOOLS_HTML_anchor},        HtmlTokenId::ANCHOR_ON},
    {{OOO_STRING_SVTOOLS_HTML_applet},      HtmlTokenId::APPLET_ON},    // HotJava
    {{OOO_STRING_SVTOOLS_HTML_author},      HtmlTokenId::AUTHOR_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_banner},      HtmlTokenId::BANNER_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_basefont},        HtmlTokenId::BASEFONT_ON},  // Netscape
    {{OOO_STRING_SVTOOLS_HTML_bigprint},        HtmlTokenId::BIGPRINT_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_blink},       HtmlTokenId::BLINK_ON}, // Netscape
    {{OOO_STRING_SVTOOLS_HTML_blockquote},    HtmlTokenId::BLOCKQUOTE_ON},
    {{OOO_STRING_SVTOOLS_HTML_blockquote30},    HtmlTokenId::BLOCKQUOTE30_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_body},            HtmlTokenId::BODY_ON},
    {{OOO_STRING_SVTOOLS_HTML_bold},            HtmlTokenId::BOLD_ON},
    {{OOO_STRING_SVTOOLS_HTML_caption},     HtmlTokenId::CAPTION_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_center},      HtmlTokenId::CENTER_ON},    // Netscape
    {{OOO_STRING_SVTOOLS_HTML_citiation},      HtmlTokenId::CITIATION_ON},
    {{OOO_STRING_SVTOOLS_HTML_col},             HtmlTokenId::COL_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_colgroup},        HtmlTokenId::COLGROUP_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_code},            HtmlTokenId::CODE_ON},
    {{OOO_STRING_SVTOOLS_HTML_credit},      HtmlTokenId::CREDIT_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_dd},            HtmlTokenId::DD_ON},
    {{OOO_STRING_SVTOOLS_HTML_deflist},      HtmlTokenId::DEFLIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_deletedtext}, HtmlTokenId::DELETEDTEXT_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_dirlist},      HtmlTokenId::DIRLIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_division},        HtmlTokenId::DIVISION_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_dt},            HtmlTokenId::DT_ON},
    {{OOO_STRING_SVTOOLS_HTML_emphasis},        HtmlTokenId::EMPHASIS_ON},
    {{OOO_STRING_SVTOOLS_HTML_figure},      HtmlTokenId::FIGURE_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_font},            HtmlTokenId::FONT_ON}, // Netscape
    {{OOO_STRING_SVTOOLS_HTML_footnote},        HtmlTokenId::FOOTNOTE_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_form},            HtmlTokenId::FORM_ON},
    {{OOO_STRING_SVTOOLS_HTML_frame},       HtmlTokenId::FRAME_ON}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_frameset},        HtmlTokenId::FRAMESET_ON},  // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_head},            HtmlTokenId::HEAD_ON},
    {{OOO_STRING_SVTOOLS_HTML_head1},          HtmlTokenId::HEAD1_ON},
    {{OOO_STRING_SVTOOLS_HTML_head2},          HtmlTokenId::HEAD2_ON},
    {{OOO_STRING_SVTOOLS_HTML_head3},          HtmlTokenId::HEAD3_ON},
    {{OOO_STRING_SVTOOLS_HTML_head4},          HtmlTokenId::HEAD4_ON},
    {{OOO_STRING_SVTOOLS_HTML_head5},          HtmlTokenId::HEAD5_ON},
    {{OOO_STRING_SVTOOLS_HTML_head6},          HtmlTokenId::HEAD6_ON},
    {{OOO_STRING_SVTOOLS_HTML_html},            HtmlTokenId::HTML_ON},
    {{OOO_STRING_SVTOOLS_HTML_iframe},      HtmlTokenId::IFRAME_ON},    // IE 3.0b2
    {{OOO_STRING_SVTOOLS_HTML_insertedtext},    HtmlTokenId::INSERTEDTEXT_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_italic},        HtmlTokenId::ITALIC_ON},
    {{OOO_STRING_SVTOOLS_HTML_keyboard},        HtmlTokenId::KEYBOARD_ON},
    {{OOO_STRING_SVTOOLS_HTML_language},        HtmlTokenId::LANGUAGE_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_li},            HtmlTokenId::LI_ON},
    {{OOO_STRING_SVTOOLS_HTML_listheader},  HtmlTokenId::LISTHEADER_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_map},         HtmlTokenId::MAP_ON},   // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_menulist},        HtmlTokenId::MENULIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_multicol},        HtmlTokenId::MULTICOL_ON},  // Netscape 3.0b5
    {{OOO_STRING_SVTOOLS_HTML_nobr},            HtmlTokenId::NOBR_ON},  // Netscape
    {{OOO_STRING_SVTOOLS_HTML_noembed},     HtmlTokenId::NOEMBED_ON},   // Netscape 2.0 ???
    {{OOO_STRING_SVTOOLS_HTML_noframe},     HtmlTokenId::NOFRAMES_ON},  // Netscape 2.0 ???
    {{OOO_STRING_SVTOOLS_HTML_noframes},        HtmlTokenId::NOFRAMES_ON},  // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_noscript},        HtmlTokenId::NOSCRIPT_ON},  // Netscape 3.0
    {{OOO_STRING_SVTOOLS_HTML_note},            HtmlTokenId::NOTE_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_object},     HtmlTokenId::OBJECT_ON},
    {{OOO_STRING_SVTOOLS_HTML_orderlist},      HtmlTokenId::ORDERLIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_parabreak},      HtmlTokenId::PARABREAK_ON},
    {{OOO_STRING_SVTOOLS_HTML_person},      HtmlTokenId::PERSON_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_plaintext},   HtmlTokenId::PLAINTEXT_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_preformtxt},    HtmlTokenId::PREFORMTXT_ON},
    {{OOO_STRING_SVTOOLS_HTML_sample},        HtmlTokenId::SAMPLE_ON},
    {{OOO_STRING_SVTOOLS_HTML_script},        HtmlTokenId::SCRIPT_ON}, // HTML 3.2
    {{OOO_STRING_SVTOOLS_HTML_select},        HtmlTokenId::SELECT_ON},
    {{OOO_STRING_SVTOOLS_HTML_shortquote},  HtmlTokenId::SHORTQUOTE_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_smallprint},  HtmlTokenId::SMALLPRINT_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_span},            HtmlTokenId::SPAN_ON},  // Style Sheets
    {{OOO_STRING_SVTOOLS_HTML_strikethrough}, HtmlTokenId::STRIKETHROUGH_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_strong},        HtmlTokenId::STRONG_ON},
    {{OOO_STRING_SVTOOLS_HTML_style},       HtmlTokenId::STYLE_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_subscript},   HtmlTokenId::SUBSCRIPT_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_superscript}, HtmlTokenId::SUPERSCRIPT_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_table},       HtmlTokenId::TABLE_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_tabledata},   HtmlTokenId::TABLEDATA_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_tableheader}, HtmlTokenId::TABLEHEADER_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_tablerow},        HtmlTokenId::TABLEROW_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_tbody},          HtmlTokenId::TBODY_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_teletype},        HtmlTokenId::TELETYPE_ON},
    {{OOO_STRING_SVTOOLS_HTML_textarea},        HtmlTokenId::TEXTAREA_ON},
    {{OOO_STRING_SVTOOLS_HTML_tfoot},          HtmlTokenId::TFOOT_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_thead},          HtmlTokenId::THEAD_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_title},          HtmlTokenId::TITLE_ON},
    {{OOO_STRING_SVTOOLS_HTML_underline},      HtmlTokenId::UNDERLINE_ON},
    {{OOO_STRING_SVTOOLS_HTML_unorderlist},  HtmlTokenId::UNORDERLIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_variable},        HtmlTokenId::VARIABLE_ON},

    {{OOO_STRING_SVTOOLS_HTML_xmp},         HtmlTokenId::XMP_ON},
    {{OOO_STRING_SVTOOLS_HTML_listing},     HtmlTokenId::LISTING_ON},

    {{OOO_STRING_SVTOOLS_HTML_definstance},  HtmlTokenId::DEFINSTANCE_ON},
    {{OOO_STRING_SVTOOLS_HTML_strike},        HtmlTokenId::STRIKE_ON},

    {{OOO_STRING_SVTOOLS_HTML_comment2},        HtmlTokenId::COMMENT2_ON},
    {{OOO_STRING_SVTOOLS_HTML_marquee},         HtmlTokenId::MARQUEE_ON},
    {{OOO_STRING_SVTOOLS_HTML_plaintext2},    HtmlTokenId::PLAINTEXT2_ON},

    {{OOO_STRING_SVTOOLS_HTML_sdfield},     HtmlTokenId::SDFIELD_ON}
};


extern "C"
{

static int SAL_CALL HTMLKeyCompare( const void *pFirst, const void *pSecond)
{
    HTML_TokenEntry const * pFirstEntry = static_cast<HTML_TokenEntry const *>(pFirst);
    HTML_TokenEntry const * pSecondEntry = static_cast<HTML_TokenEntry const *>(pSecond);
    int nRet = 0;
    if( HtmlTokenId::INVALID == pFirstEntry->nToken )
    {
        if( HtmlTokenId::INVALID == pSecondEntry->nToken )
            nRet = pFirstEntry->pUToken->compareTo( *pSecondEntry->pUToken );
        else
            nRet = pFirstEntry->pUToken->compareToAscii( pSecondEntry->sToken );
    }
    else
    {
        if( HtmlTokenId::INVALID == pSecondEntry->nToken )
            nRet = -1 * pSecondEntry->pUToken->compareToAscii( pFirstEntry->sToken );
        else
            nRet = strcmp( pFirstEntry->sToken, pSecondEntry->sToken );
    }

    return nRet;
}

}

HtmlTokenId GetHTMLToken( const OUString& rName )
{
    if( !bSortKeyWords )
    {
        qsort( static_cast<void*>(aHTMLTokenTab),
                SAL_N_ELEMENTS( aHTMLTokenTab ),
                sizeof( HTML_TokenEntry ),
                HTMLKeyCompare );
        bSortKeyWords = true;
    }

    HtmlTokenId nRet = HtmlTokenId::NONE;

    if( rName.startsWith( OOO_STRING_SVTOOLS_HTML_comment ))
        return HtmlTokenId::COMMENT;

    void* pFound;
    HTML_TokenEntry aSrch;
    aSrch.pUToken = &rName;
    aSrch.nToken = HtmlTokenId::INVALID;

    pFound = bsearch( &aSrch,
                      static_cast<void*>(aHTMLTokenTab),
                      SAL_N_ELEMENTS( aHTMLTokenTab ),
                      sizeof( HTML_TokenEntry ),
                      HTMLKeyCompare );
    if( nullptr != pFound )
        nRet = static_cast<HTML_TokenEntry*>(pFound)->nToken;
    return nRet;
}

struct HTML_CharEntry
{
    union
    {
        const sal_Char *sName;
        const OUString *pUName;
    };
    sal_Unicode cChar;
};

// Flag: RTF token table has already been sorted
static bool bSortCharKeyWords = false;

static HTML_CharEntry aHTMLCharNameTab[] = {
    {{OOO_STRING_SVTOOLS_HTML_C_lt},             60},
    {{OOO_STRING_SVTOOLS_HTML_C_gt},             62},
    {{OOO_STRING_SVTOOLS_HTML_C_amp},        38},
    {{OOO_STRING_SVTOOLS_HTML_C_quot},       34},

    {{OOO_STRING_SVTOOLS_HTML_C_Agrave},        192},
    {{OOO_STRING_SVTOOLS_HTML_C_Aacute},        193},
    {{OOO_STRING_SVTOOLS_HTML_C_Acirc},     194},
    {{OOO_STRING_SVTOOLS_HTML_C_Atilde},        195},
    {{OOO_STRING_SVTOOLS_HTML_C_Auml},      196},
    {{OOO_STRING_SVTOOLS_HTML_C_Aring},     197},
    {{OOO_STRING_SVTOOLS_HTML_C_AElig},     198},
    {{OOO_STRING_SVTOOLS_HTML_C_Ccedil},        199},
    {{OOO_STRING_SVTOOLS_HTML_C_Egrave},        200},
    {{OOO_STRING_SVTOOLS_HTML_C_Eacute},        201},
    {{OOO_STRING_SVTOOLS_HTML_C_Ecirc},     202},
    {{OOO_STRING_SVTOOLS_HTML_C_Euml},      203},
    {{OOO_STRING_SVTOOLS_HTML_C_Igrave},        204},
    {{OOO_STRING_SVTOOLS_HTML_C_Iacute},        205},
    {{OOO_STRING_SVTOOLS_HTML_C_Icirc},     206},
    {{OOO_STRING_SVTOOLS_HTML_C_Iuml},      207},
    {{OOO_STRING_SVTOOLS_HTML_C_ETH},       208},
    {{OOO_STRING_SVTOOLS_HTML_C_Ntilde},        209},
    {{OOO_STRING_SVTOOLS_HTML_C_Ograve},        210},
    {{OOO_STRING_SVTOOLS_HTML_C_Oacute},        211},
    {{OOO_STRING_SVTOOLS_HTML_C_Ocirc},     212},
    {{OOO_STRING_SVTOOLS_HTML_C_Otilde},        213},
    {{OOO_STRING_SVTOOLS_HTML_C_Ouml},      214},
    {{OOO_STRING_SVTOOLS_HTML_C_Oslash},        216},
    {{OOO_STRING_SVTOOLS_HTML_C_Ugrave},        217},
    {{OOO_STRING_SVTOOLS_HTML_C_Uacute},        218},
    {{OOO_STRING_SVTOOLS_HTML_C_Ucirc},     219},
    {{OOO_STRING_SVTOOLS_HTML_C_Uuml},      220},
    {{OOO_STRING_SVTOOLS_HTML_C_Yacute},        221},

    {{OOO_STRING_SVTOOLS_HTML_C_THORN},     222},
    {{OOO_STRING_SVTOOLS_HTML_C_szlig},     223},

    {{OOO_STRING_SVTOOLS_HTML_S_agrave},        224},
    {{OOO_STRING_SVTOOLS_HTML_S_aacute},        225},
    {{OOO_STRING_SVTOOLS_HTML_S_acirc},     226},
    {{OOO_STRING_SVTOOLS_HTML_S_atilde},        227},
    {{OOO_STRING_SVTOOLS_HTML_S_auml},      228},
    {{OOO_STRING_SVTOOLS_HTML_S_aring},     229},
    {{OOO_STRING_SVTOOLS_HTML_S_aelig},     230},
    {{OOO_STRING_SVTOOLS_HTML_S_ccedil},        231},
    {{OOO_STRING_SVTOOLS_HTML_S_egrave},        232},
    {{OOO_STRING_SVTOOLS_HTML_S_eacute},        233},
    {{OOO_STRING_SVTOOLS_HTML_S_ecirc},     234},
    {{OOO_STRING_SVTOOLS_HTML_S_euml},      235},
    {{OOO_STRING_SVTOOLS_HTML_S_igrave},        236},
    {{OOO_STRING_SVTOOLS_HTML_S_iacute},        237},
    {{OOO_STRING_SVTOOLS_HTML_S_icirc},     238},
    {{OOO_STRING_SVTOOLS_HTML_S_iuml},      239},
    {{OOO_STRING_SVTOOLS_HTML_S_eth},       240},
    {{OOO_STRING_SVTOOLS_HTML_S_ntilde},        241},
    {{OOO_STRING_SVTOOLS_HTML_S_ograve},        242},
    {{OOO_STRING_SVTOOLS_HTML_S_oacute},        243},
    {{OOO_STRING_SVTOOLS_HTML_S_ocirc},     244},
    {{OOO_STRING_SVTOOLS_HTML_S_otilde},        245},
    {{OOO_STRING_SVTOOLS_HTML_S_ouml},      246},
    {{OOO_STRING_SVTOOLS_HTML_S_oslash},        248},
    {{OOO_STRING_SVTOOLS_HTML_S_ugrave},        249},
    {{OOO_STRING_SVTOOLS_HTML_S_uacute},        250},
    {{OOO_STRING_SVTOOLS_HTML_S_ucirc},     251},
    {{OOO_STRING_SVTOOLS_HTML_S_uuml},      252},
    {{OOO_STRING_SVTOOLS_HTML_S_yacute},        253},
    {{OOO_STRING_SVTOOLS_HTML_S_thorn},     254},
    {{OOO_STRING_SVTOOLS_HTML_S_yuml},      255},

// special characters
    {{OOO_STRING_SVTOOLS_HTML_S_acute},     180},
    {{OOO_STRING_SVTOOLS_HTML_S_brvbar},    166},
    {{OOO_STRING_SVTOOLS_HTML_S_cedil},      184},
    {{OOO_STRING_SVTOOLS_HTML_S_cent},       162},
    {{OOO_STRING_SVTOOLS_HTML_S_copy},      169},
    {{OOO_STRING_SVTOOLS_HTML_S_curren},     164},
    {{OOO_STRING_SVTOOLS_HTML_S_deg},        176},
    {{OOO_STRING_SVTOOLS_HTML_S_divide},     247},
    {{OOO_STRING_SVTOOLS_HTML_S_frac12},     189},
    {{OOO_STRING_SVTOOLS_HTML_S_frac14},     188},
    {{OOO_STRING_SVTOOLS_HTML_S_frac34},     190},
    {{OOO_STRING_SVTOOLS_HTML_S_iexcl},      161},
    {{OOO_STRING_SVTOOLS_HTML_S_iquest},     191},
    {{OOO_STRING_SVTOOLS_HTML_S_laquo},      171},
    {{OOO_STRING_SVTOOLS_HTML_S_macr},       175},
    {{OOO_STRING_SVTOOLS_HTML_S_micro},      181},
    {{OOO_STRING_SVTOOLS_HTML_S_middot},     183},
    {{OOO_STRING_SVTOOLS_HTML_S_not},        172},
    {{OOO_STRING_SVTOOLS_HTML_S_ordf},       170},
    {{OOO_STRING_SVTOOLS_HTML_S_ordm},       186},
    {{OOO_STRING_SVTOOLS_HTML_S_para},       182},
    {{OOO_STRING_SVTOOLS_HTML_S_plusmn},     177},
    {{OOO_STRING_SVTOOLS_HTML_S_pound},      163},
    {{OOO_STRING_SVTOOLS_HTML_S_raquo},      187},
    {{OOO_STRING_SVTOOLS_HTML_S_reg},       174},
    {{OOO_STRING_SVTOOLS_HTML_S_sect},       167},
    {{OOO_STRING_SVTOOLS_HTML_S_sup1},       185},
    {{OOO_STRING_SVTOOLS_HTML_S_sup2},       178},
    {{OOO_STRING_SVTOOLS_HTML_S_sup3},       179},
    {{OOO_STRING_SVTOOLS_HTML_S_times},      215},
    {{OOO_STRING_SVTOOLS_HTML_S_uml},        168},
    {{OOO_STRING_SVTOOLS_HTML_S_yen},        165},

// special characters, which will be converted to tokens !!!
    {{OOO_STRING_SVTOOLS_HTML_S_nbsp},      1},
    {{OOO_STRING_SVTOOLS_HTML_S_shy},       2},


// HTML4
    {{OOO_STRING_SVTOOLS_HTML_S_OElig},     338},
    {{OOO_STRING_SVTOOLS_HTML_S_oelig},     339},
    {{OOO_STRING_SVTOOLS_HTML_S_Scaron},        352},
    {{OOO_STRING_SVTOOLS_HTML_S_scaron},        353},
    {{OOO_STRING_SVTOOLS_HTML_S_Yuml},      376},
    {{OOO_STRING_SVTOOLS_HTML_S_fnof},      402},
    {{OOO_STRING_SVTOOLS_HTML_S_circ},      710},
    {{OOO_STRING_SVTOOLS_HTML_S_tilde},     732},
    {{OOO_STRING_SVTOOLS_HTML_S_Alpha},     913},
    {{OOO_STRING_SVTOOLS_HTML_S_Beta},      914},
    {{OOO_STRING_SVTOOLS_HTML_S_Gamma},     915},
    {{OOO_STRING_SVTOOLS_HTML_S_Delta},     916},
    {{OOO_STRING_SVTOOLS_HTML_S_Epsilon},   917},
    {{OOO_STRING_SVTOOLS_HTML_S_Zeta},      918},
    {{OOO_STRING_SVTOOLS_HTML_S_Eta},       919},
    {{OOO_STRING_SVTOOLS_HTML_S_Theta},     920},
    {{OOO_STRING_SVTOOLS_HTML_S_Iota},      921},
    {{OOO_STRING_SVTOOLS_HTML_S_Kappa},     922},
    {{OOO_STRING_SVTOOLS_HTML_S_Lambda},        923},
    {{OOO_STRING_SVTOOLS_HTML_S_Mu},            924},
    {{OOO_STRING_SVTOOLS_HTML_S_Nu},            925},
    {{OOO_STRING_SVTOOLS_HTML_S_Xi},            926},
    {{OOO_STRING_SVTOOLS_HTML_S_Omicron},   927},
    {{OOO_STRING_SVTOOLS_HTML_S_Pi},            928},
    {{OOO_STRING_SVTOOLS_HTML_S_Rho},       929},
    {{OOO_STRING_SVTOOLS_HTML_S_Sigma},     931},
    {{OOO_STRING_SVTOOLS_HTML_S_Tau},       932},
    {{OOO_STRING_SVTOOLS_HTML_S_Upsilon},   933},
    {{OOO_STRING_SVTOOLS_HTML_S_Phi},       934},
    {{OOO_STRING_SVTOOLS_HTML_S_Chi},       935},
    {{OOO_STRING_SVTOOLS_HTML_S_Psi},       936},
    {{OOO_STRING_SVTOOLS_HTML_S_Omega},     937},
    {{OOO_STRING_SVTOOLS_HTML_S_alpha},     945},
    {{OOO_STRING_SVTOOLS_HTML_S_beta},      946},
    {{OOO_STRING_SVTOOLS_HTML_S_gamma},     947},
    {{OOO_STRING_SVTOOLS_HTML_S_delta},     948},
    {{OOO_STRING_SVTOOLS_HTML_S_epsilon},   949},
    {{OOO_STRING_SVTOOLS_HTML_S_zeta},      950},
    {{OOO_STRING_SVTOOLS_HTML_S_eta},       951},
    {{OOO_STRING_SVTOOLS_HTML_S_theta},     952},
    {{OOO_STRING_SVTOOLS_HTML_S_iota},      953},
    {{OOO_STRING_SVTOOLS_HTML_S_kappa},     954},
    {{OOO_STRING_SVTOOLS_HTML_S_lambda},        955},
    {{OOO_STRING_SVTOOLS_HTML_S_mu},            956},
    {{OOO_STRING_SVTOOLS_HTML_S_nu},            957},
    {{OOO_STRING_SVTOOLS_HTML_S_xi},            958},
    {{OOO_STRING_SVTOOLS_HTML_S_omicron},   959},
    {{OOO_STRING_SVTOOLS_HTML_S_pi},            960},
    {{OOO_STRING_SVTOOLS_HTML_S_rho},       961},
    {{OOO_STRING_SVTOOLS_HTML_S_sigmaf},        962},
    {{OOO_STRING_SVTOOLS_HTML_S_sigma},     963},
    {{OOO_STRING_SVTOOLS_HTML_S_tau},       964},
    {{OOO_STRING_SVTOOLS_HTML_S_upsilon},   965},
    {{OOO_STRING_SVTOOLS_HTML_S_phi},       966},
    {{OOO_STRING_SVTOOLS_HTML_S_chi},       967},
    {{OOO_STRING_SVTOOLS_HTML_S_psi},       968},
    {{OOO_STRING_SVTOOLS_HTML_S_omega},     969},
    {{OOO_STRING_SVTOOLS_HTML_S_thetasym},  977},
    {{OOO_STRING_SVTOOLS_HTML_S_upsih},     978},
    {{OOO_STRING_SVTOOLS_HTML_S_piv},       982},
    {{OOO_STRING_SVTOOLS_HTML_S_ensp},      8194},
    {{OOO_STRING_SVTOOLS_HTML_S_emsp},      8195},
    {{OOO_STRING_SVTOOLS_HTML_S_thinsp},        8201},
    {{OOO_STRING_SVTOOLS_HTML_S_zwnj},      8204},
    {{OOO_STRING_SVTOOLS_HTML_S_zwj},       8205},
    {{OOO_STRING_SVTOOLS_HTML_S_lrm},       8206},
    {{OOO_STRING_SVTOOLS_HTML_S_rlm},       8207},
    {{OOO_STRING_SVTOOLS_HTML_S_ndash},     8211},
    {{OOO_STRING_SVTOOLS_HTML_S_mdash},     8212},
    {{OOO_STRING_SVTOOLS_HTML_S_lsquo},     8216},
    {{OOO_STRING_SVTOOLS_HTML_S_rsquo},     8217},
    {{OOO_STRING_SVTOOLS_HTML_S_sbquo},     8218},
    {{OOO_STRING_SVTOOLS_HTML_S_ldquo},     8220},
    {{OOO_STRING_SVTOOLS_HTML_S_rdquo},     8221},
    {{OOO_STRING_SVTOOLS_HTML_S_bdquo},     8222},
    {{OOO_STRING_SVTOOLS_HTML_S_dagger},        8224},
    {{OOO_STRING_SVTOOLS_HTML_S_Dagger},        8225},
    {{OOO_STRING_SVTOOLS_HTML_S_bull},      8226},
    {{OOO_STRING_SVTOOLS_HTML_S_hellip},        8230},
    {{OOO_STRING_SVTOOLS_HTML_S_permil},        8240},
    {{OOO_STRING_SVTOOLS_HTML_S_prime},     8242},
    {{OOO_STRING_SVTOOLS_HTML_S_Prime},     8243},
    {{OOO_STRING_SVTOOLS_HTML_S_lsaquo},        8249},
    {{OOO_STRING_SVTOOLS_HTML_S_rsaquo},        8250},
    {{OOO_STRING_SVTOOLS_HTML_S_oline},     8254},
    {{OOO_STRING_SVTOOLS_HTML_S_frasl},     8260},
    {{OOO_STRING_SVTOOLS_HTML_S_euro},      8364},
    {{OOO_STRING_SVTOOLS_HTML_S_image},     8465},
    {{OOO_STRING_SVTOOLS_HTML_S_weierp},        8472},
    {{OOO_STRING_SVTOOLS_HTML_S_real},      8476},
    {{OOO_STRING_SVTOOLS_HTML_S_trade},     8482},
    {{OOO_STRING_SVTOOLS_HTML_S_alefsym},   8501},
    {{OOO_STRING_SVTOOLS_HTML_S_larr},      8592},
    {{OOO_STRING_SVTOOLS_HTML_S_uarr},      8593},
    {{OOO_STRING_SVTOOLS_HTML_S_rarr},      8594},
    {{OOO_STRING_SVTOOLS_HTML_S_darr},      8595},
    {{OOO_STRING_SVTOOLS_HTML_S_harr},      8596},
    {{OOO_STRING_SVTOOLS_HTML_S_crarr},     8629},
    {{OOO_STRING_SVTOOLS_HTML_S_lArr},      8656},
    {{OOO_STRING_SVTOOLS_HTML_S_uArr},      8657},
    {{OOO_STRING_SVTOOLS_HTML_S_rArr},      8658},
    {{OOO_STRING_SVTOOLS_HTML_S_dArr},      8659},
    {{OOO_STRING_SVTOOLS_HTML_S_hArr},      8660},
    {{OOO_STRING_SVTOOLS_HTML_S_forall},        8704},
    {{OOO_STRING_SVTOOLS_HTML_S_part},      8706},
    {{OOO_STRING_SVTOOLS_HTML_S_exist},     8707},
    {{OOO_STRING_SVTOOLS_HTML_S_empty},     8709},
    {{OOO_STRING_SVTOOLS_HTML_S_nabla},     8711},
    {{OOO_STRING_SVTOOLS_HTML_S_isin},      8712},
    {{OOO_STRING_SVTOOLS_HTML_S_notin},     8713},
    {{OOO_STRING_SVTOOLS_HTML_S_ni},            8715},
    {{OOO_STRING_SVTOOLS_HTML_S_prod},      8719},
    {{OOO_STRING_SVTOOLS_HTML_S_sum},       8721},
    {{OOO_STRING_SVTOOLS_HTML_S_minus},     8722},
    {{OOO_STRING_SVTOOLS_HTML_S_lowast},        8727},
    {{OOO_STRING_SVTOOLS_HTML_S_radic},     8730},
    {{OOO_STRING_SVTOOLS_HTML_S_prop},      8733},
    {{OOO_STRING_SVTOOLS_HTML_S_infin},     8734},
    {{OOO_STRING_SVTOOLS_HTML_S_ang},       8736},
    {{OOO_STRING_SVTOOLS_HTML_S_and},       8743},
    {{OOO_STRING_SVTOOLS_HTML_S_or},            8744},
    {{OOO_STRING_SVTOOLS_HTML_S_cap},       8745},
    {{OOO_STRING_SVTOOLS_HTML_S_cup},       8746},
    {{OOO_STRING_SVTOOLS_HTML_S_int},       8747},
    {{OOO_STRING_SVTOOLS_HTML_S_there4},        8756},
    {{OOO_STRING_SVTOOLS_HTML_S_sim},       8764},
    {{OOO_STRING_SVTOOLS_HTML_S_cong},      8773},
    {{OOO_STRING_SVTOOLS_HTML_S_asymp},     8776},
    {{OOO_STRING_SVTOOLS_HTML_S_ne},            8800},
    {{OOO_STRING_SVTOOLS_HTML_S_equiv},     8801},
    {{OOO_STRING_SVTOOLS_HTML_S_le},            8804},
    {{OOO_STRING_SVTOOLS_HTML_S_ge},            8805},
    {{OOO_STRING_SVTOOLS_HTML_S_sub},       8834},
    {{OOO_STRING_SVTOOLS_HTML_S_sup},       8835},
    {{OOO_STRING_SVTOOLS_HTML_S_nsub},      8836},
    {{OOO_STRING_SVTOOLS_HTML_S_sube},      8838},
    {{OOO_STRING_SVTOOLS_HTML_S_supe},      8839},
    {{OOO_STRING_SVTOOLS_HTML_S_oplus},     8853},
    {{OOO_STRING_SVTOOLS_HTML_S_otimes},        8855},
    {{OOO_STRING_SVTOOLS_HTML_S_perp},      8869},
    {{OOO_STRING_SVTOOLS_HTML_S_sdot},      8901},
    {{OOO_STRING_SVTOOLS_HTML_S_lceil},     8968},
    {{OOO_STRING_SVTOOLS_HTML_S_rceil},     8969},
    {{OOO_STRING_SVTOOLS_HTML_S_lfloor},        8970},
    {{OOO_STRING_SVTOOLS_HTML_S_rfloor},        8971},
    {{OOO_STRING_SVTOOLS_HTML_S_lang},      9001},
    {{OOO_STRING_SVTOOLS_HTML_S_rang},      9002},
    {{OOO_STRING_SVTOOLS_HTML_S_loz},       9674},
    {{OOO_STRING_SVTOOLS_HTML_S_spades},        9824},
    {{OOO_STRING_SVTOOLS_HTML_S_clubs},     9827},
    {{OOO_STRING_SVTOOLS_HTML_S_hearts},        9829},
    {{OOO_STRING_SVTOOLS_HTML_S_diams},     9830}
};

extern "C"
{

static int SAL_CALL HTMLCharNameCompare( const void *pFirst, const void *pSecond)
{
    HTML_CharEntry const * pFirstEntry = static_cast<HTML_CharEntry const *>(pFirst);
    HTML_CharEntry const * pSecondEntry = static_cast<HTML_CharEntry const *>(pSecond);
    int nRet = 0;
    if( USHRT_MAX == pFirstEntry->cChar )
    {
        if( USHRT_MAX == pSecondEntry->cChar )
            nRet = pFirstEntry->pUName->compareTo( *pSecondEntry->pUName );
        else
            nRet = pFirstEntry->pUName->compareToAscii( pSecondEntry->sName );
    }
    else
    {
        if( USHRT_MAX == pSecondEntry->cChar )
            nRet = -1 * pSecondEntry->pUName->compareToAscii( pFirstEntry->sName );
        else
            nRet = strcmp( pFirstEntry->sName, pSecondEntry->sName );
    }

    return nRet;
}

} // extern "C"

sal_Unicode GetHTMLCharName( const OUString& rName )
{
    if( !bSortCharKeyWords )
    {
        qsort( static_cast<void*>(aHTMLCharNameTab),
                SAL_N_ELEMENTS( aHTMLCharNameTab ),
                sizeof( HTML_CharEntry ),
                HTMLCharNameCompare );
        bSortCharKeyWords = true;
    }

    sal_Unicode cRet = 0;
    void* pFound;
    HTML_CharEntry aSrch;
    aSrch.pUName = &rName;
    aSrch.cChar = USHRT_MAX;

    if( nullptr != ( pFound = bsearch( &aSrch,
                        static_cast<void*>(aHTMLCharNameTab),
                        SAL_N_ELEMENTS( aHTMLCharNameTab),
                        sizeof( HTML_CharEntry ),
                        HTMLCharNameCompare )))
        cRet = static_cast<HTML_CharEntry*>(pFound)->cChar;
    return cRet;
}

// Flag: Options table has already been sorted
static bool bSortOptionKeyWords = false;

struct HTML_OptionEntry
{
    union
    {
        const sal_Char *sToken;
        const OUString *pUToken;
    };
    HtmlOptionId nToken;
};
static HTML_OptionEntry aHTMLOptionTab[] = {

// Attributes without value
    {{OOO_STRING_SVTOOLS_HTML_O_checked},   HtmlOptionId::CHECKED},
    {{OOO_STRING_SVTOOLS_HTML_O_compact},   HtmlOptionId::COMPACT},
    {{OOO_STRING_SVTOOLS_HTML_O_declare},   HtmlOptionId::DECLARE},
    {{OOO_STRING_SVTOOLS_HTML_O_disabled},  HtmlOptionId::DISABLED},
    {{OOO_STRING_SVTOOLS_HTML_O_ismap},     HtmlOptionId::ISMAP},
    {{OOO_STRING_SVTOOLS_HTML_O_mayscript}, HtmlOptionId::MAYSCRIPT},
    {{OOO_STRING_SVTOOLS_HTML_O_multiple},  HtmlOptionId::MULTIPLE},
    {{OOO_STRING_SVTOOLS_HTML_O_nohref},        HtmlOptionId::NOHREF}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_noresize},  HtmlOptionId::NORESIZE}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_noshade},   HtmlOptionId::NOSHADE}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_nowrap},        HtmlOptionId::NOWRAP},
    {{OOO_STRING_SVTOOLS_HTML_O_sdfixed},   HtmlOptionId::SDFIXED},
    {{OOO_STRING_SVTOOLS_HTML_O_selected},      HtmlOptionId::SELECTED},

// Attributes with a string value
    {{OOO_STRING_SVTOOLS_HTML_O_accept},        HtmlOptionId::ACCEPT},
    {{OOO_STRING_SVTOOLS_HTML_O_accesskey}, HtmlOptionId::ACCESSKEY},
    {{OOO_STRING_SVTOOLS_HTML_O_alt},       HtmlOptionId::ALT},
    {{OOO_STRING_SVTOOLS_HTML_O_axis},      HtmlOptionId::AXIS},
    {{OOO_STRING_SVTOOLS_HTML_O_char},      HtmlOptionId::CHAR}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_charset},   HtmlOptionId::CHARSET},
    {{OOO_STRING_SVTOOLS_HTML_O_class},     HtmlOptionId::CLASS},
    {{OOO_STRING_SVTOOLS_HTML_O_code},      HtmlOptionId::CODE}, // HotJava
    {{OOO_STRING_SVTOOLS_HTML_O_codetype},  HtmlOptionId::CODETYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_content},   HtmlOptionId::CONTENT},
    {{OOO_STRING_SVTOOLS_HTML_O_coords},        HtmlOptionId::COORDS}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_enctype},   HtmlOptionId::ENCTYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_face},      HtmlOptionId::FACE}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_frameborder}, HtmlOptionId::FRAMEBORDER}, // IExplorer 3.0
    {{OOO_STRING_SVTOOLS_HTML_O_httpequiv}, HtmlOptionId::HTTPEQUIV},
    {{OOO_STRING_SVTOOLS_HTML_O_language},  HtmlOptionId::LANGUAGE}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_name},      HtmlOptionId::NAME},
    {{OOO_STRING_SVTOOLS_HTML_O_prompt},        HtmlOptionId::PROMPT},
    {{OOO_STRING_SVTOOLS_HTML_O_shape},     HtmlOptionId::SHAPE},
    {{OOO_STRING_SVTOOLS_HTML_O_standby},   HtmlOptionId::STANDBY},
    {{OOO_STRING_SVTOOLS_HTML_O_style},     HtmlOptionId::STYLE},
    {{OOO_STRING_SVTOOLS_HTML_O_title},     HtmlOptionId::TITLE},
    {{OOO_STRING_SVTOOLS_HTML_O_value},     HtmlOptionId::VALUE},
    {{OOO_STRING_SVTOOLS_HTML_O_SDval},     HtmlOptionId::SDVAL}, // StarDiv NumberValue
    {{OOO_STRING_SVTOOLS_HTML_O_SDnum},     HtmlOptionId::SDNUM}, // StarDiv NumberFormat
    {{OOO_STRING_SVTOOLS_HTML_O_sdlibrary}, HtmlOptionId::SDLIBRARY},
    {{OOO_STRING_SVTOOLS_HTML_O_sdmodule},  HtmlOptionId::SDMODULE},

// Attributes with a SGML identifier value
    {{OOO_STRING_SVTOOLS_HTML_O_id},            HtmlOptionId::ID},
    {{OOO_STRING_SVTOOLS_HTML_O_target},        HtmlOptionId::TARGET}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_to},            HtmlOptionId::TO},

// Attributes with an URI value
    {{OOO_STRING_SVTOOLS_HTML_O_action},        HtmlOptionId::ACTION},
    {{OOO_STRING_SVTOOLS_HTML_O_archive},       HtmlOptionId::ARCHIVE},
    {{OOO_STRING_SVTOOLS_HTML_O_background},    HtmlOptionId::BACKGROUND},
    {{OOO_STRING_SVTOOLS_HTML_O_classid},   HtmlOptionId::CLASSID},
    {{OOO_STRING_SVTOOLS_HTML_O_codebase},  HtmlOptionId::CODEBASE}, // HotJava
    {{OOO_STRING_SVTOOLS_HTML_O_data},      HtmlOptionId::DATA},
    {{OOO_STRING_SVTOOLS_HTML_O_href},      HtmlOptionId::HREF},
    {{OOO_STRING_SVTOOLS_HTML_O_script},        HtmlOptionId::SCRIPT},
    {{OOO_STRING_SVTOOLS_HTML_O_src},       HtmlOptionId::SRC},
    {{OOO_STRING_SVTOOLS_HTML_O_usemap},        HtmlOptionId::USEMAP}, // Netscape 2.0

// Attributes with a color value (all Netscape versions)
    {{OOO_STRING_SVTOOLS_HTML_O_alink},     HtmlOptionId::ALINK},
    {{OOO_STRING_SVTOOLS_HTML_O_bgcolor},   HtmlOptionId::BGCOLOR},
    {{OOO_STRING_SVTOOLS_HTML_O_bordercolor}, HtmlOptionId::BORDERCOLOR}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_bordercolorlight}, HtmlOptionId::BORDERCOLORLIGHT}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_bordercolordark}, HtmlOptionId::BORDERCOLORDARK}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_color},     HtmlOptionId::COLOR},
    {{OOO_STRING_SVTOOLS_HTML_O_link},      HtmlOptionId::LINK},
    {{OOO_STRING_SVTOOLS_HTML_O_text},      HtmlOptionId::TEXT},
    {{OOO_STRING_SVTOOLS_HTML_O_vlink},     HtmlOptionId::VLINK},

// Attributes with a numerical value
    {{OOO_STRING_SVTOOLS_HTML_O_border},        HtmlOptionId::BORDER},
    {{OOO_STRING_SVTOOLS_HTML_O_cellspacing},HtmlOptionId::CELLSPACING}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_cellpadding},HtmlOptionId::CELLPADDING}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_charoff},   HtmlOptionId::CHAROFF}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_colspan},   HtmlOptionId::COLSPAN},
    {{OOO_STRING_SVTOOLS_HTML_O_framespacing}, HtmlOptionId::FRAMESPACING}, // IExplorer 3.0
    {{OOO_STRING_SVTOOLS_HTML_O_gutter},        HtmlOptionId::GUTTER}, // Netscape 3.0b5
    {{OOO_STRING_SVTOOLS_HTML_O_height},        HtmlOptionId::HEIGHT},
    {{OOO_STRING_SVTOOLS_HTML_O_hspace},        HtmlOptionId::HSPACE}, // Netscape
    {{OOO_STRING_SVTOOLS_HTML_O_left},      HtmlOptionId::LEFT},
    {{OOO_STRING_SVTOOLS_HTML_O_loop},      HtmlOptionId::LOOP}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_marginheight},HtmlOptionId::MARGINHEIGHT}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_marginwidth},HtmlOptionId::MARGINWIDTH}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_maxlength}, HtmlOptionId::MAXLENGTH},
    {{OOO_STRING_SVTOOLS_HTML_O_rowspan},   HtmlOptionId::ROWSPAN},
    {{OOO_STRING_SVTOOLS_HTML_O_scrollamount}, HtmlOptionId::SCROLLAMOUNT}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_scrolldelay}, HtmlOptionId::SCROLLDELAY}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_span},      HtmlOptionId::SPAN}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_tabindex},  HtmlOptionId::TABINDEX},
    {{OOO_STRING_SVTOOLS_HTML_O_vspace},        HtmlOptionId::VSPACE}, // Netscape
    {{OOO_STRING_SVTOOLS_HTML_O_width},     HtmlOptionId::WIDTH},
    {{OOO_STRING_SVTOOLS_HTML_O_zindex},        HtmlOptionId::ZINDEX},

// Attributes with enum values
    {{OOO_STRING_SVTOOLS_HTML_O_behavior},  HtmlOptionId::BEHAVIOR}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_clear},     HtmlOptionId::CLEAR},
    {{OOO_STRING_SVTOOLS_HTML_O_dir},       HtmlOptionId::DIR}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_direction},     HtmlOptionId::DIRECTION}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_format},        HtmlOptionId::FORMAT},
    {{OOO_STRING_SVTOOLS_HTML_O_frame},     HtmlOptionId::FRAME}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_lang},      HtmlOptionId::LANG},
    {{OOO_STRING_SVTOOLS_HTML_O_method},        HtmlOptionId::METHOD},
    {{OOO_STRING_SVTOOLS_HTML_O_rel},       HtmlOptionId::REL},
    {{OOO_STRING_SVTOOLS_HTML_O_rev},       HtmlOptionId::REV},
    {{OOO_STRING_SVTOOLS_HTML_O_rules},     HtmlOptionId::RULES}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_scrolling}, HtmlOptionId::SCROLLING}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_sdreadonly},    HtmlOptionId::SDREADONLY},
    {{OOO_STRING_SVTOOLS_HTML_O_subtype},   HtmlOptionId::SUBTYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_type},      HtmlOptionId::TYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_valign},        HtmlOptionId::VALIGN},
    {{OOO_STRING_SVTOOLS_HTML_O_valuetype}, HtmlOptionId::VALUETYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_wrap},      HtmlOptionId::WRAP},

// Attributes with script code value
    {{OOO_STRING_SVTOOLS_HTML_O_onblur},        HtmlOptionId::ONBLUR}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onchange},  HtmlOptionId::ONCHANGE}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onclick},   HtmlOptionId::ONCLICK}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onfocus},   HtmlOptionId::ONFOCUS}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onload},    HtmlOptionId::ONLOAD}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onmouseover}, HtmlOptionId::ONMOUSEOVER}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onreset},   HtmlOptionId::ONRESET}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onselect},  HtmlOptionId::ONSELECT}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onsubmit},  HtmlOptionId::ONSUBMIT}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onunload},  HtmlOptionId::ONUNLOAD}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onabort},   HtmlOptionId::ONABORT}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onerror},   HtmlOptionId::ONERROR}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onmouseout},    HtmlOptionId::ONMOUSEOUT}, // JavaScript

    {{OOO_STRING_SVTOOLS_HTML_O_SDonblur},      HtmlOptionId::SDONBLUR}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonchange},    HtmlOptionId::SDONCHANGE}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonclick},         HtmlOptionId::SDONCLICK}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonfocus},         HtmlOptionId::SDONFOCUS}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonload},      HtmlOptionId::SDONLOAD}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonmouseover},     HtmlOptionId::SDONMOUSEOVER}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonreset},     HtmlOptionId::SDONRESET}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonselect},        HtmlOptionId::SDONSELECT}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonsubmit},        HtmlOptionId::SDONSUBMIT}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonunload},        HtmlOptionId::SDONUNLOAD}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonabort},     HtmlOptionId::SDONABORT}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonerror},         HtmlOptionId::SDONERROR}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonmouseout},  HtmlOptionId::SDONMOUSEOUT}, // StarBasic

// Attributes with context sensitive values
    {{OOO_STRING_SVTOOLS_HTML_O_align},     HtmlOptionId::ALIGN},
    {{OOO_STRING_SVTOOLS_HTML_O_cols},      HtmlOptionId::COLS}, // Netscape 2.0 vs HTML 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_rows},      HtmlOptionId::ROWS}, // Netscape 2.0 vs HTML 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_size},      HtmlOptionId::SIZE},
    {{OOO_STRING_SVTOOLS_HTML_O_start},     HtmlOptionId::START}, // Netscape 2.0 vs IExplorer 2.0
};

HtmlOptionId GetHTMLOption( const OUString& rName )
{
    if( !bSortOptionKeyWords )
    {
        qsort( static_cast<void*>(aHTMLOptionTab),
                SAL_N_ELEMENTS( aHTMLOptionTab ),
                sizeof( HTML_OptionEntry ),
                HTMLKeyCompare );
        bSortOptionKeyWords = true;
    }

    HtmlOptionId nRet = HtmlOptionId::UNKNOWN;
    void* pFound;
    HTML_TokenEntry aSrch;
    aSrch.pUToken = &rName;
    aSrch.nToken = HtmlTokenId::INVALID;

    if( nullptr != ( pFound = bsearch( &aSrch,
                        static_cast<void*>(aHTMLOptionTab),
                        SAL_N_ELEMENTS( aHTMLOptionTab ),
                        sizeof( HTML_OptionEntry ),
                        HTMLKeyCompare )))
        nRet = static_cast<HTML_OptionEntry*>(pFound)->nToken;
    return nRet;
}


struct HTML_ColorEntry
{
    union
    {
        const sal_Char* sName;
        const OUString *pUName;
    };
    sal_uInt32 nColor;
};

// Flag: color table has already been sorted
static bool bSortColorKeyWords = false;

#define HTML_NO_COLOR 0xffffffffUL

// Color names are not exported (source:
// "http://www.uio.no/~mnbjerke/colors_w.html")
// "http://www.infi.net/wwwimages/colorindex.html" seem to be buggy.
static HTML_ColorEntry aHTMLColorNameTab[] = {
    { { "aliceblue" }, 0x00f0f8ffUL },
    { { "antiquewhite" }, 0x00faebd7UL },
    { { "aqua" }, 0x0000ffffUL },
    { { "aquamarine" }, 0x007fffd4UL },
    { { "azure" }, 0x00f0ffffUL },
    { { "beige" }, 0x00f5f5dcUL },
    { { "bisque" }, 0x00ffe4c4UL },
    { { "black" }, 0x00000000UL },
    { { "blanchedalmond" }, 0x00ffebcdUL },
    { { "blue" }, 0x000000ffUL },
    { { "blueviolet" }, 0x008a2be2UL },
    { { "brown" }, 0x00a52a2aUL },
    { { "burlywood" }, 0x00deb887UL },
    { { "cadetblue" }, 0x005f9ea0UL },
    { { "chartreuse" }, 0x007fff00UL },
    { { "chocolate" }, 0x00d2691eUL },
    { { "coral" }, 0x00ff7f50UL },
    { { "cornflowerblue" }, 0x006495edUL },
    { { "cornsilk" }, 0x00fff8dcUL },
    { { "crimson" }, 0x00dc143cUL },
    { { "cyan" }, 0x0000ffffUL },
    { { "darkblue" }, 0x0000008bUL },
    { { "darkcyan" }, 0x00008b8bUL },
    { { "darkgoldenrod" }, 0x00b8860bUL },
    { { "darkgray" }, 0x00a9a9a9UL },
    { { "darkgreen" }, 0x00006400UL },
    { { "darkkhaki" }, 0x00bdb76bUL },
    { { "darkmagenta" }, 0x008b008bUL },
    { { "darkolivegreen" }, 0x00556b2fUL },
    { { "darkorange" }, 0x00ff8c00UL },
    { { "darkorchid" }, 0x009932ccUL },
    { { "darkred" }, 0x008b0000UL },
    { { "darksalmon" }, 0x00e9967aUL },
    { { "darkseagreen" }, 0x008fbc8fUL },
    { { "darkslateblue" }, 0x00483d8bUL },
    { { "darkslategray" }, 0x002f4f4fUL },
    { { "darkturquoise" }, 0x0000ced1UL },
    { { "darkviolet" }, 0x009400d3UL },
    { { "deeppink" }, 0x00ff1493UL },
    { { "deepskyblue" }, 0x0000bfffUL },
    { { "dimgray" }, 0x00696969UL },
    { { "dodgerblue" }, 0x001e90ffUL },
    { { "firebrick" }, 0x00b22222UL },
    { { "floralwhite" }, 0x00fffaf0UL },
    { { "forestgreen" }, 0x00228b22UL },
    { { "fuchsia" }, 0x00ff00ffUL },
    { { "gainsboro" }, 0x00dcdcdcUL },
    { { "ghostwhite" }, 0x00f8f8ffUL },
    { { "gold" }, 0x00ffd700UL },
    { { "goldenrod" }, 0x00daa520UL },
    { { "gray" }, 0x00808080UL },
    { { "green" }, 0x00008000UL },
    { { "greenyellow" }, 0x00adff2fUL },
    { { "honeydew" }, 0x00f0fff0UL },
    { { "hotpink" }, 0x00ff69b4UL },
    { { "indianred" }, 0x00cd5c5cUL },
    { { "indigo" }, 0x004b0082UL },
    { { "ivory" }, 0x00fffff0UL },
    { { "khaki" }, 0x00f0e68cUL },
    { { "lavender" }, 0x00e6e6faUL },
    { { "lavenderblush" }, 0x00fff0f5UL },
    { { "lawngreen" }, 0x007cfc00UL },
    { { "lemonchiffon" }, 0x00fffacdUL },
    { { "lightblue" }, 0x00add8e6UL },
    { { "lightcoral" }, 0x00f08080UL },
    { { "lightcyan" }, 0x00e0ffffUL },
    { { "lightgoldenrodyellow" }, 0x00fafad2UL },
    { { "lightgreen" }, 0x0090ee90UL },
    { { "lightgrey" }, 0x00d3d3d3UL },
    { { "lightpink" }, 0x00ffb6c1UL },
    { { "lightsalmon" }, 0x00ffa07aUL },
    { { "lightseagreen" }, 0x0020b2aaUL },
    { { "lightskyblue" }, 0x0087cefaUL },
    { { "lightslategray" }, 0x00778899UL },
    { { "lightsteelblue" }, 0x00b0c4deUL },
    { { "lightyellow" }, 0x00ffffe0UL },
    { { "lime" }, 0x0000ff00UL },
    { { "limegreen" }, 0x0032cd32UL },
    { { "linen" }, 0x00faf0e6UL },
    { { "magenta" }, 0x00ff00ffUL },
    { { "maroon" }, 0x00800000UL },
    { { "mediumaquamarine" }, 0x0066cdaaUL },
    { { "mediumblue" }, 0x000000cdUL },
    { { "mediumorchid" }, 0x00ba55d3UL },
    { { "mediumpurple" }, 0x009370dbUL },
    { { "mediumseagreen" }, 0x003cb371UL },
    { { "mediumslateblue" }, 0x007b68eeUL },
    { { "mediumspringgreen" }, 0x0000fa9aUL },
    { { "mediumturquoise" }, 0x0048d1ccUL },
    { { "mediumvioletred" }, 0x00c71585UL },
    { { "midnightblue" }, 0x00191970UL },
    { { "mintcream" }, 0x00f5fffaUL },
    { { "mistyrose" }, 0x00ffe4e1UL },
    { { "moccasin" }, 0x00ffe4b5UL },
    { { "navajowhite" }, 0x00ffdeadUL },
    { { "navy" }, 0x00000080UL },
    { { "oldlace" }, 0x00fdf5e6UL },
    { { "olive" }, 0x00808000UL },
    { { "olivedrab" }, 0x006b8e23UL },
    { { "orange" }, 0x00ffa500UL },
    { { "orangered" }, 0x00ff4500UL },
    { { "orchid" }, 0x00da70d6UL },
    { { "palegoldenrod" }, 0x00eee8aaUL },
    { { "palegreen" }, 0x0098fb98UL },
    { { "paleturquoise" }, 0x00afeeeeUL },
    { { "palevioletred" }, 0x00db7093UL },
    { { "papayawhip" }, 0x00ffefd5UL },
    { { "peachpuff" }, 0x00ffdab9UL },
    { { "peru" }, 0x00cd853fUL },
    { { "pink" }, 0x00ffc0cbUL },
    { { "plum" }, 0x00dda0ddUL },
    { { "powderblue" }, 0x00b0e0e6UL },
    { { "purple" }, 0x00800080UL },
    { { "red" }, 0x00ff0000UL },
    { { "rosybrown" }, 0x00bc8f8fUL },
    { { "royalblue" }, 0x004169e1UL },
    { { "saddlebrown" }, 0x008b4513UL },
    { { "salmon" }, 0x00fa8072UL },
    { { "sandybrown" }, 0x00f4a460UL },
    { { "seagreen" }, 0x002e8b57UL },
    { { "seashell" }, 0x00fff5eeUL },
    { { "sienna" }, 0x00a0522dUL },
    { { "silver" }, 0x00c0c0c0UL },
    { { "skyblue" }, 0x0087ceebUL },
    { { "slateblue" }, 0x006a5acdUL },
    { { "slategray" }, 0x00708090UL },
    { { "snow" }, 0x00fffafaUL },
    { { "springgreen" }, 0x0000ff7fUL },
    { { "steelblue" }, 0x004682b4UL },
    { { "tan" }, 0x00d2b48cUL },
    { { "teal" }, 0x00008080UL },
    { { "thistle" }, 0x00d8bfd8UL },
    { { "tomato" }, 0x00ff6347UL },
    { { "turquoise" }, 0x0040e0d0UL },
    { { "violet" }, 0x00ee82eeUL },
    { { "wheat" }, 0x00f5deb3UL },
    { { "white" }, 0x00ffffffUL },
    { { "whitesmoke" }, 0x00f5f5f5UL },
    { { "yellow" }, 0x00ffff00UL },
    { { "yellowgreen" }, 0x009acd32UL }
};

extern "C"
{

static int SAL_CALL HTMLColorNameCompare( const void *pFirst, const void *pSecond)
{
    HTML_ColorEntry const * pFirstEntry = static_cast<HTML_ColorEntry const *>(pFirst);
    HTML_ColorEntry const * pSecondEntry = static_cast<HTML_ColorEntry const *>(pSecond);
    int nRet = 0;
    if( HTML_NO_COLOR == pFirstEntry->nColor )
    {
        if( HTML_NO_COLOR == pSecondEntry->nColor )
            nRet = pFirstEntry->pUName->compareTo( *pSecondEntry->pUName );
        else
            nRet = pFirstEntry->pUName->compareToAscii( pSecondEntry->sName );
    }
    else
    {
        if( HTML_NO_COLOR  == pSecondEntry->nColor )
            nRet = -1 * pSecondEntry->pUName->compareToAscii( pFirstEntry->sName );
        else
            nRet = strcmp( pFirstEntry->sName, pSecondEntry->sName );
    }

    return nRet;
}

} // extern "C"

sal_uInt32 GetHTMLColor( const OUString& rName )
{
    if( !bSortColorKeyWords )
    {
        qsort( static_cast<void*>(aHTMLColorNameTab),
                SAL_N_ELEMENTS( aHTMLColorNameTab ),
                sizeof( HTML_ColorEntry ),
                HTMLColorNameCompare );
        bSortColorKeyWords = true;
    }

    sal_uInt32 nRet = HTML_NO_COLOR;
    void* pFound;
    HTML_ColorEntry aSrch;
    OUString aLowerCase(rName.toAsciiLowerCase());

    aSrch.pUName = &aLowerCase;
    aSrch.nColor = HTML_NO_COLOR;

    if( nullptr != ( pFound = bsearch( &aSrch,
                        static_cast<void*>(aHTMLColorNameTab),
                        SAL_N_ELEMENTS( aHTMLColorNameTab),
                        sizeof( HTML_ColorEntry ),
                        HTMLColorNameCompare )))
        nRet = static_cast<HTML_ColorEntry*>(pFound)->nColor;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
