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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MOZAB_BOOTSTRAP_MNSPROFILEDISCOVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MOZAB_BOOTSTRAP_MNSPROFILEDISCOVER_HXX

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/conditn.hxx>
#include <com/sun/star/mozilla/MozillaProductType.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <map>

using namespace com::sun::star::mozilla;
namespace connectivity
{
    namespace mozab
    {
        class ProfileStruct;
    }
}
typedef ::std::map < OUString, ::connectivity::mozab::ProfileStruct* > ProfileList;
namespace connectivity
{
    namespace mozab
    {
        class ProfileStruct
        {
        public:
            ProfileStruct(MozillaProductType aProduct, const OUString& aProfileName,
                          const OUString &aProfilePath);
            const OUString& getProfileName(){ return profileName;}
            const OUString& getProfilePath();
        protected:
            MozillaProductType product;
            OUString profileName;
            OUString profilePath;
        };

        class ProductStruct
        {
        public:
            OUString mCurrentProfileName;
            ProfileList mProfileList;
        };

        //Used to query profiles information
        class ProfileAccess final
        {
        public:
            ~ProfileAccess();
            ProfileAccess();
            OUString getProfilePath( css::mozilla::MozillaProductType product, const OUString& profileName ) throw (css::uno::RuntimeException);
            ::sal_Int32 getProfileCount( css::mozilla::MozillaProductType product ) throw (css::uno::RuntimeException);
            ::sal_Int32 getProfileList( css::mozilla::MozillaProductType product, css::uno::Sequence< OUString >& list ) throw (css::uno::RuntimeException);
            OUString getDefaultProfile( css::mozilla::MozillaProductType product ) throw (css::uno::RuntimeException);
            bool SAL_CALL isProfileLocked( css::mozilla::MozillaProductType product, const OUString& profileName ) throw (css::uno::RuntimeException);
            bool SAL_CALL getProfileExists( css::mozilla::MozillaProductType product, const OUString& profileName ) throw (css::uno::RuntimeException);
        private:
            ProductStruct m_ProductProfileList[4];
            void      LoadProductsInfo();
            void      LoadXPToolkitProfiles(MozillaProductType product);
        };

    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MOZAB_BOOTSTRAP_MNSPROFILEDISCOVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
