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

#ifndef INCLUDED_SVTOOLS_HTMLTOKN_H
#define INCLUDED_SVTOOLS_HTMLTOKN_H

#include <svtools/svtdllapi.h>
#include <sal/types.h>
#include <stdexcept>

namespace rtl {
    class OUString;
};
enum class HtmlOptionId;
enum class HtmlTokenId : sal_Int16;

// search the char for the CharName
sal_Unicode GetHTMLCharName( const rtl::OUString& rName );

// search the TokenID for the token
SVT_DLLPUBLIC HtmlTokenId GetHTMLToken( const rtl::OUString& rName );

// search the TokenId for an attribute token
HtmlOptionId GetHTMLOption( const rtl::OUString& rName );

// search the 24-bit color for a color name (not found = ULONG_MAX)
SVT_DLLPUBLIC sal_uInt32 GetHTMLColor( const rtl::OUString& rName );

enum class HtmlTokenId : sal_Int16
{
    INVALID = -1,
    NONE = 0,
// always starting from 256 on, greater than a char
    TEXTTOKEN                      = 0x100,
    SINGLECHAR,
    NEWPARA,
    TABCHAR,
    RAWDATA,
    LINEFEEDCHAR,

    // these will only be turned on
    AREA, // Netscape 2.0
    BASE, // HTML 3.0
    COMMENT,
    DOCTYPE,
    EMBED, // Netscape 2.0            ignore </EMBED>
    HORZRULE,                      // ignore </HR>
    IMAGE,                         // ignore </IMG>
    INPUT,                         // ignore </INPUT>
    LINEBREAK,                     // </BR> -> <BR>
    LINK, // HTML 3.0
    META, // HTML 3.0                 ignore </META>
    OPTION,                        // ignore </OPTION>
    PARAM, // HotJava
    SPACER, // Netscape 3.0b5      // ignore </SPACER>

    // tokens recognised using HTML character
    NONBREAKSPACE,
    SOFTHYPH,

