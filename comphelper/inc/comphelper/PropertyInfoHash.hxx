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



#ifndef _COMPHELPER_PROPERTYINFOHASH_HXX_
#define _COMPHELPER_PROPERTYINFOHASH_HXX_

#include <rtl/ustring.hxx>
#include <comphelper/TypeGeneration.hxx>
#include <hash_map>
namespace comphelper
{
    struct PropertyInfo
    {
        const sal_Char* mpName;
        sal_uInt16 mnNameLen;
        sal_Int32 mnHandle;
        CppuTypes meCppuType;
        sal_Int16 mnAttributes;
        sal_uInt8 mnMemberId;
    };
    struct PropertyData
    {
        sal_uInt8 mnMapId;
        PropertyInfo *mpInfo;
        PropertyData ( sal_uInt8 nMapId, PropertyInfo *pInfo )
        : mnMapId ( nMapId )
        , mpInfo ( pInfo ) {}
    };
    struct eqFunc
    {
        sal_Bool operator()( const rtl::OUString &r1,
                             const rtl::OUString &r2) const
        {
            return r1 == r2;
        }
    };
}

typedef std::hash_map < ::rtl::OUString,
                        ::comphelper::PropertyInfo*,
                        ::rtl::OUStringHash,
                        ::comphelper::eqFunc > PropertyInfoHash;
typedef std::hash_map < ::rtl::OUString,
                        ::comphelper::PropertyData*,
                        ::rtl::OUStringHash,
                        ::comphelper::eqFunc > PropertyDataHash;
#endif
