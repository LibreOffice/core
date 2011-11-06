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



#if ! defined INCLUDED_DP_PLATFORM_HXX
#define INCLUDED_DP_PLATFORM_HXX


#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_API_HXX
#include "dp_misc_api.hxx"
#endif

#include "com/sun/star/uno/Sequence.hxx"
#include "rtl/ustring.hxx"

namespace dp_misc
{


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString const & getPlatformString();

DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
    bool platform_fits( ::rtl::OUString const & platform_string );

/** determines if the current platform corresponds to one of the platform strings.

*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
bool hasValidPlatform( ::com::sun::star::uno::Sequence< ::rtl::OUString > const & platformStrings);

}

#endif
