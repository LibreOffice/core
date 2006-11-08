/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: componentmodule.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-08 12:01:30 $
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
#ifndef COMPHELPER_INC_COMPHELPER_COMPONENTMODULE_HXX
#include <comphelper/componentmodule.hxx>
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <vector>

//........................................................................
namespace comphelper
{
//........................................................................

    using namespace ::cppu;
    /** === being UNO using === **/
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::registry::XRegistryKey;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::XInterface;
    /** === end UNO using === **/

    typedef ::std::vector< ComponentDescription >   ComponentDescriptions;

    //=========================================================================
    //= OModuleImpl
    //=========================================================================
    /** implementation for <type>OModule</type>. not threadsafe, has to be guarded by it's owner
    */
    class OModuleImpl
    {
    public:
        ComponentDescriptions                           m_aRegisteredComponents;

        OModuleImpl();
        ~OModuleImpl();
    };

    //-------------------------------------------------------------------------
    OModuleImpl::OModuleImpl()
    {
    }

    //-------------------------------------------------------------------------
    OModuleImpl::~OModuleImpl()
    {
    }

    //=========================================================================
    //= OModule
    //=========================================================================
    //-------------------------------------------------------------------------
    OModule::OModule()
        :m_nClients( 0 )
        ,m_pImpl( new OModuleImpl )
    {
    }

    OModule::~OModule() {}

    //-------------------------------------------------------------------------
    void OModule::registerClient( OModule::ClientAccess )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( 1 == osl_incrementInterlockedCount( &m_nClients ) )
            onFirstClient();
    }

    //-------------------------------------------------------------------------
    void OModule::revokeClient( OModule::ClientAccess )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( 0 == osl_decrementInterlockedCount( &m_nClients ) )
            onLastClient();
    }

    //--------------------------------------------------------------------------
    void OModule::onFirstClient()
    {
    }

    //--------------------------------------------------------------------------
    void OModule::onLastClient()
    {
    }

    //--------------------------------------------------------------------------
    void OModule::registerImplementation( const ComponentDescription& _rComp )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pImpl )
            throw RuntimeException();

        m_pImpl->m_aRegisteredComponents.push_back( _rComp );
    }

    //--------------------------------------------------------------------------
    void OModule::registerImplementation( const ::rtl::OUString& _rImplementationName, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
        ::cppu::ComponentFactoryFunc _pCreateFunction, FactoryInstantiation _pFactoryFunction )
    {
        ComponentDescription aComponent( _rImplementationName, _rServiceNames, _pCreateFunction, _pFactoryFunction );
        registerImplementation( aComponent );
    }

    //--------------------------------------------------------------------------
    sal_Bool OModule::writeComponentInfos( void* pServiceManager, void* pRegistryKey )
    {
        Reference< XMultiServiceFactory > xFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) );
        Reference< XRegistryKey > xRegistryKey( static_cast< XRegistryKey* >( pRegistryKey ) );
        return writeComponentInfos( xFactory, xRegistryKey );
    }

    //--------------------------------------------------------------------------
    sal_Bool OModule::writeComponentInfos(
            const Reference< XMultiServiceFactory >& /*_rxServiceManager*/,
            const Reference< XRegistryKey >& _rxRootKey )
    {
        OSL_ENSURE(_rxRootKey.is(), "OModule::writeComponentInfos : invalid argument !");

        ::rtl::OUString sRootKey( "/", 1, RTL_TEXTENCODING_ASCII_US );

        for (   ComponentDescriptions::const_iterator component = m_pImpl->m_aRegisteredComponents.begin();
                component != m_pImpl->m_aRegisteredComponents.end();
                ++component
            )
        {
            ::rtl::OUString sMainKeyName( sRootKey );
            sMainKeyName += component->sImplementationName;
            sMainKeyName += ::rtl::OUString::createFromAscii( "/UNO/SERVICES" );

            try
            {
                Reference< XRegistryKey >  xNewKey( _rxRootKey->createKey( sMainKeyName ) );

                const ::rtl::OUString* pService = component->aSupportedServices.getConstArray();
                const ::rtl::OUString* pServiceEnd = component->aSupportedServices.getConstArray() + component->aSupportedServices.getLength();
                for ( ; pService != pServiceEnd; ++pService )
                    xNewKey->createKey( *pService );
            }
            catch( Exception& )
            {
                OSL_ASSERT( "OModule::writeComponentInfos: something went wrong while creating the keys!" );
                return sal_False;
            }
        }

        return sal_True;
    }

    //--------------------------------------------------------------------------
    void* OModule::getComponentFactory( const sal_Char* _pImplementationName, void* _pServiceManager, void* /*_pRegistryKey*/ )
    {
        Reference< XInterface > xFactory( getComponentFactory(
            ::rtl::OUString::createFromAscii( _pImplementationName ),
            Reference< XMultiServiceFactory >( static_cast< XMultiServiceFactory* >( _pServiceManager ) )
        ) );
        return xFactory.get();
    }

    //--------------------------------------------------------------------------
    Reference< XInterface > OModule::getComponentFactory( const ::rtl::OUString& _rImplementationName,
        const Reference< XMultiServiceFactory >& /* _rxServiceManager */ )
    {
        Reference< XInterface > xReturn;

        for (   ComponentDescriptions::const_iterator component = m_pImpl->m_aRegisteredComponents.begin();
                component != m_pImpl->m_aRegisteredComponents.end();
                ++component
            )
        {
            if ( component->sImplementationName == _rImplementationName )
            {
                xReturn = component->pFactoryCreationFunc(
                    component->pComponentCreationFunc,
                    component->sImplementationName,
                    component->aSupportedServices,
                    NULL
                );
                if ( xReturn.is() )
                {
                    xReturn->acquire();
                    return xReturn.get();
                }
            }
        }

        return NULL;
    }

//........................................................................
} // namespace comphelper
//........................................................................
