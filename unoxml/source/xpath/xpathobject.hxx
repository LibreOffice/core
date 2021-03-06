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

#pragma once

#include <memory>

#include <libxml/xpath.h>

#include <sal/types.h>
#include <rtl/ref.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>

#include "../dom/document.hxx"

namespace DOM {
    class CDocument;
}

namespace XPath
{
    class CXPathObject : public cppu::WeakImplHelper< css::xml::xpath::XXPathObject >
    {
    private:
        ::rtl::Reference< DOM::CDocument > const m_pDocument;
        ::osl::Mutex & m_rMutex;
        std::shared_ptr<xmlXPathObject> const m_pXPathObj;
        css::xml::xpath::XPathObjectType const m_XPathObjectType;

    public:
        CXPathObject( ::rtl::Reference<DOM::CDocument> const& pDocument,
            ::osl::Mutex & rMutex,
            std::shared_ptr<xmlXPathObject> const& pXPathObj);

    /**
        get object type
    */
    virtual css::xml::xpath::XPathObjectType SAL_CALL getObjectType() override;

    /**
        get the nodes from a nodelist type object
    */
    virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getNodeList() override;

     /**
        get value of a boolean object
     */
     virtual sal_Bool SAL_CALL getBoolean() override;

    /**
        get number as byte
    */
    virtual sal_Int8 SAL_CALL getByte() override;

    /**
        get number as short
    */
    virtual sal_Int16 SAL_CALL getShort() override;

    /**
        get number as long
    */
    virtual sal_Int32 SAL_CALL getLong() override;

    /**
        get number as hyper
    */
    virtual sal_Int64 SAL_CALL getHyper() override;

    /**
        get number as float
    */
    virtual float SAL_CALL getFloat() override;

    /**
        get number as double
    */
    virtual double SAL_CALL getDouble() override;

    /**
        get string value
    */
    virtual OUString SAL_CALL getString() override;

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
