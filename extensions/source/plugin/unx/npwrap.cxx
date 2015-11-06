/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <signal.h>

#include <plugin/unx/plugcon.hxx>

#include <osl/file.h>
#include <osl/module.h>
#include <sal/log.hxx>

#include <config_vclplug.h>

#include <npwrap.hxx>

PluginConnector* pConnector = NULL;

int         nAppArguments = 0;
char**      pAppArguments = NULL;
Display*    pAppDisplay = NULL;
Display*    pXtAppDisplay = NULL;

extern oslModule pPluginLib;
extern NPError (*pNP_Shutdown)();

XtAppContext app_context;
Widget topLevel = NULL, topBox = NULL;
int wakeup_fd[2] = { 0, 0 };
static bool bPluginAppQuit = false;

static void GlobalConnectionLostHdl( void* /*pInst*/, Mediator* /*pArg*/ )
{
    SAL_WARN("extensions.plugin", "pluginapp exiting due to connection lost");

    bool bSuccess = (4 == write(wakeup_fd[1], "xxxx", 4 ));
    SAL_WARN_IF(!bSuccess, "extensions.plugin", "short write");
}

extern "C"
{
    static int plugin_x_error_handler( Display*, XErrorEvent* )
    {
        return 0;
    }

#if ! ENABLE_GTK
    static void ThreadEventHandler( XtPointer /*client_data*/, int* /*source*/, XtInputId* id )
    {
        char buf[256];
        // clear pipe
        int len, nLast = -1;

        while( (len = read( wakeup_fd[0], buf, sizeof( buf ) ) ) > 0 )
            nLast = len-1;
        if( ! bPluginAppQuit ) {
            if( ( nLast == -1  || buf[nLast] != 'x' ) && pConnector )
                pConnector->CallWorkHandler();
            else {
                // it seems you can use XtRemoveInput only
                // safely from within the callback
                // why is that ?
                SAL_INFO("extensions.plugin", "removing wakeup pipe");
                XtRemoveInput( *id );
                XtAppSetExitFlag( app_context );
                bPluginAppQuit = true;

                delete pConnector;
                pConnector = NULL;
            }
        }
    }
#endif
}


IMPL_LINK_NOARG_TYPED( PluginConnector, NewMessageHdl, Mediator*, void )
{
    (void) this; // loplugin:staticmethods
    SAL_INFO("extensions.plugin", "new message handler");
    bool bSuccess = (4 == write(wakeup_fd[1], "cccc", 4));
    SAL_WARN_IF(!bSuccess, "extensions.plugin", "short write");
}

Widget createSubWidget( char* /*pPluginText*/, Widget shell, Window aParentWindow )
{
    Widget newWidget = XtVaCreateManagedWidget(
#if defined USE_MOTIF
                           "drawingArea",
                           xmDrawingAreaWidgetClass,
#else
                           "",
                           compositeWidgetClass,
#endif
                           shell,
                           XtNwidth, 200,
                           XtNheight, 200,
                           nullptr );
    XtRealizeWidget( shell );
    XtRealizeWidget( newWidget );

    SAL_INFO(
        "extensions.plugin",
        "reparenting new widget " << XtWindow( newWidget ) << " to "
        << aParentWindow);
    XReparentWindow( pXtAppDisplay,
                     XtWindow( shell ),
                     aParentWindow,
                     0, 0 );
    XtMapWidget( shell );
    XtMapWidget( newWidget );
    XRaiseWindow( pXtAppDisplay, XtWindow( shell ) );
    XSync( pXtAppDisplay, False );

    return newWidget;
}

void* CreateNewShell( void** pShellReturn, Window aParentWindow )
{
    String n, c;
    XtGetApplicationNameAndClass(pXtAppDisplay, &n, &c);

    Widget newShell =
        XtVaAppCreateShell( "pane", c,
                            topLevelShellWidgetClass,
                            pXtAppDisplay,
                            XtNwidth, 200,
                            XtNheight, 200,
                            XtNoverrideRedirect, True,
                            nullptr );
    *pShellReturn = newShell;

    char pText[1024];
    sprintf( pText, "starting plugin %s ...", pAppArguments[2] );

    Widget newWidget = createSubWidget( pText, newShell, aParentWindow );

    return newWidget;
}

static oslModule LoadModule( const char* pPath )
{
    OUString sSystemPath( OUString::createFromAscii( pPath ) );
    OUString sFileURL;
    osl_getFileURLFromSystemPath( sSystemPath.pData, &sFileURL.pData );

    oslModule pLib = osl_loadModule( sFileURL.pData, SAL_LOADMODULE_LAZY );
    SAL_INFO_IF(!pLib, "extensions.plugin", "could not open " << pPath);
    return pLib;
}

