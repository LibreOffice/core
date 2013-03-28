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
#include <CoreGraphics/CoreGraphics.h>
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

void IosSalInstance::BlitFrameToBuffer(char *pPixelBuffer,
                                       int nPBWidth, int nPBHeight,
                                       int nDestX, int nDestY,
                                       int nDestWidth, int nDestHeight,
                                       const basebmp::BitmapDeviceSharedPtr& aDev)
{
    // TODO: Cropping (taking all the parameters into account)
    (void) nPBHeight;
    (void) nDestWidth;
    (void) nDestHeight;

    basebmp::RawMemorySharedArray aSrcData = aDev->getBuffer();
    const basegfx::B2IVector aDevSize = aDev->getSize();
    const sal_Int32 nStride = aDev->getScanlineStride();
    const unsigned char *pSrc = aSrcData.get();

    if (aDev->getScanlineFormat() != basebmp::Format::THIRTYTWO_BIT_TC_MASK_RGBA)
    {
        SAL_INFO( "vcl.ios", "BlitFrameToBuffer: format is not 32bpp RGBA but " << aDev->getScanlineFormat() );
        return;
    }

    for (unsigned int y = 0; y < (unsigned int)aDevSize.getY(); y++)
    {
        const unsigned char *sp( pSrc + nStride * (aDevSize.getY() - 1 - y) );

        unsigned char *dp( (unsigned char *)pPixelBuffer +
                           nPBWidth * 4 * (y + nDestY) +
                           nDestX * 4 );
        memcpy(dp, sp, aDevSize.getX()*4);
    }
}

void IosSalInstance::RedrawWindows(char *pPixelBuffer,
                                   int nPBWidth, int nPBHeight,
                                   int nDestX, int nDestY,
                                   int nDestWidth, int nDestHeight)
{
    int i = 0;
    std::list< SalFrame* >::const_iterator it;
    for ( it = getFrames().begin(); it != getFrames().end(); i++, it++ )
    {
        SvpSalFrame *pFrame = static_cast<SvpSalFrame *>(*it);

        if (pFrame->IsVisible())
            BlitFrameToBuffer( pPixelBuffer, nPBWidth, nPBHeight, nDestX, nDestY, nDestWidth, nDestHeight, pFrame->getDevice() );
    }
}

void IosSalInstance::damaged(IosSalFrame */* frame */)
{
    lo_damaged();
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

}

IosSalInstance::~IosSalInstance()
{
}

bool IosSalInstance::AnyInput( sal_uInt16 nType )
{
    if( (nType & VCL_INPUT_TIMER) != 0 )
        return CheckTimeout( false );

    // Unfortunately there is no way to check for a specific type of
    // input being queued. That information is too hidden, sigh.
    return SvpSalInstance::s_pDefaultInstance->PostedEventsInQueue();
}

class IosSalSystem : public SvpSalSystem {
public:
    IosSalSystem() : SvpSalSystem() {}
    virtual ~IosSalSystem() {}
    virtual int ShowNativeDialog( const rtl::OUString& rTitle,
                                  const rtl::OUString& rMessage,
                                  const std::list< rtl::OUString >& rButtons,
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
                       true, basebmp::Format::THIRTYTWO_BIT_TC_MASK_RGBA,
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

    virtual void damaged( const basegfx::B2IBox& rDamageRect)
    {
        if (rDamageRect.getWidth() <= 0 ||
            rDamageRect.getHeight() <= 0)
        {
            return;
        }
        IosSalInstance::getInstance()->damaged( this );
    }

    virtual void UpdateSettings( AllSettings &rSettings )
    {
        // Clobber the UI fonts
        Font aFont( rtl::OUString( "Helvetica" ), Size( 0, 14 ) );

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

int IosSalSystem::ShowNativeDialog( const rtl::OUString& rTitle,
                                    const rtl::OUString& rMessage,
                                    const std::list< rtl::OUString >& rButtons,
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

extern "C"
void lo_set_view_size(int width, int height)
{
    // Horrible
    viewWidth = width;
    viewHeight = height;
}

extern "C"
void lo_render_windows(char *pixelBuffer, int width, int height)
{
    // Hack: assume so far that we are asked to redraw the whole pixel buffer
    if (IosSalInstance::getInstance() != NULL)
        IosSalInstance::getInstance()->RedrawWindows(pixelBuffer, width, height, 0, 0, width, height);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
