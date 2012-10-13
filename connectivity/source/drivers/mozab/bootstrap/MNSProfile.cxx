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


#include "MNSProfile.hxx"
#include "MNSProfileDirServiceProvider.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::mozilla;

using ::rtl::OUString;

// Interfaces Needed

static Reference<XMozillaBootstrap> xMozillaBootstrap;


static PRInt32          gInstanceCount = 0;

// Profile database to remember which profile has been
// created with UILocale and contentLocale on profileManager
static nsProfileDirServiceProvider *gDirServiceProvider = nsnull;




/*
 * Constructor/Destructor
 */
nsProfile::nsProfile()
{
    mStartingUp = PR_FALSE;
    mCurrentProfileAvailable = PR_FALSE;

    mShutdownProfileToreDownNetwork = PR_FALSE;

    mProfileChangeVetoed = PR_FALSE;
    mProfileChangeFailed = PR_FALSE;
    Reference<XMultiServiceFactory> xFactory = ::comphelper::getProcessServiceFactory();
    OSL_ENSURE( xFactory.is(), "can't get service factory" );

    Reference<XInterface> xInstance = xFactory->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap")) );
    OSL_ENSURE( xInstance.is(), "failed to create instance" );
    xMozillaBootstrap = Reference<XMozillaBootstrap>(xInstance,UNO_QUERY);


}

nsProfile::~nsProfile()
{
}


nsresult
nsProfile::Init()
{
    nsresult rv = NS_OK;
    RegisterProfileManager(this);
    if (gInstanceCount++ == 0) {
        rv = NS_NewProfileDirServiceProvider( &gDirServiceProvider);
        if (NS_SUCCEEDED(rv))
            rv = gDirServiceProvider->Register();
    }
    return rv;
}

/*
 * nsISupports Implementation
 */
NS_IMPL_THREADSAFE_ADDREF(nsProfile)
NS_IMPL_THREADSAFE_RELEASE(nsProfile)

NS_INTERFACE_MAP_BEGIN(nsProfile)
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIProfile)
    NS_INTERFACE_MAP_ENTRY(nsIProfile)
    NS_INTERFACE_MAP_ENTRY(nsIProfileInternal)
    NS_INTERFACE_MAP_ENTRY(nsIProfileChangeStatus)
    NS_INTERFACE_MAP_ENTRY(nsIFactory)
NS_INTERFACE_MAP_END

/*
 * nsIProfileChangeStatus Implementation
 */

NS_IMETHODIMP nsProfile::VetoChange()
{
    mProfileChangeVetoed = PR_TRUE;
    return NS_OK;
}

NS_IMETHODIMP nsProfile::ChangeFailed()
{
    mProfileChangeFailed = PR_TRUE;
    return NS_OK;
}
/*
 * nsIProfile Implementation
 */

NS_IMETHODIMP nsProfile::GetProfileCount(PRInt32 *numProfiles)
{
    NS_ENSURE_ARG_POINTER(numProfiles);

    *numProfiles = 0;

    *numProfiles = xMozillaBootstrap->getProfileCount(xMozillaBootstrap->getCurrentProduct());
    return NS_OK;
}
NS_IMETHODIMP nsProfile::GetProfileList(PRUint32 *length, PRUnichar ***profileNames)
{
    NS_ENSURE_ARG_POINTER(length);
    NS_ENSURE_ARG_POINTER(profileNames);
    *profileNames = nsnull;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > list;

    *length = xMozillaBootstrap->getProfileList(xMozillaBootstrap->getCurrentProduct(),list);

    const OUString * pArray = list.getConstArray();

    nsresult rv = NS_OK;
    PRInt32 count = list.getLength();
    PRUnichar **outArray, **next;

    next = outArray = (PRUnichar **)nsMemory::Alloc(count * sizeof(PRUnichar *));
    if (!outArray)
        return NS_ERROR_OUT_OF_MEMORY;

    for (PRInt32 index=0; index < count; index++)
    {
        *next = (PRUnichar *)nsMemory::Clone(pArray[index].getStr(),(pArray[index].getLength()+1) * sizeof(PRUnichar ));
        if (*next == nsnull)
        {
            rv = NS_ERROR_OUT_OF_MEMORY;
            break;
        }
        next++;
    }
    *profileNames = outArray;
    return rv;
}
NS_IMETHODIMP nsProfile::ProfileExists(const PRUnichar *profileName, PRBool *exists)
{
    NS_ENSURE_ARG_POINTER(profileName);
    NS_ENSURE_ARG_POINTER(exists);

    // PRUnichar != sal_Unicode in mingw
    *exists = xMozillaBootstrap->getProfileExists(xMozillaBootstrap->getCurrentProduct(),reinterpret_cast_mingw_only<const sal_Unicode *>(profileName));
    return NS_OK;
}
// Returns the name of the current profile i.e., the last used profile
NS_IMETHODIMP
nsProfile::GetCurrentProfile(PRUnichar **profileName)
{
    NS_ENSURE_ARG_POINTER(profileName);
    *profileName = nsnull;

    if (mCurrentProfileName.Length())
        *profileName = (PRUnichar *)nsMemory::Clone(mCurrentProfileName.get(),(mCurrentProfileName.Length() + 1) * sizeof(PRUnichar ));
    else
    {
        rtl::OUString profile = xMozillaBootstrap->getDefaultProfile(xMozillaBootstrap->getCurrentProduct());
        *profileName = (PRUnichar *)nsMemory::Clone(profile.getStr(),( profile.getLength() + 1) * sizeof(PRUnichar ));
        SetCurrentProfile(*profileName);
    }

    return (*profileName == nsnull) ? NS_ERROR_FAILURE : NS_OK;
}

