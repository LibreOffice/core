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
#include "precompiled_xmlhelp.hxx"
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include "tvfactory.hxx"
#include "tvread.hxx"


using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;



TVFactory::TVFactory( const uno::Reference< XMultiServiceFactory >& xMSF )
    : m_xMSF( xMSF )
{
}


TVFactory::~TVFactory()
{
}


//////////////////////////////////////////////////////////////////////////
// XInterface
//////////////////////////////////////////////////////////////////////////

void SAL_CALL
TVFactory::acquire(
    void )
    throw()
{
  OWeakObject::acquire();
}


void SAL_CALL
TVFactory::release(
              void )
  throw()
{
  OWeakObject::release();
}


Any SAL_CALL
TVFactory::queryInterface(
    const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( XServiceInfo*,  this ),
                                     SAL_STATIC_CAST( XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XMultiServiceFactory*, this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


////////////////////////////////////////////////////////////////////////////////
//
// XTypeProvider methods.

XTYPEPROVIDER_IMPL_3( TVFactory,
                         XServiceInfo,
                         XTypeProvider,
                         XMultiServiceFactory );



////////////////////////////////////////////////////////////////////////////////

// XServiceInfo methods.

rtl::OUString SAL_CALL
TVFactory::getImplementationName()
    throw( RuntimeException )
{
    return TVFactory::getImplementationName_static();
}


sal_Bool SAL_CALL
TVFactory::supportsService(
    const rtl::OUString& ServiceName )
    throw( RuntimeException )
{
    return
        ServiceName.compareToAscii( "com.sun.star.help.TreeView" ) == 0 ||
        ServiceName.compareToAscii( "com.sun.star.ucb.HiearchyDataSource" ) == 0;
}


Sequence< rtl::OUString > SAL_CALL
TVFactory::getSupportedServiceNames( void )
    throw( RuntimeException )
{
    return TVFactory::getSupportedServiceNames_static();
}



// XMultiServiceFactory

Reference< XInterface > SAL_CALL
TVFactory::createInstance(
    const rtl::OUString& aServiceSpecifier )
    throw( Exception,
           RuntimeException )
{
    Any aAny;
    aAny <<= rtl::OUString();
    Sequence< Any > seq( 1 );
    seq[0] <<= PropertyValue(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "nodepath" )),
        -1,
        aAny,
        PropertyState_DIRECT_VALUE );

    return createInstanceWithArguments( aServiceSpecifier,
                                        seq );
}


Reference< XInterface > SAL_CALL
TVFactory::createInstanceWithArguments(
    const rtl::OUString& ServiceSpecifier,
    const Sequence< Any >& Arguments )
    throw( Exception,
           RuntimeException )
{
    (void)ServiceSpecifier;

    if( ! m_xHDS.is() )
    {
        cppu::OWeakObject* p = new TVChildTarget( m_xMSF );
        m_xHDS = Reference< XInterface >( p );
    }

    Reference< XInterface > ret = m_xHDS;

    rtl::OUString hierview;
    for( int i = 0; i < Arguments.getLength(); ++i )
    {
        PropertyValue pV;
        if( ! ( Arguments[i] >>= pV ) )
            continue;

        if( pV.Name.compareToAscii( "nodepath" ) )
            continue;

        if( ! ( pV.Value >>= hierview ) )
            continue;

        break;
    }

    if( hierview.getLength() )
    {
        Reference< XHierarchicalNameAccess > xhieraccess( m_xHDS,UNO_QUERY );
        Any aAny = xhieraccess->getByHierarchicalName( hierview );
        Reference< XInterface > xInterface;
        aAny >>= xInterface;
        return xInterface;
    }
    else
        return m_xHDS;
}


Sequence< rtl::OUString > SAL_CALL
TVFactory::getAvailableServiceNames( )
    throw( RuntimeException )
{
    Sequence< rtl::OUString > seq( 1 );
    seq[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.HierarchyDataReadAccess" ));
    return seq;
}



// static


rtl::OUString SAL_CALL
TVFactory::getImplementationName_static()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.help.TreeViewImpl" ));
}


Sequence< rtl::OUString > SAL_CALL
TVFactory::getSupportedServiceNames_static()
{
    Sequence< rtl::OUString > seq( 2 );
    seq[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.help.TreeView" ));
    seq[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.HiearchyDataSource" ));
    return seq;
}


Reference< XSingleServiceFactory > SAL_CALL
TVFactory::createServiceFactory(
    const Reference< XMultiServiceFactory >& rxServiceMgr )
{
    return Reference< XSingleServiceFactory > (
        cppu::createSingleFactory(
            rxServiceMgr,
            TVFactory::getImplementationName_static(),
            TVFactory::CreateInstance,
            TVFactory::getSupportedServiceNames_static() ) );
}



Reference< XInterface > SAL_CALL
TVFactory::CreateInstance(
    const Reference< XMultiServiceFactory >& xMultiServiceFactory )
{
    XServiceInfo* xP = (XServiceInfo*) new TVFactory( xMultiServiceFactory );
    return Reference< XInterface >::query( xP );
}

//=========================================================================
extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    (void)ppEnv;

    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//=========================================================================
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName,void * pServiceManager,void * pRegistryKey )
{
    (void)pRegistryKey;

    void * pRet = 0;

    Reference< XMultiServiceFactory > xSMgr(
        reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );

    Reference< XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // File Content Provider.
    //////////////////////////////////////////////////////////////////////

    if ( TVFactory::getImplementationName_static().compareToAscii( pImplName ) == 0 )
    {
        xFactory = TVFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
