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

#include <unistd.h>
#include <fcntl.h>

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#ifdef SUN
#include <sys/systeminfo.h>
#endif
#ifdef FREEBSD
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <osl/process.h>

#include <unx/saldisp.hxx>
#include <unx/saldata.hxx>
#include <unx/salunxtime.h>
#include <unx/sm.hxx>
#include <unx/i18n_im.hxx>

#include <X11/Xlib.h>
#include <X11/Xproto.h>

#include <salinst.hxx>
#include <saltimer.hxx>

#include <osl/diagnose.h>
#include <osl/signal.h>
#include <osl/thread.h>
#include <sal/log.hxx>

#include <vcl/svapp.hxx>

X11SalData* GetX11SalData()
{
    return static_cast<X11SalData*>(ImplGetSVData()->mpSalData);
}

extern "C" {

static int XErrorHdl( Display *pDisplay, XErrorEvent *pEvent )
{
    GetX11SalData()->XError( pDisplay, pEvent );
    return 0;
}

static int XIOErrorHdl( Display * )
{
    if ( Application::IsMainThread() )
    {
        /*  #106197# hack: until a real shutdown procedure exists
         *  _exit ASAP
         */
        if( ImplGetSVData()->maAppData.mbAppQuit )
            _exit(1);

        // really bad hack
        if( ! SessionManagerClient::checkDocumentsSaved() )
            /* oslSignalAction eToDo = */ osl_raiseSignal (OSL_SIGNAL_USER_X11SUBSYSTEMERROR, nullptr);
    }

    std::fprintf( stderr, "X IO Error\n" );
    std::fflush( stdout );
    std::fflush( stderr );

    /*  #106197# the same reasons to use _exit instead of exit in salmain
     *  do apply here. Since there is nothing to be done after an XIO
     *  error we have to _exit immediately.
     */
    _exit(1);
    return 0;
}

}

const struct timeval noyield_ = { 0, 0 };
const struct timeval yield_   = { 0, 10000 };

static const char* XRequest[] = {
    // see /usr/lib/X11/XErrorDB, /usr/openwin/lib/XErrorDB ...
    nullptr,
    "X_CreateWindow",
    "X_ChangeWindowAttributes",
    "X_GetWindowAttributes",
    "X_DestroyWindow",
    "X_DestroySubwindows",
    "X_ChangeSaveSet",
    "X_ReparentWindow",
    "X_MapWindow",
    "X_MapSubwindows",
    "X_UnmapWindow",
    "X_UnmapSubwindows",
    "X_ConfigureWindow",
    "X_CirculateWindow",
    "X_GetGeometry",
    "X_QueryTree",
    "X_InternAtom",
    "X_GetAtomName",
    "X_ChangeProperty",
    "X_DeleteProperty",
    "X_GetProperty",
    "X_ListProperties",
    "X_SetSelectionOwner",
    "X_GetSelectionOwner",
    "X_ConvertSelection",
    "X_SendEvent",
    "X_GrabPointer",
    "X_UngrabPointer",
    "X_GrabButton",
    "X_UngrabButton",
    "X_ChangeActivePointerGrab",
    "X_GrabKeyboard",
    "X_UngrabKeyboard",
    "X_GrabKey",
    "X_UngrabKey",
    "X_AllowEvents",
    "X_GrabServer",
    "X_UngrabServer",
    "X_QueryPointer",
    "X_GetMotionEvents",
    "X_TranslateCoords",
    "X_WarpPointer",
    "X_SetInputFocus",
    "X_GetInputFocus",
    "X_QueryKeymap",
    "X_OpenFont",
    "X_CloseFont",
    "X_QueryFont",
    "X_QueryTextExtents",
    "X_ListFonts",
    "X_ListFontsWithInfo",
    "X_SetFontPath",
    "X_GetFontPath",
    "X_CreatePixmap",
    "X_FreePixmap",
    "X_CreateGC",
    "X_ChangeGC",
    "X_CopyGC",
    "X_SetDashes",
    "X_SetClipRectangles",
    "X_FreeGC",
    "X_ClearArea",
    "X_CopyArea",
    "X_CopyPlane",
    "X_PolyPoint",
    "X_PolyLine",
    "X_PolySegment",
    "X_PolyRectangle",
    "X_PolyArc",
    "X_FillPoly",
    "X_PolyFillRectangle",
    "X_PolyFillArc",
    "X_PutImage",
    "X_GetImage",
    "X_PolyText8",
    "X_PolyText16",
    "X_ImageText8",
    "X_ImageText16",
    "X_CreateColormap",
    "X_FreeColormap",
    "X_CopyColormapAndFree",
    "X_InstallColormap",
    "X_UninstallColormap",
    "X_ListInstalledColormaps",
    "X_AllocColor",
    "X_AllocNamedColor",
    "X_AllocColorCells",
    "X_AllocColorPlanes",
    "X_FreeColors",
    "X_StoreColors",
    "X_StoreNamedColor",
    "X_QueryColors",
    "X_LookupColor",
    "X_CreateCursor",
    "X_CreateGlyphCursor",
    "X_FreeCursor",
    "X_RecolorCursor",
    "X_QueryBestSize",
    "X_QueryExtension",
    "X_ListExtensions",
    "X_ChangeKeyboardMapping",
    "X_GetKeyboardMapping",
    "X_ChangeKeyboardControl",
    "X_GetKeyboardControl",
    "X_Bell",
    "X_ChangePointerControl",
    "X_GetPointerControl",
    "X_SetScreenSaver",
    "X_GetScreenSaver",
    "X_ChangeHosts",
    "X_ListHosts",
    "X_SetAccessControl",
    "X_SetCloseDownMode",
    "X_KillClient",
    "X_RotateProperties",
    "X_ForceScreenSaver",
    "X_SetPointerMapping",
    "X_GetPointerMapping",
    "X_SetModifierMapping",
    "X_GetModifierMapping",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "X_NoOperation"
};

