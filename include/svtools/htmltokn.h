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

#ifndef _HTMLTOKN_H
#define _HTMLTOKN_H

#include "svtools/svtdllapi.h"
#include <sal/types.h>
#include <tools/solar.h>

namespace rtl {
    class OUString;
};

// suche das Char zu dem CharNamen
sal_Unicode GetHTMLCharName( const rtl::OUString& rName );

// suche die TokenID zu dem Token
SVT_DLLPUBLIC int GetHTMLToken( const rtl::OUString& rName );

// suche die TokenId zu einemm Attribut-Token
int GetHTMLOption( const rtl::OUString& rName );

// suche die 24-bit-Farbe zu einem Farbnamen (nicht gefunden = ULONG_MAX)
SVT_DLLPUBLIC sal_uInt32 GetHTMLColor( const rtl::OUString& rName );

// beginnen immer ab 256, groesser als ein char
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

    // diese werden nur eingeschaltet
    HTML_AREA, // Netscape 2.0
    HTML_BASE, // HTML 3.0
    HTML_COMMENT,
    HTML_DOCTYPE,
    HTML_EMBED, // Netscape 2.0            </EMBED> ignorieren
    HTML_FIGUREOVERLAY, // HTML 3.0
    HTML_HORZRULE,                      // </HR> ignorieren
    HTML_HORZTAB, // HTML 3.0
    HTML_IMAGE,                         // </IMG> ignorieren
    HTML_INPUT,                         // </INPUT> ignorieren
    HTML_ISINDEX, // HTML 3.0
    HTML_LINEBREAK,                     // </BR> -> <BR>
    HTML_LINK, // HTML 3.0
    HTML_META, // HTML 3.0                 </META> ignorieren
    HTML_NEXTID, // HTML 3.0
    HTML_OF, // HTML 3.0
    HTML_OPTION,                        // </OPTION> ignorieren
    HTML_PARAM, // HotJava
    HTML_RANGE, // HTML 3.0
    HTML_SPACER, // Netscape 3.0b5      // </SPACER> ignorieren
    HTML_WBR, // Netscape

    // Tokens, die ueber HTML-Charakter erkannt werden
    HTML_NONBREAKSPACE,
    HTML_SOFTHYPH,

    // diese werden wieder abgeschaltet,
    //  der off-Wert liegt immer dahinter (+1) !!
    HTML_ABBREVIATION_ON = HTML_TOKEN_ONOFF, // HTML 3.0
    HTML_ABBREVIATION_OFF, // HTML 3.0
    HTML_ABOVE_ON, // HTML 3.0
    HTML_ABOVE_OFF, // HTML 3.0
    HTML_ACRONYM_ON, // HTML 3.0
    HTML_ACRONYM_OFF, // HTML 3.0
    HTML_ADDRESS_ON,
    HTML_ADDRESS_OFF,
    HTML_ANCHOR_ON,
    HTML_ANCHOR_OFF,
    HTML_APPLET_ON, // HotJava
    HTML_APPLET_OFF, // HotJava
    HTML_ARRAY_ON, // HTML 3.0
    HTML_ARRAY_OFF, // HTML 3.0
    HTML_AUTHOR_ON, // HTML 3.0
    HTML_AUTHOR_OFF, // HTML 3.0
    HTML_BANNER_ON, // HTML 3.0
    HTML_BANNER_OFF, // HTML 3.0
    HTML_BAR_ON, // HTML 3.0
    HTML_BAR_OFF, // HTML 3.0
    HTML_BASEFONT_ON, // Netscape
    HTML_BASEFONT_OFF, // Netscape
    HTML_BELOW_ON, // HTML 3.0
    HTML_BELOW_OFF, // HTML 3.0
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
    HTML_BOLDTEXT_ON, // HTML 3.0
    HTML_BOLDTEXT_OFF, // HTML 3.0
    HTML_BOLD_ON,
    HTML_BOLD_OFF,
    HTML_BOX_ON, // HTML 3.0
    HTML_BOX_OFF, // HTML 3.0
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
    HTML_DOT_ON, // HTML 3.0
    HTML_DOT_OFF, // HTML 3.0
    HTML_DOUBLEDOT_ON, // HTML 3.0
    HTML_DOUBLEDOT_OFF, // HTML 3.0
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
    HTML_HAT_ON, // HTML 3.0
    HTML_HAT_OFF, // HTML 3.0
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
    HTML_ILAYER_ON,
    HTML_ILAYER_OFF,
    HTML_INSERTEDTEXT_ON, // HTML 3.0
    HTML_INSERTEDTEXT_OFF, // HTML 3.0
    HTML_ITALIC_ON,
    HTML_ITALIC_OFF,
    HTML_ITEM_ON, // HTML 3.0
    HTML_ITEM_OFF, // HTML 3.0
    HTML_KEYBOARD_ON,
    HTML_KEYBOARD_OFF,
    HTML_LAYER_ON,
    HTML_LAYER_OFF,
    HTML_LANGUAGE_ON, // HTML 3.0
    HTML_LANGUAGE_OFF, // HTML 3.0
    HTML_LISTHEADER_ON, // HTML 3.0
    HTML_LISTHEADER_OFF, // HTML 3.0
    HTML_LI_ON,
    HTML_LI_OFF,
    HTML_MAP_ON, // Netscape 2.0
    HTML_MAP_OFF, // Netscape 2.0
    HTML_MATH_ON, // HTML 3.0
    HTML_MATH_OFF, // HTML 3.0
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
    HTML_ROOT_ON, // HTML 3.0
    HTML_ROOT_OFF, // HTML 3.0
    HTML_ROW_ON, // HTML 3.0
    HTML_ROW_OFF, // HTML 3.0
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
    HTML_SQUAREROOT_ON, // HTML 3.0
    HTML_SQUAREROOT_OFF, // HTML 3.0
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
    HTML_TEXTFLOW_ON, // HTML 3.2
    HTML_TEXTFLOW_OFF, // HTML 3.2
    HTML_TEXT_ON, // HTML 3.0
    HTML_TEXT_OFF, // HTML 3.0
    HTML_TFOOT_ON, // HTML3 Table Model Draft
    HTML_TFOOT_OFF, // HTML3 Table Model Draft
    HTML_THEAD_ON, // HTML3 Table Model Draft
    HTML_THEAD_OFF, // HTML3 Table Model Draft
    HTML_TILDE_ON, // HTML 3.0
    HTML_TILDE_OFF, // HTML 3.0
    HTML_TITLE_ON,
    HTML_TITLE_OFF,
    HTML_UNDERLINE_ON,
    HTML_UNDERLINE_OFF,
    HTML_UNORDERLIST_ON,
    HTML_UNORDERLIST_OFF,
    HTML_VARIABLE_ON,
    HTML_VARIABLE_OFF,
    HTML_VECTOR_ON, // HTML 3.0
    HTML_VECTOR_OFF, // HTML 3.0

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

    HTML_BGSOUND = HTML_TOKEN_MICROSOFT|HTML_TOKEN_START,

    HTML_COMMENT2_ON = HTML_TOKEN_MICROSOFT|HTML_TOKEN_ONOFF, // HTML 2.0 ?
    HTML_COMMENT2_OFF, // HTML 2.0 ?
    HTML_MARQUEE_ON,
    HTML_MARQUEE_OFF,
    HTML_PLAINTEXT2_ON, // HTML 2.0 ?
    HTML_PLAINTEXT2_OFF, // HTML 2.0 ?

    HTML_SDFIELD_ON,
    HTML_SDFIELD_OFF
};

