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

#pragma once

#include <sal/types.h>
#include <com/sun/star/mozilla/MozillaProductType.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <map>

using namespace com::sun::star::mozilla;
namespace connectivity::mozab { class ProfileStruct; }

typedef std::map<OUString, ::connectivity::mozab::ProfileStruct> ProfileList;
namespace connectivity::mozab
    {
        class ProfileStruct
        {
        public:
            ProfileStruct();
            ProfileStruct(const OUString& aProfileName, const OUString &aProfilePath);
            const OUString& getProfileName() const { return profileName;}
            const OUString& getProfilePath() const;
        private:
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
            /// @throws css::uno::RuntimeException
            OUString getProfilePath( css::mozilla::MozillaProductType product, const OUString& profileName );
            /// @throws css::uno::RuntimeException
            ::sal_Int32 getProfileCount( css::mozilla::MozillaProductType product );
            /// @throws css::uno::RuntimeException
            ::sal_Int32 getProfileList( css::mozilla::MozillaProductType product, css::uno::Sequence< OUString >& list );
            /// @throws css::uno::RuntimeException
            OUString getDefaultProfile( css::mozilla::MozillaProductType product );
            /// @throws css::uno::RuntimeException
            bool getProfileExists( css::mozilla::MozillaProductType product, const OUString& profileName );
        private:
            ProductStruct m_ProductProfileList[4];
            void      LoadProductsInfo();
            void      LoadXPToolkitProfiles(MozillaProductType product);
        };

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