X11SalData::X11SalData()
    : GenericUnixSalData()
{
    pXLib_          = nullptr;

    m_aOrigXIOErrorHandler = XSetIOErrorHandler ( XIOErrorHdl );
    PushXErrorLevel( !!getenv( "SAL_IGNOREXERRORS" ) );
}

X11SalData::~X11SalData()
{
    DeleteDisplay();
    PopXErrorLevel();
    XSetIOErrorHandler (m_aOrigXIOErrorHandler);
}

void X11SalData::Dispose()
{
    delete GetDisplay();
    SetSalData( nullptr );
}

void X11SalData::DeleteDisplay()
{
    delete GetDisplay();
    SetDisplay( nullptr );
    pXLib_.reset();
}

void X11SalData::Init()
{
    pXLib_.reset(new SalXLib());
    pXLib_->Init();
}

void X11SalData::ErrorTrapPush()
{
    PushXErrorLevel( true );
}

bool X11SalData::ErrorTrapPop( bool bIgnoreError )
{
    bool err = false;
    if( !bIgnoreError )
        err = HasXErrorOccurred();
    ResetXErrorOccurred();
    PopXErrorLevel();
    return err;
}

void X11SalData::PushXErrorLevel( bool bIgnore )
{
    m_aXErrorHandlerStack.emplace_back( );
    XErrorStackEntry& rEnt = m_aXErrorHandlerStack.back();
    rEnt.m_bWas = false;
    rEnt.m_bIgnore = bIgnore;
    rEnt.m_aHandler = XSetErrorHandler( XErrorHdl );
}

void X11SalData::PopXErrorLevel()
{
    if( !m_aXErrorHandlerStack.empty() )
    {
        XSetErrorHandler( m_aXErrorHandlerStack.back().m_aHandler );
        m_aXErrorHandlerStack.pop_back();
    }
}

SalXLib::SalXLib()
{
    m_aTimeout.tv_sec       = 0;
    m_aTimeout.tv_usec      = 0;
    m_nTimeoutMS            = 0;

    nFDs_                   = 0;
    FD_ZERO( &aReadFDS_ );
    FD_ZERO( &aExceptionFDS_ );

    m_pInputMethod          = nullptr;
    m_pDisplay              = nullptr;

    m_pTimeoutFDS[0] = m_pTimeoutFDS[1] = -1;
    if (pipe (m_pTimeoutFDS) == -1)
        return;

    // initialize 'wakeup' pipe.
    int flags;

    // set close-on-exec descriptor flag.
    if ((flags = fcntl (m_pTimeoutFDS[0], F_GETFD)) != -1)
    {
        flags |= FD_CLOEXEC;
        (void)fcntl(m_pTimeoutFDS[0], F_SETFD, flags);
    }
    if ((flags = fcntl (m_pTimeoutFDS[1], F_GETFD)) != -1)
    {
        flags |= FD_CLOEXEC;
        (void)fcntl(m_pTimeoutFDS[1], F_SETFD, flags);
    }

    // set non-blocking I/O flag.
    if ((flags = fcntl (m_pTimeoutFDS[0], F_GETFL)) != -1)
    {
        flags |= O_NONBLOCK;
        (void)fcntl(m_pTimeoutFDS[0], F_SETFL, flags);
    }
    if ((flags = fcntl (m_pTimeoutFDS[1], F_GETFL)) != -1)
    {
        flags |= O_NONBLOCK;
        (void)fcntl(m_pTimeoutFDS[1], F_SETFL, flags);
    }

    // insert [0] into read descriptor set.
    FD_SET( m_pTimeoutFDS[0], &aReadFDS_ );
    nFDs_ = m_pTimeoutFDS[0] + 1;
}

