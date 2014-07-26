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

#include <stdio.h>

#include <rtl/instance.hxx>
#include <osl/mutex.hxx>
#include <sax/fastattribs.hxx>
#include "OOXMLFactory.hxx"
#include "OOXMLFastHelper.hxx"

namespace writerfilter {
namespace ooxml {

AttributeInfo::AttributeInfo()
:m_nResource(RT_NoResource), m_nRef(0)
{
}

AttributeInfo::AttributeInfo(ResourceType_t nResource, Id nRef)
 :m_nResource(nResource), m_nRef(nRef)
{
}

CreateElement::CreateElement()
:m_nResource(RT_NoResource), m_nId(0)
{
}

CreateElement::CreateElement(ResourceType_t nResource, Id nId)
: m_nResource(nResource), m_nId(nId)
{
}

// class OOXMLFactory_ns

OOXMLFactory_ns::~OOXMLFactory_ns()
{
}

AttributeToResourceMapPointer OOXMLFactory_ns::getAttributeToResourceMap(Id nId)
{
    if (m_AttributesMap.find(nId) == m_AttributesMap.end())
        m_AttributesMap[nId] = createAttributeToResourceMap(nId);

    return m_AttributesMap[nId];
}

ListValueMapPointer OOXMLFactory_ns::getListValueMap(Id nId)
{
    if (m_ListValuesMap.find(nId) == m_ListValuesMap.end())
        m_ListValuesMap[nId] = createListValueMap(nId);

    return m_ListValuesMap[nId];
}

CreateElementMapPointer OOXMLFactory_ns::getCreateElementMap(Id nId)
{
    if (m_CreateElementsMap.find(nId) == m_CreateElementsMap.end())
        m_CreateElementsMap[nId] = createCreateElementMap(nId);

    return m_CreateElementsMap[nId];
}

TokenToIdMapPointer OOXMLFactory_ns::getTokenToIdMap(Id nId)
{
    if (m_TokenToIdsMap.find(nId) == m_TokenToIdsMap.end())
        m_TokenToIdsMap[nId] = createTokenToIdMap(nId);

    return m_TokenToIdsMap[nId];
}

#ifdef DEBUG_DOMAINMAPPER
string OOXMLFactory_ns::getDefineName(Id /*nId*/) const
{
    return "";
}
#endif

// class OOXMLFactory

typedef rtl::Static< osl::Mutex, OOXMLFactory > OOXMLFactory_Mutex;

OOXMLFactory::Pointer_t OOXMLFactory::m_Instance;

OOXMLFactory::OOXMLFactory()
{
    // multi-thread-safe mutex for all platforms
    osl::MutexGuard aGuard(OOXMLFactory_Mutex::get());
    mnRefCnt = 0;
}

OOXMLFactory::~OOXMLFactory()
{
}

OOXMLFactory::Pointer_t OOXMLFactory::getInstance()
{
    if (m_Instance.get() == NULL)
        m_Instance.reset(new OOXMLFactory());

    return m_Instance;
}

void OOXMLFactory::attributes(OOXMLFastContextHandler * pHandler,
                              const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
    {
        TokenToIdMapPointer pTokenToIdMap = pFactory->getTokenToIdMap(nDefine);
        AttributeToResourceMapPointer pMap = pFactory->getAttributeToResourceMap(nDefine);

        AttributeToResourceMap::const_iterator aIt;
        AttributeToResourceMap::const_iterator aEndIt = pMap->end();

        assert( dynamic_cast< sax_fastparser::FastAttributeList *>( Attribs.get() ) != NULL );
        sax_fastparser::FastAttributeList *pAttribs;
        pAttribs = static_cast< sax_fastparser::FastAttributeList *>( Attribs.get() );

        for (aIt = pMap->begin(); aIt != aEndIt; ++aIt)
        {
            sal_Int32 nToken = aIt->first;
            if (pAttribs->hasAttribute(nToken))
            {
                Id nId = (*pTokenToIdMap)[nToken];

                switch (aIt->second.m_nResource)
                {
                case RT_Boolean:
                    {
                        const char *pValue = "";
                        pAttribs->getAsChar(nToken, pValue);
                        OOXMLValue::Pointer_t xValue(OOXMLBooleanValue::Create(pValue));
                        pHandler->newProperty(nId, xValue);
                        pFactory->attributeAction(pHandler, nToken, xValue);
                    }
                    break;
                case RT_String:
                    {
                        OUString aValue(pAttribs->getValue(nToken));
                        OOXMLValue::Pointer_t xValue(new OOXMLStringValue(aValue));
                        pHandler->newProperty(nId, xValue);
                        pFactory->attributeAction(pHandler, nToken, xValue);
                    }
                    break;
                case RT_Integer:
                    {
                        sal_Int32 nValue;
                        pAttribs->getAsInteger(nToken,nValue);
                        OOXMLValue::Pointer_t xValue = OOXMLIntegerValue::Create(nValue);
                        pHandler->newProperty(nId, xValue);
                        pFactory->attributeAction(pHandler, nToken, xValue);
                    }
                    break;
                case RT_Hex:
                    {
                        const char *pValue = "";
                        pAttribs->getAsChar(nToken, pValue);
                        OOXMLValue::Pointer_t xValue(new OOXMLHexValue(pValue));
                        pHandler->newProperty(nId, xValue);
                        pFactory->attributeAction(pHandler, nToken, xValue);
                    }
                    break;
                case RT_UniversalMeasure:
                    {
                        const char *pValue = "";
                        pAttribs->getAsChar(nToken, pValue);
                        OOXMLValue::Pointer_t xValue(new OOXMLUniversalMeasureValue(pValue));
                        pHandler->newProperty(nId, xValue);
                        pFactory->attributeAction(pHandler, nToken, xValue);
                    }
                    break;
                case RT_List:
                    {
                        ListValueMapPointer pListValueMap =
                            pFactory->getListValueMap(aIt->second.m_nRef);

                        if (pListValueMap.get() != NULL)
                        {
                            OUString aValue(Attribs->getValue(nToken));
                            sal_uInt32 nValue = (*pListValueMap)[aValue];
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
    if ((Element & 0xffff) < OOXML_FAST_TOKENS_END)
        ret = createFastChildContextFromFactory(pHandler, pFactory, Element);

    return ret;
}

void OOXMLFactory::characters(OOXMLFastContextHandler * pHandler,
                              const OUString & rString)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
    {
        pFactory->charactersAction(pHandler, rString);
    }
}

void OOXMLFactory::startAction(OOXMLFastContextHandler * pHandler, Token_t /*nToken*/)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
    {
        pFactory->startAction(pHandler);
    }
}

void OOXMLFactory::endAction(OOXMLFastContextHandler * pHandler, Token_t /*nToken*/)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
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

void OOXMLFactory_ns::attributeAction(OOXMLFastContextHandler *, Token_t, OOXMLValue::Pointer_t)
{
}

#ifdef DEBUG_DOMAINMAPPER
string OOXMLFactory_ns::getName() const
{
    return "noname";
}
#endif

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
