/*************************************************************************
 *
 *  $RCSfile: MNSProfile.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 12:27:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "nsIProfile.h"
#include "nsIProfileInternal.h"
#include "nsIProfileStartupListener.h"
#include "nsIProfileChangeStatus.h"
#include "nsCOMPtr.h"
#include "nsISupports.h"
#include "nsIRegistry.h"
#include "nsString.h"

// Interfaces Needed
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIDirectoryService.h"
#include "nsIFactory.h"

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

