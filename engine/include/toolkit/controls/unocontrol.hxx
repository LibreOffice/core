/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>
#include <com/sun/star/awt/XStyleSettingsSupplier.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <osl/mutex.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <tools/long.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase9.hxx>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <memory>


struct UnoControlComponentInfos
{
    tools::Long nX, nY, nWidth, nHeight;
    float       nZoomX, nZoomY;
    sal_uInt16  nFlags;
    bool        bVisible;
    bool        bEnable;

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


typedef ::cppu::WeakAggImplHelper9  <   css::awt::XControl
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

    cpo::uno::Reference< css::awt::XVclWindowPeer >  mxVclWindowPeer;

protected:
    EventListenerMultiplexer            maDisposeListeners;
    WindowListenerMultiplexer           maWindowListeners;
    FocusListenerMultiplexer            maFocusListeners;
    KeyListenerMultiplexer              maKeyListeners;
    MouseListenerMultiplexer            maMouseListeners;
    MouseMotionListenerMultiplexer      maMouseMotionListeners;
    PaintListenerMultiplexer            maPaintListeners;
    ::comphelper::OInterfaceContainerHelper3<css::util::XModeChangeListener> maModeChangeListeners;

    cpo::uno::Reference< cpo::uno::XInterface >       mxContext;
    cpo::uno::Reference< css::awt::XControlModel >    mxModel;
    cpo::uno::Reference< css::awt::XGraphics >        mxGraphics;

    cpo::uno::WeakReferenceHelper   maAccessibleContext;    /// our most recent XAccessibleContext instance

    bool                            mbDisposePeer;
    bool                            mbRefreshingPeer;
    bool                            mbCreatingPeer;
    bool                            mbCreatingCompatiblePeer;
    bool                            mbDesignMode;

    UnoControlComponentInfos            maComponentInfos;
    std::unique_ptr<UnoControl_Data>    mpData;

    ::osl::Mutex&                                                               GetMutex() { return maMutex; }

    cpo::uno::Reference< css::awt::XWindow >          getParentPeer() const;
    virtual void                                                                updateFromModel();
    void                                                                        peerCreated();
    bool                                                                        ImplCheckLocalize( OUString& _rPossiblyLocalizable );
    cpo::uno::Reference< css::awt::XVclWindowPeer >      ImplGetCompatiblePeer();
    virtual void                                                                ImplSetPeerProperty( const OUString& rPropName, const cpo::uno::Any& rVal );
    virtual void                                                                PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc );
    virtual void                                                                ImplModelPropertiesChanged( const cpo::uno::Sequence< css::beans::PropertyChangeEvent >& rEvents );

    void                                                                        ImplLockPropertyChangeNotification( const OUString& rPropertyName, bool bLock );
    void                                                                        ImplLockPropertyChangeNotifications( const cpo::uno::Sequence< OUString >& rPropertyNames, bool bLock );

    void DisposeAccessibleContext(cpo::uno::Reference<
            css::lang::XComponent> const& xContext);

    void setPeer( const cpo::uno::Reference< css::awt::XVclWindowPeer >& _xPeer)
    {
        mxVclWindowPeer = _xPeer;
    }

    virtual bool   requiresNewPeer( const OUString& _rPropertyName ) const;

