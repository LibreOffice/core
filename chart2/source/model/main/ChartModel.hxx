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
#ifndef _CHART_MODEL_HXX
#define _CHART_MODEL_HXX

#include "LifeTime.hxx"
#include "ServiceMacros.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable2.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XCloseable.hpp>
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

#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#ifndef INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_22
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_22
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 22
#include "comphelper/implbase_var.hxx"
#endif
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <svtools/grfmgr.hxx>

// for auto_ptr
#include <memory>

class SvNumberFormatter;

namespace chart
{

namespace impl
{

// Note: needed for queryInterface (if it calls the base-class implementation)
typedef ::comphelper::WeakImplHelper22<
//       ::com::sun::star::frame::XModel        //comprehends XComponent (required interface), base of XChartDocument
         ::com::sun::star::util::XCloseable     //comprehends XCloseBroadcaster
        ,::com::sun::star::frame::XStorable2    //(extension of XStorable)
        ,::com::sun::star::util::XModifiable    //comprehends XModifyBroadcaster (required interface)
        ,::com::sun::star::lang::XServiceInfo
        ,::com::sun::star::lang::XInitialization
        ,::com::sun::star::chart2::XChartDocument  // derived from XModel
        ,::com::sun::star::chart2::data::XDataReceiver   // public API
        ,::com::sun::star::chart2::XTitled
        ,::com::sun::star::frame::XLoadable
        ,::com::sun::star::util::XCloneable
        ,::com::sun::star::embed::XVisualObject
        ,::com::sun::star::lang::XMultiServiceFactory
        ,::com::sun::star::document::XStorageBasedDocument
        ,::com::sun::star::lang::XUnoTunnel
        ,::com::sun::star::util::XNumberFormatsSupplier
        ,::com::sun::star::container::XChild
        ,::com::sun::star::util::XModifyListener
        ,::com::sun::star::datatransfer::XTransferable
        ,::com::sun::star::document::XDocumentPropertiesSupplier
        ,::com::sun::star::chart2::data::XDataSource
        ,::com::sun::star::document::XUndoManagerSupplier
        ,::com::sun::star::qa::XDumper
        >
    ChartModel_Base;
}

class UndoManager;

class ChartModel : public impl::ChartModel_Base
{

private:
    mutable ::apphelper::CloseableLifeTimeManager   m_aLifeTimeManager;

    mutable ::osl::Mutex    m_aModelMutex;
    sal_Bool volatile       m_bReadOnly;
    sal_Bool volatile       m_bModified;
    sal_Int32               m_nInLoad;
    sal_Bool volatile       m_bUpdateNotificationsPending;

    OUString                                                             m_aResource;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aMediaDescriptor;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties > m_xDocumentProperties;
    ::rtl::Reference< UndoManager >                                             m_pUndoManager;

