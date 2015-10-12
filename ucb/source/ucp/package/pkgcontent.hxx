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

#ifndef INCLUDED_UCB_SOURCE_UCP_PACKAGE_PKGCONTENT_HXX
#define INCLUDED_UCB_SOURCE_UCP_PACKAGE_PKGCONTENT_HXX

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



struct ContentProperties
{
    OUString  aTitle;                // Title
    OUString  aContentType;          // ContentType
    bool         bIsDocument;           // IsDocument
    bool         bIsFolder;             // IsFolder
    OUString  aMediaType;            // MediaType
    com::sun::star::uno::Sequence <
        sal_Int8 >   aEncryptionKey;        // EncryptionKey
    sal_Int64        nSize;                 // Size
    bool         bCompressed;           // Compressed
    bool         bEncrypted;            // Encrypted
    bool         bHasEncryptedEntries;  // HasEncryptedEntries

    ContentProperties()
    : bIsDocument( true ), bIsFolder( false ), nSize( 0 ),
      bCompressed( true ), bEncrypted( false ),
      bHasEncryptedEntries( false ) {}

    explicit ContentProperties( const OUString& rContentType );

    com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
    getCreatableContentsInfo( PackageUri const & rUri ) const;
};



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
                com::sun::star::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
             const ::com::sun::star::uno::Reference<
                com::sun::star::container::XHierarchicalNameAccess >& Package,
             const PackageUri& rUri,
             const ContentProperties& rProps );
    Content( const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
             const com::sun::star::uno::Reference<
                com::sun::star::container::XHierarchicalNameAccess >& Package,
             const PackageUri& rUri,
             const com::sun::star::ucb::ContentInfo& Info );

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv ) override;
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XComponentContext >& rxContext,
                       const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       const rtl::Reference<
                            ::ucbhelper::ContentProviderImplHelper >& rProvider,
                       const OUString& rContentId );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::Property >& rProperties );
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::PropertyValue >& rValues,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception, std::exception );

    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getPackage( const PackageUri& rURI );
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getPackage();

    static bool
    loadData( ContentProvider* pProvider,
              const PackageUri& rURI,
              ContentProperties& rProps,
              com::sun::star::uno::Reference<
                com::sun::star::container::XHierarchicalNameAccess > &
                    rxPackage );
    static bool
    hasData( ContentProvider* pProvider,
             const PackageUri& rURI,
             com::sun::star::uno::Reference<
                com::sun::star::container::XHierarchicalNameAccess > &
                    rxPackage );

    bool
    hasData( const PackageUri& rURI );
    bool
    renameData( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& xOldId,
                const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& xNewId );
    bool
    storeData( const com::sun::star::uno::Reference<
                    com::sun::star::io::XInputStream >& xStream );
    bool
    removeData();

    bool
    flushData();

    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren );

    bool
    exchangeIdentity( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    ::com::sun::star::uno::Any
    open( const ::com::sun::star::ucb::OpenCommandArgument2& rArg,
          const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception, std::exception );

    void insert( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::io::XInputStream >& xStream,
                 sal_Int32 nNameClashResolve,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception, std::exception );

    void destroy( bool bDeletePhysical,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception, std::exception );

    void transfer( const ::com::sun::star::ucb::TransferInfo& rInfo,
                   const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception, std::exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
    getInputStream();

    bool isFolder() const { return m_aProps.bIsFolder; }

public:
    // Create existing content. Fail, if not already exists.
    static Content* create(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
            ContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static Content* create(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
            ContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual ~Content();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XContent
    virtual OUString SAL_CALL
    getContentType()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XCommandProcessor
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;


    // Additional interfaces


    // XContentCreator
    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::ContentInfo > SAL_CALL
    queryCreatableContentsInfo()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    createNewContent( const com::sun::star::ucb::ContentInfo& Info )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;


    // Non-interface methods.


    // Called from resultset data supplier.
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XComponentContext >& rxContext,
                       const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::Property >& rProperties,
                       ContentProvider* pProvider,
                       const OUString& rContentId );

    // Called from resultset data supplier.
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XEnumeration >
    getIterator();

    static OUString
    getContentType( const OUString& aScheme,  bool bFolder );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
