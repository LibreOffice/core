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

#include <X11/Xlib.h>

#include <unx/i18n_ic.hxx>
#include <unx/i18n_im.hxx>

#include <unx/salframe.h>
#include <unx/saldisp.hxx>

#include <sal/log.hxx>

using namespace vcl;

static void sendEmptyCommit( SalFrame* pFrame )
{
    vcl::DeletionListener aDel( pFrame );

    SalExtTextInputEvent aEmptyEv;
    aEmptyEv.mpTextAttr         = nullptr;
    aEmptyEv.maText.clear();
    aEmptyEv.mnCursorPos        = 0;
    aEmptyEv.mnCursorFlags      = 0;
    pFrame->CallCallback( SalEvent::ExtTextInput, static_cast<void*>(&aEmptyEv) );
    if( ! aDel.isDeleted() )
        pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
}

// Constructor / Destructor, the InputContext is bound to the SalFrame, as it
// needs the shell window as a focus window

SalI18N_InputContext::~SalI18N_InputContext()
{
    if ( maContext != nullptr )
        XDestroyIC( maContext );
    if ( mpAttributes != nullptr )
        XFree( mpAttributes );
    if ( mpStatusAttributes != nullptr )
        XFree( mpStatusAttributes );
    if ( mpPreeditAttributes != nullptr )
        XFree( mpPreeditAttributes );

    if (maClientData.aText.pUnicodeBuffer != nullptr)
        free(maClientData.aText.pUnicodeBuffer);
    if (maClientData.aText.pCharStyle != nullptr)
        free(maClientData.aText.pCharStyle);
}

// convenience routine to add items to a XVaNestedList

static XVaNestedList
XVaAddToNestedList( XVaNestedList a_srclist, char* name, XPointer value )
{
    XVaNestedList a_dstlist;

    // if ( value == NULL )
    //  return a_srclist;

    if ( a_srclist == nullptr )
    {
        a_dstlist = XVaCreateNestedList(
                                        0,
                                        name,           value,
                                        nullptr );
    }
    else
    {
        a_dstlist = XVaCreateNestedList(
                                        0,
                                        XNVaNestedList, a_srclist,
                                        name,           value,
                                        nullptr );
    }

    return a_dstlist != nullptr ? a_dstlist : a_srclist ;
}

// convenience routine to create a fontset

static XFontSet
get_font_set( Display *p_display )
{
    static XFontSet p_font_set = nullptr;

    if (p_font_set == nullptr)
    {
        char **pp_missing_list;
        int    n_missing_count;
        char  *p_default_string;

        p_font_set = XCreateFontSet(p_display, "-*",
                                    &pp_missing_list, &n_missing_count, &p_default_string);
    }

    return p_font_set;
}

const XIMStyle g_nSupportedStatusStyle(
                               XIMStatusCallbacks   |
                               XIMStatusNothing     |
                               XIMStatusNone
                               );

// Constructor for an InputContext (IC)

