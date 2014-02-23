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


#include "mozilla_nsprofiledirserviceprovider.h"

// Forward Declarations
class nsIAtom;


// nsProfileDirServiceProvider - The nsIDirectoryServiceProvider implementation used for
// profile-relative file locations.


class nsProfileDirServiceProvider: public nsIDirectoryServiceProvider
{
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDIRECTORYSERVICEPROVIDER

  friend nsresult NS_NewProfileDirServiceProvider( nsProfileDirServiceProvider**);

public:

   virtual nsresult        SetProfileDir(nsIFile* aProfileDir);
   virtual nsresult         Register();
   virtual nsresult         Shutdown();

protected:
                           nsProfileDirServiceProvider();
   virtual                 ~nsProfileDirServiceProvider();

  nsresult                 Initialize();
  nsresult                 EnsureProfileFileExists(nsIFile* profileDir);
  nsresult                 UndefineFileLocations();

protected:

  // Atoms for file locations
  static nsIAtom*          sApp_PrefsDirectory50;
  static nsIAtom*          sApp_PreferencesFile50;
  static nsIAtom*          sApp_UserProfileDirectory50;

  nsCOMPtr<nsIFile>        mProfileDir;

};




/**
 * Global method to create an instance of nsProfileDirServiceProvider
 *
 * @param aNotifyObservers    If true, will send out profile startup
 *                            notifications when the profile directory is set.
 *                            See nsIProfileChangeStatus.
 */

nsresult NS_NewProfileDirServiceProvider( nsProfileDirServiceProvider** aProvider);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
