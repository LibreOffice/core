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
#ifndef INCLUDED_CHART2_INC_CHARTMODEL_HXX
#define INCLUDED_CHART2_INC_CHARTMODEL_HXX

#include "LifeTime.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable2.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/qa/XDumper.hpp>

// public API
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/X3DChartWindowProvider.hpp>

#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <svtools/grfmgr.hxx>

#include <memory>

class SvNumberFormatter;
class OpenGLWindow;

namespace chart
{

namespace impl
{

// Note: needed for queryInterface (if it calls the base-class implementation)
typedef cppu::WeakImplHelper<
//       css::frame::XModel        //comprehends XComponent (required interface), base of XChartDocument
         css::util::XCloseable     //comprehends XCloseBroadcaster
        ,css::frame::XStorable2    //(extension of XStorable)
        ,css::util::XModifiable    //comprehends XModifyBroadcaster (required interface)
        ,css::lang::XServiceInfo
        ,css::lang::XInitialization
        ,css::chart2::XChartDocument  // derived from XModel
        ,css::chart2::data::XDataReceiver   // public API
        ,css::chart2::XTitled
        ,css::frame::XLoadable
        ,css::util::XCloneable
        ,css::embed::XVisualObject
        ,css::lang::XMultiServiceFactory
        ,css::document::XStorageBasedDocument
        ,css::lang::XUnoTunnel
        ,css::util::XNumberFormatsSupplier
        ,css::container::XChild
        ,css::util::XModifyListener
        ,css::datatransfer::XTransferable
        ,css::document::XDocumentPropertiesSupplier
        ,css::chart2::data::XDataSource
        ,css::document::XUndoManagerSupplier
        ,css::chart2::X3DChartWindowProvider
        ,css::util::XUpdatable
        ,css::qa::XDumper
        >
    ChartModel_Base;
}

class UndoManager;
class ChartView;

class OOO_DLLPUBLIC_CHARTTOOLS ChartModel : public impl::ChartModel_Base
{

private:
    mutable ::apphelper::CloseableLifeTimeManager   m_aLifeTimeManager;

    mutable ::osl::Mutex    m_aModelMutex;
    bool volatile       m_bReadOnly;
    bool volatile       m_bModified;
    sal_Int32               m_nInLoad;
    bool volatile       m_bUpdateNotificationsPending;

    bool mbTimeBased;

    rtl::Reference<ChartView> mxChartView;

    OUString m_aResource;
    css::uno::Sequence< css::beans::PropertyValue >   m_aMediaDescriptor;
    css::uno::Reference< css::document::XDocumentProperties > m_xDocumentProperties;
    ::rtl::Reference< UndoManager >                    m_pUndoManager;

    ::comphelper::OInterfaceContainerHelper2           m_aControllers;
    css::uno::Reference< css::frame::XController >     m_xCurrentController;
    sal_uInt16                                         m_nControllerLockCount;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::uno::XAggregation >      m_xOldModelAgg;

    css::uno::Reference< css::embed::XStorage >        m_xStorage;
    //the content of this should be always synchronized with the current m_xViewWindow size. The variable is necessary to hold the information as long as no view window exists.
    css::awt::Size                                     m_aVisualAreaSize;
    css::uno::Reference< css::frame::XModel >          m_xParent;
    css::uno::Reference< css::chart2::data::XRangeHighlighter > m_xRangeHighlighter;
    css::uno::Reference<css::chart2::data::XPopupRequest> m_xPopupRequest;
    ::std::vector< GraphicObject >                            m_aGraphicObjectVector;

    css::uno::Reference< css::chart2::data::XDataProvider >   m_xDataProvider;
    /** is only valid if m_xDataProvider is set. If m_xDataProvider is set to an
        external data provider this reference must be set to 0
    */
    css::uno::Reference< css::chart2::data::XDataProvider >   m_xInternalDataProvider;

    css::uno::Reference< css::util::XNumberFormatsSupplier >
                                m_xOwnNumberFormatsSupplier;
    css::uno::Reference< css::util::XNumberFormatsSupplier >
                                m_xNumberFormatsSupplier;
    std::unique_ptr< SvNumberFormatter > m_apSvNumberFormatter; // #i113784# avoid memory leak

    css::uno::Reference< css::chart2::XChartTypeManager >
        m_xChartTypeManager;

    // Diagram Access
    css::uno::Reference< css::chart2::XDiagram >
        m_xDiagram;

    css::uno::Reference< css::chart2::XTitle >
                                          m_xTitle;

    css::uno::Reference< css::beans::XPropertySet >
                                          m_xPageBackground;

    css::uno::Reference< css::container::XNameAccess>     m_xXMLNamespaceMap;

private:
    //private methods

    OUString impl_g_getLocation();

    bool
        impl_isControllerConnected( const css::uno::Reference< com::sun::star::frame::XController >& xController );

    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::frame::XController >
        impl_getCurrentController();