    // these will be turned back off,
    //  the off value is always located behind (+1) !!
    ONOFF_START,
    ABBREVIATION_ON = ONOFF_START, // HTML 3.0
    ABBREVIATION_OFF, // HTML 3.0
    ACRONYM_ON, // HTML 3.0
    ACRONYM_OFF, // HTML 3.0
    ADDRESS_ON,
    ADDRESS_OFF,
    ANCHOR_ON,
    ANCHOR_OFF,
    APPLET_ON, // HotJava
    APPLET_OFF, // HotJava
    AUTHOR_ON, // HTML 3.0
    AUTHOR_OFF, // HTML 3.0
    BANNER_ON, // HTML 3.0
    BANNER_OFF, // HTML 3.0
    BASEFONT_ON, // Netscape
    BASEFONT_OFF, // Netscape
    BIGPRINT_ON, // HTML 3.0
    BIGPRINT_OFF, // HTML 3.0
    BLINK_ON, // Netscape
    BLINK_OFF, // Netscape
    BLOCKQUOTE30_ON, // HTML 3.0
    BLOCKQUOTE30_OFF, // HTML 3.0
    BLOCKQUOTE_ON,
    BLOCKQUOTE_OFF,
    BODY_ON,
    BODY_OFF,
    BOLD_ON,
    BOLD_OFF,
    CAPTION_ON, // HTML 3.0
    CAPTION_OFF, // HTML 3.0
    CENTER_ON, // Netscape
    CENTER_OFF,  // Netscape
    CITIATION_ON,
    CITIATION_OFF,
    CODE_ON,
    CODE_OFF,
    COL_ON, // HTML3 Table Model Draft
    COL_OFF, // HTML3 Table Model Draft
    COLGROUP_ON, // HTML3 Table Model Draft
    COLGROUP_OFF, // HTML3 Table Model Draft
    CREDIT_ON, // HTML 3.0
    CREDIT_OFF, // HTML 3.0
    DD_ON,
    DD_OFF,
    DEFLIST_ON,
    DEFLIST_OFF,
    DELETEDTEXT_ON, // HTML 3.0
    DELETEDTEXT_OFF, // HTML 3.0
    DIRLIST_ON,
    DIRLIST_OFF,
    DIVISION_ON, // HTML 3.0
    DIVISION_OFF, // HTML 3.0
    DT_ON,
    DT_OFF,
    EMPHASIS_ON,
    EMPHASIS_OFF,
    FIGURE_ON, // HTML 3.0
    FIGURE_OFF, // HTML 3.0
    FONT_ON, // Netscape
    FONT_OFF, // Netscape
    FOOTNOTE_ON, // HTML 3.0
    FOOTNOTE_OFF, // HTML 3.0
    FORM_ON,
    FORM_OFF,
    FRAME_ON, // Netscape 2.0
    FRAME_OFF, // Netscape 2.0
    FRAMESET_ON, // Netscape 2.0
    FRAMESET_OFF, // Netscape 2.0
    HEAD1_ON,
    HEAD1_OFF,
    HEAD2_ON,
    HEAD2_OFF,
    HEAD3_ON,
    HEAD3_OFF,
    HEAD4_ON,
    HEAD4_OFF,
    HEAD5_ON,
    HEAD5_OFF,
    HEAD6_ON,
    HEAD6_OFF,
    HEAD_ON,
    HEAD_OFF,
    HTML_ON,
    HTML_OFF,
    IFRAME_ON, // IE 3.0b2
    IFRAME_OFF, // IE 3.0b2
    INSERTEDTEXT_ON, // HTML 3.0
    INSERTEDTEXT_OFF, // HTML 3.0
    ITALIC_ON,
    ITALIC_OFF,
    KEYBOARD_ON,
    KEYBOARD_OFF,
    LANGUAGE_ON, // HTML 3.0
    LANGUAGE_OFF, // HTML 3.0
    LISTHEADER_ON, // HTML 3.0
    LISTHEADER_OFF, // HTML 3.0
    LI_ON,
    LI_OFF,
    MAP_ON, // Netscape 2.0
    MAP_OFF, // Netscape 2.0
    MENULIST_ON,
    MENULIST_OFF,
    MULTICOL_ON, // Netscape 3.0b5
    MULTICOL_OFF, // Netscape 3.0b5
    NOBR_ON, // Netscape
    NOBR_OFF, // Netscape
    NOEMBED_ON, // Netscape 2.0
    NOEMBED_OFF, // Netscape 2.0
    NOFRAMES_ON, // Netscape 2.0
    NOFRAMES_OFF, // Netscape 2.0
    NOSCRIPT_ON, // Netscape 2.0
    NOSCRIPT_OFF, // Netscape 3.0
    NOTE_ON, // HTML 3.0
    NOTE_OFF, // HTML 3.0
    OBJECT_ON, // HotJava
    OBJECT_OFF, // HotJava
    ORDERLIST_ON,
    ORDERLIST_OFF,
    PARABREAK_ON,
    PARABREAK_OFF,
    PERSON_ON, // HTML 3.0
    PERSON_OFF, // HTML 3.0
    PLAINTEXT_ON, // HTML 3.0
    PLAINTEXT_OFF, // HTML 3.0
    PREFORMTXT_ON,
    PREFORMTXT_OFF,
    SAMPLE_ON,
    SAMPLE_OFF,
    SCRIPT_ON, // HTML 3.2
    SCRIPT_OFF,    // HTML 3.2
    SELECT_ON,
    SELECT_OFF,
    SHORTQUOTE_ON, // HTML 3.0
    SHORTQUOTE_OFF, // HTML 3.0
    SMALLPRINT_ON, // HTML 3.0
    SMALLPRINT_OFF, // HTML 3.0
    SPAN_ON, // Style Sheets
    SPAN_OFF, // Style Sheets
    STRIKETHROUGH_ON, // HTML 3.0
    STRIKETHROUGH_OFF, // HTML 3.0
    STRONG_ON,
    STRONG_OFF,
    STYLE_ON, // HTML 3.0
    STYLE_OFF, // HTML 3.0
    SUBSCRIPT_ON, // HTML 3.0
    SUBSCRIPT_OFF, // HTML 3.0
    SUPERSCRIPT_ON, // HTML 3.0
    SUPERSCRIPT_OFF, // HTML 3.0
    TABLE_ON, // HTML 3.0
    TABLE_OFF, // HTML 3.0
    TABLEDATA_ON, // HTML 3.0
    TABLEDATA_OFF, // HTML 3.0
    TABLEHEADER_ON, // HTML 3.0
    TABLEHEADER_OFF, // HTML 3.0
    TABLEROW_ON, // HTML 3.0
    TABLEROW_OFF, // HTML 3.0
    TBODY_ON, // HTML3 Table Model Draft
    TBODY_OFF, // HTML3 Table Model Draft
    TELETYPE_ON,
    TELETYPE_OFF,
    TEXTAREA_ON,
    TEXTAREA_OFF,
    TFOOT_ON, // HTML3 Table Model Draft
    TFOOT_OFF, // HTML3 Table Model Draft
    THEAD_ON, // HTML3 Table Model Draft
    THEAD_OFF, // HTML3 Table Model Draft
    TITLE_ON,
    TITLE_OFF,
    UNDERLINE_ON,
    UNDERLINE_OFF,
    UNORDERLIST_ON,
    UNORDERLIST_OFF,
    VARIABLE_ON,
    VARIABLE_OFF,

