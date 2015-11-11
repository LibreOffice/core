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

#ifndef INCLUDED_SVTOOLS_SORES_HXX
#define INCLUDED_SVTOOLS_SORES_HXX

//  Strings
#define STR_ERROR_OBJNOCREATE                   32014
#define STR_ERROR_OBJNOCREATE_FROM_FILE         32015
#define STR_ERROR_OBJNOCREATE_PLUGIN            32018
#define STR_FURTHER_OBJECT                      32026
#define STR_UNKNOWN_SOURCE                      32027

#ifdef BMP_PLUGIN
#undef BMP_PLUGIN
#endif
#define BMP_PLUGIN                      32000

// Sot Format Strings
#define STR_FORMAT_START                        32100
#define STR_FORMAT_STRING                       (STR_FORMAT_START + 1)
#define STR_FORMAT_BITMAP                       (STR_FORMAT_START + 2)
#define STR_FORMAT_GDIMETAFILE                  (STR_FORMAT_START + 3)
#define STR_FORMAT_RTF                          (STR_FORMAT_START + 7)
#define STR_FORMAT_ID_DRAWING                   (STR_FORMAT_START + 8)
#define STR_FORMAT_ID_SVXB                      (STR_FORMAT_START + 9)
#define STR_FORMAT_ID_INTERNALLINK_STATE        (STR_FORMAT_START + 13)
#define STR_FORMAT_ID_SOLK                      (STR_FORMAT_START + 14)
#define STR_FORMAT_ID_NETSCAPE_BOOKMARK         (STR_FORMAT_START + 15)
#define STR_FORMAT_ID_STARSERVER                (STR_FORMAT_START + 19)
#define STR_FORMAT_ID_STAROBJECT                (STR_FORMAT_START + 20)
#define STR_FORMAT_ID_APPLETOBJECT              (STR_FORMAT_START + 21)
#define STR_FORMAT_ID_PLUGIN_OBJECT             (STR_FORMAT_START + 22)
#define STR_FORMAT_ID_STARWRITER_30             (STR_FORMAT_START + 23)
#define STR_FORMAT_ID_STARWRITER_40             (STR_FORMAT_START + 24)
#define STR_FORMAT_ID_STARWRITER_50             (STR_FORMAT_START + 25)
#define STR_FORMAT_ID_STARWRITERWEB_40          (STR_FORMAT_START + 26)
#define STR_FORMAT_ID_STARWRITERWEB_50          (STR_FORMAT_START + 27)
#define STR_FORMAT_ID_STARWRITERGLOB_40         (STR_FORMAT_START + 28)
#define STR_FORMAT_ID_STARWRITERGLOB_50         (STR_FORMAT_START + 29)
#define STR_FORMAT_ID_STARDRAW                  (STR_FORMAT_START + 30)
#define STR_FORMAT_ID_STARDRAW_40               (STR_FORMAT_START + 31)
#define STR_FORMAT_ID_STARIMPRESS_50            (STR_FORMAT_START + 32)
#define STR_FORMAT_ID_STARDRAW_50               (STR_FORMAT_START + 33)
#define STR_FORMAT_ID_STARCALC                  (STR_FORMAT_START + 34)
#define STR_FORMAT_ID_STARCALC_40               (STR_FORMAT_START + 35)
#define STR_FORMAT_ID_STARCALC_50               (STR_FORMAT_START + 36)
#define STR_FORMAT_ID_STARCHART                 (STR_FORMAT_START + 37)
#define STR_FORMAT_ID_STARCHART_40              (STR_FORMAT_START + 38)
#define STR_FORMAT_ID_STARCHART_50              (STR_FORMAT_START + 39)
#define STR_FORMAT_ID_STARIMAGE                 (STR_FORMAT_START + 40)
#define STR_FORMAT_ID_STARIMAGE_40              (STR_FORMAT_START + 41)
#define STR_FORMAT_ID_STARIMAGE_50              (STR_FORMAT_START + 42)
#define STR_FORMAT_ID_STARMATH                  (STR_FORMAT_START + 43)
#define STR_FORMAT_ID_STARMATH_40               (STR_FORMAT_START + 44)
#define STR_FORMAT_ID_STARMATH_50               (STR_FORMAT_START + 45)
#define STR_FORMAT_ID_STAROBJECT_PAINTDOC       (STR_FORMAT_START + 46)
#define STR_FORMAT_ID_HTML                      (STR_FORMAT_START + 48)
#define STR_FORMAT_ID_HTML_SIMPLE               (STR_FORMAT_START + 49)
#define STR_FORMAT_ID_BIFF_5                    (STR_FORMAT_START + 52)
#define STR_FORMAT_ID_BIFF_8                    (STR_FORMAT_START + 53)
#define STR_FORMAT_ID_SYLK                      (STR_FORMAT_START + 54)
#define STR_FORMAT_ID_LINK                      (STR_FORMAT_START + 56)
#define STR_FORMAT_ID_DIF                       (STR_FORMAT_START + 57)
#define STR_FORMAT_ID_MSWORD_DOC                (STR_FORMAT_START + 60)
#define STR_FORMAT_ID_STAR_FRAMESET_DOC         (STR_FORMAT_START + 61)
#define STR_FORMAT_ID_OFFICE_DOC                (STR_FORMAT_START + 62)
#define STR_FORMAT_ID_NOTES_DOCINFO             (STR_FORMAT_START + 63)
#define STR_FORMAT_ID_SFX_DOC                   (STR_FORMAT_START + 66)
#define STR_FORMAT_ID_FILEGRPDESCRIPTOR     (STR_FORMAT_START + 86)
#define STR_FORMAT_ID_STARCHARTDOCUMENT_50      (STR_FORMAT_START + 102)
#define STR_FORMAT_ID_GRAPHOBJ                  (STR_FORMAT_START + 103)
#define STR_FORMAT_ID_STARWRITER_60             (STR_FORMAT_START + 104)
#define STR_FORMAT_ID_STARWRITERWEB_60          (STR_FORMAT_START + 105)
#define STR_FORMAT_ID_STARWRITERGLOB_60         (STR_FORMAT_START + 106)
#define STR_FORMAT_ID_STARDRAW_60               (STR_FORMAT_START + 107)
#define STR_FORMAT_ID_STARIMPRESS_60            (STR_FORMAT_START + 108)
#define STR_FORMAT_ID_STARCALC_60               (STR_FORMAT_START + 109)
#define STR_FORMAT_ID_STARCHART_60              (STR_FORMAT_START + 110)
#define STR_FORMAT_ID_STARMATH_60               (STR_FORMAT_START + 111)
#define STR_FORMAT_ID_WMF                       (STR_FORMAT_START + 112)
#define STR_FORMAT_ID_DBACCESS_QUERY            (STR_FORMAT_START + 113)
#define STR_FORMAT_ID_DBACCESS_TABLE            (STR_FORMAT_START + 114)
#define STR_FORMAT_ID_DBACCESS_COMMAND          (STR_FORMAT_START + 115)
#define STR_FORMAT_ID_DIALOG_60                 (STR_FORMAT_START + 116)
#define STR_FORMAT_ID_HTML_NO_COMMENT           (STR_FORMAT_START + 119)
#define STR_FORMAT_END                          (STR_FORMAT_ID_HTML_NO_COMMENT)

#endif // INCLUDED_SVTOOLS_SORES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
