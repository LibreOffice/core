/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
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
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#include <stdio.h>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/implementationentry.hxx>

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
            RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.connectivity.pq.Driver.noext" ) );
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
            OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.connectivity.pq.Connection.noext" ) ),
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
    (void)url; (void)info;
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
        m_create( fptr ),
        m_serviceNames( serviceNames ),
        m_implName( rImplementationName_ ),
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
    (void)rArguments;
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
//      SAL_THROW(())
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

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    // need to extract the defaultcontext, because the way, sdbc
    // bypasses the servicemanager, does not allow to use the
    // XSingleComponentFactory interface ...
    void * pRet = 0;
    Reference< XSingleComponentFactory > xFactory;
    Reference< com::sun::star::lang::XMultiServiceFactory > xSmgr(
        static_cast< XInterface * >(pServiceManager),
        com::sun::star::uno::UNO_QUERY_THROW );

    for( sal_Int32 i = 0 ; g_entries[i].create ; i ++ )
    {
        OUString implName = g_entries[i].getImplementationName();
        if( 0 == implName.compareToAscii( pImplName ) )
        {
            Reference< XComponentContext > defaultContext(
                comphelper::getComponentContext( xSmgr ) );
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
