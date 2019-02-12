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
#include <com/sun/star/chart2/XDataProviderAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XCellRangesAccess.hpp>
#include <com/sun/star/sheet/opencl/XOpenCLSelection.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <svl/itemprop.hxx>
#include <vcl/ITiledRenderable.hxx>

namespace com { namespace sun { namespace star { namespace chart2 { namespace data { class XDataProvider; } } } } }
namespace com { namespace sun { namespace star { namespace sheet { namespace opencl { struct OpenCLPlatform; } } } } }

class ScDocShell;
class ScAnnotationObj;
class ScMarkData;
class ScPrintFuncCache;
struct ScPrintState;
class ScPrintSelectionStatus;
class ScTableColumnObj;
class ScTableRowObj;
class ScTableSheetObj;
class ScRangeList;
class ScPrintUIOptions;
class ScSheetSaveData;
struct ScFormatSaveData;

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
                    public css::chart2::XDataProviderAccess,
                    public css::sheet::opencl::XOpenCLSelection
{
private:
    SfxItemPropertySet const aPropSet;
    ScDocShell*             pDocShell;
    std::unique_ptr<ScPrintFuncCache> pPrintFuncCache;
    std::unique_ptr<ScPrintUIOptions> pPrinterOptions;
    std::unique_ptr<ScPrintState> m_pPrintState;
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
                                                ScMarkData& rMark, ScPrintSelectionStatus& rStatus, OUString& rPagesStr,
                                                bool& rbRenderToGraphic ) const;
    css::uno::Reference<css::uno::XAggregation> const & GetFormatter();
    void                    HandleCalculateEvents();

    css::uno::Reference<css::uno::XInterface> create(
        OUString const & aServiceSpecifier,
        css::uno::Sequence<css::uno::Any> const * arguments);

    static bool             IsOnEvenPage( sal_Int32 nPage ) { return nPage % 2 == 0; };

    OUString                maBuildId;
    std::vector<sal_Int32>  maValidPages;
protected:
    const SfxItemPropertySet&   GetPropertySet() const { return aPropSet; }

    /** abstract SdrModel provider */
    virtual SdrModel& getSdrModelFromUnoModel() const override;

