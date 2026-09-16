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

#include "address.hxx"

#include <sal/types.h>
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
#include <cpo/uno/XAggregation.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <svl/itemprop.hxx>
#include <vcl/ITiledRenderable.hxx>

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
class ScTableSheetsObj;
class SolarMutexGuard;

class SAL_DLLPUBLIC_RTTI ScModelObj : public SfxBaseModel,
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
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocShell;
    std::unique_ptr<ScPrintFuncCache> pPrintFuncCache;
    std::unique_ptr<ScPrintUIOptions> pPrinterOptions;
    std::unique_ptr<ScPrintState> m_pPrintState;
    cpo::uno::Reference<cpo::uno::XAggregation> xNumberAgg;
    cpo::uno::Reference<cpo::uno::XInterface> xDrawGradTab;
    cpo::uno::Reference<cpo::uno::XInterface> xDrawHatchTab;
    cpo::uno::Reference<cpo::uno::XInterface> xDrawBitmapTab;
    cpo::uno::Reference<cpo::uno::XInterface> xDrawTrGradTab;
    cpo::uno::Reference<cpo::uno::XInterface> xDrawMarkerTab;
    cpo::uno::Reference<cpo::uno::XInterface> xDrawDashTab;
    cpo::uno::Reference<cpo::uno::XInterface> xChartDataProv;
    cpo::uno::Reference<cpo::uno::XInterface> xObjProvider;

    ::comphelper::OInterfaceContainerHelper3<css::util::XChangesListener> maChangesListeners;

    bool                    FillRenderMarkData( const cpo::uno::Any& aSelection,
                                                const cpo::uno::Sequence< css::beans::PropertyValue >& rOptions,
                                                ScMarkData& rMark, ScPrintSelectionStatus& rStatus, OUString& rPagesStr,
                                                bool& rbRenderToGraphic ) const;
    cpo::uno::Reference<cpo::uno::XAggregation> const & GetFormatter();
    void                    HandleCalculateEvents();

    cpo::uno::Reference<cpo::uno::XInterface> create(
        OUString const & aServiceSpecifier,
        cpo::uno::Sequence<cpo::uno::Any> const * arguments);

    static bool             IsOnEvenPage( sal_Int32 nPage ) { return nPage % 2 == 0; };

    OUString                maBuildId;
    std::vector<sal_Int32>  maValidPages;
    /// the sheet the previously rendered page belonged to; a page range can leave out its first
    SCTAB mnPreviousRenderTab = -1;

protected:
    const SfxItemPropertySet&   GetPropertySet() const { return aPropSet; }

    /** abstract SdrModel provider */
    virtual SdrModel& getSdrModelFromUnoModel() const override;

