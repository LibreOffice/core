/*************************************************************************
 *
 *  $RCSfile: saldata.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pl $ $Date: 2001-03-02 14:23:27 $
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
#include <stdio.h>
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

#include <prex.h>
#include <X11/Shell.h>
#include <X11/Xproto.h>
#include <postx.h>

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#include <salunx.h>

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
static const struct timeval noyield = { 0, 0 };
static const struct timeval yield   = { 0, 10000 };
static const struct fd_set  ZeroFDS = { 0 };
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
    SalData    *pSalData    = GetSalData();
    SalDisplay *pSalDisplay = pSalData->GetDisplay( pDisplay );
    if ( pDisplay && pSalDisplay->IsDisplay() )
        pSalData->GetLib()->Remove( ConnectionNumber( pDisplay ) );

    oslSignalAction eToDo = osl_raiseSignal (OSL_SIGNAL_USER_X11SUBSYSTEMERROR, NULL);
    // einen XIOError kann man nicht ignorieren. Die Connection ist
    // zusammengebrochen, hier kann man nur noch halbwegs sinnvoll runterfahren

    fprintf( stderr, "X IO Error\n" );
    fflush( stdout );
    fflush( stderr );
    exit(0);
    return 0;
}

static void sal_XtErrorHdl( XLIB_String sMsg )
{
#ifdef DBG_UTIL
    fprintf( stderr, "X Toolkit Error: %s\n", sMsg );
#endif
    fflush( stdout );
    fflush( stderr );
    abort();
}

static void sal_XtWarningHdl( XLIB_String sMsg )
{
#ifdef DBG_UTIL
    fprintf( stderr, "X Toolkit Warning: %s\n", sMsg );
    fflush( stdout );
    fflush( stderr );
#endif
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
    delete pXLib_;
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
    return String( argv_[nParam], gsl_getSystemTextEncoding() );
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

    pXLib_->Init( pArgc, ppArgv );

    argc_           = *pArgc;
    argv_           = ppArgv;
}


// -=-= SalXLib =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalXLib::SalXLib()
{
    pApplicationContext_    = NULL;
    Timeout_.tv_sec         = 0;
    Timeout_.tv_usec        = 0;
    nTimeoutMS_             = 0;
    bWasXError_             = FALSE;
    bIgnoreXErrors_         = !!getenv( "SAL_IGNOREXERRORS" );
    nStateOfYield_          = 0;
    nFDs_                   = 0;
    pReadFDS_               = new fd_set;
    pExceptionFDS_          = new fd_set;
    FD_ZERO( pReadFDS_ );
    FD_ZERO( pExceptionFDS_ );
}

SalXLib::~SalXLib()
{
    delete pReadFDS_;
    delete pExceptionFDS_;

// completetly disabled Bug Nr. #47319 -> segv while using xsuntransport=shmem
// #ifdef SAL_XT
//  XtDestroyApplicationContext( pApplicationContext_ );
// #endif
}

static char sDISPLAY___[30];
void SalXLib::Init( int *pArgc, char *ppArgv[] )
{
    SalData *pSalData = GetSalData();
    SalI18N_InputMethod* pInputMethod = new SalI18N_InputMethod;

    pInputMethod->SetLocale();

    XtSetLanguageProc( NULL, NULL, NULL );
    XtToolkitInitialize();
    XrmInitialize();
    pApplicationContext_ = XtCreateApplicationContext();

    Display *pDisp = XtOpenDisplay( pApplicationContext_,
                                    NULL,
                                    NULL,
                                    "VCL",
                                    NULL,
                                    0,
                                    pArgc,
                                    ppArgv );

    if( !pDisp )
    {
        char *pDisplayString = getenv ("DISPLAY");

        if( pDisplayString )
        {
            fprintf( stderr, "%s:\n   cannot open display \"%s\"\n",
                     ppArgv[0],
                     pDisplayString );
            fprintf( stderr, "   Please check your \"DISPLAY\" environment variable\n   as well as the permissions to access that display.\n");
        }
        else
        {
            fprintf( stderr,
                     "%s:\n   cannot open display; DISPLAY environment variable is not set\n"
                     "   please set it to the correct value and check\n"
                     "   the permission to access that display.\n",
                     ppArgv[0]
                     );
        }
        fprintf( stderr, "   (See \"man X\" resp. \"man xhost\" for details)\n");
        fflush ( stderr );
        exit (0);
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

    Arg aArgs[10];
    int nArgs = 0;
     XtSetArg( aArgs[nArgs], XtNvisual,     aVI.visual  );  nArgs++;
     XtSetArg( aArgs[nArgs], XtNdepth,      aVI.depth   );  nArgs++;
     XtSetArg( aArgs[nArgs], XtNcolormap,   aColMap     );  nArgs++;

    Widget wInitWidget = XtAppCreateShell( NULL,
                                           "SAL",
                                           applicationShellWidgetClass,
                                           pDisp,
                                           aArgs, nArgs );


    XSetIOErrorHandler    ( (XIOErrorHandler)sal_XIOErrorHdl );
    XSetErrorHandler      ( (XErrorHandler)sal_XErrorHdl );

    XtAppSetErrorHandler  ( GetAppContext(),
                            (XtErrorHandler)sal_XtErrorHdl );
    XtAppSetWarningHandler( GetAppContext(),
                            (XtErrorHandler)sal_XtWarningHdl );

    SalDisplay *pSalDisplay = new SalDisplay( wInitWidget );

    pInputMethod->CreateMethod( pDisp );
    pSalDisplay->SetInputMethod( pInputMethod );

    sal_Bool bOldErrorSetting = GetIgnoreXErrors();
    SetIgnoreXErrors( True );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! WasXError() );
    SetIgnoreXErrors( bOldErrorSetting );

    pSalDisplay->SetKbdExtension( pKbdExtension );

#if 0 // ! USE_XTOOLKIT

    SalDisplay *pSalDisplay = new SalDisplay( pDisp, aVI.visual, aColMap );

#endif
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
    char msg[ 120 ];

    if( ! bIgnoreXErrors_ )
    {
#if defined DEBUG || defined DBG_UTIL
        XGetErrorText( pDisplay, pEvent->error_code, msg, sizeof( msg ) );
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
        }
        else
        {
            abort();
        }
    }

    bWasXError_ = TRUE;
}

#define MAX_NUM_DESCRIPTORS 128

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
    DBG_ASSERT( nFD, "can not insert stdin descriptor" )
        DBG_ASSERT( !yieldTable[nFD].fd, "SalXLib::Insert fd twice" )

        yieldTable[nFD].fd      = nFD;
    yieldTable[nFD].data        = data;
    yieldTable[nFD].pending = pending;
    yieldTable[nFD].queued  = queued;
    yieldTable[nFD].handle  = handle;

    FD_SET( nFD, pReadFDS_ );
    FD_SET( nFD, pExceptionFDS_ );

    if( nFD >= nFDs_ )
        nFDs_ = nFD + 1;
}

void SalXLib::Remove( int nFD )
{
    FD_CLR( nFD, pReadFDS_ );
    FD_CLR( nFD, pExceptionFDS_ );

    yieldTable[nFD].fd = 0;

    if ( nFD == nFDs_ )
    {
        for ( nFD = nFDs_ - 1;
              nFD >= 0 && !yieldTable[nFD].fd;
              nFD-- );

        nFDs_ = nFD + 1;
    }
}
#if 0
class YieldMutexReleaser
{
    ULONG               m_nYieldCount;
    SalYieldMutex*      m_pSalInstYieldMutex;
public:
    YieldMutexReleaser();
    ~YieldMutexReleaser();
};

YieldMutexReleaser::YieldMutexReleaser()
{
    SalData *pSalData       = GetSalData();
    m_pSalInstYieldMutex    =
        pSalData->pFirstInstance_->maInstData.mpSalYieldMutex;

    ULONG i;
    if ( m_pSalInstYieldMutex->GetThreadId() ==
         NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        m_nYieldCount = m_pSalInstYieldMutex->GetAcquireCount();
        for ( i = 0; i < m_nYieldCount; i++ )
            m_pSalInstYieldMutex->release();
    }
    else
        m_nYieldCount = 0;
}

YieldMutexReleaser::~YieldMutexReleaser()
{
    // Yield-Semaphore wieder holen
    while ( m_nYieldCount )
    {
        m_pSalInstYieldMutex->acquire();
        m_nYieldCount--;
    }
}
#endif

void SalXLib::Yield( BOOL bWait )
{
    fd_set   ReadFDS;
    fd_set   ExceptionFDS;
    int      nFound = 0;

    nStateOfYield_ = 0; // is not 0 if we are recursive called

    // first look for queued events
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
                YieldMutexReleaser aReleaser;
                if ( bWait )
                    osl_yieldThread();
                return;
            }
        }
    }

    // next select with or without timeout according to bWait

    ReadFDS         = *pReadFDS_;
    ExceptionFDS    = *pExceptionFDS_;

    struct timeval Timeout;
    Timeout = bWait ? yield : noyield;

    nStateOfYield_ = 1;

    {
        // Yield-Semaphore freigeben
        YieldMutexReleaser aReleaser;
        if( bWait )
            osl_yieldThread();
#if defined (HPUX) && defined (CMA_UX)
        nFound = select( nFDs_, (int*)&ReadFDS, (int*)NULL,
                         (int*)&ExceptionFDS, &Timeout );
#else
        nFound = select( nFDs_, &ReadFDS, NULL, &ExceptionFDS, &Timeout );
#endif
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

    // check for timeouts
    if( Timeout_.tv_sec ) // timer is started
    {
        gettimeofday( &Timeout, NULL );

        if( Timeout >= Timeout_ )
        {
            Timeout_ = Timeout + nTimeoutMS_;
            GetSalData()->Timeout();
        }
    }

    // handle events
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
            //if ( !pthread_equal (pthread_self(),
            //                   pSalData->GetMainThread()))
            return;
        }

        for ( int nFD = 0; nFD < nFDs_; nFD++ )
        {
            YieldEntry* pEntry = &(yieldTable[nFD]);
            if ( pEntry->fd )
            {
                if ( FD_ISSET( nFD, &ExceptionFDS ) ) {
#if defined DEBUG
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

