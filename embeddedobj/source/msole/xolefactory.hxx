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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_XOLEFACTORY_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_XOLEFACTORY_HXX

#include <com/sun/star/embed/XEmbeddedObjectCreator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>


class OleEmbeddedObjectFactory : public ::cppu::WeakImplHelper<
                                                css::embed::XEmbeddedObjectCreator,
                                                css::lang::XServiceInfo >
{
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

public:
    explicit OleEmbeddedObjectFactory(
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


    // XEmbedObjectCreator
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceInitNew( const css::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName, const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceInitFromEntry( const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, const css::uno::Sequence< css::beans::PropertyValue >& aMedDescr, const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceInitFromMediaDescriptor( const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr, const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XEmbedObjectFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceUserInit( const css::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName, const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, sal_Int32 nEntryConnectionMode, const css::uno::Sequence< css::beans::PropertyValue >& lArguments, const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XLinkCreator
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceLink( const css::uno::Reference< css::embed::XStorage >& xStorage, const OUString& sEntName, const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr, const css::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
