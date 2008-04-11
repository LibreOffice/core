/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: i18n_im.cxx,v $
 * $Revision: 1.39 $
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
#include "precompiled_vcl.hxx"

#include <stdio.h>
#include <string.h>
#ifdef LINUX
#  ifndef __USE_XOPEN
#    define __USE_XOPEN
#  endif
#endif
#include <poll.h>
#ifdef SOLARIS
// for SetSystemEnvironment()
#include <sal/alloca.h>
#endif

#include <prex.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <XIM.h>
#include <postx.h>

#include <salunx.h>
#include <saldisp.hxx>
#include "i18n_im.hxx"
#include <i18n_status.hxx>

#ifdef MACOSX
#include <osl/process.h>
#include <tools/string.hxx>
#endif
#include <osl/thread.h>

using namespace vcl;
#include "i18n_cb.hxx"
#if defined(SOLARIS) ||  defined(LINUX) || defined(IRIX)
extern "C" char * XSetIMValues(XIM im, ...);
#endif

// ------------------------------------------------------------------------------------
//
// kinput2 IME needs special key handling since key release events are filtered in
// preeditmode and XmbResetIC does not work
//
// ------------------------------------------------------------------------------------

Bool
IMServerKinput2 ()
{
    const static char* p_xmodifiers = getenv ("XMODIFIERS");
    const static Bool  b_kinput2    =    (p_xmodifiers != NULL)
                                      && (strcmp(p_xmodifiers, "@im=kinput2") == 0);

    return b_kinput2;
}

class XKeyEventOp : XKeyEvent
{
    private:
        void            init();

    public:
                        XKeyEventOp();
                        ~XKeyEventOp();

        XKeyEventOp&    operator= (const XKeyEvent &rEvent);
        void            erase ();
        Bool            match (const XKeyEvent &rEvent) const;
};

