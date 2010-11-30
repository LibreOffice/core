/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHARTVIEWIMPL_HXX
#define _CHARTVIEWIMPL_HXX

#include "chartview/ExplicitValueProvider.hxx"
#include "ServiceMacros.hxx"
#include <cppuhelper/implbase9.hxx>
#include <cppuhelper/interfacecontainer.hxx>

// header for class SfxListener
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

#include <vector>
#include <boost/shared_ptr.hpp>

class SdrPage;

//.............................................................................
namespace chart
{
//.............................................................................

class VCoordinateSystem;
class DrawModelWrapper;
class SeriesPlotterContainer;

//-----------------------------------------------------------------------------
/** The ChartView is responsible to manage the generation of Drawing Objects
for visualization on a given OutputDevice. The ChartModel is responsible to notify changes to the view.
The view than changes to state dirty. The view can be updated with call 'update'.

The View is not responsible to handle single user events (that is instead done by the ChartWindow).
*/

class ChartView : public ::cppu::WeakImplHelper9<
    ::com::sun::star::lang::XInitialization
        , ::com::sun::star::lang::XServiceInfo
        , ::com::sun::star::datatransfer::XTransferable
        ,::com::sun::star::lang::XUnoTunnel
        //::com::sun::star::lang::XComponent ???
        //::com::sun::star::uno::XWeak          // implemented by WeakImplHelper(optional interface)
        //::com::sun::star::uno::XInterface     // implemented by WeakImplHelper(optional interface)
        //::com::sun::star::lang::XTypeProvider // implemented by WeakImplHelper
        ,::com::sun::star::util::XModifyListener
        ,::com::sun::star::util::XModeChangeBroadcaster
        ,::com::sun::star::util::XUpdatable
        ,::com::sun::star::beans::XPropertySet
        ,::com::sun::star::lang::XMultiServiceFactory
        >
        , public ExplicitValueProvider
        , private SfxListener
{
public:
    ChartView(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    virtual ~ChartView();

    // ___lang::XServiceInfo___
    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartView)

    // ___lang::XInitialization___
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
                throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // ___ExplicitValueProvider___
    virtual sal_Bool getExplicitValuesForAxis(
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > xAxis
        , ExplicitScaleData&  rExplicitScale
        , ExplicitIncrementData& rExplicitIncrement );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        getShapeForCID( const rtl::OUString& rObjectCID );

    virtual ::com::sun::star::awt::Rectangle getRectangleOfObject( const rtl::OUString& rObjectCID, bool bSnapRect=false );

    virtual ::com::sun::star::awt::Rectangle getDiagramRectangleExcludingAxes();

    ::boost::shared_ptr< DrawModelWrapper > getDrawModelWrapper();

    // ___XTransferable___
    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
                throw (::com::sun::star::datatransfer::UnsupportedFlavorException
                     , ::com::sun::star::io::IOException
                     , ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
                throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
                throw (::com::sun::star::uno::RuntimeException);

    //-------------------------------------------------------------------------------------
    // ::com::sun::star::util::XEventListener (base of XCloseListener and XModifyListener)
    //-------------------------------------------------------------------------------------
    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XModifyListener
    //-----------------------------------------------------------------
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    //SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XModeChangeBroadcaster
    //-----------------------------------------------------------------

    virtual void SAL_CALL addModeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addModeChangeApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeApproveListener >& _rxListener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModeChangeApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeApproveListener >& _rxListener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XUpdatable
    //-----------------------------------------------------------------
    virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::beans::XPropertySet
    //-----------------------------------------------------------------
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::lang::XMultiServiceFactory
    //-----------------------------------------------------------------
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments(
        const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // for ExplicitValueProvider
    // ____ XUnoTunnel ___
    virtual ::sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aIdentifier )
            throw (::com::sun::star::uno::RuntimeException);

private: //methods
    ChartView();

    void createShapes();
    void getMetaFile( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutStream
                      , bool bUseHighContrast );
    SdrPage* getSdrPage();

    void impl_setChartModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    void impl_deleteCoordinateSystems();
    void impl_notifyModeChangeListener( const rtl::OUString& rNewMode );

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


private: //member
    ::osl::Mutex m_aMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>
            m_xCC;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
            m_xChartModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>
            m_xShapeFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage>
            m_xDrawPage;

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

    bool volatile       m_bViewDirty; //states wether the view needs to be rebuild
    bool volatile       m_bInViewUpdate;
    bool volatile       m_bViewUpdatePending;
    bool volatile       m_bRefreshAddIn;

    //better performance for big data
    ::com::sun::star::awt::Size m_aPageResolution;
    bool m_bPointsWereSkipped;

    //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
    sal_Int32 m_nScaleXNumerator;
    sal_Int32 m_nScaleXDenominator;
    sal_Int32 m_nScaleYNumerator;
    sal_Int32 m_nScaleYDenominator;

    sal_Bool m_bSdrViewIsInEditMode;

    ::com::sun::star::awt::Rectangle m_aResultingDiagramRectangleExcludingAxes;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
