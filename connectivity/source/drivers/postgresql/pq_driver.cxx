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
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "pq_driver.hxx"

using osl::MutexGuard;

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
using com::sun::star::sdbc::DriverPropertyInfo;

using com::sun::star::sdbcx::XTablesSupplier;


namespace pq_sdbc_driver
{

OUString DriverGetImplementationName()
{
    return OUString( "org.openoffice.comp.connectivity.pq.Driver.noext" );
}

Sequence< OUString > DriverGetSupportedServiceNames()
{
    return Sequence< OUString > { "com.sun.star.sdbc.Driver" };
}

Reference< XConnection > Driver::connect(
    const OUString& url,const Sequence< PropertyValue >& info )
{
    if( ! acceptsURL( url ) )  // XDriver spec tells me to do so ...
        return Reference< XConnection > ();

    Sequence< Any > seq ( 2 );
    seq[0] <<= url;
    seq[1] <<= info;
    return Reference< XConnection> (
        m_smgr->createInstanceWithArgumentsAndContext(
            "org.openoffice.comp.connectivity.pq.Connection.noext",
            seq, m_ctx ),
        UNO_QUERY );
}

sal_Bool Driver::acceptsURL( const OUString& url )
{
    return url.startsWith( "sdbc:postgresql:" );
}

Sequence< DriverPropertyInfo > Driver::getPropertyInfo(
    const OUString& url,const Sequence< PropertyValue >& info )
{
    (void)url; (void)info;
    return Sequence< DriverPropertyInfo > ();
}

sal_Int32  Driver::getMajorVersion(  )
{
    return PQ_SDBC_MAJOR;
}


sal_Int32 Driver::getMinorVersion(  )
{
    return PQ_SDBC_MINOR;
}

    // XServiceInfo
OUString SAL_CALL Driver::getImplementationName()
{
    return DriverGetImplementationName();
}

sal_Bool Driver::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > Driver::getSupportedServiceNames()
{
    return DriverGetSupportedServiceNames();
}

// XComponent
void Driver::disposing()
{

}


Reference< XTablesSupplier > Driver::getDataDefinitionByConnection(
    const Reference< XConnection >& connection )
{
     return Reference< XTablesSupplier >( connection , UNO_QUERY );
}

Reference< XTablesSupplier > Driver::getDataDefinitionByURL(
    const OUString& url, const Sequence< PropertyValue >& info )
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
    public cppu::WeakComponentImplHelper< XSingleComponentFactory, XServiceInfo >
{
public:
    OOneInstanceComponentFactory(
        const OUString & rImplementationName_,
        cppu::ComponentFactoryFunc fptr,
        const Sequence< OUString > & serviceNames,
        const Reference< XComponentContext > & defaultContext) :
        cppu::WeakComponentImplHelper< XSingleComponentFactory, XServiceInfo >( this->m_mutex ),
        m_create( fptr ),
        m_serviceNames( serviceNames ),
        m_implName( rImplementationName_ ),
        m_defaultContext( defaultContext )
    {
    }

    // XSingleComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        Reference< XComponentContext > const & xContext ) override;
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext ) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    {
        return m_implName;
    }
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override
    {
        for( int i = 0 ; i < m_serviceNames.getLength() ; i ++ )
            if( m_serviceNames[i] == ServiceName )
                return true;
        return false;
    }
    Sequence< OUString > SAL_CALL getSupportedServiceNames() override
    {
        return m_serviceNames;
    }

    // XComponent
    virtual void SAL_CALL disposing() override;

private:
    cppu::ComponentFactoryFunc     m_create;
    Sequence< OUString >           m_serviceNames;
    OUString                       m_implName;
    Reference< XInterface >        m_theInstance;
    Reference< XComponentContext > m_defaultContext;
};

Reference< XInterface > OOneInstanceComponentFactory::createInstanceWithArgumentsAndContext(
    Sequence< Any > const &rArguments, const Reference< XComponentContext > & ctx )
{
    (void)rArguments;
    return createInstanceWithContext( ctx );
}

Reference< XInterface > OOneInstanceComponentFactory::createInstanceWithContext(
    const Reference< XComponentContext > & ctx )
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
        rComp.set( m_theInstance, UNO_QUERY );
        m_theInstance.clear();
    }
    if( rComp.is() )
        rComp->dispose();
}

//  Reference< XSingleComponentFactory > createOneInstanceComponentFactory(
//      cppu::ComponentFactoryFunc fptr,
//      OUString const & rImplementationName,
//      css::uno::Sequence< OUString > const & rServiceNames,
//      rtl_ModuleCount * pModCount = 0 )
//
//  {
//      return new OOneInstanceComponentFactory( rImplementationName, fptr , rServiceNames);
//  }

}

static const struct cppu::ImplementationEntry g_entries[] =
{
    {
        pq_sdbc_driver::DriverCreateInstance, pq_sdbc_driver::DriverGetImplementationName,
        pq_sdbc_driver::DriverGetSupportedServiceNames, nullptr,
        nullptr , 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL postgresql_sdbc_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    // need to extract the defaultcontext, because the way, sdbc
    // bypasses the servicemanager, does not allow to use the
    // XSingleComponentFactory interface ...
    void * pRet = nullptr;
    Reference< XSingleComponentFactory > xFactory;
    Reference< css::lang::XMultiServiceFactory > xSmgr(
        static_cast< XInterface * >(pServiceManager),
        css::uno::UNO_QUERY_THROW );

    for( sal_Int32 i = 0 ; g_entries[i].create ; i ++ )
    {
        OUString implName = g_entries[i].getImplementationName();
        if( implName.equalsAscii( pImplName ) )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
