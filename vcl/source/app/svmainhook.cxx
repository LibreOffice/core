/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svmainhook.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 14:06:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _TOOLS_H
#include <tools/tools.h>
#endif

#ifndef MACOSX

BOOL ImplSVMainHook( BOOL * )
{
    return FALSE;   // indicate that ImplSVMainHook is not implemented
}

#else
#ifdef QUARTZ  // only Mac OS X / X11 needs this trick, the Aqua version has its own native event loop
#include <premac.h>
//#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#include <stdio.h>

extern BOOL ImplSVMain();

static BOOL* gpbInit = 0;

static  pascal  void    MainRunLoopForThreadedApps( EventLoopTimerRef inTimer, void *inUserData )
{
    BOOL bRet = ImplSVMain();
    if( gpbInit )
        *gpbInit = bRet;

    QuitApplicationEventLoop();
}

BOOL ImplSVMainHook( BOOL * pbInit )
{
    gpbInit = pbInit;

    // install handlers of AppleEvents BEFORE handling event loops
    // else we miss the for initial OpenDocument apple events
    extern void InstallAEHandlers();
    InstallAEHandlers();

    EventLoopTimerRef aMainRunLoopTimerRef;
    (void) InstallEventLoopTimer( GetCurrentEventLoop(), 0, 0, NewEventLoopTimerUPP( MainRunLoopForThreadedApps ), NULL, &aMainRunLoopTimerRef );

    //  We really only call RunApplicationEventLoop() to install the default CarbonEvent handlers.  Once the timer installed above
    //  fires, we remain in the MainRunLoopForThreadedApps() routine which is designed to yield to other cooperative threads.
    RunApplicationEventLoop();

    return TRUE;   // indicate that ImplSVMainHook is implemented
}

#else  // MACOSX (X11) needs the CFRunLoop()
#include <osl/thread.h>
#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
#include <unistd.h>

extern BOOL ImplSVMain();

// ============================================================================


static void SourceContextCallBack( void *pInfo )
{
}

struct ThreadContext
{
    BOOL* pRet;
    CFRunLoopRef* pRunLoopRef;
};

static void RunSVMain(void *pData)
{
    ThreadContext* tcx = reinterpret_cast<ThreadContext*>(pData);

    // busy waiting (ok in this case) until the run loop is
    // running
    while (!CFRunLoopIsWaiting(*tcx->pRunLoopRef))
        usleep(100);

    *tcx->pRet = ImplSVMain();

    // Force exit since some JVMs won't shutdown when only exit() is invoked
    _exit( 0 );
}

BOOL ImplSVMainHook( BOOL *pbInit )
{
    // Mac OS X requires that any Cocoa code have a CFRunLoop started in the
    // primordial thread. Since all of the AWT classes in Java 1.4 and higher
    // are written in Cocoa, we need to start the CFRunLoop here and run
    // ImplSVMain() in a secondary thread.
    // See http://developer.apple.com/samplecode/simpleJavaLauncher/listing3.html
    // for further details and an example

    CFRunLoopRef runLoopRef = CFRunLoopGetCurrent();
    ThreadContext tcx;
    tcx.pRet = pbInit;  // the return value
    tcx.pRunLoopRef = &runLoopRef;
    oslThread hThreadID = osl_createThread(RunSVMain, &tcx);

    // Start the CFRunLoop
    CFRunLoopSourceContext aSourceContext;
    aSourceContext.version = 0;
    aSourceContext.info = NULL;
    aSourceContext.retain = NULL;
    aSourceContext.release = NULL;
    aSourceContext.copyDescription = NULL;
    aSourceContext.equal = NULL;
    aSourceContext.hash = NULL;
    aSourceContext.schedule = NULL;
    aSourceContext.cancel = NULL;
    aSourceContext.perform = &SourceContextCallBack;
    CFRunLoopSourceRef aSourceRef = CFRunLoopSourceCreate(NULL, 0, &aSourceContext);
    CFRunLoopAddSource(runLoopRef, aSourceRef, kCFRunLoopCommonModes);
    CFRunLoopRun();

    osl_joinWithThread( hThreadID );
    osl_destroyThread( hThreadID );

    return TRUE;    // indicate that ImplSVMainHook is implemented
}

#endif // MACOSX
#endif
