/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#ifdef DEBUG_ATTRIBUTES
        debug_logger->startElement("attributes");
        debug_logger->attribute("define", pFactory->getDefineName(nDefine));
        char sBuffer[256];
        snprintf(sBuffer, sizeof(sBuffer), "%08" SAL_PRIxUINT32, nDefine);
        debug_logger->attribute("define-num", sBuffer);
#endif
        
        TokenToIdMapPointer pTokenToIdMap = pFactory->getTokenToIdMap(nDefine);
        AttributeToResourceMapPointer pMap = pFactory->getAttributeToResourceMap(nDefine);
        
        AttributeToResourceMap::const_iterator aIt;
        AttributeToResourceMap::const_iterator aEndIt = pMap->end();
        
        for (aIt = pMap->begin(); aIt != aEndIt; aIt++)
        {
            Id nId = (*pTokenToIdMap)[aIt->first];
#ifdef DEBUG_ATTRIBUTES
            debug_logger->startElement("attribute");
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
#ifdef DEBUG_ATTRIBUTES
                        debug_logger->element("boolean");
#endif
                        ::rtl::OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLBooleanValue>::newProperty(pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLBooleanValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_String:
                    {
#ifdef DEBUG_ATTRIBUTES
                        debug_logger->element("string");
#endif
                        ::rtl::OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLStringValue>::newProperty
                            (pHandler, nId, aValue);
                        
                        OOXMLValue::Pointer_t pValue(new OOXMLStringValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_Integer:
                    {
#ifdef DEBUG_ATTRIBUTES
                        debug_logger->element("integer");
#endif
                        ::rtl::OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLIntegerValue>::newProperty
                            (pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLIntegerValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_Hex:
                    {
#ifdef DEBUG_ATTRIBUTES
                        debug_logger->element("hex");
#endif
                        ::rtl::OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLHexValue>::newProperty
                            (pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLHexValue(aValue));
                        pFactory->attributeAction(pHandler, aIt->first, pValue);
                    }
                    break;
                case RT_List:
                    {
#ifdef DEBUG_ATTRIBUTES
                        debug_logger->startElement("list");
#endif
                        ListValueMapPointer pListValueMap = 
                            pFactory->getListValueMap(aIt->second.m_nRef);

                        if (pListValueMap.get() != NULL)
                        {
                            ::rtl::OUString aValue(Attribs->getValue(aIt->first));
                            sal_uInt32 nValue = (*pListValueMap)[aValue];

#ifdef DEBUG_ATTRIBUTES
                            debug_logger->attribute("value", aValue);
                            debug_logger->attribute("value-num", nValue);
#endif

                            OOXMLFastHelper<OOXMLIntegerValue>::newProperty
                                (pHandler, nId, nValue);

                            OOXMLValue::Pointer_t pValue(new OOXMLIntegerValue(nValue));
                            pFactory->attributeAction(pHandler, aIt->first, pValue);
                        }
#ifdef DEBUG_ATTRIBUTES
                        debug_logger->endElement("list");
#endif
                    }
                    break;
                default:
#ifdef DEBUG_ATTRIBUTES
                    debug_logger->element("unknown-attribute-type");
#endif 
                    break;
                }
            }
#ifdef DEBUG_ATTRIBUTES
            debug_logger->endElement("attribute");
#endif
        }

#ifdef DEBUG_ATTRIBUTES
        debug_logger->endElement("attributes");
#endif
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
                              const ::rtl::OUString & rString)
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
#ifdef DEBUG_ELEMENT
        debug_logger->startElement("factory-startAction");
#endif
        pFactory->startAction(pHandler);
#ifdef DEBUG_ELEMENT
        debug_logger->endElement("factory-startAction");
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
        debug_logger->startElement("factory-endAction");
#endif
        pFactory->endAction(pHandler);
#ifdef DEBUG_ELEMENT
        debug_logger->endElement("factory-endAction");
#endif
    }
}
    
void OOXMLFactory_ns::startAction(OOXMLFastContextHandler *)
{
}

void OOXMLFactory_ns::endAction(OOXMLFastContextHandler *)
{
}

void OOXMLFactory_ns::charactersAction(OOXMLFastContextHandler *, const ::rtl::OUString &)
{
}

void OOXMLFactory_ns::attributeAction(OOXMLFastContextHandler *, Token_t, OOXMLValue::Pointer_t)
{
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
