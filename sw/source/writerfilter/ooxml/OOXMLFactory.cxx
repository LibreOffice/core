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

#include <sax/fastattribs.hxx>
#include "OOXMLFactory.hxx"

namespace writerfilter::ooxml {

using namespace com::sun::star;


OOXMLFactory_ns::~OOXMLFactory_ns()
{
}


void OOXMLFactory::attributes(OOXMLFastContextHandler * pHandler,
                              const uno::Reference< xml::sax::XFastAttributeList > & xAttribs)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (!pFactory)
        return;

    sax_fastparser::FastAttributeList& rAttribs =
            sax_fastparser::castToFastAttributeList( xAttribs );

    const AttributeInfo *pAttr = pFactory->getAttributeInfoArray(nDefine);
    if (!pAttr)
        return;

    for (; pAttr->m_nToken != -1; ++pAttr)
    {
        sal_Int32 nToken = pAttr->m_nToken;
        sal_Int32 nAttrIndex = rAttribs.getAttributeIndex(nToken);
        if (nAttrIndex == -1)
            continue;

        Id nId = pFactory->getResourceId(nDefine, nToken);

        OOXMLValue::Pointer_t xValue;
        switch (pAttr->m_nResource)
        {
        case ResourceType::Boolean:
            xValue = OOXMLBooleanValue::Create(rAttribs.getAsViewByIndex(nAttrIndex));
            break;
        case ResourceType::String:
            xValue = new OOXMLStringValue(rAttribs.getValueByIndex(nAttrIndex));
            break;
        case ResourceType::Integer:
            xValue = OOXMLIntegerValue::Create(rAttribs.getAsIntegerByIndex(nAttrIndex));
            break;
        case ResourceType::Hex:
            xValue = new OOXMLHexValue(rAttribs.getAsViewByIndex(nAttrIndex));
            break;
        case ResourceType::HexColor:
            xValue = new OOXMLHexColorValue(rAttribs.getAsViewByIndex(nAttrIndex));
            break;
        case ResourceType::TwipsMeasure_asSigned:
        case ResourceType::TwipsMeasure_asZero:
            xValue = new OOXMLTwipsMeasureValue(rAttribs.getAsViewByIndex(nAttrIndex));
            if (xValue->getInt() < 0)
            {
                if (pAttr->m_nResource == ResourceType::TwipsMeasure_asZero)
                    xValue = OOXMLIntegerValue::Create(0);
            }
            break;
        case ResourceType::HpsMeasure:
            xValue = new OOXMLHpsMeasureValue(rAttribs.getAsViewByIndex(nAttrIndex));
            break;
        case ResourceType::MeasurementOrPercent:
            xValue = new OOXMLMeasurementOrPercentValue(rAttribs.getAsViewByIndex(nAttrIndex));
            break;
        case ResourceType::List:
            if (sal_uInt32 nValue;
                pFactory->getListValue(pAttr->m_nRef, rAttribs.getAsViewByIndex(nAttrIndex), nValue))
            {
                xValue = OOXMLIntegerValue::Create(nValue);
            }
            break;
        default:
            break;
        }

        if (xValue)
        {
            pHandler->newProperty(nId, xValue);
            pFactory->attributeAction(pHandler, nToken, xValue);
        }
    }
}

uno::Reference< xml::sax::XFastContextHandler>
OOXMLFactory::createFastChildContext(OOXMLFastContextHandler * pHandler,
                                     Token_t Element)
{
    Id nDefine = pHandler->getDefine();

    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    uno::Reference< xml::sax::XFastContextHandler> ret;

    //Avoid handling unknown tokens and recursing to death
    if ((Element & 0xffff) < oox::XML_TOKEN_COUNT)
        ret = createFastChildContextFromFactory(pHandler, pFactory, Element);

    return ret;
}

void OOXMLFactory::characters(OOXMLFastContextHandler * pHandler,
                              const OUString & rString)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory)
    {
        pFactory->charactersAction(pHandler, rString);
    }
}

void OOXMLFactory::startAction(OOXMLFastContextHandler * pHandler)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory)
    {
        pFactory->startAction(pHandler);
    }
}

void OOXMLFactory::endAction(OOXMLFastContextHandler * pHandler)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory)
    {
        pFactory->endAction(pHandler);
    }
}

void OOXMLFactory_ns::startAction(OOXMLFastContextHandler *)
{
}

void OOXMLFactory_ns::endAction(OOXMLFastContextHandler *)
{
}

void OOXMLFactory_ns::charactersAction(OOXMLFastContextHandler *, const OUString &)
{
}

void OOXMLFactory_ns::attributeAction(OOXMLFastContextHandler *, Token_t, const OOXMLValue::Pointer_t&)
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
