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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>

#include <stardiv/uno/repos/implementationregistration.hxx>
#include <stardiv/uno/repos/serinfo.hxx>
#include <stardiv/uno/awt/vcllstnr.hxx>
#include <stardiv/uno/awt/device.hxx>
#include <stardiv/uno/awt/graphics.hxx>
#include <stardiv/uno/awt/vclwin.hxx>

#include <usr/services.hxx>

#include <svtools/unoiface.hxx>

using namespace usr;

//==================================================================================================
class Listener_Impl
    : public UsrObject
    , public XMouseListener
    , public XMouseMotionListener
    , public XKeyListener
    , public XWindowListener
    , public XFocusListener
    , public XPaintListener
{
public:
    SMART_UNO_DECLARATION( Listener_Impl, UsrObject );

    virtual BOOL        queryInterface( Uik aUik, XInterfaceRef& rOut );

    // XMouseListener
    virtual void        mousePressed( const VclMouseEvent& evt );
    virtual void        mouseReleased( const VclMouseEvent& evt );
    virtual void        mouseEntered( const VclMouseEvent& evt );
    virtual void        mouseExited( const VclMouseEvent& evt );

    // XMouseMotionListener
    virtual void        mouseDragged( const VclMouseEvent& evt );
    virtual void        mouseMoved( const VclMouseEvent& evt );

    // XKeyListener
    virtual void        keyPressed( const VclKeyEvent& evt );
    virtual void        keyReleased( const VclKeyEvent& evt );

    // XFocusListener
    virtual void        focusGained( const FocusEvent& evt );
    virtual void        focusLost( const FocusEvent& evt );

    // XWindowListener
    virtual void        windowResized( const WindowEvent& evt );
    virtual void        windowMoved( const WindowEvent& evt );
    virtual void        windowShown( const EventObject& evt );
    virtual void        windowHidden( const EventObject& evt );

    // XPaintListener
    virtual void        windowPaint( const PaintEvent& evt );

    // XEventListener
    virtual void        disposing( const EventObject& evt );

public:
    void                addAllListeners( const XControlRef& xControl );
    void                removeAllListeners( const XControlRef& xControl );
};

//--------------------------------------------------------------------------------------------------
void Listener_Impl::addAllListeners( const XControlRef& xControl )
{
    XWindowRef xWindow( xControl, USR_QUERY );

    xWindow->addMouseListener( (XMouseListener*)this );
    xWindow->addMouseMotionListener( (XMouseMotionListener*)this );
    xWindow->addKeyListener( (XKeyListener*)this );
    xWindow->addFocusListener( (XFocusListener*)this );
    xWindow->addWindowListener( (XWindowListener*)this );
    xWindow->addPaintListener( (XPaintListener*)this );
    // cast due to ambiguities
    xControl->addEventListener( (XEventListener*)(XPaintListener*)this );
}

//--------------------------------------------------------------------------------------------------
void Listener_Impl::removeAllListeners( const XControlRef& xControl )
{
    XWindowRef xWindow( xControl, USR_QUERY );

    xWindow->removeMouseListener( (XMouseListener*)this );
    xWindow->removeMouseMotionListener( (XMouseMotionListener*)this );
    xWindow->removeKeyListener( (XKeyListener*)this );
    xWindow->removeFocusListener( (XFocusListener*)this );
    xWindow->removeWindowListener( (XWindowListener*)this );
    xWindow->removePaintListener( (XPaintListener*)this );
    // cast due to ambiguities
    xControl->removeEventListener( (XEventListener*)(XPaintListener*)this );
}

//--------------------------------------------------------------------------------------------------
SMART_UNO_IMPLEMENTATION( Listener_Impl, UsrObject );

//--------------------------------------------------------------------------------------------------
BOOL Listener_Impl::queryInterface( Uik aUik, XInterfaceRef& rOut )
{
    if (aUik == XMouseListener::getSmartUik())
        rOut = (XMouseListener*)this;
    else if (aUik == XMouseMotionListener::getSmartUik())
        rOut = (XMouseMotionListener*)this;
    else if (aUik == XWindowListener::getSmartUik())
        rOut = (XWindowListener*)this;
    else if (aUik == XFocusListener::getSmartUik())
        rOut = (XFocusListener*)this;
    else if (aUik == XKeyListener::getSmartUik())
        rOut = (XKeyListener*)this;
    else if (aUik == XPaintListener::getSmartUik())
        rOut = (XPaintListener*)this;
    else if (aUik == ((XEventListener*)NULL)->getSmartUik())
        rOut = (XEventListener*)(XMouseListener*)this;
    else
        return UsrObject::queryInterface( aUik, rOut );

    return TRUE;
}

//--------------------------------------------------------------------------------------------------
// XMouseListener
void Listener_Impl::mousePressed( const VclMouseEvent& evt )    {}
void Listener_Impl::mouseReleased( const VclMouseEvent& evt )   {}
void Listener_Impl::mouseEntered( const VclMouseEvent& evt )    {}
void Listener_Impl::mouseExited( const VclMouseEvent& evt )     {}

