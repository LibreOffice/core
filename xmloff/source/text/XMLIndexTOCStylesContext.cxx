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

#include "XMLIndexTOCStylesContext.hxx"

#include "XMLIndexSourceBaseContext.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::container::XIndexReplace;



XMLIndexTOCStylesContext::XMLIndexTOCStylesContext(
    SvXMLImport& rImport, Reference<XPropertySet> & rPropSet)
    : SvXMLImportContext(rImport)
    , rTOCPropertySet(rPropSet)
    , nOutlineLevel(0)
{
}

XMLIndexTOCStylesContext::~XMLIndexTOCStylesContext()
{
}

void XMLIndexTOCStylesContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // find text:outline-level attribute
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if ( aIter.getToken() == XML_ELEMENT(TEXT, XML_OUTLINE_LEVEL) )
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(
                    nTmp, aIter.toView(), 1,
                    GetImport().GetTextImport()->GetChapterNumbering()->
                                                                getCount()))
            {
                // API numbers 0..9, we number 1..10
                nOutlineLevel = nTmp-1;
            }
            break;
        }
    }
}

void XMLIndexTOCStylesContext::endFastElement(sal_Int32 )
{
    // if valid...
    if (nOutlineLevel < 0)
        return;

    // copy vector into sequence
    const sal_Int32 nCount = aStyleNames.size();
    Sequence<OUString> aStyleNamesSequence(nCount);
    auto aStyleNamesSequenceRange = asNonConstRange(aStyleNamesSequence);
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        aStyleNamesSequenceRange[i] = GetImport().GetStyleDisplayName(
                        XmlStyleFamily::TEXT_PARAGRAPH,
                           aStyleNames[i] );
    }

    // get index replace
    Any aAny = rTOCPropertySet->getPropertyValue(u"LevelParagraphStyles"_ustr);
    Reference<XIndexReplace> xIndexReplace;
    aAny >>= xIndexReplace;

    // set style names
    xIndexReplace->replaceByIndex(nOutlineLevel, Any(aStyleNamesSequence));
}

namespace xmloff {

OUString GetIndexSourceStyleName(
        css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList)
{
    for (auto& rIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        if (rIter.getToken() == XML_ELEMENT(TEXT, XML_STYLE_NAME))
        {
            return rIter.toString();
        }
    }
    return OUString();
}

} // namespace xmloff

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexTOCStylesContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // check for index-source-style
    if ( nElement == XML_ELEMENT(TEXT, XML_INDEX_SOURCE_STYLE) )
    {
        // find text:style-name attribute and record in aStyleNames
        OUString const styleName(xmloff::GetIndexSourceStyleName(xAttrList));
        if (!styleName.isEmpty())
        {
            aStyleNames.emplace_back(styleName);
        }
    }

    // always return default context; we already got the interesting info
    return new SvXMLImportContext(GetImport());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
