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

#include "comphelper_module.hxx"

#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <map>

using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;

class AnyCompare : public ::cppu::WeakImplHelper1< XAnyCompare >
{
    Reference< XCollator > m_rCollator;

public:
    AnyCompare( Reference< XComponentContext > xContext, const Locale& rLocale ) throw()
    {
        m_rCollator = Collator::create( xContext );
        m_rCollator->loadDefaultCollator( rLocale,
                                          0 ); //???
    }

    virtual sal_Int16 SAL_CALL compare( const Any& any1, const Any& any2 ) throw(RuntimeException);
};

class AnyCompareFactory : public cppu::WeakImplHelper3< XAnyCompareFactory, XInitialization, XServiceInfo >
{
    Reference< XAnyCompare >            m_rAnyCompare;
    Reference< XComponentContext >      m_rContext;
    Locale                              m_Locale;

public:
    AnyCompareFactory( Reference< XComponentContext > xContext ) : m_rContext( xContext )
    {}

    // XAnyCompareFactory
    virtual Reference< XAnyCompare > SAL_CALL createAnyCompareByName ( const OUString& aPropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
            throw ( Exception, RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

    // XServiceInfo - static versions (used for component registration)
    static OUString SAL_CALL getImplementationName_static();
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
    static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& );
};

sal_Int16 SAL_CALL AnyCompare::compare( const Any& any1, const Any& any2 ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 aResult = 0;

    OUString aStr1;
    OUString aStr2;

    any1 >>= aStr1;
    any2 >>= aStr2;

    aResult = ( sal_Int16 )m_rCollator->compareString( aStr1, aStr2 );

    return aResult;
}

Reference< XAnyCompare > SAL_CALL AnyCompareFactory::createAnyCompareByName( const OUString& aPropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    // for now only OUString properties compare is implemented
    // so no check for the property name is done

    if( aPropertyName == "Title" )
        return m_rAnyCompare;

    return Reference< XAnyCompare >();
}

void SAL_CALL AnyCompareFactory::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    if( aArguments.getLength() )
    {
        if( aArguments[0] >>= m_Locale )
        {
            m_rAnyCompare = new AnyCompare( m_rContext, m_Locale );
            return;
        }
    }
}

OUString SAL_CALL AnyCompareFactory::getImplementationName(  ) throw( RuntimeException )
{
    return getImplementationName_static();
}

OUString SAL_CALL AnyCompareFactory::getImplementationName_static(  )
{
    return OUString( "AnyCompareFactory" );
}

sal_Bool SAL_CALL AnyCompareFactory::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL AnyCompareFactory::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_static();
}

Sequence< OUString > SAL_CALL AnyCompareFactory::getSupportedServiceNames_static(  )
{
    const OUString aServiceName( "com.sun.star.ucb.AnyCompareFactory" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL AnyCompareFactory::Create(
                const Reference< XComponentContext >& rxContext )
{
    return (cppu::OWeakObject*)new AnyCompareFactory( rxContext );
}

void createRegistryInfo_AnyCompareFactory()
{
    static ::comphelper::module::OAutoRegistration< AnyCompareFactory > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