    // obsolete features
    XMP_ON,
    XMP_OFF,
    LISTING_ON,
    LISTING_OFF,

    // proposed features
    DEFINSTANCE_ON,
    DEFINSTANCE_OFF,
    STRIKE_ON,
    STRIKE_OFF,

    UNKNOWNCONTROL_ON,
    UNKNOWNCONTROL_OFF,

    // Microsoft features
    COMMENT2_ON, // HTML 2.0 ?
    COMMENT2_OFF, // HTML 2.0 ?
    MARQUEE_ON,
    MARQUEE_OFF,
    PLAINTEXT2_ON, // HTML 2.0 ?
    PLAINTEXT2_OFF, // HTML 2.0 ?

    SDFIELD_ON,
    SDFIELD_OFF
};

constexpr bool isOffToken(HtmlTokenId nToken)
{
    return (nToken == HtmlTokenId::NONE || nToken >= HtmlTokenId::ONOFF_START)
      ? (1 & static_cast<int>(nToken))
      : throw std::logic_error("Assertion failed!"); // C++11 does not do assert in constexpr
}

constexpr HtmlTokenId getOnToken(HtmlTokenId nToken)
{
    return (nToken == HtmlTokenId::NONE || nToken >= HtmlTokenId::ONOFF_START)
      ? HtmlTokenId(~1 & static_cast<int>(nToken))
      : throw std::logic_error("Assertion failed!"); // C++11 does not do assert in constexpr
}

// HTML attribute token (=Options)

