/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: libxmlutil.hxx,v $
 * $Revision: 1.5 $
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
