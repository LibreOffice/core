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

#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <sfx2/objsh.hxx>

using namespace css;

namespace {

class OwnSubFilterService : public cppu::WeakImplHelper < document::XFilter
                                                        ,lang::XServiceInfo >
{
    uno::Reference< frame::XModel > m_xModel;
    uno::Reference< io::XStream > m_xStream;
    SfxObjectShell* m_pObjectShell;

public:
    explicit OwnSubFilterService(const css::uno::Sequence< css::uno::Any >& aArguments)
        throw (uno::Exception, uno::RuntimeException);

    virtual ~OwnSubFilterService();

    // XFilter
    virtual sal_Bool SAL_CALL filter( const uno::Sequence< beans::PropertyValue >& aDescriptor ) throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel() throw (uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (uno::RuntimeException, std::exception) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (uno::RuntimeException, std::exception) override;
};

OwnSubFilterService::OwnSubFilterService(const css::uno::Sequence< css::uno::Any >& aArguments)
    throw (uno::Exception, uno::RuntimeException)
    : m_pObjectShell( nullptr )
{
    if ( aArguments.getLength() != 2 )
        throw lang::IllegalArgumentException();

    if ( m_pObjectShell )
        throw frame::DoubleInitializationException();

    if ( ( aArguments[1] >>= m_xStream ) && m_xStream.is()
      && ( aArguments[0] >>= m_xModel ) && m_xModel.is() )
    {
        css::uno::Reference < css::lang::XUnoTunnel > xObj( m_xModel, uno::UNO_QUERY_THROW );
        css::uno::Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
        sal_Int64 nHandle = xObj->getSomething( aSeq );
        if ( nHandle )
            m_pObjectShell = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
    }

    if ( !m_pObjectShell )
        throw lang::IllegalArgumentException();
}

OwnSubFilterService::~OwnSubFilterService()
{
}

sal_Bool SAL_CALL OwnSubFilterService::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
    throw (uno::RuntimeException, std::exception)
{
    if ( !m_pObjectShell )
        throw uno::RuntimeException();

    return m_pObjectShell->ImportFromGeneratedStream_Impl( m_xStream, aDescriptor );
}

void SAL_CALL OwnSubFilterService::cancel()
    throw (uno::RuntimeException, std::exception)
{
    // not implemented
}

OUString SAL_CALL OwnSubFilterService::getImplementationName()
    throw ( uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.document.OwnSubFilter");
}

sal_Bool SAL_CALL OwnSubFilterService::supportsService( const OUString& ServiceName )
    throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL OwnSubFilterService::getSupportedServiceNames()
    throw ( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(2);
    aRet[0] = "com.sun.star.document.OwnSubFilter";
    aRet[1] = "com.sun.star.comp.document.OwnSubFilter";
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_document_OwnSubFilter_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new OwnSubFilterService(arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