NS_IMETHODIMP
nsProfile::SetCurrentProfile(const PRUnichar * aCurrentProfile)
{
    NS_ENSURE_ARG(aCurrentProfile);

    nsresult rv;
    nsCOMPtr<nsIFile> profileDir;
    PRBool exists;

    // Ensure that the profile exists and its directory too.
    rv = GetProfileDir(aCurrentProfile, getter_AddRefs(profileDir));
    if (NS_FAILED(rv)) return rv;
    rv = profileDir->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists) return NS_ERROR_FILE_NOT_FOUND;

    PRBool isSwitch = PRBool();

    if (mCurrentProfileAvailable)
    {
        nsXPIDLString currProfileName;
        rv = GetCurrentProfile(getter_Copies(currProfileName));
        if (NS_FAILED(rv)) return rv;
        if (nsCRT::strcmp(aCurrentProfile, currProfileName.get()) == 0)
            return NS_OK;
        else
            isSwitch = PR_TRUE;
    }
    else
        isSwitch = PR_FALSE;


    nsCOMPtr<nsIObserverService> observerService =
             do_GetService("@mozilla.org/observer-service;1", &rv);
    NS_ENSURE_TRUE(observerService, NS_ERROR_FAILURE);

    nsISupports *subject = (nsISupports *)((nsIProfile *)this);
    NS_NAMED_LITERAL_STRING(switchString, "switch");
    NS_NAMED_LITERAL_STRING(startupString, "startup");
    const nsAFlatString& context = isSwitch ? switchString : startupString;

    if (isSwitch)
    {
        rv = ShutDownCurrentProfile(nsIProfile::SHUTDOWN_PERSIST);
        if (NS_FAILED(rv)) return NS_ERROR_ABORT;

        // Phase 1: See if anybody objects to the profile being changed.
        mProfileChangeVetoed = PR_FALSE;
        observerService->NotifyObservers(subject, "profile-approve-change", context.get());
        if (mProfileChangeVetoed)
            return NS_OK;

        // Phase 2a: Send the network teardown notification
        observerService->NotifyObservers(subject, "profile-change-net-teardown", context.get());
        mShutdownProfileToreDownNetwork = PR_TRUE;

        // Phase 2b: Send the "teardown" notification
        observerService->NotifyObservers(subject, "profile-change-teardown", context.get());
        if (mProfileChangeVetoed)
        {
            // Notify we will not proceed with changing the profile
            observerService->NotifyObservers(subject, "profile-change-teardown-veto", context.get());

            // Bring network back online and return
            observerService->NotifyObservers(subject, "profile-change-net-restore", context.get());
            return NS_OK;
        }

        // Phase 3: Notify observers of a profile change
        observerService->NotifyObservers(subject, "profile-before-change", context.get());
        if (mProfileChangeFailed)
          return NS_ERROR_ABORT;

    }

//
//  // Do the profile switch
    gDirServiceProvider->SetProfileDir(profileDir);
    mCurrentProfileName.Assign(aCurrentProfile);
