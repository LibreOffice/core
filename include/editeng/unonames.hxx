/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_EDITENG_UNONAMES_HXX
#define INCLUDED_EDITENG_UNONAMES_HXX

#include <rtl/ustring.hxx>

// Common across fields
inline constexpr OUString UNO_TC_PROP_ANCHOR = u"Anchor"_ustr;
inline constexpr OUString UNO_TC_PROP_TEXTFIELD_TYPE = u"TextFieldType"_ustr;
inline constexpr OUString UNO_TC_PROP_IS_FIXED = u"IsFixed"_ustr;
inline constexpr OUString UNO_TC_PROP_CURRENT_PRESENTATION = u"CurrentPresentation"_ustr;
inline constexpr OUString UNO_TC_PROP_NAME = u"Name"_ustr;
inline constexpr OUString UNO_TC_PROP_IS_FIXED_LANGUAGE = u"IsFixedLanguage"_ustr;
inline constexpr OUString UNO_TC_PROP_NUMFORMAT = u"NumberFormat"_ustr;

// Date & Time
inline constexpr OUString UNO_TC_PROP_IS_DATE = u"IsDate"_ustr;
inline constexpr OUString UNO_TC_PROP_DATE_TIME = u"DateTime"_ustr;

// URL
inline constexpr OUString UNO_TC_PROP_URL_FORMAT = u"Format"_ustr;
inline constexpr OUString UNO_TC_PROP_URL_REPRESENTATION = u"Representation"_ustr;
inline constexpr OUString UNO_TC_PROP_URL_TARGET = u"TargetFrame"_ustr;
inline constexpr OUString UNO_TC_PROP_URL = u"URL"_ustr;

// Table
inline constexpr OUString UNO_TC_PROP_TABLE_POSITION = u"TablePosition"_ustr;

// File
inline constexpr OUString UNO_TC_PROP_FILE_FORMAT = u"FileFormat"_ustr;

// Author
inline constexpr OUString UNO_TC_PROP_AUTHOR_CONTENT = u"Content"_ustr;
inline constexpr OUString UNO_TC_PROP_AUTHOR_FORMAT = u"AuthorFormat"_ustr;
inline constexpr OUString UNO_TC_PROP_AUTHOR_FULLNAME = u"FullName"_ustr;

// Measure
inline constexpr OUString UNO_TC_PROP_MEASURE_KIND = u"Kind"_ustr;

inline constexpr OUString UNO_TR_PROP_SELECTION = u"Selection"_ustr;

#endif // INCLUDED_EDITENG_UNONAMES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
