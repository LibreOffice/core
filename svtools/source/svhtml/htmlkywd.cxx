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


#include <limits.h>                     // for USHRT_MAX
#include <sal/types.h>                  // for sal_Char, sal_Unicode, etc
#include <stdlib.h>                     // for bsearch, qsort
#include <string.h>                     // for strcmp
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
    int nToken;
};

// Flag: RTF token table has already been sorted
static bool bSortKeyWords = false;

static HTML_TokenEntry aHTMLTokenTab[] = {
    {{OOO_STRING_SVTOOLS_HTML_area},            HTML_AREA}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_base},            HTML_BASE}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_comment},     HTML_COMMENT},
    {{OOO_STRING_SVTOOLS_HTML_doctype},      HTML_DOCTYPE},
    {{OOO_STRING_SVTOOLS_HTML_embed},       HTML_EMBED},    // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_figureoverlay}, HTML_FIGUREOVERLAY},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_horzrule},        HTML_HORZRULE},
    {{OOO_STRING_SVTOOLS_HTML_horztab},     HTML_HORZTAB},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_image},          HTML_IMAGE},
    {{OOO_STRING_SVTOOLS_HTML_image2},         HTML_IMAGE},
    {{OOO_STRING_SVTOOLS_HTML_input},          HTML_INPUT},
    {{OOO_STRING_SVTOOLS_HTML_isindex},     HTML_ISINDEX},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_li},            HTML_LI_ON},
    {{OOO_STRING_SVTOOLS_HTML_linebreak},      HTML_LINEBREAK},
    {{OOO_STRING_SVTOOLS_HTML_link},            HTML_LINK}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_meta},            HTML_META}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_nextid},      HTML_NEXTID},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_of},          HTML_OF},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_option},        HTML_OPTION},
    {{OOO_STRING_SVTOOLS_HTML_param},       HTML_PARAM},    // HotJava
    {{OOO_STRING_SVTOOLS_HTML_range},       HTML_RANGE},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_spacer},      HTML_SPACER},   // Netscape 3.0b5
    {{OOO_STRING_SVTOOLS_HTML_wbr},         HTML_WBR},  // Netscape

    {{OOO_STRING_SVTOOLS_HTML_abbreviation},    HTML_ABBREVIATION_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_above},       HTML_ABOVE_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_acronym},     HTML_ACRONYM_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_address},      HTML_ADDRESS_ON},
    {{OOO_STRING_SVTOOLS_HTML_anchor},        HTML_ANCHOR_ON},
    {{OOO_STRING_SVTOOLS_HTML_applet},      HTML_APPLET_ON},    // HotJava
    {{OOO_STRING_SVTOOLS_HTML_array},       HTML_ARRAY_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_author},      HTML_AUTHOR_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_banner},      HTML_BANNER_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_bar},         HTML_BAR_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_basefont},        HTML_BASEFONT_ON},  // Netscape
    {{OOO_STRING_SVTOOLS_HTML_below},       HTML_BELOW_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_bigprint},        HTML_BIGPRINT_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_blink},       HTML_BLINK_ON}, // Netscape
    {{OOO_STRING_SVTOOLS_HTML_blockquote},    HTML_BLOCKQUOTE_ON},
    {{OOO_STRING_SVTOOLS_HTML_blockquote30},    HTML_BLOCKQUOTE30_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_body},            HTML_BODY_ON},
    {{OOO_STRING_SVTOOLS_HTML_bold},            HTML_BOLD_ON},
    {{OOO_STRING_SVTOOLS_HTML_boldtext},        HTML_BOLDTEXT_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_box},         HTML_BOX_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_caption},     HTML_CAPTION_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_center},      HTML_CENTER_ON},    // Netscape
    {{OOO_STRING_SVTOOLS_HTML_citiation},      HTML_CITIATION_ON},
    {{OOO_STRING_SVTOOLS_HTML_col},             HTML_COL_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_colgroup},        HTML_COLGROUP_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_code},            HTML_CODE_ON},
    {{OOO_STRING_SVTOOLS_HTML_credit},      HTML_CREDIT_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_dd},            HTML_DD_ON},
    {{OOO_STRING_SVTOOLS_HTML_deflist},      HTML_DEFLIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_deletedtext}, HTML_DELETEDTEXT_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_dirlist},      HTML_DIRLIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_division},        HTML_DIVISION_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_dot},         HTML_DOT_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_doubledot},   HTML_DOUBLEDOT_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_dt},            HTML_DT_ON},
    {{OOO_STRING_SVTOOLS_HTML_emphasis},        HTML_EMPHASIS_ON},
    {{OOO_STRING_SVTOOLS_HTML_figure},      HTML_FIGURE_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_font},            HTML_FONT_ON}, // Netscpe
    {{OOO_STRING_SVTOOLS_HTML_footnote},        HTML_FOOTNOTE_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_form},            HTML_FORM_ON},
    {{OOO_STRING_SVTOOLS_HTML_frame},       HTML_FRAME_ON}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_frameset},        HTML_FRAMESET_ON},  // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_hat},         HTML_HAT_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_head},            HTML_HEAD_ON},
    {{OOO_STRING_SVTOOLS_HTML_head1},          HTML_HEAD1_ON},
    {{OOO_STRING_SVTOOLS_HTML_head2},          HTML_HEAD2_ON},
    {{OOO_STRING_SVTOOLS_HTML_head3},          HTML_HEAD3_ON},
    {{OOO_STRING_SVTOOLS_HTML_head4},          HTML_HEAD4_ON},
    {{OOO_STRING_SVTOOLS_HTML_head5},          HTML_HEAD5_ON},
    {{OOO_STRING_SVTOOLS_HTML_head6},          HTML_HEAD6_ON},
    {{OOO_STRING_SVTOOLS_HTML_html},            HTML_HTML_ON},
    {{OOO_STRING_SVTOOLS_HTML_iframe},      HTML_IFRAME_ON},    // IE 3.0b2
    {{OOO_STRING_SVTOOLS_HTML_ilayer},      HTML_ILAYER_ON},
    {{OOO_STRING_SVTOOLS_HTML_insertedtext},    HTML_INSERTEDTEXT_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_italic},        HTML_ITALIC_ON},
    {{OOO_STRING_SVTOOLS_HTML_item},            HTML_ITEM_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_keyboard},        HTML_KEYBOARD_ON},
    {{OOO_STRING_SVTOOLS_HTML_language},        HTML_LANGUAGE_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_layer},       HTML_LAYER_ON},
    {{OOO_STRING_SVTOOLS_HTML_listheader},  HTML_LISTHEADER_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_map},         HTML_MAP_ON},   // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_math},            HTML_MATH_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_menulist},        HTML_MENULIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_multicol},        HTML_MULTICOL_ON},  // Netscape 3.0b5
    {{OOO_STRING_SVTOOLS_HTML_nobr},            HTML_NOBR_ON},  // Netscape
    {{OOO_STRING_SVTOOLS_HTML_noembed},     HTML_NOEMBED_ON},   // Netscape 2.0 ???
    {{OOO_STRING_SVTOOLS_HTML_noframe},     HTML_NOFRAMES_ON},  // Netscape 2.0 ???
    {{OOO_STRING_SVTOOLS_HTML_noframes},        HTML_NOFRAMES_ON},  // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_noscript},        HTML_NOSCRIPT_ON},  // Netscape 3.0
    {{OOO_STRING_SVTOOLS_HTML_note},            HTML_NOTE_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_object},     HTML_OBJECT_ON},
    {{OOO_STRING_SVTOOLS_HTML_orderlist},      HTML_ORDERLIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_parabreak},      HTML_PARABREAK_ON},
    {{OOO_STRING_SVTOOLS_HTML_person},      HTML_PERSON_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_plaintext},   HTML_PLAINTEXT_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_preformtxt},    HTML_PREFORMTXT_ON},
    {{OOO_STRING_SVTOOLS_HTML_root},            HTML_ROOT_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_row},         HTML_ROW_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_sample},        HTML_SAMPLE_ON},
    {{OOO_STRING_SVTOOLS_HTML_script},        HTML_SCRIPT_ON}, // HTML 3.2
    {{OOO_STRING_SVTOOLS_HTML_select},        HTML_SELECT_ON},
    {{OOO_STRING_SVTOOLS_HTML_shortquote},  HTML_SHORTQUOTE_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_smallprint},  HTML_SMALLPRINT_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_span},            HTML_SPAN_ON},  // Style Sheets
    {{OOO_STRING_SVTOOLS_HTML_squareroot},  HTML_SQUAREROOT_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_strikethrough},HTML_STRIKETHROUGH_ON},    // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_strong},        HTML_STRONG_ON},
    {{OOO_STRING_SVTOOLS_HTML_style},       HTML_STYLE_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_subscript},   HTML_SUBSCRIPT_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_superscript}, HTML_SUPERSCRIPT_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_table},       HTML_TABLE_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_tabledata},   HTML_TABLEDATA_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_tableheader}, HTML_TABLEHEADER_ON},   // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_tablerow},        HTML_TABLEROW_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_tbody},          HTML_TBODY_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_teletype},        HTML_TELETYPE_ON},
    {{OOO_STRING_SVTOOLS_HTML_text},            HTML_TEXT_ON},  // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_textarea},        HTML_TEXTAREA_ON},
    {{OOO_STRING_SVTOOLS_HTML_textflow},        HTML_TEXTFLOW_ON},
    {{OOO_STRING_SVTOOLS_HTML_tfoot},          HTML_TFOOT_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_thead},          HTML_THEAD_ON}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_tilde},       HTML_TILDE_ON}, // HTML 3.0
    {{OOO_STRING_SVTOOLS_HTML_title},          HTML_TITLE_ON},
    {{OOO_STRING_SVTOOLS_HTML_underline},      HTML_UNDERLINE_ON},
    {{OOO_STRING_SVTOOLS_HTML_unorderlist},  HTML_UNORDERLIST_ON},
    {{OOO_STRING_SVTOOLS_HTML_variable},        HTML_VARIABLE_ON},
    {{OOO_STRING_SVTOOLS_HTML_vector},      HTML_VECTOR_ON},    // HTML 3.0

    {{OOO_STRING_SVTOOLS_HTML_xmp},         HTML_XMP_ON},
    {{OOO_STRING_SVTOOLS_HTML_listing},     HTML_LISTING_ON},

    {{OOO_STRING_SVTOOLS_HTML_definstance},  HTML_DEFINSTANCE_ON},
    {{OOO_STRING_SVTOOLS_HTML_strike},        HTML_STRIKE_ON},

    {{OOO_STRING_SVTOOLS_HTML_bgsound},     HTML_BGSOUND},
    {{OOO_STRING_SVTOOLS_HTML_comment2},        HTML_COMMENT2_ON},
    {{OOO_STRING_SVTOOLS_HTML_marquee},         HTML_MARQUEE_ON},
    {{OOO_STRING_SVTOOLS_HTML_plaintext2},    HTML_PLAINTEXT2_ON},

    {{OOO_STRING_SVTOOLS_HTML_sdfield},     HTML_SDFIELD_ON}
};