    ::cppu::OInterfaceContainerHelper                                           m_aControllers;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >    m_xCurrentController;
    sal_uInt16                                                                  m_nControllerLockCount;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >      m_xOldModelAgg;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >        m_xStorage;
    //the content of this should be always synchronized with the current m_xViewWindow size. The variable is necessary to hold the information as long as no view window exists.
    ::com::sun::star::awt::Size                                                  m_aVisualAreaSize;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >          m_xParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XRangeHighlighter > m_xRangeHighlighter;
    ::std::vector< GraphicObject >                                               m_aGraphicObjectVector;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >   m_xDataProvider;
    /** is only valid if m_xDataProvider is set. If m_xDataProvider is set to an
        external data provider this reference must be set to 0
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >   m_xInternalDataProvider;

    ::com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >
                                m_xOwnNumberFormatsSupplier;
    ::com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >
                                m_xNumberFormatsSupplier;
    std::auto_ptr< SvNumberFormatter > m_apSvNumberFormatter; // #i113784# avoid memory leak

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartTypeManager >
        m_xChartTypeManager;

    // Diagram Access
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >
        m_xDiagram;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTitle >
                                          m_xTitle;

    bool                                  m_bIsDisposed;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                          m_xPageBackground;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xXMLNamespaceMap;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >     m_xModifyListener;

private:
    //private methods

    OUString impl_g_getLocation();

    sal_Bool
        impl_isControllerConnected( const com::sun::star::uno::Reference<
                            com::sun::star::frame::XController >& xController );

    com::sun::star::uno::Reference< com::sun::star::frame::XController >
        impl_getCurrentController()
                            throw( com::sun::star::uno::RuntimeException);

    void SAL_CALL
        impl_notifyModifiedListeners()
                            throw( com::sun::star::uno::RuntimeException);
    void SAL_CALL
        impl_notifyCloseListeners()
                            throw( com::sun::star::uno::RuntimeException);
    void SAL_CALL
        impl_notifyStorageChangeListeners()
                            throw(::com::sun::star::uno::RuntimeException);

    void impl_killInternalData() throw( com::sun::star::util::CloseVetoException );

    void impl_store(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& rMediaDescriptor,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage > & xStorage );
    void impl_load(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& rMediaDescriptor,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage >& xStorage );
    void impl_loadGraphics(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage >& xStorage );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XFilter >
        impl_createFilter( const ::com::sun::star::uno::Sequence<
                      ::com::sun::star::beans::PropertyValue > & rMediaDescriptor );

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartTypeTemplate > impl_createDefaultChartTypeTemplate();
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > impl_createDefaultData();

    void impl_adjustAdditionalShapesPositionAndSize(
        const ::com::sun::star::awt::Size& aVisualAreaSize );

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
        impl_getNumberFormatsSupplier();

public:
    //no default constructor
    ChartModel(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    explicit ChartModel( const ChartModel & rOther );
    virtual ~ChartModel();

    // ::com::sun::star::lang::XServiceInfo

    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartModel)

    // ::com::sun::star::lang::XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
                throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XModel (required interface)

    virtual sal_Bool SAL_CALL
        attachResource( const OUString& rURL
                            , const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL
        getURL()            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
        getArgs()           throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        connectController( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XController >& xController )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        disconnectController( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XController >& xController )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        lockControllers()   throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        unlockControllers() throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        hasControllersLocked()
                            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL
        getCurrentController()
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setCurrentController( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XController >& xController )
                            throw (::com::sun::star::container::NoSuchElementException
                            , ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        getCurrentSelection()
                            throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent (base of XModel)
    virtual void SAL_CALL
        dispose()           throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::lang::XEventListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::lang::XEventListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XCloseable
    virtual void SAL_CALL
        close( sal_Bool bDeliverOwnership )
                            throw(::com::sun::star::util::CloseVetoException,
                                  ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XCloseBroadcaster (base of XCloseable)
    virtual void SAL_CALL
        addCloseListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XCloseListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeCloseListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XCloseListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XStorable2 (extension of XStorable)
    virtual void SAL_CALL storeSelf(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XStorable (required interface)
    virtual sal_Bool SAL_CALL
        hasLocation()       throw (::com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL
        getLocation()       throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        isReadonly()        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        store()             throw (::com::sun::star::io::IOException
                            , ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        storeAsURL( const OUString& rURL
                            , const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
                            throw (::com::sun::star::io::IOException
                            , ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        storeToURL( const OUString& rURL
                            , const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
                            throw (::com::sun::star::io::IOException
                            , ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifiable (required interface)
    virtual sal_Bool SAL_CALL
        isModified()        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setModified( sal_Bool bModified )
                            throw (::com::sun::star::beans::PropertyVetoException
                            , ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifyBroadcaster (base of XModifiable)
    virtual void SAL_CALL
        addModifyListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XModifyListener >& xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeModifyListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XModifyListener >& xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ datatransferable::XTransferable ____
    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData(
        const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
        throw (::com::sun::star::datatransfer::UnsupportedFlavorException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isDataFlavorSupported(
        const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
        throw (::com::sun::star::uno::RuntimeException);

    // lang::XTypeProvider (overloaded method of WeakImplHelper)
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes() throw (::com::sun::star::uno::RuntimeException);

    // ____ document::XDocumentPropertiesSupplier ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties > SAL_CALL
        getDocumentProperties(  ) throw (::com::sun::star::uno::RuntimeException);

    // ____ document::XUndoManagerSupplier ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > SAL_CALL
        getUndoManager(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::chart2::XChartDocument
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > SAL_CALL
        getFirstDiagram()       throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFirstDiagram(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        createInternalDataProvider( sal_Bool bCloneExistingData )
            throw (::com::sun::star::util::CloseVetoException,
                   ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasInternalDataProvider()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider > SAL_CALL
        getDataProvider()
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setChartTypeManager( const ::com::sun::star::uno::Reference<
                             ::com::sun::star::chart2::XChartTypeManager >& xNewManager )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartTypeManager > SAL_CALL
        getChartTypeManager()
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
        getPageBackground()
            throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataReceiver (public API) ____
    virtual void SAL_CALL
        attachDataProvider( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::chart2::data::XDataProvider >& xProvider )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setArguments(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getUsedRangeRepresentations()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > SAL_CALL getUsedData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL attachNumberFormatsSupplier( const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XNumberFormatsSupplier >& xSupplier )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XRangeHighlighter > SAL_CALL getRangeHighlighter()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle > SAL_CALL getTitleObject()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitleObject( const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XInterface (for old API wrapper) ____
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XLoadable ____
    virtual void SAL_CALL initNew()
        throw (::com::sun::star::frame::DoubleInitializationException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL load( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
        throw (::com::sun::star::frame::DoubleInitializationException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XVisualObject ____
    virtual void SAL_CALL setVisualAreaSize(
        ::sal_Int64 nAspect,
        const ::com::sun::star::awt::Size& aSize )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::embed::WrongStateException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getVisualAreaSize(
        ::sal_Int64 nAspect )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::embed::WrongStateException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation(
        ::sal_Int64 nAspect )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::embed::WrongStateException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getMapUnit(
        ::sal_Int64 nAspect )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);

    // ____ XMultiServiceFactory ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        createInstance( const OUString& aServiceSpecifier )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        createInstanceWithArguments( const OUString& ServiceSpecifier
                                   , const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // ____ XStorageBasedDocument ____
    virtual void SAL_CALL loadFromStorage(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::frame::DoubleInitializationException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToStorage(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL switchToStorage(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getDocumentStorage()
        throw (::com::sun::star::io::IOException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStorageChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStorageChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    // for SvNumberFormatsSupplierObj
    // ____ XUnoTunnel ___
    virtual ::sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aIdentifier )
            throw (::com::sun::star::uno::RuntimeException);

    // ____ XNumberFormatsSupplier ____
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > SAL_CALL getNumberFormatSettings()
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::util::XNumberFormats > SAL_CALL getNumberFormats()
            throw (::com::sun::star::uno::RuntimeException);

    // ____ XChild ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent )
        throw (::com::sun::star::lang::NoSupportException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XDataSource ____ allows access to the curently used data and data ranges
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences()
        throw (::com::sun::star::uno::RuntimeException);

    // XDumper
    virtual OUString SAL_CALL dump()
        throw (com::sun::star::uno::RuntimeException);
};

}  // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
