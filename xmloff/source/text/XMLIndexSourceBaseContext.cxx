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

#include "XMLIndexSourceBaseContext.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "XMLIndexTitleTemplateContext.hxx"
#include "XMLIndexTOCStylesContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;

const SvXMLTokenMapEntry aIndexSourceTokenMap[] =
{
    { XML_NAMESPACE_TEXT,
          XML_OUTLINE_LEVEL,
          XML_TOK_INDEXSOURCE_OUTLINE_LEVEL},
    { XML_NAMESPACE_TEXT,
          XML_USE_INDEX_MARKS,
          XML_TOK_INDEXSOURCE_USE_INDEX_MARKS },
    { XML_NAMESPACE_TEXT,
          XML_INDEX_SCOPE,
          XML_TOK_INDEXSOURCE_INDEX_SCOPE },
    { XML_NAMESPACE_TEXT,
          XML_RELATIVE_TAB_STOP_POSITION,
          XML_TOK_INDEXSOURCE_RELATIVE_TABS } ,
    { XML_NAMESPACE_TEXT,
          XML_USE_OTHER_OBJECTS,
          XML_TOK_INDEXSOURCE_USE_OTHER_OBJECTS },
    { XML_NAMESPACE_TEXT,
          XML_USE_SPREADSHEET_OBJECTS,
          XML_TOK_INDEXSOURCE_USE_SHEET },
    { XML_NAMESPACE_TEXT,
          XML_USE_CHART_OBJECTS,
          XML_TOK_INDEXSOURCE_USE_CHART },
    { XML_NAMESPACE_TEXT,
          XML_USE_DRAW_OBJECTS,
          XML_TOK_INDEXSOURCE_USE_DRAW },
    { XML_NAMESPACE_TEXT,
          XML_USE_IMAGE_OBJECTS,
          XML_TOK_INDEXSOURCE_USE_IMAGE },
    { XML_NAMESPACE_TEXT,
          XML_USE_MATH_OBJECTS,
          XML_TOK_INDEXSOURCE_USE_MATH },
    { XML_NAMESPACE_TEXT,
          XML_MAIN_ENTRY_STYLE_NAME,
          XML_TOK_INDEXSOURCE_MAIN_ENTRY_STYLE },
    { XML_NAMESPACE_TEXT,
          XML_IGNORE_CASE,
          XML_TOK_INDEXSOURCE_IGNORE_CASE },
    { XML_NAMESPACE_TEXT,
          XML_ALPHABETICAL_SEPARATORS,
          XML_TOK_INDEXSOURCE_SEPARATORS },
    { XML_NAMESPACE_TEXT,
          XML_COMBINE_ENTRIES,
          XML_TOK_INDEXSOURCE_COMBINE_ENTRIES },
    { XML_NAMESPACE_TEXT,
          XML_COMBINE_ENTRIES_WITH_DASH,
          XML_TOK_INDEXSOURCE_COMBINE_WITH_DASH },
    { XML_NAMESPACE_TEXT,
          XML_USE_KEYS_AS_ENTRIES,
          XML_TOK_INDEXSOURCE_KEYS_AS_ENTRIES },
    { XML_NAMESPACE_TEXT,
          XML_COMBINE_ENTRIES_WITH_PP,
          XML_TOK_INDEXSOURCE_COMBINE_WITH_PP },
    { XML_NAMESPACE_TEXT,
          XML_CAPITALIZE_ENTRIES,
          XML_TOK_INDEXSOURCE_CAPITALIZE },
    { XML_NAMESPACE_TEXT,
          XML_USE_OBJECTS,
          XML_TOK_INDEXSOURCE_USE_OBJECTS },
    { XML_NAMESPACE_TEXT,
          XML_USE_GRAPHICS,
          XML_TOK_INDEXSOURCE_USE_GRAPHICS },
    { XML_NAMESPACE_TEXT,
          XML_USE_TABLES,
          XML_TOK_INDEXSOURCE_USE_TABLES },
    { XML_NAMESPACE_TEXT,
          XML_USE_FLOATING_FRAMES,
          XML_TOK_INDEXSOURCE_USE_FRAMES },
    { XML_NAMESPACE_TEXT,
          XML_COPY_OUTLINE_LEVELS,
          XML_TOK_INDEXSOURCE_COPY_OUTLINE_LEVELS },
    { XML_NAMESPACE_TEXT,
          XML_USE_CAPTION,
          XML_TOK_INDEXSOURCE_USE_CAPTION },
    { XML_NAMESPACE_TEXT,
          XML_CAPTION_SEQUENCE_NAME,
          XML_TOK_INDEXSOURCE_SEQUENCE_NAME },
    { XML_NAMESPACE_TEXT,
          XML_CAPTION_SEQUENCE_FORMAT,
          XML_TOK_INDEXSOURCE_SEQUENCE_FORMAT },
    { XML_NAMESPACE_TEXT,
          XML_COMMA_SEPARATED,
          XML_TOK_INDEXSOURCE_COMMA_SEPARATED },
    { XML_NAMESPACE_TEXT,
          XML_USE_INDEX_SOURCE_STYLES,
          XML_TOK_INDEXSOURCE_USE_INDEX_SOURCE_STYLES },
    { XML_NAMESPACE_TEXT, XML_SORT_ALGORITHM,
          XML_TOK_INDEXSOURCE_SORT_ALGORITHM },
    { XML_NAMESPACE_STYLE, XML_RFC_LANGUAGE_TAG, XML_TOK_INDEXSOURCE_RFC_LANGUAGE_TAG },
    { XML_NAMESPACE_FO, XML_LANGUAGE, XML_TOK_INDEXSOURCE_LANGUAGE },
    { XML_NAMESPACE_FO, XML_SCRIPT, XML_TOK_INDEXSOURCE_SCRIPT },
    { XML_NAMESPACE_FO, XML_COUNTRY, XML_TOK_INDEXSOURCE_COUNTRY },
    { XML_NAMESPACE_TEXT, XML_INDEX_NAME, XML_TOK_INDEXSOURCE_USER_INDEX_NAME },
    { XML_NAMESPACE_TEXT,
          XML_USE_OUTLINE_LEVEL,
          XML_TOK_INDEXSOURCE_USE_OUTLINE_LEVEL},

    XML_TOKEN_MAP_END
};


