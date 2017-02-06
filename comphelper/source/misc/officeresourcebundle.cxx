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

#include <comphelper/officeresourcebundle.hxx>

#include <com/sun/star/resource/MissingResourceException.hpp>
#include <com/sun/star/resource/XResourceBundle.hpp>
#include <com/sun/star/resource/XResourceBundleLoader.hpp>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>


namespace comphelper
{


    using ::com::sun::star::uno::Reference;
    using com::sun::star::resource::XResourceBundle;
    using com::sun::star::resource::XResourceBundleLoader;
    using com::sun::star::resource::MissingResourceException;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;

    class ResourceBundle_Impl
    {
    private:
        Reference< XComponentContext >  m_xContext;
        OUString                 m_sBaseName;
        Reference< XResourceBundle >    m_xBundle;
        bool                            m_bAttemptedCreate;
        mutable ::osl::Mutex            m_aMutex;

    public:
        ResourceBundle_Impl( const Reference< XComponentContext >& _context, const OUString& _baseName )
            :m_xContext( _context )
            ,m_sBaseName( _baseName )
            ,m_bAttemptedCreate( false )
        {
        }

    public:
        /** loads the string with the given resource id from the resource bundle
            @param  _resourceId
                the id of the string to load
            @return
                the requested resource string. If no string with the given id exists in the resource bundle,
                an empty string is returned. In a non-product version, an OSL_ENSURE will notify you of this
                then.
        */
        OUString loadString( sal_Int32 _resourceId ) const;

        /** determines whether the resource bundle has a string with the given id
            @param  _resourceId
                the id of the string whose existence is to be checked
            @return
                <TRUE/> if and only if a string with the given ID exists in the resource
                bundle.
        */
        bool            hasString( sal_Int32 _resourceId ) const;

    private:
        /** loads the bundle represented by the instance

            The method is safe against multiple calls: If a previous call succeeded or failed, the
            previous result will be returned, without any other processing.

            @precond
                Our mutex is locked.
        */
        bool    impl_loadBundle_nothrow();

        /** returns the resource bundle key for a string with a given resource id
        */
        static OUString
                impl_getStringResourceKey( sal_Int32 _resourceId );
    };


    OUString ResourceBundle_Impl::impl_getStringResourceKey( sal_Int32 _resourceId )
    {
        OUStringBuffer key;
        key.append( "string:" );
        key.append( _resourceId );
        return key.makeStringAndClear();
    }


    OUString ResourceBundle_Impl::loadString( sal_Int32 _resourceId ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        OUString sString;

        if ( const_cast< ResourceBundle_Impl* >( this )->impl_loadBundle_nothrow() )
        {
            try
            {
                OSL_VERIFY( m_xBundle->getByName( impl_getStringResourceKey( _resourceId ) ) >>= sString );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "ResourceBundle_Impl::loadString: caught an exception!" );
            }
        }
        return sString;
    }


    bool ResourceBundle_Impl::hasString( sal_Int32 _resourceId ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        bool has = false;

        if ( const_cast< ResourceBundle_Impl* >( this )->impl_loadBundle_nothrow() )
        {
            try
            {
                has = m_xBundle->hasByName( impl_getStringResourceKey( _resourceId ) );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "ResourceBundle_Impl::hasString: caught an exception!" );
            }
        }
        return has;
    }


    bool ResourceBundle_Impl::impl_loadBundle_nothrow()
    {
        if ( m_bAttemptedCreate )
            return m_xBundle.is();

        m_bAttemptedCreate = true;

        Reference< XResourceBundleLoader > xLoader;
        try
        {
            Any aValue( m_xContext->getValueByName( "/singletons/com.sun.star.resource.OfficeResourceLoader" ) );
            OSL_VERIFY( aValue >>= xLoader );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ResourceBundle_Impl::impl_loadBundle_nopthrow: could not create the resource loader!" );
        }

        if ( !xLoader.is() )
            return false;

        try
        {
            m_xBundle = xLoader->loadBundle_Default( m_sBaseName );
        }
        catch( const MissingResourceException& )
        {
            OSL_FAIL( "ResourceBundle_Impl::impl_loadBundle_nopthrow: missing the given resource bundle!" );
        }

        return m_xBundle.is();
    }


    OfficeResourceBundle::OfficeResourceBundle( const Reference< XComponentContext >& _context, const sal_Char* _bundleBaseAsciiName )
        :m_pImpl( new ResourceBundle_Impl( _context, OUString::createFromAscii( _bundleBaseAsciiName ) ) )
    {
    }


    OfficeResourceBundle::~OfficeResourceBundle()
    {
    }


    OUString OfficeResourceBundle::loadString( sal_Int32 _resourceId ) const
    {
        return m_pImpl->loadString( _resourceId );
    }


    bool OfficeResourceBundle::hasString( sal_Int32 _resourceId ) const
    {
        return m_pImpl->hasString( _resourceId );
    }


} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
