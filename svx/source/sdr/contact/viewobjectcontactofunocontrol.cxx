/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewobjectcontactofunocontrol.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:29:03 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX
#include <svx/sdr/contact/viewobjectcontactofunocontrol.hxx>
#endif
#ifndef SVX_SDR_CONTACT_VIEWCONTACTOFUNOCONTROL_HXX
#include <svx/sdr/contact/viewcontactofunocontrol.hxx>
#endif
#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif
#ifndef _SDR_PROPERTIES_PROPERTIES_HXX
#include <svx/sdr/properties/properties.hxx>
#endif
#ifndef _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW2_HPP_
#include <com/sun/star/awt/XWindow2.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVIEW_HPP_
#include <com/sun/star/awt/XView.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_INVALIDATESTYLE_HPP_
#include <com/sun/star/awt/InvalidateStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODECHANGELISTENER_HPP_
#include <com/sun/star/util/XModeChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODECHANGEBROADCASTER_HPP_
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SVDOUNO_HXX
#include <svx/svdouno.hxx>
#endif
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif
#ifndef _SDRPAINTWINDOW_HXX
#include "sdrpaintwindow.hxx"
#endif
#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif
#ifndef SVX_SOURCE_FORM_FORMPDFEXPORT_HXX
#include "formpdfexport.hxx"
#endif

#ifndef _VCL_PDFEXTOUTDEVDATA_HXX
#include <vcl/pdfextoutdevdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

#include <boost/shared_ptr.hpp>

//........................................................................
namespace sdr { namespace contact {
//........................................................................

    /** === begin UNO using === **/
    using namespace ::com::sun::star::awt::InvalidateStyle;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::awt::XControlModel;
    using ::com::sun::star::awt::XControlContainer;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::awt::XWindow2;
    using ::com::sun::star::awt::XWindowListener;
    using ::com::sun::star::awt::PosSize::POSSIZE;
    using ::com::sun::star::awt::XView;
    using ::com::sun::star::awt::XGraphics;
    using ::com::sun::star::awt::WindowEvent;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::awt::XWindowPeer;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::util::XModeChangeListener;
    using ::com::sun::star::util::XModeChangeBroadcaster;
    using ::com::sun::star::util::ModeChangeEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::container::XContainerListener;
    using ::com::sun::star::container::XContainer;
    using ::com::sun::star::container::ContainerEvent;
    /** === end UNO using === **/

    //====================================================================
    //= UnoControlContactHelper
    //====================================================================
    class UnoControlContactHelper
    {
    public:
        /** positions a control relative to a device

            @precond <arg>_pDevice</arg> is not <NULL/>
        */
        static void positionControl_throw(
                const Reference< XControl >& _rxControl,
                const Rectangle& _rLogicBoundingRect,
                const OutputDevice* _pDevice
            );

        /** sets the zoom at a UNO control, according to a Device's MapMode

            @precond <arg>_pDevice</arg> is not <NULL/>
        */
        static void setControlZoom(
                const Reference< XControl >& _rxControl,
                const OutputDevice* _pDevice
            );

        /** draws a given control onto it's current XGraphics, at given coordinates

            Note that the control is not drawn onto the given device, instead you must
            use ->XView::setGraphics yourself, before calling this method. The given ->OutputDevice
            is only used to calculate pixel coordinates from logic coordinates

            @precond <arg>_pDevice</arg> is not <NULL/>
        */
        static void drawControl(
                const Reference< XControl >& _rxControl,
                const Point& _rLogicTopLeft,
                const OutputDevice* _pDevice
            );

        /** disposes the given control
        */
        static void disposeAndClearControl_nothrow(
                Reference< XControl >& _rxControl
            );

    private:
        UnoControlContactHelper();                                              // never implemented
        UnoControlContactHelper( const UnoControlContactHelper& );              // never implemented
        UnoControlContactHelper& operator=( const UnoControlContactHelper& );   // never implemented
    };

    //--------------------------------------------------------------------
    void UnoControlContactHelper::positionControl_throw( const Reference< XControl >& _rxControl, const Rectangle& _rLogicBoundingRect,
        const OutputDevice* _pDevice )
    {
        OSL_PRECOND( _pDevice, "UnoControlContactHelper::positionControl_throw: no device -> no survival!" );

        Reference< XWindow > xControlWindow( _rxControl, UNO_QUERY );
        if ( xControlWindow.is() )
        {
            const Rectangle aPaintRectPixel(
                _pDevice->LogicToPixel( _rLogicBoundingRect.TopLeft() ),
                _pDevice->LogicToPixel( _rLogicBoundingRect.GetSize() )
            );

            xControlWindow->setPosSize(
                aPaintRectPixel.Left(), aPaintRectPixel.Top(), aPaintRectPixel.GetWidth(), aPaintRectPixel.GetHeight(),
                POSSIZE
            );
        }
    }

    //--------------------------------------------------------------------
    void UnoControlContactHelper::setControlZoom( const Reference< XControl >& _rxControl, const OutputDevice* _pDevice )
    {
        OSL_PRECOND( _pDevice, "UnoControlContactHelper::setControlZoom: no device -> no survival!" );

        Reference< XView > xControlView( _rxControl,  UNO_QUERY );
        DBG_ASSERT( xControlView.is(), "UnoControlContactHelper::setControlZoom: invalid control: no XView!" );
        if ( xControlView.is() )
        {
            const MapMode& rMap = _pDevice->GetMapMode();
            xControlView->setZoom( (float)double( rMap.GetScaleX() ), (float)double( rMap.GetScaleY() ) );
        }
    }

