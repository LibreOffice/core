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
#include <string.h>
#include <iostream>

#ifdef LINUX
#  ifndef __USE_XOPEN
#    define __USE_XOPEN
#  endif
#endif

#include <X11/Xlib.h>

#include <unx/i18n_im.hxx>

#include <osl/thread.h>
#include <osl/process.h>
#include <sal/log.hxx>

#include <unx/i18n_cb.hxx>

// kinput2 IME needs special key handling since key release events are filtered in
// preeditmode and XmbResetIC does not work

namespace {

class XKeyEventOp : public XKeyEvent
{
    private:
        void            init();

    public:
                        XKeyEventOp();

        XKeyEventOp&    operator= (const XKeyEvent &rEvent);
        void            erase ();
        bool            match (const XKeyEvent &rEvent) const;
};

}

void
XKeyEventOp::init()
{
    type        = 0; /* serial = 0; */
    send_event  = 0; display   = nullptr;
    window      = 0; root      = 0;
    subwindow   = 0; /* time   = 0; */
 /* x           = 0; y         = 0; */
 /* x_root      = 0; y_root    = 0; */
    state       = 0; keycode   = 0;
    same_screen = 0;
}

XKeyEventOp::XKeyEventOp()
{
    init();
}

XKeyEventOp&
XKeyEventOp::operator= (const XKeyEvent &rEvent)
{
    type        = rEvent.type;     /* serial  = rEvent.serial; */
    send_event  = rEvent.send_event;  display = rEvent.display;
    window      = rEvent.window;      root    = rEvent.root;
    subwindow   = rEvent.subwindow;/* time    = rEvent.time;   */
 /* x           = rEvent.x,           y       = rEvent.y;      */
 /* x_root      = rEvent.x_root,      y_root  = rEvent.y_root; */
    state       = rEvent.state;       keycode = rEvent.keycode;
    same_screen = rEvent.same_screen;

    return *this;
}

void
XKeyEventOp::erase ()
{
    init();
}

bool
XKeyEventOp::match (const XKeyEvent &rEvent) const
{
    return (   (type == KeyPress   && rEvent.type == KeyRelease)
            || (type == KeyRelease && rEvent.type == KeyPress  ))
         /* && serial      == rEvent.serial */
            && send_event  == rEvent.send_event
            && display     == rEvent.display
            && window      == rEvent.window
            && root        == rEvent.root
            && subwindow   == rEvent.subwindow
         /* && time        == rEvent.time
            && x           == rEvent.x
            && y           == rEvent.y
            && x_root      == rEvent.x_root
            && y_root      == rEvent.y_root */
            && state       == rEvent.state
            && keycode     == rEvent.keycode
            && same_screen == rEvent.same_screen;
}

// locale handling

//  Locale handling of the operating system layer

static char*
SetSystemLocale( const char* p_inlocale )
{
    char *p_outlocale = setlocale(LC_ALL, p_inlocale);

    SAL_WARN_IF(p_outlocale == nullptr, "vcl.app",
            "I18N: Operating system doesn't support locale \""
            << p_inlocale << "\".");

    return p_outlocale;
}

#ifdef __sun
static void
SetSystemEnvironment( const OUString& rLocale )
{
    OUString LC_ALL_Var("LC_ALL");
    osl_setEnvironment(LC_ALL_Var.pData, rLocale.pData);

    OUString LANG_Var("LANG");
    osl_setEnvironment(LANG_Var.pData, rLocale.pData);
}
#endif

static Bool
IsPosixLocale( const char* p_locale )
{
    if ( p_locale == nullptr )
        return False;
    if ( (p_locale[ 0 ] == 'C') && (p_locale[ 1 ] == '\0') )
        return True;
    if ( strncmp(p_locale, "POSIX", sizeof("POSIX")) == 0 )
        return True;

    return False;
}

//  Locale handling of the X Window System layer

