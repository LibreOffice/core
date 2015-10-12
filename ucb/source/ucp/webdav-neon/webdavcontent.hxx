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

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_WEBDAVCONTENT_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_WEBDAVCONTENT_HXX

#include <config_lgpl.h>
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
    struct OpenCommandArgument3;
    struct PostCommandArgument2;
    struct PropertyCommandArgument;
    struct TransferInfo;
} } } }

namespace webdav_ucp
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
        UNKNOWN,    // the type of the Web resource is unknown
        NOT_FOUND,  // the Web resource does not exists
        FTP,        // the Web resource exists but it's ftp
        NON_DAV,    // the Web resource exists but it's not DAV
        DAV,        // the type of the Web resource is DAV with lock/unlock available
        DAV_NOLOCK  // the type of the Web resource is DAV with no lock/unlock available
    };

    std::unique_ptr< DAVResourceAccess > m_xResAccess;
    std::unique_ptr< CachableContentProperties >
                      m_xCachedProps; // locally cached props
    OUString     m_aEscapedTitle;
    ResourceType      m_eResourceType;
    ResourceType      m_eResourceTypeForLocks;
    ContentProvider*  m_pProvider; // No need for a ref, base class holds object
    rtl::Reference< DAVSessionFactory > m_rSessionFactory;
    bool              m_bTransient;
    bool              m_bCollection;
    bool              m_bDidGetOrHead;
    std::vector< OUString > m_aFailedPropNames;

private:
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment > & xEnv ) override;
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    bool isFolder( const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception, std::exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                       const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception, std::exception );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::PropertyValue >& rValues,
                       const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception, std::exception );

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
        throw ( ::com::sun::star::uno::Exception, std::exception );

    ResourceType
    getResourceType( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::ucb::XCommandEnvironment >& xEnv,
                     const std::unique_ptr< DAVResourceAccess > & rResAccess,
                     bool * networkAccessAllowed = 0)
        throw ( ::com::sun::star::uno::Exception, std::exception );

    // Command "open"
    com::sun::star::uno::Any open(
                const com::sun::star::ucb::OpenCommandArgument3 & rArg,
                const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw (css::uno::Exception, std::exception);

    // Command "post"
    void post( const com::sun::star::ucb::PostCommandArgument2 & rArg,
               const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception, std::exception );

    // Command "insert"
    void insert( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::io::XInputStream > & xInputStream,
                 bool bReplaceExisting,
                 const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception, std::exception );

    // Command "transfer"
    void transfer( const ::com::sun::star::ucb::TransferInfo & rArgs,
                   const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception, std::exception );

    // Command "delete"
    void destroy( bool bDeletePhysical )
        throw( ::com::sun::star::uno::Exception, std::exception );

    // Command "lock"
    void lock( const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception, std::exception );

    // Command "unlock"
    void unlock( const com::sun::star::uno::Reference<
                  com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception, std::exception );

    ::com::sun::star::uno::Any MapDAVException( const DAVException & e,
                                                bool bWrite );
    void cancelCommandExecution(
                    const DAVException & e,
                    const ::com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment > & xEnv,
                    bool bWrite = false )
        throw( ::com::sun::star::uno::Exception, std::exception );

    static bool shouldAccessNetworkAfterException( const DAVException & e );

    ResourceType resourceTypeForLocks(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& Environment );

    void addProperty( const com::sun::star::ucb::PropertyCommandArgument &aCmdArg,
                      const com::sun::star::uno::Reference<
                      com::sun::star::ucb::XCommandEnvironment >& Environment )
    throw( com::sun::star::beans::PropertyExistException,
           com::sun::star::beans::IllegalTypeException,
           com::sun::star::lang::IllegalArgumentException,
           com::sun::star::uno::RuntimeException,
           std::exception );

    void removeProperty( const OUString& Name,
                         const com::sun::star::uno::Reference<
                         com::sun::star::ucb::XCommandEnvironment >& Environment )
    throw( com::sun::star::beans::UnknownPropertyException,
           com::sun::star::beans::NotRemoveableException,
           com::sun::star::uno::RuntimeException,
           std::exception );

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

    // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const OUString& Name,
                 sal_Int16 Attributes,
                 const com::sun::star::uno::Any& DefaultValue )
        throw( com::sun::star::beans::PropertyExistException,
               com::sun::star::beans::IllegalTypeException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    removeProperty( const OUString& Name )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::NotRemoveableException,
               com::sun::star::uno::RuntimeException, std::exception ) override;


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


    DAVResourceAccess & getResourceAccess() { return *m_xResAccess; }

    // Called from resultset data supplier.
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                           ::com::sun::star::uno::XComponentContext >& rxContext,
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
