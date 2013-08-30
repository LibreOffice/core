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

#include <premac.h>
#include <UIKit/UIKit.h>
#include <postmac.h>

#include <osl/detail/ios-bootstrap.h>
#include "ios/iosinst.hxx"
#include "headless/svpdummies.hxx"
#include "generic/gendata.hxx"

#include <basebmp/scanlineformats.hxx>
#include <vcl/msgbox.hxx>

// Horrible hack
static int viewWidth = 1, viewHeight = 1;

class IosSalData : public SalGenericData
{
public:
    IosSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_IOS, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};

void IosSalInstance::damaged( IosSalFrame */* frame */,
                              const basegfx::B2IBox& rDamageRect )
{
    lo_damaged( CGRectMake( rDamageRect.getMinX(), rDamageRect.getMinY(), rDamageRect.getWidth(), rDamageRect.getHeight() ));
}

void IosSalInstance::GetWorkArea( Rectangle& rRect )
{
    rRect = Rectangle( Point( 0, 0 ),
                       Size( viewWidth, viewHeight ) );
}

/*
 * Try too hard to get a frame, in the absence of anything better to do
 */
SalFrame *IosSalInstance::getFocusFrame() const
{
    SalFrame *pFocus = SvpSalFrame::GetFocusFrame();
    if (!pFocus) {
        const std::list< SalFrame* >& rFrames( getFrames() );
        for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
        {
            SvpSalFrame *pFrame = const_cast<SvpSalFrame*>(static_cast<const SvpSalFrame*>(*it));
            if( pFrame->IsVisible() )
            {
                pFrame->GetFocus();
                pFocus = pFrame;
                break;
            }
        }
    }
    return pFocus;
}

IosSalInstance *IosSalInstance::getInstance()
{
    if (!ImplGetSVData())
        return NULL;
    IosSalData *pData = static_cast<IosSalData *>(ImplGetSVData()->mpSalData);
    if (!pData)
        return NULL;
    return static_cast<IosSalInstance *>(pData->m_pInstance);
}

IosSalInstance::IosSalInstance( SalYieldMutex *pMutex )
    : SvpSalInstance( pMutex )
{
    int rc;

    rc = pthread_cond_init( &m_aRenderCond, NULL );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_cond_init failed: " << strerror( rc ) );

#if OSL_DEBUG_LEVEL > 0
    pthread_mutexattr_t mutexattr;

    rc = pthread_mutexattr_init( &mutexattr );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_mutexattr_init failed: " << strerror( rc ) );

    rc = pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_ERRORCHECK );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_mutexattr_settype failed: " << strerror( rc ) );

    rc = pthread_mutex_init( &m_aRenderMutex, &mutexattr );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_mutex_init failed: " << strerror( rc ) );
#else
    rc = pthread_mutex_init( &m_aRenderMutex, NULL );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_mutex_init failed: " << strerror( rc ) );
#endif
}

IosSalInstance::~IosSalInstance()
{
    pthread_cond_destroy( &m_aRenderCond );
    pthread_mutex_destroy( &m_aRenderMutex );
}

#if 0

bool IosSalInstance::AnyInput( sal_uInt16 nType )
{
    if( (nType & VCL_INPUT_TIMER) != 0 )
        return CheckTimeout( false );

    // Unfortunately there is no way to check for a specific type of
    // input being queued. That information is too hidden, sigh.
    return SvpSalInstance::s_pDefaultInstance->PostedEventsInQueue();
}

#endif

class IosSalSystem : public SvpSalSystem {
public:
    IosSalSystem() : SvpSalSystem() {}
    virtual ~IosSalSystem() {}
    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::list< OUString >& rButtons,
                                  int nDefButton );
};

SalSystem *IosSalInstance::CreateSalSystem()
{
    return new IosSalSystem();
}

class IosSalFrame : public SvpSalFrame
{
private:
    basegfx::B2IBox m_DamagedRect;

public:
    IosSalFrame( IosSalInstance *pInstance,
                     SalFrame           *pParent,
                     sal_uLong           nSalFrameStyle,
                     SystemParentData   *pSysParent )
        : SvpSalFrame( pInstance, pParent, nSalFrameStyle,
                       true, basebmp::FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA,
                       pSysParent )
    {
        enableDamageTracker();
        if (pParent == NULL && viewWidth > 1 && viewHeight > 1)
            SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    }

    virtual void GetWorkArea( Rectangle& rRect )
    {
        IosSalInstance::getInstance()->GetWorkArea( rRect );
    }

