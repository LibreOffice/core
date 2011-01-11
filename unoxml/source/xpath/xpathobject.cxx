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

#include <string.h>
#include "xpathobject.hxx"
#include "nodelist.hxx"

namespace XPath
{
    CXPathObject::CXPathObject(xmlXPathObjectPtr xpathObj, const Reference< XNode >& contextNode)
        : m_pXPathObj(xpathObj, xmlXPathFreeObject), m_xContextNode(contextNode)
    {
        switch (m_pXPathObj->type)
        {
        case XPATH_UNDEFINED:
            m_xPathObjectType = XPathObjectType_XPATH_UNDEFINED;
            break;
        case XPATH_NODESET:
            m_xPathObjectType = XPathObjectType_XPATH_NODESET;
            break;
        case XPATH_BOOLEAN:
            m_xPathObjectType = XPathObjectType_XPATH_BOOLEAN;
            break;
        case XPATH_NUMBER:
            m_xPathObjectType = XPathObjectType_XPATH_NUMBER;
            break;
        case XPATH_STRING:
            m_xPathObjectType = XPathObjectType_XPATH_STRING;
            break;
        case XPATH_POINT:
            m_xPathObjectType = XPathObjectType_XPATH_POINT;
            break;
        case XPATH_RANGE:
            m_xPathObjectType = XPathObjectType_XPATH_RANGE;
            break;
        case XPATH_LOCATIONSET:
            m_xPathObjectType = XPathObjectType_XPATH_LOCATIONSET;
            break;
        case XPATH_USERS:
            m_xPathObjectType = XPathObjectType_XPATH_USERS;
            break;
        case XPATH_XSLT_TREE:
            m_xPathObjectType = XPathObjectType_XPATH_XSLT_TREE;
            break;
        default:
            m_xPathObjectType = XPathObjectType_XPATH_UNDEFINED;
            break;
        }
    }

    /**
        get object type
    */
    XPathObjectType CXPathObject::getObjectType() throw (RuntimeException)
    {
        return m_xPathObjectType;
    }

    /**
        get the nodes from a nodelist type object
    */
    Reference< XNodeList > SAL_CALL CXPathObject::getNodeList() throw (RuntimeException)
    {
        Reference< XNodeList > const xRet(
            new CNodeList(m_xContextNode, m_pXPathObj));
        return xRet;
    }

     /**
        get value of a boolean object
     */
    sal_Bool SAL_CALL CXPathObject::getBoolean() throw (RuntimeException)
    {
        return (sal_Bool) xmlXPathCastToBoolean(m_pXPathObj.get());
    }

    /**
        get number as byte
    */
    sal_Int8 SAL_CALL CXPathObject::getByte() throw (RuntimeException)
    {
        return (sal_Int8) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as short
    */
    sal_Int16 SAL_CALL CXPathObject::getShort() throw (RuntimeException)
    {
        return (sal_Int16) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as long
    */
    sal_Int32 SAL_CALL CXPathObject::getLong() throw (RuntimeException)
    {
        return (sal_Int32) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as hyper
    */
    sal_Int64 SAL_CALL CXPathObject::getHyper() throw (RuntimeException)
    {
        return (sal_Int64) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as float
    */
    float SAL_CALL CXPathObject::getFloat() throw (RuntimeException)
    {
        return (float) xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get number as double
    */
    double SAL_CALL CXPathObject::getDouble() throw (RuntimeException)
    {
        return  xmlXPathCastToNumber(m_pXPathObj.get());
    }

    /**
        get string value
    */
    OUString SAL_CALL CXPathObject::getString() throw (RuntimeException)
    {
        const sal_Char* x1 = (sal_Char*) xmlXPathCastToString(m_pXPathObj.get());
        return OUString(x1, strlen(x1), RTL_TEXTENCODING_UTF8);
    }

}

