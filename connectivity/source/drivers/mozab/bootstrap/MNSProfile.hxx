/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    //After that, we does not need profile.dll/libprofile.so any more. Our profile service are diffrent from the orginal one in
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
