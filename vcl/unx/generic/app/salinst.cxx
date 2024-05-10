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

#include <stdlib.h>

#include <config_features.h>
#include <vcl/skia/SkiaHelper.hxx>
#include <config_skia.h>
#if HAVE_FEATURE_SKIA
#include <skia/x11/gdiimpl.hxx>
#include <skia/salbmp.hxx>
#endif

#include <headless/svpbmp.hxx>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/salinst.h>
#include <unx/geninst.h>
#include <unx/genpspgraphics.h>
#include <unx/salframe.h>
#include <unx/sm.hxx>
#include <unx/i18n_im.hxx>

#include <vcl/inputtypes.hxx>

#include <salwtype.hxx>

// plugin factory function
extern "C"
{
    VCLPLUG_GEN_PUBLIC SalInstance* create_SalInstance()
    {
        /* #i92121# workaround deadlocks in the X11 implementation
        */
        static const char* pNoXInitThreads = getenv( "SAL_NO_XINITTHREADS" );
        /* #i90094#
           from now on we know that an X connection will be
           established, so protect X against itself
        */
        if( ! ( pNoXInitThreads && *pNoXInitThreads ) )
            XInitThreads();

        X11SalInstance* pInstance = new X11SalInstance( std::make_unique<SalYieldMutex>() );

        // initialize SalData
        X11SalData *pSalData = new X11SalData();

        pSalData->Init();
        pInstance->SetLib( pSalData->GetLib() );

        return pInstance;
    }
}

X11SalInstance::X11SalInstance(std::unique_ptr<SalYieldMutex> pMutex)
    : SalGenericInstance(std::move(pMutex))
    , mpXLib(nullptr)
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxToolkitName = u"x11"_ustr;
    m_bSupportsOpenGL = true;
#if HAVE_FEATURE_SKIA
    X11SkiaSalGraphicsImpl::prepareSkia();
#if SKIA_USE_BITMAP32
    if (SkiaHelper::isVCLSkiaEnabled())
        m_bSupportsBitmap32 = true;
#endif
#endif
}

X11SalInstance::~X11SalInstance()
{
    // close session management
    SessionManagerClient::close();

    // dispose SalDisplay list from SalData
    // would be done in a static destructor else which is
    // a little late
    GetGenericUnixSalData()->Dispose();

#if HAVE_FEATURE_SKIA
    SkiaHelper::cleanup();
#endif
}

SalX11Display* X11SalInstance::CreateDisplay() const
{
    return new SalX11Display( mpXLib->GetDisplay() );
}

// AnyInput from sv/mow/source/app/svapp.cxx

namespace {

struct PredicateReturn
{
    VclInputFlags nType;
    bool          bRet;
};

}

extern "C" {
static Bool ImplPredicateEvent( Display *, XEvent *pEvent, char *pData )
{
    PredicateReturn *pPre = reinterpret_cast<PredicateReturn *>(pData);

    if ( pPre->bRet )
        return False;

    VclInputFlags nType;

    switch( pEvent->type )
    {
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
        case EnterNotify:
        case LeaveNotify:
            nType = VclInputFlags::MOUSE;
            break;

        case KeyPress:
        //case KeyRelease:
            nType = VclInputFlags::KEYBOARD;
            break;
        case Expose:
        case GraphicsExpose:
        case NoExpose:
            nType = VclInputFlags::PAINT;
            break;
        default:
            nType = VclInputFlags::NONE;
    }

    if ( (nType & pPre->nType) || ( nType == VclInputFlags::NONE && (pPre->nType & VclInputFlags::OTHER) ) )
        pPre->bRet = true;

    return False;
}
}

bool X11SalInstance::AnyInput(VclInputFlags nType)
{
    GenericUnixSalData *pData = GetGenericUnixSalData();
    Display *pDisplay  = vcl_sal::getSalDisplay(pData)->GetDisplay();
    bool bRet = false;

    if( (nType & VclInputFlags::TIMER) && (mpXLib && mpXLib->CheckTimeout(false)) )
        bRet = true;

    if( !bRet && XPending(pDisplay) )
    {
        PredicateReturn aInput;
        XEvent          aEvent;

        aInput.bRet     = false;
        aInput.nType    = nType;

        XCheckIfEvent(pDisplay, &aEvent, ImplPredicateEvent,
                      reinterpret_cast<char *>(&aInput) );

        bRet = aInput.bRet;
    }
#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("vcl.app", "AnyInput "
            << std::showbase << std::hex
            << static_cast<unsigned int>(nType)
            << " = " << (bRet ? "true" : "false"));
#endif
    return bRet;
}

bool X11SalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents)
{
    return mpXLib->Yield( bWait, bHandleAllCurrentEvents );
}

OUString X11SalInstance::GetConnectionIdentifier()
{
    static const char* pDisplay = getenv( "DISPLAY" );
    return pDisplay ? OUString::createFromAscii(pDisplay) : OUString();
}

SalFrame *X11SalInstance::CreateFrame( SalFrame *pParent, SalFrameStyleFlags nSalFrameStyle )
{
    SalFrame *pFrame = new X11SalFrame( pParent, nSalFrameStyle );

    return pFrame;
}

SalFrame* X11SalInstance::CreateChildFrame( SystemParentData* pParentData, SalFrameStyleFlags nStyle )
{
    SalFrame* pFrame = new X11SalFrame( nullptr, nStyle, pParentData );

    return pFrame;
}

void X11SalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

void X11SalInstance::AfterAppInit()
{
    assert( mpXLib->GetDisplay() );
    assert( mpXLib->GetInputMethod() );

    SalX11Display *pSalDisplay = CreateDisplay();
    mpXLib->GetInputMethod()->CreateMethod( mpXLib->GetDisplay() );
    pSalDisplay->SetupInput();
}

void X11SalInstance::AddToRecentDocumentList(const OUString&, const OUString&, const OUString&) {}

void X11SalInstance::PostPrintersChanged()
{
    SalDisplay* pDisp = vcl_sal::getSalDisplay(GetGenericUnixSalData());
    for (auto pSalFrame : pDisp->getFrames() )
        pDisp->PostEvent( pSalFrame, nullptr, SalEvent::PrinterChanged );
}

std::unique_ptr<GenPspGraphics> X11SalInstance::CreatePrintGraphics()
{
    return std::make_unique<GenPspGraphics>();
}

std::shared_ptr<SalBitmap> X11SalInstance::CreateSalBitmap()
{
#if HAVE_FEATURE_SKIA
    if (SkiaHelper::isVCLSkiaEnabled())
        return std::make_shared<SkiaSalBitmap>();
#endif
    return std::make_shared<SvpSalBitmap>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
