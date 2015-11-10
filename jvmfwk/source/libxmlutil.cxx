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

CXPathObjectPtr::CXPathObjectPtr():_object(nullptr)
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

CXPathContextPtr::CXPathContextPtr(xmlXPathContextPtr aContext)
    : _object(aContext)
{
}

CXPathContextPtr::CXPathContextPtr():_object(nullptr)
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


CXmlDocPtr::CXmlDocPtr(xmlDoc* aDoc)
    : _object(aDoc)
{
}

CXmlDocPtr::CXmlDocPtr():_object(nullptr)
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




CXmlCharPtr::CXmlCharPtr(xmlChar * aChar)
    : _object(aChar)
{
}

CXmlCharPtr::CXmlCharPtr(const OUString & s):
    _object(nullptr)
{
    OString o = OUStringToOString(s, RTL_TEXTENCODING_UTF8);
    _object = xmlCharStrdup(o.getStr());
}
CXmlCharPtr::CXmlCharPtr():_object(nullptr)
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


CXmlCharPtr::operator OUString()
{
    OUString ret;
    if (_object != nullptr)
    {
        OString aOStr(reinterpret_cast<char*>(_object));
        ret = OStringToOUString(aOStr, RTL_TEXTENCODING_UTF8);
    }
    return ret;
}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
