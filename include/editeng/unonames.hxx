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

// Common across fields
constexpr OUStringLiteral UNO_TC_PROP_ANCHOR = u"Anchor";
constexpr OUStringLiteral UNO_TC_PROP_TEXTFIELD_TYPE = u"TextFieldType";
constexpr OUStringLiteral UNO_TC_PROP_IS_FIXED = u"IsFixed";
constexpr OUStringLiteral UNO_TC_PROP_CURRENT_PRESENTATION = u"CurrentPresentation";
constexpr OUStringLiteral UNO_TC_PROP_NAME = u"Name";
#define UNO_TC_PROP_IS_FIXED_LANGUAGE    "IsFixedLanguage"
constexpr OUStringLiteral UNO_TC_PROP_NUMFORMAT = u"NumberFormat";
#define UNO_TC_PROP_USED                 "IsFieldUsed"
#define UNO_TC_PROP_DISPLAYED            "IsFieldDisplayed"

// Date & Time
constexpr OUStringLiteral UNO_TC_PROP_IS_DATE = u"IsDate";
constexpr OUStringLiteral UNO_TC_PROP_DATE_TIME = u"DateTime";

// URL
constexpr OUStringLiteral UNO_TC_PROP_URL_FORMAT = u"Format";
constexpr OUStringLiteral UNO_TC_PROP_URL_REPRESENTATION = u"Representation";
constexpr OUStringLiteral UNO_TC_PROP_URL_TARGET = u"TargetFrame";
constexpr OUStringLiteral UNO_TC_PROP_URL = u"URL";

// Table
constexpr OUStringLiteral UNO_TC_PROP_TABLE_POSITION = u"TablePosition";

// File
constexpr OUStringLiteral UNO_TC_PROP_FILE_FORMAT = u"FileFormat";

// Author
constexpr OUStringLiteral UNO_TC_PROP_AUTHOR_CONTENT = u"Content";
constexpr OUStringLiteral UNO_TC_PROP_AUTHOR_FORMAT = u"AuthorFormat";
constexpr OUStringLiteral UNO_TC_PROP_AUTHOR_FULLNAME = u"FullName";

// Measure
constexpr OUStringLiteral UNO_TC_PROP_MEASURE_KIND = u"Kind";

constexpr OUStringLiteral UNO_TR_PROP_SELECTION = u"Selection";

#endif // INCLUDED_EDITENG_UNONAMES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