extern "C"
{

static int SAL_CALL HTMLKeyCompare( const void *pFirst, const void *pSecond)
{
    HTML_TokenEntry* pFirstEntry = (HTML_TokenEntry*)pFirst;
    HTML_TokenEntry* pSecondEntry = (HTML_TokenEntry*)pSecond;
    int nRet = 0;
    if( -1 == pFirstEntry->nToken )
    {
        if( -1 == pSecondEntry->nToken )
            nRet = pFirstEntry->pUToken->compareTo( *pSecondEntry->pUToken );
        else
            nRet = pFirstEntry->pUToken->compareToAscii( pSecondEntry->sToken );
    }
    else
    {
        if( -1 == pSecondEntry->nToken )
            nRet = -1 * pSecondEntry->pUToken->compareToAscii( pFirstEntry->sToken );
        else
            nRet = strcmp( pFirstEntry->sToken, pSecondEntry->sToken );
    }

    return nRet;
}

}

int GetHTMLToken( const OUString& rName )
{
    if( !bSortKeyWords )
    {
        qsort( (void*) aHTMLTokenTab,
                sizeof( aHTMLTokenTab ) / sizeof( HTML_TokenEntry ),
                sizeof( HTML_TokenEntry ),
                HTMLKeyCompare );
        bSortKeyWords = true;
    }

    int nRet = 0;

    if( !rName.compareTo( OOO_STRING_SVTOOLS_HTML_comment, 3  ) )
        return HTML_COMMENT;

    void* pFound;
    HTML_TokenEntry aSrch;
    aSrch.pUToken = &rName;
    aSrch.nToken = -1;

    pFound = bsearch( (sal_Char *) &aSrch,
                      (void*) aHTMLTokenTab,
                      sizeof( aHTMLTokenTab ) / sizeof( HTML_TokenEntry ),
                      sizeof( HTML_TokenEntry ),
                      HTMLKeyCompare );
    if( 0 != pFound )
        nRet = ((HTML_TokenEntry*)pFound)->nToken;
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

// Netscape has some in upper case...
    {{OOO_STRING_SVTOOLS_HTML_C_LT},             60},
    {{OOO_STRING_SVTOOLS_HTML_C_GT},             62},
    {{OOO_STRING_SVTOOLS_HTML_C_AMP},        38},
    {{OOO_STRING_SVTOOLS_HTML_C_QUOT},       34},
    {{OOO_STRING_SVTOOLS_HTML_S_COPY},      169},
    {{OOO_STRING_SVTOOLS_HTML_S_REG},       174},

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
    HTML_CharEntry* pFirstEntry = (HTML_CharEntry*)pFirst;
    HTML_CharEntry* pSecondEntry = (HTML_CharEntry*)pSecond;
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
        qsort( (void*) aHTMLCharNameTab,
                sizeof( aHTMLCharNameTab ) / sizeof( HTML_CharEntry ),
                sizeof( HTML_CharEntry ),
                HTMLCharNameCompare );
        bSortCharKeyWords = true;
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

// Flag: Options table has already been sorted
static bool bSortOptionKeyWords = false;

static HTML_TokenEntry aHTMLOptionTab[] = {

// Attributes without value
    {{OOO_STRING_SVTOOLS_HTML_O_box},       HTML_O_BOX},
    {{OOO_STRING_SVTOOLS_HTML_O_checked},   HTML_O_CHECKED},
    {{OOO_STRING_SVTOOLS_HTML_O_compact},   HTML_O_COMPACT},
    {{OOO_STRING_SVTOOLS_HTML_O_continue},  HTML_O_CONTINUE},
    {{OOO_STRING_SVTOOLS_HTML_O_controls},  HTML_O_CONTROLS}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_declare},   HTML_O_DECLARE},
    {{OOO_STRING_SVTOOLS_HTML_O_disabled},  HTML_O_DISABLED},
    {{OOO_STRING_SVTOOLS_HTML_O_folded},        HTML_O_FOLDED}, // Netscape internal
    {{OOO_STRING_SVTOOLS_HTML_O_ismap},     HTML_O_ISMAP},
    {{OOO_STRING_SVTOOLS_HTML_O_mayscript}, HTML_O_MAYSCRIPT},
    {{OOO_STRING_SVTOOLS_HTML_O_multiple},  HTML_O_MULTIPLE},
    {{OOO_STRING_SVTOOLS_HTML_O_noflow},        HTML_O_NOFLOW},
    {{OOO_STRING_SVTOOLS_HTML_O_nohref},        HTML_O_NOHREF}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_noresize},  HTML_O_NORESIZE}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_noshade},   HTML_O_NOSHADE}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_nowrap},        HTML_O_NOWRAP},
    {{OOO_STRING_SVTOOLS_HTML_O_plain},     HTML_O_PLAIN},
    {{OOO_STRING_SVTOOLS_HTML_O_sdfixed},   HTML_O_SDFIXED},
    {{OOO_STRING_SVTOOLS_HTML_O_selected},      HTML_O_SELECTED},
    {{OOO_STRING_SVTOOLS_HTML_O_shapes},    HTML_O_SHAPES},

