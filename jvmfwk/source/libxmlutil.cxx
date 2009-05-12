/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: libxmlutil.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_jvmfwk.hxx"
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
CXmlCharPtr::CXmlCharPtr(xmlChar * aChar)
    : _object(aChar)
{
}

CXmlCharPtr::CXmlCharPtr(const ::rtl::OUString & s):
    _object(NULL)
{
    ::rtl::OString o = ::rtl::OUStringToOString(s, RTL_TEXTENCODING_UTF8);
    _object = xmlCharStrdup(o.getStr());
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

CXmlCharPtr::operator xmlChar*()
{
    return _object;
}

CXmlCharPtr::operator ::rtl::OUString()
{
    ::rtl::OUString ret;
    if (_object != NULL)
    {
        ::rtl::OString aOStr((sal_Char*)_object);
        ret = ::rtl::OStringToOUString(aOStr, RTL_TEXTENCODING_UTF8);
    }
    return ret;
}

CXmlCharPtr::operator ::rtl::OString()
{
    return ::rtl::OString((sal_Char*) _object);
}



}
