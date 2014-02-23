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
CXPathObjectPtr::operator xmlXPathObject*() const
{
    return _object;
}

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

CXPathContextPtr::operator xmlXPathContext*() const
{
    return _object;
}

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

CXmlDocPtr::operator xmlDoc*() const
{
    return _object;
}


CXmlCharPtr::CXmlCharPtr(xmlChar * aChar)
    : _object(aChar)
{
}

CXmlCharPtr::CXmlCharPtr(const OUString & s):
    _object(NULL)
{
    OString o = OUStringToOString(s, RTL_TEXTENCODING_UTF8);
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

CXmlCharPtr::operator xmlChar*() const
{
    return _object;
}

CXmlCharPtr::operator OUString()
{
    OUString ret;
    if (_object != NULL)
    {
        OString aOStr((sal_Char*)_object);
        ret = OStringToOUString(aOStr, RTL_TEXTENCODING_UTF8);
    }
    return ret;
}

CXmlCharPtr::operator OString()
{
    return OString((sal_Char*) _object);
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