// HTML Attribut-Token (=Optionen)

// beginnen immer ab 256, groesser als ein char
const int HTML_OPTION_START = 0x100;

enum HTML_OPTION_IDS
{
HTML_OPTION_BOOL_START          = HTML_OPTION_START,

// Attribute ohne Wert
    HTML_O_BOX                  = HTML_OPTION_BOOL_START,
    HTML_O_CHECKED,
    HTML_O_COMPACT,
    HTML_O_CONTINUE,
    HTML_O_CONTROLS, // IExplorer 2.0
    HTML_O_DECLARE, // IExplorer 3.0b5
    HTML_O_DISABLED,
    HTML_O_FOLDED, // Netscape internal
    HTML_O_ISMAP,
    HTML_O_MAYSCRIPT, // Netcape 3.0
    HTML_O_MULTIPLE,
    HTML_O_NOFLOW,
    HTML_O_NOHREF, // Netscape
    HTML_O_NORESIZE, // Netscape 2.0
    HTML_O_NOSHADE, // Netscape
    HTML_O_NOWRAP,
    HTML_O_PLAIN,
    HTML_O_SDFIXED,
    HTML_O_SELECTED,
    HTML_O_SHAPES, // IExplorer 3.0b5
HTML_OPTION_BOOL_END,

// Attribute mit einem String als Wert
HTML_OPTION_STRING_START        = HTML_OPTION_BOOL_END,
    HTML_O_ABOVE                = HTML_OPTION_STRING_START,
    HTML_O_ACCEPT,
    HTML_O_ACCESSKEY,
    HTML_O_ADD_DATE, // Netscape internal
    HTML_O_ALT,
    HTML_O_AXES,
    HTML_O_AXIS,
    HTML_O_BELOW,
    HTML_O_CHAR, // HTML3 Table Model Draft
    HTML_O_CLASS,
    HTML_O_CLIP,
    HTML_O_CODE, // HotJava
    HTML_O_CODETYPE,
    HTML_O_COLSPEC,
    HTML_O_CONTENT,
    HTML_O_COORDS, // Netscape 2.0
    HTML_O_DP,
    HTML_O_ENCTYPE,
    HTML_O_ERROR,
    HTML_O_FACE, // IExplorer 2.0
    HTML_O_FRAMEBORDER, // IExplorer 3.0
    HTML_O_HTTPEQUIV,
    HTML_O_LANGUAGE, // JavaScript
    HTML_O_LAST_MODIFIED, // Netscape internal
    HTML_O_LAST_VISIT, // Netscape internal
    HTML_O_MD,
    HTML_O_N,
    HTML_O_NAME,
    HTML_O_NOTATION,
    HTML_O_PROMPT,
    HTML_O_SHAPE,
    HTML_O_STANDBY,
    HTML_O_STYLE, // Style Sheets
    HTML_O_TITLE,
    HTML_O_VALUE,
    HTML_O_SDVAL, // StarDiv NumberValue
    HTML_O_SDNUM, // StarDiv NumberFormat
    HTML_O_SDLIBRARY,
    HTML_O_SDMODULE,
HTML_OPTION_STRING_END,

// Attribute mit einem SGML-Identifier als Wert
HTML_OPTION_SGMLID_START        = HTML_OPTION_STRING_END,
    HTML_O_FROM                 = HTML_OPTION_SGMLID_START,
    HTML_O_ID,
    HTML_O_TARGET, // Netscape 2.0
    HTML_O_TO,
    HTML_O_UNTIL,
HTML_OPTION_SGMLID_END,

// Attribute mit einem URI als Wert
HTML_OPTION_URI_START           = HTML_OPTION_SGMLID_END,
    HTML_O_ACTION               = HTML_OPTION_URI_START,
    HTML_O_ARCHIVE,
    HTML_O_BACKGROUND,
    HTML_O_CLASSID,
    HTML_O_CODEBASE, // HotJava
    HTML_O_DATA,
    HTML_O_DYNSRC, // IExplorer 3.0
    HTML_O_DYNSYNC, // IExplorer 2.0
    HTML_O_IMAGEMAP,
    HTML_O_HREF,
    HTML_O_LOWSRC, // Netscape 3.0
    HTML_O_SCRIPT,
    HTML_O_SRC,
    HTML_O_USEMAP, // Netscape 2.0
HTML_OPTION_URI_END,

// Attribute mit Entity-Namen als Wert
HTML_OPTION_ENTITY_START        = HTML_OPTION_URI_END,
    HTML_O_DINGBAT              = HTML_OPTION_ENTITY_START,
    HTML_O_SYM,
HTML_OPTION_ENTITY_END,

// Attribute mit einer Farbe als Wert (alle Netscape)
HTML_OPTION_COLOR_START         = HTML_OPTION_ENTITY_END,
    HTML_O_ALINK                = HTML_OPTION_COLOR_START,
    HTML_O_BGCOLOR,
    HTML_O_BORDERCOLOR, // IExplorer 2.0
    HTML_O_BORDERCOLORLIGHT, // IExplorer 2.0
    HTML_O_BORDERCOLORDARK, // IExplorer 2.0
    HTML_O_COLOR,
    HTML_O_LINK,
    HTML_O_TEXT,
    HTML_O_VLINK,
HTML_OPTION_COLOR_END,

// Attribute mit einem numerischen Wert
HTML_OPTION_NUMBER_START        = HTML_OPTION_COLOR_END,
    HTML_O_BORDER               = HTML_OPTION_NUMBER_START,
    HTML_O_CELLSPACING, // HTML3 Table Model Draft
    HTML_O_CELLPADDING, // HTML3 Table Model Draft
    HTML_O_CHAROFF, // HTML3 Table Model Draft
    HTML_O_COLSPAN,
    HTML_O_FRAMESPACING, // IExplorer 3.0
    HTML_O_GUTTER, // Netscape 3.0b5
    HTML_O_INDENT,
    HTML_O_HEIGHT,
    HTML_O_HSPACE,          // Netscape
    HTML_O_LEFT,
    HTML_O_LEFTMARGIN, // IExplorer 2.0
    HTML_O_LOOP, // IExplorer 2.0
    HTML_O_MARGINWIDTH, // Netscape 2.0
    HTML_O_MARGINHEIGHT, // Netscape 2.0
    HTML_O_MAX,
    HTML_O_MAXLENGTH,
    HTML_O_MIN,
    HTML_O_PAGEX,
    HTML_O_PAGEY,
    HTML_O_POINTSIZE,
    HTML_O_ROWSPAN,
    HTML_O_SCROLLAMOUNT, // IExplorer 2.0
    HTML_O_SCROLLDELAY, // IExplorer 2.0
    HTML_O_SEQNUM,
    HTML_O_SKIP,
    HTML_O_SPAN, // HTML3 Table Model Draft
    HTML_O_TABINDEX,
    HTML_O_TOP,
    HTML_O_TOPMARGIN, // IExplorer 2.0
    HTML_O_VSPACE,          // Netscape
    HTML_O_WEIGHT,
    HTML_O_WIDTH,
    HTML_O_X,
    HTML_O_Y,
    HTML_O_ZINDEX,
HTML_OPTION_NUMBER_END,

// Attribute mit Enum-Werten
HTML_OPTION_ENUM_START          = HTML_OPTION_NUMBER_END,
    HTML_O_BEHAVIOR             = HTML_OPTION_ENUM_START, // IExplorer 2.0
    HTML_O_BGPROPERTIES, // IExplorer 2.0
    HTML_O_CLEAR,
    HTML_O_DIR,
    HTML_O_DIRECTION, // IExplorer 2.0
    HTML_O_FORMAT,
    HTML_O_FRAME, // HTML3 Table Model Draft
    HTML_O_LANG,
    HTML_O_METHOD,
    HTML_O_PALETTE,
    HTML_O_REL,
    HTML_O_REV,
    HTML_O_RULES, // HTML3 Table Model Draft
    HTML_O_SCROLLING, // Netscape 2.0
    HTML_O_SDREADONLY,
    HTML_O_SUBTYPE,
    HTML_O_TYPE,
    HTML_O_VALIGN,
    HTML_O_VALUETYPE,
    HTML_O_VISIBILITY,
    HTML_O_WRAP,
HTML_OPTION_ENUM_END,

// Attribute mit Script-Code als Wert
HTML_OPTION_SCRIPT_START        = HTML_OPTION_ENUM_END,
    HTML_O_ONABORT              = HTML_OPTION_SCRIPT_START, // JavaScript
    HTML_O_ONBLUR,      // JavaScript
    HTML_O_ONCHANGE,    // JavaScript
    HTML_O_ONCLICK,     // JavaScript
    HTML_O_ONERROR,     // JavaScript
    HTML_O_ONFOCUS,     // JavaScript
    HTML_O_ONLOAD,      // JavaScript
    HTML_O_ONMOUSEOUT,  // JavaScript
    HTML_O_ONMOUSEOVER, // JavaScript
    HTML_O_ONRESET,     // JavaScript
    HTML_O_ONSELECT,    // JavaScript
    HTML_O_ONSUBMIT,    // JavaScript
    HTML_O_ONUNLOAD,    // JavaScript