// Attributes with a string value
    {{OOO_STRING_SVTOOLS_HTML_O_above},     HTML_O_ABOVE},
    {{OOO_STRING_SVTOOLS_HTML_O_accept},        HTML_O_ACCEPT},
    {{OOO_STRING_SVTOOLS_HTML_O_accesskey}, HTML_O_ACCESSKEY},
    {{OOO_STRING_SVTOOLS_HTML_O_add_date},  HTML_O_ADD_DATE}, // Netscape internal
    {{OOO_STRING_SVTOOLS_HTML_O_alt},       HTML_O_ALT},
    {{OOO_STRING_SVTOOLS_HTML_O_axes},      HTML_O_AXES},
    {{OOO_STRING_SVTOOLS_HTML_O_axis},      HTML_O_AXIS},
    {{OOO_STRING_SVTOOLS_HTML_O_below},     HTML_O_BELOW},
    {{OOO_STRING_SVTOOLS_HTML_O_char},      HTML_O_CHAR}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_class},     HTML_O_CLASS},
    {{OOO_STRING_SVTOOLS_HTML_O_clip},      HTML_O_CLIP},
    {{OOO_STRING_SVTOOLS_HTML_O_code},      HTML_O_CODE}, // HotJava
    {{OOO_STRING_SVTOOLS_HTML_O_codetype},  HTML_O_CODETYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_colspec},   HTML_O_COLSPEC},
    {{OOO_STRING_SVTOOLS_HTML_O_content},   HTML_O_CONTENT},
    {{OOO_STRING_SVTOOLS_HTML_O_coords},        HTML_O_COORDS}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_dp},            HTML_O_DP},
    {{OOO_STRING_SVTOOLS_HTML_O_enctype},   HTML_O_ENCTYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_error},     HTML_O_ERROR},
    {{OOO_STRING_SVTOOLS_HTML_O_face},      HTML_O_FACE}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_frameborder}, HTML_O_FRAMEBORDER}, // IExplorer 3.0
    {{OOO_STRING_SVTOOLS_HTML_O_httpequiv}, HTML_O_HTTPEQUIV},
    {{OOO_STRING_SVTOOLS_HTML_O_language},  HTML_O_LANGUAGE}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_last_modified}, HTML_O_LAST_MODIFIED}, // Netscape internal
    {{OOO_STRING_SVTOOLS_HTML_O_last_visit},    HTML_O_LAST_VISIT}, // Netscape internal
    {{OOO_STRING_SVTOOLS_HTML_O_md},            HTML_O_MD},
    {{OOO_STRING_SVTOOLS_HTML_O_n},         HTML_O_N},
    {{OOO_STRING_SVTOOLS_HTML_O_name},      HTML_O_NAME},
    {{OOO_STRING_SVTOOLS_HTML_O_notation},  HTML_O_NOTATION},
    {{OOO_STRING_SVTOOLS_HTML_O_prompt},        HTML_O_PROMPT},
    {{OOO_STRING_SVTOOLS_HTML_O_shape},     HTML_O_SHAPE},
    {{OOO_STRING_SVTOOLS_HTML_O_standby},   HTML_O_STANDBY},
    {{OOO_STRING_SVTOOLS_HTML_O_style},     HTML_O_STYLE},
    {{OOO_STRING_SVTOOLS_HTML_O_title},     HTML_O_TITLE},
    {{OOO_STRING_SVTOOLS_HTML_O_value},     HTML_O_VALUE},
    {{OOO_STRING_SVTOOLS_HTML_O_SDval},     HTML_O_SDVAL}, // StarDiv NumberValue
    {{OOO_STRING_SVTOOLS_HTML_O_SDnum},     HTML_O_SDNUM}, // StarDiv NumberFormat
    {{OOO_STRING_SVTOOLS_HTML_O_sdlibrary}, HTML_O_SDLIBRARY},
    {{OOO_STRING_SVTOOLS_HTML_O_sdmodule},  HTML_O_SDMODULE},