XMLIndexSourceBaseContext::XMLIndexSourceBaseContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet,
    bool bLevelFormats)
:   SvXMLImportContext(rImport)
,   bUseLevelFormats(bLevelFormats)
,   bChapterIndex(false)
,   bRelativeTabs(true)
,   rIndexPropertySet(rPropSet)
{
}

XMLIndexSourceBaseContext::~XMLIndexSourceBaseContext()
{
}

void XMLIndexSourceBaseContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // process attributes
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        ProcessAttribute(aIter.getToken(), aIter.toString());
}

void XMLIndexSourceBaseContext::ProcessAttribute(
    sal_Int32 nAttributeToken,
    const OUString& rValue)
{
    switch (nAttributeToken)
    {
        case XML_ELEMENT(TEXT, XML_INDEX_SCOPE):
            if ( IsXMLToken( rValue, XML_CHAPTER ) )
            {
                bChapterIndex = true;
            }
            break;

        case XML_ELEMENT(TEXT, XML_RELATIVE_TAB_STOP_POSITION):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bRelativeTabs = bTmp;
            }
            break;
        }

        default:
            // unknown attribute -> ignore
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttributeToken, rValue);
            break;
    }
}

void XMLIndexSourceBaseContext::endFastElement(sal_Int32 )
{
    rIndexPropertySet->setPropertyValue("IsRelativeTabstops", css::uno::Any(bRelativeTabs));
    rIndexPropertySet->setPropertyValue("CreateFromChapter", css::uno::Any(bChapterIndex));
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexSourceBaseContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& )
{
    SvXMLImportContextRef xContext;

    if (nElement == XML_ELEMENT(TEXT, XML_INDEX_TITLE_TEMPLATE) )
    {
        xContext = new XMLIndexTitleTemplateContext(GetImport(),
                                                    rIndexPropertySet);
    }
    else if ( bUseLevelFormats &&
              nElement == XML_ELEMENT(TEXT, XML_INDEX_SOURCE_STYLES) )
    {
        xContext = new XMLIndexTOCStylesContext(GetImport(),
                                                rIndexPropertySet);
    }
    // else: unknown namespace -> ignore

    return xContext.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