    HTML_O_SDONABORT,       // StarBasic
    HTML_O_SDONBLUR,        // StarBasic
    HTML_O_SDONCHANGE,      // StarBasic
    HTML_O_SDONCLICK,       // StarBasic
    HTML_O_SDONERROR,       // StarBasic
    HTML_O_SDONFOCUS,       // StarBasic
    HTML_O_SDONLOAD,        // StarBasic
    HTML_O_SDONMOUSEOUT,    // StarBasic
    HTML_O_SDONMOUSEOVER,   // StarBasic
    HTML_O_SDONRESET,       // StarBasic
    HTML_O_SDONSELECT,      // StarBasic
    HTML_O_SDONSUBMIT,      // StarBasic
    HTML_O_SDONUNLOAD,      // StarBasic
HTML_OPTION_SCRIPT_END,

// Attribute mit Kontext-abhaengigen Werten
HTML_OPTION_CONTEXT_START       = HTML_OPTION_SCRIPT_END,
    HTML_O_ALIGN                = HTML_OPTION_CONTEXT_START,
    HTML_O_COLS, // Netscape 2.0 vs HTML 2.0
    HTML_O_ROWS, // Netscape 2.0 vs HTML 2.0
    HTML_O_SIZE,
    HTML_O_START,
    HTML_O_UNITS,
HTML_OPTION_CONTEXT_END,

// eine unbekannte Option
HTML_O_UNKNOWN                  = HTML_OPTION_CONTEXT_END,
HTML_OPTION_END
};

#endif  // _HTMLTOKN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