void
XKeyEventOp::init()
{
    type        = 0; /* serial = 0; */
    send_event  = 0; display   = 0;
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

XKeyEventOp::~XKeyEventOp()
{
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

Bool
XKeyEventOp::match (const XKeyEvent &rEvent) const
{
    return (   (type == XLIB_KeyPress   && rEvent.type == KeyRelease)
            || (type == KeyRelease && rEvent.type == XLIB_KeyPress  ))
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

// -------------------------------------------------------------------------
//
// locale handling
//
// -------------------------------------------------------------------------

//  Locale handling of the operating system layer

static char*
SetSystemLocale( const char* p_inlocale )
{
    char *p_outlocale;

    if ( (p_outlocale = setlocale(LC_ALL, p_inlocale)) == NULL )
    {
        fprintf( stderr, "I18N: Operating system doesn't support locale \"%s\"\n",
            p_inlocale );
    }

    return p_outlocale;
}

#ifdef SOLARIS
static void
SetSystemEnvironment( const char* p_locale )
{
    const char *lc_all = "LC_ALL=%s";
    const char *lang   = "LANG=%s";

    char *p_buffer;

    if (p_locale != NULL)
    {
        p_buffer = (char*)alloca(10 + strlen(p_locale));
        sprintf(p_buffer, lc_all, p_locale);
        putenv(strdup(p_buffer));
        sprintf(p_buffer, lang, p_locale);
        putenv(strdup(p_buffer));
    }
}
#endif

static Bool
IsPosixLocale( const char* p_locale )
{
    if ( p_locale == NULL )
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
    if ( p_locale == NULL )
        return False;

    if ( !XSupportsLocale() )
    {
        fprintf (stderr, "I18N: X Window System doesn't support locale \"%s\"\n",
                p_locale );
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

Bool
SalI18N_InputMethod::SetLocale( const char* pLocale )
{
    // check whether we want an Input Method engine, if we don't we
    // do not need to set the locale
    if ( mbUseable )
    {
        char *locale = SetSystemLocale( pLocale );
        if ( (!IsXWindowCompatibleLocale(locale)) || IsPosixLocale(locale) )
        {
            #ifdef MACOSX // MacOS X always uses UTF-8 for the filesystem
            osl_setThreadTextEncoding (RTL_TEXTENCODING_UTF8);
            locale = SetSystemLocale( "en_US.UTF-8" );
            #else
            osl_setThreadTextEncoding (RTL_TEXTENCODING_ISO_8859_1);
            locale = SetSystemLocale( "en_US" );
            #endif
            #ifdef SOLARIS
            SetSystemEnvironment( "en_US" );
            #endif
            if (! IsXWindowCompatibleLocale(locale))
            {
                locale = SetSystemLocale( "C" );
                #ifdef SOLARIS
                SetSystemEnvironment( "C" );
                #endif
                if (! IsXWindowCompatibleLocale(locale))
                    mbUseable = False;
            }
        }

        // must not fail if mbUseable since XSupportsLocale() asserts success
        if ( mbUseable && XSetLocaleModifiers("") == NULL )
        {
            fprintf (stderr, "I18N: Can't set X modifiers for locale \"%s\"\n",
                locale);
            mbUseable = False;
        }
    }

    return mbUseable;
}

Bool
SalI18N_InputMethod::PosixLocale()
{
    if (mbMultiLingual)
        return False;
    if (maMethod)
        return IsPosixLocale (XLocaleOfIM (maMethod));
    return False;
}

// ------------------------------------------------------------------------
//
// Constructor / Destructor / Initialisation
//
// ------------------------------------------------------------------------

SalI18N_InputMethod::SalI18N_InputMethod( ) : mbUseable( bUseInputMethodDefault ),
                                              mbMultiLingual( False ),
                                              maMethod( (XIM)NULL ),
                                                mpStyles( (XIMStyles*)NULL )
{
    const char *pUseInputMethod = getenv( "SAL_USEINPUTMETHOD" );
    if ( pUseInputMethod != NULL )
        mbUseable = pUseInputMethod[0] != '\0' ;
}

SalI18N_InputMethod::~SalI18N_InputMethod()
{
    ::vcl::I18NStatus::free();
    if ( mpStyles != NULL )
        XFree( mpStyles );
    if ( maMethod != NULL )
        XCloseIM ( maMethod );
}

//
// XXX
// debug routine: lets have a look at the provided method styles
//

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
        fprintf( stderr, "no input method styles\n");
    else
    for ( int nStyle = 0; nStyle < pStyle->count_styles; nStyle++ )
    {
        fprintf( stderr, "style #%i = %s\n", nStyle,
              GetMethodName(pStyle->supported_styles[nStyle], pBuf, nBuf) );
    }
}

#endif

//
// this is the real constructing routine, since locale setting has to be done
// prior to xopendisplay, the xopenim call has to be delayed
//

Bool
SalI18N_InputMethod::CreateMethod ( Display *pDisplay )
{
    if ( mbUseable )
    {
        const bool bTryMultiLingual =
        #ifdef LINUX
                        false;
        #else
                        true;
        #endif
        if ( bTryMultiLingual && getenv("USE_XOPENIM") == NULL )
        {
            mbMultiLingual = True; // set ml-input flag to create input-method
            maMethod = XvaOpenIM(pDisplay, NULL, NULL, NULL,
                    XNMultiLingualInput, mbMultiLingual, /* dummy */
                     (void *)0);
            // get ml-input flag from input-method
            if ( maMethod == (XIM)NULL )
                mbMultiLingual = False;
            else
            if ( XGetIMValues(maMethod,
                    XNMultiLingualInput, &mbMultiLingual, NULL ) != NULL )
                mbMultiLingual = False;
            if( mbMultiLingual )
            {
                XIMUnicodeCharacterSubsets* subsets;
                if( XGetIMValues( maMethod,
                                  XNQueryUnicodeCharacterSubset, &subsets, NULL ) == NULL )
                {
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "IM reports %d subsets: ", subsets->count_subsets );
#endif
                    I18NStatus& rStatus( I18NStatus::get() );
                    rStatus.clearChoices();
                    for( int i = 0; i < subsets->count_subsets; i++ )
                    {
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr,"\"%s\" ", subsets->supported_subsets[i].name );
#endif
                        rStatus.addChoice( String( subsets->supported_subsets[i].name, RTL_TEXTENCODING_UTF8 ), &subsets->supported_subsets[i] );
                    }
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "\n" );
#endif
                }
#if OSL_DEBUG_LEVEL > 1
                else
                    fprintf( stderr, "query subsets failed\n" );
#endif
            }
        }
        else
        {
            maMethod = XOpenIM(pDisplay, NULL, NULL, NULL);
            mbMultiLingual = False;
        }

        if ((maMethod == (XIM)NULL) && (getenv("XMODIFIERS") != NULL))
        {
                putenv (strdup("XMODIFIERS"));
                XSetLocaleModifiers("");
                maMethod = XOpenIM(pDisplay, NULL, NULL, NULL);
                mbMultiLingual = False;
        }

        if ( maMethod != (XIM)NULL )
        {
            if (   XGetIMValues(maMethod, XNQueryInputStyle, &mpStyles, NULL)
                != NULL)
                mbUseable = False;
            #if OSL_DEBUG_LEVEL > 1
            fprintf(stderr, "Creating %s-Lingual InputMethod\n",
                mbMultiLingual ? "Multi" : "Mono" );
            PrintInputStyle( mpStyles );
            #endif
        }
        else
        {
            mbUseable = False;
        }
    }

    #if OSL_DEBUG_LEVEL > 1
    if ( !mbUseable )
        fprintf(stderr, "input method creation failed\n");
    #endif

    maDestroyCallback.callback    = (XIMProc)IM_IMDestroyCallback;
    maDestroyCallback.client_data = (XPointer)this;
    if (mbUseable && maMethod != NULL)
        XSetIMValues(maMethod, XNDestroyCallback, &maDestroyCallback, NULL);

    return mbUseable;
}

//
// give IM the opportunity to look at the event, and possibly hide it
//

Bool
SalI18N_InputMethod::FilterEvent( XEvent *pEvent, XLIB_Window window    )
{
    if (!mbUseable)
        return False;

    Bool bFilterEvent = XFilterEvent (pEvent, window);

    if (pEvent->type != XLIB_KeyPress && pEvent->type != KeyRelease)
        return bFilterEvent;

    /*
     * fix broken key release handling of some IMs
     */
    XKeyEvent*         pKeyEvent = &(pEvent->xkey);
    static XKeyEventOp maLastKeyPress;

    if (bFilterEvent)
    {
        if (pKeyEvent->type == KeyRelease)
            bFilterEvent = !maLastKeyPress.match (*pKeyEvent);
        maLastKeyPress.erase();
    }
    else /* (!bFilterEvent) */
    {
        if (pKeyEvent->type == XLIB_KeyPress)
            maLastKeyPress = *pKeyEvent;
        else
            maLastKeyPress.erase();
    }

    return bFilterEvent;
}

void
SalI18N_InputMethod::HandleDestroyIM()
{
    mbUseable       = False;
    mbMultiLingual  = False;
    maMethod        = NULL;
}

// ------------------------------------------------------------------------
//
// add a connection watch into the SalXLib yieldTable to allow iiimp
// connection processing: soffice waits in select() not in XNextEvent(), so
// there may be requests pending on the iiimp internal connection that will
// not be processed until XNextEvent is called the next time. If we do not
// have the focus because the atok12 lookup choice aux window has it we stay
// deaf and dump otherwise.
//
// ------------------------------------------------------------------------

int
InputMethod_HasPendingEvent(int nFileDescriptor, void *pData)
{
    if (pData == NULL)
        return 0;

    struct pollfd aFileDescriptor;
    #ifdef SOLARIS
    nfds_t        nNumDescriptor = 1;
    #else
    unsigned int      nNumDescriptor = 1;
    #endif
    aFileDescriptor.fd      = nFileDescriptor;
    aFileDescriptor.events  = POLLRDNORM;
    aFileDescriptor.revents = 0;

    int nPoll = poll (&aFileDescriptor, nNumDescriptor, 0 /* timeout */ );

    if (nPoll > 0)
    {
        /* at least some conditions in revent are set */
        if (   (aFileDescriptor.revents & POLLHUP)
            || (aFileDescriptor.revents & POLLERR)
            || (aFileDescriptor.revents & POLLNVAL))
            return 0; /* oops error condition set */

        if (aFileDescriptor.revents & POLLRDNORM)
            return 1; /* success */
    }

    /* nPoll == 0 means timeout, nPoll < 0 means error */
    return 0;
}

int
InputMethod_IsEventQueued(int nFileDescriptor, void *pData)
{
    return InputMethod_HasPendingEvent (nFileDescriptor, pData);
}

int
InputMethod_HandleNextEvent(int nFileDescriptor, void *pData)
{
    if (pData != NULL)
        XProcessInternalConnection((Display*)pData, nFileDescriptor);

    return 0;
}

extern "C" void
InputMethod_ConnectionWatchProc (Display *pDisplay, XPointer pClientData,
    int nFileDescriptor, Bool bOpening, XPointer*)
{
    SalXLib *pConnectionHandler = (SalXLib*)pClientData;

    if (pConnectionHandler == NULL)
        return;

    if (bOpening)
    {
        pConnectionHandler->Insert (nFileDescriptor, pDisplay,
                                    InputMethod_HasPendingEvent,
                                    InputMethod_IsEventQueued,
                                    InputMethod_HandleNextEvent);
    }
    else
    {
        pConnectionHandler->Remove (nFileDescriptor);
    }
}

Bool
SalI18N_InputMethod::AddConnectionWatch(Display *pDisplay, void *pConnectionHandler)
{
    // sanity check
    if (pDisplay == NULL || pConnectionHandler == NULL)
        return False;

    // if we are not ml all the extended text input comes on the stock X queue,
    // so there is no need to monitor additional file descriptors.
#ifndef SOLARIS
     if (!mbMultiLingual || !mbUseable)
         return False;
#endif

    // pConnectionHandler must be really a pointer to a SalXLib
    Status nStatus = XAddConnectionWatch (pDisplay, InputMethod_ConnectionWatchProc,
                                          (XPointer)pConnectionHandler);
    return (Bool)nStatus;
}



