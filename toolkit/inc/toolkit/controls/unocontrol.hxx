/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#define _TOOLKIT_CONTROLS_UNOCONTROL_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>
#include <com/sun/star/awt/XStyleSettingsSupplier.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <cppuhelper/weakagg.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase9.hxx>
#include <comphelper/componentcontext.hxx>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>


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

struct UnoControl_Data;
//  ----------------------------------------------------
//  class UnoControl
//  ----------------------------------------------------
typedef ::cppu::WeakAggImplHelper9  <   ::com::sun::star::awt::XControl
                                    ,   ::com::sun::star::awt::XWindow2
                                    ,   ::com::sun::star::awt::XView
                                    ,   ::com::sun::star::beans::XPropertiesChangeListener
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::accessibility::XAccessible
                                    ,   ::com::sun::star::util::XModeChangeBroadcaster
                                    ,   ::com::sun::star::awt::XUnitConversion
                                    ,   ::com::sun::star::awt::XStyleSettingsSupplier
                                    >   UnoControl_Base;

class TOOLKIT_DLLPUBLIC UnoControl :    public UnoControl_Base
{
private:
    ::osl::Mutex    maMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >      mxPeer;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >   mxVclWindowPeer; // just to avoid the query_interface thing

protected:
    const ::comphelper::ComponentContext    maContext;
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
    UnoControl_Data*                    mpData;

    ::osl::Mutex&                                                               GetMutex() { return maMutex; }

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >          getParentPeer() const;
    virtual void                                                                updateFromModel();
    void                                                                        peerCreated();
    bool                                                                        ImplCheckLocalize( ::rtl::OUString& _rPossiblyLocalizable );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >      ImplGetCompatiblePeer( sal_Bool bAcceptExistingPeer );
    virtual void                                                                ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );
    virtual void                                                                PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );
    virtual void                                                                ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents );

    void                                                                        ImplLockPropertyChangeNotification( const ::rtl::OUString& rPropertyName, bool bLock );
    void                                                                        ImplLockPropertyChangeNotifications( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rPropertyNames, bool bLock );

    void                                                                        disposeAccessibleContext();

    inline void setPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _xPeer)
    {
        mxPeer = _xPeer;
        mxVclWindowPeer = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >(mxPeer,::com::sun::star::uno::UNO_QUERY); // just to avoid the query_interface thing
    }

    virtual sal_Bool   requiresNewPeer( const ::rtl::OUString& _rPropertyName ) const;

protected:
                UnoControl();

public:
                UnoControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
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

    // XUnitConversion
    virtual ::com::sun::star::awt::Point SAL_CALL convertPointToLogic( const ::com::sun::star::awt::Point& Point, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL convertPointToPixel( const ::com::sun::star::awt::Point& aPoint, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL convertSizeToLogic( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL convertSizeToPixel( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XStyleSettingsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XStyleSettings > SAL_CALL getStyleSettings() throw (::com::sun::star::uno::RuntimeException);

private:
    // ::com::sun::star::beans::XPropertiesChangeListener
    void SAL_CALL propertiesChange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt ) throw(::com::sun::star::uno::RuntimeException);
};




#endif // _TOOLKIT_CONTROLS_UNOCONTROL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
