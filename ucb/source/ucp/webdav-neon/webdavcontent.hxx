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

#ifndef _WEBDAV_UCP_CONTENT_HXX
#define _WEBDAV_UCP_CONTENT_HXX

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
    struct TransferInfo;
} } } }

namespace webdav_ucp
{

//=========================================================================

// UNO service name for the content.
#define WEBDAV_CONTENT_SERVICE_NAME "com.sun.star.ucb.WebDAVContent"

//=========================================================================

class ContentProvider;
class ContentProperties;
class CachableContentProperties;

class Content : public ::ucbhelper::ContentImplHelper,
                public com::sun::star::ucb::XContentCreator
{
    enum ResourceType
    {
        UNKNOWN,
        FTP,
        NON_DAV,
        DAV
    };

    std::auto_ptr< DAVResourceAccess > m_xResAccess;
    std::auto_ptr< CachableContentProperties >
                      m_xCachedProps; // locally cached props
    rtl::OUString     m_aEscapedTitle;
    ResourceType      m_eResourceType;
    ContentProvider*  m_pProvider; // No need for a ref, base class holds object
    bool              m_bTransient;
    bool              m_bCollection;
    bool              m_bDidGetOrHead;
    std::vector< rtl::OUString > m_aFailedPropNames;

private:
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual ::rtl::OUString getParentURL();

    sal_Bool isFolder( const ::com::sun::star::uno::Reference<
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

    sal_Bool
    exchangeIdentity( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const rtl::OUString
    getBaseURI( const std::auto_ptr< DAVResourceAccess > & rResAccess );
    SAL_WNODEPRECATED_DECLARATIONS_POP

    const ResourceType &
    getResourceType( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::uno::Exception );

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ResourceType &
    getResourceType( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::ucb::XCommandEnvironment >& xEnv,
                     const std::auto_ptr< DAVResourceAccess > & rResAccess )
        throw ( ::com::sun::star::uno::Exception );
    SAL_WNODEPRECATED_DECLARATIONS_POP

    // Command "open"
    com::sun::star::uno::Any open(
                const com::sun::star::ucb::OpenCommandArgument3 & rArg,
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
                 sal_Bool bReplaceExisting,
                 const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

    // Command "transfer"
    void transfer( const ::com::sun::star::ucb::TransferInfo & rArgs,
                   const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

    // Command "delete"
    void destroy( sal_Bool bDeletePhysical )
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
                                                sal_Bool bWrite );
    void cancelCommandExecution(
                    const DAVException & e,
                    const ::com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment > & xEnv,
                    sal_Bool bWrite = sal_False )
        throw( ::com::sun::star::uno::Exception );

    static bool shouldAccessNetworkAfterException( const DAVException & e );

    bool supportsExclusiveWriteLock(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& Environment );

public:
    Content( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const ::com::sun::star::uno::Reference<
                 ::com::sun::star::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory )
        throw ( ::com::sun::star::ucb::ContentCreationException );
    Content( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const ::com::sun::star::uno::Reference<
                 ::com::sun::star::ucb::XContentIdentifier >& Identifier,
             rtl::Reference< DAVSessionFactory > const & rSessionFactory,
             sal_Bool isCollection )
        throw ( ::com::sun::star::ucb::ContentCreationException );
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

    // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const rtl::OUString& Name,
                 sal_Int16 Attributes,
                 const com::sun::star::uno::Any& DefaultValue )
        throw( com::sun::star::beans::PropertyExistException,
               com::sun::star::beans::IllegalTypeException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    removeProperty( const rtl::OUString& Name )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::NotRemoveableException,
               com::sun::star::uno::RuntimeException );

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

    DAVResourceAccess & getResourceAccess() { return *m_xResAccess; }

    // Called from resultset data supplier.
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                           ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       const rtl::Reference<
                           ::ucbhelper::ContentProviderImplHelper >& rProvider,
                       const ::rtl::OUString& rContentId );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
