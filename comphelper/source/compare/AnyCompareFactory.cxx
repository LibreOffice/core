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

#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;

namespace {

class AnyCompare : public ::cppu::WeakImplHelper< XAnyCompare >
{
    Reference< XCollator > m_xCollator;

public:
    AnyCompare( Reference< XComponentContext > const & xContext, const Locale& rLocale )
       : m_xCollator(Collator::create( xContext ))
    {
        m_xCollator->loadDefaultCollator( rLocale,
                                          0 ); //???
    }

    virtual sal_Int16 SAL_CALL compare( const Any& any1, const Any& any2 ) override;
};

class AnyCompareFactory : public cppu::WeakImplHelper< XAnyCompareFactory, XInitialization, XServiceInfo >
{
    Reference< XAnyCompare >            m_xAnyCompare;
    Reference< XComponentContext >      m_xContext;
    Locale                              m_Locale;

public:
    explicit AnyCompareFactory( Reference< XComponentContext > const & xContext ) : m_xContext( xContext )
    {}

    // XAnyCompareFactory
    virtual Reference< XAnyCompare > SAL_CALL createAnyCompareByName ( const OUString& aPropertyName ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

}

sal_Int16 SAL_CALL AnyCompare::compare( const Any& any1, const Any& any2 )
{
    sal_Int16 aResult = 0;

    OUString aStr1;
    OUString aStr2;

    any1 >>= aStr1;
    any2 >>= aStr2;

    aResult = static_cast<sal_Int16>(m_xCollator->compareString(aStr1, aStr2));

    return aResult;
}

Reference< XAnyCompare > SAL_CALL AnyCompareFactory::createAnyCompareByName( const OUString& aPropertyName )
{
    // for now only OUString properties compare is implemented
    // so no check for the property name is done

    if( aPropertyName == "Title" )
        return m_xAnyCompare;

    return Reference< XAnyCompare >();
}

void SAL_CALL AnyCompareFactory::initialize( const Sequence< Any >& aArguments )
{
    if( aArguments.hasElements() )
    {
        if( aArguments[0] >>= m_Locale )
        {
            m_xAnyCompare = new AnyCompare( m_xContext, m_Locale );
            return;
        }
    }
}

OUString SAL_CALL AnyCompareFactory::getImplementationName(  )
{
    return u"AnyCompareFactory"_ustr;
}

sal_Bool SAL_CALL AnyCompareFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL AnyCompareFactory::getSupportedServiceNames(  )
{
    return { u"com.sun.star.ucb.AnyCompareFactory"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
AnyCompareFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new AnyCompareFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