// Attributes with a SGML identifier value
    {{OOO_STRING_SVTOOLS_HTML_O_from},      HTML_O_FROM},
    {{OOO_STRING_SVTOOLS_HTML_O_id},            HTML_O_ID},
    {{OOO_STRING_SVTOOLS_HTML_O_target},        HTML_O_TARGET}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_to},            HTML_O_TO},
    {{OOO_STRING_SVTOOLS_HTML_O_until},     HTML_O_UNTIL},

// Attributes with an URI value
    {{OOO_STRING_SVTOOLS_HTML_O_action},        HTML_O_ACTION},
    {{OOO_STRING_SVTOOLS_HTML_O_archive},       HTML_O_ARCHIVE},
    {{OOO_STRING_SVTOOLS_HTML_O_background},    HTML_O_BACKGROUND},
    {{OOO_STRING_SVTOOLS_HTML_O_classid},   HTML_O_CLASSID},
    {{OOO_STRING_SVTOOLS_HTML_O_codebase},  HTML_O_CODEBASE}, // HotJava
    {{OOO_STRING_SVTOOLS_HTML_O_data},      HTML_O_DATA},
    {{OOO_STRING_SVTOOLS_HTML_O_dynsrc},        HTML_O_DYNSRC}, // IExplorer 3.0b1
    {{OOO_STRING_SVTOOLS_HTML_O_dynsync},   HTML_O_DYNSYNC}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_imagemap},  HTML_O_IMAGEMAP},
    {{OOO_STRING_SVTOOLS_HTML_O_href},      HTML_O_HREF},
    {{OOO_STRING_SVTOOLS_HTML_O_lowsrc},        HTML_O_LOWSRC}, // Netscape 3.0
    {{OOO_STRING_SVTOOLS_HTML_O_script},        HTML_O_SCRIPT},
    {{OOO_STRING_SVTOOLS_HTML_O_src},       HTML_O_SRC},
    {{OOO_STRING_SVTOOLS_HTML_O_usemap},        HTML_O_USEMAP}, // Netscape 2.0

