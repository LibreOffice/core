/*************************************************************************
 *
 *  $RCSfile: i18n_im.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2001-03-08 17:06:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>
#ifdef LINUX
#define __USE_XOPEN
#endif
#include <poll.h>

#include <prex.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <XIM.h>
#include <postx.h>

#include <salunx.h>
#include <saldisp.hxx>

#ifndef _SAL_I18N_INPUTMETHOD_HXX
#include "i18n_im.hxx"
#endif

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
        fprintf( stderr,
            "I18N: Operating system doesn't support locale \"%s\"\n",
            p_inlocale );
    }

    return p_outlocale;
}

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
        if ( p_locale != NULL )
            fprintf (stderr,
                "I18N: X Window System doesn't support locale \"%s\"\n",
                p_locale );
        return False;
    }
    return True;
}

// Locale setting for the Input Method
// allways provide a fallback, even if it means falling back to the
// portable POSIX "C" locale

Bool
SalI18N_InputMethod::SetLocale( const char* pLocale )
{
    // check whether we want an Input Method engine, if we don't we
    // do not need to set the locale
    if ( mbUseable )
    {
        char *locale;

        // check whether the operating system supports the LANG
        if ( (locale = SetSystemLocale( pLocale )) == NULL )
        {
            if ( (locale = SetSystemLocale( "C" )) == NULL )
                mbUseable = False;
        }

        // check whether the XWindow system supports the LANG
        if ( !IsXWindowCompatibleLocale(locale) )
        {
            if ( !IsPosixLocale(locale) )
            {
                locale = SetSystemLocale( "C" );
                if ( !IsXWindowCompatibleLocale(locale) )
                    mbUseable = False;
            }
            else
            {
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

// ------------------------------------------------------------------------
//
// Constructor / Destructor / Initialisation
//
// ------------------------------------------------------------------------

SalI18N_InputMethod::SalI18N_InputMethod( ) : maMethod( (XIM)NULL ),
                                                mpStyles( (XIMStyles*)NULL ),
                                                mbUseable( bUseInputMethodDefault ),
                                              mbMultiLingual( False )
{
    const char *pUseInputMethod = getenv( "SAL_USEINPUTMETHOD" );
    if ( pUseInputMethod != NULL )
        mbUseable = pUseInputMethod[0] != '\0' ;
}

SalI18N_InputMethod::~SalI18N_InputMethod()
{
    if ( mpStyles != NULL )
        XFree( mpStyles );
    if ( maMethod != NULL )
        XCloseIM ( maMethod );
}

//
// XXX
// debug routine: lets have a look at the provided method styles
//

#ifdef DEBUG

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
        if ( getenv("USE_XOPENIM") == NULL )
        {
            mbMultiLingual = True; // set ml-input flag to create input-method
            maMethod = XvaOpenIM(pDisplay, NULL, NULL, NULL,
                    XNMultiLingualInput, mbMultiLingual, /* dummy */
                     0);
            // get ml-input flag from input-method
            if ( maMethod == (XIM)NULL )
                mbMultiLingual = False;
            #if !defined(LINUX)
            else
            if ( XGetIMValues(maMethod,
                    XNMultiLingualInput, &mbMultiLingual, NULL ) != NULL )
                mbMultiLingual = False;
            #else
                mbMultiLingual = False;
            #endif
        }
        else
        {
            maMethod = XOpenIM(pDisplay, NULL, NULL, NULL);
            mbMultiLingual = False;
        }

        if ( maMethod != (XIM)NULL )
        {
            if (   XGetIMValues(maMethod, XNQueryInputStyle, &mpStyles, NULL)
                != NULL)
                mbUseable = False;
            #ifdef DEBUG
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

    #ifdef DEBUG
    if ( !mbUseable )
        fprintf(stderr, "input method creation failed\n");
    #endif

    return mbUseable;
}

//
// give IM the opportunity to look at the event, and possibly hide it
//

Bool
SalI18N_InputMethod::FilterEvent( XEvent *pEvent, XLIB_Window window    )
{
    return mbUseable && (Bool)XFilterEvent( pEvent, window );
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
    int nFileDescriptor, Bool bOpening, XPointer *pWatchData)
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
    if (!mbMultiLingual || !mbUseable)
        return False;

    // pConnectionHandler must be really a pointer to a SalXLib
    Status nStatus = XAddConnectionWatch (pDisplay, InputMethod_ConnectionWatchProc,
                                          (XPointer)pConnectionHandler);
    return (Bool)nStatus;
}