    //--------------------------------------------------------------------
    void UnoControlContactHelper::drawControl( const Reference< XControl >& _rxControl, const Point& _rLogicTopLeft,
        const OutputDevice* _pDevice )
    {
        try
        {
            Point aPixelPos = _pDevice->LogicToPixel( _rLogicTopLeft );
            Reference< XView > xControlView( _rxControl, UNO_QUERY_THROW );
            xControlView->draw( aPixelPos.X(), aPixelPos.Y() );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "UnoControlContactHelper::drawControl: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void UnoControlContactHelper::disposeAndClearControl_nothrow( Reference< XControl >& _rxControl )
    {
        try
        {
            Reference< XComponent > xControlComp( _rxControl, UNO_QUERY );
            if ( xControlComp.is() )
                xControlComp->dispose();
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "UnoControlContactHelper::disposeAndClearControl_nothrow: caught an exception!" );
        }
        _rxControl = NULL;
    }

    //====================================================================
    //= IPageViewAccess
    //====================================================================
    /** interface encapsulating access to an SdrPageView, stripped down to the methods we really need
     */
    class IPageViewAccess
    {
    public:
        /** determines whether the view is currently in design mode
         */
        virtual bool    isDesignMode() const = 0;

        /** retrieves the control container for a given output device
         */
        virtual Reference< XControlContainer >
                        getControlContainer( const OutputDevice& _rDevice ) const = 0;

        /** determines whether a given layer is visible
         */
        virtual bool    isLayerVisible( SdrLayerID _nLayerID ) const = 0;
    };

    //====================================================================
    //= SdrPageViewAccess
    //====================================================================
    /** is a ->IPageViewAccess implementation based on a real ->SdrPageView instance
     */
    class SdrPageViewAccess : public IPageViewAccess
    {
        const SdrPageView&  m_rPageView;
    public:
        SdrPageViewAccess( const SdrPageView& _rPageView ) : m_rPageView( _rPageView ) { }

        virtual bool    isDesignMode() const;
        virtual Reference< XControlContainer >
                        getControlContainer( const OutputDevice& _rDevice ) const;
        virtual bool    isLayerVisible( SdrLayerID _nLayerID ) const;
    };

    //--------------------------------------------------------------------
    bool SdrPageViewAccess::isDesignMode() const
    {
        return m_rPageView.GetView().IsDesignMode();
    }

    //--------------------------------------------------------------------
    Reference< XControlContainer > SdrPageViewAccess::getControlContainer( const OutputDevice& _rDevice ) const
    {
        Reference< XControlContainer > xControlContainer = m_rPageView.GetControlContainer( _rDevice );
        DBG_ASSERT( xControlContainer.is() || NULL == m_rPageView.FindPageWindow( ( const_cast< OutputDevice& >( _rDevice ) ) ),
            "SdrPageViewAccess::getControlContainer: the output device is known, but there is no control container for it?" );
        return xControlContainer;
    }

    //--------------------------------------------------------------------
    bool SdrPageViewAccess::isLayerVisible( SdrLayerID _nLayerID ) const
    {
        return m_rPageView.GetVisibleLayers().IsSet( _nLayerID );
    }

    //====================================================================
    //= DummyPageViewAccess
    //====================================================================
    /** is a ->IPageViewAccess implementation which defaults all attribute access, and thus can be
        operated without an actual ->SdrPageView
     */
    class DummyPageViewAccess : public IPageViewAccess
    {
    public:
        virtual bool    isDesignMode() const;
        virtual Reference< XControlContainer >
                        getControlContainer( const OutputDevice& _rDevice ) const;
        virtual bool    isLayerVisible( SdrLayerID _nLayerID ) const;
    };

    //--------------------------------------------------------------------
    bool DummyPageViewAccess::isDesignMode() const
    {
        return true;
    }

    //--------------------------------------------------------------------
    Reference< XControlContainer > DummyPageViewAccess::getControlContainer( const OutputDevice& /*_rDevice*/ ) const
    {
        return Reference< XControlContainer >();
    }

    //--------------------------------------------------------------------
    bool DummyPageViewAccess::isLayerVisible( SdrLayerID /*_nLayerID*/ ) const
    {
        return true;
    }

    //====================================================================
    //= DummyPageViewAccess
    //====================================================================
    /** is a ->IPageViewAccess implementation which can be used to create an invisble control for
        an arbitrary device
     */
    class InvisibleControlViewAccess : public IPageViewAccess
    {
    private:
        Reference< XControlContainer >& m_rControlContainer;
    public:
        InvisibleControlViewAccess( Reference< XControlContainer >& _inout_ControlContainer )
            :m_rControlContainer( _inout_ControlContainer )
        {
        }

        virtual bool    isDesignMode() const;
        virtual Reference< XControlContainer >
                        getControlContainer( const OutputDevice& _rDevice ) const;
        virtual bool    isLayerVisible( SdrLayerID _nLayerID ) const;
    };

    //--------------------------------------------------------------------
    bool InvisibleControlViewAccess::isDesignMode() const
    {
        return true;
    }

    //--------------------------------------------------------------------
    Reference< XControlContainer > InvisibleControlViewAccess::getControlContainer( const OutputDevice& _rDevice ) const
    {
        if ( !m_rControlContainer.is() )
        {
            const Window* pWindow = dynamic_cast< const Window* >( &_rDevice );
            OSL_ENSURE( pWindow, "InvisibleControlViewAccess::getControlContainer: expected to be called for a window only!" );
            if ( pWindow )
                m_rControlContainer = VCLUnoHelper::CreateControlContainer( const_cast< Window* >( pWindow ) );
        }
        return m_rControlContainer;
    }

    //--------------------------------------------------------------------
    bool InvisibleControlViewAccess::isLayerVisible( SdrLayerID /*_nLayerID*/ ) const
    {
        return false;
    }

    //====================================================================
    //= ViewObjectContactOfUnoControl_Impl
    //====================================================================
    typedef     ::cppu::WeakImplHelper4 <   XWindowListener
                                        ,   XPropertyChangeListener
                                        ,   XContainerListener
                                        ,   XModeChangeListener
                                        >   ViewObjectContactOfUnoControl_Impl_Base;

    class SVX_DLLPRIVATE ViewObjectContactOfUnoControl_Impl : public ViewObjectContactOfUnoControl_Impl_Base
    {
    private:
        /// the instance whose IMPL we are
        ViewObjectContactOfUnoControl*    m_pAntiImpl;

        /** thread safety

            (not really. ATM only our X* implementations are guarded with this, but not
            the object as a whole.)
        */
        mutable ::osl::Mutex            m_aMutex;

        /// the control we're responsible for
        Reference< XControl >           m_xControl;

        /// the ControlContainer where we inserted our control
        Reference< XContainer >         m_xContainer;

        /// the output device for which the control was created
        OutputDevice const*             m_pOutputDeviceForWindow;

        /// flag indicating whether the control is currently visible
        bool                            m_bControlIsVisible;

        /// are we currently listening at a design mode control?
        bool                            m_bIsDesignModeListening;

        enum ViewControlMode
        {
            eDesign,
            eAlive,
            eUnknown
        };
        /// is the control currently in design mode?
        mutable ViewControlMode         m_eControlDesignMode;

        /** recursion level indicating how much PaintObject calls are currently on the stack
        */
        sal_uInt16                      m_nPaintLevel;

    public:
        ViewObjectContactOfUnoControl_Impl( ViewObjectContactOfUnoControl* _pAntiImpl );

        /** disposes the instance, which is nonfunctional afterwards
        */
        void dispose();

        /** determines whether the instance is disposed
        */
        bool isDisposed() const { return impl_isDisposed_nofail(); }

        /** determines whether the instance is alive
        */
        bool isAlive() const { return !isDisposed(); }

        /** returns the SdrUnoObject associated with the ViewContact

            @precond
                We're not disposed.
        */
        bool    getUnoObject( SdrUnoObj*& _out_rpObject ) const;

        /** does initializations for a paint-related method

            @param _rDisplayInfo
                display info as passed to the paint-related method
            @param _out_rpObject
                out-parameter taking our SdrUnoObj upon successfull return
            @param _out_rxControlView
                out-parameter taking our control's XView interface upon successfull return
        */
        bool initPaint( const DisplayInfo& _rDisplayInfo, SdrUnoObj*& _out_rpObject,
            Reference< XView >& _out_rxControlView );

        /** does initializations for a paint-related method

            @param _rDisplayInfo
                display info as passed to the paint-related method
        */
        bool initPaint( const DisplayInfo& _rDisplayInfo )
        {
            Reference< XView > xUnused;
            SdrUnoObj* pUnused( NULL );
            return initPaint( _rDisplayInfo, pUnused, xUnused );
        }

        /** ensures that we have an XControl which can be painted onto the given display
        */
        bool    ensureControl( const DisplayInfo& _rDisplayInfo );

        /** ensures that we have an ->XControl

            Must only be called if a control is needed when no DisplayInfo is present, yet.

            For creating a control, an ->OutputDevice is needed, and an ->SdrPageView. Both can only be
            obtained from a ->DisplayInfo struct, or alternatively a ->ObjectContactOfPageView. So, if
            our (anti-impl's) object contact is not a ->ObjectContactOfPageView, this method fill fail.

            Failure of this method will be reported via an assertion in a non-product version.
        */
        bool    ensureControl();

        /** returns our XControl, if it already has been created

            If you want to ensure that the control exists before accessing it, use ->ensureControl
        */
        inline Reference< XControl >
                getExistentControl() const { return m_xControl; }

        /** positions our XControl according to the geometry settings in the SdrUnoObj,
            and sets proper zoom settings according to our device

            @precond
                ->m_pOutputDeviceForWindow and ->m_xControl are not <NULL/>
            @tolerant
                If the preconditions are not met, nothing is done at all
        */
        void    positionAndZoomControl() const;

        /** positions the control for a paint onto a given device

            If we do not (yet) have a control, or the control does not belong to the
            device for which a paint is requested, no positioning happens.
        */
        void    positionControlForPaint( const DisplayInfo& _rDisplayInfo ) const;

        /** prepares drawing the control for print or print preview

            @param _rDevice
                the device onto which the control is going to be painted
            @return <TRUE/>
                if and only if drawing should continue
        */
        bool    preparePrintOrPrintPreview( OutputDevice& _rDevice ) const;

        /** determines whether or not our control is printable

            Effectively, this method returns the value of the "Printable" property
            of the control's model. If we have no control, <FALSE/> is returned.
        */
        bool    isPrintableControl() const;

        /** prepares painting the control onto the given device.

            Basically, this method obtains an XGraphics object for the device,
            and forwards it to the control.
        */
        bool    preparePaintOnDevice( OutputDevice& _rDevice ) const;

        /** paints the control
        */
        void    paintControl( const DisplayInfo& _rDisplayInfo ) const;

        /** sets the design mode on the control, or at least remembers the flag for the
            time the control is created
        */
        void    setControlDesignMode( bool _bDesignMode ) const;

        /** determines whether our control is currently visible
            @nofail
        */
        bool    isControlVisible() const { return impl_isControlVisible_nofail(); }

        /** determines whether the instance belongs to a given OutputDevice
            @precond
                The instance knows the device it belongs to, or can determine it.
                If this is not the case, you will notice an assertion, and the method will
                return false.
        */
        bool    belongsToDevice( const OutputDevice* _pDevice ) const;

        /// creates an XControl for the given device and SdrUnoObj
        static bool
                createControlForDevice(
                    IPageViewAccess& _rPageView,
                    const OutputDevice& _rDevice,
                    const SdrUnoObj& _rUnoObject,
                    Reference< XControl >& _out_rControl
                );

        /// access control for locking the paint level
        struct PaintLockAccessControl { friend class PaintLock; private: PaintLockAccessControl() { } };
        /// indicates begin of a paint
        inline void enterPaint( const PaintLockAccessControl& )
        {
            ++m_nPaintLevel;
        }
        /// indicates end of a paint
        inline void leavePaint( const PaintLockAccessControl& )
        {
            DBG_ASSERT( m_nPaintLevel, "ViewObjectContactOfUnoControl_Impl::leavePaint: not locked!" );
            --m_nPaintLevel;
        }

        struct GuardAccess { friend class VOCGuard; private: GuardAccess() { } };
        ::osl::Mutex&   getMutex( GuardAccess ) const { return m_aMutex; }

    protected:
        ~ViewObjectContactOfUnoControl_Impl();

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) throw(RuntimeException);

        // XWindowListener
        virtual void SAL_CALL windowResized( const WindowEvent& e ) throw(RuntimeException);
        virtual void SAL_CALL windowMoved( const WindowEvent& e ) throw(RuntimeException);
        virtual void SAL_CALL windowShown( const EventObject& e ) throw(RuntimeException);
        virtual void SAL_CALL windowHidden( const EventObject& e ) throw(RuntimeException);

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const PropertyChangeEvent& evt ) throw(RuntimeException);

