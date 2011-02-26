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
#include <comphelper/componentmodule.hxx>

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <comphelper/sequence.hxx>
#include <osl/diagnose.h>

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
        ComponentDescription aComponent( _rImplementationName, _rServiceNames, ::rtl::OUString(), _pCreateFunction, _pFactoryFunction );
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
        OSL_ENSURE( _rxRootKey.is(), "OModule::writeComponentInfos: invalid argument!" );

        ::rtl::OUString sRootKey( "/", 1, RTL_TEXTENCODING_ASCII_US );

        for (   ComponentDescriptions::const_iterator component = m_pImpl->m_aRegisteredComponents.begin();
                component != m_pImpl->m_aRegisteredComponents.end();
                ++component
            )
        {
            ::rtl::OUString sMainKeyName( sRootKey );
            sMainKeyName += component->sImplementationName;
            sMainKeyName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));

            try
            {
                Reference< XRegistryKey >  xNewKey( _rxRootKey->createKey( sMainKeyName ) );

                const ::rtl::OUString* pService = component->aSupportedServices.getConstArray();
                const ::rtl::OUString* pServiceEnd = component->aSupportedServices.getConstArray() + component->aSupportedServices.getLength();
                for ( ; pService != pServiceEnd; ++pService )
                    xNewKey->createKey( *pService );

                if ( component->sSingletonName.getLength() )
                {
                    OSL_ENSURE( component->aSupportedServices.getLength() == 1, "OModule::writeComponentInfos: singletons should support exactly one service, shouldn't they?" );

                    ::rtl::OUString sSingletonKeyName( sRootKey );
                    sSingletonKeyName += component->sImplementationName;
                    sSingletonKeyName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SINGLETONS/"));
                    sSingletonKeyName += component->sSingletonName;

                    xNewKey = _rxRootKey->createKey( sSingletonKeyName );
                    xNewKey->setStringValue( component->aSupportedServices[ 0 ] );
                }
            }
            catch( Exception& )
            {
                OSL_FAIL( "OModule::writeComponentInfos: something went wrong while creating the keys!" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