SalI18N_InputContext::SalI18N_InputContext ( SalFrame *pFrame ) :
        mbUseable( True ),
        maContext( nullptr ),
        mnSupportedPreeditStyle(
                                XIMPreeditCallbacks |
                                XIMPreeditNothing   |
                                XIMPreeditNone
                                ),
        mnStatusStyle( 0 ),
        mnPreeditStyle( 0 ),
        mpAttributes( nullptr ),
        mpStatusAttributes( nullptr ),
        mpPreeditAttributes( nullptr )
{
#ifdef __sun
    static const char* pIIIMPEnable = getenv( "SAL_DISABLE_OWN_IM_STATUS" );
    if( pIIIMPEnable && *pIIIMPEnable )
        mnSupportedStatusStyle &= ~XIMStatusCallbacks;
#endif

    memset(&maPreeditStartCallback, 0, sizeof(maPreeditStartCallback));
    memset(&maPreeditDoneCallback, 0, sizeof(maPreeditDoneCallback));
    memset(&maPreeditDrawCallback, 0, sizeof(maPreeditDrawCallback));
    memset(&maPreeditCaretCallback, 0, sizeof(maPreeditCaretCallback));
    memset(&maCommitStringCallback, 0, sizeof(maCommitStringCallback));
    memset(&maSwitchIMCallback, 0, sizeof(maSwitchIMCallback));
    memset(&maDestroyCallback, 0, sizeof(maDestroyCallback));

    maClientData.aText.pUnicodeBuffer       = nullptr;
    maClientData.aText.pCharStyle           = nullptr;
    maClientData.aInputEv.mpTextAttr        = nullptr;
    maClientData.aInputEv.mnCursorPos       = 0;
    maClientData.aInputEv.mnCursorFlags     = 0;

    SalI18N_InputMethod *pInputMethod;
    pInputMethod = vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetInputMethod();

    mnSupportedPreeditStyle =   XIMPreeditCallbacks | XIMPreeditPosition
        | XIMPreeditNothing   | XIMPreeditNone;
    if (pInputMethod->UseMethod()
        && SupportInputMethodStyle( pInputMethod->GetSupportedStyles() ) )
    {
        const SystemEnvData* pEnv = pFrame->GetSystemData();
        ::Window  aClientWindow = pEnv->aShellWindow;
        ::Window  aFocusWindow  = pEnv->GetWindowHandle(pFrame);

        // for status callbacks and commit string callbacks
#define PREEDIT_BUFSZ 16
        maClientData.eState                 = PreeditStatus::StartPending;
        maClientData.pFrame                 = pFrame;
        maClientData.aText.pUnicodeBuffer   =
            static_cast<sal_Unicode*>(malloc(PREEDIT_BUFSZ * sizeof(sal_Unicode)));
        maClientData.aText.pCharStyle       =
            static_cast<XIMFeedback*>(malloc(PREEDIT_BUFSZ * sizeof(XIMFeedback)));
        maClientData.aText.nSize            = PREEDIT_BUFSZ;
        maClientData.aText.nLength          = 0;

        // Status attributes

        switch ( mnStatusStyle )
        {
            case XIMStatusCallbacks:
            {
                static XIMCallback aStatusStartCallback;
                static XIMCallback aStatusDoneCallback;
                static XIMCallback aStatusDrawCallback;

                aStatusStartCallback.callback    = reinterpret_cast<XIMProc>(StatusStartCallback);
                aStatusStartCallback.client_data = reinterpret_cast<XPointer>(&maClientData);
                aStatusDoneCallback.callback     = reinterpret_cast<XIMProc>(StatusDoneCallback);
                aStatusDoneCallback.client_data  = reinterpret_cast<XPointer>(&maClientData);
                aStatusDrawCallback.callback     = reinterpret_cast<XIMProc>(StatusDrawCallback);
                aStatusDrawCallback.client_data  = reinterpret_cast<XPointer>(&maClientData);

                mpStatusAttributes = XVaCreateNestedList (
                                                          0,
                                                          XNStatusStartCallback, &aStatusStartCallback,
                                                          XNStatusDoneCallback,  &aStatusDoneCallback,
                                                          XNStatusDrawCallback,  &aStatusDrawCallback,
                                                          nullptr );

                break;
            }

            case XIMStatusArea:
                /* not supported */
                break;

            case XIMStatusNone:
            case XIMStatusNothing:
            default:
                /* no arguments needed */
                break;
        }

        // set preedit attributes

        switch ( mnPreeditStyle )
        {
            case XIMPreeditCallbacks:

                maPreeditCaretCallback.callback = reinterpret_cast<XIMProc>(PreeditCaretCallback);
                maPreeditStartCallback.callback = reinterpret_cast<XIMProc>(PreeditStartCallback);
                maPreeditDoneCallback.callback  = reinterpret_cast<XIMProc>(PreeditDoneCallback);
                maPreeditDrawCallback.callback  = reinterpret_cast<XIMProc>(PreeditDrawCallback);
                maPreeditCaretCallback.client_data = reinterpret_cast<XPointer>(&maClientData);
                maPreeditStartCallback.client_data = reinterpret_cast<XPointer>(&maClientData);
                maPreeditDoneCallback.client_data  = reinterpret_cast<XPointer>(&maClientData);
                maPreeditDrawCallback.client_data  = reinterpret_cast<XPointer>(&maClientData);

                mpPreeditAttributes = XVaCreateNestedList (
                                                           0,
                                                           XNPreeditStartCallback, &maPreeditStartCallback,
                                                           XNPreeditDoneCallback,  &maPreeditDoneCallback,
                                                           XNPreeditDrawCallback,   &maPreeditDrawCallback,
                                                           XNPreeditCaretCallback, &maPreeditCaretCallback,
                                                           nullptr );

                break;

            case XIMPreeditArea:
                /* not supported */
                break;

            case XIMPreeditPosition:
            {
                // spot location
                SalExtTextInputPosEvent aPosEvent;
                pFrame->CallCallback(SalEvent::ExtTextInputPos, static_cast<void*>(&aPosEvent));

                static XPoint aSpot;
                aSpot.x = aPosEvent.mnX + aPosEvent.mnWidth;
                aSpot.y = aPosEvent.mnY + aPosEvent.mnHeight;

                // create attributes for preedit position style
                mpPreeditAttributes = XVaCreateNestedList (
                                                           0,
                                                           XNSpotLocation, &aSpot,
                                                           nullptr );

                // XCreateIC() fails on Redflag Linux 2.0 if there is no
                // fontset though the data itself is not evaluated nor is
                // it required according to the X specs.
                Display* pDisplay = vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetDisplay();
                XFontSet pFontSet = get_font_set(pDisplay);

                if (pFontSet != nullptr)
                {
                    mpPreeditAttributes = XVaAddToNestedList( mpPreeditAttributes,
                                                              const_cast<char*>(XNFontSet), reinterpret_cast<XPointer>(pFontSet));
                }

                break;
            }

            case XIMPreeditNone:
            case XIMPreeditNothing:
            default:
                /* no arguments needed */
                break;
        }

        // Create the InputContext by giving it exactly the information it
        // deserves, because inappropriate attributes
        // let XCreateIC fail on Solaris (eg. for C locale)

        mpAttributes = XVaCreateNestedList(
                                           0,
                                           XNFocusWindow,       aFocusWindow,
                                           XNClientWindow,      aClientWindow,
                                           XNInputStyle,        mnPreeditStyle | mnStatusStyle,
                                           nullptr );

        if ( mnPreeditStyle != XIMPreeditNone )
        {
#if defined LINUX || defined FREEBSD || defined NETBSD || defined OPENBSD || defined DRAGONFLY
            if ( mpPreeditAttributes != nullptr )
#endif
                mpAttributes = XVaAddToNestedList( mpAttributes,
                                                   const_cast<char*>(XNPreeditAttributes), static_cast<XPointer>(mpPreeditAttributes) );
        }
        if ( mnStatusStyle != XIMStatusNone )
        {
#if defined LINUX || defined FREEBSD || defined NETBSD || defined OPENBSD || defined DRAGONFLY
            if ( mpStatusAttributes != nullptr )
#endif
                mpAttributes = XVaAddToNestedList( mpAttributes,
                                                   const_cast<char*>(XNStatusAttributes), static_cast<XPointer>(mpStatusAttributes) );
        }
        maContext = XCreateIC( pInputMethod->GetMethod(),
                               XNVaNestedList, mpAttributes,
                               nullptr );
    }

    if ( maContext == nullptr )
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN("vcl.app", "input context creation failed.");
#endif

        mbUseable = False;

        if ( mpAttributes != nullptr )
            XFree( mpAttributes );
        if ( mpStatusAttributes != nullptr )
            XFree( mpStatusAttributes );
        if ( mpPreeditAttributes != nullptr )
            XFree( mpPreeditAttributes );
        if ( maClientData.aText.pUnicodeBuffer != nullptr )
            free ( maClientData.aText.pUnicodeBuffer );
        if ( maClientData.aText.pCharStyle != nullptr )
            free ( maClientData.aText.pCharStyle );

        mpAttributes                      = nullptr;
        mpStatusAttributes                = nullptr;
        mpPreeditAttributes               = nullptr;
        maClientData.aText.pUnicodeBuffer = nullptr;
        maClientData.aText.pCharStyle     = nullptr;
    }

    if ( maContext != nullptr)
    {
        maDestroyCallback.callback    = IC_IMDestroyCallback;
        maDestroyCallback.client_data = reinterpret_cast<XPointer>(this);
        XSetICValues( maContext,
                      XNDestroyCallback,      &maDestroyCallback,
                      nullptr );
    }
}

