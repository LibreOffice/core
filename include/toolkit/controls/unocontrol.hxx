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

#ifndef INCLUDED_TOOLKIT_CONTROLS_UNOCONTROL_HXX
#define INCLUDED_TOOLKIT_CONTROLS_UNOCONTROL_HXX

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
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>


struct UnoControlComponentInfos
{
    bool    bVisible;
    bool    bEnable;
    long        nX, nY, nWidth, nHeight;
    sal_uInt16  nFlags;
    float       nZoomX, nZoomY;

    UnoControlComponentInfos()
    {
        bVisible = true;
        bEnable = true;
        nX = nY = nWidth = nHeight = 0;
        nFlags = 0; // POSSIZE_POSSIZE;
        nZoomX = nZoomY = 1.0f;
    }
};

struct UnoControl_Data;

//  class UnoControl

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

    bool                            mbDisposePeer;
    bool                            mbRefeshingPeer;
    bool                            mbCreatingPeer;
    bool                            mbCreatingCompatiblePeer;
    bool                            mbDesignMode;

    UnoControlComponentInfos            maComponentInfos;
    UnoControl_Data*                    mpData;

    ::osl::Mutex&                                                               GetMutex() { return maMutex; }

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >          getParentPeer() const;
    virtual void                                                                updateFromModel();
    void                                                                        peerCreated();
    bool                                                                        ImplCheckLocalize( OUString& _rPossiblyLocalizable );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >      ImplGetCompatiblePeer( bool bAcceptExistingPeer );
    virtual void                                                                ImplSetPeerProperty( const OUString& rPropName, const ::com::sun::star::uno::Any& rVal );
    virtual void                                                                PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );
    virtual void                                                                ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents );

    void                                                                        ImplLockPropertyChangeNotification( const OUString& rPropertyName, bool bLock );
    void                                                                        ImplLockPropertyChangeNotifications( const ::com::sun::star::uno::Sequence< OUString >& rPropertyNames, bool bLock );

    void DisposeAccessibleContext(::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> const& xContext);

    inline void setPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _xPeer)
    {
        mxPeer = _xPeer;
        mxVclWindowPeer = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >(mxPeer,::com::sun::star::uno::UNO_QUERY); // just to avoid the query_interface thing
    }

    virtual bool   requiresNewPeer( const OUString& _rPropertyName ) const;

public:
                UnoControl();
                virtual ~UnoControl();

                UnoControlComponentInfos&           GetComponentInfos() { return maComponentInfos; }


    virtual OUString GetComponentServiceName();

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XWindow2
    virtual void SAL_CALL setOutputSize( const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Size SAL_CALL getOutputSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isVisible(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isActive(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isEnabled(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setVisible( sal_Bool Visible ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setEnable( sal_Bool Enable ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setFocus(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XView
    sal_Bool SAL_CALL setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& aDevice ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setZoom( float fZoomX, float fZoomY ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XControl
    void SAL_CALL setContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Context ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getContext(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isTransparent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XModeChangeBroadcaster
    virtual void SAL_CALL addModeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModeChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addModeChangeApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeApproveListener >& _rxListener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModeChangeApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeChangeApproveListener >& _rxListener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XUnitConversion
    virtual ::com::sun::star::awt::Point SAL_CALL convertPointToLogic( const ::com::sun::star::awt::Point& Point, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Point SAL_CALL convertPointToPixel( const ::com::sun::star::awt::Point& aPoint, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Size SAL_CALL convertSizeToLogic( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Size SAL_CALL convertSizeToPixel( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XStyleSettingsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XStyleSettings > SAL_CALL getStyleSettings() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    // ::com::sun::star::beans::XPropertiesChangeListener
    void SAL_CALL propertiesChange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};




#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
