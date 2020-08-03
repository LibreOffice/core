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

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/implbase.hxx>


namespace fileaccess {

    class TaskManager;

class XPropertySetInfo_impl : public cppu::WeakImplHelper<
    css::beans::XPropertySetInfo >
    {
    public:
        XPropertySetInfo_impl( TaskManager* pMyShell,const OUString& aUnqPath );
        XPropertySetInfo_impl( TaskManager* pMyShell,const css::uno::Sequence< css::beans::Property >& seq );

        virtual ~XPropertySetInfo_impl() override;

        virtual css::uno::Sequence< css::beans::Property > SAL_CALL
        getProperties() override;

        virtual css::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName ) override;

        virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name ) override;

    private:
        TaskManager*                                               m_pMyShell;
        css::uno::Sequence< css::beans::Property >           m_seq;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