// In Solaris 8 the status window does not unmap if the frame unmapps, so
// unmap it the hard way

void
SalI18N_InputContext::Unmap()
{
    UnsetICFocus();
    maClientData.pFrame = nullptr;
}

void
SalI18N_InputContext::Map( SalFrame *pFrame )
{
    if( !mbUseable )
        return;

    if( !pFrame )
        return;

    if ( maContext == nullptr )
    {
        SalI18N_InputMethod *pInputMethod;
        pInputMethod = vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetInputMethod();

        maContext = XCreateIC( pInputMethod->GetMethod(),
                               XNVaNestedList, mpAttributes,
                               nullptr );
    }
    if( maClientData.pFrame != pFrame )
        SetICFocus( pFrame );
}

// Handle DestroyCallbacks
// in fact this is a callback called from the XNDestroyCallback

void
SalI18N_InputContext::HandleDestroyIM()
{
    maContext = nullptr;      // don't change
    mbUseable = False;
}

//  make sure, the input method gets all the X-Events it needs, this is only
//  called once on each frame, it relies on a valid maContext

void
SalI18N_InputContext::ExtendEventMask( ::Window aFocusWindow )
{
    unsigned long nIMEventMask;
    XWindowAttributes aWindowAttributes;

    if ( mbUseable )
    {
        Display *pDisplay = XDisplayOfIM( XIMOfIC(maContext) );

        XGetWindowAttributes( pDisplay, aFocusWindow,
                              &aWindowAttributes );
        XGetICValues ( maContext,
                       XNFilterEvents, &nIMEventMask,
                       nullptr);
        nIMEventMask |= aWindowAttributes.your_event_mask;
        XSelectInput ( pDisplay, aFocusWindow, nIMEventMask );
    }
}

