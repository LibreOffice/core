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

#ifndef INCLUDED_SC_INC_DOCUNO_HXX
#define INCLUDED_SC_INC_DOCUNO_HXX

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
#include <com/sun/star/sheet/opencl/XOpenCLSelection.hpp>
#include <com/sun/star/sheet/opencl/OpenCLPlatform.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <svl/itemprop.hxx>
#include <vcl/event.hxx>
#include <vcl/ITiledRenderable.hxx>
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
                    public vcl::ITiledRenderable,
                    public css::sheet::XSpreadsheetDocument,
                    public css::document::XActionLockable,
                    public css::sheet::XCalculatable,
                    public css::util::XProtectable,
                    public css::drawing::XDrawPagesSupplier,
                    public css::sheet::XGoalSeek,
                    public css::sheet::XConsolidatable,
                    public css::sheet::XDocumentAuditing,
                    public css::style::XStyleFamiliesSupplier,
                    public css::view::XRenderable,
                    public css::document::XLinkTargetSupplier,
                    public css::beans::XPropertySet,
                    public SvxFmMSFactory,  ///< derived from XMultiServiceFactory
                    public css::lang::XServiceInfo,
                    public css::util::XChangesNotifier,
                    public css::sheet::opencl::XOpenCLSelection
{
private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocShell;
    ScPrintFuncCache*       pPrintFuncCache;
    ScPrintUIOptions*       pPrinterOptions;
    css::uno::Reference<css::uno::XAggregation> xNumberAgg;
    css::uno::Reference<css::uno::XInterface> xDrawGradTab;
    css::uno::Reference<css::uno::XInterface> xDrawHatchTab;
    css::uno::Reference<css::uno::XInterface> xDrawBitmapTab;
    css::uno::Reference<css::uno::XInterface> xDrawTrGradTab;
    css::uno::Reference<css::uno::XInterface> xDrawMarkerTab;
    css::uno::Reference<css::uno::XInterface> xDrawDashTab;
    css::uno::Reference<css::uno::XInterface> xChartDataProv;
    css::uno::Reference<css::uno::XInterface> xObjProvider;

    ::comphelper::OInterfaceContainerHelper2 maChangesListeners;

    bool                    FillRenderMarkData( const css::uno::Any& aSelection,
                                                const css::uno::Sequence< css::beans::PropertyValue >& rOptions,
                                                ScMarkData& rMark, ScPrintSelectionStatus& rStatus, OUString& rPagesStr ) const;
    css::uno::Reference<css::uno::XAggregation> GetFormatter();
    void                    HandleCalculateEvents();

    css::uno::Reference<css::uno::XInterface> create(
        OUString const & aServiceSpecifier,
        css::uno::Sequence<css::uno::Any> const * arguments);

    OUString           maBuildId;
protected:
    const SfxItemPropertySet&   GetPropertySet() const { return aPropSet; }

public:
                            ScModelObj(ScDocShell* pDocSh);
    virtual                 ~ScModelObj();

    /// create ScModelObj and set at pDocSh (SetBaseModel)
    static void             CreateAndSet(ScDocShell* pDocSh);

    ScDocument*             GetDocument() const;
    SfxObjectShell*         GetEmbeddedObject() const;

    void UpdateAllRowHeights();

    void                    BeforeXMLLoading();
    void                    AfterXMLLoading();
    ScSheetSaveData*        GetSheetSaveData();

    void                    RepaintRange( const ScRange& rRange );
    void                    RepaintRange( const ScRangeList& rRange );

    bool                    HasChangesListeners() const;

    void                    NotifyChanges( const OUString& rOperation, const ScRangeList& rRanges,
                                           const css::uno::Sequence< css::beans::PropertyValue >& rProperties =
                                               css::uno::Sequence< css::beans::PropertyValue >() );

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XSpreadsheetDocument
    virtual css::uno::Reference< css::sheet::XSpreadsheets > SAL_CALL
                            getSheets() throw(css::uno::RuntimeException, std::exception) override;

                            /// XStyleFamiliesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getStyleFamilies() throw(css::uno::RuntimeException, std::exception) override;

                            /// XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const css::uno::Any& aSelection,
                                    const css::uno::Sequence< css::beans::PropertyValue >& xOptions )
                                throw (css::lang::IllegalArgumentException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getRenderer( sal_Int32 nRenderer, const css::uno::Any& aSelection,
                                    const css::uno::Sequence< css::beans::PropertyValue >& xOptions )
                                throw (css::lang::IllegalArgumentException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   render( sal_Int32 nRenderer, const css::uno::Any& aSelection,
                                    const css::uno::Sequence< css::beans::PropertyValue >& xOptions )
                                throw (css::lang::IllegalArgumentException,
                                       css::uno::RuntimeException,
                                       std::exception) override;

                            /// XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getLinks() throw(css::uno::RuntimeException, std::exception) override;

                            /// XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addActionLock() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeActionLock() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setActionLocks( sal_Int16 nLock )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL resetActionLocks() throw(css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL   lockControllers() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL   unlockControllers() throw (css::uno::RuntimeException, std::exception) override;

                            /// XCalculatable
    virtual void SAL_CALL   calculate() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   calculateAll() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAutomaticCalculationEnabled()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   enableAutomaticCalculation( sal_Bool bEnabled )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XProtectable
    virtual void SAL_CALL   protect( const OUString& aPassword )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   unprotect( const OUString& aPassword )
                                throw(css::lang::IllegalArgumentException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isProtected() throw(css::uno::RuntimeException, std::exception) override;

                            /// XDrawPagesSupplier
    virtual css::uno::Reference< css::drawing::XDrawPages > SAL_CALL
                            getDrawPages() throw(css::uno::RuntimeException, std::exception) override;

                            /// XGoalSeek
    virtual css::sheet::GoalResult SAL_CALL seekGoal(
                                const css::table::CellAddress& aFormulaPosition,
                                const css::table::CellAddress& aVariablePosition,
                                const OUString& aGoalValue )
                                    throw (css::uno::RuntimeException,
                                           std::exception) override;

                            /// XConsolidatable
    virtual css::uno::Reference< css::sheet::XConsolidationDescriptor >
                            SAL_CALL createConsolidationDescriptor( sal_Bool bEmpty )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL consolidate( const css::uno::Reference< css::sheet::XConsolidationDescriptor >& xDescriptor )
                                    throw (css::uno::RuntimeException,
                                          std::exception) override;

                            /// XDocumentAuditing
    virtual void SAL_CALL   refreshArrows() throw(css::uno::RuntimeException, std::exception) override;

                            /// XViewDataSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getViewData(  )
                                throw (css::uno::RuntimeException, std::exception) override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException,
                                    std::exception) override;
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

                            /// XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
                            createInstance( const OUString& aServiceSpecifier )
                                throw(css::uno::Exception,
                                        css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
                            createInstanceWithArguments( const OUString& ServiceSpecifier,
                                const css::uno::Sequence< css::uno::Any >& Arguments )
                                throw(css::uno::Exception,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScModelObj* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);

                            /// XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XChangesNotifier
    virtual void SAL_CALL addChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener )
                                throw (css::uno::RuntimeException, std::exception) override;

    // XOpenCLSelection
    virtual sal_Bool SAL_CALL isOpenCLEnabled()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL enableOpenCL(sal_Bool bEnable)
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL enableAutomaticDeviceSelection(sal_Bool bForce)
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL disableAutomaticDeviceSelection()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL selectOpenCLDevice( sal_Int32 platform, sal_Int32 device )
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getPlatformID()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getDeviceID()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::sheet::opencl::OpenCLPlatform >
        SAL_CALL getOpenCLPlatforms()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL enableOpcodeSubsetTest()
                                throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL disableOpcodeSubsetTest()
                                throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isOpcodeSubsetTested()
                                throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setFormulaCellNumberLimit( sal_Int32 number )
                                throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getFormulaCellNumberLimit()
                                throw (css::uno::RuntimeException, std::exception) override;

    /// @see vcl::ITiledRenderable::paintTile().
    virtual void paintTile( VirtualDevice& rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            long nTileWidth,
                            long nTileHeight ) override;

    /// @see vcl::ITiledRenderable::getDocumentSize().
    virtual Size getDocumentSize() override;

    /// @see vcl::ITiledRenderable::setPart().
    virtual void setPart(int nPart) override;

    /// @see vcl::ITiledRenderable::getPart().
    virtual int getPart() override;

    /// @see vcl::ITiledRenderable::getParts().
    virtual int getParts() override;

    /// @see vcl::ITiledRenderable::getPartName().
    virtual OUString getPartName(int nPart) override;

    /// @see vcl::ITiledRenderable::getPartHash().
    virtual OUString getPartHash( int nPart ) override;

    /// @see vcl::ITiledRenderable::initializeForTiledRendering().
    virtual void initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& rArguments) override;

    /// @see vcl::ITiledRenderable::postKeyEvent().
    virtual void postKeyEvent(int nType, int nCharCode, int nKeyCode) override;

    /// @see vcl::ITiledRenderable::postMouseEvent().
    virtual void postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier) override;

    /// @see vcl::ITiledRenderable::setTextSelection().
    virtual void setTextSelection(int nType, int nX, int nY) override;

    /// @see vcl::ITiledRenderable::getTextSelection().
    virtual OString getTextSelection(const char* pMimeType, OString& rUsedMimeType) override;

    /// @see vcl::ITiledRenderable::setGraphicSelection().
    virtual void setGraphicSelection(int nType, int nX, int nY) override;

    /// @see lok::Document::resetSelection().
    virtual void resetSelection() override;

    /// @see vcl::ITiledRenderable::setClipboard().
    virtual void setClipboard(const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) override;

    /// @see vcl::ITiledRenderable::isMimeTypeSupported().
    virtual bool isMimeTypeSupported() override;

    /// @see vcl::ITiledRenderable::setClientZoom().
    virtual void setClientZoom(int nTilePixelWidth, int nTilePixelHeight, int nTileTwipWidth, int nTileTwipHeight) override;

    /// @see vcl::ITiledRenderable::getRowColumnHeaders().
    virtual OUString getRowColumnHeaders(const Rectangle& rRectangle) override;

    /// @see vcl::ITiledRenderable::getCellCursor().
    virtual OString getCellCursor( int nOutputWidth,
                                   int nOutputHeight,
                                   long nTileWidth,
                                   long nTileHeight ) override;

    /// @see vcl::ITiledRenderable::getPointer().
    virtual Pointer getPointer() override;
};

