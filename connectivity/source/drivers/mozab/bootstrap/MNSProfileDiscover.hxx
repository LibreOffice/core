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

#ifndef __MNSProfileDiscover_h___
#define __MNSProfileDiscover_h___

#ifndef MINIMAL_PROFILEDISCOVER
#include "mozilla_profile_discover.h"
#endif

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/conditn.hxx>
#include <com/sun/star/mozilla/MozillaProductType.hpp>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