// tune the styles provided by the input method with the supported one

unsigned int
SalI18N_InputContext::GetWeightingOfIMStyle( XIMStyle nStyle )
{
    struct StyleWeightingT {
        const XIMStyle      nStyle;
        const unsigned int  nWeight;
    };

    StyleWeightingT const *pWeightPtr;
    static const StyleWeightingT pWeight[] = {
        { XIMPreeditCallbacks, 0x10000000 },
        { XIMPreeditPosition,  0x02000000 },
        { XIMPreeditArea,      0x01000000 },
        { XIMPreeditNothing,   0x00100000 },
        { XIMPreeditNone,      0x00010000 },
        { XIMStatusCallbacks,      0x1000 },
        { XIMStatusArea,           0x0100 },
        { XIMStatusNothing,        0x0010 },
        { XIMStatusNone,           0x0001 },
        { 0, 0x0 }
    };

    int nWeight = 0;
    for ( pWeightPtr = pWeight; pWeightPtr->nStyle != 0; pWeightPtr++ )
    {
        if ( (pWeightPtr->nStyle & nStyle) != 0 )
            nWeight += pWeightPtr->nWeight;
    }
    return nWeight;
}

bool
SalI18N_InputContext::IsSupportedIMStyle( XIMStyle nStyle ) const
{
    return (nStyle & mnSupportedPreeditStyle)
           && (nStyle & g_nSupportedStatusStyle);
}

bool
SalI18N_InputContext::SupportInputMethodStyle( XIMStyles const *pIMStyles )
{
    mnPreeditStyle = 0;
    mnStatusStyle  = 0;

    if ( pIMStyles != nullptr )
    {
        int nBestScore   = 0;
        int nActualScore = 0;

        // check whether the XIM supports one of the desired styles
        // only a single preedit and a single status style must occur
        // in an input method style. Hideki said so, so i trust him
        for ( int nStyle = 0; nStyle < pIMStyles->count_styles; nStyle++ )
        {
            XIMStyle nProvidedStyle = pIMStyles->supported_styles[ nStyle ];
            if ( IsSupportedIMStyle(nProvidedStyle) )
            {
                nActualScore = GetWeightingOfIMStyle( nProvidedStyle );
                if ( nActualScore >= nBestScore )
                {
                    nBestScore = nActualScore;
                    mnPreeditStyle = nProvidedStyle & mnSupportedPreeditStyle;
                    mnStatusStyle  = nProvidedStyle & g_nSupportedStatusStyle;
                }
            }
        }
    }

    return (mnPreeditStyle != 0) && (mnStatusStyle != 0) ;
}

