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

#ifndef INCLUDED_PACKAGE_INC_ZIPFILEACCESS_HXX
#define INCLUDED_PACKAGE_INC_ZIPFILEACCESS_HXX

#include <com/sun/star/packages/zip/XZipFileAccess2.hpp>
#include <com/sun/star/packages/zip/ZipException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase.hxx>

#include <mutexholder.hxx>

#include <ZipFile.hxx>
#include <HashMaps.hxx>

class OZipFileAccess : public ::cppu::WeakImplHelper<
                        css::packages::zip::XZipFileAccess2,
                        css::lang::XInitialization,
                        css::lang::XComponent,
                        css::lang::XServiceInfo >
{
    rtl::Reference<SotMutexHolder> m_aMutexHolder;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::io::XInputStream > m_xContentStream;
    ZipFile* m_pZipFile;
    ::cppu::OInterfaceContainerHelper* m_pListenersContainer;
    bool m_bDisposed;
    bool m_bOwnContent;

public:
    OZipFileAccess( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~OZipFileAccess();

    static css::uno::Sequence< OUString > GetPatternsFromString_Impl( const OUString& aString );

    static bool StringGoodForPattern_Impl( const OUString& aString,
                                        const css::uno::Sequence< OUString >& aPattern );

    static css::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static OUString SAL_CALL impl_staticGetImplementationName();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_staticCreateSelfInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMSF );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XZipFileAccess
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getStreamByPattern( const OUString& aPattern ) throw (css::container::NoSuchElementException, css::io::IOException, css::uno::RuntimeException, css::packages::WrongPasswordException, css::packages::zip::ZipException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
