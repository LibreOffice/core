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
#include <com/sun/star/container/XIndexReplace.hpp>
#include "XMLIndexTemplateContext.hxx"
#include "XMLIndexTitleTemplateContext.hxx"
#include "XMLIndexTOCStylesContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/token/tokens.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/xml/sax/FastToken.hpp>


using namespace xmloff;
using namespace css::xml::sax;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;
using css::xml::sax::FastToken::NAMESPACE;

const sal_Char sAPI_CreateFromChapter[] = "CreateFromChapter";
const sal_Char sAPI_IsRelativeTabstops[] = "IsRelativeTabstops";

static const SvXMLTokenMapEntry aIndexSourceTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_OUTLINE_LEVEL, XML_TOK_INDEXSOURCE_OUTLINE_LEVEL,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_outline_level) },
    { XML_NAMESPACE_TEXT, XML_USE_INDEX_MARKS, XML_TOK_INDEXSOURCE_USE_INDEX_MARKS,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_index_marks) },
    { XML_NAMESPACE_TEXT, XML_INDEX_SCOPE, XML_TOK_INDEXSOURCE_INDEX_SCOPE,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_index_scope) },
    { XML_NAMESPACE_TEXT, XML_RELATIVE_TAB_STOP_POSITION, XML_TOK_INDEXSOURCE_RELATIVE_TABS,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_relative_tab_stop_position) } ,
    { XML_NAMESPACE_TEXT, XML_USE_OTHER_OBJECTS, XML_TOK_INDEXSOURCE_USE_OTHER_OBJECTS,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_other_objects) },
    { XML_NAMESPACE_TEXT, XML_USE_SPREADSHEET_OBJECTS, XML_TOK_INDEXSOURCE_USE_SHEET,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_spreadsheet_objects) },
    { XML_NAMESPACE_TEXT, XML_USE_CHART_OBJECTS, XML_TOK_INDEXSOURCE_USE_CHART,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_chart_objects) },
    { XML_NAMESPACE_TEXT, XML_USE_DRAW_OBJECTS, XML_TOK_INDEXSOURCE_USE_DRAW,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_draw_objects) },
    { XML_NAMESPACE_TEXT, XML_USE_IMAGE_OBJECTS, XML_TOK_INDEXSOURCE_USE_IMAGE,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_image_objects) },
    { XML_NAMESPACE_TEXT, XML_USE_MATH_OBJECTS, XML_TOK_INDEXSOURCE_USE_MATH,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_math_objects) },
    { XML_NAMESPACE_TEXT, XML_MAIN_ENTRY_STYLE_NAME, XML_TOK_INDEXSOURCE_MAIN_ENTRY_STYLE,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_main_entry_style_name) },
    { XML_NAMESPACE_TEXT, XML_IGNORE_CASE, XML_TOK_INDEXSOURCE_IGNORE_CASE,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_ignore_case) },
    { XML_NAMESPACE_TEXT, XML_ALPHABETICAL_SEPARATORS, XML_TOK_INDEXSOURCE_SEPARATORS,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_alphabetical_separators) },
    { XML_NAMESPACE_TEXT, XML_COMBINE_ENTRIES, XML_TOK_INDEXSOURCE_COMBINE_ENTRIES,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_combine_entries) },
    { XML_NAMESPACE_TEXT, XML_COMBINE_ENTRIES_WITH_DASH, XML_TOK_INDEXSOURCE_COMBINE_WITH_DASH,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_combine_entries_with_dash) },
    { XML_NAMESPACE_TEXT, XML_USE_KEYS_AS_ENTRIES, XML_TOK_INDEXSOURCE_KEYS_AS_ENTRIES,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_keys_as_entries) },
    { XML_NAMESPACE_TEXT, XML_COMBINE_ENTRIES_WITH_PP, XML_TOK_INDEXSOURCE_COMBINE_WITH_PP,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_combine_entries_with_pp) },
    { XML_NAMESPACE_TEXT, XML_CAPITALIZE_ENTRIES, XML_TOK_INDEXSOURCE_CAPITALIZE,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_capitalize_entries) },
    { XML_NAMESPACE_TEXT, XML_USE_OBJECTS, XML_TOK_INDEXSOURCE_USE_OBJECTS,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_objects) },
    { XML_NAMESPACE_TEXT, XML_USE_GRAPHICS, XML_TOK_INDEXSOURCE_USE_GRAPHICS,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_graphics) },
    { XML_NAMESPACE_TEXT, XML_USE_TABLES, XML_TOK_INDEXSOURCE_USE_TABLES,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_tables) },
    { XML_NAMESPACE_TEXT, XML_USE_FLOATING_FRAMES, XML_TOK_INDEXSOURCE_USE_FRAMES,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_floating_frames) },
    { XML_NAMESPACE_TEXT, XML_COPY_OUTLINE_LEVELS, XML_TOK_INDEXSOURCE_COPY_OUTLINE_LEVELS,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_copy_outline_levels) },
    { XML_NAMESPACE_TEXT, XML_USE_CAPTION, XML_TOK_INDEXSOURCE_USE_CAPTION,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_caption) },
    { XML_NAMESPACE_TEXT, XML_CAPTION_SEQUENCE_NAME, XML_TOK_INDEXSOURCE_SEQUENCE_NAME,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_caption_sequence_name) },
    { XML_NAMESPACE_TEXT, XML_CAPTION_SEQUENCE_FORMAT, XML_TOK_INDEXSOURCE_SEQUENCE_FORMAT,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_caption_sequence_format) },
    { XML_NAMESPACE_TEXT, XML_COMMA_SEPARATED, XML_TOK_INDEXSOURCE_COMMA_SEPARATED,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_comma_separated) },
    { XML_NAMESPACE_TEXT, XML_USE_INDEX_SOURCE_STYLES, XML_TOK_INDEXSOURCE_USE_INDEX_SOURCE_STYLES,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_index_source_styles) },
    { XML_NAMESPACE_TEXT, XML_SORT_ALGORITHM, XML_TOK_INDEXSOURCE_SORT_ALGORITHM,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_sort_algorithm) },
    { XML_NAMESPACE_STYLE, XML_RFC_LANGUAGE_TAG, XML_TOK_INDEXSOURCE_RFC_LANGUAGE_TAG,
        (NAMESPACE | XML_NAMESPACE_STYLE | XML_rfc_language_tag) },
    { XML_NAMESPACE_FO, XML_LANGUAGE, XML_TOK_INDEXSOURCE_LANGUAGE,
        (NAMESPACE | XML_NAMESPACE_FO | XML_language) },
    { XML_NAMESPACE_FO, XML_SCRIPT, XML_TOK_INDEXSOURCE_SCRIPT,
        (NAMESPACE | XML_NAMESPACE_FO | XML_script) },
    { XML_NAMESPACE_FO, XML_COUNTRY, XML_TOK_INDEXSOURCE_COUNTRY,
        (NAMESPACE | XML_NAMESPACE_FO | XML_country) },
    { XML_NAMESPACE_TEXT, XML_INDEX_NAME, XML_TOK_INDEXSOURCE_USER_INDEX_NAME,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_index_name) },
    { XML_NAMESPACE_TEXT, XML_USE_OUTLINE_LEVEL, XML_TOK_INDEXSOURCE_USE_OUTLINE_LEVEL,
        (NAMESPACE | XML_NAMESPACE_TEXT | XML_use_outline_level) },

    XML_TOKEN_MAP_END
};


