/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svmainhook.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-13 09:36:48 $
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

#ifndef _TOOLS_H
#include <tools/tools.h>
#endif

#ifndef MACOSX

BOOL ImplSVMainHook( BOOL *pbInit )
{
    return FALSE;   // indicate that ImplSVMainHook is not implemented
}

#else
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
