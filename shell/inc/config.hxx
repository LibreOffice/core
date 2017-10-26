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

#ifndef INCLUDED_SHELL_INC_INTERNAL_CONFIG_HXX
#define INCLUDED_SHELL_INC_INTERNAL_CONFIG_HXX

#ifdef _AMD64_
#define MODULE_NAME L"shlxthdl_x64.dll"
#else
#define MODULE_NAME L"shlxthdl.dll"
#endif

#define COLUMN_HANDLER_DESCRIPTIVE_NAME    L"LibreOffice Column Handler"
#define INFOTIP_HANDLER_DESCRIPTIVE_NAME   L"LibreOffice Infotip Handler"
#define PROPSHEET_HANDLER_DESCRIPTIVE_NAME L"LibreOffice Property Sheet Handler"
#define THUMBVIEWER_HANDLER_DESCRIPTIVE_NAME L"LibreOffice Thumbnail Viewer"

#define META_CONTENT_NAME               "meta.xml"
#define DOC_CONTENT_NAME                "content.xml"

#define EMPTY_STRING                    L""
#define SPACE                           L" "
#define LF                              L"\n"
#define META_INFO_TITLE                 L"title"
#define META_INFO_AUTHOR                L"initial-creator"
#define META_INFO_SUBJECT               L"subject"
#define META_INFO_KEYWORDS              L"keywords"
#define META_INFO_KEYWORD               L"keyword"
#define META_INFO_DESCRIPTION           L"description"

#define META_INFO_PAGES                 L"page-count"
#define META_INFO_TABLES                L"table-count"
#define META_INFO_DRAWS                 L"image-count"
#define META_INFO_OBJECTS               L"object-count"
#define META_INFO_PARAGRAPHS            L"paragraph-count"
#define META_INFO_WORDS                 L"word-count"
#define META_INFO_CHARACTERS            L"character-count"
#define META_INFO_CELLS                 L"cell-count"
#define META_INFO_DOCUMENT_STATISTIC    L"document-statistic"
#define META_INFO_MODIFIED              L"date"
#define META_INFO_DOCUMENT_NUMBER       L"editing-cycles"
#define META_INFO_EDITING_TIME          L"editing-duration"

#define META_INFO_LANGUAGE              L"language"
#define META_INFO_CREATOR               L"creator"
#define META_INFO_CREATION              L"creation-date"
#define META_INFO_GENERATOR             L"generator"


#define CONTENT_TEXT_A                           L"a"
#define CONTENT_TEXT_P                           L"p"
#define CONTENT_TEXT_H                           L"h"
#define CONTENT_TEXT_SPAN                        L"span"
#define CONTENT_TEXT_SEQUENCE                    L"sequence"
#define CONTENT_TEXT_BOOKMARK_REF                L"bookmark-ref"
#define CONTENT_TEXT_INDEX_TITLE_TEMPLATE        L"index-title-template"
#define CONTENT_TEXT_STYLENAME                   L"style-name"

#define CONTENT_STYLE_STYLE                      L"style"
#define CONTENT_STYLE_STYLE_NAME                 L"name"
#define CONTENT_STYLE_PROPERTIES                 L"properties"
#define CONTENT_TEXT_STYLE_PROPERTIES            L"text-properties"        // added for OASIS Open Office XML format.
#define CONTENT_STYLE_PROPERTIES_LANGUAGE        L"language"
#define CONTENT_STYLE_PROPERTIES_COUNTRY         L"country"
#define CONTENT_STYLE_PROPERTIES_LANGUAGEASIAN   L"language-asian"
#define CONTENT_STYLE_PROPERTIES_COUNTRYASIAN    L"country-asian"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