SalXLib::~SalXLib()
{
    // close 'wakeup' pipe.
    close (m_pTimeoutFDS[0]);
    close (m_pTimeoutFDS[1]);

    m_pInputMethod.reset();
}

static Display *OpenX11Display(OString& rDisplay)
{
    /*
     * open connection to X11 Display
     * try in this order:
     *  o  -display command line parameter,
     *  o  $DISPLAY environment variable
     *  o  default display
     */

    Display *pDisp = nullptr;

    // is there a -display command line parameter?

    sal_uInt32 nParams = osl_getCommandArgCount();
    OUString aParam;
    for (sal_uInt32 i=0; i<nParams; i++)
    {
        osl_getCommandArg(i, &aParam.pData);
        if ( aParam == "-display" )
        {
            osl_getCommandArg(i+1, &aParam.pData);
            rDisplay = OUStringToOString(
                   aParam, osl_getThreadTextEncoding());

            if ((pDisp = XOpenDisplay(rDisplay.getStr()))!=nullptr)
            {
                /*
                 * if a -display switch was used, we need
                 * to set the environment accordingly since
                 * the clipboard build another connection
                 * to the xserver using $DISPLAY
                 */
                OUString envVar(u"DISPLAY"_ustr);
                osl_setEnvironment(envVar.pData, aParam.pData);
            }
            break;
        }
    }

    if (!pDisp && rDisplay.isEmpty())
    {
        // Open $DISPLAY or default...
        char *pDisplay = getenv("DISPLAY");
        if (pDisplay != nullptr)
            rDisplay = OString(pDisplay);
        pDisp  = XOpenDisplay(pDisplay);
    }

    return pDisp;
}

void SalXLib::Init()
{
    m_pInputMethod.reset( new SalI18N_InputMethod );
    m_pInputMethod->SetLocale();
    XrmInitialize();

    OString aDisplay;
    m_pDisplay = OpenX11Display(aDisplay);

    if ( m_pDisplay )
        return;

    OUString aProgramFileURL;
    osl_getExecutableFile( &aProgramFileURL.pData );
    OUString aProgramSystemPath;
    osl_getSystemPathFromFileURL (aProgramFileURL.pData, &aProgramSystemPath.pData);
    OString  aProgramName = OUStringToOString(
                                        aProgramSystemPath,
                                        osl_getThreadTextEncoding() );
    std::fprintf( stderr, "%s X11 error: Can't open display: %s\n",
            aProgramName.getStr(), aDisplay.getStr());
    std::fprintf( stderr, "   Set DISPLAY environment variable, use -display option\n");
    std::fprintf( stderr, "   or check permissions of your X-Server\n");
    std::fprintf( stderr, "   (See \"man X\" resp. \"man xhost\" for details)\n");
    std::fflush( stderr );
    exit(0);

}

extern "C" {
static void EmitFontpathWarning()
{
    static Bool bOnce = False;
    if ( !bOnce )
    {
        bOnce = True;
        std::fprintf( stderr, "Please verify your fontpath settings\n"
                "\t(See \"man xset\" for details"
                " or ask your system administrator)\n" );
    }
}

} /* extern "C" */

static void PrintXError( Display *pDisplay, XErrorEvent *pEvent )
{
    char msg[ 120 ] = "";
    XGetErrorText( pDisplay, pEvent->error_code, msg, sizeof( msg ) );
    std::fprintf( stderr, "X-Error: %s\n", msg );
    if( pEvent->request_code < SAL_N_ELEMENTS( XRequest ) )
    {
        const char* pName = XRequest[pEvent->request_code];
        if( !pName )
            pName = "BadRequest?";
        std::fprintf( stderr, "\tMajor opcode: %d (%s)\n", pEvent->request_code, pName );
    }
    else
    {
        std::fprintf( stderr, "\tMajor opcode: %d\n", pEvent->request_code );
        // TODO: also display extension name?
        std::fprintf( stderr, "\tMinor opcode: %d\n", pEvent->minor_code );
    }

    std::fprintf( stderr, "\tResource ID:  0x%lx\n",
             pEvent->resourceid );
    std::fprintf( stderr, "\tSerial No:    %ld (%ld)\n",
             pEvent->serial, LastKnownRequestProcessed(pDisplay) );

    if( !getenv( "SAL_SYNCHRONIZE" ) )
    {
        std::fprintf( stderr, "These errors are reported asynchronously,\n");
        std::fprintf( stderr, "set environment variable SAL_SYNCHRONIZE to 1 to help debugging\n");
    }

    std::fflush( stdout );
    std::fflush( stderr );
}

