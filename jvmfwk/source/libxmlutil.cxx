/*************************************************************************
 *
 *  $RCSfile: libxmlutil.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2004-04-27 15:22:15 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "libxmlutil.hxx"

namespace jfw
{

CXPathObjectPtr::CXPathObjectPtr(xmlXPathObject* aObject)
    : _object(aObject)
{
}

CXPathObjectPtr::CXPathObjectPtr():_object(NULL)
{
}

CXPathObjectPtr::~CXPathObjectPtr()
{
    xmlXPathFreeObject(_object);
}
CXPathObjectPtr & CXPathObjectPtr::operator = (xmlXPathObject* pObj)
{
    if (_object == pObj)
        return *this;

    xmlXPathFreeObject(_object);
    _object = pObj;
    return *this;
}
xmlXPathObject* CXPathObjectPtr::operator ->()

{
    return _object;
}
CXPathObjectPtr::operator xmlXPathObject*()
{
    return _object;
}
//===========================================================
CXPathContextPtr::CXPathContextPtr(xmlXPathContextPtr aContext)
    : _object(aContext)
{
}

CXPathContextPtr::CXPathContextPtr():_object(NULL)
{
}

CXPathContextPtr::~CXPathContextPtr()
{
    xmlXPathFreeContext(_object);
}

CXPathContextPtr & CXPathContextPtr::operator = (xmlXPathContextPtr pObj)
{
    if (_object == pObj)
        return *this;
    xmlXPathFreeContext(_object);
    _object = pObj;
    return *this;
}
xmlXPathContext* CXPathContextPtr::operator ->()
{
    return _object;
}

CXPathContextPtr::operator xmlXPathContext*()
{
    return _object;
}
//===========================================================
CXmlDocPtr::CXmlDocPtr(xmlDoc* aDoc)
    : _object(aDoc)
{
}

CXmlDocPtr::CXmlDocPtr():_object(NULL)
{
}

CXmlDocPtr::~CXmlDocPtr()
{
    xmlFreeDoc(_object);
}
CXmlDocPtr & CXmlDocPtr::operator = (xmlDoc* pObj)
{
    if (_object == pObj)
        return *this;
    xmlFreeDoc(_object);
    _object = pObj;
    return *this;
}

xmlDoc* CXmlDocPtr::operator ->()
{
    return _object;
}

CXmlDocPtr::operator xmlDoc*()
{
    return _object;
}

//===========================================================
// CXmlNsPtr::CXmlNsPtr(xmlNs* pNs)
//     : _object(pNs)
// {
// }

// CXmlNsPtr::CXmlNsPtr():_object(NULL)
// {
// }

// CXmlNsPtr::~CXmlNsPtr()
// {
// //    xmlFreeNs(_object);
// }
// CXmlNsPtr & CXmlNsPtr::operator = (xmlNs* pObj)
// {
//     if (_object == pObj)
//         return *this;
//     xmlFreeNs(_object);
//     _object = pObj;
//     return *this;
// }

// xmlNs* CXmlNsPtr::operator ->()
// {
//     return _object;
// }

// CXmlNsPtr::operator xmlNs*()
// {
//     return _object;
// }

//===========================================================
CXmlCharPtr::CXmlCharPtr(xmlChar * aChar)
    : _object(aChar)
{
}

CXmlCharPtr::CXmlCharPtr():_object(NULL)
{
}

CXmlCharPtr::~CXmlCharPtr()
{
    xmlFree(_object);
}

CXmlCharPtr & CXmlCharPtr::operator = (xmlChar* pObj)
{
    if (pObj == _object)
        return *this;
    xmlFree(_object);
    _object = pObj;
    return *this;
}
// xmlChar* CXmlCharPtr::operator ->()
// {
//     return _object;
// }

CXmlCharPtr::operator xmlChar*()
{
    return _object;
}

CXmlCharPtr::operator rtl::OUString()
{
    rtl::OUString ret;
    if (_object != NULL)
    {
        rtl::OString aOStr((sal_Char*)_object);
        ret = rtl::OStringToOUString(aOStr, RTL_TEXTENCODING_UTF8);
    }
    return ret;
}

CXmlCharPtr::operator rtl::OString()
{
    return rtl::OString((sal_Char*) _object);
}



}