public:
                            ScModelObj(ScDocShell* pDocSh);
    virtual                 ~ScModelObj() override;

    /// create ScModelObj and set at pDocSh (SetBaseModel)
    static void             CreateAndSet(ScDocShell* pDocSh);

    SC_DLLPUBLIC ScDocument*     GetDocument() const;
    SC_DLLPUBLIC SfxObjectShell* GetEmbeddedObject() const;

    SC_DLLPUBLIC void UpdateAllRowHeights();

    void                    BeforeXMLLoading();
    void                    AfterXMLLoading();
    ScSheetSaveData*        GetSheetSaveData();
    ScFormatSaveData*       GetFormatSaveData();

    void                    RepaintRange( const ScRange& rRange );
    void                    RepaintRange( const ScRangeList& rRange );

    bool                    HasChangesListeners() const;

    void                    NotifyChanges( const OUString& rOperation, const ScRangeList& rRanges,
                                           const cpo::uno::Sequence< css::beans::PropertyValue >& rProperties );

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void   acquire() noexcept override;
    virtual void   release() noexcept override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XSpreadsheetDocument
    SC_DLLPUBLIC virtual cpo::uno::Reference< css::sheet::XSpreadsheets >
                            getSheets() override final;
    SC_DLLPUBLIC rtl::Reference< ScTableSheetsObj > getScSheets();

                            /// XDataProviderAccess
    virtual ::cpo::uno::Reference< css::chart2::data::XDataProvider >
                            createDataProvider() override;

                            /// XStyleFamiliesSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess >
                            getStyleFamilies() override;

                            /// XRenderable
    virtual sal_Int32 getRendererCount( const cpo::uno::Any& aSelection,
                                    const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue >
                            getRenderer( sal_Int32 nRenderer, const cpo::uno::Any& aSelection,
                                    const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual void   render( sal_Int32 nRenderer, const cpo::uno::Any& aSelection,
                                    const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;

                            /// XLinkTargetSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess >
                            getLinks() override;

                            /// XActionLockable
    virtual bool isActionLocked() override;
    virtual void   addActionLock() override;
    virtual void   removeActionLock() override;
    virtual void   setActionLocks( sal_Int16 nLock ) override;
    virtual sal_Int16 resetActionLocks() override;

        virtual void   lockControllers() override;
        virtual void   unlockControllers() override;

                            /// XCalculatable
    virtual void   calculate() override;
    virtual void   calculateAll() override;
    virtual bool isAutomaticCalculationEnabled() override;
    virtual void   enableAutomaticCalculation( bool bEnabled ) override;

                            /// XProtectable
    virtual void   protect( const OUString& aPassword ) override;
    virtual void   unprotect( const OUString& aPassword ) override;
    virtual bool isProtected() override;

                            /// XDrawPagesSupplier
    virtual cpo::uno::Reference< css::drawing::XDrawPages >
                            getDrawPages() override;

                            /// XGoalSeek
    virtual css::sheet::GoalResult seekGoal(
                                const css::table::CellAddress& aFormulaPosition,
                                const css::table::CellAddress& aVariablePosition,
                                const OUString& aGoalValue ) override;

                            /// XConsolidatable
    virtual cpo::uno::Reference< css::sheet::XConsolidationDescriptor >
                            createConsolidationDescriptor( bool bEmpty ) override;
    virtual void consolidate( const cpo::uno::Reference< css::sheet::XConsolidationDescriptor >& xDescriptor ) override;

                            /// XDocumentAuditing
    virtual void   refreshArrows() override;

                            /// XViewDataSupplier
    virtual cpo::uno::Reference< css::container::XIndexAccess > getViewData(  ) override;

                            /// XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
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

                            /// XMultiServiceFactory
    virtual cpo::uno::Reference< cpo::uno::XInterface >
                            createInstance( const OUString& aServiceSpecifier ) override;
    virtual cpo::uno::Reference< cpo::uno::XInterface >
                            createInstanceWithArguments( const OUString& ServiceSpecifier,
                                const cpo::uno::Sequence< cpo::uno::Any >& Arguments ) override;
    virtual cpo::uno::Sequence< OUString > getAvailableServiceNames() override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

                            /// XUnoTunnel
    virtual sal_Int64 getSomething( const cpo::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    SC_DLLPUBLIC static const cpo::uno::Sequence<sal_Int8>& getUnoTunnelId();

                            /// XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    virtual void dispose() override;

                            /// XChangesNotifier
    virtual void addChangesListener( const cpo::uno::Reference< css::util::XChangesListener >& aListener ) override;
    virtual void removeChangesListener( const cpo::uno::Reference< css::util::XChangesListener >& aListener ) override;

    // XOpenCLSelection
    virtual bool isOpenCLEnabled() override;

    virtual void enableOpenCL(bool bEnable) override;

    virtual void enableAutomaticDeviceSelection(bool bForce) override;

    virtual void disableAutomaticDeviceSelection() override;

    virtual void selectOpenCLDevice( sal_Int32 platform, sal_Int32 device ) override;

    virtual sal_Int32 getPlatformID() override;

    virtual sal_Int32 getDeviceID() override;

    virtual cpo::uno::Sequence< css::sheet::opencl::OpenCLPlatform >
        getOpenCLPlatforms() override;

    virtual void enableOpcodeSubsetTest() override;

    virtual void disableOpcodeSubsetTest() override;

    virtual bool isOpcodeSubsetTested() override;

    virtual void setFormulaCellNumberLimit( sal_Int32 number ) override;

    virtual sal_Int32 getFormulaCellNumberLimit() override;

    /// @see vcl::ITiledRenderable::paintTile().
    virtual void paintTile( VirtualDevice& rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            tools::Long nTileWidth,
                            tools::Long nTileHeight ) override;

    /// @see vcl::ITiledRenderable::getDocumentSize().
    SC_DLLPUBLIC virtual Size getDocumentSize() final override;

    /// @see vcl::ITiledRenderable::getDataArea().
    virtual Size getDataArea(long nPart) override;

    /// @see vcl::ITiledRenderable::getPrintRanges().
    virtual std::string getPrintRanges() override;

    /// @see vcl::ITiledRenderable::setPart().
    virtual void setPart(int nPart, bool bAllowChangeFocus = true) override;

    /// @see vcl::ITiledRenderable::getPart().
    virtual int getPart() override;

    /// @see vcl::ITiledRenderable::getParts().
    virtual int getParts() override;

    /// @see vcl::ITiledRenderable::getPartInfo().
    virtual std::string getPartInfo( int nPart ) override;

    /// @see vcl::ITiledRenderable::getPartName().
    virtual OUString getPartName(int nPart) override;

    /// @see vcl::ITiledRenderable::getPartHash().
    virtual OUString getPartHash( int nPart ) override;

    /// @see vcl::ITiledRenderable::getDocWindow().
    virtual VclPtr<vcl::Window> getDocWindow() override;

    /// @see vcl::ITiledRenderable::initializeForTiledRendering().
    virtual void initializeForTiledRendering(const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments) override;

    /// @see vcl::ITiledRenderable::postKeyEvent().
    virtual void postKeyEvent(COKitKeyEventType eType, int nCharCode, int nKeyCode) override;

    /// @see vcl::ITiledRenderable::postMouseEvent().
    virtual void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount, int nButtons,
                                int nModifier) override;

    /// @see vcl::ITiledRenderable::setTextSelection().
    virtual void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) override;

    /// @see vcl::ITiledRenderable::getSelection().
    virtual cpo::uno::Reference<css::datatransfer::XTransferable> getSelection() override;

    /// @see vcl::ITiledRenderable::setGraphicSelection().
    virtual void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) override;

    /// @see COKitDocument::resetSelection().
    virtual void resetSelection() override;

    /// @see vcl::ITiledRenderable::setClipboard().
    virtual void setClipboard(const cpo::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) override;

    /// @see vcl::ITiledRenderable::isMimeTypeSupported().
    virtual bool isMimeTypeSupported() override;

    /// @see vcl::ITiledRenderable::setClientZoom().
    virtual void setClientZoom(int nTilePixelWidth, int nTilePixelHeight, int nTileTwipWidth, int nTileTwipHeight) override;

    /// @see vcl::ITiledRenderable::setExportZoom().
    virtual void setExportZoom(int nExportZoom) override;

    /// @see vcl::ITiledRenderable::setOutlineState().
    virtual void setOutlineState(bool bColumn, int nLevel, int nIndex, bool bHidden) override;

    /// @see vcl::ITiledRenderable::getRowColumnHeaders().
    virtual void getRowColumnHeaders(const tools::Rectangle& rRectangle, tools::JsonWriter& rJsonWriter) override;

    /// @see vcl::ITiledRenderable::exportRaisesDialog().
    virtual bool exportRaisesDialog(const OUString& rFilterName) override;

    /// @see vcl::ITiledRenderable::getSheetGeometryData().
    virtual std::string getSheetGeometryData(bool bColumns, bool bRows, bool bSizes, bool bHidden,
                                         bool bFiltered, bool bGroups) override;

    /// @see vcl::ITiledRenderable::getCellCursor().
    virtual void getCellCursor(tools::JsonWriter& rJsonWriter) override;

    /// @see vcl::ITiledRenderable::getPointer().
    virtual PointerStyle getPointer() override;

    /// @see vcl::ITiledRenderable::getTrackedChanges().
    void getTrackedChanges(tools::JsonWriter&) override;

    /// @see vcl::ITiledRenderable::setClientVisibleArea().
    virtual void setClientVisibleArea(const tools::Rectangle& rRectangle) override;

    /// @see vcl::ITiledRenderable::getPostIts().
    void getPostIts(tools::JsonWriter& rJsonWriter) override;

    /// @see vcl::ITiledRenderable::getPostItsPos().
    void getPostItsPos(tools::JsonWriter& rJsonWriter) override;

    /// @see vcl::ITiledRenderable::completeFunction().
    virtual void completeFunction(const OUString& rFunctionName) override;

    /// @see vcl::ITiledRenderable::getViewRenderState().
    OString getViewRenderState(const SfxViewShell* pViewShell = nullptr) override;

    /// @see vcl::ITiledRenderable::supportsCommand().
    bool supportsCommand(std::u16string_view rCommand) override;

    /// @see vcl::ITiledRenderable::getCommandValues().
    void getCommandValues(tools::JsonWriter& rJsonWriter, std::string_view rCommand) override;

private:
    Size getDocumentSize(SCCOL& rnTiledRenderingAreaEndCol, SCROW& rnTiledRenderingAreaEndRow );
};