    void ShowFullScreen( sal_Bool, sal_Int32 )
    {
        SetPosSize( 0, 0, viewWidth, viewHeight,
                    SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }

    virtual void damaged( const basegfx::B2IBox& rDamageRect )
    {
        if (rDamageRect.isEmpty())
            return;

        m_DamagedRect.expand(rDamageRect);
    }

    virtual void UpdateSettings( AllSettings &rSettings )
    {
        // Clobber the UI fonts
        Font aFont( OUString( "Helvetica" ), Size( 0, 14 ) );

        StyleSettings aStyleSet = rSettings.GetStyleSettings();
        aStyleSet.SetAppFont( aFont );
        aStyleSet.SetHelpFont( aFont );
        aStyleSet.SetMenuFont( aFont );
        aStyleSet.SetToolFont( aFont );
        aStyleSet.SetLabelFont( aFont );
        aStyleSet.SetInfoFont( aFont );
        aStyleSet.SetRadioCheckFont( aFont );
        aStyleSet.SetPushButtonFont( aFont );
        aStyleSet.SetFieldFont( aFont );
        aStyleSet.SetIconFont( aFont );
        aStyleSet.SetGroupFont( aFont );

        rSettings.SetStyleSettings( aStyleSet );
    }

    virtual void Flush()
    {
        IosSalInstance::getInstance()->damaged( this, m_DamagedRect );
    }

    void resetDamaged()
    {
        m_DamagedRect.reset();
    }
};

SalFrame *IosSalInstance::CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle )
{
    return new IosSalFrame( this, NULL, nStyle, pParent );
}

SalFrame *IosSalInstance::CreateFrame( SalFrame* pParent, sal_uLong nStyle )
{
    return new IosSalFrame( this, pParent, nStyle, NULL );
}


// All the interesting stuff is slaved from the IosSalInstance
void InitSalData()   {}
void DeInitSalData() {}
void InitSalMain()   {}

void SalAbort( const OUString& rErrorText, bool bDumpCore )
{
    (void) bDumpCore;

    NSLog(@"SalAbort: %s", OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );
}

const OUString& SalGetDesktopEnvironment()
{
    static OUString aEnv( "iOS" );
    return aEnv;
}

SalData::SalData() :
    m_pInstance( 0 ),
    m_pPlugin( 0 ),
    m_pPIManager(0 ),
    mpFontList( 0 )
{
}

SalData::~SalData()
{
}

// This is our main entry point:
SalInstance *CreateSalInstance()
{
    IosSalInstance* pInstance = new IosSalInstance( new SalYieldMutex() );
    new IosSalData( pInstance );
    pInstance->AcquireYieldMutex(1);
    return pInstance;
}

void DestroySalInstance( SalInstance *pInst )
{
    pInst->ReleaseYieldMutex();
    delete pInst;
}

int IosSalSystem::ShowNativeDialog( const OUString& rTitle,
                                    const OUString& rMessage,
                                    const std::list< OUString >& rButtons,
                                    int nDefButton )
{
    (void)rButtons;
    (void)nDefButton;

    if (IosSalInstance::getInstance() != NULL)
    {
        // Temporary...

        ErrorBox aVclErrBox( NULL, WB_OK, rTitle );
        aVclErrBox.SetText( rMessage );
        aVclErrBox.Execute();
    }

    return 0;
}

IMPL_LINK( IosSalInstance, DisplayConfigurationChanged, void*, )
{
    for( std::list< SalFrame* >::const_iterator it = getFrames().begin();
         it != getFrames().end();
         it++ ) {
        (*it)->Show( sal_False, sal_False );
        (*it)->CallCallback( SALEVENT_SETTINGSCHANGED, 0 );
        (*it)->SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
        (*it)->Show( sal_True, sal_False );
    }

    lo_damaged( CGRectMake( 0, 0, viewWidth, viewHeight ) );
    return 0;
}

extern "C"
void lo_set_view_size(int width, int height)
{
    int oldWidth = viewWidth;

    viewWidth = width;
    viewHeight = height;

    if (oldWidth > 1) {
        // Inform about change in display size (well, just orientation
        // presumably).
        IosSalInstance *pInstance = IosSalInstance::getInstance();

        if ( pInstance == NULL )
            return;

        Application::PostUserEvent( LINK( pInstance, IosSalInstance, DisplayConfigurationChanged ), NULL );
    }
}

