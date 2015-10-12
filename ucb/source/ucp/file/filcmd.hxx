/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILCMD_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILCMD_HXX

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

        explicit XCommandInfo_impl( shell* pMyShell );

        virtual ~XCommandInfo_impl();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL
        acquire(
            void )
            throw() override;

        virtual void SAL_CALL
        release(
            void )
            throw() override;

        // XCommandInfo

        virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo > SAL_CALL
        getCommands(
            void )
            throw( com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::ucb::CommandInfo SAL_CALL
        getCommandInfoByName(
            const OUString& Name )
            throw( com::sun::star::ucb::UnsupportedCommandException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        virtual com::sun::star::ucb::CommandInfo SAL_CALL
        getCommandInfoByHandle(
            sal_Int32 Handle )
            throw( com::sun::star::ucb::UnsupportedCommandException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL
        hasCommandByName(
            const OUString& Name )
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL
        hasCommandByHandle(
            sal_Int32 Handle )
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;


    private:

        shell*                                                                  m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider > m_xProvider;
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
