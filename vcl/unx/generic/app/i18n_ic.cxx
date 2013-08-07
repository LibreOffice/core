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


#include <stdio.h>

#include <sal/alloca.h>
#include <osl/thread.h>

#include <prex.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <postx.h>

#include <unx/salunx.h>
#include <unx/XIM.h>
#include <unx/i18n_ic.hxx>
#include <unx/i18n_im.hxx>
#include <unx/i18n_status.hxx>

#include <unx/salframe.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>

using namespace vcl;

static void sendEmptyCommit( SalFrame* pFrame )
{
    vcl::DeletionListener aDel( pFrame );

    SalExtTextInputEvent aEmptyEv;
    aEmptyEv.mnTime             = 0;
    aEmptyEv.mpTextAttr         = 0;
    aEmptyEv.maText             = "";
    aEmptyEv.mnCursorPos        = 0;
    aEmptyEv.mnCursorFlags      = 0;
    aEmptyEv.mnDeltaStart       = 0;
    aEmptyEv.mbOnlyCursor       = False;
    pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEmptyEv );
    if( ! aDel.isDeleted() )
        pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, NULL );
}

// ---------------------------------------------------------------------------
//
// Constructor / Destructor, the InputContext is bound to the SalFrame, as it
// needs the shell window as a focus window
//
// ----------------------------------------------------------------------------

SalI18N_InputContext::~SalI18N_InputContext()
{
    if ( maContext != NULL )
        XDestroyIC( maContext );
    if ( mpAttributes != NULL )
        XFree( mpAttributes );
    if ( mpStatusAttributes != NULL )
        XFree( mpStatusAttributes );
    if ( mpPreeditAttributes != NULL )
        XFree( mpPreeditAttributes );

    if (maClientData.aText.pUnicodeBuffer != NULL)
        free(maClientData.aText.pUnicodeBuffer);
    if (maClientData.aText.pCharStyle != NULL)
        free(maClientData.aText.pCharStyle);
}

// ----------------------------------------------------------------------------
// convenience routine to add items to a XVaNestedList
// ----------------------------------------------------------------------------

static XVaNestedList
XVaAddToNestedList( XVaNestedList a_srclist, char* name, XPointer value )
{
    XVaNestedList a_dstlist;

    // if ( value == NULL )
    //  return a_srclist;

    if ( a_srclist == NULL )
    {
        a_dstlist = XVaCreateNestedList(
                                        0,
                                        name,           value,
                                        NULL );
    }
    else
    {
        a_dstlist = XVaCreateNestedList(
                                        0,
                                        XNVaNestedList, a_srclist,
                                        name,           value,
                                        NULL );
    }

    return a_dstlist != NULL ? a_dstlist : a_srclist ;
}

// ----------------------------------------------------------------------------
// convenience routine to create a fontset
// ----------------------------------------------------------------------------

static XFontSet
get_font_set( Display *p_display )
{
    static XFontSet p_font_set = NULL;

    if (p_font_set == NULL)
    {
        char **pp_missing_list;
        int    n_missing_count;
        char  *p_default_string;

        p_font_set = XCreateFontSet(p_display, "-*",
                                    &pp_missing_list, &n_missing_count, &p_default_string);
    }

    return p_font_set;
}

// ---------------------------------------------------------------------------
//
// Constructor for a InputContext (IC)
//
// ----------------------------------------------------------------------------

