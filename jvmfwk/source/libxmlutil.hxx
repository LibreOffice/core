/*************************************************************************
 *
 *  $RCSfile: libxmlutil.hxx,v $
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
