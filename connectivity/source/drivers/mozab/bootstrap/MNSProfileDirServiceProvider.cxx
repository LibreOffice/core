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


#if defined _MSC_VER
    #pragma warning(disable:4710)
#endif

#include "pre_include_mozilla.h"
#include "MNSProfileDirServiceProvider.hxx"
#include "nsIAtom.h"
#include "nsStaticAtom.h"
#include "nsILocalFile.h"
#include "nsDirectoryServiceDefs.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsISupportsUtils.h"
#include "post_include_mozilla.h"



#define PREFS_FILE_50_NAME           NS_LITERAL_CSTRING("prefs.js")


nsIAtom*   nsProfileDirServiceProvider::sApp_PrefsDirectory50;
nsIAtom*   nsProfileDirServiceProvider::sApp_PreferencesFile50;
nsIAtom*   nsProfileDirServiceProvider::sApp_UserProfileDirectory50;


//*****************************************************************************
// nsProfileDirServiceProvider::nsProfileDirServiceProvider
//*****************************************************************************

nsProfileDirServiceProvider::nsProfileDirServiceProvider()
{
}


nsProfileDirServiceProvider::~nsProfileDirServiceProvider()
{
}

nsresult
nsProfileDirServiceProvider::SetProfileDir(nsIFile* aProfileDir)
{
  if (mProfileDir) {
    PRBool isEqual;
    if (aProfileDir &&
        NS_SUCCEEDED(aProfileDir->Equals(mProfileDir, &isEqual)) && isEqual) {
      NS_WARNING("Setting profile dir to same as current");
      return NS_OK;
    }
    UndefineFileLocations();
  }
  mProfileDir = aProfileDir;
  if (!mProfileDir)
    return NS_OK;

  nsresult rv = EnsureProfileFileExists(mProfileDir);
  return rv;

}

nsresult
nsProfileDirServiceProvider::Register()
{
  nsCOMPtr<nsIDirectoryService> directoryService =
          do_GetService(NS_DIRECTORY_SERVICE_CONTRACTID);
  if (!directoryService)
    return NS_ERROR_FAILURE;
  return directoryService->RegisterProvider(this);
}

nsresult
nsProfileDirServiceProvider::Shutdown()
{
  nsCOMPtr<nsIDirectoryService> directoryService =
          do_GetService(NS_DIRECTORY_SERVICE_CONTRACTID);
  if (!directoryService)
    return NS_ERROR_FAILURE;
  return directoryService->UnregisterProvider(this);
}

// nsProfileDirServiceProvider::nsISupports

NS_IMPL_THREADSAFE_ISUPPORTS1(nsProfileDirServiceProvider,
                              nsIDirectoryServiceProvider)

// nsProfileDirServiceProvider::nsIDirectoryServiceProvider

NS_IMETHODIMP
nsProfileDirServiceProvider::GetFile(const char *prop, PRBool *persistent, nsIFile **_retval)
{
  NS_ENSURE_ARG(prop);
  NS_ENSURE_ARG_POINTER(persistent);
  NS_ENSURE_ARG_POINTER(_retval);

  if (!mProfileDir)
    return NS_ERROR_FAILURE;

  *persistent = PR_TRUE;
  nsIFile* domainDir = mProfileDir;


  nsCOMPtr<nsIFile>  localFile;
  nsresult rv = NS_ERROR_FAILURE;

  nsIAtom* inAtom = NS_NewAtom(prop);
  NS_ENSURE_TRUE(inAtom, NS_ERROR_OUT_OF_MEMORY);

  if (inAtom == sApp_PrefsDirectory50) {
    rv = domainDir->Clone(getter_AddRefs(localFile));
  }
  else if (inAtom == sApp_PreferencesFile50) {
    rv = domainDir->Clone(getter_AddRefs(localFile));
    if (NS_SUCCEEDED(rv))
      rv = localFile->AppendNative(PREFS_FILE_50_NAME);
  }
  else if (inAtom == sApp_UserProfileDirectory50) {
    rv = domainDir->Clone(getter_AddRefs(localFile));
  }

  NS_RELEASE(inAtom);

  if (localFile && NS_SUCCEEDED(rv))
    return CallQueryInterface(localFile, _retval);

  return rv;
}

//*****************************************************************************
// Protected methods
//*****************************************************************************

nsresult
nsProfileDirServiceProvider::Initialize()
{

  static const nsStaticAtom provider_atoms[] = {
    { NS_APP_PREFS_50_DIR,           &sApp_PrefsDirectory50 },
    { NS_APP_PREFS_50_FILE,          &sApp_PreferencesFile50 },
    { NS_APP_USER_PROFILE_50_DIR,    &sApp_UserProfileDirectory50 },
    { NS_APP_PROFILE_DEFAULTS_NLOC_50_DIR, nsnull },
  };

  // Register our directory atoms
  NS_RegisterStaticAtoms(provider_atoms, NS_ARRAY_LENGTH(provider_atoms));

  return NS_OK;
}

nsresult
nsProfileDirServiceProvider::EnsureProfileFileExists(nsIFile *aFile)
{
  nsresult rv;
  PRBool exists;

  rv = aFile->Exists(&exists);
  if (NS_FAILED(rv))
    return rv;
  if (exists)
    return NS_OK;

  nsCOMPtr<nsIFile> defaultsFile;

  // Attempt first to get the localized subdir of the defaults
  rv = NS_GetSpecialDirectory(NS_APP_PROFILE_DEFAULTS_50_DIR, getter_AddRefs(defaultsFile));
  if (NS_FAILED(rv)) {
    // If that has not been defined, use the top level of the defaults
    rv = NS_GetSpecialDirectory(NS_APP_PROFILE_DEFAULTS_NLOC_50_DIR, getter_AddRefs(defaultsFile));
    if (NS_FAILED(rv))
      return rv;
  }

    mProfileDir = defaultsFile;
    return rv;
}


nsresult
nsProfileDirServiceProvider::UndefineFileLocations()
{
  nsresult rv;

  nsCOMPtr<nsIProperties> directoryService =
           do_GetService(NS_DIRECTORY_SERVICE_CONTRACTID, &rv);
  NS_ENSURE_TRUE(directoryService, NS_ERROR_FAILURE);

  (void) directoryService->Undefine(NS_APP_PREFS_50_DIR);
  (void) directoryService->Undefine(NS_APP_PREFS_50_FILE);
  (void) directoryService->Undefine(NS_APP_USER_PROFILE_50_DIR);

  return NS_OK;
}

//*****************************************************************************
// Global creation function
//*****************************************************************************

nsresult NS_NewProfileDirServiceProvider(nsProfileDirServiceProvider** aProvider)
{
  NS_ENSURE_ARG_POINTER(aProvider);
  *aProvider = nsnull;

  nsProfileDirServiceProvider *prov = new nsProfileDirServiceProvider();
  if (!prov)
    return NS_ERROR_OUT_OF_MEMORY;
  nsresult rv = prov->Initialize();
  if (NS_FAILED(rv)) {
    delete prov;
    return rv;
  }
  NS_ADDREF(*aProvider = prov);
  return NS_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
