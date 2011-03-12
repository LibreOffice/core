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
#include <comphelper/officeresourcebundle.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/resource/XResourceBundle.hpp>
#include <com/sun/star/resource/XResourceBundleLoader.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
/** === end UNO includes === **/
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using com::sun::star::resource::XResourceBundle;
    using com::sun::star::resource::XResourceBundleLoader;
    using com::sun::star::resource::MissingResourceException;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    /** === end UNO using === **/

    //====================================================================
    //= ResourceBundle_Impl
    //====================================================================
    class ResourceBundle_Impl
    {
    private:
        Reference< XComponentContext >  m_xContext;
        ::rtl::OUString                 m_sBaseName;
        Reference< XResourceBundle >    m_xBundle;
        bool                            m_bAttemptedCreate;
        mutable ::osl::Mutex            m_aMutex;

    public:
        ResourceBundle_Impl( const Reference< XComponentContext >& _context, const ::rtl::OUString& _baseName )
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
        ::rtl::OUString loadString( sal_Int32 _resourceId ) const;

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

            The method is safe against multiple calls: If a previos call succeeded or failed, the
            previous result will be returned, without any other processing.

            @precond
                Our mutex is locked.
        */
        bool    impl_loadBundle_nothrow();

        /** returns the resource bundle key for a string with a given resource id
        */
        static ::rtl::OUString
                impl_getStringResourceKey( sal_Int32 _resourceId );
    };

    //--------------------------------------------------------------------
    ::rtl::OUString ResourceBundle_Impl::impl_getStringResourceKey( sal_Int32 _resourceId )
    {
        ::rtl::OUStringBuffer key;
        key.appendAscii( "string:" );
        key.append( _resourceId );
        return key.makeStringAndClear();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString ResourceBundle_Impl::loadString( sal_Int32 _resourceId ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ::rtl::OUString sString;

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

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    bool ResourceBundle_Impl::impl_loadBundle_nothrow()
    {
        if ( m_bAttemptedCreate )
            return m_xBundle.is();

        m_bAttemptedCreate = true;

        Reference< XResourceBundleLoader > xLoader;
        try
        {
            Any aValue( m_xContext->getValueByName(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/com.sun.star.resource.OfficeResourceLoader" ) ) ) );
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

    //====================================================================
    //= OfficeResourceBundle
    //====================================================================
    //--------------------------------------------------------------------
    OfficeResourceBundle::OfficeResourceBundle( const Reference< XComponentContext >& _context, const ::rtl::OUString& _bundleBaseName )
        :m_pImpl( new ResourceBundle_Impl( _context, _bundleBaseName ) )
    {
        if ( !_context.is() )
            throw NullPointerException();
    }

    //--------------------------------------------------------------------
    OfficeResourceBundle::OfficeResourceBundle( const Reference< XComponentContext >& _context, const sal_Char* _bundleBaseAsciiName )
        :m_pImpl( new ResourceBundle_Impl( _context, ::rtl::OUString::createFromAscii( _bundleBaseAsciiName ) ) )
    {
        if ( !_context.is() )
            throw NullPointerException();
    }

    //--------------------------------------------------------------------
    OfficeResourceBundle::~OfficeResourceBundle()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OfficeResourceBundle::loadString( sal_Int32 _resourceId ) const
    {
        return m_pImpl->loadString( _resourceId );
    }

    //--------------------------------------------------------------------
    bool OfficeResourceBundle::hasString( sal_Int32 _resourceId ) const
    {
        return m_pImpl->hasString( _resourceId );
    }

//........................................................................
} // namespace comphelper
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
