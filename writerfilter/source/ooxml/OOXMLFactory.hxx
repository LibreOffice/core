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



#ifndef INCLUDED_OOXML_FACTORY_HXX
#define INCLUDED_OOXML_FACTORY_HXX

#include <hash_map>
#include <boost/shared_ptr.hpp>

#ifndef INCLUDED_WW8_RESOURCE_MODEL_HXX
#include <resourcemodel/WW8ResourceModel.hxx>
#endif

#ifndef INCLUDED_OOXML_FAST_TOKENS_HXX
#include <ooxml/OOXMLFastTokens.hxx>
#endif

#ifndef INCLUDED_OOXML_FAST_CONTEXT_HANDLER_HXX
#include "OOXMLFastContextHandler.hxx"
#endif

namespace writerfilter {
namespace ooxml {

using namespace std;

enum ResourceType_t {
    RT_NoResource,
    RT_Table,
    RT_Stream,
    RT_List,
    RT_Integer,
    RT_Properties,
    RT_Hex,
    RT_String,
    RT_Shape,
    RT_Boolean,
    RT_HexValue,
    RT_Value,
    RT_XNote,
    RT_TextTableCell,
    RT_TextTableRow,
    RT_TextTable,
    RT_PropertyTable,
    RT_Any
};

struct AttributeInfo
{
    ResourceType_t m_nResource;
    Id m_nRef;

    AttributeInfo(ResourceType_t nResource, Id nRef);
    AttributeInfo();
};

typedef hash_map<sal_Int32, AttributeInfo, TokenHash> AttributeToResourceMap;
typedef boost::shared_ptr<AttributeToResourceMap> AttributeToResourceMapPointer;
typedef hash_map<Id, AttributeToResourceMapPointer> AttributesMap;

typedef hash_map<rtl::OUString, sal_Int32, ::rtl::OUStringHash> ListValueMap;
typedef boost::shared_ptr<ListValueMap> ListValueMapPointer;
typedef hash_map<Id, ListValueMapPointer> ListValuesMap;

struct CreateElement
{
    ResourceType_t m_nResource;
    Id m_nId;

    CreateElement(ResourceType_t nResource, Id nId);
    CreateElement();
};

typedef hash_map<sal_Int32, CreateElement, TokenHash> CreateElementMap;
typedef boost::shared_ptr<CreateElementMap> CreateElementMapPointer;
typedef hash_map<Id, CreateElementMapPointer> CreateElementsMap;
typedef hash_map<Id, string> IdToStringMap;
typedef boost::shared_ptr<IdToStringMap> IdToStringMapPointer;

typedef hash_map<Id, sal_Int32> TokenToIdMap;
typedef boost::shared_ptr<TokenToIdMap> TokenToIdMapPointer;
typedef hash_map<Id, TokenToIdMapPointer> TokenToIdsMap;

class OOXMLFactory_ns {
public:
    typedef boost::shared_ptr<OOXMLFactory_ns> Pointer_t;

    virtual void startAction(OOXMLFastContextHandler * pHandler);
    virtual void charactersAction(OOXMLFastContextHandler * pHandler, const ::rtl::OUString & rString);
    virtual void endAction(OOXMLFastContextHandler * pHandler);
    virtual void attributeAction(OOXMLFastContextHandler * pHandler, sal_Int32 nToken, OOXMLValue::Pointer_t pValue);
    virtual string getDefineName(Id nId) const;
#ifdef DEBUG_FACTORY
    virtual string getName() const;
#endif

    AttributeToResourceMapPointer getAttributeToResourceMap(Id nId);
    ListValueMapPointer getListValueMap(Id nId);
    CreateElementMapPointer getCreateElementMap(Id nId);
    TokenToIdMapPointer getTokenToIdMap(Id nId);

protected:
    virtual ~OOXMLFactory_ns();

    AttributesMap m_AttributesMap;
    ListValuesMap m_ListValuesMap;
    CreateElementsMap m_CreateElementsMap;
    TokenToIdsMap m_TokenToIdsMap;

    virtual AttributeToResourceMapPointer createAttributeToResourceMap(Id nId) = 0;
    virtual ListValueMapPointer createListValueMap(Id nId) = 0;
    virtual CreateElementMapPointer createCreateElementMap(Id nId) = 0;
    virtual TokenToIdMapPointer createTokenToIdMap(Id nId) = 0;
};

class OOXMLFactory
{
public:
    typedef boost::shared_ptr<OOXMLFactory> Pointer_t;

    static Pointer_t getInstance();

    uno::Reference< xml::sax::XFastContextHandler> createFastChildContext
    (OOXMLFastContextHandler * pHandler, sal_Int32 Element);

    uno::Reference< xml::sax::XFastContextHandler> createFastChildContextFromStart
    (OOXMLFastContextHandler * pHandler, sal_Int32 Element);

    void attributes(OOXMLFastContextHandler * pHandler,
                    const uno::Reference< xml::sax::XFastAttributeList > & Attribs);

    void characters(OOXMLFastContextHandler * pHandler,
                    const ::rtl::OUString & rString);

    void startAction(OOXMLFastContextHandler * pHandler, sal_Int32 nToken);
    void endAction(OOXMLFastContextHandler * pHandler, sal_Int32 nToken);

    virtual ~OOXMLFactory();

private:
    static Pointer_t m_Instance;

    OOXMLFactory();
    OOXMLFactory_ns::Pointer_t getFactoryForNamespace(Id id);

    uno::Reference< xml::sax::XFastContextHandler>
    createFastChildContextFromFactory(OOXMLFastContextHandler * pHandler,
                                      OOXMLFactory_ns::Pointer_t pFactory,
                                      sal_Int32 Element);
};

}
}

#endif // INCLUDED_OOXML_FACTORY_HXX
