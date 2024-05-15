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


#include "XMLIndexTableSourceContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>

#include <sax/tools/converter.hxx>

#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <rtl/ustring.hxx>


using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

XMLIndexTableSourceContext::XMLIndexTableSourceContext(
    SvXMLImport& rImport, Reference<XPropertySet> & rPropSet)
    : XMLIndexSourceBaseContext(rImport, rPropSet, UseStyles::Single)
    , nDisplayFormat(0)
    , bSequenceOK(false)
    , bDisplayFormatOK(false)
    , bUseCaption(true)
{
}

XMLIndexTableSourceContext::~XMLIndexTableSourceContext()
{
}

SvXMLEnumMapEntry<sal_uInt16> const lcl_aReferenceTypeTokenMap[] =
{

    { XML_TEXT,                 ReferenceFieldPart::TEXT },
    { XML_CATEGORY_AND_VALUE,   ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { XML_CAPTION,              ReferenceFieldPart::ONLY_CAPTION },

    // wrong values that previous versions wrote:
    { XML_CHAPTER,              ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { XML_PAGE,                 ReferenceFieldPart::ONLY_CAPTION },

    { XML_TOKEN_INVALID,        0 }
};

void XMLIndexTableSourceContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    bool bTmp(false);

    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_USE_CAPTION):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseCaption = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_CAPTION_SEQUENCE_NAME):
            sSequence = aIter.toString();
            bSequenceOK = true;
            break;

        case XML_ELEMENT(TEXT, XML_CAPTION_SEQUENCE_FORMAT):
        {
             sal_uInt16 nTmp;
             if (SvXMLUnitConverter::convertEnum(nTmp, aIter.toView(),
                                                 lcl_aReferenceTypeTokenMap))
             {
                 nDisplayFormat = nTmp;
                 bDisplayFormatOK = true;
             }
             break;
        }

        default:
            XMLIndexSourceBaseContext::ProcessAttribute(aIter);
            break;
    }
}

void XMLIndexTableSourceContext::endFastElement(sal_Int32 nElement)
{
    rIndexPropertySet->setPropertyValue(u"CreateFromLabels"_ustr, css::uno::Any(bUseCaption));

    if (bSequenceOK)
    {
        rIndexPropertySet->setPropertyValue(u"LabelCategory"_ustr, css::uno::Any(sSequence));
    }

    if (bDisplayFormatOK)
    {
        rIndexPropertySet->setPropertyValue(u"LabelDisplayType"_ustr, css::uno::Any(nDisplayFormat));
    }

    XMLIndexSourceBaseContext::endFastElement(nElement);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexTableSourceContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_TABLE_INDEX_ENTRY_TEMPLATE) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           aLevelNameTableMap,
                                           XML_TOKEN_INVALID, // no outline-level attr
                                           aLevelStylePropNameTableMap,
                                           aAllowedTokenTypesTable);
    }
    else
    {
        return XMLIndexSourceBaseContext::createFastChildContext(nElement,
                                                             xAttrList);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