void X11SalData::XError( Display *pDisplay, XErrorEvent *pEvent )
{
    if( ! m_aXErrorHandlerStack.back().m_bIgnore )
    {
        if (   (pEvent->error_code   == BadAlloc)
            && (pEvent->request_code == X_OpenFont) )
        {
            static Bool bOnce = False;
            if ( !bOnce )
            {
                std::fprintf(stderr, "X-Error occurred in a request for X_OpenFont\n");
                EmitFontpathWarning();

                bOnce = True ;
            }
            return;
        }
        /* ignore
        * X_SetInputFocus: it's a hint only anyway
        * X_GetProperty: this is part of the XGetWindowProperty call and will
        *                be handled by the return value of that function
        */
        else if( pEvent->request_code == X_SetInputFocus ||
                 pEvent->request_code == X_GetProperty
            )
            return;

        if( pDisplay != vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetDisplay() )
            return;

        PrintXError( pDisplay, pEvent );

        oslSignalAction eToDo = osl_raiseSignal (OSL_SIGNAL_USER_X11SUBSYSTEMERROR, nullptr);
        switch (eToDo)
        {
            case osl_Signal_ActIgnore       :
                return;
            case osl_Signal_ActAbortApp     :
                abort();
            case osl_Signal_ActKillApp      :
                exit(0);
            case osl_Signal_ActCallNextHdl  :
                break;
            default :
                break;
        }

    }

    m_aXErrorHandlerStack.back().m_bWas = true;
}

void X11SalData::Timeout()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maSchedCtx.mpSalTimer )
        pSVData->maSchedCtx.mpSalTimer->CallCallback();
}

namespace {

struct YieldEntry
{
    int         fd;         // file descriptor for reading
    void*           data;       // data for predicate and callback
    YieldFunc       pending;    // predicate (determines pending events)
    YieldFunc       queued;     // read and queue up events
    YieldFunc       handle;     // handle pending events

    int  HasPendingEvent()   const { return pending( fd, data ); }
    int  IsEventQueued()     const { return queued( fd, data ); }
    void HandleNextEvent()   const { handle( fd, data ); }
};

}

#define MAX_NUM_DESCRIPTORS 128

static YieldEntry yieldTable[ MAX_NUM_DESCRIPTORS ];

void SalXLib::Insert( int nFD, void* data,
                      YieldFunc     pending,
                      YieldFunc     queued,
                      YieldFunc     handle )
{
    SAL_WARN_IF( !nFD, "vcl", "can not insert stdin descriptor" );
    SAL_WARN_IF( yieldTable[nFD].fd, "vcl", "SalXLib::Insert fd twice" );

    yieldTable[nFD].fd      = nFD;
    yieldTable[nFD].data    = data;
    yieldTable[nFD].pending = pending;
    yieldTable[nFD].queued  = queued;
    yieldTable[nFD].handle  = handle;

    FD_SET( nFD, &aReadFDS_ );
    FD_SET( nFD, &aExceptionFDS_ );

    if( nFD >= nFDs_ )
        nFDs_ = nFD + 1;
}

void SalXLib::Remove( int nFD )
{
    FD_CLR( nFD, &aReadFDS_ );
    FD_CLR( nFD, &aExceptionFDS_ );

    yieldTable[nFD].fd = 0;

    if ( nFD == nFDs_ )
    {
        for ( nFD = nFDs_ - 1;
              nFD >= 0 && !yieldTable[nFD].fd;
              nFD-- ) ;

        nFDs_ = nFD + 1;
    }
}