//    gProfileDataAccess->SetCurrentProfile(aCurrentProfile);

    if (NS_FAILED(rv)) return rv;
    mCurrentProfileAvailable = PR_TRUE;

    if (!isSwitch)
    {
        // Ensure that the prefs service exists so it can respond to
        // the notifications we're about to send around. It needs to.
        nsCOMPtr<nsIPrefService> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
        NS_ASSERTION(NS_SUCCEEDED(rv), "Could not get prefs service");
    }

    if (mShutdownProfileToreDownNetwork)
    {
        // Bring network back online
        observerService->NotifyObservers(subject, "profile-change-net-restore", context.get());
        mShutdownProfileToreDownNetwork = PR_FALSE;
        if (mProfileChangeFailed)
          return NS_ERROR_ABORT;
    }

    // Phase 4: Notify observers that the profile has changed - Here they respond to new profile
    observerService->NotifyObservers(subject, "profile-do-change", context.get());
    if (mProfileChangeFailed)
      return NS_ERROR_ABORT;

    // Phase 5: Now observers can respond to something another observer did in phase 4
    observerService->NotifyObservers(subject, "profile-after-change", context.get());
    if (mProfileChangeFailed)
      return NS_ERROR_ABORT;

    // Phase 6: One last notification after the new profile is established
    observerService->NotifyObservers(subject, "profile-initial-state", context.get());
    if (mProfileChangeFailed)
      return NS_ERROR_ABORT;

    return NS_OK;
}
/* void shutDownCurrentProfile (in unsigned long shutDownType); */
NS_IMETHODIMP nsProfile::ShutDownCurrentProfile(PRUint32 shutDownType)
{
    nsresult rv;

    // if shutDownType is not a well know value, skip the notifications
    // see DoOnShutdown() in nsAppRunner.cpp for where we use this behaviour to our benefit
    if (shutDownType == (PRUint32)SHUTDOWN_PERSIST || shutDownType == (PRUint32)SHUTDOWN_CLEANSE ) {
      nsCOMPtr<nsIObserverService> observerService =
        do_GetService("@mozilla.org/observer-service;1", &rv);
      NS_ENSURE_TRUE(observerService, NS_ERROR_FAILURE);

      nsISupports *subject = (nsISupports *)((nsIProfile *)this);

      NS_NAMED_LITERAL_STRING(cleanseString, "shutdown-cleanse");
      NS_NAMED_LITERAL_STRING(persistString, "shutdown-persist");
      const nsAFlatString& context = (shutDownType == (PRUint32)SHUTDOWN_CLEANSE) ? cleanseString : persistString;

      // Phase 1: See if anybody objects to the profile being changed.
      mProfileChangeVetoed = PR_FALSE;
      observerService->NotifyObservers(subject, "profile-approve-change", context.get());
      if (mProfileChangeVetoed)
        return NS_OK;

      // Phase 2a: Send the network teardown notification
      observerService->NotifyObservers(subject, "profile-change-net-teardown", context.get());
      mShutdownProfileToreDownNetwork = PR_TRUE;

      // Phase 2b: Send the "teardown" notification
      observerService->NotifyObservers(subject, "profile-change-teardown", context.get());


      // Phase 3: Notify observers of a profile change
      observerService->NotifyObservers(subject, "profile-before-change", context.get());
    }


    gDirServiceProvider->SetProfileDir(nsnull);
    mCurrentProfileAvailable = PR_FALSE;
    mCurrentProfileName.Truncate(0);

    return NS_OK;
}
/* void createNewProfile (in wstring profileName, in wstring nativeProfileDir, in wstring langcode, in boolean useExistingDir); */
NS_IMETHODIMP nsProfile::CreateNewProfile(const PRUnichar* /*profileName*/, const PRUnichar* /*nativeProfileDir*/, const PRUnichar* /*langcode*/, PRBool /*useExistingDir*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void renameProfile (in wstring oldName, in wstring newName); */
NS_IMETHODIMP nsProfile::RenameProfile(const PRUnichar* /*oldName*/, const PRUnichar* /*newName*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void deleteProfile (in wstring name, in boolean canDeleteFiles); */
NS_IMETHODIMP nsProfile::DeleteProfile(const PRUnichar* /*name*/, PRBool /*canDeleteFiles*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void cloneProfile (in wstring profileName); */
NS_IMETHODIMP nsProfile::CloneProfile(const PRUnichar* /*profileName*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
//nsIProfileInternal Implementation

/* [noscript] void startupWithArgs (in nsICmdLineService cmdLine, in boolean canInteract); */
class nsICmdLineService;

NS_IMETHODIMP nsProfile::StartupWithArgs(nsICmdLineService* /*cmdLineArgs*/, PRBool /*canInteract*/)
{
    Init();
    return NS_OK;
}

/* [noscript] readonly attribute boolean isStartingUp; */
NS_IMETHODIMP nsProfile::GetIsStartingUp(PRBool *aIsStartingUp)
{
    NS_ENSURE_ARG_POINTER(aIsStartingUp);
    *aIsStartingUp = mStartingUp;
    return NS_OK;
}

/* long get4xProfileCount (); */
NS_IMETHODIMP nsProfile::Get4xProfileCount(PRInt32* /*_retval*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getProfileListX (in unsigned long which, out unsigned long length, [array, size_is (length), retval] out wstring profileNames); */
NS_IMETHODIMP nsProfile::GetProfileListX(PRUint32 /*whichKind*/, PRUint32* /*length*/, PRUnichar*** /*profileNames*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void migrateProfileInfo (); */
NS_IMETHODIMP nsProfile::MigrateProfileInfo()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void migrateAllProfiles (); */
NS_IMETHODIMP nsProfile::MigrateAllProfiles()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void migrateProfile (in wstring profileName); */
NS_IMETHODIMP nsProfile::MigrateProfile(const PRUnichar* /*profileName*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void remigrateProfile (in wstring profileName); */
NS_IMETHODIMP nsProfile::RemigrateProfile(const PRUnichar* /*profileName*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void forgetCurrentProfile (); */
NS_IMETHODIMP nsProfile::ForgetCurrentProfile()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void createDefaultProfile (); */
NS_IMETHODIMP nsProfile::CreateDefaultProfile()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIFile getProfileDir (in wstring profileName); */
// Gets the profiles directory for a given profile
// Sets the given profile to be a current profile
NS_IMETHODIMP nsProfile::GetProfileDir(const PRUnichar *profileName, nsIFile **profileDir)
{
    NS_ENSURE_ARG(profileName);
    NS_ENSURE_ARG_POINTER(profileDir);
    *profileDir = nsnull;

    // PRUnichar != sal_Unicode in mingw
    rtl::OUString path = xMozillaBootstrap->getProfilePath(xMozillaBootstrap->getCurrentProduct(),reinterpret_cast_mingw_only<const sal_Unicode *>(profileName));

    nsCOMPtr<nsILocalFile>  localFile;
    // PRUnichar != sal_Unicode in mingw
    nsAutoString filePath(reinterpret_cast_mingw_only<const PRUnichar *>(path.getStr()));

    nsresult rv = NS_NewLocalFile(filePath, PR_TRUE,
                                   getter_AddRefs(localFile));
    if (localFile && NS_SUCCEEDED(rv))
        return localFile->QueryInterface(NS_GET_IID(nsIFile), (void**)profileDir);
    return rv;
}

/* wstring getProfilePath (in wstring profileName); */
NS_IMETHODIMP nsProfile::GetProfilePath(const PRUnichar* /*profileName*/, PRUnichar** /*_retval*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsILocalFile getOriginalProfileDir (in wstring profileName); */
NS_IMETHODIMP nsProfile::GetOriginalProfileDir(const PRUnichar* /*profileName*/, nsILocalFile** /*originalDir*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* PRInt64 getProfileLastModTime (in wstring profileName); */
NS_IMETHODIMP nsProfile::GetProfileLastModTime(const PRUnichar* /*profileName*/, PRInt64* /*_retval*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean automigrate; */
NS_IMETHODIMP nsProfile::GetAutomigrate(PRBool* /*aAutomigrate*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsProfile::SetAutomigrate(PRBool /*aAutomigrate*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIFile defaultProfileParentDir; */
NS_IMETHODIMP nsProfile::GetDefaultProfileParentDir(nsIFile** /*aDefaultProfileParentDir*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute wstring firstProfile; */
NS_IMETHODIMP nsProfile::GetFirstProfile(PRUnichar **profileName)
{
    NS_ENSURE_ARG_POINTER(profileName);

    return GetCurrentProfile(profileName);
}

/* attribute boolean startWithLastUsedProfile; */
NS_IMETHODIMP nsProfile::GetStartWithLastUsedProfile(PRBool* /*aStartWithLastUsedProfile*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsProfile::SetStartWithLastUsedProfile(PRBool /*aStartWithLastUsedProfile*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void updateRegistry (in nsIFile regName); */
NS_IMETHODIMP nsProfile::UpdateRegistry(nsIFile* /*regName*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void getRegStrings (in wstring profileName, out wstring regString, out wstring regName, out wstring regEmail, out wstring regOption); */
NS_IMETHODIMP nsProfile::GetRegStrings(const PRUnichar* /*profileName*/, PRUnichar** /*regString*/, PRUnichar** /*regName*/, PRUnichar** /*regEmail*/, PRUnichar** /*regOption*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void setRegStrings (in wstring profileName, in wstring regString, in wstring regName, in wstring regEmail, in wstring regOption); */
NS_IMETHODIMP nsProfile::SetRegStrings(const PRUnichar* /*profileName*/, const PRUnichar* /*regString*/, const PRUnichar* /*regName*/, const PRUnichar* /*regEmail*/, const PRUnichar* /*regOption*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] string isRegStringSet (in wstring profileName); */
NS_IMETHODIMP nsProfile::IsRegStringSet(const PRUnichar* /*profileName*/, char** /*_retval*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void createNewProfileWithLocales (in wstring profileName, in wstring nativeProfileDir, in wstring UILocale, in wstring contentLocale, in boolean useExistingDir); */
NS_IMETHODIMP nsProfile::CreateNewProfileWithLocales(const PRUnichar* /*profileName*/, const PRUnichar* /*nativeProfileDir*/, const PRUnichar* /*UILocale*/, const PRUnichar* /*contentLocale*/, PRBool /*useExistingDir*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isCurrentProfileAvailable (); */
NS_IMETHODIMP nsProfile::IsCurrentProfileAvailable(PRBool *available)
{
    NS_ENSURE_ARG_POINTER(available);

    *available = mCurrentProfileAvailable;
    return NS_OK;
}

/* [noscript] void getCurrentProfileDir (out nsIFile profileDir); */
// Returns the name of the current profile directory
NS_IMETHODIMP nsProfile::GetCurrentProfileDir(nsIFile **profileDir)
{
    NS_ENSURE_ARG_POINTER(profileDir);
    nsresult rv;

    nsXPIDLString profileName;
    rv = GetCurrentProfile(getter_Copies(profileName));
    if (NS_FAILED(rv)) return rv;

    rv = GetProfileDir(profileName, profileDir);
    if (NS_FAILED(rv)) return rv;

    return NS_OK;
}

//Implementation nsIFactory
NS_IMETHODIMP
nsProfile::LockFactory(PRBool /*aVal*/)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsProfile::CreateInstance(nsISupports* aOuter, const nsID& aIID,
                                        void** aResult)
{
    if (aOuter)
        return NS_ERROR_NO_AGGREGATION;
    // return this object
    return QueryInterface(aIID, aResult);
}

//Register profile manager
#include "pre_include_mozilla.h"
#include "nsIComponentManager.h"
#include "nsIComponentRegistrar.h"
#include "post_include_mozilla.h"

static const nsCID kProfileManagerCID = NS_PROFILE_CID;

#define NS_PROFILE_CONTRACTID   \
    "@mozilla.org/profile/manager;1"

nsresult
nsProfile::RegisterProfileManager(nsIProfile* aProfileService)
{
    nsCOMPtr<nsIServiceManager>     mgr;
    NS_GetServiceManager(getter_AddRefs(mgr));

    NS_ASSERTION(mgr, "Not initialized!");

    nsCOMPtr<nsIFactory> factory = do_QueryInterface(aProfileService);
    NS_ASSERTION(factory, "Supposed to be an nsIFactory!");

    nsCOMPtr<nsIComponentRegistrar> reg (do_QueryInterface(mgr));
    if (!reg)
        return NS_ERROR_NO_INTERFACE;

    return reg->RegisterFactory(kProfileManagerCID,
                             "Profile Manager",
                             NS_PROFILE_CONTRACTID,
                             factory);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
