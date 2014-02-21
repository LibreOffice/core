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

#include <basebmp/scanlineformats.hxx>
#include <vcl/msgbox.hxx>
#include <touch/touch.h>
#include <touch/touch-impl.h>

#include "ios/iosinst.hxx"
#include "headless/svpdummies.hxx"
#include "generic/gendata.hxx"
#include "quartz/utils.h"
#include <vcl/settings.hxx>

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
    touch_ui_damaged( rDamageRect.getMinX(), rDamageRect.getMinY(), rDamageRect.getWidth(), rDamageRect.getHeight() );
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

    void ShowFullScreen( bool, sal_Int32 )
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
    mpFontList( 0 ),
    mxRGBSpace( CGColorSpaceCreateDeviceRGB() ),
    mxGraySpace( CGColorSpaceCreateDeviceGray() )
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

    NSLog(@"%@: %@", CreateNSString(rTitle), CreateNSString(rMessage));

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

    touch_ui_damaged( 0, 0, viewWidth, viewHeight );
    return 0;
}

extern "C"
void touch_lo_set_view_size(int width, int height)
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

    CGRect invalidRect = arg->rect;

    for( std::list< SalFrame* >::const_reverse_iterator it = getFrames().rbegin();
         it != getFrames().rend();
         it++ ) {
        IosSalFrame *pFrame = static_cast<IosSalFrame *>(*it);
        SalFrameGeometry aGeom = pFrame->GetGeometry();
        CGRect bbox = CGRectMake( 0, 0, aGeom.nWidth, aGeom.nHeight );
        if ( pFrame->IsVisible() &&
             CGRectIntersectsRect( invalidRect, bbox ) ) {

            const basebmp::BitmapDeviceSharedPtr aDevice = pFrame->getDevice();
            touch_lo_copy_buffer(aDevice->getBuffer().get(),
                                 aDevice->getSize().getX(),
                                 aDevice->getSize().getY(),
                                 aDevice->getScanlineStride(),
                                 arg->context,
                                 aGeom.nWidth,
                                 aGeom.nHeight);
            /*CGDataProviderRef provider =
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
             */
            // if current frame covers the whole invalidRect then break
            if (CGRectEqualToRect(CGRectIntersection(invalidRect, bbox), invalidRect))
            {
                break;
            }

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
void
touch_lo_copy_buffer(const void * source, size_t sourceWidth, size_t sourceHeight, size_t sourceBytesPerRow, void * target, size_t targetWidth, size_t targetHeight){

    CGDataProviderRef provider =CGDataProviderCreateWithData(NULL,
                                                             source,
                                                             sourceHeight * sourceBytesPerRow,
                                                             NULL );
    CGImage *sourceImage =  CGImageCreate(sourceWidth,
                                    sourceHeight,
                                    8,
                                    32,
                                    sourceBytesPerRow,
                                    CGColorSpaceCreateDeviceRGB(),
                                    kCGImageAlphaNoneSkipLast,
                                    provider,
                                    NULL,
                                    false,
                                    kCGRenderingIntentDefault );
    CGContextRef context =(CGContextRef) target;
    CGRect targetRect = CGRectMake( 0, 0, targetWidth, targetHeight );
    CGContextDrawImage( context, targetRect, sourceImage );
    CGImageRelease(sourceImage);
    CGDataProviderRelease(provider);
}

extern "C"
void touch_lo_render_windows(void *context, int minX, int minY, int width, int height)
{
    CGContextRef cgContext = (CGContextRef) context;
    int rc;
    IosSalInstance *pInstance = IosSalInstance::getInstance();

    if ( pInstance == NULL )
        return;

    rc = pthread_mutex_lock( &pInstance->m_aRenderMutex );
    if (rc != 0) {
        SAL_WARN( "vcl.ios", "pthread_mutex_lock failed: " << strerror( rc ) );
        return;
    }

    CGRect rect = CGRectMake(minX, minY, width, height);
    IosSalInstance::RenderWindowsArg arg = { false, cgContext, rect };
    Application::PostUserEvent( LINK( pInstance, IosSalInstance, RenderWindows), &arg );

    while (!arg.done) {
        rc = pthread_cond_wait( &pInstance->m_aRenderCond, &pInstance->m_aRenderMutex );
        SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_cond_wait failed: " << strerror( rc ) );
    }

    rc = pthread_mutex_unlock( &pInstance->m_aRenderMutex );
    SAL_WARN_IF( rc != 0, "vcl.ios", "pthread_mutex_unlock failed: " << strerror( rc ) );
}

extern "C"
void touch_lo_tap(int x, int y)
{
    touch_lo_mouse(x, y, DOWN, NONE);
    touch_lo_mouse(x, y, UP, NONE);
}

extern "C"
void touch_lo_mouse(int x, int y, MLOMouseButtonState state, MLOModifierMask modifiers)
{
    SalFrame *pFocus = IosSalInstance::getInstance()->getFocusFrame();

    if (pFocus) {
        MouseEvent aEvent;
        sal_uLong nEvent;
        sal_uInt16 nModifiers = 0;

        if (modifiers & SHIFT)
            nModifiers |= KEY_SHIFT;

        if (modifiers & META)
            nModifiers |= KEY_MOD1;

        switch (state) {
        case DOWN:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLECLICK, MOUSE_LEFT, nModifiers);
            nEvent = VCLEVENT_WINDOW_MOUSEBUTTONDOWN;
            break;
        case MOVE:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLEMOVE, MOUSE_LEFT, nModifiers);
            nEvent = VCLEVENT_WINDOW_MOUSEMOVE;
            break;
        case UP:
            aEvent = MouseEvent(Point(x, y), 1, MOUSE_SIMPLECLICK, MOUSE_LEFT, nModifiers);
            nEvent = VCLEVENT_WINDOW_MOUSEBUTTONUP;
            break;
        default:
            assert(false);
        }
        Application::PostMouseEvent(nEvent, pFocus->GetWindow(), &aEvent);
    }
}

