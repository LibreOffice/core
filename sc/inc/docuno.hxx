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

#ifndef SC_DOCUNO_HXX
#define SC_DOCUNO_HXX

#include "address.hxx"
#include <sfx2/sfxbasemodel.hxx>
#include <svl/lstner.hxx>
#include <svx/fmdmod.hxx>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/sheet/XGoalSeek.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/sheet/XScenarios.hpp>
#include <com/sun/star/sheet/XConsolidatable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets2.hpp>
#include <com/sun/star/sheet/XDocumentAuditing.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XCellRangesAccess.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <svl/itemprop.hxx>
#include "drwlayer.hxx"

class ScDocShell;
class ScAnnotationObj;
class ScMarkData;
class ScPrintFuncCache;
class ScPrintSelectionStatus;
class ScTableColumnObj;
class ScTableRowObj;
class ScTableSheetObj;
class ScRangeList;
class ScPrintUIOptions;
class ScSheetSaveData;

class SC_DLLPUBLIC ScModelObj : public SfxBaseModel,
                    public com::sun::star::sheet::XSpreadsheetDocument,
                    public com::sun::star::document::XActionLockable,
                    public com::sun::star::sheet::XCalculatable,
                    public com::sun::star::util::XProtectable,
                    public com::sun::star::drawing::XDrawPagesSupplier,
                    public com::sun::star::sheet::XGoalSeek,
                    public com::sun::star::sheet::XConsolidatable,
                    public com::sun::star::sheet::XDocumentAuditing,
                    public com::sun::star::style::XStyleFamiliesSupplier,
                    public com::sun::star::view::XRenderable,
                    public com::sun::star::document::XLinkTargetSupplier,
                    public com::sun::star::beans::XPropertySet,
                    public SvxFmMSFactory,  // derived from XMultiServiceFactory
                    public com::sun::star::lang::XServiceInfo,
                    public ::com::sun::star::util::XChangesNotifier
{
private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocShell;
    ScPrintFuncCache*       pPrintFuncCache;
    ScPrintUIOptions*       pPrinterOptions;
    com::sun::star::uno::Reference<com::sun::star::uno::XAggregation> xNumberAgg;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xDrawGradTab;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xDrawHatchTab;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xDrawBitmapTab;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xDrawTrGradTab;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xDrawMarkerTab;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xDrawDashTab;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xChartDataProv;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> xObjProvider;

    ::cppu::OInterfaceContainerHelper maChangesListeners;

    bool                    FillRenderMarkData( const com::sun::star::uno::Any& aSelection,
                                                const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rOptions,
                                                ScMarkData& rMark, ScPrintSelectionStatus& rStatus, String& rPagesStr ) const;
    com::sun::star::uno::Reference<com::sun::star::uno::XAggregation> GetFormatter();
    void                    HandleCalculateEvents();

    OUString           maBuildId;
protected:
    const SfxItemPropertySet&   GetPropertySet() const { return aPropSet; }

public:
                            ScModelObj(ScDocShell* pDocSh);
    virtual                 ~ScModelObj();

    //  create ScModelObj and set at pDocSh (SetBaseModel)
    static void             CreateAndSet(ScDocShell* pDocSh);

    ScDocument*             GetDocument() const;
    SfxObjectShell*         GetEmbeddedObject() const;

    void UpdateAllRowHeights();

    void                    BeforeXMLLoading();
    void                    AfterXMLLoading(sal_Bool bRet);
    ScSheetSaveData*        GetSheetSaveData();

    void                    RepaintRange( const ScRange& rRange );
    void                    RepaintRange( const ScRangeList& rRange );

    bool                    HasChangesListeners() const;

    void                    NotifyChanges( const OUString& rOperation, const ScRangeList& rRanges,
                                           const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rProperties =
                                               ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >() );

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw();
    virtual void SAL_CALL   release() throw();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XSpreadsheetDocument
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheets > SAL_CALL
                            getSheets() throw(::com::sun::star::uno::RuntimeException);

                                // XStyleFamiliesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getStyleFamilies() throw(::com::sun::star::uno::RuntimeException);

                            // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getRenderer( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   render( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getLinks() throw(::com::sun::star::uno::RuntimeException);

                            // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addActionLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeActionLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setActionLocks( sal_Int16 nLock )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL resetActionLocks() throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL   lockControllers() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL   unlockControllers() throw (::com::sun::star::uno::RuntimeException);

                                // XCalculatable
    virtual void SAL_CALL   calculate() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   calculateAll() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAutomaticCalculationEnabled()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   enableAutomaticCalculation( sal_Bool bEnabled )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XProtectable
    virtual void SAL_CALL   protect( const OUString& aPassword )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   unprotect( const OUString& aPassword )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isProtected() throw(::com::sun::star::uno::RuntimeException);

                            //  XDrawPagesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > SAL_CALL
                            getDrawPages() throw(::com::sun::star::uno::RuntimeException);

                            //  XGoalSeek
    virtual ::com::sun::star::sheet::GoalResult SAL_CALL seekGoal(
                                const ::com::sun::star::table::CellAddress& aFormulaPosition,
                                const ::com::sun::star::table::CellAddress& aVariablePosition,
                                const OUString& aGoalValue )
                                    throw(::com::sun::star::uno::RuntimeException);

                            //  XConsolidatable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XConsolidationDescriptor >
                            SAL_CALL createConsolidationDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL consolidate( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XConsolidationDescriptor >& xDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException);

                            //  XDocumentAuditing
    virtual void SAL_CALL   refreshArrows() throw(::com::sun::star::uno::RuntimeException);

                            //  XViewDataSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getViewData(  )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            createInstance( const OUString& aServiceSpecifier )
                                throw(::com::sun::star::uno::Exception,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            createInstanceWithArguments( const OUString& ServiceSpecifier,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::uno::Any >& Arguments )
                                throw(::com::sun::star::uno::Exception,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScModelObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XChangesNotifier
    virtual void SAL_CALL addChangesListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XChangesListener >& aListener )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChangesListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XChangesListener >& aListener )
                                throw (::com::sun::star::uno::RuntimeException);
};


