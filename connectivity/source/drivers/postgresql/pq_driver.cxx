/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  $RCSfile: pq_driver.cxx,v $
 *
 *  $Revision: 1.1.2.2 $
 *
 *  last change: $Author: jbu $ $Date: 2004/05/09 19:47:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
 *
 *
 ************************************************************************/

#include <stdio.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include "pq_driver.hxx"

using rtl::OUString;
using rtl::OUStringToOString;
using osl::MutexGuard;

using cppu::WeakComponentImplHelper2;

using com::sun::star::lang::XSingleComponentFactory;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::XComponent;

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::Any;

using com::sun::star::beans::PropertyValue;
using com::sun::star::beans::XPropertySet;

using com::sun::star::sdbc::XConnection;
using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::DriverPropertyInfo;

using com::sun::star::sdbcx::XTablesSupplier;


namespace pq_sdbc_driver
{
#define ASCII_STR(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )

OUString DriverGetImplementationName()
{
    static OUString *p;
    if (! p )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        static OUString instance(
            RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.connectivity.pq.Driver" ) );
        p = &instance;
    }
    return *p;
}

Sequence< OUString > DriverGetSupportedServiceNames()
{
    static Sequence< OUString > *p;
    if( ! p )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        OUString tmp( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdbc.Driver" ) );
        static Sequence< OUString > instance( &tmp,1 );
        p = &instance;
    }
    return *p;
}

Reference< XConnection > Driver::connect(
    const OUString& url,const Sequence< PropertyValue >& info )
    throw (SQLException, RuntimeException)
{
    if( ! acceptsURL( url ) )  // XDriver spec tells me to do so ...
        return Reference< XConnection > ();

    Sequence< Any > seq ( 2 );
    seq[0] <<= url;
    seq[1] <<= info;
    return Reference< XConnection> (
        m_smgr->createInstanceWithArgumentsAndContext(
            OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.connectivity.pq.Connection" ) ),
            seq, m_ctx ),
        UNO_QUERY );
}

sal_Bool Driver::acceptsURL( const ::rtl::OUString& url )
    throw (SQLException, RuntimeException)
{
    return url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "sdbc:postgresql:" ) );
}

Sequence< DriverPropertyInfo > Driver::getPropertyInfo(
    const OUString& url,const Sequence< PropertyValue >& info )
    throw (SQLException, RuntimeException)
{
    return Sequence< DriverPropertyInfo > ();
}

sal_Int32  Driver::getMajorVersion(  ) throw (RuntimeException)
{
    return PQ_SDBC_MAJOR;
}


sal_Int32 Driver::getMinorVersion(  ) throw (RuntimeException)
{
    return PQ_SDBC_MINOR;
}

    // XServiceInfo
OUString SAL_CALL Driver::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return DriverGetImplementationName();
}

sal_Bool Driver::supportsService(const OUString& ServiceName)
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > serviceNames = DriverGetSupportedServiceNames();
    for( int i = 0 ; i < serviceNames.getLength() ; i ++ )
        if( serviceNames[i] == ServiceName )
            return sal_True;
    return sal_False;
}

Sequence< OUString > Driver::getSupportedServiceNames(void)
    throw(::com::sun::star::uno::RuntimeException)
{
    return DriverGetSupportedServiceNames();
}

// XComponent
void Driver::disposing()
{

}


Reference< XTablesSupplier > Driver::getDataDefinitionByConnection(
    const Reference< XConnection >& connection )
        throw (SQLException, RuntimeException)
{
     return Reference< XTablesSupplier >( connection , UNO_QUERY );
}

Reference< XTablesSupplier > Driver::getDataDefinitionByURL(
    const ::rtl::OUString& url, const Sequence< PropertyValue >& info )
        throw (SQLException, RuntimeException)
{
    return Reference< XTablesSupplier > ( connect( url, info ), UNO_QUERY );
}


Reference< XInterface > DriverCreateInstance( const Reference < XComponentContext > & ctx )
{
    Reference< XInterface >  ret = * new Driver( ctx );
    return ret;
}




