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
                public css::ucb::XContentCreator
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
    virtual css::uno::Sequence< css::beans::Property >
    getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual css::uno::Sequence< css::ucb::CommandInfo >
    getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    bool isFolder( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw ( css::uno::Exception );

    css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Sequence< css::beans::Property >& rProperties,
                       const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw ( css::uno::Exception );

    css::uno::Sequence< css::uno::Any >
    setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& rValues,
                       const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw ( css::uno::Exception );

    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren);

    bool
    exchangeIdentity( const css::uno::Reference< css::ucb::XContentIdentifier >& xNewId );

    const OUString
    getBaseURI( const std::unique_ptr< DAVResourceAccess > & rResAccess );

    ResourceType
    getResourceType( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
        throw ( css::uno::Exception );

    ResourceType
    getResourceType( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
                     const std::unique_ptr< DAVResourceAccess > & rResAccess,
                     bool * networkAccessAllowed = 0 )
        throw ( css::uno::Exception );

    // Command "open"
    css::uno::Any open(
                const css::ucb::OpenCommandArgument2 & rArg,
                const css::uno::Reference<
                    css::ucb::XCommandEnvironment > & xEnv )
        throw( css::uno::Exception );

    // Command "post"
    void post( const css::ucb::PostCommandArgument2 & rArg,
               const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
        throw( css::uno::Exception );

    // Command "insert"
    void insert( const css::uno::Reference< css::io::XInputStream > & xInputStream,
                 bool bReplaceExisting,
                 const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
        throw( css::uno::Exception );

    // Command "transfer"
    void transfer( const css::ucb::TransferInfo & rArgs,
                   const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
        throw( css::uno::Exception );

    // Command "delete"
    void destroy( bool bDeletePhysical )
        throw( css::uno::Exception );

    // Command "lock"
    void lock( const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
        throw( css::uno::Exception );

    // Command "unlock"
    void unlock( const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
        throw( css::uno::Exception );

    css::uno::Any MapDAVException( const DAVException & e,
                                                bool bWrite );
    void cancelCommandExecution(
                    const DAVException & e,
                    const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv,
                    bool bWrite = false )
        throw( css::uno::Exception );

    static bool shouldAccessNetworkAfterException( const DAVException & e );

    bool supportsExclusiveWriteLock(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment );

    // XPropertyContainer replacement
    void addProperty( const css::ucb::PropertyCommandArgument &aCmdArg,
                      const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
    throw( css::beans::PropertyExistException,
           css::beans::IllegalTypeException,
           css::lang::IllegalArgumentException,
           css::uno::RuntimeException );

    void removeProperty( const rtl::OUString& Name,
                         const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
    throw( css::beans::UnknownPropertyException,
           css::beans::NotRemoveableException,
           css::uno::RuntimeException );
public:
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory )
        throw ( css::ucb::ContentCreationException );
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory,
             bool isCollection )
        throw ( css::ucb::ContentCreationException );
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
        throw( css::uno::RuntimeException ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw( css::uno::RuntimeException ) override;

    // XContent
    virtual OUString SAL_CALL
    getContentType()
        throw( css::uno::RuntimeException ) override;

    // XCommandProcessor
    virtual css::uno::Any SAL_CALL
    execute( const css::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
        throw( css::uno::Exception,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException ) override;
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( css::uno::RuntimeException ) override;

    // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const OUString& Name,
                 sal_Int16 Attributes,
                 const css::uno::Any& DefaultValue )
        throw( css::beans::PropertyExistException,
               css::beans::IllegalTypeException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException ) override;

    virtual void SAL_CALL
    removeProperty( const OUString& Name )
        throw( css::beans::UnknownPropertyException,
               css::beans::NotRemoveableException,
               css::uno::RuntimeException ) override;


    // Additional interfaces


    // XContentCreator
    virtual css::uno::Sequence< css::ucb::ContentInfo > SAL_CALL
   queryCreatableContentsInfo()
        throw( css::uno::RuntimeException ) override;
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    createNewContent( const css::ucb::ContentInfo& Info )
        throw( css::uno::RuntimeException ) override;


    // Non-interface methods.


    DAVResourceAccess & getResourceAccess() { return *m_xResAccess; }

    // Called from resultset data supplier.
    static css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Reference< css::uno::XComponentContext >& rContext,
                       const css::uno::Sequence< css::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       const rtl::Reference< ::ucbhelper::ContentProviderImplHelper >& rProvider,
                       const OUString& rContentId );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
