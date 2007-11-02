/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: config.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:09:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIG_HXX_INCLUDED
#define CONFIG_HXX_INCLUDED

#ifdef _MSC_VER
#pragma warning (disable : 4786 4503 4917)
#endif

#ifndef OS2
#include <tchar.h>
#endif

#define MODULE_NAME TEXT("shlxthdl.dll")
#define MODULE_NAME_FILTER TEXT("ooofilt.dll")

#define COLUMN_HANDLER_DESCRIPTIVE_NAME    TEXT("OpenOffice.org Column Handler")
#define INFOTIP_HANDLER_DESCRIPTIVE_NAME   TEXT("OpenOffice.org Infotip Handler")
#define PROPSHEET_HANDLER_DESCRIPTIVE_NAME TEXT("OpenOffice.org Property Sheet Handler")
#define THUMBVIEWER_HANDLER_DESCRIPTIVAE_NAME TEXT("OpenOffice.org Thumbnail Viewer")

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
