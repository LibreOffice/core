/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef CONFIG_HXX_INCLUDED
#define CONFIG_HXX_INCLUDED

#ifdef _MSC_VER
#pragma warning (disable : 4786 4503 4917)
#endif

#ifndef OS2
#include <tchar.h>
#endif

#ifdef _AMD64_
#define MODULE_NAME TEXT("shlxthdl_x64.dll")
#define MODULE_NAME_FILTER TEXT("ooofilt_x64.dll")
#else
#define MODULE_NAME TEXT("shlxthdl.dll")
#define MODULE_NAME_FILTER TEXT("ooofilt.dll")
#endif

#define COLUMN_HANDLER_DESCRIPTIVE_NAME    TEXT("Apache OpenOffice Column Handler")
#define INFOTIP_HANDLER_DESCRIPTIVE_NAME   TEXT("Apache OpenOffice Infotip Handler")
#define PROPSHEET_HANDLER_DESCRIPTIVE_NAME TEXT("Apache OpenOffice Property Sheet Handler")
#define THUMBVIEWER_HANDLER_DESCRIPTIVAE_NAME TEXT("Apache OpenOffice Thumbnail Viewer")

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
#define META_INFO_OLE_OBJECTS           L"object-count"
#define META_INFO_PARAGRAPHS            L"paragraph-count"
#define META_INFO_WORDS                 L"word-count"
#define META_INFO_CHARACTERS            L"character-count"
#define META_INFO_ROWS                  L"row-count"
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
