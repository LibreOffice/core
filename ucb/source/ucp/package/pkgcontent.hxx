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

#ifndef _PKGCONTENT_HXX
#define _PKGCONTENT_HXX

#include <list>
#include <rtl/ref.hxx>

#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include "pkguri.hxx"

namespace com { namespace sun { namespace star {
    namespace beans
    {
        struct Property;
        struct PropertyValue;
    }
    namespace container
    {
        class XHierarchicalNameAccess;
        class XEnumeration;
    }
    namespace io
    {
        class XInputStream;
    }
    namespace sdbc
    {
        class XRow;
    }
    namespace ucb
    {
        struct OpenCommandArgument2;
        struct TransferInfo;
    }
} } }

namespace package_ucp
{

//=========================================================================

// UNO service name for the content.
#define PACKAGE_FOLDER_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.PackageFolderContent"
#define PACKAGE_STREAM_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.PackageStreamContent"

//=========================================================================

struct ContentProperties
{
    ::rtl::OUString  aTitle;                // Title
    ::rtl::OUString  aContentType;          // ContentType
    sal_Bool         bIsDocument;           // IsDocument
    sal_Bool         bIsFolder;             // IsFolder
    ::rtl::OUString  aMediaType;            // MediaType
    com::sun::star::uno::Sequence <
        sal_Int8 >   aEncryptionKey;        // EncryptionKey
    sal_Int64        nSize;                 // Size
    sal_Bool         bCompressed;           // Compressed
    sal_Bool         bEncrypted;            // Encrypted
    sal_Bool         bHasEncryptedEntries;  // HasEncryptedEntries

    ContentProperties()
    : bIsDocument( sal_True ), bIsFolder( sal_False ), nSize( 0 ),
      bCompressed( sal_True ), bEncrypted( sal_False ),
      bHasEncryptedEntries( sal_False ) {}

    ContentProperties( const ::rtl::OUString& rContentType );

    com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
    getCreatableContentsInfo( PackageUri const & rUri ) const;
};

//=========================================================================

class ContentProvider;

class Content : public ::ucbhelper::ContentImplHelper,
                public com::sun::star::ucb::XContentCreator
{
    enum ContentState { TRANSIENT,  // created via CreateNewContent,
                                        // but did not process "insert" yet
                        PERSISTENT, // processed "insert"
                        DEAD        // processed "delete"
                      };

    PackageUri              m_aUri;
    ContentProperties       m_aProps;
    ContentState            m_eState;
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess > m_xPackage;
    ContentProvider*        m_pProvider;
    sal_uInt32              m_nModifiedProps;

private:
    Content( const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
             const ::com::sun::star::uno::Reference<
                com::sun::star::container::XHierarchicalNameAccess >& Package,
             const PackageUri& rUri,
             const ContentProperties& rProps );
    Content( const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
             const com::sun::star::uno::Reference<
                com::sun::star::container::XHierarchicalNameAccess >& Package,
             const PackageUri& rUri,
             const com::sun::star::ucb::ContentInfo& Info );

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual ::rtl::OUString getParentURL();

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       const rtl::Reference<
                            ::ucbhelper::ContentProviderImplHelper >& rProvider,
                       const ::rtl::OUString& rContentId );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::Property >& rProperties );
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::PropertyValue >& rValues,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getPackage( const PackageUri& rURI );
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getPackage();

    static sal_Bool
    loadData( ContentProvider* pProvider,
              const PackageUri& rURI,
              ContentProperties& rProps,
              com::sun::star::uno::Reference<
                com::sun::star::container::XHierarchicalNameAccess > &
                    rxPackage );
    static sal_Bool
    hasData( ContentProvider* pProvider,
             const PackageUri& rURI,
             com::sun::star::uno::Reference<
                com::sun::star::container::XHierarchicalNameAccess > &
                    rxPackage );

    sal_Bool
    hasData( const PackageUri& rURI );
    sal_Bool
    renameData( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& xOldId,
                const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& xNewId );
    sal_Bool
    storeData( const com::sun::star::uno::Reference<
                    com::sun::star::io::XInputStream >& xStream );
    sal_Bool
    removeData();

    sal_Bool
    flushData();

    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren );

    sal_Bool
    exchangeIdentity( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    ::com::sun::star::uno::Any
    open( const ::com::sun::star::ucb::OpenCommandArgument2& rArg,
          const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    void insert( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::io::XInputStream >& xStream,
                 sal_Int32 nNameClashResolve,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    void destroy( sal_Bool bDeletePhysical,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    void transfer( const ::com::sun::star::ucb::TransferInfo& rInfo,
                   const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
    getInputStream();

    sal_Bool isFolder() const { return m_aProps.bIsFolder; }

public:
    // Create existing content. Fail, if not already exists.
    static Content* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static Content* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual ~Content();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
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
                com::sun::star::ucb::XCommandEnvironment >& Environment )
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

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    // Called from resultset data supplier.
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::Property >& rProperties,
                       ContentProvider* pProvider,
                       const ::rtl::OUString& rContentId );

    // Called from resultset data supplier.
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XEnumeration >
    getIterator();

    static ::rtl::OUString
    getContentType( const ::rtl::OUString& aScheme,  sal_Bool bFolder );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
