/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include "comphelper_module.hxx"

#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <comphelper/stl_types.hxx>
#include <map>


using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;

using ::rtl::OUString;

//=============================================================================

class AnyCompare : public ::cppu::WeakImplHelper1< XAnyCompare >
{
    Reference< XCollator > m_rCollator;

public:
    AnyCompare( Reference< XComponentContext > xContext, const Locale& rLocale ) throw()
    {
        Reference< XMultiComponentFactory > xFactory = xContext->getServiceManager();
        if ( xFactory.is() )
    {
        m_rCollator = Reference< XCollator >(
                xFactory->createInstanceWithContext( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.Collator" )), xContext ),
                        UNO_QUERY );
        m_rCollator->loadDefaultCollator( rLocale,
                                          0 ); //???
        }

    }

    virtual sal_Int16 SAL_CALL compare( const Any& any1, const Any& any2 ) throw(RuntimeException);
};

//=============================================================================

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
    static ::rtl::OUString SAL_CALL getImplementationName_static();
    static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
    static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& );
};

//===========================================================================================

sal_Int16 SAL_CALL AnyCompare::compare( const Any& any1, const Any& any2 ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 aResult = 0;

    if( m_rCollator.is() )
    {
        OUString aStr1;
        OUString aStr2;

        any1 >>= aStr1;
        any2 >>= aStr2;

        aResult = ( sal_Int16 )m_rCollator->compareString( aStr1, aStr2 );
    }

    return aResult;
}

//===========================================================================================

Reference< XAnyCompare > SAL_CALL AnyCompareFactory::createAnyCompareByName( const OUString& aPropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    // for now only OUString properties compare is implemented
    // so no check for the property name is done

    if( aPropertyName.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" )) ) )
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
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AnyCompareFactory" ) );
}

sal_Bool SAL_CALL AnyCompareFactory::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.AnyCompareFactory" ) );
    return aServiceName == ServiceName;
}

Sequence< OUString > SAL_CALL AnyCompareFactory::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_static();
}

Sequence< OUString > SAL_CALL AnyCompareFactory::getSupportedServiceNames_static(  )
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.AnyCompareFactory" ) );
    const Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
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