IMPL_LINK( IosSalInstance, RenderWindows, RenderWindowsArg*, arg )
{
    int rc;

    rc = pthread_mutex_lock( &m_aRenderMutex );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_mutex_lock failed: " << strerror( rc ) );

    for( std::list< SalFrame* >::const_iterator it = getFrames().begin();
         it != getFrames().end();
         it++ ) {
        IosSalFrame *pFrame = static_cast<IosSalFrame *>(*it);
        SalFrameGeometry aGeom = pFrame->GetGeometry();
        CGRect bbox = CGRectMake( aGeom.nX, aGeom.nY, aGeom.nWidth, aGeom.nHeight );
        if ( pFrame->IsVisible() &&
             CGRectIntersectsRect( arg->rect, bbox ) ) {

            const basebmp::BitmapDeviceSharedPtr aDevice = pFrame->getDevice();
            CGDataProviderRef provider =
                CGDataProviderCreateWithData( NULL,
                                              aDevice->getBuffer().get(),
                                              aDevice->getSize().getY() * aDevice->getScanlineStride(),
                                              NULL );
            CGImage *image =
                CGImageCreate( aDevice->getSize().getX(), aDevice->getSize().getY(),
                               8, 32, aDevice->getScanlineStride(),
                               CGColorSpaceCreateDeviceRGB(),
                               kCGImageAlphaNoneSkipLast,
                               provider,
                               NULL,
                               false,
                               kCGRenderingIntentDefault );
            CGContextDrawImage( arg->context, bbox, image );

            pFrame->resetDamaged();
        }
    }

    arg->done = true;

    rc = pthread_cond_signal( &m_aRenderCond );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_cond_signal failed:" << strerror( rc ) );

    rc = pthread_mutex_unlock( &m_aRenderMutex );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_mutex_unlock failed: " << strerror( rc ) );

    return 0;
}

extern "C"
void lo_render_windows( CGContextRef context, CGRect rect )
{
    int rc;
    IosSalInstance *pInstance = IosSalInstance::getInstance();

    if ( pInstance == NULL )
        return;

    rc = pthread_mutex_lock( &pInstance->m_aRenderMutex );
    if (rc != 0) {
        SAL_WARN( "vcl.ios", "pthread_mutex_lock failed: " << strerror( rc ) );
        return;
    }

    IosSalInstance::RenderWindowsArg arg = { false, context, rect };
    Application::PostUserEvent( LINK( pInstance, IosSalInstance, RenderWindows), &arg );

    while (!arg.done) {
        rc = pthread_cond_wait( &pInstance->m_aRenderCond, &pInstance->m_aRenderMutex );
        SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_cond_wait failed: " << strerror( rc ) );
    }

    rc = pthread_mutex_unlock( &pInstance->m_aRenderMutex );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_mutex_unlock failed: " << strerror( rc ) );
}

extern "C"
void lo_tap(int x, int y)
{
    SalFrame *pFocus = IosSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        MouseEvent aEvent;
        sal_uLong nEvent;

        aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLECLICK, MOUSE_LEFT);
        nEvent = VCLEVENT_WINDOW_MOUSEBUTTONDOWN;
        Application::PostMouseEvent(nEvent, pFocus->GetWindow(), &aEvent);

        nEvent = VCLEVENT_WINDOW_MOUSEBUTTONUP;
        Application::PostMouseEvent(nEvent, pFocus->GetWindow(), &aEvent);
    }
}

extern "C"
void lo_pan(int x, int y)
{
    SalFrame *pFocus = IosSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        SAL_INFO( "vcl.ios", "scroll: " << "(" << x << "," << y << ")" );
        ScrollEvent aEvent( x, y );
        Application::PostScrollEvent(VCLEVENT_WINDOW_SCROLL, pFocus->GetWindow(), &aEvent);
    }
}

extern "C"
void lo_keyboard_input(int c)
{
    SalFrame *pFocus = IosSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        KeyEvent aEvent(c, c, 0);
        Application::PostKeyEvent(VCLEVENT_WINDOW_KEYINPUT, pFocus->GetWindow(), &aEvent);
        Application::PostKeyEvent(VCLEVENT_WINDOW_KEYUP, pFocus->GetWindow(), &aEvent);
    }
}

extern "C"
void lo_keyboard_did_hide()
{
    // Tell LO it has lost "focus", which will cause it to stop
    // displaying any text insertion cursor etc

    SalFrame *pFocus = IosSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        MouseEvent aEvent;

        aEvent = MouseEvent(Point(0, 0), 0, MOUSE_LEAVEWINDOW, MOUSE_LEFT);
        Application::PostMouseEvent(VCLEVENT_WINDOW_MOUSEMOVE, pFocus->GetWindow(), &aEvent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
