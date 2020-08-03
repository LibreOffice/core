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

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>

#include "SalAquaPicker.hxx"

#include <rtl/ustring.hxx>




class SalAquaFolderPicker :
        public SalAquaPicker,
    public cppu::WeakImplHelper<
    css::ui::dialogs::XFolderPicker2,
    css::lang::XServiceInfo,
    css::lang::XEventListener >
{
public:

    // constructor
    SalAquaFolderPicker();


    // XExecutableDialog functions


    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;

    virtual sal_Int16 SAL_CALL execute(  ) override;


    // XFolderPicker functions


    virtual void SAL_CALL setDisplayDirectory( const OUString& rDirectory ) override;

    virtual OUString SAL_CALL getDisplayDirectory(  ) override;

    virtual OUString SAL_CALL getDirectory( ) override;

    virtual void SAL_CALL setDescription( const OUString& rDescription ) override;


    // XServiceInfo


    virtual OUString SAL_CALL getImplementationName(  ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;


    // XCancellable


    virtual void SAL_CALL cancel( ) override;


    // XEventListener


    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;

private:
    SalAquaFolderPicker( const SalAquaFolderPicker& ) = delete;
    SalAquaFolderPicker& operator=( const SalAquaFolderPicker& ) = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