        // XModeChangeListener
        virtual void SAL_CALL modeChanged( const ModeChangeEvent& _rSource ) throw (RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** retrieves the SdrPageView which our associated SdrPageViewWindow belongs to

            @param out_rpPageView
                a reference to a pointer holding, upon return, the desired SdrPageView

            @return
                <TRUE/> if and only if a ->SdrPageView could be obtained

            @precond
                We really belong to an SdrPageViewWindow. Perhaps (I'm not sure ATM :)
                there are instance for which this might not be true, but those instances
                should never have a need to call this method.

            @precond
                We're not disposed.

            @postcond
                The method expects success, if it returns with <FALSE/>, this will have been
                asserted.

            @nothrow
        */
        bool    impl_getPageView_nothrow( SdrPageView*& _out_rpPageView );

        /** adjusts the control visibility so it respects its layer's visibility

            @param _bForce
                set to <TRUE/> if you want to force a ->XWindow::setVisible call,
                no matter if the control visibility is already correct

            @precond
                ->m_xControl is not <NULL/>

            @precond
                We're not disposed.

            @precond
                We really belong to an SdrPageViewWindow. There are instance for which this
                might not be true, but those instances should never have a need to call
                this method.
        */
        void impl_adjustControlVisibilityToLayerVisibility_throw( bool _bForce );

        /** adjusts the control visibility so it respects its layer's visibility

            The control must never be visibile if it's in design mode.
            In alive mode, it must be visibility if and only it's on a visible layer.

            @param _rxControl
                the control whose visibility is to be adjusted

            @param _rPageView
                provides access to the attributes of the SdrPageView which the control finally belongs to

            @param _rUnoObject
                our SdrUnoObj

            @param _bIsCurrentlyVisible
                determines whether the control is currently visible. Note that this is only a shortcut for
                querying _rxControl for the XWindow2 interface, and calling isVisible at this interface.
                This shortcut has been chosen since the caller usually already has this information.
                If _bForce is <TRUE/>, _bIsCurrentlyVisible is ignored.

            @param _bForce
                set to <TRUE/> if you want to force a ->XWindow::setVisible call,
                no matter if the control visibility is already correct

            @precond
                We're not disposed.
        */
        static void impl_adjustControlVisibilityToLayerVisibility_throw( const Reference< XControl >& _rxControl, const SdrUnoObj& _rUnoObject,
            IPageViewAccess& _rPageView, bool _bIsCurrentlyVisible, bool _bForce );

        /** starts or stops listening at various aspects of our control

            @precond
                ->m_xControl is not <NULL/>
        */
        void impl_switchControlListening_nothrow( bool _bStart );

        /** starts or stops listening at our control container

            @precond
                ->m_xContainer is not <NULL/>
        */
        void impl_switchContainerListening_nothrow( bool _bStart );

        /** starts or stops listening at the control for design-mode relevant facets
        */
        void impl_switchDesignModeListening_nothrow( bool _bStart );

        /** starts or stops listening for all properties at our control

            @param _bStart
                determines whether to start or to stop listening

            @precond
                ->m_xControl is not <NULL/>
        */
        void impl_switchPropertyListening_nothrow( bool _bStart );

        /** disposes the instance
            @param _bAlsoDisposeControl
                determines whether the XControl should be disposed, too
        */
        void impl_dispose_nothrow( bool _bAlsoDisposeControl );

        /** determines whether the instance is disposed
            @nofail
        */
        bool    impl_isDisposed_nofail() const { return m_pAntiImpl == NULL; }

        /** determines whether our control is currently visible
            @nofail
        */
        bool    impl_isControlVisible_nofail() const { return m_bControlIsVisible; }

        /** determines whether we are currently a listener at the control for desgin-mode relevant facets
            @nofail
        */
        bool    impl_isDesignModeListening_nofail() const { return m_bIsDesignModeListening; }

        /** determines whether the control currently is in design mode

            @precond
                The design mode must already be known. It is known when we first had access to
                an SdrPageView (which carries this flag), or somebody explicitly set it from
                outside.
        */
        inline bool impl_isControlDesignMode_nothrow() const
        {
            DBG_ASSERT( m_eControlDesignMode != eUnknown, "ViewObjectContactOfUnoControl_Impl::impl_isControlDesignMode_nothrow: mode is still unknown!" );
            return m_eControlDesignMode == eDesign;
        }

        /** ensures that we have a control for the given PageView/OutputDevice
        */
        bool impl_ensureControl_nothrow( IPageViewAccess& _rPageView, const OutputDevice& _rDevice );

        /** retrieves the device which a PageView belongs to, starting from its ObjectContactOfPageView

            Since #i72752#, the PaintWindow (and thus the OutputDevice) associated with a PageView is not
            constant over its lifetime. Instead, during some paint operations, the PaintWindow/OutputDevice
            might be temporarily patched.

            This method cares for this, by retrieving the very original OutputDevice.
        */
        const OutputDevice& imp_getPageViewDevice_nothrow( const ObjectContactOfPageView& _rObjectContact ) const;

    private:
        ViewObjectContactOfUnoControl_Impl();                                                     // never implemented
        ViewObjectContactOfUnoControl_Impl( const ViewObjectContactOfUnoControl_Impl& );            // never implemented
        ViewObjectContactOfUnoControl_Impl& operator=( const ViewObjectContactOfUnoControl_Impl& ); // never implemented
    };

    //====================================================================
    //= VOCGuard
    //====================================================================
    /** class for guarding a ViewObjectContactOfUnoControl_Impl method
     */
    class VOCGuard
    {
        const ViewObjectContactOfUnoControl_Impl& m_rImpl;
        ::osl::MutexGuard                       m_aMutexGuard;

    public:
        VOCGuard( const ViewObjectContactOfUnoControl_Impl& _rImpl )
            :m_rImpl( _rImpl )
            ,m_aMutexGuard( _rImpl.getMutex( ViewObjectContactOfUnoControl_Impl::GuardAccess() ) )
        {
        }
    };

    //====================================================================
    //= ViewObjectContactOfUnoControl_Impl
    //====================================================================
    DBG_NAME( ViewObjectContactOfUnoControl_Impl )
    //--------------------------------------------------------------------
    ViewObjectContactOfUnoControl_Impl::ViewObjectContactOfUnoControl_Impl( ViewObjectContactOfUnoControl* _pAntiImpl )
        :m_pAntiImpl( _pAntiImpl )
        ,m_pOutputDeviceForWindow( NULL )
        ,m_bControlIsVisible( false )
        ,m_bIsDesignModeListening( false )
        ,m_eControlDesignMode( eUnknown )
        ,m_nPaintLevel( 0 )
    {
        DBG_CTOR( ViewObjectContactOfUnoControl_Impl, NULL );
        DBG_ASSERT( m_pAntiImpl, "ViewObjectContactOfUnoControl_Impl::ViewObjectContactOfUnoControl_Impl: invalid AntiImpl!" );
    }

    //--------------------------------------------------------------------
    ViewObjectContactOfUnoControl_Impl::~ViewObjectContactOfUnoControl_Impl()
    {
        if ( !impl_isDisposed_nofail() )
        {
            acquire();
            dispose();
        }

        DBG_DTOR( ViewObjectContactOfUnoControl_Impl, NULL );
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::impl_dispose_nothrow( bool _bAlsoDisposeControl )
    {
        if ( impl_isDisposed_nofail() )
            return;

        if ( m_xControl.is() )
            impl_switchControlListening_nothrow( false );

        if ( m_xContainer.is() )
            impl_switchContainerListening_nothrow( false );

        // dispose the control
        if ( _bAlsoDisposeControl )
            UnoControlContactHelper::disposeAndClearControl_nothrow( m_xControl );

        m_xControl.clear();
        m_xContainer.clear();
        m_pOutputDeviceForWindow = NULL;
        m_bControlIsVisible = false;

        m_pAntiImpl = NULL;
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::dispose()
    {
        VOCGuard aGuard( *this );
        impl_dispose_nothrow( true );
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::getUnoObject( SdrUnoObj*& _out_rpObject ) const
    {
        OSL_PRECOND( !impl_isDisposed_nofail(), "ViewObjectContactOfUnoControl_Impl::getUnoObject: already disposed()" );
        if ( impl_isDisposed_nofail() )
            _out_rpObject = NULL;
        else
        {
            _out_rpObject = dynamic_cast< SdrUnoObj* >( m_pAntiImpl->GetViewContact().TryToGetSdrObject() );
            DBG_ASSERT( _out_rpObject || !m_pAntiImpl->GetViewContact().TryToGetSdrObject(),
                "ViewObjectContactOfUnoControl_Impl::getUnoObject: invalid SdrObject!" );
        }
        return ( _out_rpObject != NULL );
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::initPaint( const DisplayInfo& _rDisplayInfo, SdrUnoObj*& _out_rpObject,
        Reference< XView >& _out_rxControlView )
    {
        _out_rpObject = NULL;
        if ( !getUnoObject( _out_rpObject ) )
            return false;

        ensureControl( _rDisplayInfo );

        _out_rxControlView = _out_rxControlView.query( m_xControl );
        DBG_ASSERT( _out_rxControlView.is(), "ViewObjectContactOfUnoControl_Impl::initPaint: no control!" );
        return _out_rxControlView.is();
    }


    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::positionControlForPaint( const DisplayInfo& /* #i74769# _rDisplayInfo*/ ) const
    {
        if ( !m_xControl.is() )
            return;

        positionAndZoomControl();
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::positionAndZoomControl() const
    {
        OSL_PRECOND( m_pOutputDeviceForWindow && m_xControl.is(), "ViewObjectContactOfUnoControl_Impl::positionAndZoomControl: no output device or no control!" );
        if ( !m_pOutputDeviceForWindow || !m_xControl.is() )
            return;

        try
        {
            SdrUnoObj* pUnoObject( NULL );
            if ( getUnoObject( pUnoObject ) )
                UnoControlContactHelper::positionControl_throw( m_xControl, pUnoObject->GetLogicRect(), m_pOutputDeviceForWindow );
            UnoControlContactHelper::setControlZoom( m_xControl, m_pOutputDeviceForWindow );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::ensureControl( const DisplayInfo& _rDisplayInfo )
    {
        OSL_PRECOND( !impl_isDisposed_nofail(), "ViewObjectContactOfUnoControl_Impl::ensureControl: already disposed()" );
        if ( impl_isDisposed_nofail() )
            return false;

        const OutputDevice* pDeviceForControl( NULL );

        // if we're working for a page view, use the respective OutputDevice at the proper
        // PaintWindow. The DisplayInfo might only contain a temporary (virtual) device, which
        // is dangerous to remember
        // 2006-10-24 / #i70604# / frank.schoenheit@sun.com
        ObjectContactOfPageView* pPageViewContact = dynamic_cast< ObjectContactOfPageView* >( &m_pAntiImpl->GetObjectContact() );
        if ( pPageViewContact )
            pDeviceForControl = &imp_getPageViewDevice_nothrow( *pPageViewContact );

        if ( !pDeviceForControl )
            pDeviceForControl = _rDisplayInfo.GetOutputDevice();

        DBG_ASSERT( pDeviceForControl, "ViewObjectContactOfUnoControl_Impl::ensureControl: no output device!" );
        if ( !pDeviceForControl )
            return false;

        SdrPageView* pPageView = pPageViewContact ? &pPageViewContact->GetPageWindow().GetPageView() : _rDisplayInfo.GetPageView();

        ::std::auto_ptr< IPageViewAccess > pPVAccess;
        pPVAccess.reset( pPageView ? (IPageViewAccess*)new SdrPageViewAccess( *pPageView ) : (IPageViewAccess*)new DummyPageViewAccess() );
        return impl_ensureControl_nothrow( *pPVAccess, *pDeviceForControl );
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::ensureControl()
    {
        OSL_PRECOND( !impl_isDisposed_nofail(), "ViewObjectContactOfUnoControl_Impl::ensureControl: already disposed()" );
        if ( impl_isDisposed_nofail() )
            return false;

        ObjectContactOfPageView* pPageViewContact = dynamic_cast< ObjectContactOfPageView* >( &m_pAntiImpl->GetObjectContact() );
        DBG_ASSERT( pPageViewContact, "ViewObjectContactOfUnoControl_Impl::ensureControl: cannot create a control if I don't have a PageView!" );
        if ( !pPageViewContact )
            return false;

        SdrPageViewAccess aPVAccess( pPageViewContact->GetPageWindow().GetPageView() );
        return impl_ensureControl_nothrow(
            aPVAccess,
            imp_getPageViewDevice_nothrow( *pPageViewContact )
        );
    }

    //--------------------------------------------------------------------
    const OutputDevice& ViewObjectContactOfUnoControl_Impl::imp_getPageViewDevice_nothrow( const ObjectContactOfPageView& _rObjectContact ) const
    {
        // if the PageWindow has a patched PaintWindow, use the original PaintWindow
        // this ensures that our control is _not_ re-created just because somebody
        // (temporarily) changed the window to paint onto.
        // #i72429# / 2007-02-20 / frank.schoenheit@sun.com
        const SdrPageWindow& rPageWindow( _rObjectContact.GetPageWindow() );
        if ( rPageWindow.GetOriginalPaintWindow() )
            return rPageWindow.GetOriginalPaintWindow()->GetOutputDevice();

        return rPageWindow.GetPaintWindow().GetOutputDevice();
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::impl_ensureControl_nothrow( IPageViewAccess& _rPageView, const OutputDevice& _rDevice )
    {
        if ( m_xControl.is() )
        {
            if ( m_pOutputDeviceForWindow == &_rDevice )
                return true;

            // Somebody requested a control for a new device, which means either of
            // - our PageView's paint window changed since we were last here
            // - we don't belong to a page view, and are simply painted onto different devices
            // The first sounds strange (doens't  it?), the second means we could perhaps
            // optimize this in the future - there is no need to re-create the control every time,
            // is it?
            // #i74523# / 2007-02-15 / frank.schoenheit@sun.com
            if ( m_xContainer.is() )
                impl_switchContainerListening_nothrow( false );
            impl_switchControlListening_nothrow( false );
            UnoControlContactHelper::disposeAndClearControl_nothrow( m_xControl );
        }

        SdrUnoObj* pUnoObject( NULL );
        if ( !getUnoObject( pUnoObject ) )
            return false;

        Reference< XControl >   xControl;
        if ( !createControlForDevice( _rPageView, _rDevice, *pUnoObject, xControl ) )
            return false;

        // listen for changes in the control container

        m_pOutputDeviceForWindow = &_rDevice;
        m_xControl = xControl;
        m_xContainer = m_xContainer.query( _rPageView.getControlContainer( _rDevice ) );
        DBG_ASSERT( (   m_xContainer.is()                                           // either have a XControlContainer
                    ||  (   ( !_rPageView.getControlContainer( _rDevice ).is() )    // or don't have any container,
                        &&  ( dynamic_cast< const Window* >( &_rDevice ) == NULL )  // which is allowed for non-Window instances only
                        )
                    ),
            "ViewObjectContactOfUnoControl_Impl::impl_ensureControl_nothrow: no XContainer at the ControlContainer!" );

        try
        {
            m_eControlDesignMode = m_xControl->isDesignMode() ? eDesign : eAlive;

            Reference< XWindow2 > xControlWindow( m_xControl, UNO_QUERY_THROW );
            m_bControlIsVisible = xControlWindow->isVisible();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // start listening at all aspects of the control which are interesting to us ...
        impl_switchControlListening_nothrow( true );

        // start listening at the control container, in case somebody tampers with our control
        if ( m_xContainer.is() )
            impl_switchContainerListening_nothrow( true );

        return m_xControl.is();
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::createControlForDevice( IPageViewAccess& _rPageView,
        const OutputDevice& _rDevice, const SdrUnoObj& _rUnoObject, Reference< XControl >& _out_rControl )
    {
        _out_rControl = NULL;

        Reference< XControlModel > xControlModel( _rUnoObject.GetUnoControlModel() );
        DBG_ASSERT( xControlModel.is(), "ViewObjectContactOfUnoControl_Impl::createControlForDevice: no control model at the SdrUnoObject!?" );
        if ( !xControlModel.is() )
            return false;

        bool bSuccess = false;
        try
        {
            do
            {
                const ::rtl::OUString sControlServiceName( _rUnoObject.GetUnoControlTypeName() );

                Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
                if( xFactory.is() )
                    _out_rControl = _out_rControl.query( xFactory->createInstance( sControlServiceName ) );
                DBG_ASSERT( _out_rControl.is(), "ViewObjectContactOfUnoControl_Impl::createControlForDevice: no control could be created!" );
                if ( !_out_rControl.is() )
                    break;

                // knit the model and the control
                _out_rControl->setModel( xControlModel );

                UnoControlContactHelper::positionControl_throw( _out_rControl, _rUnoObject.GetLogicRect(), &_rDevice );

                // proper zoom
                UnoControlContactHelper::setControlZoom( _out_rControl, &_rDevice );

                // #107049# set design mode before peer is created,
                // this is also needed for accessibility
                _out_rControl->setDesignMode( _rPageView.isDesignMode() );

                // adjust the initial visibility according to the visibility of the layer
                // 2003-06-03 - #110592# - fs@openoffice.org
                impl_adjustControlVisibilityToLayerVisibility_throw( _out_rControl, _rUnoObject, _rPageView, false, true );

                // add the control to the respective control container
                // #108327# do this last
                Reference< XControlContainer > xControlContainer( _rPageView.getControlContainer( _rDevice ) );
                if ( xControlContainer.is() )
                    xControlContainer->addControl( sControlServiceName, _out_rControl );

                bSuccess = true;
            }
            while ( false );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !bSuccess )
        {
            // delete the control which might have been created already
            UnoControlContactHelper::disposeAndClearControl_nothrow( _out_rControl );
        }

        return _out_rControl.is();
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::impl_getPageView_nothrow( SdrPageView*& _out_rpPageView )
    {
        OSL_PRECOND( !impl_isDisposed_nofail(), "ViewObjectContactOfUnoControl_Impl::impl_getPageView_nothrow: already disposed!" );

        _out_rpPageView = NULL;
        if ( impl_isDisposed_nofail() )
            return false;

        ObjectContactOfPageView* pPageViewContact = dynamic_cast< ObjectContactOfPageView* >( &m_pAntiImpl->GetObjectContact() );
        if ( pPageViewContact )
            _out_rpPageView = &pPageViewContact->GetPageWindow().GetPageView();

        DBG_ASSERT( _out_rpPageView != NULL, "ViewObjectContactOfUnoControl_Impl::impl_getPageView_nothrow: this method is expected to always have success!" );
        return ( _out_rpPageView != NULL );
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::impl_adjustControlVisibilityToLayerVisibility_throw( bool _bForce )
    {
        OSL_PRECOND( m_xControl.is(),
            "ViewObjectContactOfUnoControl_Impl::impl_adjustControlVisibilityToLayerVisibility_throw: only valid if we have a control!" );

        SdrPageView* pPageView( NULL );
        if ( !impl_getPageView_nothrow( pPageView ) )
            return;

        SdrUnoObj* pUnoObject( NULL );
        if ( !getUnoObject( pUnoObject ) )
            return;

        SdrPageViewAccess aPVAccess( *pPageView );
        impl_adjustControlVisibilityToLayerVisibility_throw( m_xControl, *pUnoObject, aPVAccess, impl_isControlVisible_nofail(), _bForce );
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::impl_adjustControlVisibilityToLayerVisibility_throw( const Reference< XControl >& _rxControl,
        const SdrUnoObj& _rUnoObject, IPageViewAccess& _rPageView, bool _bIsCurrentlyVisible, bool _bForce )
    {
        // in design mode, there is no problem with the visibility: The XControl is hidden by
        // default, and the Drawing Layer will simply not call our paint routine, if we're in
        // a hidden layer. So, only alive mode matters.
        if ( !_rxControl->isDesignMode() )
        {
            // the layer of our object
            SdrLayerID nObjectLayer = _rUnoObject.GetLayer();
            // is the layer we're residing in visible in this view?
            bool bIsObjectLayerVisible = _rPageView.isLayerVisible( nObjectLayer );

            if ( _bForce || ( bIsObjectLayerVisible != _bIsCurrentlyVisible ) )
            {
                Reference< XWindow2 > xControlWindow( _rxControl, UNO_QUERY_THROW );
                xControlWindow->setVisible( bIsObjectLayerVisible );
            }
        }
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::impl_switchContainerListening_nothrow( bool _bStart )
    {
        OSL_PRECOND( m_xContainer.is(), "ViewObjectContactOfUnoControl_Impl::impl_switchContainerListening_nothrow: no control container!" );
        if ( !m_xContainer.is() )
            return;

        try
        {
            if ( _bStart )
                m_xContainer->addContainerListener( this );
            else
                m_xContainer->removeContainerListener( this );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ViewObjectContactOfUnoControl_Impl::impl_switchContainerListening_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::impl_switchControlListening_nothrow( bool _bStart )
    {
        try
        {
            Reference< XWindow > xWindow( m_xControl, UNO_QUERY_THROW );

            // listen for visibility changes
            if ( _bStart )
                xWindow->addWindowListener( this );
            else
                xWindow->removeWindowListener( this );

            // in design mode, listen for some more aspects
            impl_switchDesignModeListening_nothrow( impl_isControlDesignMode_nothrow() && _bStart );

            // listen for design mode changes
            Reference< XModeChangeBroadcaster > xDesignModeChanges( m_xControl, UNO_QUERY_THROW );
            if ( _bStart )
                xDesignModeChanges->addModeChangeListener( this );
            else
                xDesignModeChanges->removeModeChangeListener( this );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ViewObjectContactOfUnoControl_Impl::impl_switchControlListening_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::impl_switchDesignModeListening_nothrow( bool _bStart )
    {
        if ( impl_isDesignModeListening_nofail() != _bStart )
        {
            m_bIsDesignModeListening = _bStart;
            impl_switchPropertyListening_nothrow( _bStart );
        }
    }

    //------------------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::impl_switchPropertyListening_nothrow( bool _bStart )
    {
        OSL_PRECOND( m_xControl.is(), "ViewObjectContactOfUnoControl_Impl::impl_switchPropertyListening_nothrow: no control!" );
        if ( !m_xControl.is() )
            return;

        try
        {
            Reference< XPropertySet > xModelProperties( m_xControl->getModel(), UNO_QUERY_THROW );

            ::rtl::OUString sPropertyToListenFor;
            if ( _bStart )
                xModelProperties->addPropertyChangeListener( sPropertyToListenFor, this );
            else
                xModelProperties->removePropertyChangeListener( sPropertyToListenFor, this );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ViewObjectContactOfUnoControl_Impl::impl_switchPropertyListening_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::isPrintableControl() const
    {
        if ( !m_xControl.is() )
            return false;

        bool bIsPrintable = false;
        try
        {
            Reference< XPropertySet > xModelProperties( m_xControl->getModel(), UNO_QUERY );
            Reference< XPropertySetInfo > xPropertyInfo( xModelProperties.is() ? xModelProperties->getPropertySetInfo() : Reference< XPropertySetInfo >() );
            const ::rtl::OUString sPrintablePropertyName( RTL_CONSTASCII_USTRINGPARAM( "Printable" ) );

            if ( xPropertyInfo.is() && xPropertyInfo->hasPropertyByName( sPrintablePropertyName ) )
                OSL_VERIFY( xModelProperties->getPropertyValue( sPrintablePropertyName ) >>= bIsPrintable );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bIsPrintable;
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::preparePrintOrPrintPreview( OutputDevice& _rDevice ) const
    {
        try
        {
            if ( isPrintableControl() )
                return preparePaintOnDevice( _rDevice );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::preparePaintOnDevice( OutputDevice& _rDevice ) const
    {
        OSL_PRECOND( m_xControl.is(), "ViewObjectContactOfUnoControl_Impl::preparePaintOnDevice: no control!" );
        try
        {
            Reference< XGraphics > xGraphics( _rDevice.CreateUnoGraphics() );
            Reference< XView > xControlView( m_xControl, UNO_QUERY_THROW );
            xControlView->setGraphics( xGraphics );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::paintControl( const DisplayInfo& _rDisplayInfo ) const
    {
        OSL_PRECOND( _rDisplayInfo.GetOutputDevice() && m_xControl.is(), "ViewObjectContactOfUnoControl_Impl::paintControl: no output device or no control!" );
        if ( !_rDisplayInfo.GetOutputDevice() || !m_xControl.is() )
            return;

        SdrUnoObj* pUnoObject( NULL );
        if ( !getUnoObject( pUnoObject ) )
            return;

        const Rectangle& rPaintRect( pUnoObject->GetLogicRect() );

        SdrPageView* pPageView = _rDisplayInfo.GetPageView();
        if ( pPageView && pPageView->GetView().IsFillDraft() )
        {
            XOutputDevice* pExtendedDevice( _rDisplayInfo.GetExtendedOutputDevice() );
            if ( pExtendedDevice )
            {
                const SfxItemSet& rSet = pUnoObject->GetProperties().GetObjectItemSet();

                // perepare ItemSet to avoid old XOut filling
                SfxItemSet aFillDescriptor( *rSet.GetPool() );
                aFillDescriptor.Put( XFillStyleItem( XFILL_NONE ) );
                pExtendedDevice->SetFillAttr( aFillDescriptor );
                pExtendedDevice->SetLineAttr( rSet );

                pExtendedDevice->DrawRect( rPaintRect );
            }
        }
        else
        {
            UnoControlContactHelper::drawControl( m_xControl, rPaintRect.TopLeft(), _rDisplayInfo.GetOutputDevice() );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::disposing( const EventObject& Source ) throw(RuntimeException)
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            // some code below - in particular our disposal - might trigger actions which require the
            // SolarMutex. In particular, in our disposal, we remove ourself as listener from the control,
            // which alone needs the SolarMutex. Of course this - a removeFooListener needed the SolarMutex -
            // is the real bug. Toolkit really is infested with solar mutex usage ... :(
            // #i82169# / 2007-11-14 / frank.schoenheit@sun.com
        VOCGuard aGuard( *this );

        if ( !m_xControl.is() )
            return;

        if  (   ( Source.Source == m_xControl )
            ||  ( Source.Source == m_xControl->getModel() )
            )
        {
            // the model or the control is dying ... hmm, not much sense in that we ourself continue
            // living
            impl_dispose_nothrow( false );
            return;
        }

        DBG_ASSERT( Source.Source == m_xContainer, "ViewObjectContactOfUnoControl_Impl::disposing: Who's this?" );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::windowResized( const WindowEvent& /*e*/ ) throw(RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::windowMoved( const WindowEvent& /*e*/ ) throw(RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::windowShown( const EventObject& /*e*/ ) throw(RuntimeException)
    {
        VOCGuard aGuard( *this );
        m_bControlIsVisible = true;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::windowHidden( const EventObject& /*e*/ ) throw(RuntimeException)
    {
        VOCGuard aGuard( *this );
        m_bControlIsVisible = false;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::propertyChange( const PropertyChangeEvent& /*_rEvent*/ ) throw(RuntimeException)
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            // (re)painting might require VCL operations, which need the SolarMutex

        OSL_PRECOND( !impl_isDisposed_nofail(), "ViewObjectContactOfUnoControl_Impl::propertyChange: already disposed()" );
        if ( impl_isDisposed_nofail() )
            return;

        VOCGuard aGuard( *this );
        DBG_ASSERT( m_xControl.is(), "ViewObjectContactOfUnoControl_Impl::propertyChange: " );
        if ( !m_xControl.is() )
            return;

        // a generic property changed. If we're in design mode, we need to repaint the control
        if ( impl_isControlDesignMode_nothrow() )
            m_pAntiImpl->GetObjectContact().InvalidatePartOfView( m_pAntiImpl->GetViewContact().GetPaintRectangle() );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::modeChanged( const ModeChangeEvent& _rSource ) throw (RuntimeException)
    {
        VOCGuard aGuard( *this );

        DBG_ASSERT( _rSource.NewMode.equalsAscii( "design" ) || _rSource.NewMode.equalsAscii( "alive" ),
            "ViewObjectContactOfUnoControl_Impl::modeChanged: unexpected mode!" );

        m_eControlDesignMode = _rSource.NewMode.equalsAscii( "design" ) ? eDesign : eAlive;

        impl_switchDesignModeListening_nothrow( impl_isControlDesignMode_nothrow() );

        try
        {
            // if the control is part of a invisible layer, we need to explicitly hide it in alive mode
            // 2003-06-03 - #110592# - fs@openoffice.org
            impl_adjustControlVisibilityToLayerVisibility_throw( false );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ViewObjectContactOfUnoControl_Impl::modeChanged: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::elementInserted( const ContainerEvent& /*_Event*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::elementRemoved( const ContainerEvent& Event ) throw (RuntimeException)
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            // some code below - in particular our disposal - might trigger actions which require the
            // SolarMutex. In particular, in our disposal, we remove ourself as listener from the control,
            // which alone needs the SolarMutex. Of course this - a removeFooListener needed the SolarMutex -
            // is the real bug. Toolkit really is infested with solar mutex usage ... :(
            // #i82169# / 2007-11-14 / frank.schoenheit@sun.com
        VOCGuard aGuard( *this );
        DBG_ASSERT( Event.Source == m_xContainer, "ViewObjectContactOfUnoControl_Impl::elementRemoved: where did this come from?" );

        Reference< XControl > xRemovedControl( Event.Element, UNO_QUERY );
        DBG_ASSERT( xRemovedControl.is(), "ViewObjectContactOfUnoControl_Impl::elementRemoved: invalid Element!" );
        if ( !xRemovedControl.is() )
            return;

        if ( xRemovedControl.get() == m_xControl.get() )
            impl_dispose_nothrow( false );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ViewObjectContactOfUnoControl_Impl::elementReplaced( const ContainerEvent& Event ) throw (RuntimeException)
    {
        VOCGuard aGuard( *this );
        DBG_ASSERT( Event.Source == m_xContainer, "ViewObjectContactOfUnoControl_Impl::elementReplaced: where did this come from?" );

        Reference< XControl > xReplacedControl( Event.ReplacedElement, UNO_QUERY );
        DBG_ASSERT( xReplacedControl.is(), "ViewObjectContactOfUnoControl_Impl::elementReplaced: invalid ReplacedElement!" );
        if ( !xReplacedControl.is() )
            return;

        if ( xReplacedControl.get() == m_xControl.get() )
        {
            Reference< XControl > xNewControl( Event.Element, UNO_QUERY );
            DBG_ASSERT( xNewControl.is(), "ViewObjectContactOfUnoControl_Impl::elementReplaced: invalid new control!" );
            if ( !xNewControl.is() )
                return;

            DBG_ASSERT( xNewControl->getModel() == m_xControl->getModel(), "ViewObjectContactOfUnoControl_Impl::elementReplaced: another model at the new control?" );
            // another model should - in the drawing layer - also imply another SdrUnoObj, which
            // should also result in new ViewContact, and thus in new ViewObjectContacts

            impl_switchControlListening_nothrow( false );

            m_xControl = xNewControl;
            positionAndZoomControl();
            m_xControl->setDesignMode( impl_isControlDesignMode_nothrow() );

            Reference< XWindow2 > xControlWindow( m_xControl, UNO_QUERY );
            DBG_ASSERT( xControlWindow.is(), "ViewObjectContactOfUnoControl_Impl::elementReplaced: no XWindow2 at the new control!" );
            if ( xControlWindow.is() )
                m_bControlIsVisible = xControlWindow->isVisible();

            impl_switchControlListening_nothrow( true );
        }
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl_Impl::setControlDesignMode( bool _bDesignMode ) const
    {
        if ( ( m_eControlDesignMode != eUnknown ) && ( _bDesignMode == impl_isControlDesignMode_nothrow() ) )
            // nothing to do
            return;
        m_eControlDesignMode = _bDesignMode ? eDesign : eAlive;

        if ( !m_xControl.is() )
            // nothing to do, the setting will be respected as soon as the control
            // is created
            return;

        try
        {
            m_xControl->setDesignMode( _bDesignMode );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ViewObjectContactOfUnoControl_Impl::setControlDesignMode: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl_Impl::belongsToDevice( const OutputDevice* _pDevice ) const
    {
        DBG_ASSERT( _pDevice, "ViewObjectContactOfUnoControl_Impl::belongsToDevice: invalid device!" );

        OSL_PRECOND( !impl_isDisposed_nofail(), "ViewObjectContactOfUnoControl_Impl::belongsToDevice: already disposed!" );
        if ( impl_isDisposed_nofail() )
            return false;

        if ( m_pOutputDeviceForWindow )
        {
            if ( _pDevice == m_pOutputDeviceForWindow )
                return true;
            return false;
        }

        ObjectContactOfPageView* pPageViewContact = dynamic_cast< ObjectContactOfPageView* >( &m_pAntiImpl->GetObjectContact() );
        if ( pPageViewContact )
            return ( _pDevice == &imp_getPageViewDevice_nothrow( *pPageViewContact ) );

        DBG_ERROR( "ViewObjectContactOfUnoControl_Impl::belongsToDevice: could not determine the device I belong to!" );
        return false;
    }

    //====================================================================
    //= PaintLock
    //====================================================================
    class SVX_DLLPRIVATE PaintLock
    {
    private:
        ViewObjectContactOfUnoControl_Impl&  m_rVOC;

    public:
        inline PaintLock( ViewObjectContactOfUnoControl_Impl& _rVOC )
            :m_rVOC( _rVOC )
        {
            m_rVOC.enterPaint( ViewObjectContactOfUnoControl_Impl::PaintLockAccessControl( ) );
        }
        inline ~PaintLock( )
        {
            m_rVOC.leavePaint( ViewObjectContactOfUnoControl_Impl::PaintLockAccessControl( ) );
        }
    };

    //====================================================================
    //= ViewObjectContactOfUnoControl
    //====================================================================
    DBG_NAME( ViewObjectContactOfUnoControl )
    //--------------------------------------------------------------------
    ViewObjectContactOfUnoControl::ViewObjectContactOfUnoControl( ObjectContact& _rObjectContact, ViewContactOfUnoControl& _rViewContact )
        :ViewObjectContact( _rObjectContact, _rViewContact )
        ,m_pImpl( new ViewObjectContactOfUnoControl_Impl( this ) )
    {
        DBG_CTOR( ViewObjectContactOfUnoControl, NULL );
    }

    //--------------------------------------------------------------------
    ViewObjectContactOfUnoControl::~ViewObjectContactOfUnoControl()
    {
        m_pImpl->dispose();
        m_pImpl = NULL;

        DBG_DTOR( ViewObjectContactOfUnoControl, NULL );
    }

    //--------------------------------------------------------------------
    Reference< XControl > ViewObjectContactOfUnoControl::getExistentControl() const
    {
        VOCGuard aGuard( *m_pImpl );
        return m_pImpl->getExistentControl();
    }

    //--------------------------------------------------------------------
    Reference< XControl > ViewObjectContactOfUnoControl::getControl()
    {
        VOCGuard aGuard( *m_pImpl );
        m_pImpl->ensureControl();

        return m_pImpl->getExistentControl();
    }

    //--------------------------------------------------------------------
    Reference< XControl > ViewObjectContactOfUnoControl::getTemporaryControlForWindow(
        const Window& _rWindow, Reference< XControlContainer >& _inout_ControlContainer, const SdrUnoObj& _rUnoObject )
    {
        Reference< XControl > xControl;

        InvisibleControlViewAccess aSimulatePageView( _inout_ControlContainer );
        OSL_VERIFY( ViewObjectContactOfUnoControl_Impl::createControlForDevice( aSimulatePageView, _rWindow, _rUnoObject, xControl ) );
        return xControl;
    }

    //--------------------------------------------------------------------
    namespace
    {
        /** helper class to restore graphics at <awt::XView> object after painting

            OD 08.05.2003 #109432#
            Restoration of graphics necessary to assure that paint on a window

            @author OD
        */
        class RestoreViewGraphics
        {
            private:
                Reference< XView >        m_xView;
                Reference< XGraphics >    m_xGraphics;

            public:
                RestoreViewGraphics( const Reference< XView >& _rxView )
                {
                    m_xView = _rxView;
                    m_xGraphics = m_xView->getGraphics();
                }
                ~RestoreViewGraphics()
                {
                    m_xView->setGraphics( m_xGraphics );
                }
        };
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl::positionControlForPaint( DisplayInfo& _rDisplayInfo ) const
    {
        VOCGuard aGuard( *m_pImpl );

        // ensure we have a control. If we don't, then the Drawing Layer might be tempted to
        // never draw the complete form layer.
        // #i75095# / 2007-03-05 / frank.schoenheit@sun.com
        m_pImpl->ensureControl( _rDisplayInfo );

        // position the control
        m_pImpl->positionControlForPaint( _rDisplayInfo );
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl::PaintObject( DisplayInfo& _rDisplayInfo )
    {
        VOCGuard aGuard( *m_pImpl );

        SdrUnoObj* pObject( NULL );
        Reference< XView > xControlView;
        if ( !m_pImpl->initPaint( _rDisplayInfo, pObject, xControlView ) )
            return;

        try
        {
            RestoreViewGraphics aRestoreGraphics( xControlView );
            PaintLock aPaintLock( *m_pImpl );
            doPaintObject( _rDisplayInfo, pObject );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ViewObjectContactOfUnoControl::PaintObject: caught an exception while tampering with the view's XGraphics!" );
        }

        mbIsPainted = sal_True;
        maPaintedRectangle = pObject->GetLogicRect();
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl::ensureControlVisibility( bool _bVisible ) const
    {
        VOCGuard aGuard( *m_pImpl );

        try
        {
            Reference< XControl > xControl( m_pImpl->getExistentControl() );
            if ( !xControl.is() )
                return;

            // only need to care for alive mode
            if ( xControl->isDesignMode() )
                return;

            // is the visibility correct?
            if ( m_pImpl->isControlVisible() == _bVisible )
                return;

            // no -> adjust it
            Reference< XWindow > xControlWindow( xControl, UNO_QUERY_THROW );
            xControlWindow->setVisible( _bVisible );
            DBG_ASSERT( m_pImpl->isControlVisible() == _bVisible, "ViewObjectContactOfUnoControl::ensureControlVisibility: this didn't work!" );
                // now this would mean that either isControlVisible is not reliable,
                // or that showing/hiding the window did not work as intended.
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ViewObjectContactOfUnoControl::ensureControlVisibility: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void ViewObjectContactOfUnoControl::setControlDesignMode( bool _bDesignMode ) const
    {
        VOCGuard aGuard( *m_pImpl );
        m_pImpl->setControlDesignMode( _bDesignMode );
    }

    //--------------------------------------------------------------------
    bool ViewObjectContactOfUnoControl::belongsToDevice( const OutputDevice* _pDevice ) const
    {
        VOCGuard aGuard( *m_pImpl );
        return m_pImpl->belongsToDevice( _pDevice );
    }

    //====================================================================
    //= UnoControlDefaultContact
    //====================================================================
    DBG_NAME( UnoControlDefaultContact )
    //--------------------------------------------------------------------
    UnoControlDefaultContact::UnoControlDefaultContact( ObjectContact& _rObjectContact, ViewContactOfUnoControl& _rViewContact )
        :ViewObjectContactOfUnoControl( _rObjectContact, _rViewContact )
    {
        DBG_CTOR( UnoControlDefaultContact, NULL );
    }

    //--------------------------------------------------------------------
    UnoControlDefaultContact::~UnoControlDefaultContact()
    {
        DBG_DTOR( UnoControlDefaultContact, NULL );
    }

    //--------------------------------------------------------------------
    void UnoControlDefaultContact::doPaintObject( const DisplayInfo& _rDisplayInfo, const SdrUnoObj* /*_pUnoObject*/ ) const
    {
        DBG_ASSERT( _rDisplayInfo.GetOutputDevice(), "UnoControlDefaultContact::doPaintObject: no output device!" );
        if ( !_rDisplayInfo.GetOutputDevice() )
            throw RuntimeException();

        // set the graphics at the control
        if ( m_pImpl->preparePaintOnDevice( *_rDisplayInfo.GetOutputDevice() ) )
            m_pImpl->paintControl( _rDisplayInfo );
    }

    //====================================================================
    //= UnoControlWindowContact
    //====================================================================
    DBG_NAME( UnoControlWindowContact )
    //--------------------------------------------------------------------
    UnoControlWindowContact::UnoControlWindowContact( ObjectContactOfPageView& _rObjectContact, ViewContactOfUnoControl& _rViewContact )
        :ViewObjectContactOfUnoControl( _rObjectContact, _rViewContact )
    {
        DBG_CTOR( UnoControlWindowContact, NULL );
    }

    //--------------------------------------------------------------------
    UnoControlWindowContact::~UnoControlWindowContact()
    {
        DBG_DTOR( UnoControlWindowContact, NULL );
    }

    //--------------------------------------------------------------------
    void UnoControlWindowContact::doPaintObject( const DisplayInfo& _rDisplayInfo, const SdrUnoObj* /*_pUnoObject*/ ) const
    {
        OSL_PRECOND( m_pImpl->getExistentControl().is(),
            "UnoControlWindowContact::doPaintObject: the control was said to be non-NULL here!" );

        // don't paint if there's a "visible control" which paints itself
        bool bVisibleControl = false;
        try
        {
            Reference< XWindow2 > xControlWindow( m_pImpl->getExistentControl(), UNO_QUERY_THROW );
            bVisibleControl = xControlWindow->isVisible();
        }
        catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }


        // we need to paint if the control is not visible
        bool bNeedPaint = !bVisibleControl;

        // or if our control is visible, but the paint request is for a device other than the
        // control's parent window
        if ( !bNeedPaint )
        {
            try
            {
                Window* pControlWindow = VCLUnoHelper::GetWindow( m_pImpl->getExistentControl()->getPeer() );
                Window* pControlParentWindow = pControlWindow ? pControlWindow->GetParent() : NULL;
                bNeedPaint = pControlParentWindow != _rDisplayInfo.GetOutputDevice();
            }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
        }

        if ( bNeedPaint )
        {
            OSL_ENSURE( _rDisplayInfo.GetOutputDevice(), "UnoControlWindowContact::doPaintObject: invalid output device!" );
            if ( !_rDisplayInfo.GetOutputDevice() )
                throw RuntimeException();

            if ( m_pImpl->preparePaintOnDevice( *_rDisplayInfo.GetOutputDevice() ) )
                m_pImpl->paintControl( _rDisplayInfo );
        }

        if ( bVisibleControl )
        {
            try
            {
                Reference< XControl > xControl( m_pImpl->getExistentControl() );
                if ( xControl->isTransparent() )
                {
                    // TODO: isn't there a better way to do this? At the moment, it seems to be
                    // used to force the background of transparent controls to be repainted - it
                    // must be possible to do this more clever, using the drawing layer mechanisms.
                    Reference< XWindowPeer > xControlPeer( xControl->getPeer() );
                    if ( xControlPeer.is() )
                        xControlPeer->invalidate( INVALIDATE_NOTRANSPARENT | INVALIDATE_CHILDREN );
                }
            }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
        }
    }

    //====================================================================
    //= UnoControlPrintOrPreviewContact
    //====================================================================
    DBG_NAME( UnoControlPrintOrPreviewContact )
    //--------------------------------------------------------------------
    UnoControlPrintOrPreviewContact::UnoControlPrintOrPreviewContact( ObjectContactOfPageView& _rObjectContact, ViewContactOfUnoControl& _rViewContact )
        :ViewObjectContactOfUnoControl( _rObjectContact, _rViewContact )
    {
        DBG_CTOR( UnoControlPrintOrPreviewContact, NULL );
    }

    //--------------------------------------------------------------------
    UnoControlPrintOrPreviewContact::~UnoControlPrintOrPreviewContact()
    {
        DBG_DTOR( UnoControlPrintOrPreviewContact, NULL );
    }

    //--------------------------------------------------------------------
    void UnoControlPrintOrPreviewContact::doPaintObject( const DisplayInfo& _rDisplayInfo, const SdrUnoObj* /*_pUnoObject*/ ) const
    {
        OutputDevice* pDevice = _rDisplayInfo.GetOutputDevice();
        OSL_ENSURE( pDevice, "UnoControlPrintOrPreviewContact::doPaintObject: invalid device!" );
        if ( pDevice && m_pImpl->preparePrintOrPrintPreview( *pDevice ) )
            m_pImpl->paintControl( _rDisplayInfo );
    }

    //====================================================================
    //= UnoControlPDFExportContact
    //====================================================================
    DBG_NAME( UnoControlPDFExportContact )
    //--------------------------------------------------------------------
    UnoControlPDFExportContact::UnoControlPDFExportContact( ObjectContactOfPageView& _rObjectContact, ViewContactOfUnoControl& _rViewContact )
        :ViewObjectContactOfUnoControl( _rObjectContact, _rViewContact )
    {
        DBG_CTOR( UnoControlPDFExportContact, NULL );
    }

    //--------------------------------------------------------------------
    UnoControlPDFExportContact::~UnoControlPDFExportContact()
    {
        DBG_DTOR( UnoControlPDFExportContact, NULL );
    }

    //--------------------------------------------------------------------
    void UnoControlPDFExportContact::doPaintObject( const DisplayInfo& _rDisplayInfo, const SdrUnoObj* _pUnoObject ) const
    {
        OutputDevice* pDevice = _rDisplayInfo.GetOutputDevice();
        vcl::PDFExtOutDevData* pPDFExport = pDevice ? PTR_CAST( vcl::PDFExtOutDevData, pDevice->GetExtOutDevData() ) : NULL;
        DBG_ASSERT( pPDFExport, "UnoControlPDFExportContact::doPaintObject: this is no PDF export output device!" );
        if ( !pPDFExport )
            return;

        if ( !m_pImpl->isPrintableControl() )
            // controls declared as "do not print" are not exported at all - neither as
            // native PDF control, nor as normal drawing
            // 2006-11-22 / #i71370# / frank.schoenheit@sun.com
            return;

        if( pPDFExport->GetIsExportFormFields() )
        {
            ::std::auto_ptr< ::vcl::PDFWriter::AnyWidget > pPDFControl;
            ::svxform::describePDFControl( m_pImpl->getExistentControl(), pPDFControl );
            if ( pPDFControl.get() != NULL )
            {
                // still need to fill in the location
                pPDFControl->Location = _pUnoObject->GetLogicRect();

                Size aFontSize( pPDFControl->TextFont.GetSize() );
                aFontSize = pDevice->LogicToLogic( aFontSize, MapMode( MAP_POINT ), pDevice->GetMapMode() );
                pPDFControl->TextFont.SetSize( aFontSize );

                pPDFExport->BeginStructureElement( vcl::PDFWriter::Form );
                pPDFExport->CreateControl( *pPDFControl.get() );
                pPDFExport->EndStructureElement();
                return;
            }
        }

        if ( m_pImpl->preparePaintOnDevice( *pDevice ) )
            m_pImpl->paintControl( _rDisplayInfo );
    }

//........................................................................
} } // namespace sdr::contact
//........................................................................