public:
                UnoControl();
                virtual ~UnoControl() override;

                UnoControlComponentInfos&           GetComponentInfos() { return maComponentInfos; }


    virtual OUString GetComponentServiceName() const;

    // css::lang::XTypeProvider
    cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

    // css::lang::XComponent
    void dispose(  ) override;
    void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XEventListener
    void disposing( const css::lang::EventObject& Source ) override;

    // css::awt::XWindow2
    virtual void setOutputSize( const css::awt::Size& aSize ) override;
    virtual css::awt::Size getOutputSize(  ) override;
    virtual bool isVisible(  ) override;
    virtual bool isActive(  ) override;
    virtual bool isEnabled(  ) override;
    virtual bool hasFocus(  ) override;

    // css::awt::XWindow
    void setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;
    css::awt::Rectangle getPosSize(  ) override;
    void setVisible( bool Visible ) override;
    void setEnable( bool Enable ) override;
    void setFocus(  ) override;
    void addWindowListener( const cpo::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    void removeWindowListener( const cpo::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    void addFocusListener( const cpo::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    void removeFocusListener( const cpo::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    void addKeyListener( const cpo::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    void removeKeyListener( const cpo::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    void addMouseListener( const cpo::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    void removeMouseListener( const cpo::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    void addMouseMotionListener( const cpo::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    void removeMouseMotionListener( const cpo::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    void addPaintListener( const cpo::uno::Reference< css::awt::XPaintListener >& xListener ) override;
    void removePaintListener( const cpo::uno::Reference< css::awt::XPaintListener >& xListener ) override;

    // css::awt::XView
    bool setGraphics( const cpo::uno::Reference< css::awt::XGraphics >& aDevice ) override;
    cpo::uno::Reference< css::awt::XGraphics > getGraphics(  ) override;
    css::awt::Size getSize(  ) override;
    void draw( sal_Int32 nX, sal_Int32 nY ) override;
    void setZoom( float fZoomX, float fZoomY ) override;

    // css::awt::XControl
    void setContext( const cpo::uno::Reference< cpo::uno::XInterface >& Context ) override;
    cpo::uno::Reference< cpo::uno::XInterface > getContext(  ) override;
    void createPeer( const cpo::uno::Reference< css::awt::XToolkit >& Toolkit, const cpo::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
    cpo::uno::Reference< css::awt::XWindowPeer > getPeer(  ) override;
    bool setModel( const cpo::uno::Reference< css::awt::XControlModel >& Model ) override;
    cpo::uno::Reference< css::awt::XControlModel > getModel(  ) override;
    cpo::uno::Reference< css::awt::XView > getView(  ) override;
    void setDesignMode( bool bOn ) override;
    bool isDesignMode(  ) override;
    bool isTransparent(  ) override;

    // css::lang::XServiceInfo
    OUString getImplementationName(  ) override;
    bool supportsService( const OUString& ServiceName ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XAccessible
    virtual cpo::uno::Reference< css::accessibility::XAccessibleContext > getAccessibleContext(  ) override;

    // XModeChangeBroadcaster
    virtual void addModeChangeListener( const cpo::uno::Reference< css::util::XModeChangeListener >& _rxListener ) override;
    virtual void removeModeChangeListener( const cpo::uno::Reference< css::util::XModeChangeListener >& _rxListener ) override;
    virtual void addModeChangeApproveListener( const cpo::uno::Reference< css::util::XModeChangeApproveListener >& _rxListener ) override;
    virtual void removeModeChangeApproveListener( const cpo::uno::Reference< css::util::XModeChangeApproveListener >& _rxListener ) override;

    // XUnitConversion
    virtual css::awt::Point convertPointToLogic( const css::awt::Point& Point, ::sal_Int16 TargetUnit ) override;
    virtual css::awt::Point convertPointToPixel( const css::awt::Point& aPoint, ::sal_Int16 SourceUnit ) override;
    virtual css::awt::Size convertSizeToLogic( const css::awt::Size& aSize, ::sal_Int16 TargetUnit ) override;
    virtual css::awt::Size convertSizeToPixel( const css::awt::Size& aSize, ::sal_Int16 SourceUnit ) override;

    // XStyleSettingsSupplier
    virtual cpo::uno::Reference< css::awt::XStyleSettings > getStyleSettings() override;

    cpo::uno::Reference< css::awt::XVclWindowPeer > getVclWindowPeer();

protected:
    // css::beans::XPropertiesChangeListener
    void propertiesChange( const cpo::uno::Sequence< css::beans::PropertyChangeEvent >& evt ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
