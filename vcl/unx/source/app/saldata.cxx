/*************************************************************************
 *
 *  $RCSfile: saldata.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-22 10:11:55 $
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

#define _SV_SALDATA_CXX

#ifdef USE_XTOOLKIT
#  define SAL_XT
#endif

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include <sys/resource.h>
#ifdef SUN
#include <sys/systeminfo.h>
#endif
#ifdef AIX
#include <strings.h>
#endif
#ifdef FREEBSD
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#include <prex.h>

// [ed] 6/15/02 There's a conflicting definition of INT8 within the Xmd.h header
// and the solar.h OOo header.  So, wrap the X11 header with a bogus #define
// to use the OOo definition of the symbol for INT8.

#ifdef MACOSX
#define INT8 blehBlahFooBar
#endif

#include <X11/Xproto.h>

// [ed] 6/15/02 Get rid of INT8 hack

#ifdef MACOSX
#undef INT8
#endif

#include <postx.h>

#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _OSL_SIGNAL_H_
#include <osl/signal.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#include <tools/debug.hxx>
#include <sm.hxx>

#ifndef _SAL_I18N_INPUTMETHOD_HXX
#include "i18n_im.hxx"
#endif
#ifndef _SAL_I18N_XKBDEXTENSION_HXX
#include "i18n_xkb.hxx"
#endif

// -=-= <signal.h> -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef UNX
#ifndef SIGBUS
#define SIGBUS 10
#endif
#ifndef SIGSEGV
#define SIGSEGV 11
#endif
#ifndef SIGIOT
#define SIGIOT SIGABRT
#endif
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const struct timeval noyield__ = { 0, 0 };
static const struct timeval yield__   = { 0, 10000 };

static const char* XRequest[] = {
    // see /usr/lib/X11/XErrorDB, /usr/openwin/lib/XErrorDB ...
    NULL,
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
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "X_NoOperation"
};

BEGIN_C
// -=-= C statics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


static oslSignalAction SalSignalHdl (void* pData, oslSignalInfo* pInfo)
{
    ULONG nException = 0;

    switch (pInfo->Signal)
    {
        case osl_Signal_System :
            return osl_Signal_ActCallNextHdl;
        case osl_Signal_Terminate :
            if (!GetSalData()->Close())
                return osl_Signal_ActIgnore;
            break;
        case osl_Signal_User :
            return osl_Signal_ActCallNextHdl;
        default: break;
    }

    return osl_Signal_ActAbortApp;
}


static int sal_XErrorHdl( Display *pDisplay, XErrorEvent *pEvent )
{
    GetSalData()->XError( pDisplay, pEvent );
    return 0;
}

static int sal_XIOErrorHdl( Display *pDisplay )
{
    /*  #106197# hack: until a real shutdown procedure exists
     *  _exit ASAP
     */
    if( ImplGetSVData()->maAppData.mbAppQuit )
        _exit(1);

    // really bad hack
    if( ! SessionManagerClient::checkDocumentsSaved() )
        oslSignalAction eToDo = osl_raiseSignal (OSL_SIGNAL_USER_X11SUBSYSTEMERROR, NULL);

    fprintf( stderr, "X IO Error\n" );
    fflush( stdout );
    fflush( stderr );

    /*  #106197# the same reasons to use _exit instead of exit in salmain
     *  do apply here. Since there is nothing to be done after an XIO
     *  error we have to _exit immediately.
     */
    _exit(0);
    return 0;
}

END_C

// -=-= SalData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <pthread.h>

SalData::SalData()
{
    argv_           = 0;
    argc_           = 0;

    pTimerProc_     = 0;

    memset( sig_, 0, sizeof( sig_ ) ); // SIG_DFL
    bNoExceptions_  = !!getenv( "SAL_NOSEGV" );

    pXLib_          = new SalXLib();
    pDefDisp_       = 0;
    pCurDisp_       = 0;

    hMainThread_    = pthread_self();

    pFirstInstance_ = NULL;
    pFirstFrame_    = NULL;
}

SalData::~SalData()
{
    while( SalDisplays_.Count() )
        delete SalDisplays_.Remove( (ULONG)0 );

    delete pXLib_;
    pDefDisp_ = NULL;
    pCurDisp_ = NULL;
}

