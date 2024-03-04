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


#include "XMLIndexObjectSourceContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <sax/tools/converter.hxx>

#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using namespace ::xmloff::token;


XMLIndexObjectSourceContext::XMLIndexObjectSourceContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet)
    : XMLIndexSourceBaseContext(rImport, rPropSet, UseStyles::Single),
        bUseCalc(false),
        bUseChart(false),
        bUseDraw(false),
        bUseMath(false),
        bUseOtherObjects(false)
{
}

XMLIndexObjectSourceContext::~XMLIndexObjectSourceContext()
{
}

void XMLIndexObjectSourceContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    bool bTmp(false);

    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_USE_OTHER_OBJECTS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseOtherObjects = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_SPREADSHEET_OBJECTS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseCalc = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_CHART_OBJECTS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseChart = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_DRAW_OBJECTS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseDraw = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_MATH_OBJECTS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseMath = bTmp;
            }
            break;

        default:
            XMLIndexSourceBaseContext::ProcessAttribute(aIter);
            break;
    }
}

void XMLIndexObjectSourceContext::endFastElement(sal_Int32 nElement)
{
    rIndexPropertySet->setPropertyValue("CreateFromStarCalc", css::uno::Any(bUseCalc));
    rIndexPropertySet->setPropertyValue("CreateFromStarChart", css::uno::Any(bUseChart));
    rIndexPropertySet->setPropertyValue("CreateFromStarDraw", css::uno::Any(bUseDraw));
    rIndexPropertySet->setPropertyValue("CreateFromStarMath", css::uno::Any(bUseMath));
    rIndexPropertySet->setPropertyValue("CreateFromOtherEmbeddedObjects", css::uno::Any(bUseOtherObjects));

    XMLIndexSourceBaseContext::endFastElement(nElement);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexObjectSourceContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_OBJECT_INDEX_ENTRY_TEMPLATE) )
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
