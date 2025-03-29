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
#pragma once

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>

#include <svl/lstner.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/qa/XDumper.hpp>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XUpdatable2.hpp>
#include <rtl/ref.hxx>
#include <svx/unopage.hxx>
#include <svx/unoshape.hxx>

#include <vector>
#include <memory>
#include <mutex>

#include <vcl/timer.hxx>

namespace com::sun::star::drawing { class XDrawPage; }
namespace com::sun::star::drawing { class XShapes; }
namespace com::sun::star::io { class XOutputStream; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::util { class XUpdatable2; }
namespace com::sun::star::util { class XNumberFormatsSupplier; }

class SdrPage;

namespace chart {

class Axis;
class BaseCoordinateSystem;
class ChartModel;
struct CreateShapeParam2D;
class DrawModelWrapper;
struct ExplicitIncrementData;
struct ExplicitScaleData;
class VCoordinateSystem;
class VDataSeries;

struct TimeBasedInfo
{
    TimeBasedInfo():
        bTimeBased(false),
        nFrame(0) {}

    bool bTimeBased;
    size_t nFrame;
    Timer maTimer { "chart2 TimeBasedInfo" };

    // only valid when we are in the time based mode
    std::vector< std::vector< VDataSeries* > > m_aDataSeriesList;
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
class ChartView final : public ::cppu::WeakImplHelper<
    css::lang::XInitialization
        ,css::lang::XServiceInfo
        ,css::datatransfer::XTransferable
        ,css::util::XModifyListener
        ,css::util::XModeChangeBroadcaster
        ,css::util::XUpdatable2
        ,css::beans::XPropertySet
        ,css::lang::XMultiServiceFactory
        ,css::qa::XDumper
        >
        , private SfxListener
{
private:
    void init();

public:
    ChartView() = delete;
    ChartView(css::uno::Reference<css::uno::XComponentContext> xContext, ChartModel& rModel);

    virtual ~ChartView() override;

    // ___lang::XServiceInfo___
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ___lang::XInitialization___
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    /** Gives calculated scale and increment values for a given xAxis in the current view.
        In contrast to the model data these explicit values are always complete as missing auto properties are calculated.
        If the given Axis could not be found or for another reason no correct output can be given false is returned.
     */
    bool getExplicitValuesForAxis(
        const rtl::Reference< Axis > & xAxis
        , ExplicitScaleData&  rExplicitScale
        , ExplicitIncrementData& rExplicitIncrement );
    rtl::Reference< SvxShape >
        getShapeForCID( const OUString& rObjectCID );

    /** for rotated objects the shape size and position differs from the visible rectangle
        if bSnapRect is set to true you get the resulting visible position (left-top) and size
    */
    css::awt::Rectangle getRectangleOfObject( const OUString& rObjectCID, bool bSnapRect=false );

    css::awt::Rectangle getDiagramRectangleExcludingAxes();

    std::shared_ptr< DrawModelWrapper > getDrawModelWrapper();

    // ___XTransferable___
    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) override;
    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) override;
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) override;

    // css::util::XEventListener (base of XCloseListener and XModifyListener)
    virtual void SAL_CALL
        disposing( const css::lang::EventObject& Source ) override;

    // css::util::XModifyListener
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    //SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // css::util::XModeChangeBroadcaster

    virtual void SAL_CALL addModeChangeListener( const css::uno::Reference< css::util::XModeChangeListener >& _rxListener ) override;
    virtual void SAL_CALL removeModeChangeListener( const css::uno::Reference< css::util::XModeChangeListener >& _rxListener ) override;
    virtual void SAL_CALL addModeChangeApproveListener( const css::uno::Reference< css::util::XModeChangeApproveListener >& _rxListener ) override;
    virtual void SAL_CALL removeModeChangeApproveListener( const css::uno::Reference< css::util::XModeChangeApproveListener >& _rxListener ) override;

    // css::util::XUpdatable
    virtual void SAL_CALL update() override;

    // util::XUpdatable2
    virtual void SAL_CALL updateSoft() override;
    virtual void SAL_CALL updateHard() override;

    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // css::lang::XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(
        const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

    // XDumper
    virtual OUString SAL_CALL dump(OUString const & kind) override;

    void setViewDirty();

    css::uno::Reference<css::uno::XComponentContext> const& getComponentContext() { return m_xCC;}

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    static css::awt::Rectangle
        AddSubtractAxisTitleSizes(
                ChartModel& rModel
            , ChartView* pChartView
            , const css::awt::Rectangle& rPositionAndSize, bool bSubtract );

    static sal_Int32 getExplicitNumberFormatKeyForAxis(
              const rtl::Reference< ::chart::Axis >& xAxis
            , const rtl::Reference< ::chart::BaseCoordinateSystem > & xCorrespondingCoordinateSystem
            , const rtl::Reference<::chart::ChartModel>& xChartDoc);

    static sal_Int32 getExplicitNumberFormatKeyForDataLabel(
            const css::uno::Reference< css::beans::XPropertySet >& xSeriesOrPointProp );

    static sal_Int32 getExplicitPercentageNumberFormatKeyForDataLabel(
            const css::uno::Reference< css::beans::XPropertySet >& xSeriesOrPointProp
            , const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

private: //methods
    void createShapes();
    void createShapes2D( const css::awt::Size& rPageSize );
    bool createAxisTitleShapes2D( CreateShapeParam2D& rParam, const css::awt::Size& rPageSize, bool bHasRelativeSize );
    void getMetaFile( const css::uno::Reference< css::io::XOutputStream >& xOutStream
                      , bool bUseHighContrast );
    SdrPage* getSdrPage();

    void impl_deleteCoordinateSystems();
    void impl_notifyModeChangeListener( const OUString& rNewMode );

    void impl_refreshAddIn();

    void impl_updateView( bool bCheckLockedCtrler = true );

    css::awt::Rectangle impl_createDiagramAndContent( const CreateShapeParam2D& rParam, const css::awt::Size& rPageSize );

    DECL_LINK( UpdateTimeBased, Timer*, void );

private: //member
    std::mutex m_aMutex;

    css::uno::Reference< css::uno::XComponentContext> m_xCC;

    ChartModel& mrChartModel;

    css::uno::Reference< css::lang::XMultiServiceFactory>
            m_xShapeFactory;
    rtl::Reference<SvxDrawPage>
            m_xDrawPage;
    rtl::Reference<SvxShapeGroupAnyD>
            mxRootShape;

    css::uno::Reference< css::uno::XInterface > m_xDashTable;
    css::uno::Reference< css::uno::XInterface > m_xGradientTable;
    css::uno::Reference< css::uno::XInterface > m_xHatchTable;
    css::uno::Reference< css::uno::XInterface > m_xBitmapTable;
    css::uno::Reference< css::uno::XInterface > m_xTransGradientTable;
    css::uno::Reference< css::uno::XInterface > m_xMarkerTable;

    std::shared_ptr< DrawModelWrapper > m_pDrawModelWrapper;

    std::vector< std::unique_ptr<VCoordinateSystem> > m_aVCooSysList;

    comphelper::OInterfaceContainerHelper4<css::util::XModeChangeListener>
                        m_aModeChangeListeners;

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

    TimeBasedInfo maTimeBased;
    std::mutex maTimeMutex;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
