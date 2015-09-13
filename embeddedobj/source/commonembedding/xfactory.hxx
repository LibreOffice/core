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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_COMMONEMBEDDING_XFACTORY_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_COMMONEMBEDDING_XFACTORY_HXX

#include <com/sun/star/embed/XEmbeddedObjectCreator.hpp>
#include <com/sun/star/embed/XLinkFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/implbase.hxx>
#include <comphelper/mimeconfighelper.hxx>

class OOoEmbeddedObjectFactory : public ::cppu::WeakImplHelper<
                                                ::com::sun::star::embed::XEmbeddedObjectCreator,
                                                css::embed::XLinkFactory,
                                                ::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    ::comphelper::MimeConfigurationHelper m_aConfigHelper;

public:
    explicit OOoEmbeddedObjectFactory(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
    : m_xContext( rxContext )
    , m_aConfigHelper( rxContext )
    {
        OSL_ENSURE( rxContext.is(), "No service manager is provided!\n" );
    }

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static OUString SAL_CALL impl_staticGetImplementationName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );


    // XEmbedObjectCreator
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceInitNew( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceInitFromEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceInitFromMediaDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEmbedObjectFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceUserInit( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntName, sal_Int32 nEntryConnectionMode, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLinkCreator
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceLink( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLinkFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceLinkUserInit( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};

class OOoSpecialEmbeddedObjectFactory : public ::cppu::WeakImplHelper<
                                                ::com::sun::star::embed::XEmbedObjectFactory,
                                                ::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    ::comphelper::MimeConfigurationHelper m_aConfigHelper;

public:
    explicit OOoSpecialEmbeddedObjectFactory(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
    : m_xContext( rxContext )
    , m_aConfigHelper( rxContext )
    {
        OSL_ENSURE( rxContext.is(), "No service manager is provided!\n" );
    }

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();

    static OUString SAL_CALL impl_staticGetImplementationName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

    // XEmbedObjectFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceUserInit( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID, const OUString& aClassName, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const OUString& sEntName, sal_Int32 nEntryConnectionMode, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
