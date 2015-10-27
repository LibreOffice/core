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

class ScSheetLinkObj : public cppu::WeakImplHelper<
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
    virtual                 ~ScSheetLinkObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XRefreshable
    virtual void SAL_CALL   refresh()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // aus get/setPropertyValue gerufen:
    OUString         getFileName() const;
    void                    setFileName(const OUString& FileName);
    OUString         getFilter() const;
    void                    setFilter(const OUString& Filter);
    OUString         getFilterOptions() const;
    void                    setFilterOptions(const OUString& FilterOptions);
    sal_Int32               getRefreshDelay() const;
    void                    setRefreshDelay(sal_Int32 nRefreshDelay);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScSheetLinksObj : public cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScSheetLinkObj*         GetObjectByIndex_Impl(sal_Int32 nIndex);
    ScSheetLinkObj*         GetObjectByName_Impl(const OUString& aName);

public:
                            ScSheetLinksObj(ScDocShell* pDocSh);
    virtual                 ~ScSheetLinksObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScAreaLinkObj : public cppu::WeakImplHelper<
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
    virtual                 ~ScAreaLinkObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XRefreshable
    virtual void SAL_CALL   refresh()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

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
    virtual OUString SAL_CALL getSourceArea() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setSourceArea( const OUString& aSourceArea )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::table::CellRangeAddress SAL_CALL getDestArea()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDestArea( const css::table::CellRangeAddress& aDestArea )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScAreaLinksObj : public cppu::WeakImplHelper<
                            css::sheet::XAreaLinks,
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScAreaLinkObj*          GetObjectByIndex_Impl(sal_Int32 nIndex);

public:
                            ScAreaLinksObj(ScDocShell* pDocSh);
    virtual                 ~ScAreaLinksObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XAreaLinks
    virtual void SAL_CALL   insertAtPosition( const css::table::CellAddress& aDestPos,
                                                const OUString& aFileName,
                                                const OUString& aSourceArea,
                                                const OUString& aFilter,
                                                const OUString& aFilterOptions )
                                            throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
                                            throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

//! order of XNamed and DDELink changed to avoid "duplicate comdat" symbols

class ScDDELinkObj : public cppu::WeakImplHelper<
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
    virtual                 ~ScDDELinkObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XDDELink
    virtual OUString SAL_CALL getApplication() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTopic() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getItem() throw(css::uno::RuntimeException, std::exception) override;

                            // XRefreshable
    virtual void SAL_CALL   refresh() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XDDELinkResults
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > >
        SAL_CALL getResults(  )
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL setResults(
        const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aResults )
        throw (css::uno::RuntimeException,
               std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScDDELinksObj : public cppu::WeakImplHelper<
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::sheet::XDDELinks,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScDDELinkObj*           GetObjectByIndex_Impl(sal_Int32 nIndex);
    ScDDELinkObj*           GetObjectByName_Impl(const OUString& aName);

public:
                            ScDDELinksObj(ScDocShell* pDocSh);
    virtual                 ~ScDDELinksObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XDDELinks
    virtual css::uno::Reference< css::sheet::XDDELink > SAL_CALL addDDELink(
        const OUString& aApplication, const OUString& aTopic,
        const OUString& aItem, css::sheet::DDELinkMode nMode )
        throw (css::uno::RuntimeException,
               std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScExternalSheetCacheObj : public cppu::WeakImplHelper< css::sheet::XExternalSheetCache >
{
public:
    explicit ScExternalSheetCacheObj(ScDocShell* pDocShell, ScExternalRefCache::TableTypeRef pTable, size_t nIndex);
    virtual ~ScExternalSheetCacheObj();

                            // XExternalSheetCache
    virtual void SAL_CALL setCellValue(
        sal_Int32 nCol, sal_Int32 nRow, const css::uno::Any& rAny)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getCellValue(sal_Int32 nCol, sal_Int32 nRow)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getAllRows()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getAllColumns(sal_Int32 nRow)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // Attributes
    virtual sal_Int32 SAL_CALL getTokenIndex()
            throw (css::uno::RuntimeException, std::exception) override;

private:
    ScExternalSheetCacheObj(const ScExternalSheetCacheObj&) = delete;

private:
    ScDocShell* mpDocShell;
    ScExternalRefCache::TableTypeRef mpTable;
    size_t mnIndex;
};

class ScExternalDocLinkObj : public cppu::WeakImplHelper< css::sheet::XExternalDocLink >
{
public:
    ScExternalDocLinkObj(ScDocShell* pDocShell, ScExternalRefManager* pRefMgr, sal_uInt16 nFileId);
    virtual ~ScExternalDocLinkObj();

                            // XExternalDocLink
    virtual css::uno::Reference< css::sheet::XExternalSheetCache >
        SAL_CALL addSheetCache( const OUString& aSheetName, sal_Bool bDynamicCache )
            throw (css::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

    // Attributes
    virtual sal_Int32 SAL_CALL getTokenIndex()
            throw (css::uno::RuntimeException, std::exception) override;

private:
    ScDocShell* mpDocShell;
    ScExternalRefManager*   mpRefMgr;
    sal_uInt16              mnFileId;
};

/** This is the UNO API equivalent of ScExternalRefManager. */
class ScExternalDocLinksObj : public cppu::WeakImplHelper< css::sheet::XExternalDocLinks >
{
public:
    ScExternalDocLinksObj(ScDocShell* pDocShell);
    virtual ~ScExternalDocLinksObj();

                            // XExternalDocLinks
    virtual css::uno::Reference< css::sheet::XExternalDocLink >
        SAL_CALL addDocLink( const OUString& aDocName )
            throw (css::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

private:
    ScExternalDocLinksObj(const ScExternalDocLinksObj&) = delete;

private:
    ScDocShell*                         mpDocShell;
    ScExternalRefManager*               mpRefMgr;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
