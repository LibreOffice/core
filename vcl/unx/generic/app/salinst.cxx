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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <osl/module.hxx>

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/geninst.h>
#include <unx/genpspgraphics.h>
#include <unx/salframe.h>
#include <unx/genprn.h>
#include <unx/sm.hxx>
#include <unx/i18n_im.hxx>
#include <unx/saldisp.hxx>
#include <unx/helper.hxx>

#include <vcl/inputtypes.hxx>

#include "salwtype.hxx"
#include <sal/macros.h>

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

        X11SalInstance* pInstance = new X11SalInstance( new SalYieldMutex() );

        // initialize SalData
        X11SalData *pSalData = new X11SalData( SAL_DATA_UNX, pInstance );

        pSalData->Init();
        pInstance->SetLib( pSalData->GetLib() );

        return pInstance;
    }
}

X11SalInstance::X11SalInstance(SalYieldMutex* pMutex)
    : SalGenericInstance(pMutex)
    , mpXLib(nullptr)
{
    ImplSVData* pSVData = ImplGetSVData();
    delete pSVData->maAppData.mpToolkitName;
    pSVData->maAppData.mpToolkitName = new OUString("x11");
}

X11SalInstance::~X11SalInstance()
{
    // close session management
    SessionManagerClient::close();

    // dispose SalDisplay list from SalData
    // would be done in a static destructor else which is
    // a little late
    GetGenericData()->Dispose();
}

SalX11Display* X11SalInstance::CreateDisplay() const
{
    return new SalX11Display( mpXLib->GetDisplay() );
}

// AnyInput from sv/mow/source/app/svapp.cxx

struct PredicateReturn
{
    VclInputFlags nType;
    bool          bRet;
};

extern "C" {
Bool ImplPredicateEvent( Display *, XEvent *pEvent, char *pData )
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
    SalGenericData *pData = GetGenericData();
    Display *pDisplay  = vcl_sal::getSalDisplay(pData)->GetDisplay();
    bool bRet = false;

    if( (nType & VclInputFlags::TIMER) && (mpXLib && mpXLib->CheckTimeout(false)) )
        bRet = true;
    else if (XPending(pDisplay) )
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
    fprintf( stderr, "AnyInput 0x%x = %s\n", static_cast<unsigned int>(nType), bRet ? "true" : "false" );
#endif
    return bRet;
}

SalYieldResult X11SalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong const nReleased)
{
    (void) nReleased;
    assert(nReleased == 0); // not implemented
    return mpXLib->Yield( bWait, bHandleAllCurrentEvents );
}

void* X11SalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType,
                                               int& rReturnedBytes )
{
    static const char* pDisplay = getenv( "DISPLAY" );
    rReturnedType   = AsciiCString;
    rReturnedBytes  = pDisplay ? strlen( pDisplay )+1 : 1;
    return pDisplay ? const_cast<char *>(pDisplay) : const_cast<char *>("");
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

extern "C" { static void SAL_CALL thisModule() {} }

void X11SalInstance::AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService)
{
    typedef void (*PFUNC_ADD_TO_RECENTLY_USED_LIST)(const OUString&, const OUString&, const OUString&);

    PFUNC_ADD_TO_RECENTLY_USED_LIST add_to_recently_used_file_list = nullptr;

    osl::Module module;
    module.loadRelative( &thisModule, "librecentfile.so" );
    if (module.is())
        add_to_recently_used_file_list = reinterpret_cast<PFUNC_ADD_TO_RECENTLY_USED_LIST>(module.getFunctionSymbol("add_to_recently_used_file_list"));
    if (add_to_recently_used_file_list)
        add_to_recently_used_file_list(rFileUrl, rMimeType, rDocumentService);
}

void X11SalInstance::PostPrintersChanged()
{
    SalDisplay* pDisp = vcl_sal::getSalDisplay(GetGenericData());
    const std::list< SalFrame* >& rList = pDisp->getFrames();
    for( std::list< SalFrame* >::const_iterator it = rList.begin();
         it != rList.end(); ++it )
        pDisp->SendInternalEvent( *it, nullptr, SalEvent::PrinterChanged );
}

GenPspGraphics *X11SalInstance::CreatePrintGraphics()
{
    return new GenPspGraphics();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
