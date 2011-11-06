/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __MNSProfileDiscover_h___
#define __MNSProfileDiscover_h___

#ifndef MINIMAL_PROFILEDISCOVER
#include "mozilla_profile_discover.h"
#endif

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/conditn.hxx>
#ifndef _COM_SUN_STAR_MOZILLA_MOZILLPRODUCTTYPE_HPP_
#include <com/sun/star/mozilla/MozillaProductType.hpp>
#endif
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <map>

#ifdef XP_OS2
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#include <os2.h>
#endif

using namespace com::sun::star::mozilla;
namespace connectivity
{
    namespace mozab
    {
        class ProfileStruct;
    }
}
typedef ::std::map < ::rtl::OUString, ::connectivity::mozab::ProfileStruct* > ProfileList;
namespace connectivity
{
    namespace mozab
    {
        class ProfileStruct
        {
        public:
            ProfileStruct(MozillaProductType aProduct,::rtl::OUString aProfileName,
#ifdef MINIMAL_PROFILEDISCOVER
                          const ::rtl::OUString &aProfilePath
#else
                          nsILocalFile * aProfilePath
#endif
                            );
            MozillaProductType getProductType() { return product;}
            ::rtl::OUString getProfileName(){ return profileName;}
            ::rtl::OUString getProfilePath() ;
#ifndef MINIMAL_PROFILEDISCOVER
            nsILocalFile    *getProfileLocal(){ return profilePath;}
#endif
        protected:
            MozillaProductType product;
            ::rtl::OUString profileName;
#ifdef MINIMAL_PROFILEDISCOVER
            ::rtl::OUString profilePath;
#else
            nsCOMPtr<nsILocalFile> profilePath;
#endif
        };

        class ProductStruct
        {
        public:
            void setCurrentProfile(::rtl::OUString aProfileName){mCurrentProfileName = aProfileName;}

            ::rtl::OUString mCurrentProfileName;

            ProfileList mProfileList;
        };

        //Used to query profiles information
        class ProfileAccess
        {
        public:

            virtual ~ProfileAccess();
            ProfileAccess();
            ::rtl::OUString getProfilePath( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);
            ::sal_Int32 getProfileCount( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException);
            ::sal_Int32 getProfileList( ::com::sun::star::mozilla::MozillaProductType product, ::com::sun::star::uno::Sequence< ::rtl::OUString >& list ) throw (::com::sun::star::uno::RuntimeException);
            ::rtl::OUString getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException);
            ::sal_Bool SAL_CALL isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);
            ::sal_Bool SAL_CALL getProfileExists( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);
        protected:
            ProductStruct m_ProductProfileList[4];
            sal_Int32 LoadProductsInfo();
            sal_Int32 LoadXPToolkitProfiles(MozillaProductType product);
#ifndef MINIMAL_PROFILEDISCOVER
            //used by isProfileLocked
            nsresult isExistFileOrSymlink(nsILocalFile* aFile,PRBool *bExist);
            nsresult isLockExist(nsILocalFile* aFile);
#endif
        };

    }
}

#endif // __MNSProfileDiscover_h___

