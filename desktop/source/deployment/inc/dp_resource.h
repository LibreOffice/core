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



#if ! defined INCLUDED_DP_RESOURCE_H
#define INCLUDED_DP_RESOURCE_H

#include "tools/resmgr.hxx"
#include "tools/string.hxx"
#include "tools/resid.hxx"
#include "com/sun/star/lang/Locale.hpp"
#include "dp_misc.h"
#include <memory>
#include "dp_misc_api.hxx"

namespace dp_misc {

//==============================================================================
ResId getResId( sal_uInt16 id );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC String getResourceString( sal_uInt16 id );

template <typename Unique, sal_uInt16 id>
struct StaticResourceString :
        public ::rtl::StaticWithInit<const ::rtl::OUString, Unique> {
    const ::rtl::OUString operator () () { return getResourceString(id); }
};

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::com::sun::star::lang::Locale toLocale( ::rtl::OUString const & slang );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::com::sun::star::lang::Locale getOfficeLocale();

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::rtl::OUString getOfficeLocaleString();

}

#endif