long SalData::Close() const
{
    signal( SIGTERM, sig_[SIGTERM] );
    if( !pFirstFrame_ )
        return 1;

    SalFrame *pFrame = pFirstFrame_;
    while( pFrame )
    {
        if( !pFrame->maFrameData.Close() )
            return 0;
        pFrame = pFrame->maFrameData.GetNextFrame();
    }
    return 1;
}

long SalData::ShutDown() const
{
    if( !pFirstFrame_ )
        return 1;

    SalFrame *pFrame = pFirstFrame_;
    while( pFrame )
    {
        if( !pFrame->maFrameData.ShutDown() )
            return 0;
        pFrame = pFrame->maFrameData.GetNextFrame();
    }
    return 1;
}

XubString SalData::GetCommandLineParam( USHORT nParam ) const
{
    if( !nParam ) { return aBinaryPath_; }
    if( nParam >= argc_ ) return String();
    return String( argv_[nParam], osl_getThreadTextEncoding() );
}

SalDisplay *SalData::GetDisplay( Display *pDisplay )
{
    SalDisplay *pSalDisplay = SalDisplays_.First();
    while( pSalDisplay && pSalDisplay->GetDisplay() != pDisplay )
        pSalDisplay = SalDisplays_.Next();
    return pSalDisplay;
}

void SalData::Init( int *pArgc, char *ppArgv[] )
{
    // Pfad zum Executable bestimmen
    char aFilePath[ PATH_MAX ];
    ::rtl::OUString aPath;
    osl_getExecutableFile( &aPath.pData );

    aBinaryPath_ = aPath;

    argc_           = *pArgc;
    argv_           = ppArgv;
    pXLib_->Init( pArgc, ppArgv );
}

// -=-= SalXLib =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalXLib::SalXLib()
{
    Timeout_.tv_sec         = 0;
    Timeout_.tv_usec        = 0;
    nTimeoutMS_             = 0;

    nFDs_                   = 0;
    FD_ZERO( &aReadFDS_ );
    FD_ZERO( &aExceptionFDS_ );

    pTimeoutFDS_[0] = pTimeoutFDS_[1] = -1;
    if (pipe (pTimeoutFDS_) != -1)
    {
        // initialize 'wakeup' pipe.
        int flags;

        // set close-on-exec descriptor flag.
        if ((flags = fcntl (pTimeoutFDS_[0], F_GETFD)) != 1)
        {
            flags |= FD_CLOEXEC;
            fcntl (pTimeoutFDS_[0], F_SETFD, flags);
        }
        if ((flags = fcntl (pTimeoutFDS_[1], F_GETFD)) != 1)
        {
            flags |= FD_CLOEXEC;
            fcntl (pTimeoutFDS_[1], F_SETFD, flags);
        }

        // set non-blocking I/O flag.
        if ((flags = fcntl (pTimeoutFDS_[0], F_GETFL)) != 1)
        {
            flags |= O_NONBLOCK;
            fcntl (pTimeoutFDS_[0], F_SETFL, flags);
        }
        if ((flags = fcntl (pTimeoutFDS_[1], F_GETFL)) != 1)
        {
            flags |= O_NONBLOCK;
            fcntl (pTimeoutFDS_[1], F_SETFL, flags);
        }

        // insert [0] into read descriptor set.
        FD_SET( pTimeoutFDS_[0], &aReadFDS_ );
        nFDs_ = pTimeoutFDS_[0] + 1;
    }

    bWasXError_             = FALSE;
    bIgnoreXErrors_         = !!getenv( "SAL_IGNOREXERRORS" );
    nStateOfYield_          = 0;
}

SalXLib::~SalXLib()
{
    // close 'wakeup' pipe.
    close (pTimeoutFDS_[0]);
    close (pTimeoutFDS_[1]);

// completetly disabled Bug Nr. #47319 -> segv while using xsuntransport=shmem
// #ifdef SAL_XT
//  XtDestroyApplicationContext( pApplicationContext_ );
// #endif
}


