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

string OOXMLFactory_ns::getDefineName(Id /*nId*/) const
{
    return "";
}

// class OOXMLFactory

typedef rtl::Static< osl::Mutex, OOXMLFactory > OOXMLFactory_Mutex;

OOXMLFactory::Pointer_t OOXMLFactory::m_Instance;

OOXMLFactory::OOXMLFactory()
{
    // multi-thread-safe mutex for all platforms

    osl::MutexGuard aGuard(OOXMLFactory_Mutex::get());
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
#ifdef DEBUG_FACTORY
        debug_logger->startElement("factory.attributes");
        debug_logger->attribute("define", pFactory->getDefineName(nDefine));
        char sBuffer[256];
        snprintf(sBuffer, sizeof(sBuffer), "%08" SAL_PRIxUINT32, nDefine);
        debug_logger->attribute("define-num", sBuffer);
#endif

        TokenToIdMapPointer pTokenToIdMap = pFactory->getTokenToIdMap(nDefine);
        AttributeToResourceMapPointer pMap = pFactory->getAttributeToResourceMap(nDefine);

        AttributeToResourceMap::const_iterator aIt;
        AttributeToResourceMap::const_iterator aEndIt = pMap->end();

        for (aIt = pMap->begin(); aIt != aEndIt; ++aIt)
        {
            Id nId = (*pTokenToIdMap)[aIt->first];
#ifdef DEBUG_FACTORY
            debug_logger->startElement("factory.attribute");
            debug_logger->attribute("name", fastTokenToId(aIt->first));
            debug_logger->attribute("tokenid", (*QNameToString::Instance())(nId));
            snprintf(sBuffer, sizeof(sBuffer), "%08" SAL_PRIxUINT32, nId);
            debug_logger->attribute("tokenid-num", sBuffer);
#endif
            if (Attribs->hasAttribute(aIt->first))
            {
                switch (aIt->second.m_nResource)
                {
                case RT_Boolean:
                    {
#ifdef DEBUG_FACTORY
                        debug_logger->element("boolean");
#endif
                        OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLBooleanValue>::newProperty(pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLBooleanValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_String:
                    {
#ifdef DEBUG_FACTORY
                        debug_logger->element("string");
#endif
                        OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLStringValue>::newProperty
                            (pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLStringValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_Integer:
                    {
#ifdef DEBUG_FACTORY
                        debug_logger->element("integer");
#endif
                        OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLIntegerValue>::newProperty
                            (pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLIntegerValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_Hex:
                    {
#ifdef DEBUG_FACTORY
                        debug_logger->element("hex");
#endif
                        OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLHexValue>::newProperty
                            (pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLHexValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_UniversalMeasure:
                    {
#ifdef DEBUG_FACTORY
                        debug_logger->element("universalMeasure");
#endif
                        OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLUniversalMeasureValue>::newProperty(pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLUniversalMeasureValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_List:
                    {
#ifdef DEBUG_FACTORY
                        debug_logger->startElement("list");
#endif
                        ListValueMapPointer pListValueMap =
                            pFactory->getListValueMap(aIt->second.m_nRef);

                        if (pListValueMap.get() != NULL)
                        {
                            OUString aValue(Attribs->getValue(aIt->first));
                            sal_uInt32 nValue = (*pListValueMap)[aValue];

#ifdef DEBUG_FACTORY
                            debug_logger->attribute("value", aValue);
                            debug_logger->attribute("value-num", nValue);
#endif

                            OOXMLFastHelper<OOXMLIntegerValue>::newProperty
                                (pHandler, nId, nValue);

                            OOXMLValue::Pointer_t pValue(new OOXMLIntegerValue(nValue));
                            pFactory->attributeAction(pHandler, aIt->first, pValue);
                        }
#ifdef DEBUG_FACTORY
                        debug_logger->endElement();
#endif
                    }
                    break;
                default:
#ifdef DEBUG_FACTORY
                    debug_logger->element("unknown-attribute-type");
#endif
                    break;
                }
            }
#ifdef DEBUG_FACTORY
            debug_logger->endElement();
#endif
        }

#ifdef DEBUG_FACTORY
        debug_logger->endElement();
#endif
    }
}

uno::Reference< xml::sax::XFastContextHandler>
OOXMLFactory::createFastChildContext(OOXMLFastContextHandler * pHandler,
                                     Token_t Element)
{
#ifdef DEBUG_FACTORY
    debug_logger->startElement("factory.createFastChildContext");
    debug_logger->attribute("token", fastTokenToId(Element));
#endif

    Id nDefine = pHandler->getDefine();

    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    uno::Reference< xml::sax::XFastContextHandler> ret;

    //Avoid handling unknown tokens and recursing to death
    if ((Element & 0xffff) < OOXML_FAST_TOKENS_END)
        ret = createFastChildContextFromFactory(pHandler, pFactory, Element);

#ifdef DEBUG_FACTORY
    debug_logger->endElement();
#endif

    return ret;
}

void OOXMLFactory::characters(OOXMLFastContextHandler * pHandler,
                              const OUString & rString)
{
#ifdef DEBUG_FACTORY
    debug_logger->startElement("factory.characters");
    debug_logger->chars(rString);
#endif

    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
    {
        pFactory->charactersAction(pHandler, rString);
    }

#ifdef DEBUG_FACTORY
    debug_logger->endElement();
#endif
}

void OOXMLFactory::startAction(OOXMLFastContextHandler * pHandler, Token_t /*nToken*/)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
    {
#ifdef DEBUG_ELEMENT
        debug_logger->startElement("factory.startAction");
#endif
        pFactory->startAction(pHandler);
#ifdef DEBUG_ELEMENT
        debug_logger->endElement();
#endif
    }
}

void OOXMLFactory::endAction(OOXMLFastContextHandler * pHandler, Token_t /*nToken*/)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
    {
#ifdef DEBUG_ELEMENT
        debug_logger->startElement("factory.endAction");
#endif
        pFactory->endAction(pHandler);
#ifdef DEBUG_ELEMENT
        debug_logger->endElement();
#endif
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

#ifdef DEBUG_FACTORY
string OOXMLFactory_ns::getName() const
{
    return "noname";
}
#endif

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
