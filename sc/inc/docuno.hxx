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
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/interfacecontainer.h>
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
                    public SvxFmMSFactory,  ///< derived from XMultiServiceFactory
                    public com::sun::star::lang::XServiceInfo,
                    public ::com::sun::star::util::XChangesNotifier,
                    public com::sun::star::sheet::opencl::XOpenCLSelection
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
                                                ScMarkData& rMark, ScPrintSelectionStatus& rStatus, OUString& rPagesStr ) const;
    com::sun::star::uno::Reference<com::sun::star::uno::XAggregation> GetFormatter();
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
    void                    AfterXMLLoading(bool bRet);
    ScSheetSaveData*        GetSheetSaveData();

    void                    RepaintRange( const ScRange& rRange );
    void                    RepaintRange( const ScRangeList& rRange );

    bool                    HasChangesListeners() const;

    void                    NotifyChanges( const OUString& rOperation, const ScRangeList& rRanges,
                                           const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rProperties =
                                               ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >() );

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XSpreadsheetDocument
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheets > SAL_CALL
                            getSheets() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XStyleFamiliesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getStyleFamilies() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                       ::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getRenderer( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                       ::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   render( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::beans::PropertyValue >& xOptions )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                       ::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;

                            /// XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getLinks() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addActionLock() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeActionLock() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setActionLocks( sal_Int16 nLock )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL resetActionLocks() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL   lockControllers() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL   unlockControllers() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XCalculatable
    virtual void SAL_CALL   calculate() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   calculateAll() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isAutomaticCalculationEnabled()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   enableAutomaticCalculation( sal_Bool bEnabled )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XProtectable
    virtual void SAL_CALL   protect( const OUString& aPassword )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   unprotect( const OUString& aPassword )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isProtected() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XDrawPagesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > SAL_CALL
                            getDrawPages() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XGoalSeek
    virtual ::com::sun::star::sheet::GoalResult SAL_CALL seekGoal(
                                const ::com::sun::star::table::CellAddress& aFormulaPosition,
                                const ::com::sun::star::table::CellAddress& aVariablePosition,
                                const OUString& aGoalValue )
                                    throw (::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;

                            /// XConsolidatable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XConsolidationDescriptor >
                            SAL_CALL createConsolidationDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL consolidate( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XConsolidationDescriptor >& xDescriptor )
                                    throw (::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;

                            /// XDocumentAuditing
    virtual void SAL_CALL   refreshArrows() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XViewDataSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getViewData(  )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException,
                                    std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            createInstance( const OUString& aServiceSpecifier )
                                throw(::com::sun::star::uno::Exception,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            createInstanceWithArguments( const OUString& ServiceSpecifier,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::uno::Any >& Arguments )
                                throw(::com::sun::star::uno::Exception,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScModelObj* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);

                            /// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XChangesNotifier
    virtual void SAL_CALL addChangesListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XChangesListener >& aListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeChangesListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XChangesListener >& aListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XOpenCLSelection
    virtual sal_Bool SAL_CALL isOpenCLEnabled()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL enableOpenCL(sal_Bool bEnable)
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL enableAutomaticDeviceSelection(sal_Bool bForce)
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL disableAutomaticDeviceSelection()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL selectOpenCLDevice( sal_Int32 platform, sal_Int32 device )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getPlatformID()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getDeviceID()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< com::sun::star::sheet::opencl::OpenCLPlatform >
        SAL_CALL getOpenCLPlatforms()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL enableOpcodeSubsetTest()
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL disableOpcodeSubsetTest()
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isOpcodeSubsetTested()
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setFormulaCellNumberLimit( sal_Int32 number )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getFormulaCellNumberLimit()
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::paintTile().
    virtual void paintTile( VirtualDevice& rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            long nTileWidth,
                            long nTileHeight ) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::getDocumentSize().
    virtual Size getDocumentSize() SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::setPart().
    virtual void setPart(int nPart) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::getPart().
    virtual int getPart() SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::getParts().
    virtual int getParts() SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::getPartName().
    virtual OUString getPartName(int nPart) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::initializeForTiledRendering().
    virtual void initializeForTiledRendering() SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::registerCallback().
    virtual void registerCallback(LibreOfficeKitCallback pCallback, void* pData) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::postKeyEvent().
    virtual void postKeyEvent(int nType, int nCharCode, int nKeyCode) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::postMouseEvent().
    virtual void postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons = MOUSE_LEFT, int nModifier = 0) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::setTextSelection().
    virtual void setTextSelection(int nType, int nX, int nY) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::getTextSelection().
    virtual OString getTextSelection(const char* pMimeType, OString& rUsedMimeType) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::setGraphicSelection().
    virtual void setGraphicSelection(int nType, int nX, int nY) SAL_OVERRIDE;

    /// @see lok::Document::resetSelection().
    virtual void resetSelection() SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::setClipboard().
    virtual void setClipboard(const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::isMimeTypeSupported().
    virtual bool isMimeTypeSupported() SAL_OVERRIDE;
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XDrawPages
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
                            insertNewByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   remove( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XDrawPage >& xPage )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XSpreadsheets
    virtual void SAL_CALL   insertNewByName( const OUString& aName, sal_Int16 nPosition )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   moveByName( const OUString& aName, sal_Int16 nDestination )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   copyByName( const OUString& aName,
                                const OUString& aCopy, sal_Int16 nDestination )
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;

                            /// XSpreadsheets2
    virtual sal_Int32 SAL_CALL importSheet(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::sheet::XSpreadsheetDocument > & xDocSrc,
        const OUString& srcName,
        sal_Int32 nDestPosition)
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::lang::IndexOutOfBoundsException,
              ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XCellRangesAccess

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
        SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
        SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence < ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > >
        SAL_CALL getCellRangesByName( const OUString& aRange )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::ElementExistException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeByName( const OUString& Name )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XTableColumns
    virtual void SAL_CALL   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException,
                                    std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XTableRows
    virtual void SAL_CALL   insertByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException,
                                    std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScAnnotationsObj : public cppu::WeakImplHelper3<
                                com::sun::star::sheet::XSheetAnnotations,
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::lang::XServiceInfo>,
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XSheetAnnotations
    virtual void SAL_CALL   insertNew( const ::com::sun::star::table::CellAddress& aPosition,
                                const OUString& aText )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
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

    bool                    GetScenarioIndex_Impl( const OUString& rName, SCTAB& rIndex );
    ScTableSheetObj*        GetObjectByIndex_Impl(sal_Int32 nIndex);
    ScTableSheetObj*        GetObjectByName_Impl(const OUString& aName);

public:
                            ScScenariosObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScScenariosObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XScenarios
    virtual void SAL_CALL   addNewByName( const OUString& aName,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::table::CellRangeAddress >& aRanges,
                                const OUString& aComment )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
