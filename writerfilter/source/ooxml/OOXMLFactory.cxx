/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

        for (aIt = pMap->begin(); aIt != aEndIt; aIt++)
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
                        ::rtl::OUString aValue(Attribs->getValue(aIt->first));
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
                        ::rtl::OUString aValue(Attribs->getValue(aIt->first));
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
                        ::rtl::OUString aValue(Attribs->getValue(aIt->first));
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
                        ::rtl::OUString aValue(Attribs->getValue(aIt->first));
                        OOXMLFastHelper<OOXMLHexValue>::newProperty
                            (pHandler, nId, aValue);

                        OOXMLValue::Pointer_t pValue(new OOXMLHexValue(aValue));
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
                            ::rtl::OUString aValue(Attribs->getValue(aIt->first));
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
                        debug_logger->endElement("list");
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
            debug_logger->endElement("factory.attribute");
#endif
        }

#ifdef DEBUG_FACTORY
        debug_logger->endElement("factory.attributes");
#endif
    }
}

uno::Reference< xml::sax::XFastContextHandler>
OOXMLFactory::createFastChildContext(OOXMLFastContextHandler * pHandler,
                                     sal_Int32 Element)
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
    debug_logger->endElement("factory.createFastChildContext");
#endif

    return ret;
}

void OOXMLFactory::characters(OOXMLFastContextHandler * pHandler,
                              const ::rtl::OUString & rString)
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
    debug_logger->endElement("factory.characters");
#endif
}

void OOXMLFactory::startAction(OOXMLFastContextHandler * pHandler, sal_Int32 /*nToken*/)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
    {
#ifdef DEBUG_FACTORY
        debug_logger->startElement("factory.startAction");
#endif
        pFactory->startAction(pHandler);
#ifdef DEBUG_FACTORY
        debug_logger->endElement("factory.startAction");
#endif
    }
}

void OOXMLFactory::endAction(OOXMLFastContextHandler * pHandler, sal_Int32 /*nToken*/)
{
    Id nDefine = pHandler->getDefine();
    OOXMLFactory_ns::Pointer_t pFactory = getFactoryForNamespace(nDefine);

    if (pFactory.get() != NULL)
    {
#ifdef DEBUG_FACTORY
        debug_logger->startElement("factory.endAction");
#endif
        pFactory->endAction(pHandler);
#ifdef DEBUG_FACTORY
        debug_logger->endElement("factory.endAction");
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

void OOXMLFactory_ns::attributeAction(OOXMLFastContextHandler *, sal_Int32, OOXMLValue::Pointer_t)
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

