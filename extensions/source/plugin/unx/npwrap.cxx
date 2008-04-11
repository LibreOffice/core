/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: npwrap.cxx,v $
 * $Revision: 1.16 $
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
#include "precompiled_extensions.hxx"
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <plugin/unx/plugcon.hxx>

#include <osl/file.h>
#include <osl/module.h>

PluginConnector* pConnector = NULL;

int         nAppArguments = 0;
char**      pAppArguments = NULL;
Display*    pAppDisplay = NULL;

extern oslModule pPluginLib;
extern NPError (*pNP_Shutdown)();

void LoadAdditionalLibs(const char*);

XtAppContext app_context;
Widget topLevel = NULL, topBox = NULL;
int wakeup_fd[2] = { 0, 0 };
static bool bPluginAppQuit = false;

static long GlobalConnectionLostHdl( void* /*pInst*/, void* /*pArg*/ )
{
    medDebug( 1, "pluginapp exiting due to connection lost\n" );

    write( wakeup_fd[1], "xxxx", 4 );
    return 0;
}

extern "C"
{
    static int plugin_x_error_handler( Display*, XErrorEvent* )
    {
        return 0;
    }

    static void ThreadEventHandler( XtPointer /*client_data*/, int* /*source*/, XtInputId* id )
    {
        char buf[256];
        // clear pipe
        int len, nLast = -1;

        while( (len = read( wakeup_fd[0], buf, sizeof( buf ) ) ) > 0 )
            nLast = len-1;
        if( ! bPluginAppQuit )
        {
            if( ( nLast == -1  || buf[nLast] != 'x' ) && pConnector )
                pConnector->CallWorkHandler();
            else
            {
                // it seems you can use XtRemoveInput only
                // safely from within the callback
                // why is that ?
                medDebug( 1, "removing wakeup pipe\n" );
                XtRemoveInput( *id );
                XtAppSetExitFlag( app_context );
                bPluginAppQuit = true;

                delete pConnector;
                pConnector = NULL;
            }
        }
    }
}


IMPL_LINK( PluginConnector, NewMessageHdl, Mediator*, /*pMediator*/ )
{
    medDebug( 1, "new message handler\n" );
    write( wakeup_fd[1], "cccc", 4 );
    return 0;

}

Widget createSubWidget( char* /*pPluginText*/, Widget shell, XLIB_Window aParentWindow )
{
    Widget newWidget = XtVaCreateManagedWidget(
#if defined USE_MOTIF
          "drawingArea",
        xmDrawingAreaWidgetClass,
#else
        "",
#  if defined DISABLE_XAW
        compositeWidgetClass,
#  else
        labelWidgetClass,
#  endif
#endif
          shell,
        XtNwidth, 200,
        XtNheight, 200,
          (char *)NULL );
    XtRealizeWidget( shell );

    medDebug( 1, "Reparenting new widget %x to %x\n", XtWindow( newWidget ), aParentWindow );
    XReparentWindow( pAppDisplay,
                     XtWindow( shell ),
                     aParentWindow,
                     0, 0 );
    XtMapWidget( shell );
    XtMapWidget( newWidget );
    XRaiseWindow( pAppDisplay, XtWindow( shell ) );
    XSync( pAppDisplay, False );

    return newWidget;
}

void* CreateNewShell( void** pShellReturn, XLIB_Window aParentWindow )
{
    XLIB_String n, c;
    XtGetApplicationNameAndClass(pAppDisplay, &n, &c);

    Widget newShell =
        XtVaAppCreateShell( "pane", c,
                            topLevelShellWidgetClass,
                            pAppDisplay,
                            XtNwidth, 200,
                            XtNheight, 200,
                            XtNoverrideRedirect, True,
                            (char *)NULL );
    *pShellReturn = newShell;

    char pText[1024];
    sprintf( pText, "starting plugin %s ...", pAppArguments[2] );

    Widget newWidget = createSubWidget( pText, newShell, aParentWindow );

    return newWidget;
}

static oslModule LoadModule( const char* pPath )
{
    ::rtl::OUString sSystemPath( ::rtl::OUString::createFromAscii( pPath ) );
    ::rtl::OUString sFileURL;
    osl_getFileURLFromSystemPath( sSystemPath.pData, &sFileURL.pData );

    oslModule pLib = osl_loadModule( sFileURL.pData, SAL_LOADMODULE_LAZY );
    if( ! pLib )
    {
        medDebug( 1, "could not open %s\n", pPath );
    }
    return pLib;
}

// Unix specific implementation
static void CheckPlugin( const char* pPath )
{
    oslModule pLib = LoadModule( pPath );

    char*(*pNP_GetMIMEDescription)() = (char*(*)())
        osl_getAsciiFunctionSymbol( pLib, "NP_GetMIMEDescription" );
    if( pNP_GetMIMEDescription )
        printf( "%s\n", pNP_GetMIMEDescription() );
    else
        medDebug( 1, "could not symbol NP_GetMIMEDescription\n" );

    osl_unloadModule( pLib );
}

#if OSL_DEBUG_LEVEL > 1 && defined LINUX
#include <execinfo.h>
#endif

extern "C" {

static void signal_handler( int nSig )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "caught signal %d, exiting\n", nSig );
#ifdef LINUX
    void* pStack[64];
    int nStackLevels = backtrace( pStack, sizeof(pStack)/sizeof(pStack[0]) );
    backtrace_symbols_fd( pStack, nStackLevels, STDERR_FILENO );
