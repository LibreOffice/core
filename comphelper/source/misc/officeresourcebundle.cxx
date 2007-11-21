/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: officeresourcebundle.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:53:32 $
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

#ifndef COMPHELPER_OFFICE_RESOURCE_BUNDLE_HXX
#include <comphelper/officeresourcebundle.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_RESOURCE_XRESOURCEBUNDLE_HPP_
#include <com/sun/star/resource/XResourceBundle.hpp>
#endif
#ifndef _COM_SUN_STAR_RESOURCE_XRESOURCEBUNDLELOADER_HPP_
#include <com/sun/star/resource/XResourceBundleLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
/** === end UNO includes === **/

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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
                OSL_ENSURE( false, "ResourceBundle_Impl::loadString: caught an exception!" );
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
                OSL_ENSURE( false, "ResourceBundle_Impl::hasString: caught an exception!" );
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
            OSL_ENSURE( false, "ResourceBundle_Impl::impl_loadBundle_nopthrow: could not create the resource loader!" );
        }

        if ( !xLoader.is() )
            return false;

        try
        {
            m_xBundle = xLoader->loadBundle_Default( m_sBaseName );
        }
        catch( const MissingResourceException& )
        {
            OSL_ENSURE( false, "ResourceBundle_Impl::impl_loadBundle_nopthrow: missing the given resource bundle!" );
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