// XMouseMotionListener
void Listener_Impl::mouseDragged( const VclMouseEvent& evt )    {}
void Listener_Impl::mouseMoved( const VclMouseEvent& evt )      {}

// XKeyListener
void Listener_Impl::keyPressed( const VclKeyEvent& evt )        {}
void Listener_Impl::keyReleased( const VclKeyEvent& evt )       {}

// XFocusListener
void Listener_Impl::focusGained( const FocusEvent& evt )        {}
void Listener_Impl::focusLost( const FocusEvent& evt )          {}

// XWindowListener
void Listener_Impl::windowResized( const WindowEvent& evt )     {}
void Listener_Impl::windowMoved( const WindowEvent& evt )       {}
void Listener_Impl::windowShown( const EventObject& evt )       {}
void Listener_Impl::windowHidden( const EventObject& evt )      {}

// XPaintListener
void Listener_Impl::windowPaint( const PaintEvent& evt )
{
    if (evt.Source.is())
    {
        XControlRef xControl( evt.Source, USR_QUERY );
        if (xControl.is())
        {
            XDeviceRef xDev( xControl->getPeer(), USR_QUERY );
            XGraphicsRef xGraphics = xDev->createGraphics();
            xGraphics->drawLine( 0, 0, 200, 200 );
            xGraphics->drawLine( 200, 0, 0, 200 );
        }
    }
}

// XEventListener
void Listener_Impl::disposing( const EventObject& evt )     {}


//==================================================================================================
class FrameControlApplication
    : public Application
{
public:
    virtual void        Main();
    virtual void        ShowStatusText( const XubString& rText );

public:
                        FrameControlApplication()       {}

private:
    void                init();
    void                deinit();

private:
    Listener_Impl*      _pListener;
    XControlRef         _xControl;

    WorkWindow*         _pWorkWin;
};

FrameControlApplication g_App;

#ifdef __MWERKS__
Application* pApp = &g_App;
#endif


//--------------------------------------------------------------------------------------------------
void FrameControlApplication::init()
{
    XMultiServiceFactoryRef xMgr = createRegistryServiceManager( L"test.rdb" );
    registerUsrServices( xMgr );
    setProcessServiceManager( xMgr );
    InitExtVclToolkit();
    Application::RegisterUnoServices();

    XServiceRegistryRef xRegMgr(xMgr, USR_QUERY);

    XImplementationRegistrationRef xIR( xMgr->createInstance(L"stardiv.uno.repos.ImplementationRegistration"), USR_QUERY );
    try
    {
        char szDllName[_MAX_PATH]="";

        ORealDynamicLoader::computeModuleName("fc", szDllName, _MAX_PATH);
        UString aFCDllName = StringToOUString(szDllName, CHARSET_SYSTEM);
        xIR->registerImplementation(L"stardiv.loader.SharedLibrary", aFCDllName, XSimpleRegistryRef() );
    }
    catch( CannotRegisterImplementationException& e )
    {
    }



    // ...

    XInterfaceRef xInst = xMgr->createInstance( L"stardiv.one.frame.FrameControl" );
    if (xInst->queryInterface( XControl::getSmartUik(), _xControl ))
    {
        _pWorkWin = new WorkWindow( NULL, WB_APP | WB_STDWORK );
        _pWorkWin->Show();
        XWindowPeerRef xParent( _pWorkWin->GetComponentInterface() );

        XToolkitRef xToolkit( xMgr->createInstance( L"stardiv.vcl.VclToolkit" ), USR_QUERY );
        //xToolkit = XToolkitRef( xMgr->createInstance( L"stardiv.uno.awt.Toolkit" ), USR_QUERY );
        _xControl->createPeer( xToolkit, xParent );
        XWindowRef xWin( _xControl, USR_QUERY );
        xWin->setPosSize( 50, 50, 400, 400, PosSize_POSSIZE );
        xWin->setVisible( TRUE );

        _pListener = new Listener_Impl();
        _pListener->acquire();
        _pListener->addAllListeners( _xControl );
        // ... on paint a cross should be drawn
    }
}

//--------------------------------------------------------------------------------------------------
void FrameControlApplication::deinit()
{
    if (_pListener)
    {

        _pListener->removeAllListeners( _xControl );
        _xControl->dispose();   // disposing event should occur
        _pListener->release();
        _pListener = NULL;

        _xControl = XControlRef();
    }

    _pWorkWin->Hide();
    delete _pWorkWin;
}


//--------------------------------------------------------------------------------------------------
void FrameControlApplication::Main()
{
//      void TestErrcodes();
//      TestErrcodes();

    EnterMultiThread();
    SetAppName( "RadioActiveControl-Demo" );
    EnableSVLook();

    init();

    Execute();

    deinit();
}

//--------------------------------------------------------------------------------------------------
void FrameControlApplication::ShowStatusText( const XubString& rStatus )
{
    Application::GetAppWindow()->SetText( rStatus );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
