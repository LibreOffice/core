/*************************************************************************
 *
 *  $RCSfile: xpathobject.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:32:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

