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

namespace rtl {
    class OUString;
};
enum class HtmlOptionId;

// search the char for the CharName
sal_Unicode GetHTMLCharName( const rtl::OUString& rName );

// search the TokenID for the token
SVT_DLLPUBLIC int GetHTMLToken( const rtl::OUString& rName );

// search the TokenId for an attribute token
HtmlOptionId GetHTMLOption( const rtl::OUString& rName );

// search the 24-bit color for a color name (not found = ULONG_MAX)
SVT_DLLPUBLIC sal_uInt32 GetHTMLColor( const rtl::OUString& rName );

// always starting from 256 on, greater than a char
const int HTML_TOKEN_START      = 0x100;
const int HTML_TOKEN_ONOFF      = 0x200;
const int HTML_TOKEN_MICROSOFT  = 0x1000;

enum HTML_TOKEN_IDS
{
    HTML_TEXTTOKEN                      = HTML_TOKEN_START,
    HTML_SINGLECHAR,
    HTML_NEWPARA,
    HTML_TABCHAR,
    HTML_RAWDATA,
    HTML_LINEFEEDCHAR,

    // these will only be turned on
    HTML_AREA, // Netscape 2.0
    HTML_BASE, // HTML 3.0
    HTML_COMMENT,
    HTML_DOCTYPE,
    HTML_EMBED, // Netscape 2.0            ignore </EMBED>
    HTML_HORZRULE,                      // ignore </HR>
    HTML_IMAGE,                         // ignore </IMG>
    HTML_INPUT,                         // ignore </INPUT>
    HTML_LINEBREAK,                     // </BR> -> <BR>
    HTML_LINK, // HTML 3.0
    HTML_META, // HTML 3.0                 ignore </META>
    HTML_OPTION,                        // ignore </OPTION>
    HTML_PARAM, // HotJava
    HTML_SPACER, // Netscape 3.0b5      // ignore </SPACER>

    // tokens recognised using HTML character
    HTML_NONBREAKSPACE,
    HTML_SOFTHYPH,

