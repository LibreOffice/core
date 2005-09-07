/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: libxmlutil.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:36:19 $
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
    CXmlDocPtr & operator = (const CXmlDocPtr&);
public:
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
    ~CXmlCharPtr();
    CXmlCharPtr & operator = (xmlChar* pObj);
//    xmlChar* operator -> ();
    operator xmlChar* ();
    operator rtl::OUString ();
    operator rtl::OString ();
};


}
#endif