void SalXLib::Init( int *pArgc, char *ppArgv[] )
{
    SalData *pSalData = GetSalData();
    SalI18N_InputMethod* pInputMethod = new SalI18N_InputMethod;
    pInputMethod->SetLocale();
    XrmInitialize();

    /*
     * open connection to X11 Display
     * try in this order:
     *  o  -display command line parameter,
     *  o  $DISPLAY environment variable
     *  o  defualt display
     */

    Display *pDisp = NULL;

    // is there a -display command line parameter?
    vos::OExtCommandLine aCommandLine;
    sal_uInt32 nParams = aCommandLine.getCommandArgCount();
    rtl::OUString aParam;
    rtl::OString aDisplay;
    for (USHORT i=0; i<nParams; i++)
    {
        aCommandLine.getCommandArg(i, aParam);
        if (aParam.equalsAscii("-display"))
        {
            aCommandLine.getCommandArg(i+1, aParam);
            aDisplay = rtl::OUStringToOString(
                   aParam, osl_getThreadTextEncoding());
            if ((pDisp = XOpenDisplay(aDisplay.getStr()))!=NULL)
            {
                /*
                 * if a -display switch was used, we need
                 * to set the environment accoringly since
                 * the clipboard build another connection
                 * to the xserver using $DISPLAY
                 */
                const char envpre[] = "DISPLAY=";
                char *envstr = new char[sizeof(envpre)+aDisplay.getLength()];
                sprintf(envstr, "DISPLAY=%s", aDisplay.getStr());
                putenv(envstr);
            }
            break;
        }
    }

    if (!pDisp && !aDisplay.getLength())
    {
        // Open $DISPLAY or default...
        char *pDisplay = getenv("DISPLAY");
        if (pDisplay != NULL)
            aDisplay = rtl::OString(pDisplay);
        pDisp  = XOpenDisplay(pDisplay);
    }

    if ( !pDisp )
    {
        rtl::OUString aProgramFileURL = pSalData->GetCommandLineParam(0);
        rtl::OUString aProgramSystemPath;
        osl_getSystemPathFromFileURL (aProgramFileURL.pData, &aProgramSystemPath.pData);
        rtl::OString  aProgramName = rtl::OUStringToOString(
                                            aProgramSystemPath,
                                            osl_getThreadTextEncoding() );
        fprintf( stderr, "%s X11 error: Can't open display: %s\n",
                aProgramName.getStr(), aDisplay.getStr());
        fprintf( stderr, "   Set DISPLAY environment variable, use -display option\n");
        fprintf( stderr, "   or check permissions of your X-Server\n");
        fprintf( stderr, "   (See \"man X\" resp. \"man xhost\" for details)\n");
        fflush( stderr );
        exit(0);
    }

    XVisualInfo aVI;
    Colormap    aColMap;
    int         nScreen = DefaultScreen( pDisp );

    if( SalDisplay::BestVisual( pDisp, nScreen, aVI ) ) // DefaultVisual
        aColMap = DefaultColormap( pDisp, nScreen );
    else
        aColMap = XCreateColormap( pDisp,
                                   RootWindow( pDisp, nScreen ),
                                   aVI.visual,
                                   AllocNone );

    XSetIOErrorHandler    ( (XIOErrorHandler)sal_XIOErrorHdl );
    XSetErrorHandler      ( (XErrorHandler)sal_XErrorHdl );

    SalDisplay *pSalDisplay = new SalDisplay( pDisp, aVI.visual, aColMap );

    pInputMethod->CreateMethod( pDisp );
    pInputMethod->AddConnectionWatch( pDisp, (void*)this );
    pSalDisplay->SetInputMethod( pInputMethod );

    sal_Bool bOldErrorSetting = GetIgnoreXErrors();
    SetIgnoreXErrors( True );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! WasXError() );
    SetIgnoreXErrors( bOldErrorSetting );

    pSalDisplay->SetKbdExtension( pKbdExtension );
}

extern "C" {
void EmitFontpathWarning( void )
{
    static Bool bOnce = False;
    if ( !bOnce )
    {
        bOnce = True;
        fprintf( stderr, "Please verify your fontpath settings\n"
                "\t(See \"man xset\" for details"
                " or ask your system administrator)\n" );
    }
}

} /* extern "C" */

