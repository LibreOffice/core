/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/tools.h>

#ifndef MACOSX

sal_Bool ImplSVMainHook( sal_Bool * )
{
    return sal_False;   // indicate that ImplSVMainHook is not implemented
}

#else
// MACOSX cocoa implementation of ImplSVMainHook is in aqua/source/app/salinst.cxx
#ifndef QUARTZ  // MACOSX (X11) needs the CFRunLoop()
#include <osl/thread.h>
#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
#include <unistd.h>

extern sal_Bool ImplSVMain();

// ============================================================================


static void SourceContextCallBack( void *pInfo )
{
}

struct ThreadContext
{
    sal_Bool* pRet;
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

sal_Bool ImplSVMainHook( sal_Bool *pbInit )
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

    return sal_True;    // indicate that ImplSVMainHook is implemented
}

#endif // MACOSX
#endif
