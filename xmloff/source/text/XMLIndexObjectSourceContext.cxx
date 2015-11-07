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

#include <rtl/ustring.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>

#include <sax/tools/converter.hxx>

#include "XMLIndexTemplateContext.hxx"
#include "XMLIndexTitleTemplateContext.hxx"
#include "XMLIndexTOCStylesContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_OBJECT_INDEX_ENTRY_TEMPLATE;
using ::xmloff::token::XML_TOKEN_INVALID;

const sal_Char sAPI_CreateFromStarCalc[] = "CreateFromStarCalc";
const sal_Char sAPI_CreateFromStarChart[] = "CreateFromStarChart";
const sal_Char sAPI_CreateFromStarDraw[] = "CreateFromStarDraw";
const sal_Char sAPI_CreateFromStarMath[] = "CreateFromStarMath";
const sal_Char sAPI_CreateFromOtherEmbeddedObjects[] = "CreateFromOtherEmbeddedObjects";



XMLIndexObjectSourceContext::XMLIndexObjectSourceContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet) :
        XMLIndexSourceBaseContext(rImport, nPrfx, rLocalName,
                                  rPropSet, false),
        sCreateFromStarCalc(sAPI_CreateFromStarCalc),
        sCreateFromStarChart(sAPI_CreateFromStarChart),
        sCreateFromStarDraw(sAPI_CreateFromStarDraw),
        sCreateFromStarMath(sAPI_CreateFromStarMath),
        sCreateFromOtherEmbeddedObjects(sAPI_CreateFromOtherEmbeddedObjects),
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

void XMLIndexObjectSourceContext::ProcessAttribute(
    enum IndexSourceParamEnum eParam,
    const OUString& rValue)
{
    bool bTmp(false);

    switch (eParam)
    {
        case XML_TOK_INDEXSOURCE_USE_OTHER_OBJECTS:
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseOtherObjects = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_SHEET:
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseCalc = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_CHART:
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseChart = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_DRAW:
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseDraw = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_MATH:
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseMath = bTmp;
            }
            break;

        default:
            XMLIndexSourceBaseContext::ProcessAttribute(eParam, rValue);
            break;
    }
}

void XMLIndexObjectSourceContext::EndElement()
{
    Any aAny;

    aAny.setValue(&bUseCalc, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromStarCalc, aAny);

    aAny.setValue(&bUseChart, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromStarChart, aAny);

    aAny.setValue(&bUseDraw, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromStarDraw, aAny);

    aAny.setValue(&bUseMath, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromStarMath, aAny);

    aAny.setValue(&bUseOtherObjects, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromOtherEmbeddedObjects, aAny);

    XMLIndexSourceBaseContext::EndElement();
}

SvXMLImportContext* XMLIndexObjectSourceContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         (IsXMLToken(rLocalName, XML_OBJECT_INDEX_ENTRY_TEMPLATE)) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           nPrefix, rLocalName,
                                           aLevelNameTableMap,
                                           XML_TOKEN_INVALID, // no outline-level attr
                                           aLevelStylePropNameTableMap,
                                           aAllowedTokenTypesTable);
    }
    else
    {
        return XMLIndexSourceBaseContext::CreateChildContext(nPrefix,
                                                             rLocalName,
                                                             xAttrList);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
