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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_XDIALOGCREATOR_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_XDIALOGCREATOR_HXX

#include <com/sun/star/embed/XInsertObjectDialog.hpp>
#include <com/sun/star/embed/XEmbedObjectClipboardCreator.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>


class MSOLEDialogObjectCreator : public ::cppu::WeakImplHelper<
                                                css::embed::XInsertObjectDialog,
                                                css::embed::XEmbedObjectClipboardCreator,
                                                css::lang::XServiceInfo >
{
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

public:
    MSOLEDialogObjectCreator(
        const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
    : m_xFactory( xFactory )
    {
        OSL_ENSURE( xFactory.is(), "No service manager is provided!\n" );
    }

    static css::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static OUString SAL_CALL impl_staticGetImplementationName();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );


    // XInsertObjectDialog
    virtual css::embed::InsertedObjectInfo SAL_CALL createInstanceByDialog( const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException);

    // XEmbedObjectClipboardCreator
    virtual css::embed::InsertedObjectInfo SAL_CALL createInstanceInitFromClipboard( const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntryName, const css::uno::Sequence< css::beans::PropertyValue >& aObjectArgs ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
