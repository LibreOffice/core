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
#include <xmloff/xmltoken.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;


XMLIndexSourceBaseContext::XMLIndexSourceBaseContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet,
    UseStyles const eUseStyles)
:   SvXMLImportContext(rImport)
,   m_UseStyles(eUseStyles)
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
        ProcessAttribute(aIter);
}

void XMLIndexSourceBaseContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_INDEX_SCOPE):
            if ( IsXMLToken( aIter, XML_CHAPTER ) )
            {
                bChapterIndex = true;
            }
            break;

        case XML_ELEMENT(TEXT, XML_RELATIVE_TAB_STOP_POSITION):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bRelativeTabs = bTmp;
            }
            break;
        }

        default:
            // unknown attribute -> ignore
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
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
    const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList)
{
    SvXMLImportContextRef xContext;

    if (nElement == XML_ELEMENT(TEXT, XML_INDEX_TITLE_TEMPLATE) )
    {
        xContext = new XMLIndexTitleTemplateContext(GetImport(),
                                                    rIndexPropertySet);
    }
    else if (m_UseStyles == UseStyles::Level
            && nElement == XML_ELEMENT(TEXT, XML_INDEX_SOURCE_STYLES))
    {
        xContext = new XMLIndexTOCStylesContext(GetImport(),
                                                rIndexPropertySet);
    }
    else if (m_UseStyles == UseStyles::Single
            && (nElement == XML_ELEMENT(LO_EXT, XML_INDEX_SOURCE_STYLE)
                || nElement == XML_ELEMENT(TEXT, XML_INDEX_SOURCE_STYLE)))
    {
        OUString const styleName(xmloff::GetIndexSourceStyleName(xAttrList));
        if (!styleName.isEmpty())
        {
            OUString const convertedStyleName(GetImport().GetStyleDisplayName(
                        XmlStyleFamily::TEXT_PARAGRAPH, styleName));
            rIndexPropertySet->setPropertyValue("CreateFromParagraphStyle", css::uno::Any(convertedStyleName));
        }
    }

    // else: unknown namespace -> ignore

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
