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



#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_WEBDAVCONTENT_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_WEBDAVCONTENT_HXX

#include <memory>
#include <list>
#include <rtl/ref.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include "DAVResourceAccess.hxx"
#include "PropertyMap.hxx"

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    struct OpenCommandArgument2;
    struct PropertyCommandArgument;
    struct PostCommandArgument2;
    struct TransferInfo;
} } } }

namespace http_dav_ucp
{



// UNO service name for the content.
#define WEBDAV_CONTENT_SERVICE_NAME "com.sun.star.ucb.WebDAVContent"



class ContentProvider;
class ContentProperties;
class CachableContentProperties;

class Content : public ::ucbhelper::ContentImplHelper,
                public com::sun::star::ucb::XContentCreator
{
    enum ResourceType
    {
        UNKNOWN,
        NON_DAV,
        DAV
    };

    std::unique_ptr< DAVResourceAccess > m_xResAccess;
    std::unique_ptr< CachableContentProperties > m_xCachedProps; // locally cached props
    OUString     m_aEscapedTitle;
    ResourceType      m_eResourceType;
    ContentProvider*  m_pProvider; // No need for a ref, base class holds object
    bool              m_bTransient;
    bool              m_bLocked;
    bool              m_bCollection;
    bool              m_bDidGetOrHead;
    std::vector< OUString > m_aFailedPropNames;

private:
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment > & xEnv ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment > & xEnv ) SAL_OVERRIDE;
    virtual OUString getParentURL() SAL_OVERRIDE;

    bool isFolder( const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                       const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::PropertyValue >& rValues,
                       const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren);

    bool
    exchangeIdentity( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    const OUString
    getBaseURI( const std::unique_ptr< DAVResourceAccess > & rResAccess );

    ResourceType
    getResourceType( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    ResourceType
    getResourceType( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::ucb::XCommandEnvironment >& xEnv,
                     const std::unique_ptr< DAVResourceAccess > & rResAccess,
                     bool * networkAccessAllowed = 0 )
        throw ( ::com::sun::star::uno::Exception );

    // Command "open"
    com::sun::star::uno::Any open(
                const com::sun::star::ucb::OpenCommandArgument2 & rArg,
                const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    // Command "post"
    void post( const com::sun::star::ucb::PostCommandArgument2 & rArg,
               const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    // Command "insert"
    void insert( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::io::XInputStream > & xInputStream,
                 bool bReplaceExisting,
                 const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

    // Command "transfer"
    void transfer( const ::com::sun::star::ucb::TransferInfo & rArgs,
                   const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

    // Command "delete"
    void destroy( bool bDeletePhysical )
        throw( ::com::sun::star::uno::Exception );

    // Command "lock"
    void lock( const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

    // Command "unlock"
    void unlock( const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Any MapDAVException( const DAVException & e,
                                                bool bWrite );
    void cancelCommandExecution(
                    const DAVException & e,
                    const ::com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment > & xEnv,
                    bool bWrite = false )
        throw( ::com::sun::star::uno::Exception );

    static bool shouldAccessNetworkAfterException( const DAVException & e );

    bool supportsExclusiveWriteLock(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& Environment );

    // XPropertyContainer replacement
    void addProperty( const com::sun::star::ucb::PropertyCommandArgument &aCmdArg,
                      const com::sun::star::uno::Reference<
                      com::sun::star::ucb::XCommandEnvironment >& Environment )
    throw( com::sun::star::beans::PropertyExistException,
           com::sun::star::beans::IllegalTypeException,
           com::sun::star::lang::IllegalArgumentException,
           com::sun::star::uno::RuntimeException );

    void removeProperty( const rtl::OUString& Name,
                         const com::sun::star::uno::Reference<
                         com::sun::star::ucb::XCommandEnvironment >& Environment )
    throw( com::sun::star::beans::UnknownPropertyException,
           com::sun::star::beans::NotRemoveableException,
           com::sun::star::uno::RuntimeException );
public:
    Content( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const ::com::sun::star::uno::Reference<
                 ::com::sun::star::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory )
        throw ( ::com::sun::star::ucb::ContentCreationException );
    Content( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const ::com::sun::star::uno::Reference<
                 ::com::sun::star::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory,
             bool isCollection )
        throw ( ::com::sun::star::ucb::ContentCreationException );
    virtual ~Content();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw() SAL_OVERRIDE;
    virtual void SAL_CALL release()
        throw() SAL_OVERRIDE;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XContent
    virtual OUString SAL_CALL
    getContentType()
        throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XCommandProcessor
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const OUString& Name,
                 sal_Int16 Attributes,
                 const com::sun::star::uno::Any& DefaultValue )
        throw( com::sun::star::beans::PropertyExistException,
               com::sun::star::beans::IllegalTypeException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual void SAL_CALL
    removeProperty( const OUString& Name )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::NotRemoveableException,
               com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;


    // Additional interfaces


    // XContentCreator
    virtual com::sun::star::uno::Sequence<
        com::sun::star::ucb::ContentInfo > SAL_CALL
   queryCreatableContentsInfo()
        throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
    createNewContent( const com::sun::star::ucb::ContentInfo& Info )
        throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;


    // Non-interface methods.


    DAVResourceAccess & getResourceAccess() { return *m_xResAccess; }

    // Called from resultset data supplier.
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                           ::com::sun::star::uno::XComponentContext >& rContext,
                       const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       const rtl::Reference<
                           ::ucbhelper::ContentProviderImplHelper >& rProvider,
                       const OUString& rContentId );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