class ScDrawPagesObj final : public cppu::WeakImplHelper<
                                css::drawing::XDrawPages,
                                css::lang::XServiceInfo>,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

cpo::uno::Reference< css::drawing::XDrawPage >
                            GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScDrawPagesObj(ScDocShell* pDocSh);
    virtual                 ~ScDrawPagesObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDrawPages
    virtual cpo::uno::Reference< css::drawing::XDrawPage >
                            insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void   remove( const cpo::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

                            /// XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            /// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class SC_DLLPUBLIC ScTableSheetsObj final : public cppu::WeakImplHelper<
                                css::sheet::XSpreadsheets2,
                                css::sheet::XCellRangesAccess,
                                css::container::XEnumerationAccess,
                                css::container::XIndexAccess,
                                css::lang::XServiceInfo>,
                         public SfxListener
{
private:
    ScDocShell*             pDocShell;

    rtl::Reference<ScTableSheetObj> GetObjectByName_Impl(const OUString& aName) const;

public:
                            ScTableSheetsObj(ScDocShell* pDocSh);
    virtual                 ~ScTableSheetsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XSpreadsheets
    virtual void   insertNewByName( const OUString& aName, sal_Int16 nPosition ) override;
    virtual void   moveByName( const OUString& aName, sal_Int16 nDestination ) override;
    virtual void   copyByName( const OUString& aName,
                                const OUString& aCopy, sal_Int16 nDestination ) override;

                            /// XSpreadsheets2
    virtual sal_Int32 importSheet(
        const cpo::uno::Reference< css::sheet::XSpreadsheetDocument > & xDocSrc,
        const OUString& srcName,
        sal_Int32 nDestPosition) override;

                            /// XCellRangesAccess

    virtual cpo::uno::Reference< css::table::XCell >
        getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet ) override;

    virtual cpo::uno::Reference< css::table::XCellRange >
        getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet ) override;

    virtual cpo::uno::Sequence < cpo::uno::Reference< css::table::XCellRange > >
        getCellRangesByName( const OUString& aRange ) override;

                            /// XNameContainer
    virtual void   insertByName( const OUString& aName,
                                const cpo::uno::Any& aElement ) override;
    virtual void   removeByName( const OUString& Name ) override;

                            /// XNameReplace
    virtual void   replaceByName( const OUString& aName,
                                const cpo::uno::Any& aElement ) override;

                            /// XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            /// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            /// XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            /// XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    rtl::Reference<ScTableSheetObj> GetSheetByIndex(sal_Int32 nIndex) const;
    rtl::Reference<ScTableSheetObj> GetSheetByName(const OUString& rName) const;
};

