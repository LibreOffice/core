/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <xpathobject.hxx>

#include <string.h>

#include "../dom/document.hxx"
#include <nodelist.hxx>


namespace XPath
{
    static XPathObjectType lcl_GetType(xmlXPathObjectPtr const pXPathObj)
    {
        switch (pXPathObj->type)
        {
            case XPATH_UNDEFINED:
                return XPathObjectType_XPATH_UNDEFINED;
            case XPATH_NODESET:
                return XPathObjectType_XPATH_NODESET;
            case XPATH_BOOLEAN:
                return XPathObjectType_XPATH_BOOLEAN;
            case XPATH_NUMBER:
                return XPathObjectType_XPATH_NUMBER;
            case XPATH_STRING:
                return XPathObjectType_XPATH_STRING;
            case XPATH_POINT:
                return XPathObjectType_XPATH_POINT;
            case XPATH_RANGE:
                return XPathObjectType_XPATH_RANGE;
            case XPATH_LOCATIONSET:
                return XPathObjectType_XPATH_LOCATIONSET;
            case XPATH_USERS:
                return XPathObjectType_XPATH_USERS;
            case XPATH_XSLT_TREE:
                return XPathObjectType_XPATH_XSLT_TREE;
            default:
                return XPathObjectType_XPATH_UNDEFINED;
        }
    }

    CXPathObject::CXPathObject(
            ::rtl::Reference<DOM::CDocument> const& pDocument,
            ::osl::Mutex & rMutex,
            ::boost::shared_ptr<xmlXPathObject> const& pXPathObj)
        : m_pDocument(pDocument)
        , m_rMutex(rMutex)
        , m_pXPathObj(pXPathObj)
        , m_XPathObjectType(lcl_GetType(pXPathObj.get()))
    {
    }

    /**
        get object type
    */
    XPathObjectType CXPathObject::getObjectType() throw (RuntimeException)
    {
        return m_XPathObjectType;
    }

    /**
        get the nodes from a nodelist type object
    */
    Reference< XNodeList > SAL_CALL
    CXPathObject::getNodeList() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNodeList > const xRet(
            new CNodeList(m_pDocument, m_rMutex, m_pXPathObj));
        return xRet;
    }

     /**
        get value of a boolean object
     */
    sal_Bool SAL_CALL CXPathObject::getBoolean() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        return (sal_Bool) xmlXPathCastToBoolean(m_pXPathObj.get());
    }

    /**
        get number as byte
    */
    sal_Int8 SAL_CALL CXPathObject::getByte() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        return (sal_Int8) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as short
    */
    sal_Int16 SAL_CALL CXPathObject::getShort() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        return (sal_Int16) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as long
    */
    sal_Int32 SAL_CALL CXPathObject::getLong() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        return (sal_Int32) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as hyper
    */
    sal_Int64 SAL_CALL CXPathObject::getHyper() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        return (sal_Int64) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as float
    */
    float SAL_CALL CXPathObject::getFloat() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        return (float) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as double
    */
    double SAL_CALL CXPathObject::getDouble() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        return  xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get string value
    */
    OUString SAL_CALL CXPathObject::getString() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        ::boost::shared_ptr<xmlChar const> str(
            xmlXPathCastToString(m_pXPathObj.get()), xmlFree);
        sal_Char const*const pS(reinterpret_cast<sal_Char const*>(str.get()));
        return OUString(pS, strlen(pS), RTL_TEXTENCODING_UTF8);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