// Unix specific implementation
static void CheckPlugin( const char* pPath )
{
    oslModule pLib = LoadModule( pPath );
    if (pLib != 0) {
        char*(*pNP_GetMIMEDescription)() = reinterpret_cast<char*(*)()>(
                                               osl_getAsciiFunctionSymbol( pLib, "NP_GetMIMEDescription" ));
        if( pNP_GetMIMEDescription )
            printf( "%s\n", pNP_GetMIMEDescription() );
        else
            SAL_WARN(
                "extensions.plugin",
                "could not get symbol NP_GetMIMEDescription " << dlerror());
        osl_unloadModule( pLib );
    }
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
        int nStackLevels = backtrace( pStack, SAL_N_ELEMENTS(pStack) );
        backtrace_symbols_fd( pStack, nStackLevels, STDERR_FILENO );
#endif
#endif
        if( pConnector ) {
            // ensure that a read on the other side will wakeup
            delete pConnector;
            pConnector = NULL;
        }

        _exit(nSig);
    }

#if ENABLE_GTK

    static gboolean noClosure( gpointer )
    {
        return sal_True;
    }

// Xt events
    static gboolean prepareXtEvent( GSource*, gint* )
    {
        int nMask = XtAppPending( app_context );
        return (nMask & XtIMAll) != 0;
    }

    static gboolean checkXtEvent( GSource* )
    {
        int nMask = XtAppPending( app_context );
        return (nMask & XtIMAll) != 0;
    }

    static gboolean dispatchXtEvent( GSource*, GSourceFunc, gpointer )
    {
        XtAppProcessEvent( app_context, XtIMAll );
        return sal_True;
    }

    static GSourceFuncs aXtEventFuncs = {
        prepareXtEvent,
        checkXtEvent,
        dispatchXtEvent,
        NULL,
        noClosure,
        NULL
    };

    static gboolean pollXtTimerCallback(gpointer)
    {
        for(int i = 0; i < 5; i++) {
            if( (XtAppPending(app_context) & (XtIMAll & ~XtIMXEvent)) == 0 )
                break;
            XtAppProcessEvent(app_context, XtIMAll & ~XtIMXEvent);
        }
        return sal_True;
    }

    static gboolean prepareWakeupEvent( GSource*, gint* )
    {
        struct pollfd aPoll = { wakeup_fd[0], POLLIN, 0 };
        (void)poll(&aPoll, 1, 0);
        return (aPoll.revents & POLLIN ) != 0;
    }

    static gboolean checkWakeupEvent( GSource* pSource )
    {
        gint nDum = 0;
        return prepareWakeupEvent( pSource, &nDum );
    }

    static gboolean dispatchWakeupEvent( GSource*, GSourceFunc, gpointer )
    {
        char buf[256];
        // clear pipe
        int len, nLast = -1;

        while( (len = read( wakeup_fd[0], buf, sizeof( buf ) ) ) > 0 )
            nLast = len-1;
        if( ( nLast == -1  || buf[nLast] != 'x' ) && pConnector )
            pConnector->CallWorkHandler();
        else {
            XtAppSetExitFlag( app_context );
            bPluginAppQuit = true;

            delete pConnector;
            pConnector = NULL;
        }

        return sal_True;
    }

    static GSourceFuncs aWakeupEventFuncs = {
        prepareWakeupEvent,
        checkWakeupEvent,
        dispatchWakeupEvent,
        NULL,
        noClosure,
        NULL
    };

#endif // GTK

}

