/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

static PRBool sXPCOMInitializedFlag = PR_FALSE;
#endif


extern "C" void NS_SetupRegistry();

 const PRUnichar* determineProfile( PRUnichar const* const* _pValidProfiles, const PRUint32 _nValidProfiles )
{
    
    const PRUnichar* pUsedProfile = *_pValidProfiles;

    
    
    const PRUnichar* pPreferredProfile = reinterpret_cast_mingw_only< const PRUnichar* >( getUserProfile( ) );
    if ( pPreferredProfile && *pPreferredProfile )
    {
        PRUnichar const* const* pLoop = _pValidProfiles;
        PRUnichar const* const* pLoopEnd = pLoop + _nValidProfiles;
        for ( ; pLoop != pLoopEnd; ++pLoop )
        {
            
            
            const PRUnichar* pCurrent   = *pLoop;
            const PRUnichar* pPref      = pPreferredProfile;
            while ( *pCurrent && ( *pCurrent == *pPref ) )  
            {
                ++pCurrent;
                ++pPref;
            }
            if ( *pCurrent == *pPref )
                
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
    

#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
    
    
    sInitCounter++;
#endif 

    
#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
    
    if (!sXPCOMInitializedFlag)
#endif
    {
        nsCOMPtr<nsILocalFile> binDir;
        
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



    
    NS_InitXPCOM2(&sServiceManager, binDir, NULL);




#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
        sXPCOMInitializedFlag = PR_TRUE;
#endif
    }


    
    //
    
    
    
    nsCOMPtr<nsIEventQueueService> eventQService(
                do_GetService(NS_EVENTQUEUESERVICE_CONTRACTID, &rv));
    if (NS_FAILED(rv))
      return NS_SUCCEEDED( rv ) ? sal_True : sal_False;

    eventQService->CreateThreadEventQueue();






#ifdef HACK_AROUND_THREADING_ISSUES
    
    nsCOMPtr<nsIStringBundleService> sBundleService;
    sBundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv))
    {
        nsCOMPtr<nsIStringBundle> stringBundle;
        const char* propertyURL = "chrome:
        rv = sBundleService->CreateBundle(propertyURL,
                                          getter_AddRefs(stringBundle));
    }
#endif

    
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
    m_aUI_Thread_Condition.set(); 
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

    
    if (!MNS_InitXPCOM(aProfileExists))
    {
        m_aUI_Thread_Condition.set(); 
        return;
    }

    
    MNS_XPCOM_EventLoop();
    

    if (sServiceManager)
    {
        NS_RELEASE(sServiceManager);

    
#ifndef HACK_AROUND_NONREENTRANT_INITXPCOM
        NS_ShutdownXPCOM(sServiceManager);
#endif
    }

    m_aUI_Thread_Condition.set();   

    OSL_TRACE( "OUT : MNS_Mozilla_UI_Thread()" );

}


sal_Bool MNS_Init(sal_Bool& aProfileExists)
{
    aProfileExists = sal_False ;

    OSL_TRACE( "IN : MNS_Init()" );
    
    sInitCounter++;
    if (sInitCounter > 1) {
        OSL_TRACE( "IN : MNS_Init() wait for xpcom to be initted" );
        
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

    
    m_aUI_Thread_Condition.wait();

    
    MNSTerminateListener::addTerminateListener();

    OSL_TRACE( "OUT : MNS_Init() - First Init" );

    return sal_True;
}

sal_Bool MNS_Term(sal_Bool aForce)
{
    
    OSL_TRACE( "IN : MNS_Term()" );
    if (!aForce && sInitCounter > 1)
    {
        --sInitCounter;
        OSL_TRACE( "OUT : MNS_Term() : counter = %d", sInitCounter );
        return sal_True;
    }
    sInitCounter = 0;

    aLive=0;

    
    TimeValue               timeValue = { 1, 0 };
    m_aUI_Thread_Condition.wait(&timeValue);


    OSL_TRACE( "OUT : MNS_Term() - Final Term" );
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