class ScDrawPagesObj : public cppu::WeakImplHelper<
                                css::drawing::XDrawPages,
                                css::lang::XServiceInfo>,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

css::uno::Reference< css::drawing::XDrawPage >
                            GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScDrawPagesObj(ScDocShell* pDocSh);
    virtual                 ~ScDrawPagesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDrawPages
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL
                            insertNewByIndex( sal_Int32 nIndex )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   remove( const css::uno::Reference< css::drawing::XDrawPage >& xPage )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScTableSheetsObj : public cppu::WeakImplHelper<
                                css::sheet::XSpreadsheets2,
                                css::sheet::XCellRangesAccess,
                                css::container::XEnumerationAccess,
                                css::container::XIndexAccess,
                                css::lang::XServiceInfo>,
                         public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScTableSheetObj*        GetObjectByIndex_Impl(sal_Int32 nIndex) const;
    ScTableSheetObj*        GetObjectByName_Impl(const OUString& aName) const;

public:
                            ScTableSheetsObj(ScDocShell* pDocSh);
    virtual                 ~ScTableSheetsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XSpreadsheets
    virtual void SAL_CALL   insertNewByName( const OUString& aName, sal_Int16 nPosition )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   moveByName( const OUString& aName, sal_Int16 nDestination )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   copyByName( const OUString& aName,
                                const OUString& aCopy, sal_Int16 nDestination )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;

                            /// XSpreadsheets2
    virtual sal_Int32 SAL_CALL importSheet(
        const css::uno::Reference< css::sheet::XSpreadsheetDocument > & xDocSrc,
        const OUString& srcName,
        sal_Int32 nDestPosition)
        throw(css::lang::IllegalArgumentException,
              css::lang::IndexOutOfBoundsException,
              css::uno::RuntimeException, std::exception) override;

                            /// XCellRangesAccess

    virtual css::uno::Reference< css::table::XCell >
        SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::table::XCellRange >
        SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence < css::uno::Reference< css::table::XCellRange > >
        SAL_CALL getCellRangesByName( const OUString& aRange )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

                            /// XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                    throw(css::lang::IllegalArgumentException,
                                        css::container::ElementExistException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByName( const OUString& Name )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            /// XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                    throw(css::lang::IllegalArgumentException,
                                        css::container::NoSuchElementException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            /// XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScTableColumnsObj : public cppu::WeakImplHelper<
                                css::table::XTableColumns,
                                css::container::XEnumerationAccess,
                                css::container::XNameAccess,
                                css::beans::XPropertySet,
                                css::lang::XServiceInfo>,
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XTableColumns
    virtual void SAL_CALL   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
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
                                    css::uno::RuntimeException,
                                    std::exception) override;
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

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScTableRowsObj : public cppu::WeakImplHelper<
                                css::table::XTableRows,
                                css::container::XEnumerationAccess,
                                css::beans::XPropertySet,
                                css::lang::XServiceInfo>,
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XTableRows
    virtual void SAL_CALL   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
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
                                    css::uno::RuntimeException,
                                    std::exception) override;
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

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScSpreadsheetSettingsObj : public cppu::WeakImplHelper<
                                    css::beans::XPropertySet,
                                    css::lang::XServiceInfo>,
                                 public SfxListener
{
private:
    ScDocShell*             pDocShell;

public:
    virtual                 ~ScSpreadsheetSettingsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
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

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScAnnotationsObj : public cppu::WeakImplHelper<
                                css::sheet::XSheetAnnotations,
                                css::container::XEnumerationAccess,
                                css::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;           ///< Collection belongs to the sheet

    bool                    GetAddressByIndex_Impl( sal_Int32 nIndex, ScAddress& rPos ) const;
    ScAnnotationObj*        GetObjectByIndex_Impl( sal_Int32 nIndex ) const;

public:
                            ScAnnotationsObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScAnnotationsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XSheetAnnotations
    virtual void SAL_CALL   insertNew( const css::table::CellAddress& aPosition,
                                const OUString& aText )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScScenariosObj : public cppu::WeakImplHelper<
                                css::sheet::XScenarios,
                                css::container::XEnumerationAccess,
                                css::container::XIndexAccess,
                                css::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;

    bool                    GetScenarioIndex_Impl( const OUString& rName, SCTAB& rIndex );
    ScTableSheetObj*        GetObjectByIndex_Impl(sal_Int32 nIndex);
    ScTableSheetObj*        GetObjectByName_Impl(const OUString& aName);

public:
                            ScScenariosObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScScenariosObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XScenarios
    virtual void SAL_CALL   addNewByName( const OUString& aName,
                                const css::uno::Sequence< css::table::CellRangeAddress >& aRanges,
                                const OUString& aComment )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