    /// @throws css::uno::RuntimeException
    void SAL_CALL
        impl_notifyModifiedListeners();
    /// @throws css::uno::RuntimeException
    void SAL_CALL
        impl_notifyCloseListeners();
    /// @throws css::uno::RuntimeException
    void SAL_CALL
        impl_notifyStorageChangeListeners();

    void impl_store(
        const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor,
        const css::uno::Reference< css::embed::XStorage > & xStorage );
    void impl_load(
        const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor,
        const css::uno::Reference< css::embed::XStorage >& xStorage );
    void impl_loadGraphics(
        const css::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    css::uno::Reference< css::document::XFilter >
        impl_createFilter( const css::uno::Sequence< css::beans::PropertyValue > & rMediaDescriptor );

    css::uno::Reference< css::chart2::XChartTypeTemplate > impl_createDefaultChartTypeTemplate();
    css::uno::Reference< css::chart2::data::XDataSource > impl_createDefaultData();

    void impl_adjustAdditionalShapesPositionAndSize(
        const css::awt::Size& aVisualAreaSize );

    void insertDefaultChart();

public:
    ChartModel() = delete;
    ChartModel(css::uno::Reference< css::uno::XComponentContext > const & xContext);
    explicit ChartModel( const ChartModel & rOther );
    virtual ~ChartModel() override;

    // css::lang::XServiceInfo

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::lang::XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // css::frame::XModel (required interface)

    virtual sal_Bool SAL_CALL
        attachResource( const OUString& rURL,
                        const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor ) override;

    virtual OUString SAL_CALL
        getURL() override;

    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
        getArgs() override;

    virtual void SAL_CALL
        connectController( const css::uno::Reference< css::frame::XController >& xController ) override;

    virtual void SAL_CALL
        disconnectController( const css::uno::Reference< css::frame::XController >& xController ) override;

    virtual void SAL_CALL
        lockControllers() override;

    virtual void SAL_CALL
        unlockControllers() override;

    virtual sal_Bool SAL_CALL
        hasControllersLocked() override;

    virtual css::uno::Reference< css::frame::XController > SAL_CALL
        getCurrentController() override;

    virtual void SAL_CALL
        setCurrentController( const css::uno::Reference< css::frame::XController >& xController ) override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
        getCurrentSelection() override;

    // css::lang::XComponent (base of XModel)
    virtual void SAL_CALL
        dispose() override;

    virtual void SAL_CALL
        addEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener ) override;

    virtual void SAL_CALL
        removeEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener ) override;

    // css::util::XCloseable
    virtual void SAL_CALL
        close( sal_Bool bDeliverOwnership ) override;

    // css::util::XCloseBroadcaster (base of XCloseable)
    virtual void SAL_CALL
        addCloseListener( const css::uno::Reference< css::util::XCloseListener > & xListener ) override;

    virtual void SAL_CALL
        removeCloseListener( const css::uno::Reference< css::util::XCloseListener > & xListener ) override;

    // css::frame::XStorable2 (extension of XStorable)
    virtual void SAL_CALL storeSelf(
        const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor ) override;

    // css::frame::XStorable (required interface)
    virtual sal_Bool SAL_CALL
        hasLocation() override;

    virtual OUString SAL_CALL
        getLocation() override;

    virtual sal_Bool SAL_CALL
        isReadonly() override;

    virtual void SAL_CALL
        store() override;

    virtual void SAL_CALL
        storeAsURL( const OUString& rURL,
                    const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor ) override;

    virtual void SAL_CALL
        storeToURL( const OUString& rURL,
                    const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor ) override;

    // css::util::XModifiable (required interface)
    virtual sal_Bool SAL_CALL
        isModified() override;

    virtual void SAL_CALL
        setModified( sal_Bool bModified ) override;

    // css::util::XModifyBroadcaster (base of XModifiable)
    virtual void SAL_CALL
        addModifyListener( const css::uno::Reference< css::util::XModifyListener >& xListener ) override;

