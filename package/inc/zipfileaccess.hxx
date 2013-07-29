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

#ifndef _ZIPFILEACCESS_HXX_
#define _ZIPFILEACCESS_HXX_

#include <com/sun/star/packages/zip/XZipFileAccess2.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase4.hxx>

#include <mutexholder.hxx>

#include <ZipFile.hxx>
#include <HashMaps.hxx>

class OZipFileAccess : public ::cppu::WeakImplHelper4<
                        ::com::sun::star::packages::zip::XZipFileAccess2,
                        ::com::sun::star::lang::XInitialization,
                        ::com::sun::star::lang::XComponent,
                        ::com::sun::star::lang::XServiceInfo >
{
    SotMutexHolderRef m_aMutexHolder;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xContentStream;
    ZipFile* m_pZipFile;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer;

    sal_Bool m_bDisposed;

public:
    OZipFileAccess( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    virtual ~OZipFileAccess();

    ::com::sun::star::uno::Sequence< OUString > GetPatternsFromString_Impl( const OUString& aString );

    sal_Bool StringGoodForPattern_Impl( const OUString& aString,
                                        const ::com::sun::star::uno::Sequence< OUString >& aPattern );

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static OUString SAL_CALL impl_staticGetImplementationName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF );

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

    // XZipFileAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getStreamByPattern( const OUString& aPattern ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