SalI18N_InputContext::SalI18N_InputContext ( SalFrame *pFrame ) :
        mbUseable( True ),
        maContext( (XIC)NULL ),
        mnSupportedStatusStyle(
                               XIMStatusCallbacks   |
                               XIMStatusNothing     |
                               XIMStatusNone
                               ),
        mnSupportedPreeditStyle(
                                XIMPreeditCallbacks |
                                XIMPreeditNothing   |
                                XIMPreeditNone
                                ),
        mnStatusStyle( 0 ),
        mnPreeditStyle( 0 ),
        mpAttributes( NULL ),
        mpStatusAttributes( NULL ),
        mpPreeditAttributes( NULL )
{
#ifdef SOLARIS
    static const char* pIIIMPEnable = getenv( "SAL_DISABLE_OWN_IM_STATUS" );
    if( pIIIMPEnable && *pIIIMPEnable )
        mnSupportedStatusStyle &= ~XIMStatusCallbacks;
#endif

    maClientData.aText.pUnicodeBuffer       = NULL;
    maClientData.aText.pCharStyle           = NULL;
    maClientData.aInputEv.mnTime            = 0;
    maClientData.aInputEv.mpTextAttr        = NULL;
    maClientData.aInputEv.mnCursorPos       = 0;
    maClientData.aInputEv.mnDeltaStart      = 0;
    maClientData.aInputEv.mnCursorFlags     = 0;
    maClientData.aInputEv.mbOnlyCursor      = sal_False;

    SalI18N_InputMethod *pInputMethod;
    pInputMethod = GetGenericData()->GetSalDisplay()->GetInputMethod();

    mnSupportedPreeditStyle =   XIMPreeditCallbacks | XIMPreeditPosition
        | XIMPreeditNothing   | XIMPreeditNone;
    if (pInputMethod->UseMethod()
        && SupportInputMethodStyle( pInputMethod->GetSupportedStyles() ) )
    {
        const SystemEnvData* pEnv = pFrame->GetSystemData();
        XLIB_Window  aClientWindow = pEnv->aShellWindow;
        XLIB_Window  aFocusWindow  = pEnv->aWindow;

        // for status callbacks and commit string callbacks
#define PREEDIT_BUFSZ 16
        maClientData.eState                 = ePreeditStatusStartPending;
        maClientData.pFrame                 = pFrame;
        maClientData.aText.pUnicodeBuffer   =
            (sal_Unicode*)malloc(PREEDIT_BUFSZ * sizeof(sal_Unicode));
        maClientData.aText.pCharStyle       =
            (XIMFeedback*)malloc(PREEDIT_BUFSZ * sizeof(XIMFeedback));
        maClientData.aText.nSize            = PREEDIT_BUFSZ;
        maClientData.aText.nCursorPos       = 0;
        maClientData.aText.nLength          = 0;

        //
        // Status attributes
        //

        switch ( mnStatusStyle )
        {
            case XIMStatusCallbacks:
            {
                static XIMCallback aStatusStartCallback;
                static XIMCallback aStatusDoneCallback;
                static XIMCallback aStatusDrawCallback;

                aStatusStartCallback.callback    = (XIMProc)StatusStartCallback;
                aStatusStartCallback.client_data = (XPointer)&maClientData;
                aStatusDoneCallback.callback     = (XIMProc)StatusDoneCallback;
                aStatusDoneCallback.client_data  = (XPointer)&maClientData;
                aStatusDrawCallback.callback     = (XIMProc)StatusDrawCallback;
                aStatusDrawCallback.client_data  = (XPointer)&maClientData;

                mpStatusAttributes = XVaCreateNestedList (
                                                          0,
                                                          XNStatusStartCallback, &aStatusStartCallback,
                                                          XNStatusDoneCallback,  &aStatusDoneCallback,
                                                          XNStatusDrawCallback,  &aStatusDrawCallback,
                                                          NULL );

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

        //
        // set preedit attributes
        //

        switch ( mnPreeditStyle )
        {
            case XIMPreeditCallbacks:

                maPreeditCaretCallback.callback = (XIMProc)PreeditCaretCallback;
                maPreeditStartCallback.callback = (XIMProc)PreeditStartCallback;
                maPreeditDoneCallback.callback  = (XIMProc)PreeditDoneCallback;
                maPreeditDrawCallback.callback  = (XIMProc)PreeditDrawCallback;
                maPreeditCaretCallback.client_data = (XPointer)&maClientData;
                maPreeditStartCallback.client_data = (XPointer)&maClientData;
                maPreeditDoneCallback.client_data  = (XPointer)&maClientData;
                maPreeditDrawCallback.client_data  = (XPointer)&maClientData;

                mpPreeditAttributes = XVaCreateNestedList (
                                                           0,
                                                           XNPreeditStartCallback, &maPreeditStartCallback,
                                                           XNPreeditDoneCallback,  &maPreeditDoneCallback,
                                                           XNPreeditDrawCallback,   &maPreeditDrawCallback,
                                                           XNPreeditCaretCallback, &maPreeditCaretCallback,
                                                           NULL );

                break;

            case XIMPreeditArea:
                /* not supported */
                break;

            case XIMPreeditPosition:
            {
                // spot location
                SalExtTextInputPosEvent aPosEvent;
                pFrame->CallCallback(SALEVENT_EXTTEXTINPUTPOS, (void*)&aPosEvent);

                static XPoint aSpot;
                aSpot.x = aPosEvent.mnX + aPosEvent.mnWidth;
                aSpot.y = aPosEvent.mnY + aPosEvent.mnHeight;

                // create attributes for preedit position style
                mpPreeditAttributes = XVaCreateNestedList (
                                                           0,
                                                           XNSpotLocation, &aSpot,
                                                           NULL );

                // XCreateIC() fails on Redflag Linux 2.0 if there is no
                // fontset though the data itself is not evaluated nor is
                // it required according to the X specs.
                Display* pDisplay = GetGenericData()->GetSalDisplay()->GetDisplay();
                XFontSet pFontSet = get_font_set(pDisplay);

                if (pFontSet != NULL)
                {
                    mpPreeditAttributes = XVaAddToNestedList( mpPreeditAttributes,
                                                              const_cast<char*>(XNFontSet), (XPointer)pFontSet);
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
                                           NULL );

        if ( mnPreeditStyle != XIMPreeditNone )
        {
#if defined LINUX || defined FREEBSD || defined NETBSD || defined OPENBSD || defined DRAGONFLY
            if ( mpPreeditAttributes != NULL )
#endif
                mpAttributes = XVaAddToNestedList( mpAttributes,
                                                   const_cast<char*>(XNPreeditAttributes), (XPointer)mpPreeditAttributes );
        }
        if ( mnStatusStyle != XIMStatusNone )
        {
#if defined LINUX || defined FREEBSD || defined NETBSD || defined OPENBSD || defined DRAGONFLY
            if ( mpStatusAttributes != NULL )
#endif
                mpAttributes = XVaAddToNestedList( mpAttributes,
                                                   const_cast<char*>(XNStatusAttributes), (XPointer)mpStatusAttributes );
        }
        maContext = XCreateIC( pInputMethod->GetMethod(),
                               XNVaNestedList, mpAttributes,
                               NULL );
    }

    if ( maContext == NULL )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf(stderr, "input context creation failed\n");
#endif

        mbUseable = False;

        if ( mpAttributes != NULL )
            XFree( mpAttributes );
        if ( mpStatusAttributes != NULL )
            XFree( mpStatusAttributes );
        if ( mpPreeditAttributes != NULL )
            XFree( mpPreeditAttributes );
        if ( maClientData.aText.pUnicodeBuffer != NULL )
            free ( maClientData.aText.pUnicodeBuffer );
        if ( maClientData.aText.pCharStyle != NULL )
            free ( maClientData.aText.pCharStyle );

        mpAttributes                      = NULL;
        mpStatusAttributes                = NULL;
        mpPreeditAttributes               = NULL;
        maClientData.aText.pUnicodeBuffer = NULL;
        maClientData.aText.pCharStyle     = NULL;
    }

    if ( maContext != NULL)
    {
        maDestroyCallback.callback    = (XIMProc)IC_IMDestroyCallback;
        maDestroyCallback.client_data = (XPointer)this;
        XSetICValues( maContext,
                      XNDestroyCallback,      &maDestroyCallback,
                      NULL );
    }
}

// ---------------------------------------------------------------------------
//
// In Solaris 8 the status window does not unmap if the frame unmapps, so
// unmap it the hard way
//
// ---------------------------------------------------------------------------

void
SalI18N_InputContext::Unmap( SalFrame* pFrame )
{
    if ( maContext != NULL )
    {
        I18NStatus& rStatus( I18NStatus::get() );
        if( rStatus.getParent() == pFrame )
            rStatus.show( false, I18NStatus::contextmap );

    }
    UnsetICFocus( pFrame );
    maClientData.pFrame = NULL;
}

void
SalI18N_InputContext::Map( SalFrame *pFrame )
{
    if( mbUseable )
    {
        I18NStatus& rStatus(I18NStatus::get() );
        rStatus.setParent( pFrame );
        if( pFrame )
        {
            rStatus.show( true, I18NStatus::contextmap );
            if ( maContext == NULL )
            {
                SalI18N_InputMethod *pInputMethod;
                pInputMethod = GetGenericData()->GetSalDisplay()->GetInputMethod();

                maContext = XCreateIC( pInputMethod->GetMethod(),
                                       XNVaNestedList, mpAttributes,
                                       NULL );
            }
            if( maClientData.pFrame != pFrame )
                SetICFocus( pFrame );
        }
    }
}

// --------------------------------------------------------------------------
//
// Handle DestroyCallbacks
// in fact this is a callback called from the XNDestroyCallback
//
// --------------------------------------------------------------------------

void
SalI18N_InputContext::HandleDestroyIM()
{
    maContext = 0;      // noli me tangere
    mbUseable = False;
}

// ---------------------------------------------------------------------------
//
//  make sure, the input method gets all the X-Events it needs, this is only
//  called once on each frame, it relys on a valid maContext
//
// ---------------------------------------------------------------------------

void
SalI18N_InputContext::ExtendEventMask( XLIB_Window aFocusWindow )
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
                       NULL);
        nIMEventMask |= aWindowAttributes.your_event_mask;
        XSelectInput ( pDisplay, aFocusWindow, nIMEventMask );
    }
}

