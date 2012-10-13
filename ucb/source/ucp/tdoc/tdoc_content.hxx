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

#ifndef INCLUDED_TDOC_CONTENT_HXX
#define INCLUDED_TDOC_CONTENT_HXX

#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include "tdoc_provider.hxx"

#define NO_STREAM_CREATION_WITHIN_DOCUMENT_ROOT 1

namespace com { namespace sun { namespace star {
    namespace sdbc  { class XRow; }
    namespace io    { class XInputStream; class XOutputStream; }
    namespace beans { struct PropertyValue; }
    namespace ucb   { struct OpenCommandArgument2; struct TransferInfo;
                      struct ContentInfo; }
} } }

namespace tdoc_ucp
{

//=========================================================================

#define TDOC_ROOT_CONTENT_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsRootContent"
#define TDOC_DOCUMENT_CONTENT_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsDocumentContent"
#define TDOC_FOLDER_CONTENT_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsFolderContent"
#define TDOC_STREAM_CONTENT_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsStreamContent"

//=========================================================================

enum ContentType { STREAM, FOLDER, DOCUMENT, ROOT };

class ContentProperties
{
public:
    ContentProperties()
    : m_eType( STREAM )
    {}

    ContentProperties( const ContentType & rType, const rtl::OUString & rTitle )
    : m_eType( rType ),
      m_aContentType( rType == STREAM
        ? rtl::OUString( TDOC_STREAM_CONTENT_TYPE )
        : rType == FOLDER
            ? rtl::OUString( TDOC_FOLDER_CONTENT_TYPE )
            : rType == DOCUMENT
                ? rtl::OUString( TDOC_DOCUMENT_CONTENT_TYPE )
                : rtl::OUString( TDOC_ROOT_CONTENT_TYPE ) ),
      m_aTitle( rTitle )
    {}

    ContentType getType() const { return m_eType; }

    // Properties

    const rtl::OUString & getContentType() const { return m_aContentType; }

    bool getIsFolder()   const { return m_eType > STREAM; }
    bool getIsDocument() const { return !getIsFolder(); }

    const rtl::OUString & getTitle() const { return m_aTitle; }
    void setTitle( const rtl::OUString & rTitle ) { m_aTitle = rTitle; }

    com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
    getCreatableContentsInfo() const;

    bool isContentCreator() const;

private:
    ContentType   m_eType;
    rtl::OUString m_aContentType;
    rtl::OUString m_aTitle;
};

//=========================================================================

class Content : public ::ucbhelper::ContentImplHelper,
                public com::sun::star::ucb::XContentCreator
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
    Content( const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const ContentProperties & rProps );
    Content( const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
             const com::sun::star::ucb::ContentInfo& Info );

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual ::rtl::OUString getParentURL();

    static bool hasData( ContentProvider* pProvider, const Uri & rUri );
    bool hasData( const Uri & rUri ) { return hasData( m_pProvider, rUri ); }

    static bool loadData( ContentProvider* pProvider,
                          const Uri & rUri,
                          ContentProperties& rProps );
    bool storeData( const com::sun::star::uno::Reference<
                        com::sun::star::io::XInputStream >& xData,
                    const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::ucb::CommandFailedException,
                ::com::sun::star::task::DocumentPasswordRequest );
    bool renameData( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifier >& xOldId,
                     const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifier >& xNewId );
    bool removeData();

    bool copyData( const Uri & rSourceUri, const rtl::OUString & rNewName );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentIdentifier >
    makeNewIdentifier( const rtl::OUString& rTitle );

    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren );

    sal_Bool exchangeIdentity(
                const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::Property >& rProperties );
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues(
            const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >& rValues,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    com::sun::star::uno::Any
    open( const ::com::sun::star::ucb::OpenCommandArgument2& rArg,
          const ::com::sun::star::uno::Reference<
            ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( ::com::sun::star::uno::Exception );

    void insert( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::io::XInputStream >& xData,
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

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       ContentProvider* pProvider,
                       const ::rtl::OUString& rContentId );


    static bool commitStorage(
        const com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage > & xStorage );

    static bool closeOutputStream(
        const com::sun::star::uno::Reference<
            com::sun::star::io::XOutputStream > & xOut );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
    getInputStream( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment > &
                            xEnv )
        throw ( ::com::sun::star::ucb::CommandFailedException,
                ::com::sun::star::task::DocumentPasswordRequest );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
    getTruncatedOutputStream(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( ::com::sun::star::ucb::CommandFailedException,
                ::com::sun::star::task::DocumentPasswordRequest );

    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >
    queryChildContent( const rtl::OUString & rRelativeChildUri );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >
    getStream( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( ::com::sun::star::ucb::CommandFailedException,
                ::com::sun::star::task::DocumentPasswordRequest );

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
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    getIdentifier()
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

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::Property >& rProperties,
                       ContentProvider* pProvider,
                       const ::rtl::OUString& rContentId );

    void notifyDocumentClosed();
    void notifyChildRemoved( const rtl::OUString & rRelativeChildUri );
    void notifyChildInserted( const rtl::OUString & rRelativeChildUri );

    rtl::Reference< ContentProvider > getContentProvider() const
    { return rtl::Reference< ContentProvider >( m_pProvider ); }
};

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_CONTENT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