// Attributes with entity name value
    {{OOO_STRING_SVTOOLS_HTML_O_dingbat},   HTML_O_DINGBAT},
    {{OOO_STRING_SVTOOLS_HTML_O_sym},       HTML_O_SYM},

// Attributes with a color value (all Netscape versions)
    {{OOO_STRING_SVTOOLS_HTML_O_alink},     HTML_O_ALINK},
    {{OOO_STRING_SVTOOLS_HTML_O_bgcolor},   HTML_O_BGCOLOR},
    {{OOO_STRING_SVTOOLS_HTML_O_bordercolor}, HTML_O_BORDERCOLOR}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_bordercolorlight}, HTML_O_BORDERCOLORLIGHT}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_bordercolordark}, HTML_O_BORDERCOLORDARK}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_color},     HTML_O_COLOR},
    {{OOO_STRING_SVTOOLS_HTML_O_link},      HTML_O_LINK},
    {{OOO_STRING_SVTOOLS_HTML_O_text},      HTML_O_TEXT},
    {{OOO_STRING_SVTOOLS_HTML_O_vlink},     HTML_O_VLINK},

// Attributes with a numerical value
    {{OOO_STRING_SVTOOLS_HTML_O_border},        HTML_O_BORDER},
    {{OOO_STRING_SVTOOLS_HTML_O_cellspacing},HTML_O_CELLSPACING}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_cellpadding},HTML_O_CELLPADDING}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_charoff},   HTML_O_CHAROFF}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_colspan},   HTML_O_COLSPAN},
    {{OOO_STRING_SVTOOLS_HTML_O_framespacing}, HTML_O_FRAMESPACING}, // IExplorer 3.0
    {{OOO_STRING_SVTOOLS_HTML_O_gutter},        HTML_O_GUTTER}, // Netscape 3.0b5
    {{OOO_STRING_SVTOOLS_HTML_O_indent},        HTML_O_INDENT},
    {{OOO_STRING_SVTOOLS_HTML_O_height},        HTML_O_HEIGHT},
    {{OOO_STRING_SVTOOLS_HTML_O_hspace},        HTML_O_HSPACE}, // Netscape
    {{OOO_STRING_SVTOOLS_HTML_O_left},      HTML_O_LEFT},
    {{OOO_STRING_SVTOOLS_HTML_O_leftmargin},    HTML_O_LEFTMARGIN}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_loop},      HTML_O_LOOP}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_marginheight},HTML_O_MARGINHEIGHT}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_marginwidth},HTML_O_MARGINWIDTH}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_max},       HTML_O_MAX},
    {{OOO_STRING_SVTOOLS_HTML_O_maxlength}, HTML_O_MAXLENGTH},
    {{OOO_STRING_SVTOOLS_HTML_O_min},       HTML_O_MIN},
    {{OOO_STRING_SVTOOLS_HTML_O_pagex},     HTML_O_PAGEX},
    {{OOO_STRING_SVTOOLS_HTML_O_pagey},     HTML_O_PAGEY},
    {{OOO_STRING_SVTOOLS_HTML_O_pointsize}, HTML_O_POINTSIZE},
    {{OOO_STRING_SVTOOLS_HTML_O_rowspan},   HTML_O_ROWSPAN},
    {{OOO_STRING_SVTOOLS_HTML_O_scrollamount}, HTML_O_SCROLLAMOUNT}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_scrolldelay}, HTML_O_SCROLLDELAY}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_seqnum},        HTML_O_SEQNUM},
    {{OOO_STRING_SVTOOLS_HTML_O_skip},      HTML_O_SKIP},
    {{OOO_STRING_SVTOOLS_HTML_O_span},      HTML_O_SPAN}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_tabindex},  HTML_O_TABINDEX},
    {{OOO_STRING_SVTOOLS_HTML_O_top},       HTML_O_TOP},
    {{OOO_STRING_SVTOOLS_HTML_O_topmargin}, HTML_O_TOPMARGIN}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_vspace},        HTML_O_VSPACE}, // Netscape
    {{OOO_STRING_SVTOOLS_HTML_O_weight},        HTML_O_WEIGHT},
    {{OOO_STRING_SVTOOLS_HTML_O_width},     HTML_O_WIDTH},
    {{OOO_STRING_SVTOOLS_HTML_O_x},         HTML_O_X},
    {{OOO_STRING_SVTOOLS_HTML_O_y},         HTML_O_Y},
    {{OOO_STRING_SVTOOLS_HTML_O_zindex},        HTML_O_ZINDEX},

