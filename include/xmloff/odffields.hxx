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

#ifndef INCLUDED_XMLOFF_ODFFIELDS_HXX
#define INCLUDED_XMLOFF_ODFFIELDS_HXX

#include <rtl/ustring.hxx>

inline constexpr OUString ODF_FORMTEXT = u"vnd.oasis.opendocument.field.FORMTEXT"_ustr;
#define ODF_FORMFIELD_DEFAULT_LENGTH 5

inline constexpr OUString ODF_FORMCHECKBOX = u"vnd.oasis.opendocument.field.FORMCHECKBOX"_ustr;
inline constexpr OUString ODF_FORMCHECKBOX_HELPTEXT = u"Checkbox_HelpText"_ustr;
inline constexpr OUString ODF_FORMCHECKBOX_RESULT = u"Checkbox_Checked"_ustr;

inline constexpr OUString ODF_FORMDROPDOWN = u"vnd.oasis.opendocument.field.FORMDROPDOWN"_ustr;
inline constexpr OUString ODF_FORMDROPDOWN_LISTENTRY = u"Dropdown_ListEntry"_ustr;
inline constexpr OUString ODF_FORMDROPDOWN_RESULT = u"Dropdown_Selected"_ustr;
#define ODF_FORMDROPDOWN_ENTRY_COUNT_LIMIT 25

inline constexpr OUString ODF_FORMDATE = u"vnd.oasis.opendocument.field.FORMDATE"_ustr;
inline constexpr OUString ODF_FORMDATE_DATEFORMAT
    = u"DateField_DateFormat"_ustr; // e.g. "MM.DD.YY";
inline constexpr OUString ODF_FORMDATE_DATEFORMAT_LANGUAGE
    = u"DateField_DateFormat_Language"_ustr; // e.g. "en-US", "hu-HU";
inline constexpr OUString ODF_FORMDATE_CURRENTDATE
    = u"DateField_CurrentDate"_ustr; // date string in a specific format
inline constexpr OUString ODF_FORMDATE_CURRENTDATE_FORMAT = u"YYYY-MM-DD"_ustr; // Coming from MSO
#define ODF_FORMDATE_CURRENTDATE_LANGUAGE LANGUAGE_ENGLISH_US

#define ODF_TOC "vnd.oasis.opendocument.field.TOC"

#define ODF_HYPERLINK "vnd.oasis.opendocument.field.HYPERLINK"

#define ODF_PAGEREF "vnd.oasis.opendocument.field.PAGEREF"

inline constexpr OUString ODF_UNHANDLED = u"vnd.oasis.opendocument.field.UNHANDLED"_ustr;
inline constexpr OUString ODF_OLE_PARAM = u"vnd.oasis.opendocument.field.ole"_ustr;
inline constexpr OUString ODF_ID_PARAM = u"vnd.oasis.opendocument.field.id"_ustr;
inline constexpr OUString ODF_CODE_PARAM = u"vnd.oasis.opendocument.field.code"_ustr;

#endif // INCLUDED_XMLOFF_ODFFIELDS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
