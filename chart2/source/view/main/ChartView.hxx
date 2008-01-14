/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartView.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:06:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CHARTVIEWIMPL_HXX
#define _CHARTVIEWIMPL_HXX

#include "chartview/ExplicitValueProvider.hxx"
#include "ServiceMacros.hxx"

#ifndef _CPPUHELPER_IMPLBASE8_HXX_
#include <cppuhelper/implbase8.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

// header for class SfxListener
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODECHANGEBROADCASTER_HPP_
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XUPDATABLE_HPP_
#include <com/sun/star/util/XUpdatable.hpp>
#endif

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

class ChartView : public ::cppu::WeakImplHelper8<
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
        , ::com::sun::star::chart2::ExplicitScaleData&  rExplicitScale
        , ::com::sun::star::chart2::ExplicitIncrementData& rExplicitIncrement );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        getShapeForCID( const rtl::OUString& rObjectCID );

    virtual ::com::sun::star::awt::Rectangle getRectangleOfObject( const rtl::OUString& rObjectCID, bool bSnapRect=false );

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

    void impl_createDiagramAndContent( SeriesPlotterContainer& rSeriesPlotterContainer
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes>& xDiagramPlusAxes_Shapes
        , const ::com::sun::star::awt::Point& rAvailablePos
        , const ::com::sun::star::awt::Size& rAvailableSize
        , const ::com::sun::star::awt::Size& rPageSize );


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
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
