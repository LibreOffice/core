/*************************************************************************
 *
 *  $RCSfile: npwrap.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:18:02 $
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
#include <errno.h>
#include <dlfcn.h>

#include <plugin/unx/plugcon.hxx>


PluginConnector* pConnector = NULL;

NPWindow aNPWindow;
NPSetWindowCallbackStruct aNPSetWindowCallbackStruct;

int         nAppArguments = 0;
char**      pAppArguments = NULL;
Display*    pAppDisplay = NULL;

extern void* pPluginLib;

static long GlobalConnectionLostHdl( void* pInst, void* pArg )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "xhello exiting due to connection lost\n" );
#endif
    exit( 0 );
    return 0;
}

XtAppContext app_context;
Widget topLevel, hello;

void ThreadEventHandler( Widget widget, XtPointer closure,
                         XEvent* pEvent, XLIB_Boolean* dispatch_further )
{
    if( pEvent->type == ClientMessage &&
        pEvent->xclient.format == 32 &&
        pEvent->xclient.data.l[0] == 1 &&
        pEvent->xclient.data.l[1] == 2 &&
        pEvent->xclient.data.l[2] == 3 &&
        pEvent->xclient.data.l[3] == 4 &&
        pEvent->xclient.data.l[4] == 5 )
    {
        *dispatch_further = False;
        pConnector->CallWorkHandler();
    }
    else
        *dispatch_further = True;
}

IMPL_LINK( PluginConnector, NewMessageHdl, Mediator*, pMediator )
{
    XEvent aEvent;
    aEvent.type = ClientMessage;
    aEvent.xclient.display = XtDisplay( topLevel );
    aEvent.xclient.message_type = XA_STRING;
    aEvent.xclient.window = XtWindow( topLevel );
    aEvent.xclient.format = 32;
    aEvent.xclient.data.l[0] = 1;
    aEvent.xclient.data.l[1] = 2;
    aEvent.xclient.data.l[2] = 3;
    aEvent.xclient.data.l[3] = 4;
    aEvent.xclient.data.l[4] = 5;
    XSendEvent( XtDisplay( topLevel ),
                XtWindow( topLevel ), False,
                0, &aEvent );
    XFlush( XtDisplay( topLevel ) );
    return 0;
}

#if defined USE_MOTIF
Widget createSubWidget( char* pPluginText, Widget shell )
{
    Widget newWidget;
      newWidget = XtVaCreateManagedWidget(
          "hello",
        xmDrawingAreaWidgetClass,
          shell,
          NULL );

    return newWidget;
}
#else
Widget createSubWidget( char* pPluginText, Widget shell )
{
    hello = XtVaCreateManagedWidget(
        pPluginText,
        labelWidgetClass,
        shell,
        NULL );

    return hello;
}
#endif

void* CreateNewShell( void** pShellReturn )
{
    Widget newShell =
        XtVaAppCreateShell( "SOPluginApp", "SOPluginApp",
                            applicationShellWidgetClass,
                            pAppDisplay,
                            XtNwidth, 10,
                            XtNheight, 10,
                            XtNoverrideRedirect, True,
                            NULL );
    char pText[1024];
    sprintf( pText, "starting plugin %s ...", pAppArguments[2] );
    Widget newWidget = createSubWidget( pText, newShell );

    XtRealizeWidget( newShell );

    *pShellReturn = newShell;
    return newWidget;
}

// Unix specific implementation
static void CheckPlugin( const char* pPath )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    void *pLib = dlopen( pPath, RTLD_LAZY );
    if( ! pLib )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "could not dlopen( %s ) (%s)\n", pPath, dlerror() );
#endif
        return;
    }

    char*(*pNP_GetMIMEDescription)() = (char*(*)())
        dlsym( pLib, "NP_GetMIMEDescription" );
    if( pNP_GetMIMEDescription )
        printf( "%s\n", pNP_GetMIMEDescription() );
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "could not dlsym NP_GetMIMEDescription (%s)\n", dlerror() );
#endif
    dlclose( pLib );
}

main( int argc, char **argv)
{
    if( argc < 3 )
    {
        CheckPlugin(argv[1]);
        exit(0);
    }
    nAppArguments = argc;
    pAppArguments = argv;

    XInitThreads();

    pPluginLib = dlopen( argv[2], RTLD_LAZY );
    if( ! pPluginLib )
    {
        medDebug( 1, "dlopen on %s failed because of:\n\t%s\n",
                  argv[2], dlerror() );
        exit(255);
    }
    int nSocket = atol( argv[1] );

    pConnector = new PluginConnector( nSocket );
    pConnector->SetConnectionLostHdl( Link( NULL, GlobalConnectionLostHdl ) );

    XtSetLanguageProc( NULL, NULL, NULL );

    topLevel = XtVaAppInitialize(
        &app_context,       /* Application context */
        "SOPlugin",         /* Application class */
        NULL, 0,            /* command line option list */
        &argc, argv,        /* command line args */
        NULL,               /* for missing app-defaults file */
        XtNoverrideRedirect, True,
        NULL);              /* terminate varargs list */
    pAppDisplay = XtDisplay( topLevel );
    XtAddRawEventHandler( topLevel, 0, True, ThreadEventHandler, NULL );

    char pText[1024];
    sprintf( pText, "starting plugin %s ...", pAppArguments[2] );
    hello = createSubWidget( pText, topLevel );

    /*
     *  Create windows for widgets and map them.
     */
    XtRealizeWidget(topLevel);

    INT32 nWindow;
    sscanf( argv[3], "%d", &nWindow );
    medDebug( 1, "Reparenting topLevel to %x\n", nWindow );
      XReparentWindow( pAppDisplay,
                       XtWindow( topLevel ),
                       (XLIB_Window)nWindow,
                       0, 0 );

    XSync( XtDisplay( hello ), False );

    // send that we are ready to go
       MediatorMessage* pMessage =
           pConnector->Transact( "init req", 8,
                                 NULL );
       delete pMessage;

    aNPSetWindowCallbackStruct.display      = pAppDisplay;
    aNPSetWindowCallbackStruct.visual       = (Visual*)
        XDefaultVisual( pAppDisplay, XDefaultScreen( pAppDisplay ) );
    aNPSetWindowCallbackStruct.colormap     = (Colormap)
        XDefaultColormap( pAppDisplay, XDefaultScreen( pAppDisplay ) );
    aNPSetWindowCallbackStruct.depth        =
        DefaultDepth( pAppDisplay, XDefaultScreen( pAppDisplay ) );

    /*
     *  Loop for events.
     */
    XtAppMainLoop(app_context);
}
