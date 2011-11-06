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


#if !defined INCLUDED_JVMFWK_LIBXMLUTIL_HXX
#define INCLUDED_JVMFWK_LIBXMLUTIL_HXX


#include "libxml/parser.h"
#include "libxml/xpath.h"
//#include "libxml/xpathinternals.h"
#include "rtl/ustring.hxx"
namespace jfw
{
class CXPathObjectPtr
{
    xmlXPathObject* _object;
    CXPathObjectPtr & operator = (const CXPathObjectPtr&);
    CXPathObjectPtr(const CXPathObjectPtr&);
public:
    CXPathObjectPtr();
    /** Takes ownership of xmlXPathObject
     */
    CXPathObjectPtr(xmlXPathObject* aObject);
    ~CXPathObjectPtr();
    /** Takes ownership of xmlXPathObject
     */
    CXPathObjectPtr & operator = (xmlXPathObject* pObj);
    xmlXPathObject* operator -> ();
    operator xmlXPathObject* ();
};

//===========================================================
class CXPathContextPtr
{
    xmlXPathContext* _object;

    CXPathContextPtr(const jfw::CXPathContextPtr&);
    CXPathContextPtr & operator = (const CXPathContextPtr&);
public:
    CXPathContextPtr();
    CXPathContextPtr(xmlXPathContextPtr aContext);
    CXPathContextPtr & operator = (xmlXPathContextPtr pObj);
    ~CXPathContextPtr();
    xmlXPathContext* operator -> ();
    operator xmlXPathContext* ();
};

//===========================================================
class CXmlDocPtr
{
    xmlDoc* _object;

    CXmlDocPtr(const CXmlDocPtr&);

public:
    CXmlDocPtr & operator = (const CXmlDocPtr&);
    CXmlDocPtr();
    CXmlDocPtr(xmlDoc* aDoc);
    /** Takes ownership of xmlDoc
     */
    CXmlDocPtr & operator = (xmlDoc* pObj);
    ~CXmlDocPtr();
    xmlDoc* operator -> ();
    operator xmlDoc* ();
};

//===========================================================
// class CXmlNsPtr
// {
//     xmlNs* _object;

//     CXmlNsPtr(const CXmlNsPtr&);
//     CXmlNsPtr & operator = (const CXmlNsPtr&);
// public:
//     CXmlNsPtr();
//     CXmlNsPtr(xmlNs* aDoc);
//     /** Takes ownership of xmlDoc
//      */
//     CXmlNsPtr & operator = (xmlNs* pObj);
//     ~CXmlNsPtr();
//     xmlNs* operator -> ();
//     operator xmlNs* ();
// };

//===========================================================
class CXmlCharPtr
{
    xmlChar* _object;

    CXmlCharPtr(const CXmlCharPtr&);
    CXmlCharPtr & operator = (const CXmlCharPtr&);
public:
    CXmlCharPtr();
    CXmlCharPtr(xmlChar* aDoc);
    CXmlCharPtr(const ::rtl::OUString &);
    ~CXmlCharPtr();
    CXmlCharPtr & operator = (xmlChar* pObj);
//    xmlChar* operator -> ();
    operator xmlChar* ();
    operator ::rtl::OUString ();
    operator ::rtl::OString ();
};


}
#endif
