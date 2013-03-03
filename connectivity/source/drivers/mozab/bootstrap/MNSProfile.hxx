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

#include "mozilla_nsprofile.h"

#define _MAX_LENGTH   256

class nsProfile: public nsIProfileInternal,
                 public nsIProfileChangeStatus,
                 public nsIFactory
{
    NS_DECL_ISUPPORTS
    NS_DECL_NSIPROFILE
    NS_DECL_NSIPROFILEINTERNAL
    NS_DECL_NSIPROFILECHANGESTATUS
    NS_DECL_NSIFACTORY

private:
    PRBool mStartingUp;
    PRBool mProfileChangeVetoed;
    PRBool mProfileChangeFailed;

    nsString mCurrentProfileName;
    PRBool mCurrentProfileAvailable;

    PRBool mIsContentLocaleSpecified;
    nsCString mContentLocaleName;

    PRBool mShutdownProfileToreDownNetwork;
    nsresult Init();

public:
    nsProfile();
    virtual ~nsProfile();

public:
    //We register an factory for Mozilla profile service its CID and CONTRACTID are NS_PROFILE_CID and NS_PROFILE_CONTRACTID
    //After that, we does not need profile.dll/libprofile.so any more. Our profile service are different from the orginal one in
    //these areas:
    //1. We do not lock Mozilla Profile,
    //2. We do not change Mozilla Profile settings. We do not change default profile, do not create new profiles and
    //   do not write changes back to profile register
    //3. We support can load both Thunderbird profile and Mozilla profiles
    static nsresult RegisterProfileManager(nsIProfile* aProfileService);

};

extern nsresult ConvertStringToUnicode(nsCString& aCharset, const char* inString, nsAString& outString);
extern nsresult GetPlatformCharset(nsCString& aCharset);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