bool SalXLib::CheckTimeout( bool bExecuteTimers )
{
    bool bRet = false;
    if( m_aTimeout.tv_sec ) // timer is started
    {
        timeval aTimeOfDay;
        gettimeofday( &aTimeOfDay, nullptr );
        if( aTimeOfDay >= m_aTimeout )
        {
            bRet = true;
            if( bExecuteTimers )
            {
                // timed out, update timeout
                m_aTimeout = aTimeOfDay;
                /*
                *  #107827# autorestart immediately, will be stopped (or set
                *  to different value in notify hdl if necessary;
                *  CheckTimeout should return false while
                *  timers are being dispatched.
                */
                m_aTimeout += m_nTimeoutMS;
                // notify
                X11SalData::Timeout();
            }
        }
    }
    return bRet;
}

bool
SalXLib::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    // check for timeouts here if you want to make screenshots
    static char* p_prioritize_timer = getenv ("SAL_HIGHPRIORITY_REPAINT");
    bool bHandledEvent = false;
    if (p_prioritize_timer != nullptr)
        bHandledEvent = CheckTimeout();

    const int nMaxEvents = bHandleAllCurrentEvents ? 100 : 1;

    // first, check for already queued events.
    for ( int nFD = 0; nFD < nFDs_; nFD++ )
    {
        YieldEntry* pEntry = &(yieldTable[nFD]);
        if ( pEntry->fd )
        {
            SAL_WARN_IF( nFD != pEntry->fd, "vcl", "wrong fd in Yield()" );
            for( int i = 0; i < nMaxEvents && pEntry->HasPendingEvent(); i++ )
            {
                pEntry->HandleNextEvent();
                if( ! bHandleAllCurrentEvents )
                {
                    return true;
                }
            }
        }
    }

    // next, select with or without timeout according to bWait.
    int      nFDs         = nFDs_;
    fd_set   ReadFDS      = aReadFDS_;
    fd_set   ExceptionFDS = aExceptionFDS_;
    int      nFound       = 0;

    timeval  Timeout      = noyield_;
    timeval *pTimeout     = &Timeout;


    if (bWait)
    {
        pTimeout = nullptr;
        if (m_aTimeout.tv_sec) // Timer is started.
        {
            // determine remaining timeout.
            gettimeofday (&Timeout, nullptr);
            Timeout = m_aTimeout - Timeout;
            if (yield_ >= Timeout)
            {
                // guard against micro timeout.
                Timeout = yield_;
            }
            pTimeout = &Timeout;
        }
    }

    {
        // release YieldMutex (and re-acquire at block end)
        SolarMutexReleaser aReleaser;
        nFound = select( nFDs, &ReadFDS, nullptr, &ExceptionFDS, pTimeout );
    }
    if( nFound < 0 ) // error
    {
#ifdef DBG_UTIL
        SAL_INFO("vcl.app", "SalXLib::Yield e=" << errno << " f=" << nFound);
#endif
        if( EINTR == errno )
        {
            errno = 0;
        }
    }

    // usually handle timeouts here (as in 5.2)
    if (p_prioritize_timer == nullptr)
        bHandledEvent = CheckTimeout() || bHandledEvent;

    // handle wakeup events.
    if ((nFound > 0) && FD_ISSET(m_pTimeoutFDS[0], &ReadFDS))
    {
        int buffer;
        while (read (m_pTimeoutFDS[0], &buffer, sizeof(buffer)) > 0)
            continue;
        nFound -= 1;
    }

    // handle other events.
    if( nFound > 0 )
    {
        // now we are in the protected section !
        // recall select if we have acquired fd's, ready for reading,

        struct timeval noTimeout = { 0, 0 };
        nFound = select( nFDs_, &ReadFDS, nullptr,
                         &ExceptionFDS, &noTimeout );

        // someone-else has done the job for us
        if (nFound == 0)
        {
            return false;
        }

        for ( int nFD = 0; nFD < nFDs_; nFD++ )
        {
            YieldEntry* pEntry = &(yieldTable[nFD]);
            if ( pEntry->fd )
            {
                if ( FD_ISSET( nFD, &ExceptionFDS ) ) {
#if OSL_DEBUG_LEVEL > 1
                    SAL_WARN("vcl.app", "SalXLib::Yield exception.");
#endif
                    nFound--;
                }
                if ( FD_ISSET( nFD, &ReadFDS ) )
                {
                    for( int i = 0; pEntry->IsEventQueued() && i < nMaxEvents; i++ )
                    {
                        pEntry->HandleNextEvent();
                        bHandledEvent = true;
                        // if a recursive call has done the job
                        // so abort here
                    }
                    nFound--;
                }
            }
        }
    }

    return bHandledEvent;
}

void SalXLib::Wakeup()
{
    OSL_VERIFY(write (m_pTimeoutFDS[1], "", 1) == 1);
}

void SalXLib::TriggerUserEventProcessing()
{
    Wakeup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