static Bool
IsXWindowCompatibleLocale( const char* p_locale )
{
    if ( p_locale == nullptr )
        return False;

    if ( !XSupportsLocale() )
    {
        SAL_WARN("vcl.app",
                "I18N: X Window System doesn't support locale \""
                << p_locale << "\".");
        return False;
    }
    return True;
}

// Set the operating system locale prior to trying to open an
// XIM InputMethod.
// Handle the cases where the current locale is either not supported by the
// operating system (LANG=gaga) or by the XWindow system (LANG=aa_ER@saaho)
// by providing a fallback.
// Upgrade "C" or "POSIX" to "en_US" locale to allow umlauts and accents
// see i8988, i9188, i8930, i16318
// on Solaris the environment needs to be set equivalent to the locale (#i37047#)

void
SalI18N_InputMethod::SetLocale()
{
    // check whether we want an Input Method engine, if we don't we
    // do not need to set the locale
    if ( !mbUseable )
        return;

    char *locale = SetSystemLocale( "" );
    if ( (!IsXWindowCompatibleLocale(locale)) || IsPosixLocale(locale) )
    {
        osl_setThreadTextEncoding (RTL_TEXTENCODING_ISO_8859_1);
        locale = SetSystemLocale( "en_US" );
#ifdef __sun
        SetSystemEnvironment( "en_US" );
#endif
        if (! IsXWindowCompatibleLocale(locale))
        {
            locale = SetSystemLocale( "C" );
#ifdef __sun
            SetSystemEnvironment( "C" );
#endif
            if (! IsXWindowCompatibleLocale(locale))
                mbUseable = False;
        }
    }

    // must not fail if mbUseable since XSupportsLocale() asserts success
    if ( mbUseable && XSetLocaleModifiers("") == nullptr )
    {
        SAL_WARN("vcl.app",
                "I18N: Can't set X modifiers for locale \""
                << locale << "\".");
        mbUseable = False;
    }
}

Bool
SalI18N_InputMethod::PosixLocale()
{
    if (maMethod)
        return IsPosixLocale (XLocaleOfIM (maMethod));
    return False;
}

// Constructor / Destructor / Initialisation

SalI18N_InputMethod::SalI18N_InputMethod( )
    : mbUseable( bUseInputMethodDefault )
    , maMethod( nullptr )
    , mpStyles( nullptr )
{
    maDestroyCallback.callback = nullptr;
    maDestroyCallback.client_data = nullptr;
    const char *pUseInputMethod = getenv( "SAL_USEINPUTMETHOD" );
    if ( pUseInputMethod != nullptr )
        mbUseable = pUseInputMethod[0] != '\0' ;
}

SalI18N_InputMethod::~SalI18N_InputMethod()
{
    if ( mpStyles != nullptr )
        XFree( mpStyles );
    if ( maMethod != nullptr )
        XCloseIM ( maMethod );
}

// XXX
// debug routine: lets have a look at the provided method styles

#if OSL_DEBUG_LEVEL > 1

extern "C" char*
GetMethodName( XIMStyle nStyle, char *pBuf, int nBufSize)
{
    struct StyleName {
        const XIMStyle nStyle;
        const char    *pName;
        const int      nNameLen;
    };

    StyleName *pDescPtr;
    static const StyleName pDescription[] = {
        { XIMPreeditArea,      "PreeditArea ",     sizeof("PreeditArea ")   },
        { XIMPreeditCallbacks, "PreeditCallbacks ",sizeof("PreeditCallbacks ")},
        { XIMPreeditPosition,  "PreeditPosition ", sizeof("PreeditPosition ") },
        { XIMPreeditNothing,   "PreeditNothing ",  sizeof("PreeditNothing ")  },
        { XIMPreeditNone,      "PreeditNone ",     sizeof("PreeditNone ")   },
        { XIMStatusArea,       "StatusArea ",      sizeof("StatusArea ")    },
        { XIMStatusCallbacks,  "StatusCallbacks ", sizeof("StatusCallbacks ") },
        { XIMStatusNothing,    "StatusNothing ",   sizeof("StatusNothing ") },
        { XIMStatusNone,       "StatusNone ",      sizeof("StatusNone ")    },
        { 0, "NULL", 0 }
    };

    if ( nBufSize > 0 )
        pBuf[0] = '\0';

    char *pBufPtr = pBuf;
    for ( pDescPtr = const_cast<StyleName*>(pDescription); pDescPtr->nStyle != 0; pDescPtr++ )
    {
        int nSize = pDescPtr->nNameLen - 1;
        if ( (nStyle & pDescPtr->nStyle) && (nBufSize > nSize) )
        {
            strncpy( pBufPtr, pDescPtr->pName, nSize + 1);
            pBufPtr  += nSize;
            nBufSize -= nSize;
        }
    }

    return pBuf;
}