class SAL_DLLPUBLIC_RTTI ScTableColumnsObj final : public cppu::WeakImplHelper<
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

    rtl::Reference<ScTableColumnObj> GetObjectByIndex_Impl(sal_Int32 nIndex) const;
    rtl::Reference<ScTableColumnObj> GetObjectByName_Impl(std::u16string_view aName) const;

public:
                            ScTableColumnsObj(ScDocShell* pDocSh, SCTAB nT,
                                                SCCOL nSC, SCCOL nEC);
    virtual                 ~ScTableColumnsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XTableColumns
    virtual void   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) override;
    virtual void   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) override;

                            /// XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            /// XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            /// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            /// XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
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

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    SC_DLLPUBLIC rtl::Reference<ScTableColumnObj> getScTableColumnByIndex( sal_Int32 Index ) const;
};

class SAL_DLLPUBLIC_RTTI ScTableRowsObj final : public cppu::WeakImplHelper<
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

    rtl::Reference<ScTableRowObj> GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScTableRowsObj(ScDocShell* pDocSh, SCTAB nT,
                                                SCROW nSR, SCROW nER);
    virtual                 ~ScTableRowsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XTableRows
    virtual void   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) override;
    virtual void   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) override;

                            /// XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            /// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            /// XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
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

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    void setPropertyValueIsFiltered( SolarMutexGuard& rGuard, bool b );
    bool getPropertyValueOHeight( SolarMutexGuard& rGuard );
    SC_DLLPUBLIC rtl::Reference<ScTableRowObj> GetTableRowByIndex( sal_Int32 nIndex) const;
};

