/*************************************************************************
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

#include "precompiled_sd.hxx"

#include "ctp_factory.hxx"
#include "ctp_panel.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/drawing/framework/XResourceFactoryManager.hpp>
/** === end UNO includes === **/

//......................................................................................................................
namespace sd { namespace colortoolpanel
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::drawing::framework::XResourceId;
    using ::com::sun::star::drawing::framework::XResource;
    using ::com::sun::star::lang::NotInitializedException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::drawing::framework::XResourceFactoryManager;
    /** === end UNO using === **/

    //==================================================================================================================
    //= helper
    //==================================================================================================================
    namespace
    {
        const ::rtl::OUString& lcl_getSingleColorViewURL()
        {
            static ::rtl::OUString s_sSingleColorViewURL( RTL_CONSTASCII_USTRINGPARAM( "private:resource/view/SingleColorView" ) );
            return s_sSingleColorViewURL;
        }
    }

    //==================================================================================================================
    //= ResourceFactory
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ResourceFactory::ResourceFactory( const Reference< XComponentContext >& i_rContext )
        :m_xContext( i_rContext )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ResourceFactory::~ResourceFactory()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XResource > SAL_CALL ResourceFactory::createResource( const Reference< XResourceId >& i_rResourceId ) throw (RuntimeException)
    {
        FactoryGuard aGuard( *this );
        if ( !i_rResourceId.is() )
            // TODO: the API should allow me to throw an IllegalArgumentException here
            throw NULL;

        const ::rtl::OUString sResourceURL( i_rResourceId->getResourceURL() );
        if ( sResourceURL != lcl_getSingleColorViewURL() )
            return NULL;

        return new SingleColorPanel( m_xContext, m_xControllerManager->getConfigurationController(), i_rResourceId );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ResourceFactory::releaseResource( const Reference< XResource >& i_rResource ) throw (RuntimeException)
    {
        FactoryGuard aGuard( *this );
        // TODO: place your code here
        (void)i_rResource;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ResourceFactory::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ResourceFactory::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.example.colorpanel.ResourceFactory" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL ResourceFactory::supportsService( const ::rtl::OUString& i_rServiceName ) throw (RuntimeException)
    {
        const Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() );
        for (   const ::rtl::OUString* serviceName = aServiceNames.getConstArray();
                serviceName != aServiceNames.getConstArray() + aServiceNames.getLength();
                ++serviceName
            )
        {
            if ( i_rServiceName == *serviceName )
                return  sal_True;
        }
        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ResourceFactory::getSupportedServiceNames() throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ResourceFactory::getSupportedServiceNames_static() throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServiceNames(1);
        aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.example.colorpanel.ResourceFactory" ) );
        return aServiceNames;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ResourceFactory::Create( const Reference< XComponentContext >& i_rContext ) throw (RuntimeException)
    {
        return *( new ResourceFactory( i_rContext ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ResourceFactory::checkInitialized( GuardAccess ) const
    {
        if ( !m_xControllerManager.is() )
            throw NotInitializedException( ::rtl::OUString(), *const_cast< ResourceFactory* >( this ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ResourceFactory::checkDisposed( GuardAccess ) const
    {
        // cannot be disposed currently ...
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ResourceFactory::initialize( const Sequence< Any >& i_rArguments ) throw (Exception, RuntimeException)
    {
        if ( !i_rArguments.getLength() )
            throw IllegalArgumentException();

        FactoryGuard aGuard( *this, false );
        m_xControllerManager.set( i_rArguments[0], UNO_QUERY_THROW );

        Reference< XResourceFactoryManager > xFactoryManager( m_xControllerManager->getConfigurationController(), UNO_QUERY_THROW );
        xFactoryManager->addResourceFactory( lcl_getSingleColorViewURL(), this );
    }

//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................
