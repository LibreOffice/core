/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ucpext_provider.hxx"
#include "ucpext_content.hxx"

#include <ucbhelper/contentidentifier.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>


namespace ucb { namespace ucp { namespace ext
{


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

    
    
    
    
    ContentProvider::ContentProvider( const Reference< XComponentContext >& rxContext )
        :ContentProvider_Base( rxContext )
    {
    }

    
    ContentProvider::~ContentProvider()
    {
    }

    
    OUString SAL_CALL ContentProvider::getImplementationName_static() throw (RuntimeException)
    {
        return OUString(  "org.openoffice.comp.ucp.ext.ContentProvider"  );
    }

    
    OUString SAL_CALL ContentProvider::getImplementationName() throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    
    Sequence< OUString > SAL_CALL ContentProvider::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< OUString > aServiceNames(2);
        aServiceNames[0] = "com.sun.star.ucb.ContentProvider";
        aServiceNames[1] = "com.sun.star.ucb.ExtensionContentProvider";
        return aServiceNames;
    }

    
    Sequence< OUString > SAL_CALL ContentProvider::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    
    Reference< XInterface > ContentProvider::Create( const Reference< XComponentContext >& i_rContext )
    {
        return *( new ContentProvider( i_rContext ) );
    }

    
    OUString ContentProvider::getRootURL()
    {
        return OUString(  "vnd.sun.star.extension:
    }

    
    OUString ContentProvider::getArtificialNodeContentType()
    {
        return OUString(  "application/vnd.sun.star.extension-content"  );
    }

    
    namespace
    {
        void lcl_ensureAndTransfer( OUString& io_rIdentifierFragment, OUStringBuffer& o_rNormalization, const sal_Unicode i_nLeadingChar )
        {
            if ( ( io_rIdentifierFragment.isEmpty() ) || ( io_rIdentifierFragment[0] != i_nLeadingChar ) )
                throw IllegalIdentifierException();
            io_rIdentifierFragment = io_rIdentifierFragment.copy( 1 );
            o_rNormalization.append( i_nLeadingChar );
        }
    }

    
    Reference< XContent > SAL_CALL ContentProvider::queryContent( const Reference< XContentIdentifier  >& i_rIdentifier )
        throw( IllegalIdentifierException, RuntimeException )
    {
        
        const OUString sScheme( "vnd.sun.star.extension" );
        if ( !i_rIdentifier->getContentProviderScheme().equalsIgnoreAsciiCase( sScheme ) )
            throw IllegalIdentifierException();

        
        const OUString sIdentifier( i_rIdentifier->getContentIdentifier() );

        
        OUStringBuffer aComposer;
        aComposer.append( sIdentifier.copy( 0, sScheme.getLength() ).toAsciiLowerCase() );

        
        OUString sRemaining( sIdentifier.copy( sScheme.getLength() ) );
        lcl_ensureAndTransfer( sRemaining, aComposer, ':' );

        
        lcl_ensureAndTransfer( sRemaining, aComposer, '/' );

        
        if ( sRemaining.isEmpty() )
        {
            
            aComposer.appendAscii( "
        }
        else
        {
            if ( sRemaining[0] != '/' )
            {
                aComposer.append( '/' );
                aComposer.append( sRemaining );
            }
            else
            {
                lcl_ensureAndTransfer( sRemaining, aComposer, '/' );
                
                if ( sRemaining.isEmpty() )
                {
                    
                    aComposer.append( '/' );
                }
                else
                {
                    aComposer.append( sRemaining );
                }
            }
        }
        const Reference< XContentIdentifier > xNormalizedIdentifier( new ::ucbhelper::ContentIdentifier( aComposer.makeStringAndClear() ) );

        ::osl::MutexGuard aGuard( m_aMutex );

        
        Reference< XContent > xContent( queryExistingContent( xNormalizedIdentifier ).get() );
        if ( xContent.is() )
            return xContent;

        
        xContent = new Content( m_xContext, this, xNormalizedIdentifier );
        if ( !xContent->getIdentifier().is() )
            throw IllegalIdentifierException();

        registerNewContent( xContent );
        return xContent;
    }


} } }   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
