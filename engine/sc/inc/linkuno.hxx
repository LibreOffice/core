/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svl/lstner.hxx>
#include <svl/itemprop.hxx>
#include <com/sun/star/sheet/XDDELink.hpp>
#include <com/sun/star/sheet/XDDELinkResults.hpp>
#include <com/sun/star/sheet/XDDELinks.hpp>
#include <com/sun/star/sheet/XExternalDocLink.hpp>
#include <com/sun/star/sheet/XExternalDocLinks.hpp>
#include <com/sun/star/sheet/XExternalSheetCache.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/implbase.hxx>

#include "externalrefmgr.hxx"

#include <vector>

class ScDocShell;
class ScTableLink;

typedef std::vector< cpo::uno::Reference< css::util::XRefreshListener > > XRefreshListenerArr_Impl;

class ScSheetLinkObj final : public cppu::WeakImplHelper<
                            css::container::XNamed,
                            css::util::XRefreshable,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocShell;
    OUString                aFileName;
    XRefreshListenerArr_Impl aRefreshListeners;

    ScTableLink*            GetLink_Impl() const;
    void                    Refreshed_Impl();
    void                    ModifyRefreshDelay_Impl( sal_Int32 nRefresh );

public:
                            ScSheetLinkObj(ScDocShell* pDocSh, OUString aName);
    virtual                 ~ScSheetLinkObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNamed
    virtual OUString getName() override;
    virtual void   setName( const OUString& aName ) override;

                            // XRefreshable
    virtual void   refresh() override;
    virtual void   addRefreshListener( const cpo::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void   removeRefreshListener( const cpo::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo(  ) override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // called from get/setPropertyValue:
    OUString         getFileName() const;
    void                    setFileName(const OUString& FileName);
    OUString         getFilter() const;
    void                    setFilter(const OUString& Filter);
    OUString         getFilterOptions() const;
    void                    setFilterOptions(const OUString& FilterOptions);
    sal_Int32               getRefreshDelay() const;
    void                    setRefreshDelay(sal_Int32 nRefreshDelay);

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScSheetLinksObj final : public cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    rtl::Reference<ScSheetLinkObj> GetObjectByIndex_Impl(sal_Int32 nIndex);
    rtl::Reference<ScSheetLinkObj> GetObjectByName_Impl(const OUString& aName);

public:
                            ScSheetLinksObj(ScDocShell* pDocSh);
    virtual                 ~ScSheetLinksObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScAreaLinkObj final : public cppu::WeakImplHelper<
                            css::sheet::XAreaLink,
                            css::util::XRefreshable,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocShell;
    size_t                  nPos;
    XRefreshListenerArr_Impl aRefreshListeners;

    void    Modify_Impl( const OUString* pNewFile, const OUString* pNewFilter,
                         const OUString* pNewOptions, const OUString* pNewSource,
                         const css::table::CellRangeAddress* pNewDest );
    void    ModifyRefreshDelay_Impl( sal_Int32 nRefresh );
    void    Refreshed_Impl();

public:
                            ScAreaLinkObj(ScDocShell* pDocSh, size_t nP);
    virtual                 ~ScAreaLinkObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XRefreshable
    virtual void   refresh() override;
    virtual void   addRefreshListener( const cpo::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void   removeRefreshListener( const cpo::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo(  ) override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // called from get/setPropertyValue:
    OUString                getFileName() const;
    void                    setFileName(const OUString& FileName);
    OUString                getFilter() const;
    void                    setFilter(const OUString& Filter);
    OUString                getFilterOptions() const;
    void                    setFilterOptions(const OUString& FilterOptions);
    sal_Int32               getRefreshDelay() const;
    void                    setRefreshDelay(sal_Int32 nRefreshDelay);

                            // XAreaLink
    virtual OUString getSourceArea() override;
    virtual void   setSourceArea( const OUString& aSourceArea ) override;
    virtual css::table::CellRangeAddress getDestArea() override;
    virtual void setDestArea( const css::table::CellRangeAddress& aDestArea ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScAreaLinksObj final : public cppu::WeakImplHelper<
                            css::sheet::XAreaLinks,
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    rtl::Reference<ScAreaLinkObj> GetObjectByIndex_Impl(sal_Int32 nIndex);

public:
                            ScAreaLinksObj(ScDocShell* pDocSh);
    virtual                 ~ScAreaLinksObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XAreaLinks
    virtual void   insertAtPosition( const css::table::CellAddress& aDestPos,
                                                const OUString& aFileName,
                                                const OUString& aSourceArea,
                                                const OUString& aFilter,
                                                const OUString& aFilterOptions ) override;
    virtual void   removeByIndex( sal_Int32 nIndex ) override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

//! order of XNamed and DDELink changed to avoid "duplicate comdat" symbols

class ScDDELinkObj final : public cppu::WeakImplHelper<
                            css::sheet::XDDELink,
                            css::container::XNamed,
                            css::util::XRefreshable,
                            css::sheet::XDDELinkResults,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    OUString                aAppl;
    OUString                aTopic;
    OUString                aItem;
    XRefreshListenerArr_Impl aRefreshListeners;

    void                    Refreshed_Impl();

public:
                            ScDDELinkObj(ScDocShell* pDocSh, OUString aA,
                                            OUString aT, OUString aI);
    virtual                 ~ScDDELinkObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNamed
    virtual OUString getName() override;
    virtual void   setName( const OUString& aName ) override;

                            // XDDELink
    virtual OUString getApplication() override;
    virtual OUString getTopic() override;
    virtual OUString getItem() override;

                            // XRefreshable
    virtual void   refresh() override;
    virtual void   addRefreshListener( const cpo::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void   removeRefreshListener( const cpo::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XDDELinkResults
    virtual cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >
        getResults(  ) override;
    virtual void setResults(
        const cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >& aResults ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScDDELinksObj final : public cppu::WeakImplHelper<
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::sheet::XDDELinks,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    rtl::Reference<ScDDELinkObj> GetObjectByIndex_Impl(sal_Int32 nIndex);
    rtl::Reference<ScDDELinkObj> GetObjectByName_Impl(std::u16string_view aName);

public:
                            ScDDELinksObj(ScDocShell* pDocSh);
    virtual                 ~ScDDELinksObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XDDELinks
    virtual cpo::uno::Reference< css::sheet::XDDELink > addDDELink(
        const OUString& aApplication, const OUString& aTopic,
        const OUString& aItem, css::sheet::DDELinkMode nMode ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScExternalSheetCacheObj final : public cppu::WeakImplHelper< css::sheet::XExternalSheetCache >
{
public:
    explicit ScExternalSheetCacheObj(ScDocShell* pDocShell, ScExternalRefCache::TableTypeRef pTable, size_t nIndex);
    virtual ~ScExternalSheetCacheObj() override;

                            // XExternalSheetCache
    virtual void setCellValue(
        sal_Int32 nCol, sal_Int32 nRow, const cpo::uno::Any& rAny) override;

    virtual cpo::uno::Any getCellValue(sal_Int32 nCol, sal_Int32 nRow) override;

    virtual cpo::uno::Sequence< sal_Int32 > getAllRows() override;

    virtual cpo::uno::Sequence< sal_Int32 > getAllColumns(sal_Int32 nRow) override;

    // Attributes
    virtual sal_Int32 getTokenIndex() override;

private:
    ScExternalSheetCacheObj(const ScExternalSheetCacheObj&) = delete;

private:
    ScDocShell* mpDocShell;
    ScExternalRefCache::TableTypeRef mpTable;
    size_t mnIndex;
};

class ScExternalDocLinkObj final : public cppu::WeakImplHelper< css::sheet::XExternalDocLink >
{
public:
    ScExternalDocLinkObj(ScDocShell* pDocShell, ScExternalRefManager* pRefMgr, sal_uInt16 nFileId);
    virtual ~ScExternalDocLinkObj() override;

                            // XExternalDocLink
    virtual cpo::uno::Reference< css::sheet::XExternalSheetCache >
        addSheetCache( const OUString& aSheetName, bool bDynamicCache ) override;

                            // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 nIndex ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // Attributes
    virtual sal_Int32 getTokenIndex() override;

private:
    ScDocShell*             mpDocShell;
    ScExternalRefManager*   mpRefMgr;
    sal_uInt16              mnFileId;
};

/** This is the UNO API equivalent of ScExternalRefManager. */
class ScExternalDocLinksObj final : public cppu::WeakImplHelper< css::sheet::XExternalDocLinks >
{
public:
    ScExternalDocLinksObj(ScDocShell* pDocShell);
    virtual ~ScExternalDocLinksObj() override;

                            // XExternalDocLinks
    virtual cpo::uno::Reference< css::sheet::XExternalDocLink >
        addDocLink( const OUString& aDocName ) override;

    virtual cpo::uno::Reference<css::sheet::XExternalDocLink>
        addSpecialDocLink(const OUString& aDocName, sal_Int16 nLinkType) override;

                            // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 nIndex ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

private:
    ScExternalDocLinksObj(const ScExternalDocLinksObj&) = delete;

private:
    ScDocShell*                         mpDocShell;
    ScExternalRefManager*               mpRefMgr;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