// ---------------------------------------------------------------------------
//
// tune the styles provided by the input method with the supported one
//
// ---------------------------------------------------------------------------

unsigned int
SalI18N_InputContext::GetWeightingOfIMStyle( XIMStyle nStyle ) const
{
    struct StyleWeightingT {
        const XIMStyle      nStyle;
        const unsigned int  nWeight;
    };

    StyleWeightingT const *pWeightPtr;
    const StyleWeightingT pWeight[] = {
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

Bool
SalI18N_InputContext::IsSupportedIMStyle( XIMStyle nStyle ) const
{
    if (   (nStyle & mnSupportedPreeditStyle)
           && (nStyle & mnSupportedStatusStyle) )
    {
        return True;
    }
    return False;
}

Bool
SalI18N_InputContext::SupportInputMethodStyle( XIMStyles *pIMStyles )
{
    mnPreeditStyle = 0;
    mnStatusStyle  = 0;

    if ( pIMStyles != NULL )
    {
        int nBestScore   = 0;
        int nActualScore = 0;

        // check whether the XIM supports one of the desired styles
        // only a single preedit and a single status style must occure
        // in a inpuut method style. Hideki said so, so i trust him
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
                    mnStatusStyle  = nProvidedStyle & mnSupportedStatusStyle;
                }
            }
        }
    }