TYPEINIT1( XMLIndexSourceBaseContext, SvXMLImportContext );

XMLIndexSourceBaseContext::XMLIndexSourceBaseContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet,
    bool bLevelFormats)
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
,   sCreateFromChapter(sAPI_CreateFromChapter)
,   sIsRelativeTabstops(sAPI_IsRelativeTabstops)
,   bUseLevelFormats(bLevelFormats)
,   bChapterIndex(false)
,   bRelativeTabs(true)
,   rIndexPropertySet(rPropSet)
{
}

XMLIndexSourceBaseContext::XMLIndexSourceBaseContext(
    SvXMLImport& rImport, sal_Int32 /*Element*/,
    Reference< XPropertySet >& rPropSet, bool bLevelFormats )
:   SvXMLImportContext( rImport ),
    sCreateFromChapter(sAPI_CreateFromChapter),
    sIsRelativeTabstops(sAPI_IsRelativeTabstops),
    bUseLevelFormats(bLevelFormats),
    bChapterIndex(false),
    bRelativeTabs(true),
    rIndexPropertySet(rPropSet)
{
}

XMLIndexSourceBaseContext::~XMLIndexSourceBaseContext()
{
}

void XMLIndexSourceBaseContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLTokenMap aTokenMap(aIndexSourceTokenMap);

    // process attributes
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++)
    {
        // map to IndexSourceParamEnum
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );
        sal_uInt16 nToken = aTokenMap.Get(nPrefix, sLocalName);

        // process attribute
        ProcessAttribute((enum IndexSourceParamEnum)nToken,
                         xAttrList->getValueByIndex(i));
    }
}

