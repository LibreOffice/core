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
#ifndef INCLUDED_CHART2_INC_CHARTVIEW_HXX
#define INCLUDED_CHART2_INC_CHARTVIEW_HXX

#include "ChartModel.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "ServiceMacros.hxx"
#include <cppuhelper/implbase10.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <svl/lstner.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/qa/XDumper.hpp>

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <vcl/timer.hxx>

class SdrPage;

namespace chart {

class VCoordinateSystem;
class DrawModelWrapper;
class SeriesPlotterContainer;
class VDataSeries;
class GL3DPlotterBase;

enum TimeBasedMode
{
    MANUAL,
    AUTOMATIC,
    AUTOMATIC_WRAP
};

struct TimeBasedInfo
{
    TimeBasedInfo():
        bTimeBased(false),
        nFrame(0),
        eMode(AUTOMATIC) {}

    bool bTimeBased;
    size_t nFrame;
    TimeBasedMode eMode;
    Timer maTimer;

    // only valid when we are in the time based mode
    ::std::vector< std::vector< VDataSeries* > > m_aDataSeriesList;
};

/**
 * The ChartView is responsible to manage the generation of Drawing Objects
 * for visualization on a given OutputDevice. The ChartModel is responsible
 * to notify changes to the view. The view than changes to state dirty. The
 * view can be updated with call 'update'.
 *
 * The View is not responsible to handle single user events (that is instead
 * done by the ChartWindow).
 */
class ChartView : public ::cppu::WeakImplHelper10<
    ::com::sun::star::lang::XInitialization
        ,::com::sun::star::lang::XServiceInfo
        ,::com::sun::star::datatransfer::XTransferable
        ,::com::sun::star::lang::XUnoTunnel
        ,::com::sun::star::util::XModifyListener
        ,::com::sun::star::util::XModeChangeBroadcaster
        ,::com::sun::star::util::XUpdatable
        ,::com::sun::star::beans::XPropertySet
        ,::com::sun::star::lang::XMultiServiceFactory
        ,::com::sun::star::qa::XDumper
        >
        , public ExplicitValueProvider
        , private SfxListener
{
private:
    void init();

public:
    ChartView(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext,
               ChartModel& rModel);

    virtual ~ChartView();

    // ___lang::XServiceInfo___
    APPHELPER_XSERVICEINFO_DECL()

    // ___lang::XInitialization___
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
                throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ___ExplicitValueProvider___
    virtual bool getExplicitValuesForAxis(
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > xAxis
        , ExplicitScaleData&  rExplicitScale
        , ExplicitIncrementData& rExplicitIncrement ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        getShapeForCID( const OUString& rObjectCID ) SAL_OVERRIDE;

    virtual ::com::sun::star::awt::Rectangle getRectangleOfObject( const OUString& rObjectCID, bool bSnapRect=false ) SAL_OVERRIDE;

    virtual ::com::sun::star::awt::Rectangle getDiagramRectangleExcludingAxes() SAL_OVERRIDE;

    ::boost::shared_ptr< DrawModelWrapper > getDrawModelWrapper() SAL_OVERRIDE;

    // ___XTransferable___
    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
                throw (::com::sun::star::datatransfer::UnsupportedFlavorException
                     , ::com::sun::star::io::IOException
                     , ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::util::XEventListener (base of XCloseListener and XModifyListener)
    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    // ::com::sun::star::util::XModeChangeBroadcaster

    virtual void SAL_CALL addModeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addModeChangeApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeApproveListener >& _rxListener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModeChangeApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeApproveListener >& _rxListener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::util::XUpdatable
    virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments(
        const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // for ExplicitValueProvider
    // ____ XUnoTunnel ___
    virtual ::sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aIdentifier )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XDumper
    virtual OUString SAL_CALL dump()
            throw(::com::sun::star::uno::RuntimeException,
                  std::exception) SAL_OVERRIDE;

    void setViewDirty();

private: //methods
    ChartView();

    void createShapes();
    void createShapes3D();
    bool isReal3DChart();
    void getMetaFile( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutStream
                      , bool bUseHighContrast );
    SdrPage* getSdrPage();

    void impl_deleteCoordinateSystems();
    void impl_notifyModeChangeListener( const OUString& rNewMode );

    void impl_refreshAddIn();
    bool impl_AddInDrawsAllByItself();

    void impl_updateView();

    ::com::sun::star::awt::Rectangle impl_createDiagramAndContent( SeriesPlotterContainer& rSeriesPlotterContainer
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes>& xDiagramPlusAxes_Shapes
        , const ::com::sun::star::awt::Point& rAvailablePos
        , const ::com::sun::star::awt::Size& rAvailableSize
        , const ::com::sun::star::awt::Size& rPageSize
        , bool bUseFixedInnerSize
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape>& xDiagram_MarkHandles );

    DECL_LINK( UpdateTimeBased, void* );

private: //member
    ::osl::Mutex m_aMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>
            m_xCC;

    chart::ChartModel& mrChartModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>
            m_xShapeFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage>
            m_xDrawPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
            mxRootShape;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xDashTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xGradientTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xHatchTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xBitmapTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xTransGradientTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xMarkerTable;

    ::boost::shared_ptr< DrawModelWrapper > m_pDrawModelWrapper;

    std::vector< VCoordinateSystem* > m_aVCooSysList;

    ::cppu::OMultiTypeInterfaceContainerHelper
                        m_aListenerContainer;

    bool m_bViewDirty; //states whether the view needs to be rebuild
    bool m_bInViewUpdate;
    bool m_bViewUpdatePending;
    bool m_bRefreshAddIn;

    //better performance for big data
    ::com::sun::star::awt::Size m_aPageResolution;
    bool m_bPointsWereSkipped;

    //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
    sal_Int32 m_nScaleXNumerator;
    sal_Int32 m_nScaleXDenominator;
    sal_Int32 m_nScaleYNumerator;
    sal_Int32 m_nScaleYDenominator;

    bool m_bSdrViewIsInEditMode;

    ::com::sun::star::awt::Rectangle m_aResultingDiagramRectangleExcludingAxes;

    boost::shared_ptr<GL3DPlotterBase> m_pGL3DPlotter;
    TimeBasedInfo maTimeBased;
    osl::Mutex maTimeMutex;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
