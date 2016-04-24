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
#include <cppuhelper/implbase.hxx>
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
#include <com/sun/star/util/XUpdatable2.hpp>
#include <com/sun/star/qa/XDumper.hpp>

#include <vector>
#include <memory>

#include <vcl/timer.hxx>

class SdrPage;

namespace chart {

class VCoordinateSystem;
class DrawModelWrapper;
class VDataSeries;
class GL3DPlotterBase;
class GL2DRenderer;
struct CreateShapeParam2D;

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
class ChartView : public ::cppu::WeakImplHelper<
    css::lang::XInitialization
        ,css::lang::XServiceInfo
        ,css::datatransfer::XTransferable
        ,css::lang::XUnoTunnel
        ,css::util::XModifyListener
        ,css::util::XModeChangeBroadcaster
        ,css::util::XUpdatable2
        ,css::beans::XPropertySet
        ,css::lang::XMultiServiceFactory
        ,css::qa::XDumper
        >
        , public ExplicitValueProvider
        , private SfxListener
{
    friend class GL2DRenderer;
private:
    void init();

public:
    ChartView() = delete;
    ChartView(css::uno::Reference< css::uno::XComponentContext > const & xContext,
               ChartModel& rModel);

    virtual ~ChartView();

    // ___lang::XServiceInfo___
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // ___lang::XInitialization___
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
                throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // ___ExplicitValueProvider___
    virtual bool getExplicitValuesForAxis(
        css::uno::Reference< css::chart2::XAxis > xAxis
        , ExplicitScaleData&  rExplicitScale
        , ExplicitIncrementData& rExplicitIncrement ) override;
    virtual css::uno::Reference< css::drawing::XShape >
        getShapeForCID( const OUString& rObjectCID ) override;

    virtual css::awt::Rectangle getRectangleOfObject( const OUString& rObjectCID, bool bSnapRect=false ) override;

    virtual css::awt::Rectangle getDiagramRectangleExcludingAxes() override;

    std::shared_ptr< DrawModelWrapper > getDrawModelWrapper() override;

    // ___XTransferable___
    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor )
                throw (css::datatransfer::UnsupportedFlavorException
                     , css::io::IOException
                     , css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
                throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor )
                throw (css::uno::RuntimeException, std::exception) override;

    // css::util::XEventListener (base of XCloseListener and XModifyListener)
    virtual void SAL_CALL
        disposing( const css::lang::EventObject& Source )
                            throw (css::uno::RuntimeException, std::exception) override;

    // css::util::XModifyListener
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    //SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // css::util::XModeChangeBroadcaster

    virtual void SAL_CALL addModeChangeListener( const css::uno::Reference< css::util::XModeChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModeChangeListener( const css::uno::Reference< css::util::XModeChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addModeChangeApproveListener( const css::uno::Reference< css::util::XModeChangeApproveListener >& _rxListener ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModeChangeApproveListener( const css::uno::Reference< css::util::XModeChangeApproveListener >& _rxListener ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // css::util::XUpdatable
    virtual void SAL_CALL update() throw (css::uno::RuntimeException, std::exception) override;

    // util::XUpdatable2
    virtual void SAL_CALL updateSoft() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateHard() throw (css::uno::RuntimeException, std::exception) override;

    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // css::lang::XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(
        const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // for ExplicitValueProvider
    // ____ XUnoTunnel ___
    virtual ::sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< ::sal_Int8 >& aIdentifier )
            throw (css::uno::RuntimeException, std::exception) override;

    // XDumper
    virtual OUString SAL_CALL dump()
            throw(css::uno::RuntimeException,
                  std::exception) override;

    void setViewDirty();
    void updateOpenGLWindow();

private: //methods
    void createShapes();
    void createShapes2D( const css::awt::Size& rPageSize );
    bool createAxisTitleShapes2D( CreateShapeParam2D& rParam, const css::awt::Size& rPageSize );
    void createShapes3D();
    bool isReal3DChart();
    void getMetaFile( const css::uno::Reference< css::io::XOutputStream >& xOutStream
                      , bool bUseHighContrast );
    SdrPage* getSdrPage();

    void impl_deleteCoordinateSystems();
    void impl_notifyModeChangeListener( const OUString& rNewMode );

    void impl_refreshAddIn();

    void impl_updateView( bool bCheckLockedCtrler = true );

    void render();

    css::awt::Rectangle impl_createDiagramAndContent( const CreateShapeParam2D& rParam, const css::awt::Size& rPageSize );

    DECL_LINK_TYPED( UpdateTimeBased, Timer*, void );

private: //member
    ::osl::Mutex m_aMutex;

    css::uno::Reference< css::uno::XComponentContext>
            m_xCC;

    chart::ChartModel& mrChartModel;

    css::uno::Reference< css::lang::XMultiServiceFactory>
            m_xShapeFactory;
    css::uno::Reference< css::drawing::XDrawPage>
            m_xDrawPage;
    css::uno::Reference< css::drawing::XShapes >
            mxRootShape;

    css::uno::Reference< css::uno::XInterface > m_xDashTable;
    css::uno::Reference< css::uno::XInterface > m_xGradientTable;
    css::uno::Reference< css::uno::XInterface > m_xHatchTable;
    css::uno::Reference< css::uno::XInterface > m_xBitmapTable;
    css::uno::Reference< css::uno::XInterface > m_xTransGradientTable;
    css::uno::Reference< css::uno::XInterface > m_xMarkerTable;

    std::shared_ptr< DrawModelWrapper > m_pDrawModelWrapper;

    std::vector< VCoordinateSystem* > m_aVCooSysList;

    ::cppu::OMultiTypeInterfaceContainerHelper
                        m_aListenerContainer;

    bool m_bViewDirty; //states whether the view needs to be rebuild
    bool m_bInViewUpdate;
    bool m_bViewUpdatePending;
    bool m_bRefreshAddIn;

    //better performance for big data
    css::awt::Size m_aPageResolution;
    bool m_bPointsWereSkipped;

    //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
    sal_Int32 m_nScaleXNumerator;
    sal_Int32 m_nScaleXDenominator;
    sal_Int32 m_nScaleYNumerator;
    sal_Int32 m_nScaleYDenominator;

    bool m_bSdrViewIsInEditMode;

    css::awt::Rectangle m_aResultingDiagramRectangleExcludingAxes;

    std::shared_ptr<GL3DPlotterBase> m_pGL3DPlotter;
    TimeBasedInfo maTimeBased;
    osl::Mutex maTimeMutex;
    std::unique_ptr<GL2DRenderer> mp2DRenderer;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
