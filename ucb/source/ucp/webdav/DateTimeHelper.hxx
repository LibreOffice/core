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



#ifndef _WEBDAV_DATETIME_HELPER_HXX
#define _WEBDAV_DATETIME_HELPER_HXX

#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace util {
    struct DateTime;
} } } }

namespace rtl {
    class OUString;
}

namespace webdav_ucp
{

class DateTimeHelper
{
private:
    static sal_Int32 convertMonthToInt (const ::rtl::OUString& );

    static bool ISO8601_To_DateTime (const ::rtl::OUString&,
        ::com::sun::star::util::DateTime& );

    static bool RFC2068_To_DateTime (const ::rtl::OUString&,
        ::com::sun::star::util::DateTime& );

public:
    static bool convert (const ::rtl::OUString&,
        ::com::sun::star::util::DateTime& );
};

} // namespace webdav_ucp

#endif