class ScSpreadsheetSettingsObj final : public cppu::WeakImplHelper<
                                    css::beans::XPropertySet,
                                    css::lang::XServiceInfo>,
                                 public SfxListener
{
public:
    virtual                 ~ScSpreadsheetSettingsObj() override;

                            /// XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
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

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class SAL_DLLPUBLIC_RTTI ScAnnotationsObj final : public cppu::WeakImplHelper<
                                css::sheet::XSheetAnnotations,
                                css::container::XEnumerationAccess,
                                css::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;           ///< Collection belongs to the sheet

    bool                    GetAddressByIndex_Impl( sal_Int32 nIndex, ScAddress& rPos ) const;
    rtl::Reference<ScAnnotationObj> GetObjectByIndex_Impl( sal_Int32 nIndex ) const;

public:
                            ScAnnotationsObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScAnnotationsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ScDocShell*             GetDocShell() const { return pDocShell; }

                            /// XSheetAnnotations
    virtual void   insertNew( const css::table::CellAddress& aPosition,
                                const OUString& aText ) override;
    virtual void   removeByIndex( sal_Int32 nIndex ) override;

                            /// XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            /// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class SAL_DLLPUBLIC_RTTI ScScenariosObj final : public cppu::WeakImplHelper<
                                css::sheet::XScenarios,
                                css::container::XEnumerationAccess,
                                css::container::XIndexAccess,
                                css::lang::XServiceInfo>,
                          public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;

    bool                    GetScenarioIndex_Impl( std::u16string_view rName, SCTAB& rIndex );
    rtl::Reference<ScTableSheetObj> GetObjectByIndex_Impl(sal_Int32 nIndex);
    rtl::Reference<ScTableSheetObj> GetObjectByName_Impl(std::u16string_view aName);

public:
                            ScScenariosObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScScenariosObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XScenarios
    SC_DLLPUBLIC virtual void   addNewByName( const OUString& aName,
                                const cpo::uno::Sequence< css::table::CellRangeAddress >& aRanges,
                                const OUString& aComment ) override;
    virtual void   removeByName( const OUString& aName ) override;

                            /// XNameAccess
    SC_DLLPUBLIC virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            /// XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            /// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
