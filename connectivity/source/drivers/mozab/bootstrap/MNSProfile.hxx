/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSProfile.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:47:46 $
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

