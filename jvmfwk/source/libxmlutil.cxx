/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