enum class HtmlOptionId
{
// always starting from 256 on, greater than a char
BOOL_START          = 0x100,

// attributes without value
    CHECKED              = BOOL_START,
    COMPACT,
    DECLARE, // IExplorer 3.0b5
    DISABLED,
    ISMAP,
    MAYSCRIPT, // Netscape 3.0
    MULTIPLE,
    NOHREF, // Netscape
    NORESIZE, // Netscape 2.0
    NOSHADE, // Netscape
    NOWRAP,
    SDFIXED,
    SELECTED,
BOOL_END,

// attributes with a string as value
STRING_START        = BOOL_END,
    ACCEPT               = STRING_START,
    ACCESSKEY,
    ALT,
    AXIS,
    CHAR, // HTML3 Table Model Draft
    CHARSET,
    CLASS,
    CODE, // HotJava
    CODETYPE,
    CONTENT,
    COORDS, // Netscape 2.0
    ENCTYPE,
    FACE, // IExplorer 2.0
    FRAMEBORDER, // IExplorer 3.0
    HTTPEQUIV,
    LANGUAGE, // JavaScript
    NAME,
    PROMPT,
    SHAPE,
    STANDBY,
    STYLE, // Style Sheets
    TITLE,
    VALUE,
    SDVAL, // StarDiv NumberValue
    SDNUM, // StarDiv NumberFormat
    SDLIBRARY,
    SDMODULE,
STRING_END,

// attributes with an SGML identifier as value
SGMLID_START        = STRING_END,
    ID                   = SGMLID_START,
    TARGET, // Netscape 2.0
    TO,
SGMLID_END,

// attributes with a URI as value
URI_START           = SGMLID_END,
    ACTION               = URI_START,
    ARCHIVE,
    BACKGROUND,
    CLASSID,
    CODEBASE, // HotJava
    DATA,
    HREF,
    SCRIPT,
    SRC,
    USEMAP, // Netscape 2.0
URI_END,

// attributes with a color as value (all Netscape)
COLOR_START         = URI_END,
    ALINK                = COLOR_START,
    BGCOLOR,
    BORDERCOLOR, // IExplorer 2.0
    BORDERCOLORLIGHT, // IExplorer 2.0
    BORDERCOLORDARK, // IExplorer 2.0
    COLOR,
    LINK,
    TEXT,
    VLINK,
COLOR_END,

// attributes with a numeric value
NUMBER_START        = COLOR_END,
    BORDER               = NUMBER_START,
    CELLSPACING, // HTML3 Table Model Draft
    CELLPADDING, // HTML3 Table Model Draft
    CHAROFF, // HTML3 Table Model Draft
    COLSPAN,
    FRAMESPACING, // IExplorer 3.0
    GUTTER, // Netscape 3.0b5
    HEIGHT,
    HSPACE,          // Netscape
    LEFT,
    LOOP, // IExplorer 2.0
    MARGINWIDTH, // Netscape 2.0
    MARGINHEIGHT, // Netscape 2.0
    MAXLENGTH,
    ROWSPAN,
    SCROLLAMOUNT, // IExplorer 2.0
    SCROLLDELAY, // IExplorer 2.0
    SPAN, // HTML3 Table Model Draft
    TABINDEX,
    VSPACE,          // Netscape
    WIDTH,
    ZINDEX,
NUMBER_END,

// attributes with Enum values
ENUM_START          = NUMBER_END,
    BEHAVIOR             = ENUM_START, // IExplorer 2.0
    CLEAR,
    DIR,
    DIRECTION, // IExplorer 2.0
    FORMAT,
    FRAME, // HTML3 Table Model Draft
    LANG,
    METHOD,
    REL,
    REV,
    RULES, // HTML3 Table Model Draft
    SCROLLING, // Netscape 2.0
    SDREADONLY,
    SUBTYPE,
    TYPE,
    VALIGN,
    VALUETYPE,
    WRAP,
ENUM_END,

// attributes with script code as value
SCRIPT_START        = ENUM_END,
    ONABORT              = SCRIPT_START, // JavaScript
    ONBLUR,      // JavaScript
    ONCHANGE,    // JavaScript
    ONCLICK,     // JavaScript
    ONERROR,     // JavaScript
    ONFOCUS,     // JavaScript
    ONLOAD,      // JavaScript
    ONMOUSEOUT,  // JavaScript
    ONMOUSEOVER, // JavaScript
    ONRESET,     // JavaScript
    ONSELECT,    // JavaScript
    ONSUBMIT,    // JavaScript
    ONUNLOAD,    // JavaScript

    SDONABORT,       // StarBasic
    SDONBLUR,        // StarBasic
    SDONCHANGE,      // StarBasic
    SDONCLICK,       // StarBasic
    SDONERROR,       // StarBasic
    SDONFOCUS,       // StarBasic
    SDONLOAD,        // StarBasic
    SDONMOUSEOUT,    // StarBasic
    SDONMOUSEOVER,   // StarBasic
    SDONRESET,       // StarBasic
    SDONSELECT,      // StarBasic
    SDONSUBMIT,      // StarBasic
    SDONUNLOAD,      // StarBasic
SCRIPT_END,

// attributes with context dependent values
CONTEXT_START       = SCRIPT_END,
    ALIGN                = CONTEXT_START,
    COLS, // Netscape 2.0 vs HTML 2.0
    ROWS, // Netscape 2.0 vs HTML 2.0
    SIZE,
    START,
CONTEXT_END,

// an unknown option
UNKNOWN                  = CONTEXT_END,
END
};

#endif // INCLUDED_SVTOOLS_HTMLTOKN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
