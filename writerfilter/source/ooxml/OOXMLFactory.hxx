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

#ifndef INCLUDED_OOXML_FACTORY_HXX
#define INCLUDED_OOXML_FACTORY_HXX

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <resourcemodel/WW8ResourceModel.hxx>

#include <ooxml/OOXMLFastTokens.hxx>

#include "OOXMLFastContextHandler.hxx"

namespace writerfilter {
namespace ooxml {

using namespace std;

enum ResourceType_t {
    RT_NoResource,
    RT_Table,
    RT_Stream,
    RT_StreamProperties,
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
    RT_Math,
    RT_Any,
    RT_UniversalMeasure
};

struct AttributeInfo
{
    ResourceType_t m_nResource;
    Id m_nRef;

    AttributeInfo(ResourceType_t nResource, Id nRef);
    AttributeInfo();
};

typedef boost::unordered_map<Token_t, AttributeInfo> AttributeToResourceMap;
typedef boost::shared_ptr<AttributeToResourceMap> AttributeToResourceMapPointer;
typedef boost::unordered_map<Id, AttributeToResourceMapPointer> AttributesMap;

typedef boost::unordered_map<OUString, sal_Int32, OUStringHash> ListValueMap;
typedef boost::shared_ptr<ListValueMap> ListValueMapPointer;
typedef boost::unordered_map<Id, ListValueMapPointer> ListValuesMap;

struct CreateElement
{
    ResourceType_t m_nResource;
    Id m_nId;

    CreateElement(ResourceType_t nResource, Id nId);
    CreateElement();
};

typedef boost::unordered_map<Token_t, CreateElement> CreateElementMap;
typedef boost::shared_ptr<CreateElementMap> CreateElementMapPointer;
typedef boost::unordered_map<Id, CreateElementMapPointer> CreateElementsMap;
typedef boost::unordered_map<Id, string> IdToStringMap;
typedef boost::shared_ptr<IdToStringMap> IdToStringMapPointer;

typedef boost::unordered_map<Id, Token_t> TokenToIdMap;
typedef boost::shared_ptr<TokenToIdMap> TokenToIdMapPointer;
typedef boost::unordered_map<Id, TokenToIdMapPointer> TokenToIdsMap;

class OOXMLFactory_ns {
public:
    typedef boost::shared_ptr<OOXMLFactory_ns> Pointer_t;

    virtual void startAction(OOXMLFastContextHandler * pHandler);
    virtual void charactersAction(OOXMLFastContextHandler * pHandler, const OUString & rString);
    virtual void endAction(OOXMLFastContextHandler * pHandler);
    virtual void attributeAction(OOXMLFastContextHandler * pHandler, Token_t nToken, OOXMLValue::Pointer_t pValue);
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
    (OOXMLFastContextHandler * pHandler, Token_t Element);

    uno::Reference< xml::sax::XFastContextHandler> createFastChildContextFromStart
    (OOXMLFastContextHandler * pHandler, Token_t Element);

    void attributes(OOXMLFastContextHandler * pHandler,
                    const uno::Reference< xml::sax::XFastAttributeList > & Attribs);

    void characters(OOXMLFastContextHandler * pHandler,
                    const OUString & rString);

    void startAction(OOXMLFastContextHandler * pHandler, Token_t nToken);
    void endAction(OOXMLFastContextHandler * pHandler, Token_t nToken);

    virtual ~OOXMLFactory();

private:
    static Pointer_t m_Instance;

    OOXMLFactory();
    OOXMLFactory_ns::Pointer_t getFactoryForNamespace(Id id);

    uno::Reference< xml::sax::XFastContextHandler>
    createFastChildContextFromFactory(OOXMLFastContextHandler * pHandler,
                                      OOXMLFactory_ns::Pointer_t pFactory,
                                      Token_t Element);
};

}
}

#endif // INCLUDED_OOXML_FACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