class OOneInstanceComponentFactory :
    public MutexHolder,
    public WeakComponentImplHelper2< XSingleComponentFactory, XServiceInfo >
{
public:
    OOneInstanceComponentFactory(
        const OUString & rImplementationName_,
        cppu::ComponentFactoryFunc fptr,
        const Sequence< OUString > & serviceNames,
        const Reference< XComponentContext > & defaultContext) :
        WeakComponentImplHelper2< XSingleComponentFactory, XServiceInfo >( this->m_mutex ),
        m_implName( rImplementationName_ ),
        m_create( fptr ),
        m_serviceNames( serviceNames ),
        m_defaultContext( defaultContext )
    {
    }

    // XSingleComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException)
    {
        return m_implName;
    }
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException)
    {
        for( int i = 0 ; i < m_serviceNames.getLength() ; i ++ )
            if( m_serviceNames[i] == ServiceName )
                return sal_True;
        return sal_False;
    }
    Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException)
    {
        return m_serviceNames;
    }

    // XComponent
    virtual void SAL_CALL disposing();

private:
    cppu::ComponentFactoryFunc     m_create;
    Sequence< OUString >           m_serviceNames;
    OUString                       m_implName;
    Reference< XInterface >        m_theInstance;
    Reference< XComponentContext > m_defaultContext;
};

Reference< XInterface > OOneInstanceComponentFactory::createInstanceWithArgumentsAndContext(
    Sequence< Any > const &rArguments, const Reference< XComponentContext > & ctx )
    throw( RuntimeException, Exception )
{
    return createInstanceWithContext( ctx );
}

Reference< XInterface > OOneInstanceComponentFactory::createInstanceWithContext(
    const Reference< XComponentContext > & ctx )
    throw( RuntimeException, Exception )
{
    if( ! m_theInstance.is() )
    {
        // work around the problem in sdbc
        Reference< XComponentContext > useCtx = ctx;
        if( ! useCtx.is() )
            useCtx = m_defaultContext;
        Reference< XInterface > theInstance = m_create( useCtx );
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( ! m_theInstance.is () )
        {
            m_theInstance = theInstance;
        }
    }
    return m_theInstance;
}

void OOneInstanceComponentFactory::disposing()
{
    Reference< XComponent > rComp;
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        rComp = Reference< XComponent >( m_theInstance, UNO_QUERY );
        m_theInstance.clear();
    }
    if( rComp.is() )
        rComp->dispose();
}

//  Reference< XSingleComponentFactory > createOneInstanceComponentFactory(
//      cppu::ComponentFactoryFunc fptr,
//      ::rtl::OUString const & rImplementationName,
//      ::com::sun::star::uno::Sequence< ::rtl::OUString > const & rServiceNames,
//      rtl_ModuleCount * pModCount = 0 )
//      SAL_THROW( () )
//  {
//      return new OOneInstanceComponentFactory( rImplementationName, fptr , rServiceNames);
//  }

}

static struct cppu::ImplementationEntry g_entries[] =
{
    {
        pq_sdbc_driver::DriverCreateInstance, pq_sdbc_driver::DriverGetImplementationName,
        pq_sdbc_driver::DriverGetSupportedServiceNames, 0,
        0 , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return cppu::component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    // need to extract the defaultcontext, because the way, sdbc
    // bypasses the servicemanager, does not allow to use the
    // XSingleComponentFactory interface ...
    void * pRet = 0;
    Reference< XSingleComponentFactory > xFactory;
    Reference< XInterface > xSmgr( (XInterface * ) pServiceManager );

    for( sal_Int32 i = 0 ; g_entries[i].create ; i ++ )
    {
        OUString implName = g_entries[i].getImplementationName();
        if( 0 == implName.compareToAscii( pImplName ) )
        {
            Reference< XComponentContext > defaultContext;
            Reference< XPropertySet > propSet( xSmgr, UNO_QUERY );
            if( propSet.is() )
            {
                try
                {
                    propSet->getPropertyValue( ASCII_STR( "DefaultContext" ) ) >>= defaultContext;
                }
                catch( com::sun::star::uno::Exception &e )
                {
                    // if there is no default context, ignore it
                }
            }
            xFactory = new pq_sdbc_driver::OOneInstanceComponentFactory(
                implName,
                g_entries[i].create,
                g_entries[i].getSupportedServiceNames(),
                defaultContext );
        }
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}
