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
#include <cppuhelper/implbase.hxx>
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

typedef ::cppu::WeakAggImplHelper   <   css::awt::XControl
                                    ,   css::awt::XWindow2
                                    ,   css::awt::XView
                                    ,   css::beans::XPropertiesChangeListener
                                    ,   css::lang::XServiceInfo
                                    ,   css::accessibility::XAccessible
                                    ,   css::util::XModeChangeBroadcaster
                                    ,   css::awt::XUnitConversion
                                    ,   css::awt::XStyleSettingsSupplier
                                    >   UnoControl_Base;

class TOOLKIT_DLLPUBLIC UnoControl :    public UnoControl_Base
{
private:
    ::osl::Mutex    maMutex;

    css::uno::Reference< css::awt::XWindowPeer >      mxPeer;
    css::uno::Reference< css::awt::XVclWindowPeer >   mxVclWindowPeer; // just to avoid the query_interface thing

protected:
    EventListenerMultiplexer            maDisposeListeners;
    WindowListenerMultiplexer           maWindowListeners;
    FocusListenerMultiplexer            maFocusListeners;
    KeyListenerMultiplexer              maKeyListeners;
    MouseListenerMultiplexer            maMouseListeners;
    MouseMotionListenerMultiplexer      maMouseMotionListeners;
    PaintListenerMultiplexer            maPaintListeners;
    ::comphelper::OInterfaceContainerHelper2   maModeChangeListeners;

    css::uno::Reference< css::uno::XInterface >       mxContext;
    css::uno::Reference< css::awt::XControlModel >    mxModel;
    css::uno::Reference< css::awt::XGraphics >        mxGraphics;

    css::uno::WeakReferenceHelper
                                        maAccessibleContext;    /// our most recent XAccessibleContext instance

    bool                            mbDisposePeer;
    bool                            mbRefeshingPeer;
    bool                            mbCreatingPeer;
    bool                            mbCreatingCompatiblePeer;
    bool                            mbDesignMode;

    UnoControlComponentInfos            maComponentInfos;
    UnoControl_Data*                    mpData;

    ::osl::Mutex&                                                               GetMutex() { return maMutex; }

    css::uno::Reference< css::awt::XWindow >          getParentPeer() const;
    virtual void                                                                updateFromModel();
    void                                                                        peerCreated();
    bool                                                                        ImplCheckLocalize( OUString& _rPossiblyLocalizable );
    css::uno::Reference< css::awt::XWindowPeer >      ImplGetCompatiblePeer();
    virtual void                                                                ImplSetPeerProperty( const OUString& rPropName, const css::uno::Any& rVal );
    virtual void                                                                PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc );
    virtual void                                                                ImplModelPropertiesChanged( const css::uno::Sequence< css::beans::PropertyChangeEvent >& rEvents );

    void                                                                        ImplLockPropertyChangeNotification( const OUString& rPropertyName, bool bLock );
    void                                                                        ImplLockPropertyChangeNotifications( const css::uno::Sequence< OUString >& rPropertyNames, bool bLock );

    void DisposeAccessibleContext(css::uno::Reference<
            css::lang::XComponent> const& xContext);

    inline void setPeer( const css::uno::Reference< css::awt::XWindowPeer >& _xPeer)
    {
        mxPeer = _xPeer;
        mxVclWindowPeer = css::uno::Reference< css::awt::XVclWindowPeer >(mxPeer,css::uno::UNO_QUERY); // just to avoid the query_interface thing
    }

    virtual bool   requiresNewPeer( const OUString& _rPropertyName ) const;

public:
                UnoControl();
                virtual ~UnoControl() override;

                UnoControlComponentInfos&           GetComponentInfos() { return maComponentInfos; }


    virtual OUString GetComponentServiceName();

    // css::lang::XTypeProvider
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception) override;

    // XEventListener
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XWindow2
    virtual void SAL_CALL setOutputSize( const css::awt::Size& aSize ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getOutputSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isVisible(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isActive(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isEnabled(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasFocus(  ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XWindow
    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Rectangle SAL_CALL getPosSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setVisible( sal_Bool Visible ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setEnable( sal_Bool Enable ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setFocus(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XView
    sal_Bool SAL_CALL setGraphics( const css::uno::Reference< css::awt::XGraphics >& aDevice ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::awt::XGraphics > SAL_CALL getGraphics(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setZoom( float fZoomX, float fZoomY ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XControl
    void SAL_CALL setContext( const css::uno::Reference< css::uno::XInterface >& Context ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::uno::XInterface > SAL_CALL getContext(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::awt::XWindowPeer > SAL_CALL getPeer(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& Model ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::awt::XView > SAL_CALL getView(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isDesignMode(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isTransparent(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XModeChangeBroadcaster
    virtual void SAL_CALL addModeChangeListener( const css::uno::Reference< css::util::XModeChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModeChangeListener( const css::uno::Reference< css::util::XModeChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addModeChangeApproveListener( const css::uno::Reference< css::util::XModeChangeApproveListener >& _rxListener ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModeChangeApproveListener( const css::uno::Reference< css::util::XModeChangeApproveListener >& _rxListener ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // XUnitConversion
    virtual css::awt::Point SAL_CALL convertPointToLogic( const css::awt::Point& Point, ::sal_Int16 TargetUnit ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL convertPointToPixel( const css::awt::Point& aPoint, ::sal_Int16 SourceUnit ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL convertSizeToLogic( const css::awt::Size& aSize, ::sal_Int16 TargetUnit ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL convertSizeToPixel( const css::awt::Size& aSize, ::sal_Int16 SourceUnit ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XStyleSettingsSupplier
    virtual css::uno::Reference< css::awt::XStyleSettings > SAL_CALL getStyleSettings() throw (css::uno::RuntimeException, std::exception) override;

private:
    // css::beans::XPropertiesChangeListener
    void SAL_CALL propertiesChange( const css::uno::Sequence< css::beans::PropertyChangeEvent >& evt ) throw(css::uno::RuntimeException, std::exception) override;
};


#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
