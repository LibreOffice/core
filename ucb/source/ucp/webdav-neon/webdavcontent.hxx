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

#pragma once

#include <config_lgpl.h>
#include <memory>
#include <list>
#include <rtl/ref.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include "DAVResourceAccess.hxx"
#include "PropertyMap.hxx"

namespace com::sun::star::beans {
    struct Property;
    struct PropertyValue;
}

namespace com::sun::star::io {
    class XInputStream;
}

namespace com::sun::star::sdbc {
    class XRow;
}

namespace com::sun::star::ucb {
    struct OpenCommandArgument3;
    struct PostCommandArgument2;
    struct PropertyCommandArgument;
    struct TransferInfo;
}

namespace webdav_ucp
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
    // resource type for general DAV methods
    ResourceType      m_eResourceType;
    // resource type for general LOCK method only
    ResourceType      m_eResourceTypeForLocks;
    ContentProvider*  m_pProvider; // No need for a ref, base class holds object
    bool              m_bTransient;
    bool              m_bCollection;
    bool              m_bDidGetOrHead;
    std::vector< OUString > m_aFailedPropNames;
    // Options Cache lifetime
    // for web site implementing OPTIONS, but not dav
    sal_uInt32 m_nOptsCacheLifeImplWeb;
    // for WebDAV site where OPTIONS is mandatory
    sal_uInt32 m_nOptsCacheLifeDAV;
    // same as above, but when the resource is locked by us
    sal_uInt32 m_nOptsCacheLifeDAVLocked;
// For web site not implementing OPTIONS
    // during this time we assume the site doesn't turn to WebDAV
    // but remains a simple Web
    sal_uInt32 m_nOptsCacheLifeNotImpl;
    // When resource is not found
    // may be the resource is unavailable only briefly?
    // so better have this small
    sal_uInt32 m_nOptsCacheLifeNotFound;

    void initOptsCacheLifeTime();

private:
    virtual css::uno::Sequence< css::beans::Property >
    getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual css::uno::Sequence< css::ucb::CommandInfo >
    getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    /// @throws css::uno::Exception
    bool isFolder( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Sequence< css::beans::Property >& rProperties,
                       const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    css::uno::Sequence< css::uno::Any >
    setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& rValues,
                       const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    typedef rtl::Reference< Content > ContentRef;
    typedef std::vector< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren);

    bool
    exchangeIdentity( const css::uno::Reference< css::ucb::XContentIdentifier >& xNewId );

    OUString
    getBaseURI( const std::unique_ptr< DAVResourceAccess > & rResAccess );

    /// @throws css::uno::Exception
    ResourceType
    getResourceType( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    ResourceType
    getResourceType( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
                     const std::unique_ptr< DAVResourceAccess > & rResAccess,
                     bool * networkAccessAllowed = nullptr);

    // Command "open"
    /// @throws css::uno::Exception
    css::uno::Any open(
                const css::ucb::OpenCommandArgument3 & rArg,
                const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    // Command "post"
    /// @throws css::uno::Exception
    void post( const css::ucb::PostCommandArgument2 & rArg,
               const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    // Command "insert"
    /// @throws css::uno::Exception
    void insert( const css::uno::Reference< css::io::XInputStream > & xInputStream,
                 bool bReplaceExisting,
                 const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment );

    // Command "transfer"
    /// @throws css::uno::Exception
    void transfer( const css::ucb::TransferInfo & rArgs,
                   const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment );

    // Command "delete"
    /// @throws css::uno::Exception
    void destroy( bool bDeletePhysical );

    // Command "lock"
    /// @throws css::uno::Exception
    void lock( const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment );

    // Command "unlock"
    /// @throws css::uno::Exception
    void unlock( const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment );

    css::uno::Any MapDAVException( const DAVException & e,
                                                bool bWrite );
    /// @throws css::uno::Exception
    void cancelCommandExecution(
                    const DAVException & e,
                    const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv,
                    bool bWrite = false );

    static bool shouldAccessNetworkAfterException( const DAVException & e );

    ResourceType resourceTypeForLocks(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& rEnvironment,
        const std::unique_ptr< DAVResourceAccess > & rResAccess );

    ResourceType resourceTypeForLocks(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& rEnvironment );

    /// @throws css::beans::PropertyExistException
    /// @throws css::beans::IllegalTypeException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void addProperty( const css::ucb::PropertyCommandArgument &aCmdArg,
                      const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment );

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::beans::NotRemoveableException
    /// @throws css::uno::RuntimeException
    void removeProperty( const OUString& Name,
                         const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment );

public:
    /// @throws css::ucb::ContentCreationException
    /// @throws css::uno::RuntimeException
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory );
    /// @throws css::ucb::ContentCreationException
    /// @throws css::uno::RuntimeException
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory,
             bool isCollection );
    virtual ~Content() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        noexcept override;
    virtual void SAL_CALL release()
        noexcept override;

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

    // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const OUString& Name,
                 sal_Int16 Attributes,
                 const css::uno::Any& DefaultValue ) override;

    virtual void SAL_CALL
    removeProperty( const OUString& Name ) override;


    // Additional interfaces


    // XContentCreator
    virtual css::uno::Sequence< css::ucb::ContentInfo > SAL_CALL
   queryCreatableContentsInfo() override;
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    createNewContent( const css::ucb::ContentInfo& Info ) override;


    // Non-interface methods.


    DAVResourceAccess & getResourceAccess() { return *m_xResAccess; }

    // Called from resultset data supplier.
    static css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                       const css::uno::Sequence< css::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       const rtl::Reference< ::ucbhelper::ContentProviderImplHelper >& rProvider,
                       const OUString& rContentId );

    // Use OPTIONS method to retrieve the type of the Web resource
    /// @throws css::uno::Exception
    void getResourceOptions( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
                             DAVOptions& rDAVOptions,
                             const std::unique_ptr< DAVResourceAccess > & rResAccess,
                             bool * networkAccessAllowed = nullptr);

    static bool isResourceAvailable( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
                             const std::unique_ptr< DAVResourceAccess > & rResAccess,
                                     DAVOptions& rDAVOptions );

    static void removeCachedPropertyNames( const OUString & rURL );

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
