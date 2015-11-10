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

#ifndef INCLUDED_SOT_FORMATS_HXX
#define INCLUDED_SOT_FORMATS_HXX

#include <tools/solar.h>

// - predefined formats -
// Do NOT change the order of these values as the implementation depends on them!
enum class SotClipboardFormatId : sal_uLong
{
// standard formats for that Copy/Paste methods exist
    STRING                 = 1,
    BITMAP                 = 2,
    GDIMETAFILE            = 3,
    PRIVATE                = 4,
    SIMPLE_FILE            = 5,
    FILE_LIST              = 6,
// further formats (only via CopyData/PasteData)
    RTF                    = 10,

    NONE                   = 0,
    ONLY_USED_IN_SW        = 8, // the SW module essentially creates its own ID's and this is one of them
    DRAWING                = 11,
    SVXB                   = 12,
    SVIM                   = 13,
    XFA                    = 14,
    EDITENGINE             = 15,
    INTERNALLINK_STATE     = 16,
    SOLK                   = 17,
    NETSCAPE_BOOKMARK      = 18,
    TREELISTBOX            = 19,
    NATIVE                 = 20,
    OWNERLINK              = 21,
    STARSERVER             = 22,
    STAROBJECT             = 23,
    APPLETOBJECT           = 24,
    PLUGIN_OBJECT          = 25,
    STARWRITER_30          = 26,
    STARWRITER_40          = 27,
    STARWRITER_50          = 28,
    STARWRITERWEB_40       = 29,
    STARWRITERWEB_50       = 30,
    STARWRITERGLOB_40      = 31,
    STARWRITERGLOB_50      = 32,
    STARDRAW               = 33,
    STARDRAW_40            = 34,
    STARIMPRESS_50         = 35,
    STARDRAW_50            = 36,
    STARCALC               = 37,
    STARCALC_40            = 38,
    STARCALC_50            = 39,
    STARCHART              = 40,
    STARCHART_40           = 41,
    STARCHART_50           = 42,
    STARIMAGE              = 43,
    STARIMAGE_40           = 44,
    STARIMAGE_50           = 45,
    STARMATH               = 46,
    STARMATH_40            = 47,
    STARMATH_50            = 48,
    STAROBJECT_PAINTDOC    = 49,
    FILLED_AREA            = 50,
    HTML                   = 51,
    HTML_SIMPLE            = 52,
    CHAOS                  = 53,
    CNT_MSGATTACHFILE      = 54,
    BIFF_5                 = 55,
    BIFF__5                = 56,
    SYLK                   = 57,
    SYLK_BIGCAPS           = 58,
    LINK                   = 59,
    DIF                    = 60,
    STARDRAW_TABBAR        = 61,
    SONLK                  = 62,
    MSWORD_DOC             = 63,
    STAR_FRAMESET_DOC      = 64,
    OFFICE_DOC             = 65,
    NOTES_DOCINFO          = 66,
    NOTES_HNOTE            = 67,
    NOTES_NATIVE           = 68,
    SFX_DOC                = 69,
    EVDF                   = 70,
    ESDF                   = 71,
    IDF                    = 72,
    EFTP                   = 73,
    EFD                    = 74,
    SVX_FORMFIELDEXCH      = 75,
    EXTENDED_TABBAR        = 76,
    SBA_DATAEXCHANGE       = 77,
    SBA_FIELDDATAEXCHANGE  = 78,
    SBA_PRIVATE_URL        = 79,
    SBA_TABED              = 80,
    SBA_TABID              = 81,
    SBA_JOIN               = 82,
    OBJECTDESCRIPTOR       = 83,
    LINKSRCDESCRIPTOR      = 84,
    EMBED_SOURCE           = 85,
    LINK_SOURCE            = 86,
    EMBEDDED_OBJ           = 87,
    FILECONTENT            = 88,
    FILEGRPDESCRIPTOR      = 89,
    FILENAME               = 90,
    SD_OLE                 = 91,
    EMBEDDED_OBJ_OLE       = 92,
    EMBED_SOURCE_OLE       = 93,
    OBJECTDESCRIPTOR_OLE   = 94,
    LINKSRCDESCRIPTOR_OLE  = 95,
    LINK_SOURCE_OLE        = 96,
    SBA_CTRLDATAEXCHANGE   = 97,
    OUTPLACE_OBJ           = 98,
    CNT_OWN_CLIP           = 99,
    INET_IMAGE             = 100,
    NETSCAPE_IMAGE         = 101,
    SBA_FORMEXCHANGE       = 102,
    SBA_REPORTEXCHANGE     = 103,
    UNIFORMRESOURCELOCATOR = 104,
    STARCHARTDOCUMENT_50   = 105,
    GRAPHOBJ               = 106,
    STARWRITER_60          = 107,
    STARWRITERWEB_60       = 108,
    STARWRITERGLOB_60      = 109,
    STARDRAW_60            = 110,
    STARIMPRESS_60         = 111,
    STARCALC_60            = 112,
    STARCHART_60           = 113,
    STARMATH_60            = 114,
    WMF                    = 115,
    DBACCESS_QUERY         = 116,
    DBACCESS_TABLE         = 117,
    DBACCESS_COMMAND       = 118,
    DIALOG_60              = 119,
    EMF                    = 120,
    BIFF_8                 = 121,
    BMP                    = 122,
    HTML_NO_COMMENT        = 123,
    STARWRITER_8           = 124,
    STARWRITERWEB_8        = 125,
    STARWRITERGLOB_8       = 126,
    STARDRAW_8             = 127,
    STARIMPRESS_8          = 128,
    STARCALC_8             = 129,
    STARCHART_8            = 130,
    STARMATH_8             = 131,
    XFORMS                 = 132,
    STARWRITER_8_TEMPLATE  = 133,
    STARDRAW_8_TEMPLATE    = 134,
    STARIMPRESS_8_TEMPLATE = 135,
    STARCALC_8_TEMPLATE    = 136,
    STARCHART_8_TEMPLATE   = 137,
    STARMATH_8_TEMPLATE    = 138,
    STARBASE_8             = 139,
    HC_GDIMETAFILE         = 140,
    PNG                    = 141,
    STARWRITERGLOB_8_TEMPLATE = 142,
    MATHML                 = 143,
    // the point at which we start allocating "runtime" format IDs
    USER_END  = MATHML
};

/** Make it easier to iterate over format IDs */
inline SotClipboardFormatId& operator++(SotClipboardFormatId& v)
{
    v = static_cast<SotClipboardFormatId>(static_cast<sal_uLong>(v) + 1);
    return v;
}

#define SOT_FORMAT_SYSTEM_START   SotClipboardFormatId::NONE

#endif // INCLUDED_SOT_FORMATS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
