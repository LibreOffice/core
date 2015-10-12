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
#ifndef INCLUDED_JVMFWK_SOURCE_LIBXMLUTIL_HXX
#define INCLUDED_JVMFWK_SOURCE_LIBXMLUTIL_HXX

#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "rtl/ustring.hxx"

namespace jfw
{
class CXPathObjectPtr
{
    xmlXPathObject* _object;
    CXPathObjectPtr & operator = (const CXPathObjectPtr&) = delete;
    CXPathObjectPtr(const CXPathObjectPtr&) = delete;
public:
    CXPathObjectPtr();
    ~CXPathObjectPtr();
    /** Takes ownership of xmlXPathObject
     */
    CXPathObjectPtr & operator = (xmlXPathObject* pObj);
    xmlXPathObject* operator -> () { return _object;}
    operator xmlXPathObject* () const { return _object;}
};


class CXPathContextPtr
{
    xmlXPathContext* _object;

    CXPathContextPtr(const jfw::CXPathContextPtr&) = delete;
    CXPathContextPtr & operator = (const CXPathContextPtr&) = delete;
public:
    CXPathContextPtr();
    CXPathContextPtr(xmlXPathContextPtr aContext);
    CXPathContextPtr & operator = (xmlXPathContextPtr pObj);
    ~CXPathContextPtr();
    xmlXPathContext* operator -> () { return _object;}
    operator xmlXPathContext* () const { return _object;}
};


class CXmlDocPtr
{
    xmlDoc* _object;

    CXmlDocPtr(const CXmlDocPtr&) = delete;

public:
    CXmlDocPtr & operator = (const CXmlDocPtr&);
    CXmlDocPtr();
    CXmlDocPtr(xmlDoc* aDoc);
    /** Takes ownership of xmlDoc
     */
    CXmlDocPtr & operator = (xmlDoc* pObj);
    ~CXmlDocPtr();
    xmlDoc* operator -> () { return _object;}
    operator xmlDoc* () const { return _object;}
};


class CXmlCharPtr
{
    xmlChar* _object;

    CXmlCharPtr(const CXmlCharPtr&) = delete;
    CXmlCharPtr & operator = (const CXmlCharPtr&) = delete;
public:
    CXmlCharPtr();
    CXmlCharPtr(xmlChar* aDoc);
    CXmlCharPtr(const OUString &);
    ~CXmlCharPtr();
    CXmlCharPtr & operator = (xmlChar* pObj);
    operator xmlChar* () const { return _object;}
    operator OUString ();
    operator OString () { return OString(reinterpret_cast<sal_Char*>(_object));}
};


}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
