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



#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_VERSION_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_VERSION_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Reference.hxx"
#include "dp_misc_api.hxx"

namespace com { namespace sun { namespace star { namespace deployment {
    class XPackage;
} } } }
namespace rtl { class OUString; }

namespace dp_misc {

enum Order { LESS, EQUAL, GREATER };

DESKTOP_DEPLOYMENTMISC_DLLPUBLIC Order compareVersions(
    ::rtl::OUString const & version1, ::rtl::OUString const & version2);
}

#endif
