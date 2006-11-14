/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unocontrol.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:28:00 $
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

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#define _TOOLKIT_CONTROLS_UNOCONTROL_HXX_

#ifndef TOOLKIT_DLLAPI_H
#include <toolkit/dllapi.h>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW2_HPP_
#include <com/sun/star/awt/XWindow2.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XView_HPP_
#include <com/sun/star/awt/XView.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _TOOLKIT_HELPER_MUTEXANDBROADCASTHELPER_HXX_
#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_
#include <toolkit/helper/listenermultiplexer.hxx>
#endif

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/implbase7.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODECHANGEBROADCASTER_HPP_
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif


struct UnoControlComponentInfos
{
    sal_Bool    bVisible;
    sal_Bool    bEnable;
    long        nX, nY, nWidth, nHeight;
    sal_uInt16  nFlags;
    float       nZoomX, nZoomY;

    UnoControlComponentInfos()
    {
        bVisible = sal_True;
        bEnable = sal_True;
        nX = nY = nWidth = nHeight = 0;
        nFlags = 0; // POSSIZE_POSSIZE;
        nZoomX = nZoomY = 1.0f;
    }
};

//  ----------------------------------------------------
//  class UnoControl
//  ----------------------------------------------------
typedef ::cppu::WeakAggImplHelper7  <   ::com::sun::star::awt::XControl
                                    ,   ::com::sun::star::awt::XWindow2
                                    ,   ::com::sun::star::awt::XView
                                    ,   ::com::sun::star::beans::XPropertiesChangeListener
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::accessibility::XAccessible
                                    ,   ::com::sun::star::util::XModeChangeBroadcaster
                                    >   UnoControl_Base;

class TOOLKIT_DLLPUBLIC UnoControl :    public UnoControl_Base
{
private:
    ::osl::Mutex    maMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >      mxPeer;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >   mxVclWindowPeer; // just to avoid the query_interface thing

protected:
    EventListenerMultiplexer            maDisposeListeners;
    WindowListenerMultiplexer           maWindowListeners;
    FocusListenerMultiplexer            maFocusListeners;
    KeyListenerMultiplexer              maKeyListeners;
    MouseListenerMultiplexer            maMouseListeners;
    MouseMotionListenerMultiplexer      maMouseMotionListeners;
    PaintListenerMultiplexer            maPaintListeners;
    ::cppu::OInterfaceContainerHelper   maModeChangeListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >       mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >    mxModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >        mxGraphics;

    ::com::sun::star::uno::WeakReferenceHelper
                                        maAccessibleContext;    /// our most recent XAccessibleContext instance

    sal_Bool                            mbDisposePeer;
    sal_Bool                            mbRefeshingPeer;
    sal_Bool                            mbCreatingPeer;
    sal_Bool                            mbCreatingCompatiblePeer;
    sal_Bool                            mbDesignMode;

    UnoControlComponentInfos            maComponentInfos;
    ::rtl::OUString                     msPropertyCurrentlyUpdating;

    ::osl::Mutex&                                                               GetMutex() { return maMutex; }

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >          getParentPeer() const;
    void                                                                        updateFromModel();
    void                                                                        peerCreated();
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >      ImplGetCompatiblePeer( sal_Bool bAcceptExistingPeer );
    virtual void                                                                ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );
    virtual void                                                                PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );
    virtual void                                                                ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents );

    void                                                                        disposeAccessibleContext();

    inline void setPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _xPeer)
    {
        mxPeer = _xPeer;
        mxVclWindowPeer = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >(mxPeer,::com::sun::star::uno::UNO_QUERY); // just to avoid the query_interface thing
    }

    virtual sal_Bool   requiresNewPeer( const ::rtl::OUString& _rPropertyName ) const;

public:
                UnoControl();
                ~UnoControl();

                UnoControlComponentInfos&           GetComponentInfos() { return maComponentInfos; }


    virtual ::rtl::OUString GetComponentServiceName();

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindow2
    virtual void SAL_CALL setOutputSize( const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getOutputSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isVisible(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isActive(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isEnabled(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasFocus(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setVisible( sal_Bool Visible ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setEnable( sal_Bool Enable ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFocus(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XView
    sal_Bool SAL_CALL setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& aDevice ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setZoom( float fZoomX, float fZoomY ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    void SAL_CALL setContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Context ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getContext(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XModeChangeBroadcaster
    virtual void SAL_CALL addModeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addModeChangeApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeApproveListener >& _rxListener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModeChangeApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeApproveListener >& _rxListener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

private:
    // ::com::sun::star::beans::XPropertiesChangeListener
    void SAL_CALL propertiesChange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt ) throw(::com::sun::star::uno::RuntimeException);
};




#endif // _TOOLKIT_CONTROLS_UNOCONTROL_HXX_