// Attributes with enum values
    {{OOO_STRING_SVTOOLS_HTML_O_bgproperties}, HTML_O_BGPROPERTIES}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_behavior},  HTML_O_BEHAVIOR}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_clear},     HTML_O_CLEAR},
    {{OOO_STRING_SVTOOLS_HTML_O_dir},       HTML_O_DIR}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_direction},     HTML_O_DIRECTION}, // IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_format},        HTML_O_FORMAT},
    {{OOO_STRING_SVTOOLS_HTML_O_frame},     HTML_O_FRAME}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_lang},      HTML_O_LANG},
    {{OOO_STRING_SVTOOLS_HTML_O_method},        HTML_O_METHOD},
    {{OOO_STRING_SVTOOLS_HTML_O_palette},   HTML_O_PALETTE},
    {{OOO_STRING_SVTOOLS_HTML_O_rel},       HTML_O_REL},
    {{OOO_STRING_SVTOOLS_HTML_O_rev},       HTML_O_REV},
    {{OOO_STRING_SVTOOLS_HTML_O_rules},     HTML_O_RULES}, // HTML 3 Table Model Draft
    {{OOO_STRING_SVTOOLS_HTML_O_scrolling}, HTML_O_SCROLLING}, // Netscape 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_sdreadonly},    HTML_O_SDREADONLY},
    {{OOO_STRING_SVTOOLS_HTML_O_subtype},   HTML_O_SUBTYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_type},      HTML_O_TYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_valign},        HTML_O_VALIGN},
    {{OOO_STRING_SVTOOLS_HTML_O_valuetype}, HTML_O_VALUETYPE},
    {{OOO_STRING_SVTOOLS_HTML_O_wrap},      HTML_O_WRAP},
    {{OOO_STRING_SVTOOLS_HTML_O_visibility},    HTML_O_VISIBILITY},

// Attributes with script code value
    {{OOO_STRING_SVTOOLS_HTML_O_onblur},        HTML_O_ONBLUR}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onchange},  HTML_O_ONCHANGE}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onclick},   HTML_O_ONCLICK}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onfocus},   HTML_O_ONFOCUS}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onload},    HTML_O_ONLOAD}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onmouseover}, HTML_O_ONMOUSEOVER}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onreset},   HTML_O_ONRESET}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onselect},  HTML_O_ONSELECT}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onsubmit},  HTML_O_ONSUBMIT}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onunload},  HTML_O_ONUNLOAD}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onabort},   HTML_O_ONABORT}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onerror},   HTML_O_ONERROR}, // JavaScript
    {{OOO_STRING_SVTOOLS_HTML_O_onmouseout},    HTML_O_ONMOUSEOUT}, // JavaScript

    {{OOO_STRING_SVTOOLS_HTML_O_SDonblur},      HTML_O_SDONBLUR}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonchange},    HTML_O_SDONCHANGE}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonclick},         HTML_O_SDONCLICK}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonfocus},         HTML_O_SDONFOCUS}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonload},      HTML_O_SDONLOAD}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonmouseover},     HTML_O_SDONMOUSEOVER}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonreset},     HTML_O_SDONRESET}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonselect},        HTML_O_SDONSELECT}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonsubmit},        HTML_O_SDONSUBMIT}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonunload},        HTML_O_SDONUNLOAD}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonabort},     HTML_O_SDONABORT}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonerror},         HTML_O_SDONERROR}, // StarBasic
    {{OOO_STRING_SVTOOLS_HTML_O_SDonmouseout},  HTML_O_SDONMOUSEOUT}, // StarBasic

// Attributes with context sensitive values
    {{OOO_STRING_SVTOOLS_HTML_O_align},     HTML_O_ALIGN},
    {{OOO_STRING_SVTOOLS_HTML_O_cols},      HTML_O_COLS}, // Netscape 2.0 vs HTML 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_rows},      HTML_O_ROWS}, // Netscape 2.0 vs HTML 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_size},      HTML_O_SIZE},
    {{OOO_STRING_SVTOOLS_HTML_O_start},     HTML_O_START}, // Netscape 2.0 vs IExplorer 2.0
    {{OOO_STRING_SVTOOLS_HTML_O_units},     HTML_O_UNITS}
};

