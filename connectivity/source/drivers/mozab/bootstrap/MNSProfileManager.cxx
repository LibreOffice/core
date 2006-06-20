/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSProfileManager.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:48:37 $
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

#include "MNSProfileManager.hxx"
#include "MNSProfile.hxx"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#ifndef _MNSFOLDERS_HXX_
#include <MNSFolders.hxx>
#endif
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
                return ::rtl::OUString(currentProfileStr.get());
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

                const PRUnichar* pUsedProfile = profileName.getStr();
                //set current profile
                rv = theProfile->SetCurrentProfile( pUsedProfile );
                if (NS_FAILED(rv)) return ::rtl::OUString();
                return currentProfile;
            }


    }
}
