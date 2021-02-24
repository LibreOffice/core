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

#ifndef INCLUDED_SC_INC_LINKUNO_HXX
#define INCLUDED_SC_INC_LINKUNO_HXX

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

typedef std::vector< css::uno::Reference< css::util::XRefreshListener > > XRefreshListenerArr_Impl;

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
                            ScSheetLinkObj(ScDocShell* pDocSh, const OUString& rName);
    virtual                 ~ScSheetLinkObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XRefreshable
    virtual void SAL_CALL   refresh() override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

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
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    virtual void SAL_CALL   refresh() override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

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
    virtual OUString SAL_CALL getSourceArea() override;
    virtual void SAL_CALL   setSourceArea( const OUString& aSourceArea ) override;
    virtual css::table::CellRangeAddress SAL_CALL getDestArea() override;
    virtual void SAL_CALL setDestArea( const css::table::CellRangeAddress& aDestArea ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    virtual void SAL_CALL   insertAtPosition( const css::table::CellAddress& aDestPos,
                                                const OUString& aFileName,
                                                const OUString& aSourceArea,
                                                const OUString& aFilter,
                                                const OUString& aFilterOptions ) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
                            ScDDELinkObj(ScDocShell* pDocSh, const OUString& rA,
                                            const OUString& rT, const OUString& rI);
    virtual                 ~ScDDELinkObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XDDELink
    virtual OUString SAL_CALL getApplication() override;
    virtual OUString SAL_CALL getTopic() override;
    virtual OUString SAL_CALL getItem() override;

                            // XRefreshable
    virtual void SAL_CALL   refresh() override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XDDELinkResults
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > >
        SAL_CALL getResults(  ) override;
    virtual void SAL_CALL setResults(
        const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aResults ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XDDELinks
    virtual css::uno::Reference< css::sheet::XDDELink > SAL_CALL addDDELink(
        const OUString& aApplication, const OUString& aTopic,
        const OUString& aItem, css::sheet::DDELinkMode nMode ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScExternalSheetCacheObj final : public cppu::WeakImplHelper< css::sheet::XExternalSheetCache >
{
public:
    explicit ScExternalSheetCacheObj(ScDocShell* pDocShell, ScExternalRefCache::TableTypeRef const & pTable, size_t nIndex);
    virtual ~ScExternalSheetCacheObj() override;

                            // XExternalSheetCache
    virtual void SAL_CALL setCellValue(
        sal_Int32 nCol, sal_Int32 nRow, const css::uno::Any& rAny) override;

    virtual css::uno::Any SAL_CALL getCellValue(sal_Int32 nCol, sal_Int32 nRow) override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getAllRows() override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getAllColumns(sal_Int32 nRow) override;

    // Attributes
    virtual sal_Int32 SAL_CALL getTokenIndex() override;

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
    virtual css::uno::Reference< css::sheet::XExternalSheetCache >
        SAL_CALL addSheetCache( const OUString& aSheetName, sal_Bool bDynamicCache ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // Attributes
    virtual sal_Int32 SAL_CALL getTokenIndex() override;

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
    virtual css::uno::Reference< css::sheet::XExternalDocLink >
        SAL_CALL addDocLink( const OUString& aDocName ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

private:
    ScExternalDocLinksObj(const ScExternalDocLinksObj&) = delete;

private:
    ScDocShell*                         mpDocShell;
    ScExternalRefManager*               mpRefMgr;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
