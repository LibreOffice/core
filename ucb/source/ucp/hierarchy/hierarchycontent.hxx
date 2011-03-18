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

#ifndef _HIERARCHYCONTENT_HXX
#define _HIERARCHYCONTENT_HXX

#include <list>
#include <rtl/ref.hxx>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include "hierarchydata.hxx"
#include "hierarchyprovider.hxx"

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    struct TransferInfo;
} } } }

namespace hierarchy_ucp
{

//=========================================================================

#define HIERARCHY_ROOT_FOLDER_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.HierarchyRootFolderContent"
#define HIERARCHY_FOLDER_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.HierarchyFolderContent"
#define HIERARCHY_LINK_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.HierarchyLinkContent"

//=========================================================================

class HierarchyContentProperties
{
public:
    HierarchyContentProperties() {};

    HierarchyContentProperties( const HierarchyEntryData::Type & rType )
    : m_aData( rType ),
      m_aContentType( rType == HierarchyEntryData::FOLDER
        ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_FOLDER_CONTENT_TYPE ))
        : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_LINK_CONTENT_TYPE )) ) {}

    HierarchyContentProperties( const HierarchyEntryData & rData )
    : m_aData( rData ),
      m_aContentType( rData.getType() == HierarchyEntryData::FOLDER
        ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_FOLDER_CONTENT_TYPE ))
        : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_LINK_CONTENT_TYPE )) ) {}

    const rtl::OUString & getName() const { return m_aData.getName(); }
    void setName( const rtl::OUString & rName ) { m_aData.setName( rName ); };

    const rtl::OUString & getTitle() const { return m_aData.getTitle(); }
    void setTitle( const rtl::OUString & rTitle )
    { m_aData.setTitle( rTitle ); };

    const rtl::OUString & getTargetURL() const
    { return m_aData.getTargetURL(); }
    void setTargetURL( const rtl::OUString & rURL )
    { m_aData.setTargetURL( rURL ); };

    const rtl::OUString & getContentType() const { return m_aContentType; }

    sal_Bool getIsFolder() const
    { return m_aData.getType() == HierarchyEntryData::FOLDER; }

    sal_Bool getIsDocument() const { return !getIsFolder(); }

    com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
    getCreatableContentsInfo() const;

    const HierarchyEntryData & getHierarchyEntryData() const { return m_aData; }

private:
    HierarchyEntryData m_aData;
    rtl::OUString m_aContentType;
};

//=========================================================================

class HierarchyContentProvider;

class HierarchyContent : public ::ucbhelper::ContentImplHelper,
                         public com::sun::star::ucb::XContentCreator
{
    enum ContentKind  { LINK, FOLDER, ROOT };
    enum ContentState { TRANSIENT,  // created via CreateNewContent,
                                        // but did not process "insert" yet
                        PERSISTENT, // processed "insert"
                        DEAD        // processed "delete"
                      };

    HierarchyContentProperties m_aProps;
    ContentKind                m_eKind;
    ContentState               m_eState;
    HierarchyContentProvider*  m_pProvider;
    bool                       m_bCheckedReadOnly;
    bool                       m_bIsReadOnly;

private:
    HierarchyContent(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const HierarchyContentProperties& rProps );
    HierarchyContent(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            HierarchyContentProvider* pProvider,
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

    static sal_Bool hasData(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );
    sal_Bool hasData(
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier )
    { return hasData( m_xSMgr, m_pProvider, Identifier ); }
    static sal_Bool loadData(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            HierarchyContentProperties& rProps );
    sal_Bool storeData();
    sal_Bool renameData( const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XContentIdentifier >& xOldId,
                         const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XContentIdentifier >& xNewId );
    sal_Bool removeData();

    void setKind( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier );

    bool isReadOnly();

    sal_Bool isFolder() const { return ( m_eKind > LINK ); }

    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentIdentifier >
    makeNewIdentifier( const rtl::OUString& rTitle );

    typedef rtl::Reference< HierarchyContent > HierarchyContentRef;
    typedef std::list< HierarchyContentRef > HierarchyContentRefList;
    void queryChildren( HierarchyContentRefList& rChildren );

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

    void insert( sal_Int32 nNameClashResolve,
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

public:
    // Create existing content. Fail, if not already exists.
    static HierarchyContent* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static HierarchyContent* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual ~HierarchyContent();

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
                       const HierarchyContentProperties& rData,
                       HierarchyContentProvider* pProvider,
                       const ::rtl::OUString& rContentId );
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYCONTENT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
