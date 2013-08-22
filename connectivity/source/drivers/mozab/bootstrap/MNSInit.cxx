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

#include "mozillasrc/MNSInclude.hxx"

#include "mozilla_nsinit.h"

#include "MNSInit.hxx"

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/conditn.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>

#include "MConfigAccess.hxx"
#include "mozillasrc/MNSDeclares.hxx"
#include <osl/thread.hxx>
#include "mozillasrc/MNSTerminateListener.hxx"

static nsIServiceManager*   sServiceManager = nsnull;
static sal_Int32            sInitCounter = 0;
static sal_Bool             s_bProfilePresentAfterInitialized = sal_False;

static NS_DEFINE_CID(kProfileCID, NS_PROFILE_CID);
static NS_DEFINE_CID(kPrefCID, NS_PREF_CID);

static NS_DEFINE_CID(kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID);
static PRBool aLive=1;
static ::osl::Condition    m_aUI_Thread_Condition;

#define HACK_AROUND_THREADING_ISSUES
#define HACK_AROUND_NONREENTRANT_INITXPCOM

#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
// XXX hack class to clean up XPCOM when this module is unloaded
static PRBool sXPCOMInitializedFlag = PR_FALSE;
#endif


extern "C" void NS_SetupRegistry();

 const PRUnichar* determineProfile( PRUnichar const* const* _pValidProfiles, const PRUint32 _nValidProfiles )
{
    // the fallback for the to-be-used user profile: the first profile
    const PRUnichar* pUsedProfile = *_pValidProfiles;

    // have a look what the configuration suggests as preferred profile
    // PRUnichar != sal_Unicode in mingw
    const PRUnichar* pPreferredProfile = reinterpret_cast_mingw_only< const PRUnichar* >( getUserProfile( ) );
    if ( pPreferredProfile && *pPreferredProfile )
    {
        PRUnichar const* const* pLoop = _pValidProfiles;
        PRUnichar const* const* pLoopEnd = pLoop + _nValidProfiles;
        for ( ; pLoop != pLoopEnd; ++pLoop )
        {
            // compare the current and the preferred profile
            // (by looping through the characters)
            const PRUnichar* pCurrent   = *pLoop;
            const PRUnichar* pPref      = pPreferredProfile;
            while ( *pCurrent && ( *pCurrent == *pPref ) )  // testing one of them against 0 is enough because of the second clause
            {
                ++pCurrent;
                ++pPref;
            }
            if ( *pCurrent == *pPref )
                // the are equal
                break;
        }

        if ( pLoop != pLoopEnd )
            pUsedProfile = *pLoop;
        return pUsedProfile;
    }
    else
        return NULL;
}