extern "C" void
PrintInputStyle( XIMStyles *pStyle )
{
    char pBuf[ 128 ];
    int  nBuf = sizeof( pBuf );

    if ( pStyle == NULL )
        SAL_INFO("vcl.app", "no input method styles.");
    else
    for ( int nStyle = 0; nStyle < pStyle->count_styles; nStyle++ )
    {
        SAL_INFO("vcl.app", "style #"
                << nStyle
                << " = "
                << GetMethodName(pStyle->supported_styles[nStyle], pBuf, nBuf));
    }
}

#endif

// this is the real constructing routine, since locale setting has to be done
// prior to xopendisplay, the xopenim call has to be delayed

void
SalI18N_InputMethod::CreateMethod ( Display *pDisplay )
{
    if ( mbUseable )
    {
        maMethod = XOpenIM(pDisplay, nullptr, nullptr, nullptr);

        if ((maMethod == nullptr) && (getenv("XMODIFIERS") != nullptr))
        {
                OUString envVar(u"XMODIFIERS"_ustr);
                osl_clearEnvironment(envVar.pData);
                XSetLocaleModifiers("");
                maMethod = XOpenIM(pDisplay, nullptr, nullptr, nullptr);
        }

        if ( maMethod != nullptr )
        {
            if (   XGetIMValues(maMethod, XNQueryInputStyle, &mpStyles, nullptr)
                != nullptr)
                mbUseable = False;
#if OSL_DEBUG_LEVEL > 1
            SAL_INFO("vcl.app", "Creating Mono-Lingual InputMethod.");
            PrintInputStyle( mpStyles );
#endif
        }
        else
        {
            mbUseable = False;
        }
    }

#if OSL_DEBUG_LEVEL > 1
    SAL_WARN_IF(!mbUseable, "vcl.app", "input method creation failed.");
#endif

    maDestroyCallback.callback    = IM_IMDestroyCallback;
    maDestroyCallback.client_data = reinterpret_cast<XPointer>(this);
    if (mbUseable && maMethod != nullptr)
        XSetIMValues(maMethod, XNDestroyCallback, &maDestroyCallback, nullptr);
}

// give IM the opportunity to look at the event, and possibly hide it

bool
SalI18N_InputMethod::FilterEvent( XEvent *pEvent, ::Window window    )
{
    if (!mbUseable)
        return False;

    bool bFilterEvent = XFilterEvent (pEvent, window);

    if (pEvent->type != KeyPress && pEvent->type != KeyRelease)
        return bFilterEvent;

    /*
     * fix broken key release handling of some IMs
     */
    XKeyEvent*         pKeyEvent = &(pEvent->xkey);
    static XKeyEventOp s_aLastKeyPress;

    if (bFilterEvent)
    {
        if (pKeyEvent->type == KeyRelease)
            bFilterEvent = !s_aLastKeyPress.match (*pKeyEvent);
        s_aLastKeyPress.erase();
    }
    else /* (!bFilterEvent) */
    {
        if (pKeyEvent->type == KeyPress)
            s_aLastKeyPress = *pKeyEvent;
        else
            s_aLastKeyPress.erase();
    }

    return bFilterEvent;
}

void
SalI18N_InputMethod::HandleDestroyIM()
{
    mbUseable       = False;
    maMethod        = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