    virtual void SAL_CALL
        removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& xListener ) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ____ datatransferable::XTransferable ____
    virtual css::uno::Any SAL_CALL getTransferData(
        const css::datatransfer::DataFlavor& aFlavor ) override;
    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors() override;
    virtual sal_Bool SAL_CALL isDataFlavorSupported(
        const css::datatransfer::DataFlavor& aFlavor ) override;

    // lang::XTypeProvider (override method of WeakImplHelper)
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes() override;

    // ____ document::XDocumentPropertiesSupplier ____
    virtual css::uno::Reference< css::document::XDocumentProperties > SAL_CALL
        getDocumentProperties(  ) override;

    // ____ document::XUndoManagerSupplier ____
    virtual css::uno::Reference< css::document::XUndoManager > SAL_CALL
        getUndoManager(  ) override;

    // css::chart2::XChartDocument
    virtual css::uno::Reference< css::chart2::XDiagram > SAL_CALL
        getFirstDiagram() override;
    virtual void SAL_CALL setFirstDiagram(
        const css::uno::Reference< css::chart2::XDiagram >& xDiagram ) override;
    virtual void SAL_CALL
        createInternalDataProvider( sal_Bool bCloneExistingData ) override;
    virtual sal_Bool SAL_CALL hasInternalDataProvider() override;
    virtual css::uno::Reference< css::chart2::data::XDataProvider > SAL_CALL
        getDataProvider() override;
    virtual void SAL_CALL
        setChartTypeManager( const css::uno::Reference< css::chart2::XChartTypeManager >& xNewManager ) override;
    virtual css::uno::Reference< css::chart2::XChartTypeManager > SAL_CALL
        getChartTypeManager() override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
        getPageBackground() override;

    virtual void SAL_CALL createDefaultChart() override;

    virtual sal_Bool SAL_CALL isOpenGLChart() override;

    // ____ XDataReceiver (public API) ____
    virtual void SAL_CALL
        attachDataProvider( const css::uno::Reference< css::chart2::data::XDataProvider >& xProvider ) override;
    virtual void SAL_CALL setArguments(
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getUsedRangeRepresentations() override;
    virtual css::uno::Reference< css::chart2::data::XDataSource > SAL_CALL getUsedData() override;
    virtual void SAL_CALL attachNumberFormatsSupplier( const css::uno::Reference<
        css::util::XNumberFormatsSupplier >& xSupplier ) override;
    virtual css::uno::Reference< css::chart2::data::XRangeHighlighter > SAL_CALL getRangeHighlighter() override;
    virtual css::uno::Reference< css::chart2::data::XPopupRequest > SAL_CALL getPopupRequest() override;

    // ____ XTitled ____
    virtual css::uno::Reference< css::chart2::XTitle > SAL_CALL getTitleObject() override;
    virtual void SAL_CALL setTitleObject( const css::uno::Reference< css::chart2::XTitle >& Title ) override;

    // ____ XInterface (for old API wrapper) ____
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;

    // ____ XLoadable ____
    virtual void SAL_CALL initNew() override;
    virtual void SAL_CALL load( const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor ) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XVisualObject ____
    virtual void SAL_CALL setVisualAreaSize(
        ::sal_Int64 nAspect,
        const css::awt::Size& aSize ) override;
    virtual css::awt::Size SAL_CALL getVisualAreaSize(
        ::sal_Int64 nAspect ) override;
    virtual css::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation(
        ::sal_Int64 nAspect ) override;
    virtual ::sal_Int32 SAL_CALL getMapUnit(
        ::sal_Int64 nAspect ) override;

    // ____ XMultiServiceFactory ____
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
        createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
        createInstanceWithArguments( const OUString& ServiceSpecifier
                                   , const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames() override;

    // ____ XStorageBasedDocument ____
    virtual void SAL_CALL loadFromStorage(
        const css::uno::Reference< css::embed::XStorage >& xStorage,
        const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor ) override;
    virtual void SAL_CALL storeToStorage(
        const css::uno::Reference< css::embed::XStorage >& xStorage,
        const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor ) override;
    virtual void SAL_CALL switchToStorage(
        const css::uno::Reference< css::embed::XStorage >& xStorage ) override;
    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentStorage() override;
    virtual void SAL_CALL addStorageChangeListener(
        const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) override;
    virtual void SAL_CALL removeStorageChangeListener(
        const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) override;

    // for SvNumberFormatsSupplierObj
    // ____ XUnoTunnel ___
    virtual ::sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< ::sal_Int8 >& aIdentifier ) override;

    // ____ XNumberFormatsSupplier ____
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getNumberFormatSettings() override;
    virtual css::uno::Reference< css::util::XNumberFormats > SAL_CALL getNumberFormats() override;

    // ____ XChild ____
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent() override;
    virtual void SAL_CALL setParent(
        const css::uno::Reference< css::uno::XInterface >& Parent ) override;

    // ____ XDataSource ____ allows access to the currently used data and data ranges
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences() override;

    // X3DChartWindowProvider
    virtual void SAL_CALL setWindow( sal_uInt64 nWindowPtr ) override;

    // XUpdatable
    virtual void SAL_CALL update() override;

    // XDumper
    virtual OUString SAL_CALL dump() override;

    // normal methods
    css::uno::Reference< css::util::XNumberFormatsSupplier > const &
        getNumberFormatsSupplier();

    css::uno::Reference< css::uno::XInterface > getChartView() const;

    bool isTimeBased() const { return mbTimeBased;}
    void setTimeBased(bool bTimeBased);

    void getNextTimePoint();
    void setTimeBasedRange(sal_Int32 nStart, sal_Int32 nEnd);

    OpenGLWindow* getOpenGLWindow() { return mpOpenGLWindow;}

private:
    sal_Int32 mnStart;
    sal_Int32 mnEnd;
    bool bSet;
    VclPtr<OpenGLWindow> mpOpenGLWindow;
};

}  // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
