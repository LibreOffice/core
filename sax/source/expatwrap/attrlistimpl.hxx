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

#ifndef _SAX_ATTRLISTIMPL_HXX
#define _SAX_ATTRLISTIMPL_HXX

#include "sal/config.h"

#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

namespace sax_expatwrap
{

struct AttributeList_impl;

//FIXME
class /*SAX_DLLPUBLIC*/ AttributeList :
    public ::cppu::WeakImplHelper2<
                ::com::sun::star::xml::sax::XAttributeList,
                ::com::sun::star::util::XCloneable >
{
public:
    AttributeList();
    AttributeList( const AttributeList & );
    virtual ~AttributeList();

    void addAttribute( const ::rtl::OUString &sName ,
        const ::rtl::OUString &sType , const ::rtl::OUString &sValue );
    void clear();
public:
    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength(void)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByName(const ::rtl::OUString& aName)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByName(const ::rtl::OUString& aName)
        throw( ::com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< XCloneable > SAL_CALL
        createClone()   throw(::com::sun::star::uno::RuntimeException);

private:
    struct AttributeList_impl *m_pImpl;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
