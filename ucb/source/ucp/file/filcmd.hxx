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
#pragma once

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/ucb/XCommandInfo.hpp>


namespace fileaccess {


    // forward
    class TaskManager;


    class XCommandInfo_impl
        : public cppu::OWeakObject,
          public css::ucb::XCommandInfo
    {
    public:

        explicit XCommandInfo_impl( TaskManager* pMyShell );

        virtual ~XCommandInfo_impl() override;

        // XInterface
        virtual css::uno::Any SAL_CALL
        queryInterface( const css::uno::Type& aType ) override;

        virtual void SAL_CALL
        acquire()
            throw() override;

        virtual void SAL_CALL
        release()
            throw() override;

        // XCommandInfo

        virtual css::uno::Sequence< css::ucb::CommandInfo > SAL_CALL
        getCommands() override;

        virtual css::ucb::CommandInfo SAL_CALL
        getCommandInfoByName( const OUString& Name ) override;

        virtual css::ucb::CommandInfo SAL_CALL
        getCommandInfoByHandle( sal_Int32 Handle ) override;

        virtual sal_Bool SAL_CALL
        hasCommandByName( const OUString& Name ) override;

        virtual sal_Bool SAL_CALL
        hasCommandByHandle( sal_Int32 Handle ) override;


    private:

        TaskManager*                                            m_pMyShell;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