#if OSL_DEBUG_LEVEL > 1
    char pBuf[ 128 ];
    fprintf( stderr, "selected inputmethod style = %s\n",
             GetMethodName(mnPreeditStyle | mnStatusStyle, pBuf, sizeof(pBuf)) );
#endif

    return (mnPreeditStyle != 0) && (mnStatusStyle != 0) ;
}

// ---------------------------------------------------------------------------
//
// handle extended and normal key input
//
// ---------------------------------------------------------------------------

int
SalI18N_InputContext::CommitKeyEvent(sal_Unicode* pText, sal_Size nLength)
{
    if (nLength == 1 && IsControlCode(pText[0]))
        return 0;

    if( maClientData.pFrame )
    {
        SalExtTextInputEvent aTextEvent;

        aTextEvent.mnTime        = 0;
        aTextEvent.mpTextAttr    = 0;
        aTextEvent.mnCursorPos   = nLength;
        aTextEvent.maText        = OUString(pText, nLength);
        aTextEvent.mnCursorFlags = 0;
        aTextEvent.mnDeltaStart  = 0;
        aTextEvent.mbOnlyCursor  = False;

        maClientData.pFrame->CallCallback(SALEVENT_EXTTEXTINPUT,    (void*)&aTextEvent);
        maClientData.pFrame->CallCallback(SALEVENT_ENDEXTTEXTINPUT, (void*)NULL);
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf(stderr, "CommitKeyEvent without frame\n" );
#endif

    return 0;
}

int
SalI18N_InputContext::UpdateSpotLocation()
{
    if (maContext == 0 || maClientData.pFrame == NULL)
        return -1;

    SalExtTextInputPosEvent aPosEvent;
    maClientData.pFrame->CallCallback(SALEVENT_EXTTEXTINPUTPOS, (void*)&aPosEvent);

    XPoint aSpot;
    aSpot.x = aPosEvent.mnX + aPosEvent.mnWidth;
    aSpot.y = aPosEvent.mnY + aPosEvent.mnHeight;

    XVaNestedList preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &aSpot, NULL);
    XSetICValues(maContext, XNPreeditAttributes, preedit_attr, NULL);
    XFree(preedit_attr);

    I18NStatus::get().show( true, I18NStatus::contextmap );

    return 0;
}

