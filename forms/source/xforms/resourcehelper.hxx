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


#ifndef _RESOURCEHELPER_HXX
#define _RESOURCEHELPER_HXX

// include resource IDs
#include <frm_resource.hrc>

#include <sal/types.h>

namespace rtl { class OUString; }

namespace xforms
{
    /// get a resource string for the current language
    rtl::OUString getResource( sal_uInt16 );

    // overloaded: get a resource string, and substitute parameters
    rtl::OUString getResource( sal_uInt16, const rtl::OUString& );
    rtl::OUString getResource( sal_uInt16, const rtl::OUString&,
                                           const rtl::OUString& );
    rtl::OUString getResource( sal_uInt16, const rtl::OUString&,
                                           const rtl::OUString&,
                                           const rtl::OUString& );

} // namespace

#endif