public:
                            ScModelObj(ScDocShell* pDocSh);
    virtual                 ~ScModelObj() override;

    /// create ScModelObj and set at pDocSh (SetBaseModel)
    static void             CreateAndSet(ScDocShell* pDocSh);

    ScDocument*             GetDocument() const;
    SfxObjectShell*         GetEmbeddedObject() const;

    void UpdateAllRowHeights();

    void                    BeforeXMLLoading();
    void                    AfterXMLLoading();
    ScSheetSaveData*        GetSheetSaveData();
    ScFormatSaveData*       GetFormatSaveData();

    void                    RepaintRange( const ScRange& rRange );
    void                    RepaintRange( const ScRangeList& rRange );

    bool                    HasChangesListeners() const;

    void                    NotifyChanges( const OUString& rOperation, const ScRangeList& rRanges,
                                           const css::uno::Sequence< css::beans::PropertyValue >& rProperties );

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XSpreadsheetDocument
    virtual css::uno::Reference< css::sheet::XSpreadsheets > SAL_CALL
                            getSheets() override;

                            /// XDataProviderAccess
    virtual ::css::uno::Reference< css::chart2::data::XDataProvider > SAL_CALL
                            createDataProvider() override;

                            /// XStyleFamiliesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getStyleFamilies() override;

                            /// XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const css::uno::Any& aSelection,
                                    const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getRenderer( sal_Int32 nRenderer, const css::uno::Any& aSelection,
                                    const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual void SAL_CALL   render( sal_Int32 nRenderer, const css::uno::Any& aSelection,
                                    const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;

                            /// XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getLinks() override;

                            /// XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() override;
    virtual void SAL_CALL   addActionLock() override;
    virtual void SAL_CALL   removeActionLock() override;
    virtual void SAL_CALL   setActionLocks( sal_Int16 nLock ) override;
    virtual sal_Int16 SAL_CALL resetActionLocks() override;

        virtual void SAL_CALL   lockControllers() override;
        virtual void SAL_CALL   unlockControllers() override;

                            /// XCalculatable
    virtual void SAL_CALL   calculate() override;
    virtual void SAL_CALL   calculateAll() override;
    virtual sal_Bool SAL_CALL isAutomaticCalculationEnabled() override;
    virtual void SAL_CALL   enableAutomaticCalculation( sal_Bool bEnabled ) override;

                            /// XProtectable
    virtual void SAL_CALL   protect( const OUString& aPassword ) override;
    virtual void SAL_CALL   unprotect( const OUString& aPassword ) override;
    virtual sal_Bool SAL_CALL isProtected() override;

                            /// XDrawPagesSupplier
    virtual css::uno::Reference< css::drawing::XDrawPages > SAL_CALL
                            getDrawPages() override;

                            /// XGoalSeek
    virtual css::sheet::GoalResult SAL_CALL seekGoal(
                                const css::table::CellAddress& aFormulaPosition,
                                const css::table::CellAddress& aVariablePosition,
                                const OUString& aGoalValue ) override;

                            /// XConsolidatable
    virtual css::uno::Reference< css::sheet::XConsolidationDescriptor >
                            SAL_CALL createConsolidationDescriptor( sal_Bool bEmpty ) override;
    virtual void SAL_CALL consolidate( const css::uno::Reference< css::sheet::XConsolidationDescriptor >& xDescriptor ) override;

                            /// XDocumentAuditing
    virtual void SAL_CALL   refreshArrows() override;

                            /// XViewDataSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getViewData(  ) override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
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

                            /// XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
                            createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
                            createInstanceWithArguments( const OUString& ServiceSpecifier,
                                const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

                            /// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScModelObj* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);

                            /// XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

                            /// XChangesNotifier
    virtual void SAL_CALL addChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener ) override;
    virtual void SAL_CALL removeChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener ) override;

    // XOpenCLSelection
    virtual sal_Bool SAL_CALL isOpenCLEnabled() override;

    virtual void SAL_CALL enableOpenCL(sal_Bool bEnable) override;

    virtual void SAL_CALL enableAutomaticDeviceSelection(sal_Bool bForce) override;

    virtual void SAL_CALL disableAutomaticDeviceSelection() override;

    virtual void SAL_CALL selectOpenCLDevice( sal_Int32 platform, sal_Int32 device ) override;

    virtual sal_Int32 SAL_CALL getPlatformID() override;

    virtual sal_Int32 SAL_CALL getDeviceID() override;

    virtual css::uno::Sequence< css::sheet::opencl::OpenCLPlatform >
        SAL_CALL getOpenCLPlatforms() override;

    virtual void SAL_CALL enableOpcodeSubsetTest() override;

    virtual void SAL_CALL disableOpcodeSubsetTest() override;

    virtual sal_Bool SAL_CALL isOpcodeSubsetTested() override;

    virtual void SAL_CALL setFormulaCellNumberLimit( sal_Int32 number ) override;

    virtual sal_Int32 SAL_CALL getFormulaCellNumberLimit() override;

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

    /// @see vcl::ITiledRenderable::getPartInfo().
    virtual OUString getPartInfo( int nPart ) override;

    /// @see vcl::ITiledRenderable::getPartName().
    virtual OUString getPartName(int nPart) override;

    /// @see vcl::ITiledRenderable::getPartHash().
    virtual OUString getPartHash( int nPart ) override;

    /// @see vcl::ITiledRenderable::getDocWindow().
    virtual VclPtr<vcl::Window> getDocWindow() override;

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

    /// @see vcl::ITiledRenderable::setOutlineState().
    virtual void setOutlineState(bool bColumn, int nLevel, int nIndex, bool bHidden) override;

    /// @see vcl::ITiledRenderable::getRowColumnHeaders().
    virtual OUString getRowColumnHeaders(const tools::Rectangle& rRectangle) override;

    /// @see vcl::ITiledRenderable::getCellCursor().
    virtual OString getCellCursor( int nOutputWidth,
                                   int nOutputHeight,
                                   long nTileWidth,
                                   long nTileHeight ) override;

    /// @see vcl::ITiledRenderable::getPointer().
    virtual PointerStyle getPointer() override;

    /// @see vcl::ITiledRenderable::getTrackedChanges().
    OUString getTrackedChanges() override;

    /// @see vcl::ITiledRenderable::setClientVisibleArea().
    virtual void setClientVisibleArea(const tools::Rectangle& rRectangle) override;

    /// @see vcl::ITiledRenderable::getPostIts().
    OUString getPostIts() override;

    /// @see vcl::ITiledRenderable::getPostItsPos().
    OUString getPostItsPos() override;
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
    virtual                 ~ScDrawPagesObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDrawPages
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL
                            insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void SAL_CALL   remove( const css::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    virtual                 ~ScTableSheetsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XSpreadsheets
    virtual void SAL_CALL   insertNewByName( const OUString& aName, sal_Int16 nPosition ) override;
    virtual void SAL_CALL   moveByName( const OUString& aName, sal_Int16 nDestination ) override;
    virtual void SAL_CALL   copyByName( const OUString& aName,
                                const OUString& aCopy, sal_Int16 nDestination ) override;

                            /// XSpreadsheets2
    virtual sal_Int32 SAL_CALL importSheet(
        const css::uno::Reference< css::sheet::XSpreadsheetDocument > & xDocSrc,
        const OUString& srcName,
        sal_Int32 nDestPosition) override;

                            /// XCellRangesAccess

    virtual css::uno::Reference< css::table::XCell >
        SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet ) override;

    virtual css::uno::Reference< css::table::XCellRange >
        SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet ) override;

    virtual css::uno::Sequence < css::uno::Reference< css::table::XCellRange > >
        SAL_CALL getCellRangesByName( const OUString& aRange ) override;

                            /// XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const css::uno::Any& aElement ) override;
    virtual void SAL_CALL   removeByName( const OUString& Name ) override;

                            /// XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const css::uno::Any& aElement ) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            /// XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    SCTAB const             nTab;
    SCCOL const             nStartCol;
    SCCOL const             nEndCol;

    ScTableColumnObj*       GetObjectByIndex_Impl(sal_Int32 nIndex) const;
    ScTableColumnObj*       GetObjectByName_Impl(const OUString& aName) const;

