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
#ifndef _GVFS_UCP_CONTENT_HXX
#define _GVFS_UCP_CONTENT_HXX

#include <memory>
#include <list>
#include <rtl/ref.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>

#include <glib/gthread.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-directory.h>

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
    class XOutputStream;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    struct TransferInfo;
} } } }

namespace gvfs
{

class ContentProvider;
class ContentProperties;

// Random made up names - AFAICS
#define GVFS_FILE_TYPE   "application/vnd.sun.staroffice.gvfs-file"
#define GVFS_FOLDER_TYPE "application/vnd.sun.staroffice.gvfs-folder"

class Authentication
{
public:
    // Helper class to make exceptions pleasant
    Authentication( const com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment > & xEnv );
    ~Authentication();
};

class Content : public ::ucbhelper::ContentImplHelper,
        public com::sun::star::ucb::XContentCreator
{
//=========================================================================
//                             Internals
//=========================================================================
private:
    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;

    // Instance data
    ContentProvider  *m_pProvider;  // No need for a ref, base class holds object
    sal_Bool          m_bTransient; // A non-existant (as yet) item
    GnomeVFSFileInfo  m_info;       // cached status information

    // Internal helpers
    void                       queryChildren   ( ContentRefList& rChildren );
    ::com::sun::star::uno::Any getBadArgExcept ();
    GnomeVFSResult             getInfo         ( const ::com::sun::star::uno::Reference<
                             ::com::sun::star::ucb::XCommandEnvironment >& xEnv );
    sal_Bool                   isFolder        ( const ::com::sun::star::uno::Reference<
                             ::com::sun::star::ucb::XCommandEnvironment >& xEnv );
    sal_Bool                   exchangeIdentity( const ::com::sun::star::uno::Reference<
                             ::com::sun::star::ucb::XContentIdentifier >&  xNewId);
    GnomeVFSResult             doSetFileInfo   ( const GnomeVFSFileInfo                       *newInfo,
                             GnomeVFSSetFileInfoMask                       setMask,
                             const ::com::sun::star::uno::Reference<
                             ::com::sun::star::ucb::XCommandEnvironment >& xEnv );
    ::rtl::OUString            makeNewURL      ( const char *newName );
    // End Internal helpers

    // For ucbhelper
    virtual ::rtl::OUString getParentURL();
    // For ucbhelper
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
               com::sun::star::ucb::XCommandEnvironment > & xEnv );
    // For ucbhelper
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
             com::sun::star::ucb::XCommandEnvironment > & xEnv );

public:
    // Command "getPropertyValues"
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
               ::com::sun::star::beans::Property >& rProperties,
               const ::com::sun::star::uno::Reference<
               ::com::sun::star::ucb::XCommandEnvironment >& xEnv );

private:
    // Command "setPropertyValues"
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
               ::com::sun::star::beans::PropertyValue >& rValues,
               const ::com::sun::star::uno::Reference<
               ::com::sun::star::ucb::XCommandEnvironment >& xEnv );

    // Command "insert"
    void insert( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::io::XInputStream > & xInputStream,
                 sal_Bool bReplaceExisting,
                 const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( ::com::sun::star::uno::Exception );

    // Command "transfer"
    void transfer( const ::com::sun::star::ucb::TransferInfo & rArgs,
               const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( ::com::sun::star::uno::Exception );

    // Command "delete"
    void destroy( sal_Bool bDeletePhysical )
        throw( ::com::sun::star::uno::Exception );

    // "open" helpers
    void    copyData( ::com::sun::star::uno::Reference<
                ::com::sun::star::io::XInputStream > xIn,
              ::com::sun::star::uno::Reference<
                    ::com::sun::star::io::XOutputStream > xOut );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::io::XInputStream >
        createTempStream(  const ::com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( ::com::sun::star::uno::Exception );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::io::XInputStream >
        createInputStream(  const ::com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( ::com::sun::star::uno::Exception );
            sal_Bool feedSink( ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface> aSink,
               const ::com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment >& xEnv );

    ::com::sun::star::uno::Any mapVFSException( const GnomeVFSResult result,
                            sal_Bool bWrite );

    void cancelCommandExecution(const GnomeVFSResult result,
                    const ::com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv,
                    sal_Bool bWrite = sal_False )
        throw( ::com::sun::star::uno::Exception );


public:
    // Non-interface bits
    char         *getURI ();
    rtl::OString  getOURI ();
    rtl::OUString getOUURI ();

//=========================================================================
//                            Externals
//=========================================================================
public:

    Content( const ::com::sun::star::uno::Reference<
         ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
         ContentProvider                                *pProvider,
         const ::com::sun::star::uno::Reference<
         ::com::sun::star::ucb::XContentIdentifier >&    Identifier)
            throw ( ::com::sun::star::ucb::ContentCreationException );
    Content( const ::com::sun::star::uno::Reference<
         ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
         ContentProvider                                *pProvider,
         const ::com::sun::star::uno::Reference<
         ::com::sun::star::ucb::XContentIdentifier >&    Identifier,
         sal_Bool                                        isFolder)
            throw ( ::com::sun::star::ucb::ContentCreationException );
    virtual ~Content();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException );

    // XContent
    virtual rtl::OUString SAL_CALL
    getContentType()
        throw( com::sun::star::uno::RuntimeException );

    // XCommandProcessor
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
         sal_Int32 CommandId,
         const com::sun::star::uno::Reference<
         com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    // XContentCreator
    virtual com::sun::star::uno::Sequence<
        com::sun::star::ucb::ContentInfo > SAL_CALL
    queryCreatableContentsInfo()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
    createNewContent( const com::sun::star::ucb::ContentInfo& Info )
        throw( com::sun::star::uno::RuntimeException );


    com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
    queryCreatableContentsInfo(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& xEnv)
        throw( com::sun::star::uno::RuntimeException );
};

}

extern "C" {
    extern GPrivate *auth_queue;
    extern void auth_queue_destroy( gpointer data );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
