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

#pragma once

#include <sal/config.h>

#include <string_view>
#include <vector>
#include <rtl/ref.hxx>

#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include "pkguri.hxx"

namespace com::sun::star {
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
}

namespace package_ucp
{


struct ContentProperties
{
    OUString     aTitle;                // Title
    OUString     aContentType;          // ContentType
    bool         bIsDocument;           // IsDocument
    bool         bIsFolder;             // IsFolder
    OUString     aMediaType;            // MediaType
    css::uno::Sequence < sal_Int8 >   aEncryptionKey;        // EncryptionKey
    sal_Int64    nSize;                 // Size
    bool         bCompressed;           // Compressed
    bool         bEncrypted;            // Encrypted
    bool         bHasEncryptedEntries;  // HasEncryptedEntries

    ContentProperties()
    : bIsDocument( true ), bIsFolder( false ), nSize( 0 ),
      bCompressed( true ), bEncrypted( false ),
      bHasEncryptedEntries( false ) {}

    explicit ContentProperties( const OUString& rContentType );

    css::uno::Sequence< css::ucb::ContentInfo >
    getCreatableContentsInfo( PackageUri const & rUri ) const;
};


class ContentProvider;

class Content : public ::ucbhelper::ContentImplHelper,
                public css::ucb::XContentCreator
{
    enum ContentState { TRANSIENT,  // created via CreateNewContent,
                                        // but did not process "insert" yet
                        PERSISTENT, // processed "insert"
                        DEAD        // processed "delete"
                      };

    PackageUri              m_aUri;
    ContentProperties       m_aProps;
    ContentState            m_eState;
    css::uno::Reference<
        css::container::XHierarchicalNameAccess > m_xPackage;
    ContentProvider*        m_pProvider;
    sal_uInt32              m_nModifiedProps;

private:
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
             const css::uno::Reference< css::container::XHierarchicalNameAccess >& Package,
             const PackageUri& rUri,
             const ContentProperties& rProps );
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
             const css::uno::Reference< css::container::XHierarchicalNameAccess >& Package,
             const PackageUri& rUri,
             const css::ucb::ContentInfo& Info );

    virtual css::uno::Sequence< css::beans::Property >
    getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual css::uno::Sequence< css::ucb::CommandInfo >
    getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    static css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                       const css::uno::Sequence< css::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       const rtl::Reference< ::ucbhelper::ContentProviderImplHelper >& rProvider,
                       const OUString& rContentId );

    css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Sequence< css::beans::Property >& rProperties );
    /// @throws css::uno::Exception
    css::uno::Sequence< css::uno::Any >
    setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& rValues,
                       const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    css::uno::Reference< css::container::XHierarchicalNameAccess >
    getPackage( const PackageUri& rURI );
    css::uno::Reference< css::container::XHierarchicalNameAccess >
    getPackage();

    static bool
    loadData( ContentProvider* pProvider,
              const PackageUri& rURI,
              ContentProperties& rProps,
              css::uno::Reference< css::container::XHierarchicalNameAccess > & rxPackage );
    static bool
    hasData( ContentProvider* pProvider,
             const PackageUri& rURI,
             css::uno::Reference< css::container::XHierarchicalNameAccess > & rxPackage );

    bool
    hasData( const PackageUri& rURI );
    void
    renameData( const css::uno::Reference< css::ucb::XContentIdentifier >& xOldId,
                const css::uno::Reference< css::ucb::XContentIdentifier >& xNewId );
    bool
    storeData( const css::uno::Reference< css::io::XInputStream >& xStream );
    bool
    removeData();

    bool
    flushData();

    typedef rtl::Reference< Content > ContentRef;
    typedef std::vector< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren );

    bool
    exchangeIdentity( const css::uno::Reference<
                        css::ucb::XContentIdentifier >& xNewId );

    /// @throws css::uno::Exception
    css::uno::Any
    open( const css::ucb::OpenCommandArgument2& rArg,
          const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void insert( const css::uno::Reference< css::io::XInputStream >& xStream,
                 sal_Int32 nNameClashResolve,
                 const css::uno::Reference<
                    css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void destroy( bool bDeletePhysical,
                  const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void transfer( const css::ucb::TransferInfo& rInfo,
                   const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    css::uno::Reference< css::io::XInputStream >
    getInputStream();

    bool isFolder() const { return m_aProps.bIsFolder; }

public:
    // Create existing content. Fail, if not already exists.
    static rtl::Reference<Content> create(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            ContentProvider* pProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static rtl::Reference<Content> create(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            ContentProvider* pProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
            const css::ucb::ContentInfo& Info );

    virtual ~Content() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL
    getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    // XContent
    virtual OUString SAL_CALL
    getContentType() override;

    // XCommandProcessor
    virtual css::uno::Any SAL_CALL
    execute( const css::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) override;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId ) override;


    // Additional interfaces


    // XContentCreator
    virtual css::uno::Sequence< css::ucb::ContentInfo > SAL_CALL
    queryCreatableContentsInfo() override;
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    createNewContent( const css::ucb::ContentInfo& Info ) override;


    // Non-interface methods.


    // Called from resultset data supplier.
    static css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                       const css::uno::Sequence< css::beans::Property >& rProperties,
                       ContentProvider* pProvider,
                       const OUString& rContentId );

    // Called from resultset data supplier.
    css::uno::Reference< css::container::XEnumeration >
    getIterator();

    static OUString
    getContentType( std::u16string_view aScheme,  bool bFolder );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