int main( int argc, char **argv)
{
    try
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

        if( argc == 2 ) {
            CheckPlugin(argv[1]);
            exit(0);
        }
        nAppArguments = argc;
        pAppArguments = argv;

        XSetErrorHandler( plugin_x_error_handler );

        if( pipe( wakeup_fd ) ) {
            SAL_WARN("extensions.plugin", "could not pipe()");
            return 1;
        }
        // initialize 'wakeup' pipe.
        int flags;

        // set close-on-exec descriptor flag.
        if ((flags = fcntl (wakeup_fd[0], F_GETFD)) != -1) {
            flags |= FD_CLOEXEC;
            (void)fcntl(wakeup_fd[0], F_SETFD, flags);
        }
        if ((flags = fcntl (wakeup_fd[1], F_GETFD)) != -1) {
            flags |= FD_CLOEXEC;
            (void)fcntl(wakeup_fd[1], F_SETFD, flags);
        }

        // set non-blocking I/O flag.
        if ((flags = fcntl (wakeup_fd[0], F_GETFL)) != -1) {
            flags |= O_NONBLOCK;
            (void)fcntl(wakeup_fd[0], F_SETFL, flags);
        }
        if ((flags = fcntl (wakeup_fd[1], F_GETFL)) != -1) {
            flags |= O_NONBLOCK;
            (void)fcntl(wakeup_fd[1], F_SETFL, flags);
        }

        pPluginLib = LoadModule( argv[2] );
        if( ! pPluginLib ) {
            exit(255);
        }
        int nSocket = atol( argv[1] );

    #if ENABLE_GTK
        g_thread_init(NULL);
        gtk_init(&argc, &argv);
    #endif

        pConnector = new PluginConnector( nSocket );
        pConnector->SetConnectionLostHdl( Link<Mediator*,void>( NULL, GlobalConnectionLostHdl ) );

        XtSetLanguageProc( NULL, NULL, NULL );

        XtToolkitInitialize();
        app_context = XtCreateApplicationContext();
        pXtAppDisplay = XtOpenDisplay( app_context, NULL, "SOPlugin", "SOPlugin", NULL, 0, &argc, argv );


    #if ENABLE_GTK
        // integrate Xt events into GTK event loop
        GPollFD aXtPollDesc, aWakeupPollDesc;

        GSource* pXTSource = g_source_new( &aXtEventFuncs, sizeof(GSource) );
        if( !pXTSource ) {
            SAL_WARN("extensions.plugin", "could not get Xt GSource");
            return 1;
        }

        g_source_set_priority( pXTSource, GDK_PRIORITY_EVENTS );
        g_source_set_can_recurse( pXTSource, sal_True );
        g_source_attach( pXTSource, NULL );
        aXtPollDesc.fd = ConnectionNumber( pXtAppDisplay );
        aXtPollDesc.events = G_IO_IN;
        aXtPollDesc.revents = 0;
        g_source_add_poll( pXTSource, &aXtPollDesc );

        gint xt_polling_timer_id = g_timeout_add( 25, pollXtTimerCallback, NULL);
        // Initialize wakeup events listener
        GSource *pWakeupSource = g_source_new( &aWakeupEventFuncs, sizeof(GSource) );
        if ( pWakeupSource == NULL ) {
            SAL_WARN("extensions.plugin", "could not get wakeup source");
            return 1;
        }
        g_source_set_priority( pWakeupSource, GDK_PRIORITY_EVENTS);
        g_source_attach( pWakeupSource, NULL );
        aWakeupPollDesc.fd = wakeup_fd[0];
        aWakeupPollDesc.events = G_IO_IN;
        aWakeupPollDesc.revents = 0;
        g_source_add_poll( pWakeupSource, &aWakeupPollDesc );

        pAppDisplay = gdk_x11_display_get_xdisplay( gdk_display_get_default() );
    #else
        pAppDisplay = pXtAppDisplay;
        XtAppAddInput( app_context,
                       wakeup_fd[0],
                       (XtPointer)XtInputReadMask,
                       ThreadEventHandler, NULL );
    #endif

        // send that we are ready to go
        MediatorMessage* pMessage =
            pConnector->Transact( "init req", 8,
                                  NULL );
        delete pMessage;

    #if OSL_DEBUG_LEVEL > 3
        int nPID = getpid();
        int nChild = fork();
        if( nChild == 0 ) {
            char pidbuf[16];
            char* pArgs[] = { "xterm", "-sl", "2000", "-sb", "-e", "gdb", "pluginapp.bin", pidbuf, NULL };
            sprintf( pidbuf, "%d", nPID );
            execvp( pArgs[0], pArgs );
            _exit(255);
        } else
            sleep( 10 );
    #endif

        /*
         *  Loop for events.
         */
        // for some reason XtAppSetExitFlag won't quit the application
        // in ThreadEventHandler most of times; Xt will hang in select
        // (hat is in XtAppNextEvent). Have our own mainloop instead
        // of XtAppMainLoop
        do {
    #if ENABLE_GTK
            g_main_context_iteration( NULL, sal_True );
    #else
            XtAppProcessEvent( app_context, XtIMAll );
    #endif
        } while( ! XtAppGetExitFlag( app_context ) && ! bPluginAppQuit );

        SAL_INFO("extensions.plugin", "left plugin app main loop");

    #if ENABLE_GTK
        g_source_remove(xt_polling_timer_id);
    #endif

        pNP_Shutdown();
        SAL_INFO("extensions.plugin", "NP_Shutdown done");
        osl_unloadModule( pPluginLib );
        SAL_INFO("extensions.plugin", "plugin close");

        close( wakeup_fd[0] );
        close( wakeup_fd[1] );

        return 0;
    }
    catch (std::exception& e)
    {
        SAL_WARN("extensions.plugin", "exception: " << e.what());
        return 255;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