    // these will be turned back off,
    //  the off value is always located behind (+1) !!
    HTML_ABBREVIATION_ON = HTML_TOKEN_ONOFF, // HTML 3.0
    HTML_ABBREVIATION_OFF, // HTML 3.0
    HTML_ACRONYM_ON, // HTML 3.0
    HTML_ACRONYM_OFF, // HTML 3.0
    HTML_ADDRESS_ON,
    HTML_ADDRESS_OFF,
    HTML_ANCHOR_ON,
    HTML_ANCHOR_OFF,
    HTML_APPLET_ON, // HotJava
    HTML_APPLET_OFF, // HotJava
    HTML_AUTHOR_ON, // HTML 3.0
    HTML_AUTHOR_OFF, // HTML 3.0
    HTML_BANNER_ON, // HTML 3.0
    HTML_BANNER_OFF, // HTML 3.0
    HTML_BASEFONT_ON, // Netscape
    HTML_BASEFONT_OFF, // Netscape
    HTML_BIGPRINT_ON, // HTML 3.0
    HTML_BIGPRINT_OFF, // HTML 3.0
    HTML_BLINK_ON, // Netscape
    HTML_BLINK_OFF, // Netscape
    HTML_BLOCKQUOTE30_ON, // HTML 3.0
    HTML_BLOCKQUOTE30_OFF, // HTML 3.0
    HTML_BLOCKQUOTE_ON,
    HTML_BLOCKQUOTE_OFF,
    HTML_BODY_ON,
    HTML_BODY_OFF,
    HTML_BOLD_ON,
    HTML_BOLD_OFF,
    HTML_CAPTION_ON, // HTML 3.0
    HTML_CAPTION_OFF, // HTML 3.0
    HTML_CENTER_ON, // Netscape
    HTML_CENTER_OFF,  // Netscape
    HTML_CITIATION_ON,
    HTML_CITIATION_OFF,
    HTML_CODE_ON,
    HTML_CODE_OFF,
    HTML_COL_ON, // HTML3 Table Model Draft
    HTML_COL_OFF, // HTML3 Table Model Draft
    HTML_COLGROUP_ON, // HTML3 Table Model Draft
    HTML_COLGROUP_OFF, // HTML3 Table Model Draft
    HTML_CREDIT_ON, // HTML 3.0
    HTML_CREDIT_OFF, // HTML 3.0
    HTML_DD_ON,
    HTML_DD_OFF,
    HTML_DEFLIST_ON,
    HTML_DEFLIST_OFF,
    HTML_DELETEDTEXT_ON, // HTML 3.0
    HTML_DELETEDTEXT_OFF, // HTML 3.0
    HTML_DIRLIST_ON,
    HTML_DIRLIST_OFF,
    HTML_DIVISION_ON, // HTML 3.0
    HTML_DIVISION_OFF, // HTML 3.0
    HTML_DT_ON,
    HTML_DT_OFF,
    HTML_EMPHASIS_ON,
    HTML_EMPHASIS_OFF,
    HTML_FIGURE_ON, // HTML 3.0
    HTML_FIGURE_OFF, // HTML 3.0
    HTML_FONT_ON, // Netscape
    HTML_FONT_OFF, // Netscape
    HTML_FOOTNOTE_ON, // HTML 3.0
    HTML_FOOTNOTE_OFF, // HTML 3.0
    HTML_FORM_ON,
    HTML_FORM_OFF,
    HTML_FRAME_ON, // Netscape 2.0
    HTML_FRAME_OFF, // Netscape 2.0
    HTML_FRAMESET_ON, // Netscape 2.0
    HTML_FRAMESET_OFF, // Netscape 2.0
    HTML_HEAD1_ON,
    HTML_HEAD1_OFF,
    HTML_HEAD2_ON,
    HTML_HEAD2_OFF,
    HTML_HEAD3_ON,
    HTML_HEAD3_OFF,
    HTML_HEAD4_ON,
    HTML_HEAD4_OFF,
    HTML_HEAD5_ON,
    HTML_HEAD5_OFF,
    HTML_HEAD6_ON,
    HTML_HEAD6_OFF,
    HTML_HEAD_ON,
    HTML_HEAD_OFF,
    HTML_HTML_ON,
    HTML_HTML_OFF,
    HTML_IFRAME_ON, // IE 3.0b2
    HTML_IFRAME_OFF, // IE 3.0b2
    HTML_INSERTEDTEXT_ON, // HTML 3.0
    HTML_INSERTEDTEXT_OFF, // HTML 3.0
    HTML_ITALIC_ON,
    HTML_ITALIC_OFF,
    HTML_KEYBOARD_ON,
    HTML_KEYBOARD_OFF,
    HTML_LANGUAGE_ON, // HTML 3.0
    HTML_LANGUAGE_OFF, // HTML 3.0
    HTML_LISTHEADER_ON, // HTML 3.0
    HTML_LISTHEADER_OFF, // HTML 3.0
    HTML_LI_ON,
    HTML_LI_OFF,
    HTML_MAP_ON, // Netscape 2.0
    HTML_MAP_OFF, // Netscape 2.0
    HTML_MENULIST_ON,
    HTML_MENULIST_OFF,
    HTML_MULTICOL_ON, // Netscape 3.0b5
    HTML_MULTICOL_OFF, // Netscape 3.0b5
    HTML_NOBR_ON, // Netscape
    HTML_NOBR_OFF, // Netscape
    HTML_NOEMBED_ON, // Netscape 2.0
    HTML_NOEMBED_OFF, // Netscape 2.0
    HTML_NOFRAMES_ON, // Netscape 2.0
    HTML_NOFRAMES_OFF, // Netscape 2.0
    HTML_NOSCRIPT_ON, // Netscape 2.0
    HTML_NOSCRIPT_OFF, // Netscape 3.0
    HTML_NOTE_ON, // HTML 3.0
    HTML_NOTE_OFF, // HTML 3.0
    HTML_OBJECT_ON, // HotJava
    HTML_OBJECT_OFF, // HotJava
    HTML_ORDERLIST_ON,
    HTML_ORDERLIST_OFF,
    HTML_PARABREAK_ON,
    HTML_PARABREAK_OFF,
    HTML_PERSON_ON, // HTML 3.0
    HTML_PERSON_OFF, // HTML 3.0
    HTML_PLAINTEXT_ON, // HTML 3.0
    HTML_PLAINTEXT_OFF, // HTML 3.0
    HTML_PREFORMTXT_ON,
    HTML_PREFORMTXT_OFF,
    HTML_SAMPLE_ON,
    HTML_SAMPLE_OFF,
    HTML_SCRIPT_ON, // HTML 3.2
    HTML_SCRIPT_OFF,    // HTML 3.2
    HTML_SELECT_ON,
    HTML_SELECT_OFF,
    HTML_SHORTQUOTE_ON, // HTML 3.0
    HTML_SHORTQUOTE_OFF, // HTML 3.0
    HTML_SMALLPRINT_ON, // HTML 3.0
    HTML_SMALLPRINT_OFF, // HTML 3.0
    HTML_SPAN_ON, // Style Sheets
    HTML_SPAN_OFF, // Style Sheets
    HTML_STRIKETHROUGH_ON, // HTML 3.0
    HTML_STRIKETHROUGH_OFF, // HTML 3.0
    HTML_STRONG_ON,
    HTML_STRONG_OFF,
    HTML_STYLE_ON, // HTML 3.0
    HTML_STYLE_OFF, // HTML 3.0
    HTML_SUBSCRIPT_ON, // HTML 3.0
    HTML_SUBSCRIPT_OFF, // HTML 3.0
    HTML_SUPERSCRIPT_ON, // HTML 3.0
    HTML_SUPERSCRIPT_OFF, // HTML 3.0
    HTML_TABLE_ON, // HTML 3.0
    HTML_TABLE_OFF, // HTML 3.0
    HTML_TABLEDATA_ON, // HTML 3.0
    HTML_TABLEDATA_OFF, // HTML 3.0
    HTML_TABLEHEADER_ON, // HTML 3.0
    HTML_TABLEHEADER_OFF, // HTML 3.0
    HTML_TABLEROW_ON, // HTML 3.0
    HTML_TABLEROW_OFF, // HTML 3.0
    HTML_TBODY_ON, // HTML3 Table Model Draft
    HTML_TBODY_OFF, // HTML3 Table Model Draft
    HTML_TELETYPE_ON,
    HTML_TELETYPE_OFF,
    HTML_TEXTAREA_ON,
    HTML_TEXTAREA_OFF,
    HTML_TFOOT_ON, // HTML3 Table Model Draft
    HTML_TFOOT_OFF, // HTML3 Table Model Draft
    HTML_THEAD_ON, // HTML3 Table Model Draft
    HTML_THEAD_OFF, // HTML3 Table Model Draft
    HTML_TITLE_ON,
    HTML_TITLE_OFF,
    HTML_UNDERLINE_ON,
    HTML_UNDERLINE_OFF,
    HTML_UNORDERLIST_ON,
    HTML_UNORDERLIST_OFF,
    HTML_VARIABLE_ON,
    HTML_VARIABLE_OFF,

    // obsolete features
    HTML_XMP_ON,
    HTML_XMP_OFF,
    HTML_LISTING_ON,
    HTML_LISTING_OFF,

    // proposed features
    HTML_DEFINSTANCE_ON,
    HTML_DEFINSTANCE_OFF,
    HTML_STRIKE_ON,
    HTML_STRIKE_OFF,

    HTML_UNKNOWNCONTROL_ON,
    HTML_UNKNOWNCONTROL_OFF,

    HTML_COMMENT2_ON = HTML_TOKEN_MICROSOFT|HTML_TOKEN_ONOFF, // HTML 2.0 ?
    HTML_COMMENT2_OFF, // HTML 2.0 ?
    HTML_MARQUEE_ON,
    HTML_MARQUEE_OFF,
    HTML_PLAINTEXT2_ON, // HTML 2.0 ?
    HTML_PLAINTEXT2_OFF, // HTML 2.0 ?

    HTML_SDFIELD_ON,
    HTML_SDFIELD_OFF
};

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