void XMLIndexSourceBaseContext::startFastElement( sal_Int32 /*Element*/,
    const Reference< XFastAttributeList >& xAttrList )
    throw(css::uno::RuntimeException, SAXException, std::exception)
{
    SvXMLTokenMap aTokenMap(aIndexSourceTokenMap);

    css::uno::Sequence< css::xml::FastAttribute > attributes = xAttrList->getFastAttributes();
    for( css::xml::FastAttribute attribute : attributes )
    {
        // map to IndexSourceParamEnum
        sal_uInt16 nToken = aTokenMap.Get( attribute.Token );

        // process attribute
        ProcessAttribute((enum IndexSourceParamEnum)nToken,
                         attribute.Value );
    }
}

void XMLIndexSourceBaseContext::ProcessAttribute(
    enum IndexSourceParamEnum eParam,
    const OUString& rValue)
{
    switch (eParam)
    {
        case XML_TOK_INDEXSOURCE_INDEX_SCOPE:
            if ( IsXMLToken( rValue, XML_CHAPTER ) )
            {
                bChapterIndex = true;
            }
            break;

        case XML_TOK_INDEXSOURCE_RELATIVE_TABS:
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
            break;
    }
}

void XMLIndexSourceBaseContext::EndElement()
{
    Any aAny;

    aAny.setValue(&bRelativeTabs, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sIsRelativeTabstops, aAny);

    aAny.setValue(&bChapterIndex, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromChapter, aAny);
}

void SAL_CALL XMLIndexSourceBaseContext::endFastElement( sal_Int32 /*Element*/ )
    throw(css::uno::RuntimeException, SAXException, std::exception)
{
    Any aAny;

    aAny.setValue(&bRelativeTabs, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sIsRelativeTabstops, aAny);

    aAny.setValue(&bChapterIndex, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromChapter, aAny);
}

SvXMLImportContext* XMLIndexSourceBaseContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if (XML_NAMESPACE_TEXT == nPrefix)
    {
        if ( IsXMLToken( rLocalName, XML_INDEX_TITLE_TEMPLATE ) )
        {
            pContext = new XMLIndexTitleTemplateContext(GetImport(),
                                                        rIndexPropertySet,
                                                        nPrefix, rLocalName);
        }
        else if ( bUseLevelFormats &&
                  IsXMLToken( rLocalName, XML_INDEX_SOURCE_STYLES ) )
        {
            pContext = new XMLIndexTOCStylesContext(GetImport(),
                                                    rIndexPropertySet,
                                                    nPrefix, rLocalName);
        }
        // else: unknown element in text namespace -> ignore
    }
    // else: unknown namespace -> ignore

    // use default context
    if (pContext == NULL)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}

Reference< XFastContextHandler > SAL_CALL
    XMLIndexSourceBaseContext::createFastChildContext( sal_Int32 Element,
    const Reference< XFastAttributeList >& xAttrList )
    throw(css::uno::RuntimeException, SAXException, std::exception)
{
    Reference< XFastContextHandler > pContext = 0;

    if( Element == (NAMESPACE | XML_NAMESPACE_TEXT | XML_index_title_template) )
    {
        pContext = new XMLIndexTitleTemplateContext( GetImport(),
                    rIndexPropertySet, Element );
    }
    else if( bUseLevelFormats &&
        Element == (NAMESPACE | XML_NAMESPACE_TEXT | XML_index_source_styles) )
    {
        pContext = new XMLIndexTOCStylesContext( GetImport(),
                rIndexPropertySet, Element );
    }

    if( !pContext.is() )
        pContext = SvXMLImportContext::createFastChildContext( Element, xAttrList );

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
