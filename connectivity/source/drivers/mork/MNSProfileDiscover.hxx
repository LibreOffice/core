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

#ifndef __MNSProfileDiscover_h___
#define __MNSProfileDiscover_h___

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/conditn.hxx>
#include <com/sun/star/mozilla/MozillaProductType.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <map>

#include "dllapi.h"

using namespace com::sun::star::mozilla;
namespace connectivity
{
    namespace mork
    {
        class ProfileStruct;
    }
}
typedef ::std::map < ::rtl::OUString, ::connectivity::mork::ProfileStruct* > ProfileList;
namespace connectivity
{
    namespace mork
    {
        class ProfileStruct
        {
        public:
            ProfileStruct(MozillaProductType aProduct,::rtl::OUString aProfileName,
                          const ::rtl::OUString &aProfilePath
                            );
            MozillaProductType getProductType() { return product;}
            ::rtl::OUString getProfileName(){ return profileName;}
            ::rtl::OUString getProfilePath() ;
        protected:
            MozillaProductType product;
            ::rtl::OUString profileName;
            ::rtl::OUString profilePath;
        };

        class ProductStruct
        {
        public:
            void setCurrentProfile(::rtl::OUString aProfileName){mCurrentProfileName = aProfileName;}

            ::rtl::OUString mCurrentProfileName;

            ProfileList mProfileList;
        };

        //Used to query profiles information
        class LO_DLLPUBLIC_MORK ProfileAccess
        {
        public:

            virtual ~ProfileAccess();
            ProfileAccess();
            ::rtl::OUString getProfilePath( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);
            ::rtl::OUString getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException);
        protected:
            ProductStruct m_ProductProfileList[4];
            sal_Int32 LoadProductsInfo();
            sal_Int32 LoadXPToolkitProfiles(MozillaProductType product);
        };

    }
}

#endif // __MNSProfileDiscover_h___

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
