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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "MNSProfileManager.hxx"
#include "MNSProfile.hxx"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#include <MNSFolders.hxx>
#include <MNSINIParser.hxx>

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
            ::sal_Int32 ProfileManager::bootupProfile( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
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
            ::rtl::OUString ProfileManager::getCurrentProfile(  ) throw (::com::sun::star::uno::RuntimeException)
            {
                nsresult rv;
                nsCOMPtr<nsIProfile> theProfile(do_GetService(NS_PROFILE_CONTRACTID,&rv));
                     if (NS_FAILED(rv)) return ::rtl::OUString();
                nsXPIDLString currentProfileStr;
                //call GetCurrentProfile before call SetCurrentProfile will get the default profile
                rv = theProfile->GetCurrentProfile(getter_Copies(currentProfileStr));
                if (NS_FAILED(rv) || currentProfileStr.get() == nsnull)
                    return ::rtl::OUString();
                // PRUnichar != sal_Unicode in mingw
                return ::rtl::OUString(reinterpret_cast_mingw_only<const sal_Unicode *>(currentProfileStr.get()));
            }
            ::rtl::OUString ProfileManager::setCurrentProfile( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
            {
                ::rtl::OUString currentProfile = getCurrentProfile();
                //if profileName not given, then skip and return curernt profile
                if (!profileName.getLength() && m_CurrentProduct == product)
                    return currentProfile;

                if (m_CurrentProduct == product && profileName.equals(currentProfile))
                    return currentProfile;
                //set current product
                m_CurrentProduct = product;

                //get profile mozilla profile service
                nsresult rv;
                nsCOMPtr<nsIProfile> theProfile(do_GetService(NS_PROFILE_CONTRACTID,&rv));
                if (NS_FAILED(rv)) return ::rtl::OUString();

                // PRUnichar != sal_Unicode in mingw
                const PRUnichar* pUsedProfile = reinterpret_cast_mingw_only<const PRUnichar *>(profileName.getStr());
                //set current profile
                rv = theProfile->SetCurrentProfile( pUsedProfile );
                if (NS_FAILED(rv)) return ::rtl::OUString();
                return currentProfile;
            }


    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
