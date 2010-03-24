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

#include "precompiled_ucb.hxx"

#include "ucpext_provider.hxx"
#include "ucpext_content.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <ucbhelper/contentidentifier.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <comphelper/componentcontext.hxx>
#include <rtl/ustrbuf.hxx>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
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
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::ucb::XContentIdentifier;
    using ::com::sun::star::ucb::IllegalIdentifierException;
    using ::com::sun::star::ucb::XContent;
    using ::com::sun::star::uno::XComponentContext;
    /** === end UNO using === **/

    //==================================================================================================================
    //= ContentProvider
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ContentProvider::ContentProvider( const Reference< XMultiServiceFactory >& i_rServiceManager )
        :ContentProvider_Base( i_rServiceManager )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ContentProvider::~ContentProvider()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ContentProvider::getImplementationName_static() throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.ucp.ext.ContentProvider" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ContentProvider::getImplementationName() throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ContentProvider::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServiceNames(2);
        aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.ContentProvider" ) );
        aServiceNames[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.ExtensionContentProvider" ) );
        return aServiceNames;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ContentProvider::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XInterface > ContentProvider::Create( const Reference< XComponentContext >& i_rContext )
    {
        const ::comphelper::ComponentContext aContext( i_rContext );
        return *( new ContentProvider( aContext.getLegacyServiceFactory() ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XContent > SAL_CALL ContentProvider::queryContent( const Reference< XContentIdentifier  >& i_rIdentifier )
        throw( IllegalIdentifierException, RuntimeException )
    {
        // Check URL scheme...
        const ::rtl::OUString aScheme( rtl::OUString::createFromAscii( "vnd.oracle.ooo.extension" ) );
        if ( !i_rIdentifier->getContentProviderScheme().equalsIgnoreAsciiCase( aScheme ) )
            throw IllegalIdentifierException();

        // normalize the scheme
        const ::rtl::OUString sIdentifier( i_rIdentifier->getContentIdentifier() );
        ::rtl::OUStringBuffer aComposer;
        aComposer.append( sIdentifier.copy( 0, aScheme.getLength() ).toAsciiLowerCase() );
        aComposer.append( sIdentifier.copy( aScheme.getLength() ) );
        const Reference< XContentIdentifier > xNormalizedIdentifier( new ::ucbhelper::ContentIdentifier( m_xSMgr, aComposer.makeStringAndClear() ) );

        ::osl::MutexGuard aGuard( m_aMutex );

        // check if a content with given id already exists...
        Reference< XContent > xContent( queryExistingContent( xNormalizedIdentifier ).get() );
        if ( xContent.is() )
            return xContent;

        // create a new content
        xContent = new Content( m_xSMgr, this, xNormalizedIdentifier );
        if ( !xContent->getIdentifier().is() )
            throw IllegalIdentifierException();

        registerNewContent( xContent );
        return xContent;
    }

//......................................................................................................................
} } }   // namespace ucb::ucp::ext
//......................................................................................................................
