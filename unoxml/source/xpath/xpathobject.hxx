/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _XPATHOBJECT_HXX
#define _XPATHOBJECT_HXX

#include <map>
#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <boost/shared_ptr.hpp>

using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::xpath;

namespace XPath
{
    class CXPathObject : public cppu::WeakImplHelper1< XXPathObject >
    {
    private:
    boost::shared_ptr<xmlXPathObject> m_pXPathObj;
    const Reference< XNode > m_xContextNode;
    XPathObjectType m_xPathObjectType;

    public:
    CXPathObject(xmlXPathObjectPtr xpathObj, const Reference< XNode >& contextNode);

    /**      
        get object type
    */
    virtual XPathObjectType SAL_CALL getObjectType() throw (RuntimeException);

    /**
        get the nodes from a nodelist type object
    */
    virtual Reference< XNodeList > SAL_CALL getNodeList() throw (RuntimeException);

     /**
        get value of a boolean object
     */
     virtual sal_Bool SAL_CALL getBoolean() throw (RuntimeException);

    /**
        get number as byte
    */
    virtual sal_Int8 SAL_CALL getByte() throw (RuntimeException);

    /**
        get number as short
    */
    virtual sal_Int16 SAL_CALL getShort() throw (RuntimeException);

    /**
        get number as long
    */
    virtual sal_Int32 SAL_CALL getLong() throw (RuntimeException);

    /**
        get number as hyper
    */
    virtual sal_Int64 SAL_CALL getHyper() throw (RuntimeException);

    /**
        get number as float
    */
    virtual float SAL_CALL getFloat() throw (RuntimeException);

    /**
        get number as double
    */
    virtual double SAL_CALL getDouble() throw (RuntimeException);

    /**
        get string value
    */
    virtual OUString SAL_CALL getString() throw (RuntimeException);
    
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