#endif
#endif
    if( pConnector )
    {
        // ensure that a read on the other side will wakeup
        delete pConnector;
        pConnector = NULL;
    }

    _exit(nSig);
}

}

int main( int argc, char **argv)
{
    struct sigaction aSigAction;
    aSigAction.sa_handler = signal_handler;
    sigemptyset( &aSigAction.sa_mask );
    aSigAction.sa_flags = SA_NOCLDSTOP;
    sigaction( SIGSEGV, &aSigAction, NULL );
    sigaction( SIGBUS, &aSigAction, NULL );
    sigaction( SIGABRT, &aSigAction, NULL );
    sigaction( SIGTERM, &aSigAction, NULL );
    sigaction( SIGILL, &aSigAction, NULL );

    int nArg = (argc < 3) ? 1 : 2;
    char* pBaseName = argv[nArg] + strlen(argv[nArg]);
    while( pBaseName > argv[nArg] && pBaseName[-1] != '/' )
        pBaseName--;
    LoadAdditionalLibs( pBaseName );

    if( argc < 3 )
    {
        CheckPlugin(argv[1]);
        exit(0);
    }
    nAppArguments = argc;
    pAppArguments = argv;

    XSetErrorHandler( plugin_x_error_handler );

    if( pipe( wakeup_fd ) )
    {
        medDebug( 1, "could not pipe()\n" );
        return 1;
    }
    // initialize 'wakeup' pipe.
    int flags;

    // set close-on-exec descriptor flag.
    if ((flags = fcntl (wakeup_fd[0], F_GETFD)) != -1)
    {
        flags |= FD_CLOEXEC;
        fcntl (wakeup_fd[0], F_SETFD, flags);
    }
    if ((flags = fcntl (wakeup_fd[1], F_GETFD)) != -1)
    {
        flags |= FD_CLOEXEC;
        fcntl (wakeup_fd[1], F_SETFD, flags);
    }

    // set non-blocking I/O flag.
    if ((flags = fcntl (wakeup_fd[0], F_GETFL)) != -1)
    {
        flags |= O_NONBLOCK;
        fcntl (wakeup_fd[0], F_SETFL, flags);
    }
    if ((flags = fcntl (wakeup_fd[1], F_GETFL)) != -1)
    {
        flags |= O_NONBLOCK;
        fcntl (wakeup_fd[1], F_SETFL, flags);
    }

    pPluginLib = LoadModule( argv[2] );
    if( ! pPluginLib )
    {
        exit(255);
    }
    int nSocket = atol( argv[1] );

     pConnector = new PluginConnector( nSocket );
     pConnector->SetConnectionLostHdl( Link( NULL, GlobalConnectionLostHdl ) );

    XtSetLanguageProc( NULL, NULL, NULL );

    topLevel = XtVaOpenApplication(
        &app_context,       /* Application context */
        "SOPlugin",         /* Application class */
        NULL, 0,            /* command line option list */
        &argc, argv,        /* command line args */
        NULL,               /* for missing app-defaults file */
        topLevelShellWidgetClass,
        XtNoverrideRedirect, True,
        (char *)NULL);      /* terminate varargs list */
    pAppDisplay = XtDisplay( topLevel );

    XtAppAddInput( app_context,
                   wakeup_fd[0],
                   (XtPointer)XtInputReadMask,
                   ThreadEventHandler, NULL );

    /*
     *  Create windows for widgets and map them.
     */
    int nWindow;
    sscanf( argv[3], "%d", &nWindow );
    char pText[1024];
    sprintf( pText, "starting plugin %s ...", pAppArguments[2] );
    topBox = createSubWidget( pText, topLevel, (XLIB_Window)nWindow );


     // send that we are ready to go
    MediatorMessage* pMessage =
        pConnector->Transact( "init req", 8,
                              NULL );
    delete pMessage;

#if OSL_DEBUG_LEVEL > 3
    int nPID = getpid();
    int nChild = fork();
    if( nChild == 0 )
    {
        char pidbuf[16];
        char* pArgs[] = { "xterm", "-sl", "2000", "-sb", "-e", "gdb", "pluginapp.bin", pidbuf, NULL };
        sprintf( pidbuf, "%d", nPID );
        execvp( pArgs[0], pArgs );
        _exit(255);
    }
    else
        sleep( 10 );
#endif

    /*
     *  Loop for events.
     */
    // for some reason XtAppSetExitFlag won't quit the application
    // in ThreadEventHandler most of times; Xt will hang in select
    // (hat is in XtAppNextEvent). Have our own mainloop instead
    // of XtAppMainLoop
    do
    {
        XtAppProcessEvent( app_context, XtIMAll );
    } while( ! XtAppGetExitFlag( app_context ) && ! bPluginAppQuit );

    medDebug( 1, "left plugin app main loop\n" );

    pNP_Shutdown();
    medDebug( 1, "NP_Shutdown done\n" );
    osl_unloadModule( pPluginLib );
    medDebug( 1, "plugin close\n" );

    close( wakeup_fd[0] );
    close( wakeup_fd[1] );

    return 0;
}

#ifdef GCC
extern "C" {
    void __pure_virtual()
    {}

    void* __builtin_new( int nBytes )
    { return malloc(nBytes); }
    void* __builtin_vec_new( int nBytes )
    { return malloc(nBytes); }
    void __builtin_delete( char* pMem )
    { free(pMem); }
    void __builtin_vec_delete( char* pMem )
    { free(pMem); }
}
#endif