void SalXLib::XError( Display *pDisplay, XErrorEvent *pEvent )
{
    char msg[ 120 ] = "";

    if( ! bIgnoreXErrors_ )
    {
        SalFrame* pFrame = GetSalData()->pFirstFrame_;
        while( pFrame )
        {
            if( pFrame->maFrameData.GetStyle() & SAL_FRAME_STYLE_CHILD )
            {
                bIgnoreXErrors_ = TRUE;
                break;
            }
            pFrame = pFrame->maFrameData.GetNextFrame();
        }
    }



    if( ! bIgnoreXErrors_ )
    {
        if (   (pEvent->error_code   == BadAlloc)
            && (pEvent->request_code == X_OpenFont) )
        {
            static Bool bOnce = False;
            if ( !bOnce )
            {
                fprintf(stderr, "X-Error occured in a request for X_OpenFont\n");
                EmitFontpathWarning();

                bOnce = True ;
            }
            return;
        }
        else if( pEvent->request_code == X_SetInputFocus )
            return;

#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
#if ! ( defined LINUX && defined PPC )
        XGetErrorText( pDisplay, pEvent->error_code, msg, sizeof( msg ) );
#endif
        fprintf( stderr, "X-Error: %s\n", msg );
        if( pEvent->request_code > capacityof( XRequest ) )
            fprintf( stderr, "\tMajor opcode: %d (Shm?)\n", pEvent->request_code );
        else if( XRequest[pEvent->request_code] )
            fprintf( stderr, "\tMajor opcode: %d (%s)\n",
                     pEvent->request_code, XRequest[pEvent->request_code] );
        else
            fprintf( stderr, "\tMajor opcode: %d (BadRequest?)\n", pEvent->request_code );
        fprintf( stderr, "\tResource ID:  0x%lx\n",
                 pEvent->resourceid );
        fprintf( stderr, "\tSerial No:    %ld (%ld)\n",
                 pEvent->serial, LastKnownRequestProcessed(pDisplay) );

        fflush( stdout );
        fflush( stderr );
#endif
        if( pDisplay != GetSalData()->GetDefDisp()->GetDisplay() )
            return;

        oslSignalAction eToDo = osl_raiseSignal (OSL_SIGNAL_USER_X11SUBSYSTEMERROR, NULL);
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

    bWasXError_ = TRUE;
}

struct YieldEntry
{
    YieldEntry* next;       // pointer to next entry
    int         fd;         // file descriptor for reading
    void*           data;       // data for predicate and callback
    YieldFunc       pending;    // predicate (determins pending events)
    YieldFunc       queued;     // read and queue up events
    YieldFunc       handle;     // handle pending events

    inline int  HasPendingEvent()   const { return pending( fd, data ); }
    inline int  IsEventQueued()     const { return queued( fd, data ); }
    inline void HandleNextEvent()   const { handle( fd, data ); }
};

#define MAX_NUM_DESCRIPTORS 128

static YieldEntry yieldTable[ MAX_NUM_DESCRIPTORS ];

void SalXLib::Insert( int nFD, void* data,
                      YieldFunc     pending,
                      YieldFunc     queued,
                      YieldFunc     handle )
{
    DBG_ASSERT( nFD, "can not insert stdin descriptor" );
    DBG_ASSERT( !yieldTable[nFD].fd, "SalXLib::Insert fd twice" );

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
              nFD-- );

        nFDs_ = nFD + 1;
    }
}

bool SalXLib::CheckTimeout( bool bExecuteTimers )
{
    bool bRet = false;
    if( Timeout_.tv_sec ) // timer is started
    {
        timeval aTimeOfDay;
        gettimeofday( &aTimeOfDay, 0 );
        if( aTimeOfDay >= Timeout_ )
        {
            bRet = true;
            if( bExecuteTimers )
            {
                // timed out, update timeout
                Timeout_ = aTimeOfDay;
                /*
               *  #107827# autorestart immediately, will be stopped (or set
               *  to different value in notify hdl if necessary;
               *  CheckTimeout should return false while
               *  timers are being dispatched.
               */
                Timeout_ += nTimeoutMS_;
                // notify
                GetSalData()->Timeout();
            }
        }
    }
    return bRet;
}