public:
                            ScTableColumnsObj(ScDocShell* pDocSh, SCTAB nT,
                                                SCCOL nSC, SCCOL nEC);
    virtual                 ~ScTableColumnsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XTableColumns
    virtual void SAL_CALL   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) override;

                            /// XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
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

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    SCTAB const             nTab;
    SCROW const             nStartRow;
    SCROW const             nEndRow;

    ScTableRowObj*          GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScTableRowsObj(ScDocShell* pDocSh, SCTAB nT,
                                                SCROW nSR, SCROW nER);
    virtual                 ~ScTableRowsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XTableRows
    virtual void SAL_CALL   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
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

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScSpreadsheetSettingsObj : public cppu::WeakImplHelper<
                                    css::beans::XPropertySet,
                                    css::lang::XServiceInfo>,
                                 public SfxListener
{
public:
    virtual                 ~ScSpreadsheetSettingsObj() override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
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

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScAnnotationsObj : public cppu::WeakImplHelper<
                                css::sheet::XSheetAnnotations,
                                css::container::XEnumerationAccess,
                                css::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB const             nTab;           ///< Collection belongs to the sheet

    bool                    GetAddressByIndex_Impl( sal_Int32 nIndex, ScAddress& rPos ) const;
    ScAnnotationObj*        GetObjectByIndex_Impl( sal_Int32 nIndex ) const;

public:
                            ScAnnotationsObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScAnnotationsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XSheetAnnotations
    virtual void SAL_CALL   insertNew( const css::table::CellAddress& aPosition,
                                const OUString& aText ) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex ) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    SCTAB const             nTab;

    bool                    GetScenarioIndex_Impl( const OUString& rName, SCTAB& rIndex );
    ScTableSheetObj*        GetObjectByIndex_Impl(sal_Int32 nIndex);
    ScTableSheetObj*        GetObjectByName_Impl(const OUString& aName);

public:
                            ScScenariosObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScScenariosObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XScenarios
    virtual void SAL_CALL   addNewByName( const OUString& aName,
                                const css::uno::Sequence< css::table::CellRangeAddress >& aRanges,
                                const OUString& aComment ) override;
    virtual void SAL_CALL   removeByName( const OUString& aName ) override;

                            /// XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
