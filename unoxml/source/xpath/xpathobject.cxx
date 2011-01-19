/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
            xmlXPathObjectPtr const xpathObj)
        : m_pDocument(pDocument)
        , m_rMutex(rMutex)
        , m_pXPathObj(xpathObj, xmlXPathFreeObject)
        , m_XPathObjectType(lcl_GetType(xpathObj))
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

        const sal_Char* x1 = (sal_Char*) xmlXPathCastToString(m_pXPathObj.get());
        return OUString(x1, strlen(x1), RTL_TEXTENCODING_UTF8);
    }

}

