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


#include <ucbhelper/contentidentifier.hxx>
#include <libgnomevfs/gnome-vfs-init.h>
#include "gvfs_provider.hxx"
#include "gvfs_content.hxx"

using namespace com::sun::star;
using namespace gvfs;

//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

ContentProvider::ContentProvider(
    const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
: ::ucbhelper::ContentProviderImplHelper( rSMgr )
{
}
// sdafas
//=========================================================================
// virtual
ContentProvider::~ContentProvider()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   com::sun::star::ucb::XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      com::sun::star::ucb::XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( ContentProvider,
                     rtl::OUString( "com.sun.star.comp.GnomeVFSContentProvider" ),
                     rtl::OUString( "com.sun.star.ucb.GnomeVFSContentProvider" ) );
//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
    throw( com::sun::star::ucb::IllegalIdentifierException,
           uno::RuntimeException )
{
#if OSL_DEBUG_LEVEL > 1
    g_warning ("QueryContent: '%s'",
               rtl::OUStringToOString (Identifier->getContentIdentifier(),
                                       RTL_TEXTENCODING_UTF8).getStr() );
#endif

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< com::sun::star::ucb::XContent > xContent
        = queryExistingContent( Identifier ).get();
    if ( xContent.is() )
        return xContent;

    try
    {
        xContent = new ::gvfs::Content(m_xSMgr, this, Identifier );
        registerNewContent( xContent );
    }
    catch ( com::sun::star::ucb::ContentCreationException const & )
    {
        throw com::sun::star::ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw com::sun::star::ucb::IllegalIdentifierException();

    return xContent;
}


//============================ shlib entry points =============================================

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpgvfs1_component_getFactory( const sal_Char *pImplName,
              void           *pServiceManager,
              void           */*pRegistryKey*/ )
{
    void * pRet = 0;

    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if (!gnome_vfs_initialized ())
            gnome_vfs_init ();
        if (!auth_queue)
            auth_queue = g_private_new( auth_queue_destroy );
    }

    uno::Reference< lang::XMultiServiceFactory > xSMgr
        (reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( !::gvfs::ContentProvider::getImplementationName_Static().compareToAscii( pImplName ) )
        xFactory = ::gvfs::ContentProvider::createServiceFactory( xSMgr );

    if ( xFactory.is() ) {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
