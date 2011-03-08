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

#ifndef SC_XMLCODENAMEPROVIDER_HXX
#define SC_XMLCODENAMEPROVIDER_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase1.hxx>

class ScDocument;
class String;

class XMLCodeNameProvider : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XNameAccess >
{
    ScDocument* mpDoc;
    ::rtl::OUString msDocName;
    ::rtl::OUString msCodeNameProp;

    static sal_Bool _getCodeName( const ::com::sun::star::uno::Any& aAny,
                           String& rCodeName );

public:
    XMLCodeNameProvider( ScDocument* pDoc );
    virtual ~XMLCodeNameProvider();

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException );

    static void set( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& xNameAccess, ScDocument *pDoc );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
