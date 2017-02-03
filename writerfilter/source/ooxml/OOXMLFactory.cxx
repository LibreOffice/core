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

#include <rtl/instance.hxx>
#include <osl/mutex.hxx>
#include <sax/fastattribs.hxx>
#include "OOXMLFactory.hxx"
#include "OOXMLFastHelper.hxx"

namespace writerfilter {
namespace ooxml {

using namespace com::sun::star;

// class OOXMLFactory_ns

OOXMLFactory_ns::~OOXMLFactory_ns()
{
}

// class OOXMLFactory

void OOXMLFactory::attributes(OOXMLFastContextHandler * pHandler,
                              const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() == nullptr)
        return;

    assert( dynamic_cast< sax_fastparser::FastAttributeList *>( Attribs.get() ) != nullptr );
    sax_fastparser::FastAttributeList *pAttribs;
    pAttribs = static_cast< sax_fastparser::FastAttributeList *>( Attribs.get() );

    const AttributeInfo *pAttr = pFactory->getAttributeInfoArray(nDefine);
    if (!pAttr)
        return;

    for (; pAttr->m_nToken != -1; ++pAttr)
    {
        sal_Int32 nToken = pAttr->m_nToken;
        if (!pAttribs->hasAttribute(nToken))
            continue;

        Id nId = pFactory->getResourceId(nDefine, nToken);

        switch (pAttr->m_nResource)
        {
        case ResourceType::Boolean:
            {
                const char *pValue = "";
                pAttribs->getAsChar(nToken, pValue);
                OOXMLValue::Pointer_t xValue(OOXMLBooleanValue::Create(pValue));
                pHandler->newProperty(nId, xValue);
                pFactory->attributeAction(pHandler, nToken, xValue);
            }
            break;
        case ResourceType::String:
            {
                OUString aValue(pAttribs->getValue(nToken));
                OOXMLValue::Pointer_t xValue(new OOXMLStringValue(aValue));
                pHandler->newProperty(nId, xValue);
                pFactory->attributeAction(pHandler, nToken, xValue);
            }
            break;
        case ResourceType::Integer:
            {
                sal_Int32 nValue;
                pAttribs->getAsInteger(nToken,nValue);
                OOXMLValue::Pointer_t xValue = OOXMLIntegerValue::Create(nValue);
                pHandler->newProperty(nId, xValue);
                pFactory->attributeAction(pHandler, nToken, xValue);
            }
            break;
        case ResourceType::Hex:
            {
                const char *pValue = "";
                pAttribs->getAsChar(nToken, pValue);
                OOXMLValue::Pointer_t xValue(new OOXMLHexValue(pValue));
                pHandler->newProperty(nId, xValue);
                pFactory->attributeAction(pHandler, nToken, xValue);
            }
            break;
        case ResourceType::UniversalMeasure:
            {
                const char *pValue = "";
                pAttribs->getAsChar(nToken, pValue);
                OOXMLValue::Pointer_t xValue(new OOXMLUniversalMeasureValue(pValue));
                pHandler->newProperty(nId, xValue);
                pFactory->attributeAction(pHandler, nToken, xValue);
            }
            break;
        case ResourceType::List:
            {
                sal_uInt32 nValue;
                if (pFactory->getListValue(pAttr->m_nRef, Attribs->getValue(nToken), nValue))
                {
                    OOXMLValue::Pointer_t xValue = OOXMLIntegerValue::Create(nValue);
                    pHandler->newProperty(nId, xValue);
                    pFactory->attributeAction(pHandler, nToken, xValue);
                }
            }
            break;
        default:
            break;
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

    if (pFactory.get() != nullptr)
    {
        pFactory->charactersAction(pHandler, rString);
    }
}

void OOXMLFactory::startAction(OOXMLFastContextHandler * pHandler, Token_t /*nToken*/)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != nullptr)
    {
        pFactory->startAction(pHandler);
    }
}

void OOXMLFactory::endAction(OOXMLFastContextHandler * pHandler, Token_t /*nToken*/)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != nullptr)
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
