/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: provider.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 14:49:04 $
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
#include "precompiled_ucb.hxx"

#include <ucbhelper/contentidentifier.hxx>
#include <libgnomevfs/gnome-vfs-init.h>
#include "provider.hxx"
#include "content.hxx"

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
                     rtl::OUString::createFromAscii(
            "com.sun.star.comp.GnomeVFSContentProvider" ),
                     rtl::OUString::createFromAscii(
            "com.sun.star.ucb.GnomeVFSContentProvider" ) );
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
#ifdef DEBUG
    g_warning ("QueryContent: '%s'",
               (const sal_Char *)rtl::OUStringToOString
               (Identifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8));
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


// cut and paste verbatim from webdav (that sucks).
static sal_Bool
writeInfo( void                                 *pRegistryKey,
       const rtl::OUString                  &rImplementationName,
       uno::Sequence< rtl::OUString > const &rServiceNames )
{
    rtl::OUString aKeyName( rtl::OUString::createFromAscii( "/" ) );
    aKeyName += rImplementationName;
    aKeyName += rtl::OUString::createFromAscii( "/UNO/SERVICES" );

    uno::Reference< registry::XRegistryKey > xKey;
    try {
        xKey = static_cast< registry::XRegistryKey * >
            (pRegistryKey )->createKey( aKeyName );
    }
    catch ( registry::InvalidRegistryException const & ) {
    }

    if ( !xKey.is() )
        return sal_False;

    sal_Bool bSuccess = sal_True;

    for ( sal_Int32 n = 0; n < rServiceNames.getLength(); ++n ) {
        try {
            xKey->createKey( rServiceNames[ n ] );

        } catch ( registry::InvalidRegistryException const & ) {
            bSuccess = sal_False;
            break;
        }
    }
    return bSuccess;
}

extern "C" void SAL_CALL
component_getImplementationEnvironment( const sal_Char  **ppEnvTypeName,
                    uno_Environment **/*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" sal_Bool SAL_CALL
component_writeInfo( void */*pServiceManager*/,
             void *pRegistryKey )
{
    return pRegistryKey &&
        writeInfo( pRegistryKey,
               ::gvfs::ContentProvider::getImplementationName_Static(),
               ::gvfs::ContentProvider::getSupportedServiceNames_Static() );
}
extern "C" void * SAL_CALL
component_getFactory( const sal_Char *pImplName,
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


