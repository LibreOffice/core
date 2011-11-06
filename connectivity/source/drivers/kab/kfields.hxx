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



#ifndef _CONNECTIVITY_KAB_FIELDS_HXX_
#define _CONNECTIVITY_KAB_FIELDS_HXX_

#include <shell/kde_headers.h>
#include <connectivity/dbexception.hxx>
#include <rtl/ustring.hxx>

#define KAB_FIELD_REVISION  0
#define KAB_DATA_FIELDS     1

namespace connectivity
{
    namespace kab
    {
        QString valueOfKabField(const ::KABC::Addressee &aAddressee, sal_Int32 nFieldNumber);
        sal_uInt32 findKabField(const ::rtl::OUString& columnName) throw(::com::sun::star::sdbc::SQLException);
    }
}

#endif
