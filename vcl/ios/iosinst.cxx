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
#include <touch/touch.h>
#include <touch/touch-impl.h>

#include "ios/iosinst.hxx"
#include "headless/svpdummies.hxx"
#include "generic/gendata.hxx"
#include "quartz/utils.h"
#include <vcl/layout.hxx>
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
}

IosSalInstance::~IosSalInstance()
{
}

#if 0

bool IosSalInstance::AnyInput( VclInputFlags nType )
{
    if( nType & VclInputFlags::TIMER )
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
public:
    IosSalFrame( IosSalInstance *pInstance,
                     SalFrame           *pParent,
                     sal_uLong           nSalFrameStyle,
                     SystemParentData   *pSysParent )
        : SvpSalFrame( pInstance, pParent, nSalFrameStyle,
                       true, basebmp::FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA,
                       pSysParent )
    {
        if (pParent == NULL && viewWidth > 1 && viewHeight > 1)
            SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    }

    virtual void GetWorkArea( Rectangle& rRect ) SAL_OVERRIDE
    {
        IosSalInstance::getInstance()->GetWorkArea( rRect );
    }

    virtual void ShowFullScreen( bool, sal_Int32 ) SAL_OVERRIDE
    {
        SetPosSize( 0, 0, viewWidth, viewHeight,
                    SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }

    virtual void UpdateSettings( AllSettings &rSettings ) SAL_OVERRIDE
    {
        // Clobber the UI fonts
        vcl::Font aFont( OUString( "Helvetica" ), Size( 0, 14 ) );

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
    ImplGetSVData()->maWinData.mbNoSaveBackground = true;
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

    return 0;
}

IMPL_LINK( IosSalInstance, DisplayConfigurationChanged, void*, )
{
    for( std::list< SalFrame* >::const_iterator it = getFrames().begin();
         it != getFrames().end();
         ++it ) {
        (*it)->Show( false, false );
        (*it)->CallCallback( SALEVENT_SETTINGSCHANGED, 0 );
        (*it)->SetPosSize(0, 0, viewWidth, viewHeight, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
        (*it)->Show( true, false );
    }

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
            aEvent = MouseEvent(Point(x, y), 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT, nModifiers);
            nEvent = VCLEVENT_WINDOW_MOUSEBUTTONDOWN;
            break;
        case MOVE:
            aEvent = MouseEvent(Point(x, y), 1, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT, nModifiers);
            nEvent = VCLEVENT_WINDOW_MOUSEMOVE;
            break;
        case UP:
            aEvent = MouseEvent(Point(x, y), 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT, nModifiers);
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

        aEvent = MouseEvent(Point(0, 0), 0, MouseEventModifiers::LEAVEWINDOW, MOUSE_LEFT);
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
