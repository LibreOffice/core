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

#ifndef __MNSProfileManager_h___
#define __MNSProfileManager_h___

#include "mozilla_profilemanager.h"

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
class nsProfile;
namespace connectivity
{
    namespace mozab
    {
        class ProfileManager
        {
        public:

            virtual ~ProfileManager();
            ProfileManager();

            ::sal_Int32 SAL_CALL bootupProfile( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException) ;
            ::sal_Int32 SAL_CALL shutdownProfile(  ) throw (::com::sun::star::uno::RuntimeException) ;
            ::com::sun::star::mozilla::MozillaProductType SAL_CALL getCurrentProduct(  ) throw (::com::sun::star::uno::RuntimeException) ;
            OUString SAL_CALL getCurrentProfile(  ) throw (::com::sun::star::uno::RuntimeException) ;
            OUString SAL_CALL setCurrentProfile( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException) ;

        protected:
            ::com::sun::star::mozilla::MozillaProductType m_CurrentProduct;
            nsProfile *aProfile;
        };

    }
}

#endif // __MNSProfileManager_h___

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