void SalXLib::Yield( BOOL bWait )
{
    // check for timeouts here if you want to make screenshots
    static char* p_prioritize_timer = getenv ("SAL_HIGHPRIORITY_REPAINT");
    if (p_prioritize_timer != NULL)
        CheckTimeout();

    nStateOfYield_ = 0; // is not 0 if we are recursive called

    // first, check for already queued events.
    for ( int nFD = 0; nFD < nFDs_; nFD++ )
    {
        YieldEntry* pEntry = &(yieldTable[nFD]);
        if ( pEntry->fd )
        {
            DBG_ASSERT( nFD == pEntry->fd, "wrong fd in Yield()" );
            if ( pEntry->HasPendingEvent() )
            {
                pEntry->HandleNextEvent();
                // #63862# da jetzt alle user-events ueber die interne
                // queue kommen, wird die Kontrolle analog zum select
                // gesteuerten Zweig einmal bei bWait abgegeben

                /* #i9277# do not reschedule since performance gets down the
                   the drain under heavy load
                YieldMutexReleaser aReleaser;
                if ( bWait ) osl_yieldThread();
                */

                return;
            }
        }
    }

    // next, select with or without timeout according to bWait.
    int      nFDs         = nFDs_;
    fd_set   ReadFDS      = aReadFDS_;
    fd_set   ExceptionFDS = aExceptionFDS_;
    int      nFound       = 0;

    timeval  Timeout      = noyield__;
    timeval *pTimeout     = &Timeout;

    if (bWait)
    {
        pTimeout = 0;
        if (Timeout_.tv_sec) // Timer is started.
        {
            // determine remaining timeout.
            gettimeofday (&Timeout, 0);
            Timeout = Timeout_ - Timeout;
            if (yield__ >= Timeout)
            {
                // guard against micro timeout.
                Timeout = yield__;
            }
            pTimeout = &Timeout;
        }
    }

    nStateOfYield_ = 1;
    {
        // release YieldMutex (and re-acquire at block end)
        YieldMutexReleaser aReleaser;
        if( !bWait )
            osl_yieldThread();
        nFound = select( nFDs, &ReadFDS, NULL, &ExceptionFDS, pTimeout );
    }
    if( nFound < 0 ) // error
    {
#ifdef DBG_UTIL
        fprintf( stderr, "SalXLib::Yield s=%d e=%d f=%d\n",
                 nStateOfYield_, errno, nFound );
#endif
        nStateOfYield_ = 0;
        if( EINTR == errno )
        {
            errno = 0;
        }
    }

    // usually handle timeouts here (as in 5.2)
    if (p_prioritize_timer == NULL)
        CheckTimeout();

    // handle wakeup events.
    if ((nFound > 0) && (FD_ISSET(pTimeoutFDS_[0], &ReadFDS)))
    {
        int buffer;
        while (read (pTimeoutFDS_[0], &buffer, sizeof(buffer)) > 0)
            continue;
        nFound -= 1;
    }

    // handle other events.
    if( nFound > 0 )
    {
        // now we are in the protected section !
        // recall select if we have acquired fd's, ready for reading,

        struct timeval noTimeout = { 0, 0 };
        nFound = select( nFDs_, &ReadFDS, NULL,
                         &ExceptionFDS, &noTimeout );

        // someone-else has done the job for us
        if (nFound == 0)
        {
            nStateOfYield_ = 0;
            return;
        }

        for ( int nFD = 0; nFD < nFDs_; nFD++ )
        {
            YieldEntry* pEntry = &(yieldTable[nFD]);
            if ( pEntry->fd )
            {
                if ( FD_ISSET( nFD, &ExceptionFDS ) ) {
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "SalXLib::Yield exception\n" );
#endif
                    nFound--;
                }
                if ( FD_ISSET( nFD, &ReadFDS ) )
                {
                    nStateOfYield_ = 3;
                    if ( pEntry->IsEventQueued() )
                    {
                        nStateOfYield_ = 4;
                        pEntry->HandleNextEvent();
                        // if a recursive call has done the job
                        // so abort here
                        if ( nStateOfYield_ != 4 )
                            break;
                    }
                    nFound--;
                }
            }
        }
    }
    nStateOfYield_ = 0;
}

void SalXLib::Wakeup()
{
    write (pTimeoutFDS_[1], "", 1);
}
