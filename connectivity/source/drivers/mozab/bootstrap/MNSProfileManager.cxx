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


#include "MNSProfileManager.hxx"
#include "MNSProfile.hxx"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#include "MNSFolders.hxx"
#include "MNSINIParser.hxx"

static ::sal_Int32 nInitCounter=0;
namespace connectivity
{
    namespace mozab
    {

        ProfileManager::~ProfileManager()
        {
        }
        ProfileManager::ProfileManager()
            :m_CurrentProduct(MozillaProductType_Mozilla)
            ,aProfile(NULL)
        {
        }
            ::sal_Int32 ProfileManager::bootupProfile( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
            {
                if (!aProfile)
                {
                    aProfile=new nsProfile();
                    aProfile->StartupWithArgs(NULL,sal_False);
                }

                setCurrentProfile(product,profileName);
                return ++nInitCounter;
            }
            ::sal_Int32 ProfileManager::shutdownProfile(  ) throw (::com::sun::star::uno::RuntimeException)
            {
                return --nInitCounter;
            }
            ::com::sun::star::mozilla::MozillaProductType ProfileManager::getCurrentProduct(  ) throw (::com::sun::star::uno::RuntimeException)
            {
                return m_CurrentProduct;
            }
            OUString ProfileManager::getCurrentProfile(  ) throw (::com::sun::star::uno::RuntimeException)
            {
                nsresult rv;
                nsCOMPtr<nsIProfile> theProfile(do_GetService(NS_PROFILE_CONTRACTID,&rv));
                     if (NS_FAILED(rv)) return OUString();
                nsXPIDLString currentProfileStr;
                //call GetCurrentProfile before call SetCurrentProfile will get the default profile
                rv = theProfile->GetCurrentProfile(getter_Copies(currentProfileStr));
                if (NS_FAILED(rv) || currentProfileStr.get() == nsnull)
                    return OUString();
                // PRUnichar != sal_Unicode in mingw
                return OUString(reinterpret_cast_mingw_only<const sal_Unicode *>(currentProfileStr.get()));
            }
            OUString ProfileManager::setCurrentProfile( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
            {
                OUString currentProfile = getCurrentProfile();
                //if profileName not given, then skip and return current profile
                if (profileName.isEmpty() && m_CurrentProduct == product)
                    return currentProfile;

                if (m_CurrentProduct == product && profileName.equals(currentProfile))
                    return currentProfile;
                //set current product
                m_CurrentProduct = product;

                //get profile mozilla profile service
                nsresult rv;
                nsCOMPtr<nsIProfile> theProfile(do_GetService(NS_PROFILE_CONTRACTID,&rv));
                if (NS_FAILED(rv)) return OUString();

                // PRUnichar != sal_Unicode in mingw
                const PRUnichar* pUsedProfile = reinterpret_cast_mingw_only<const PRUnichar *>(profileName.getStr());
                //set current profile
                rv = theProfile->SetCurrentProfile( pUsedProfile );
                if (NS_FAILED(rv)) return OUString();
                return currentProfile;
            }


    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