extern "C"
void touch_lo_mouse_drag(int x, int y, MLOMouseButtonState state)
{
    touch_lo_mouse(x, y, state, NONE);
}

extern "C"
void touch_lo_pan(int deltaX, int deltaY)
{
    SalFrame *pFocus = IosSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        SAL_INFO( "vcl.ios", "pan delta: " << "(" << deltaX << "," << deltaY << ") ");
        ScrollEvent aEvent( deltaX, deltaY );
        Application::PostScrollEvent(VCLEVENT_WINDOW_SCROLL, pFocus->GetWindow(), &aEvent);
    }
}

extern "C"
void touch_lo_zoom(int x, int y, float scale)
{
    SalFrame *pFocus = IosSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        SAL_INFO( "vcl.ios", "pinch: " << "(" << scale  << ") ");
        ZoomEvent aEvent( Point(x,y), scale);
        Application::PostZoomEvent(VCLEVENT_WINDOW_ZOOM, pFocus->GetWindow(), &aEvent);
    }
}

extern "C"
void touch_lo_keyboard_input(int c)
{
    SalFrame *pFocus = IosSalInstance::getInstance()->getFocusFrame();
    if (pFocus) {
        KeyEvent aEvent(c, c, 0);
        Application::PostKeyEvent(VCLEVENT_WINDOW_KEYINPUT, pFocus->GetWindow(), &aEvent);
        Application::PostKeyEvent(VCLEVENT_WINDOW_KEYUP, pFocus->GetWindow(), &aEvent);
    }
}

extern "C"
void touch_lo_keyboard_did_hide()
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

IMPL_LINK( IosSalInstance, SelectionStartMove, SelectionStartMoveArg*, pArg )
{
    touch_lo_selection_start_move_impl(pArg->documentHandle, pArg->x, pArg->y);

    delete pArg;

    return 0;
}

extern "C"
void touch_lo_selection_start_move(const void *documentHandle,
                                   int x,
                                   int y)
{
    IosSalInstance *pInstance = IosSalInstance::getInstance();

    if ( pInstance == NULL )
        return;

    IosSalInstance::SelectionStartMoveArg *pArg = new IosSalInstance::SelectionStartMoveArg;
    pArg->documentHandle = documentHandle;
    pArg->x = x;
    pArg->y = y;
    Application::PostUserEvent( LINK( pInstance, IosSalInstance, SelectionStartMove), pArg );
}

IMPL_LINK( IosSalInstance, SelectionEndMove, SelectionEndMoveArg*, pArg )
{
    touch_lo_selection_end_move_impl(pArg->documentHandle, pArg->x, pArg->y);

    delete pArg;

    return 0;
}

extern "C"
void touch_lo_selection_end_move(const void *documentHandle,
                                 int x,
                                 int y)
{
    IosSalInstance *pInstance = IosSalInstance::getInstance();

    if ( pInstance == NULL )
        return;

    IosSalInstance::SelectionEndMoveArg *pArg = new IosSalInstance::SelectionEndMoveArg;
    pArg->documentHandle = documentHandle;
    pArg->x = x;
    pArg->y = y;
    Application::PostUserEvent( LINK( pInstance, IosSalInstance, SelectionEndMove), pArg );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
