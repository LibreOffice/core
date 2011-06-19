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
#ifndef _FILID_HXX_
#define _FILID_HXX_

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>

namespace fileaccess {

    class shell;

    class FileContentIdentifier :
        public cppu::OWeakObject,
        public com::sun::star::lang::XTypeProvider,
        public com::sun::star::ucb::XContentIdentifier
    {

        // This implementation has to be reworked
    public:
        FileContentIdentifier( shell* pMyShell,
                               const rtl::OUString& aUnqPath,
                               sal_Bool IsNormalized = true );

        virtual ~FileContentIdentifier();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();

        // XTypeProvider
        virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
        getTypes(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId(
            void )
            throw( com::sun::star::uno::RuntimeException );

        // XContentIdentifier
        virtual rtl::OUString SAL_CALL
        getContentIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL
        getContentProviderScheme(
            void )
            throw( com::sun::star::uno::RuntimeException );

    private:
        shell* m_pMyShell;
        rtl::OUString m_aContentId;              // The URL string
        rtl::OUString m_aNormalizedId;           // The somehow normalized string
        rtl::OUString m_aProviderScheme;
        sal_Bool      m_bNormalized;
    };

} // end namespace fileaccess


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
