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

#ifndef INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYCONTENT_HXX
#define INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYCONTENT_HXX

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



class HierarchyContentProperties
{
public:
    HierarchyContentProperties() {};

    explicit HierarchyContentProperties( const HierarchyEntryData::Type & rType )
    : m_aData( rType ),
      m_aContentType( rType == HierarchyEntryData::FOLDER
        ? OUString( HIERARCHY_FOLDER_CONTENT_TYPE )
        : OUString( HIERARCHY_LINK_CONTENT_TYPE ) ) {}

    explicit HierarchyContentProperties( const HierarchyEntryData & rData )
    : m_aData( rData ),
      m_aContentType( rData.getType() == HierarchyEntryData::FOLDER
        ? OUString( HIERARCHY_FOLDER_CONTENT_TYPE )
        : OUString( HIERARCHY_LINK_CONTENT_TYPE ) ) {}

    const OUString & getName() const { return m_aData.getName(); }
    void setName( const OUString & rName ) { m_aData.setName( rName ); };

    const OUString & getTitle() const { return m_aData.getTitle(); }
    void setTitle( const OUString & rTitle )
    { m_aData.setTitle( rTitle ); };

    const OUString & getTargetURL() const
    { return m_aData.getTargetURL(); }
    void setTargetURL( const OUString & rURL )
    { m_aData.setTargetURL( rURL ); };

    const OUString & getContentType() const { return m_aContentType; }

    bool getIsFolder() const
    { return m_aData.getType() == HierarchyEntryData::FOLDER; }

    bool getIsDocument() const { return !getIsFolder(); }

    com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
    getCreatableContentsInfo() const;

    const HierarchyEntryData & getHierarchyEntryData() const { return m_aData; }

private:
    HierarchyEntryData m_aData;
    OUString m_aContentType;
};



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
                com::sun::star::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const HierarchyContentProperties& rProps );
    HierarchyContent(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv ) override;
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    static bool hasData(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );
    bool hasData(
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier )
    { return hasData( m_xContext, m_pProvider, Identifier ); }
    static bool loadData(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            HierarchyContentProperties& rProps );
    bool storeData();
    bool renameData( const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XContentIdentifier >& xOldId,
                         const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XContentIdentifier >& xNewId );
    bool removeData();

    void setKind( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier );

    bool isReadOnly();

    bool isFolder() const { return ( m_eKind > LINK ); }

    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentIdentifier >
    makeNewIdentifier( const OUString& rTitle );

    typedef rtl::Reference< HierarchyContent > HierarchyContentRef;
    typedef std::list< HierarchyContentRef > HierarchyContentRefList;
    void queryChildren( HierarchyContentRefList& rChildren );

    bool exchangeIdentity(
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
        throw( ::com::sun::star::uno::Exception, std::exception );

    void insert( sal_Int32 nNameClashResolve,
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

public:
    // Create existing content. Fail, if not already exists.
    static HierarchyContent* create(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static HierarchyContent* create(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual ~HierarchyContent();

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
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    getIdentifier()
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


    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XComponentContext >& rxContext,
                       const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::Property >& rProperties,
                       const HierarchyContentProperties& rData,
                       HierarchyContentProvider* pProvider,
                       const OUString& rContentId );
};

} // namespace hierarchy_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYCONTENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
