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

#include <vector>
#include <rtl/ref.hxx>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include "hierarchydata.hxx"
#include "hierarchyprovider.hxx"

namespace com::sun::star::beans {
    struct Property;
    struct PropertyValue;
}

namespace com::sun::star::sdbc {
    class XRow;
}

namespace com::sun::star::ucb {
    struct TransferInfo;
}

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

    css::uno::Sequence< css::ucb::ContentInfo >
    getCreatableContentsInfo() const;

    const HierarchyEntryData & getHierarchyEntryData() const { return m_aData; }

private:
    HierarchyEntryData m_aData;
    OUString m_aContentType;
};


class HierarchyContentProvider;

class HierarchyContent : public ::ucbhelper::ContentImplHelper,
                         public css::ucb::XContentCreator
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
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
            const HierarchyContentProperties& rProps );
    HierarchyContent(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
            const css::ucb::ContentInfo& Info );

    virtual css::uno::Sequence< css::beans::Property >
    getProperties( const css::uno::Reference<css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual css::uno::Sequence< css::ucb::CommandInfo >
    getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    static bool hasData(
            const css::uno::Reference<  css::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier );
    bool hasData(
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
    { return hasData( m_xContext, m_pProvider, Identifier ); }
    static bool loadData(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
            HierarchyContentProperties& rProps );
    bool storeData();
    void renameData( const css::uno::Reference< css::ucb::XContentIdentifier >& xOldId,
                     const css::uno::Reference< css::ucb::XContentIdentifier >& xNewId );
    bool removeData();

    void setKind( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier );

    bool isReadOnly();

    bool isFolder() const { return ( m_eKind > LINK ); }

    css::uno::Reference< css::ucb::XContentIdentifier >
    makeNewIdentifier( const OUString& rTitle );

    typedef rtl::Reference< HierarchyContent > HierarchyContentRef;
    typedef std::vector< HierarchyContentRef > HierarchyContentRefVector;
    void queryChildren( HierarchyContentRefVector& rChildren );

    bool exchangeIdentity(
                const css::uno::Reference< css::ucb::XContentIdentifier >& xNewId );

    css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Sequence< css::beans::Property >& rProperties );
    /// @throws css::uno::Exception
    css::uno::Sequence< css::uno::Any >
    setPropertyValues(
            const css::uno::Sequence< css::beans::PropertyValue >& rValues,
            const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void insert( sal_Int32 nNameClashResolve,
                 const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void destroy( bool bDeletePhysical,
                  const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void transfer( const css::ucb::TransferInfo& rInfo,
                   const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

public:
    // Create existing content. Fail, if not already exists.
    static rtl::Reference<HierarchyContent> create(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const css::uno::Reference<
                css::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static rtl::Reference<HierarchyContent> create(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            HierarchyContentProvider* pProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
            const css::ucb::ContentInfo& Info );

    virtual ~HierarchyContent() override;

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
                       const HierarchyContentProperties& rData,
                       HierarchyContentProvider* pProvider,
                       const OUString& rContentId );
};

} // namespace hierarchy_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