// ---------------------------------------------------------------------------
//
// set and unset the focus for the Input Context
// the context may be NULL despite it is useable if the framewindow is
// in unmapped state
//
// ---------------------------------------------------------------------------

void
SalI18N_InputContext::SetICFocus( SalFrame* pFocusFrame )
{
    I18NStatus::get().setParent( pFocusFrame );
    if ( mbUseable && (maContext != NULL)  )
    {
        maClientData.pFrame = pFocusFrame;

        const SystemEnvData* pEnv   = pFocusFrame->GetSystemData();
        XLIB_Window  aClientWindow  = pEnv->aShellWindow;
        XLIB_Window  aFocusWindow   = pEnv->aWindow;

        XSetICValues( maContext,
                      XNFocusWindow,       aFocusWindow,
                      XNClientWindow,      aClientWindow,
                      NULL );

        if( maClientData.aInputEv.mpTextAttr )
        {
            sendEmptyCommit(pFocusFrame);
            // begin preedit again
            GetGenericData()->GetSalDisplay()->SendInternalEvent( pFocusFrame, &maClientData.aInputEv, SALEVENT_EXTTEXTINPUT );
        }

        XSetICFocus( maContext );
    }
}

void
SalI18N_InputContext::UnsetICFocus( SalFrame* pFrame )
{
    I18NStatus& rStatus( I18NStatus::get() );
    if( rStatus.getParent() == pFrame )
        rStatus.setParent( NULL );

    if ( mbUseable && (maContext != NULL) )
    {
        // cancel an eventual event posted to begin preedit again
        GetGenericData()->GetSalDisplay()->CancelInternalEvent( maClientData.pFrame, &maClientData.aInputEv, SALEVENT_EXTTEXTINPUT );
        maClientData.pFrame = NULL;
        XUnsetICFocus( maContext );
    }
}

// ---------------------------------------------------------------------------
//
// multi byte input method only
//
// ---------------------------------------------------------------------------

void
SalI18N_InputContext::SetLanguage(LanguageType)
{
    // not yet implemented
    return;
}

void
SalI18N_InputContext::EndExtTextInput( sal_uInt16 /*nFlags*/ )
{
    if ( mbUseable && (maContext != NULL) && maClientData.pFrame )
    {
        vcl::DeletionListener aDel( maClientData.pFrame );
        // delete preedit in sal (commit an empty string)
        sendEmptyCommit( maClientData.pFrame );
        if( ! aDel.isDeleted() )
        {
            // mark previous preedit state again (will e.g. be sent at focus gain)
            maClientData.aInputEv.mpTextAttr = &maClientData.aInputFlags[0];
            if( static_cast<X11SalFrame*>(maClientData.pFrame)->hasFocus() )
            {
                // begin preedit again
                GetGenericData()->GetSalDisplay()->SendInternalEvent( maClientData.pFrame, &maClientData.aInputEv, SALEVENT_EXTTEXTINPUT );
            }
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
