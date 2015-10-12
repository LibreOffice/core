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
#include "comphelper_services.hxx"

#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase.hxx>
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

class AnyCompare : public ::cppu::WeakImplHelper< XAnyCompare >
{
    Reference< XCollator > m_xCollator;

public:
    AnyCompare( Reference< XComponentContext > xContext, const Locale& rLocale )
    {
        m_xCollator = Collator::create( xContext );
        m_xCollator->loadDefaultCollator( rLocale,
                                          0 ); //???
    }

    virtual sal_Int16 SAL_CALL compare( const Any& any1, const Any& any2 ) throw(RuntimeException, std::exception) override;
};

class AnyCompareFactory : public cppu::WeakImplHelper< XAnyCompareFactory, XInitialization, XServiceInfo >
{
    Reference< XAnyCompare >            m_xAnyCompare;
    Reference< XComponentContext >      m_xContext;
    Locale                              m_Locale;

public:
    explicit AnyCompareFactory( Reference< XComponentContext > xContext ) : m_xContext( xContext )
    {}

    // XAnyCompareFactory
    virtual Reference< XAnyCompare > SAL_CALL createAnyCompareByName ( const OUString& aPropertyName ) throw(css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
            throw ( Exception, RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException, std::exception) override;

    // XServiceInfo - static versions (used for component registration)
    static OUString SAL_CALL getImplementationName_static();
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
    static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& );
};

sal_Int16 SAL_CALL AnyCompare::compare( const Any& any1, const Any& any2 ) throw(css::uno::RuntimeException, std::exception)
{
    sal_Int16 aResult = 0;

    OUString aStr1;
    OUString aStr2;

    any1 >>= aStr1;
    any2 >>= aStr2;

    aResult = static_cast<sal_Int16>(m_xCollator->compareString(aStr1, aStr2));

    return aResult;
}

Reference< XAnyCompare > SAL_CALL AnyCompareFactory::createAnyCompareByName( const OUString& aPropertyName ) throw(css::uno::RuntimeException, std::exception)
{
    // for now only OUString properties compare is implemented
    // so no check for the property name is done

    if( aPropertyName == "Title" )
        return m_xAnyCompare;

    return Reference< XAnyCompare >();
}

void SAL_CALL AnyCompareFactory::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException, std::exception )
{
    if( aArguments.getLength() )
    {
        if( aArguments[0] >>= m_Locale )
        {
            m_xAnyCompare = new AnyCompare( m_xContext, m_Locale );
            return;
        }
    }
}

OUString SAL_CALL AnyCompareFactory::getImplementationName(  ) throw( RuntimeException, std::exception )
{
    return getImplementationName_static();
}

OUString SAL_CALL AnyCompareFactory::getImplementationName_static(  )
{
    return OUString( "AnyCompareFactory" );
}

sal_Bool SAL_CALL AnyCompareFactory::supportsService( const OUString& ServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL AnyCompareFactory::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
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
    return static_cast<cppu::OWeakObject*>(new AnyCompareFactory( rxContext ));
}

void createRegistryInfo_AnyCompareFactory()
{
    static ::comphelper::module::OAutoRegistration< AnyCompareFactory > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