sal_Bool MNS_InitXPCOM(sal_Bool* aProfileExists)
{
    nsresult rv;
    OSL_TRACE( "IN : MNS_InitXPCOM()" );
    // Reentrant calls to this method do nothing except increment a counter

#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
    // The first time, add another increment so that it'll be left until exit
    // for the final cleanup to happen
    sInitCounter++;
#endif // HACK_AROUND_NONREENTRANT_INITXPCOM

    // Initialise XPCOM
#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
    // Can't call NS_InitXPCom more than once or things go boom!
    if (!sXPCOMInitializedFlag)
#endif
    {
        nsCOMPtr<nsILocalFile> binDir;
        // Note: if path3 construction fails, mozilla will default to using MOZILLA_FIVE_HOME in the NS_InitXPCOM2()
        OUString path1("$BRAND_BASE_DIR/program");
        rtl::Bootstrap::expandMacros(path1);
        OString path2;
        if ((osl::FileBase::getSystemPathFromFileURL(path1, path1) ==
             osl::FileBase::E_None) &&
            path1.convertToString(
                &path2, osl_getThreadTextEncoding(),
                (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                 RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
        {
            nsDependentCString sPath(path2.getStr());
            rv = NS_NewNativeLocalFile(sPath, PR_TRUE, getter_AddRefs(binDir));
            if (NS_FAILED(rv))
                return sal_False;
        }



    // Initialise XPCOM
    NS_InitXPCOM2(&sServiceManager, binDir, NULL);

//      if (!sServiceManager)
//          return sal_False;

#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
        sXPCOMInitializedFlag = PR_TRUE;
#endif
    }


    // Create the Event Queue for the UI thread...
    //
    // If an event queue already exists for the thread, then
    // CreateThreadEventQueue(...) will fail...
    // CreateThread0ueue(...) will fail...
    nsCOMPtr<nsIEventQueueService> eventQService(
                do_GetService(NS_EVENTQUEUESERVICE_CONTRACTID, &rv));
    if (NS_FAILED(rv))
      return NS_SUCCEEDED( rv ) ? sal_True : sal_False;

    eventQService->CreateThreadEventQueue();

//  nsCOMPtr<nsIObserver> mStartupNotifier = do_CreateInstance(NS_APPSTARTUPNOTIFIER_CONTRACTID, &rv);
//  if(NS_FAILED(rv))
//      return rv;
//  mStartupNotifier->Observe(nsnull, APPSTARTUP_TOPIC, nsnull);

#ifdef HACK_AROUND_THREADING_ISSUES
    // XXX force certain objects to be created on the main thread
    nsCOMPtr<nsIStringBundleService> sBundleService;
    sBundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv))
    {
        nsCOMPtr<nsIStringBundle> stringBundle;
        const char* propertyURL = "chrome://necko/locale/necko.properties";
        rv = sBundleService->CreateBundle(propertyURL,
                                          getter_AddRefs(stringBundle));
    }
#endif

    // Need to create a Pref Service
    nsCOMPtr< nsIPref > thePref = do_GetService( kPrefCID, &rv );
    if (NS_SUCCEEDED(rv) )
    {
        OSL_TRACE("Created an nsIPref i/f");
        thePref->ReadUserPrefs( nsnull );
        *aProfileExists = sal_True ;
        s_bProfilePresentAfterInitialized = sal_True;
    }
    OSL_TRACE( "OUT : MNS_InitXPCOM() - XPCOM Init" );

    return sal_True;
}

void MNS_XPCOM_EventLoop()
{
    OSL_TRACE( "IN : MNS_XPCOM_EventLoop()" );
    nsresult rv;
    nsCOMPtr<nsIEventQueue> eventQ;
    nsCOMPtr<nsIEventQueueService>  eventQService;
    rv=nsServiceManager::GetService(kEventQueueServiceCID,
                                 NS_GET_IID(nsIEventQueueService),
                                 getter_AddRefs(eventQService));

    if (NS_SUCCEEDED(rv))
    {
      rv = eventQService->GetThreadEventQueue(NS_CURRENT_THREAD , getter_AddRefs(eventQ));
      if (NS_FAILED(rv))
      {
          rv = eventQService->CreateThreadEventQueue();
          if (NS_FAILED(rv))
            return ;
          else
            rv = eventQService->GetThreadEventQueue(NS_CURRENT_THREAD, getter_AddRefs(eventQ));
      }
    }

    if (NS_FAILED(rv)) return ;

    PLEvent* event = nsnull;
    m_aUI_Thread_Condition.set(); //we are ready to recive event
    do
    {
        rv = eventQ->GetEvent(&event);
        if (NS_FAILED(rv))
            return ;
        if (event)
            eventQ->HandleEvent(event);
    }while ( PR_SUCCESS == PR_Sleep( PR_MillisecondsToInterval(20)) && aLive );

    eventQ->ProcessPendingEvents();
    OSL_TRACE( "OUT : MNS_XPCOM_EventLoop()" );
}

extern "C" void MNS_Mozilla_UI_Thread( void *arg )
{
    aLive=1;
    OSL_TRACE( "IN : MNS_Mozilla_UI_Thread()" );
    UI_Thread_ARGS * args = (UI_Thread_ARGS*) arg;
    sal_Bool* aProfileExists=args->bProfileExists;
    delete args;
    args=NULL;

    //Init xpcom
    if (!MNS_InitXPCOM(aProfileExists))
    {
        m_aUI_Thread_Condition.set(); // error happened
        return;
    }

    //do the mozilla event loop
    MNS_XPCOM_EventLoop();
    //we are interupted

    if (sServiceManager)
    {
        NS_RELEASE(sServiceManager);

    // Terminate XPCOM & cleanup
#ifndef HACK_AROUND_NONREENTRANT_INITXPCOM
        NS_ShutdownXPCOM(sServiceManager);
#endif
    }

    m_aUI_Thread_Condition.set();   //release all blocks

    OSL_TRACE( "OUT : MNS_Mozilla_UI_Thread()" );

}


sal_Bool MNS_Init(sal_Bool& aProfileExists)
{
    aProfileExists = sal_False ;

    OSL_TRACE( "IN : MNS_Init()" );
    // Reentrant calls to this method do nothing except increment a counter
    sInitCounter++;
    if (sInitCounter > 1) {
        OSL_TRACE( "IN : MNS_Init() wait for xpcom to be initted" );
        //wait for xpcom to be initted
        m_aUI_Thread_Condition.wait();

        OSL_TRACE( "OUT : MNS_Init() : counter = %d", sInitCounter );
        aProfileExists = s_bProfilePresentAfterInitialized;
        return sal_True;
    }

    UI_Thread_ARGS * args = new UI_Thread_ARGS;
    args->bProfileExists = &aProfileExists;

    m_aUI_Thread_Condition.reset();
    if (osl_createThread(MNS_Mozilla_UI_Thread, (void*)args) == 0)
    {
        return sal_False;
    }

    //wait for xpcom to be initted
    m_aUI_Thread_Condition.wait();

    //Add Terminate Listener to XDesktop to get application exit event
    MNSTerminateListener::addTerminateListener();

    OSL_TRACE( "OUT : MNS_Init() - First Init" );

    return sal_True;
}

sal_Bool MNS_Term(sal_Bool aForce)
{
    // Reentrant calls to this method do nothing except decrement a counter
    OSL_TRACE( "IN : MNS_Term()" );
    if (!aForce && sInitCounter > 1)
    {
        --sInitCounter;
        OSL_TRACE( "OUT : MNS_Term() : counter = %d", sInitCounter );
        return sal_True;
    }
    sInitCounter = 0;

    aLive=0;

    //wait for xpcom to be finished
    TimeValue               timeValue = { 1, 0 };
    m_aUI_Thread_Condition.wait(&timeValue);


    OSL_TRACE( "OUT : MNS_Term() - Final Term" );
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
