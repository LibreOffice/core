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

#ifndef __XDIALOGCREATOR_HXX_
#define __XDIALOGCREATOR_HXX_

#include <com/sun/star/embed/XInsertObjectDialog.hpp>
#include <com/sun/star/embed/XEmbedObjectClipboardCreator.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>


#include <cppuhelper/implbase3.hxx>


class MSOLEDialogObjectCreator : public ::cppu::WeakImplHelper3<
                                                ::com::sun::star::embed::XInsertObjectDialog,
                                                ::com::sun::star::embed::XEmbedObjectClipboardCreator,
                                                ::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

public:
    MSOLEDialogObjectCreator(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : m_xFactory( xFactory )
    {
        OSL_ENSURE( xFactory.is(), "No service manager is provided!\n" );
    }

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static OUString SAL_CALL impl_staticGetImplementationName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );


    // XInsertObjectDialog
    virtual ::com::sun::star::embed::InsertedObjectInfo SAL_CALL createInstanceByDialog( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XEmbedObjectClipboardCreator
    virtual ::com::sun::star::embed::InsertedObjectInfo SAL_CALL createInstanceInitFromClipboard( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntryName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aObjectArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
