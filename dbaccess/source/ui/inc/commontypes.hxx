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



#ifndef _DBAUI_COMMON_TYPES_HXX_
#define _DBAUI_COMMON_TYPES_HXX_

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef UNOTOOLS_INC_SHAREDUNOCOMPONENT_HXX
#include <unotools/sharedunocomponent.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace sdbc {
        class XConnection;
    }
} } }

//.........................................................................
namespace dbaui
{
//.........................................................................

    DECLARE_STL_STDKEY_SET( ::rtl::OUString, StringBag );
    DECLARE_STL_VECTOR( sal_Int8, ByteVector );
    DECLARE_STL_VECTOR( ::rtl::OUString, StringArray );

    typedef ::utl::SharedUNOComponent< ::com::sun::star::sdbc::XConnection > SharedConnection;

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_COMMON_TYPES_HXX_

