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
#ifndef _FILCMD_HXX_
#define _FILCMD_HXX_

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>


namespace fileaccess {


    // forward
    class shell;


    class XCommandInfo_impl
        : public cppu::OWeakObject,
          public com::sun::star::ucb::XCommandInfo
    {
    public:

        XCommandInfo_impl( shell* pMyShell );

        virtual ~XCommandInfo_impl();

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

        // XCommandInfo

        virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo > SAL_CALL
        getCommands(
            void )
            throw( com::sun::star::uno::RuntimeException);

        virtual com::sun::star::ucb::CommandInfo SAL_CALL
        getCommandInfoByName(
            const rtl::OUString& Name )
            throw( com::sun::star::ucb::UnsupportedCommandException,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::ucb::CommandInfo SAL_CALL
        getCommandInfoByHandle(
            sal_Int32 Handle )
            throw( com::sun::star::ucb::UnsupportedCommandException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasCommandByName(
            const rtl::OUString& Name )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasCommandByHandle(
            sal_Int32 Handle )
            throw( com::sun::star::uno::RuntimeException );


    private:

        shell*                                                                  m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider > m_xProvider;
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
