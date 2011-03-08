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
#ifndef _FILPRP_HXX_
#define _FILPRP_HXX_

#include <ucbhelper/macros.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>


namespace fileaccess {

    class shell;

    class XPropertySetInfo_impl
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::beans::XPropertySetInfo
    {
    public:
        XPropertySetInfo_impl( shell* pMyShell,const rtl::OUString& aUnqPath );
        XPropertySetInfo_impl( shell* pMyShell,const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq );

        virtual ~XPropertySetInfo_impl();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()

        virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property > SAL_CALL
        getProperties(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::beans::Property SAL_CALL
        getPropertyByName(
            const rtl::OUString& aName )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        hasPropertyByName( const rtl::OUString& Name )
            throw( com::sun::star::uno::RuntimeException );

    private:
        shell*                                                                     m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >    m_xProvider;
        sal_Int32                                                                  m_count;
        com::sun::star::uno::Sequence< com::sun::star::beans::Property >           m_seq;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
