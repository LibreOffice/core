/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xpathobject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:09:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <string.h>
#include "xpathobject.hxx"
#include "nodelist.hxx"

namespace XPath
{
    CXPathObject::CXPathObject(const xmlXPathObjectPtr xpathObj)
        : m_pXPathObj(xpathObj)
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
        return Reference< XNodeList >(new CNodeList(m_pXPathObj));
    }

     /**
        get value of a boolean object
     */
    sal_Bool SAL_CALL CXPathObject::getBoolean() throw (RuntimeException)
    {
        return (sal_Bool) xmlXPathCastToBoolean(m_pXPathObj);
    }

    /**
        get number as byte
    */
    sal_Int8 SAL_CALL CXPathObject::getByte() throw (RuntimeException)
    {
        return (sal_Int8) xmlXPathCastToNumber(m_pXPathObj);
    }

    /**
        get number as short
    */
    sal_Int16 SAL_CALL CXPathObject::getShort() throw (RuntimeException)
    {
        return (sal_Int16) xmlXPathCastToNumber(m_pXPathObj);
   }

    /**
        get number as long
    */
    sal_Int32 SAL_CALL CXPathObject::getLong() throw (RuntimeException)
    {
        return (sal_Int32) xmlXPathCastToNumber(m_pXPathObj);
    }

    /**
        get number as hyper
    */
    sal_Int64 SAL_CALL CXPathObject::getHyper() throw (RuntimeException)
    {
        return (sal_Int64) xmlXPathCastToNumber(m_pXPathObj);
    }

    /**
        get number as float
    */
    float SAL_CALL CXPathObject::getFloat() throw (RuntimeException)
    {
        return (float) xmlXPathCastToNumber(m_pXPathObj);
    }

    /**
        get number as double
    */
    double SAL_CALL CXPathObject::getDouble() throw (RuntimeException)
    {
        return  xmlXPathCastToNumber(m_pXPathObj);
    }

    /**
        get string value
    */
    OUString SAL_CALL CXPathObject::getString() throw (RuntimeException)
    {
        const sal_Char* x1 = (sal_Char*) xmlXPathCastToString(m_pXPathObj);
        return OUString(x1, strlen(x1), RTL_TEXTENCODING_UTF8);
    }

}

