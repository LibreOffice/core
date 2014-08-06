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

#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFACTORY_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFACTORY_HXX

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <resourcemodel/WW8ResourceModel.hxx>

#include <oox/token/tokens.hxx>

#include "OOXMLFastContextHandler.hxx"
#include <boost/intrusive_ptr.hpp>

namespace writerfilter {
namespace ooxml {

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
typedef boost::unordered_map<Id, std::string> IdToStringMap;
typedef boost::shared_ptr<IdToStringMap> IdToStringMapPointer;

class OOXMLFactory_ns {
public:
    typedef boost::shared_ptr<OOXMLFactory_ns> Pointer_t;

    virtual void startAction(OOXMLFastContextHandler * pHandler);
    virtual void charactersAction(OOXMLFastContextHandler * pHandler, const OUString & rString);
    virtual void endAction(OOXMLFastContextHandler * pHandler);
    virtual void attributeAction(OOXMLFastContextHandler * pHandler, Token_t nToken, OOXMLValue::Pointer_t pValue);

    AttributeToResourceMapPointer getAttributeToResourceMap(Id nId);
    CreateElementMapPointer getCreateElementMap(Id nId);

protected:
    virtual ~OOXMLFactory_ns();

    AttributesMap m_AttributesMap;
    CreateElementsMap m_CreateElementsMap;

    virtual AttributeToResourceMapPointer createAttributeToResourceMap(Id nId) = 0;
    virtual CreateElementMapPointer createCreateElementMap(Id nId) = 0;

public:
    virtual bool getListValue(Id nId, const OUString& rValue, sal_uInt32& rOutValue) = 0;
    virtual Id getResourceId(Id nDefine, sal_Int32 nToken) = 0;
};

class OOXMLFactory
{
public:
    typedef boost::intrusive_ptr<OOXMLFactory>Pointer_t;

    static Pointer_t getInstance();

    css::uno::Reference< css::xml::sax::XFastContextHandler> createFastChildContext(OOXMLFastContextHandler * pHandler, Token_t Element);

    css::uno::Reference< css::xml::sax::XFastContextHandler> createFastChildContextFromStart(OOXMLFastContextHandler * pHandler, Token_t Element);

    void attributes(OOXMLFastContextHandler * pHandler, const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs);

    void characters(OOXMLFastContextHandler * pHandler, const OUString & rString);

    void startAction(OOXMLFastContextHandler * pHandler, Token_t nToken);
    void endAction(OOXMLFastContextHandler * pHandler, Token_t nToken);

    virtual ~OOXMLFactory();
public:
    sal_uInt32 mnRefCnt;
private:
    static Pointer_t m_Instance;

    OOXMLFactory();
    OOXMLFactory_ns::Pointer_t getFactoryForNamespace(Id id);

    css::uno::Reference< css::xml::sax::XFastContextHandler> createFastChildContextFromFactory(OOXMLFastContextHandler * pHandler, OOXMLFactory_ns::Pointer_t pFactory, Token_t Element);
};

  inline void intrusive_ptr_add_ref(OOXMLFactory* p)
  {
      p->mnRefCnt++;
  }
  inline void intrusive_ptr_release(OOXMLFactory* p)
  {
      if (!(--p->mnRefCnt))
          delete p;
  }
}
}

#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