// handle extended and normal key input

void
SalI18N_InputContext::CommitKeyEvent(sal_Unicode const * pText, std::size_t nLength)
{
    if (nLength == 1 && IsControlCode(pText[0]))
        return;

    if( maClientData.pFrame )
    {
        SalExtTextInputEvent aTextEvent;

        aTextEvent.mpTextAttr    = nullptr;
        aTextEvent.mnCursorPos   = nLength;
        aTextEvent.maText        = OUString(pText, nLength);
        aTextEvent.mnCursorFlags = 0;

        maClientData.pFrame->CallCallback(SalEvent::ExtTextInput,    static_cast<void*>(&aTextEvent));
        maClientData.pFrame->CallCallback(SalEvent::EndExtTextInput, nullptr);
    }
#if OSL_DEBUG_LEVEL > 1
    else
        SAL_WARN("vcl.app", "CommitKeyEvent without frame.");
#endif
}

int
SalI18N_InputContext::UpdateSpotLocation()
{
    if (maContext == nullptr || maClientData.pFrame == nullptr)
        return -1;

    SalExtTextInputPosEvent aPosEvent;
    maClientData.pFrame->CallCallback(SalEvent::ExtTextInputPos, static_cast<void*>(&aPosEvent));

    XPoint aSpot;
    aSpot.x = aPosEvent.mnX + aPosEvent.mnWidth;
    aSpot.y = aPosEvent.mnY + aPosEvent.mnHeight;

    XVaNestedList preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &aSpot, nullptr);
    XSetICValues(maContext, XNPreeditAttributes, preedit_attr, nullptr);
    XFree(preedit_attr);

    return 0;
}

// set and unset the focus for the Input Context
// the context may be NULL despite it is usable if the framewindow is
// in unmapped state

void
SalI18N_InputContext::SetICFocus( SalFrame* pFocusFrame )
{
    if ( !(mbUseable && (maContext != nullptr))  )
        return;

    maClientData.pFrame = pFocusFrame;

    const SystemEnvData* pEnv   = pFocusFrame->GetSystemData();
    ::Window  aClientWindow  = pEnv->aShellWindow;
    ::Window  aFocusWindow   = pEnv->GetWindowHandle(pFocusFrame);

    XSetICValues( maContext,
                  XNFocusWindow,       aFocusWindow,
                  XNClientWindow,      aClientWindow,
                  nullptr );

    if( maClientData.aInputEv.mpTextAttr )
    {
        sendEmptyCommit(pFocusFrame);
        // begin preedit again
        vcl_sal::getSalDisplay(GetGenericUnixSalData())->SendInternalEvent( pFocusFrame, &maClientData.aInputEv, SalEvent::ExtTextInput );
    }

    XSetICFocus( maContext );
}

void
SalI18N_InputContext::UnsetICFocus()
{

    if ( mbUseable && (maContext != nullptr) )
    {
        // cancel an eventual event posted to begin preedit again
        vcl_sal::getSalDisplay(GetGenericUnixSalData())->CancelInternalEvent( maClientData.pFrame, &maClientData.aInputEv, SalEvent::ExtTextInput );
        maClientData.pFrame = nullptr;
        XUnsetICFocus( maContext );
    }
}

// multi byte input method only

void
SalI18N_InputContext::EndExtTextInput()
{
    if ( !mbUseable || (maContext == nullptr) || !maClientData.pFrame )
        return;

    vcl::DeletionListener aDel( maClientData.pFrame );
    // delete preedit in sal (commit an empty string)
    sendEmptyCommit( maClientData.pFrame );
    if( ! aDel.isDeleted() )
    {
        // mark previous preedit state again (will e.g. be sent at focus gain)
        maClientData.aInputEv.mpTextAttr = maClientData.aInputFlags.data();
        if( static_cast<X11SalFrame*>(maClientData.pFrame)->hasFocus() )
        {
            // begin preedit again
            vcl_sal::getSalDisplay(GetGenericUnixSalData())->SendInternalEvent( maClientData.pFrame, &maClientData.aInputEv, SalEvent::ExtTextInput );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