class ScDrawPagesObj : public cppu::WeakImplHelper2<
                                com::sun::star::drawing::XDrawPages,
                                com::sun::star::lang::XServiceInfo>,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                            GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScDrawPagesObj(ScDocShell* pDocSh);
    virtual                 ~ScDrawPagesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XDrawPages
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
                            insertNewByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   remove( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XDrawPage >& xPage )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScTableSheetsObj : public cppu::WeakImplHelper5<
                                com::sun::star::sheet::XSpreadsheets2,
                                com::sun::star::sheet::XCellRangesAccess,
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::container::XIndexAccess,
                                com::sun::star::lang::XServiceInfo>,
                         public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScTableSheetObj*        GetObjectByIndex_Impl(sal_Int32 nIndex) const;
    ScTableSheetObj*        GetObjectByName_Impl(const OUString& aName) const;

public:
                            ScTableSheetsObj(ScDocShell* pDocSh);
    virtual                 ~ScTableSheetsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XSpreadsheets
    virtual void SAL_CALL   insertNewByName( const OUString& aName, sal_Int16 nPosition )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   moveByName( const OUString& aName, sal_Int16 nDestination )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   copyByName( const OUString& aName,
                                const OUString& aCopy, sal_Int16 nDestination )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSpreadsheets2
    virtual sal_Int32 SAL_CALL importSheet(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::sheet::XSpreadsheetDocument > & xDocSrc,
        const OUString& srcName,
        const sal_Int32 nDestPosition)
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::lang::IndexOutOfBoundsException,
              ::com::sun::star::uno::RuntimeException);

                            // XCellRangesAccess

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
        SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
        SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence < ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > >
        SAL_CALL getCellRangesByName( const OUString& aRange )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

                            // XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::ElementExistException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByName( const OUString& Name )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScTableColumnsObj : public cppu::WeakImplHelper5<
                                com::sun::star::table::XTableColumns,
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::container::XNameAccess,
                                com::sun::star::beans::XPropertySet,
                                com::sun::star::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;
    SCCOL                   nStartCol;
    SCCOL                   nEndCol;

    ScTableColumnObj*       GetObjectByIndex_Impl(sal_Int32 nIndex) const;
    ScTableColumnObj*       GetObjectByName_Impl(const OUString& aName) const;

public:
                            ScTableColumnsObj(ScDocShell* pDocSh, SCTAB nT,
                                                SCCOL nSC, SCCOL nEC);
    virtual                 ~ScTableColumnsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XTableColumns
    virtual void SAL_CALL   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScTableRowsObj : public cppu::WeakImplHelper4<
                                com::sun::star::table::XTableRows,
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::beans::XPropertySet,
                                com::sun::star::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;
    SCROW                   nStartRow;
    SCROW                   nEndRow;

    ScTableRowObj*          GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScTableRowsObj(ScDocShell* pDocSh, SCTAB nT,
                                                SCROW nSR, SCROW nER);
    virtual                 ~ScTableRowsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XTableRows
    virtual void SAL_CALL   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScSpreadsheetSettingsObj : public cppu::WeakImplHelper2<
                                    com::sun::star::beans::XPropertySet,
                                    com::sun::star::lang::XServiceInfo>,
                                 public SfxListener
{
private:
    ScDocShell*             pDocShell;

public:
    virtual                 ~ScSpreadsheetSettingsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScAnnotationsObj : public cppu::WeakImplHelper3<
                                com::sun::star::sheet::XSheetAnnotations,
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;           // Collection belongs to the sheet

    bool                    GetAddressByIndex_Impl( sal_Int32 nIndex, ScAddress& rPos ) const;
    ScAnnotationObj*        GetObjectByIndex_Impl( sal_Int32 nIndex ) const;

public:
                            ScAnnotationsObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScAnnotationsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XSheetAnnotations
    virtual void SAL_CALL   insertNew( const ::com::sun::star::table::CellAddress& aPosition,
                                const OUString& aText )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScScenariosObj : public cppu::WeakImplHelper4<
                                com::sun::star::sheet::XScenarios,
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::container::XIndexAccess,
                                com::sun::star::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;

    sal_Bool                    GetScenarioIndex_Impl( const OUString& rName, SCTAB& rIndex );
    ScTableSheetObj*        GetObjectByIndex_Impl(sal_Int32 nIndex);
    ScTableSheetObj*        GetObjectByName_Impl(const OUString& aName);

public:
                            ScScenariosObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScScenariosObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XScenarios
    virtual void SAL_CALL   addNewByName( const OUString& aName,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::table::CellRangeAddress >& aRanges,
                                const OUString& aComment )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