int GetHTMLOption( const OUString& rName )
{
    if( !bSortOptionKeyWords )
    {
        qsort( (void*) aHTMLOptionTab,
                sizeof( aHTMLOptionTab ) / sizeof( HTML_TokenEntry ),
                sizeof( HTML_TokenEntry ),
                HTMLKeyCompare );
        bSortOptionKeyWords = true;
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
    { { "ALICEBLUE" }, 0x00f0f8ffUL },
    { { "ANTIQUEWHITE" }, 0x00faebd7UL },
    { { "AQUA" }, 0x0000ffffUL },
    { { "AQUAMARINE" }, 0x007fffd4UL },
    { { "AZURE" }, 0x00f0ffffUL },
    { { "BEIGE" }, 0x00f5f5dcUL },
    { { "BISQUE" }, 0x00ffe4c4UL },
    { { "BLACK" }, 0x00000000UL },
    { { "BLANCHEDALMOND" }, 0x00ffebcdUL },
    { { "BLUE" }, 0x000000ffUL },
    { { "BLUEVIOLET" }, 0x008a2be2UL },
    { { "BROWN" }, 0x00a52a2aUL },
    { { "BURLYWOOD" }, 0x00deb887UL },
    { { "CADETBLUE" }, 0x005f9ea0UL },
    { { "CHARTREUSE" }, 0x007fff00UL },
    { { "CHOCOLATE" }, 0x00d2691eUL },
    { { "CORAL" }, 0x00ff7f50UL },
    { { "CORNFLOWERBLUE" }, 0x006495edUL },
    { { "CORNSILK" }, 0x00fff8dcUL },
    { { "CRIMSON" }, 0x00dc143cUL },
    { { "CYAN" }, 0x0000ffffUL },
    { { "DARKBLUE" }, 0x0000008bUL },
    { { "DARKCYAN" }, 0x00008b8bUL },
    { { "DARKGOLDENROD" }, 0x00b8860bUL },
    { { "DARKGRAY" }, 0x00a9a9a9UL },
    { { "DARKGREEN" }, 0x00006400UL },
    { { "DARKKHAKI" }, 0x00bdb76bUL },
    { { "DARKMAGENTA" }, 0x008b008bUL },
    { { "DARKOLIVEGREEN" }, 0x00556b2fUL },
    { { "DARKORANGE" }, 0x00ff8c00UL },
    { { "DARKORCHID" }, 0x009932ccUL },
    { { "DARKRED" }, 0x008b0000UL },
    { { "DARKSALMON" }, 0x00e9967aUL },
    { { "DARKSEAGREEN" }, 0x008fbc8fUL },
    { { "DARKSLATEBLUE" }, 0x00483d8bUL },
    { { "DARKSLATEGRAY" }, 0x002f4f4fUL },
    { { "DARKTURQUOISE" }, 0x0000ced1UL },
    { { "DARKVIOLET" }, 0x009400d3UL },
    { { "DEEPPINK" }, 0x00ff1493UL },
    { { "DEEPSKYBLUE" }, 0x0000bfffUL },
    { { "DIMGRAY" }, 0x00696969UL },
    { { "DODGERBLUE" }, 0x001e90ffUL },
    { { "FIREBRICK" }, 0x00b22222UL },
    { { "FLORALWHITE" }, 0x00fffaf0UL },
    { { "FORESTGREEN" }, 0x00228b22UL },
    { { "FUCHSIA" }, 0x00ff00ffUL },
    { { "GAINSBORO" }, 0x00dcdcdcUL },
    { { "GHOSTWHITE" }, 0x00f8f8ffUL },
    { { "GOLD" }, 0x00ffd700UL },
    { { "GOLDENROD" }, 0x00daa520UL },
    { { "GRAY" }, 0x00808080UL },
    { { "GREEN" }, 0x00008000UL },
    { { "GREENYELLOW" }, 0x00adff2fUL },
    { { "HONEYDEW" }, 0x00f0fff0UL },
    { { "HOTPINK" }, 0x00ff69b4UL },
    { { "INDIANRED" }, 0x00cd5c5cUL },
    { { "INDIGO" }, 0x004b0082UL },
    { { "IVORY" }, 0x00fffff0UL },
    { { "KHAKI" }, 0x00f0e68cUL },
    { { "LAVENDER" }, 0x00e6e6faUL },
    { { "LAVENDERBLUSH" }, 0x00fff0f5UL },
    { { "LAWNGREEN" }, 0x007cfc00UL },
    { { "LEMONCHIFFON" }, 0x00fffacdUL },
    { { "LIGHTBLUE" }, 0x00add8e6UL },
    { { "LIGHTCORAL" }, 0x00f08080UL },
    { { "LIGHTCYAN" }, 0x00e0ffffUL },
    { { "LIGHTGOLDENRODYELLOW" }, 0x00fafad2UL },
    { { "LIGHTGREEN" }, 0x0090ee90UL },
    { { "LIGHTGREY" }, 0x00d3d3d3UL },
    { { "LIGHTPINK" }, 0x00ffb6c1UL },
    { { "LIGHTSALMON" }, 0x00ffa07aUL },
    { { "LIGHTSEAGREEN" }, 0x0020b2aaUL },
    { { "LIGHTSKYBLUE" }, 0x0087cefaUL },
    { { "LIGHTSLATEGRAY" }, 0x00778899UL },
    { { "LIGHTSTEELBLUE" }, 0x00b0c4deUL },
    { { "LIGHTYELLOW" }, 0x00ffffe0UL },
    { { "LIME" }, 0x0000ff00UL },
    { { "LIMEGREEN" }, 0x0032cd32UL },
    { { "LINEN" }, 0x00faf0e6UL },
    { { "MAGENTA" }, 0x00ff00ffUL },
    { { "MAROON" }, 0x00800000UL },
    { { "MEDIUMAQUAMARINE" }, 0x0066cdaaUL },
    { { "MEDIUMBLUE" }, 0x000000cdUL },
    { { "MEDIUMORCHID" }, 0x00ba55d3UL },
    { { "MEDIUMPURPLE" }, 0x009370dbUL },
    { { "MEDIUMSEAGREEN" }, 0x003cb371UL },
    { { "MEDIUMSLATEBLUE" }, 0x007b68eeUL },
    { { "MEDIUMSPRINGGREEN" }, 0x0000fa9aUL },
    { { "MEDIUMTURQUOISE" }, 0x0048d1ccUL },
    { { "MEDIUMVIOLETRED" }, 0x00c71585UL },
    { { "MIDNIGHTBLUE" }, 0x00191970UL },
    { { "MINTCREAM" }, 0x00f5fffaUL },
    { { "MISTYROSE" }, 0x00ffe4e1UL },
    { { "MOCCASIN" }, 0x00ffe4b5UL },
    { { "NAVAJOWHITE" }, 0x00ffdeadUL },
    { { "NAVY" }, 0x00000080UL },
    { { "OLDLACE" }, 0x00fdf5e6UL },
    { { "OLIVE" }, 0x00808000UL },
    { { "OLIVEDRAB" }, 0x006b8e23UL },
    { { "ORANGE" }, 0x00ffa500UL },
    { { "ORANGERED" }, 0x00ff4500UL },
    { { "ORCHID" }, 0x00da70d6UL },
    { { "PALEGOLDENROD" }, 0x00eee8aaUL },
    { { "PALEGREEN" }, 0x0098fb98UL },
    { { "PALETURQUOISE" }, 0x00afeeeeUL },
    { { "PALEVIOLETRED" }, 0x00db7093UL },
    { { "PAPAYAWHIP" }, 0x00ffefd5UL },
    { { "PEACHPUFF" }, 0x00ffdab9UL },
    { { "PERU" }, 0x00cd853fUL },
    { { "PINK" }, 0x00ffc0cbUL },
    { { "PLUM" }, 0x00dda0ddUL },
    { { "POWDERBLUE" }, 0x00b0e0e6UL },
    { { "PURPLE" }, 0x00800080UL },
    { { "RED" }, 0x00ff0000UL },
    { { "ROSYBROWN" }, 0x00bc8f8fUL },
    { { "ROYALBLUE" }, 0x004169e1UL },
    { { "SADDLEBROWN" }, 0x008b4513UL },
    { { "SALMON" }, 0x00fa8072UL },
    { { "SANDYBROWN" }, 0x00f4a460UL },
    { { "SEAGREEN" }, 0x002e8b57UL },
    { { "SEASHELL" }, 0x00fff5eeUL },
    { { "SIENNA" }, 0x00a0522dUL },
    { { "SILVER" }, 0x00c0c0c0UL },
    { { "SKYBLUE" }, 0x0087ceebUL },
    { { "SLATEBLUE" }, 0x006a5acdUL },
    { { "SLATEGRAY" }, 0x00708090UL },
    { { "SNOW" }, 0x00fffafaUL },
    { { "SPRINGGREEN" }, 0x0000ff7fUL },
    { { "STEELBLUE" }, 0x004682b4UL },
    { { "TAN" }, 0x00d2b48cUL },
    { { "TEAL" }, 0x00008080UL },
    { { "THISTLE" }, 0x00d8bfd8UL },
    { { "TOMATO" }, 0x00ff6347UL },
    { { "TURQUOISE" }, 0x0040e0d0UL },
    { { "VIOLET" }, 0x00ee82eeUL },
    { { "WHEAT" }, 0x00f5deb3UL },
    { { "WHITE" }, 0x00ffffffUL },
    { { "WHITESMOKE" }, 0x00f5f5f5UL },
    { { "YELLOW" }, 0x00ffff00UL },
    { { "YELLOWGREEN" }, 0x009acd32UL }
};

extern "C"
{

static int SAL_CALL HTMLColorNameCompare( const void *pFirst, const void *pSecond)
{
    HTML_ColorEntry* pFirstEntry = (HTML_ColorEntry*)pFirst;
    HTML_ColorEntry* pSecondEntry = (HTML_ColorEntry*)pSecond;
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
        qsort( (void*) aHTMLColorNameTab,
                sizeof( aHTMLColorNameTab ) / sizeof( HTML_ColorEntry ),
                sizeof( HTML_ColorEntry ),
                HTMLColorNameCompare );
        bSortColorKeyWords = true;
    }

    sal_uInt32 nRet = HTML_NO_COLOR;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
