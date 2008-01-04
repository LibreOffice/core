/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AnyCompareFactory.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 16:36:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"


#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef __SGI_STL_MAP
#include <map>
#endif


using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace rtl;

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
                xFactory->createInstanceWithContext( OUString::createFromAscii( "com.sun.star.i18n.Collator" ), xContext ),
                        UNO_QUERY );
        m_rCollator->loadDefaultCollator( rLocale,
                                          0 ); //???
        }

    }

    virtual sal_Int16 SAL_CALL compare( const Any& any1, const Any& any2 ) throw(RuntimeException);
};

//=============================================================================

Sequence< rtl::OUString > SAL_CALL AnyCompareFactory_getSupportedServiceNames() throw();
rtl::OUString SAL_CALL AnyCompareFactory_getImplementationName() throw();
Reference< XInterface > SAL_CALL AnyCompareFactory_createInstance(
                const Reference< XComponentContext >& rxContext ) throw( Exception );

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

    if( aPropertyName.equals( OUString::createFromAscii( "Title" ) ) )
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
    return AnyCompareFactory_getImplementationName();
}

sal_Bool SAL_CALL AnyCompareFactory::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.NamedPropertyValues" ) );
    return aServiceName == ServiceName;
}

Sequence< OUString > SAL_CALL AnyCompareFactory::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return AnyCompareFactory_getSupportedServiceNames();
}


Sequence< rtl::OUString > SAL_CALL AnyCompareFactory_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.AnyCompareFactory" ) );
    const Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

rtl::OUString SAL_CALL AnyCompareFactory_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AnyCompareFactory" ) );
}

Reference< XInterface > SAL_CALL AnyCompareFactory_createInstance(
                const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return (cppu::OWeakObject*)new AnyCompareFactory( rxContext );
}
