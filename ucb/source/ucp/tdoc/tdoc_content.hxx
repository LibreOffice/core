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

#ifndef INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_CONTENT_HXX
#define INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_CONTENT_HXX

#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include "tdoc_provider.hxx"

namespace com::sun::star {
    namespace sdbc  { class XRow; }
    namespace io    { class XInputStream; class XOutputStream; }
    namespace beans { struct PropertyValue; }
    namespace ucb   { struct OpenCommandArgument2; struct TransferInfo;
                      struct ContentInfo; }
}

namespace tdoc_ucp
{


enum ContentType { STREAM, FOLDER, DOCUMENT, ROOT };

class ContentProperties
{
public:
    ContentProperties()
    : m_eType( STREAM )
    {}

    ContentProperties( const ContentType & rType, const OUString & rTitle )
    : m_eType( rType ),
      m_aContentType( rType == STREAM
        ? OUString( TDOC_STREAM_CONTENT_TYPE )
        : rType == FOLDER
            ? OUString( TDOC_FOLDER_CONTENT_TYPE )
            : rType == DOCUMENT
                ? OUString( TDOC_DOCUMENT_CONTENT_TYPE )
                : OUString( TDOC_ROOT_CONTENT_TYPE ) ),
      m_aTitle( rTitle )
    {}

    ContentType getType() const { return m_eType; }

    // Properties

    const OUString & getContentType() const { return m_aContentType; }

    bool getIsFolder()   const { return m_eType > STREAM; }
    bool getIsDocument() const { return !getIsFolder(); }

    const OUString & getTitle() const { return m_aTitle; }
    void setTitle( const OUString & rTitle ) { m_aTitle = rTitle; }

    css::uno::Sequence< css::ucb::ContentInfo >
    getCreatableContentsInfo() const;

    bool isContentCreator() const;

private:
    ContentType   m_eType;
    OUString m_aContentType;
    OUString m_aTitle;
};


class Content : public ::ucbhelper::ContentImplHelper,
                public css::ucb::XContentCreator
{
    enum ContentState { TRANSIENT,  // created via createNewContent,
                                        // but did not process "insert" yet
                        PERSISTENT, // processed "insert"
                        DEAD        // processed "delete" / document was closed
                      };

    ContentProperties m_aProps;
    ContentState      m_eState;
    ContentProvider*  m_pProvider;

private:
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
            const ContentProperties & rProps );
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
             const css::ucb::ContentInfo& Info );

    virtual css::uno::Sequence< css::beans::Property >
    getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual css::uno::Sequence< css::ucb::CommandInfo >
    getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    static bool hasData( ContentProvider const * pProvider, const Uri & rUri );
    bool hasData( const Uri & rUri ) { return hasData( m_pProvider, rUri ); }

    static bool loadData( ContentProvider const * pProvider,
                          const Uri & rUri,
                          ContentProperties& rProps );
    /// @throws css::ucb::CommandFailedException
    /// @throws css::task::DocumentPasswordRequest
    /// @throws css::uno::RuntimeException
    bool storeData( const css::uno::Reference< css::io::XInputStream >& xData,
                    const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );
    void renameData( const css::uno::Reference< css::ucb::XContentIdentifier >& xOldId,
                     const css::uno::Reference< css::ucb::XContentIdentifier >& xNewId );
    bool removeData();

    bool copyData( const Uri & rSourceUri, const OUString & rNewName );

    css::uno::Reference< css::ucb::XContentIdentifier >
    makeNewIdentifier( const OUString& rTitle );

    typedef rtl::Reference< Content > ContentRef;
    typedef std::vector< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren );

    bool exchangeIdentity(
                const css::uno::Reference< css::ucb::XContentIdentifier >& xNewId );

    css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Sequence< css::beans::Property >& rProperties );
    css::uno::Sequence< css::uno::Any >
    /// @throws css::uno::Exception
    setPropertyValues(
            const css::uno::Sequence< css::beans::PropertyValue >& rValues,
            const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    css::uno::Any
    open( const css::ucb::OpenCommandArgument2& rArg,
          const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    void insert( const css::uno::Reference< css::io::XInputStream >& xData,
                 sal_Int32 nNameClashResolve,
                 const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void destroy( bool bDeletePhysical,
                  const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void transfer( const css::ucb::TransferInfo& rInfo,
                   const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    static css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                       const css::uno::Sequence< css::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       ContentProvider* pProvider,
                       const OUString& rContentId );


    static bool commitStorage(
        const css::uno::Reference< css::embed::XStorage > & xStorage );

    static bool closeOutputStream(
        const css::uno::Reference< css::io::XOutputStream > & xOut );

    /// @throws css::ucb::CommandFailedException
    /// @throws css::task::DocumentPasswordRequest
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::io::XInputStream >
    getInputStream( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::ucb::CommandFailedException
    /// @throws css::task::DocumentPasswordRequest
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::io::XOutputStream >
    getTruncatedOutputStream(
        const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    css::uno::Reference< css::ucb::XContent >
    queryChildContent( const OUString & rRelativeChildUri );

    /// @throws css::ucb::CommandFailedException
    /// @throws css::task::DocumentPasswordRequest
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::io::XStream >
    getStream( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

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
    virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
    getIdentifier() override;

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


    static css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                       const css::uno::Sequence< css::beans::Property >& rProperties,
                       ContentProvider* pProvider,
                       const OUString& rContentId );

    void notifyDocumentClosed();
    void notifyChildRemoved( const OUString & rRelativeChildUri );
    void notifyChildInserted( const OUString & rRelativeChildUri );

    rtl::Reference< ContentProvider > getContentProvider() const
    { return rtl::Reference< ContentProvider >( m_pProvider ); }
};

} // namespace tdoc_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_CONTENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
