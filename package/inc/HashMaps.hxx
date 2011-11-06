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


#ifndef _HASHMAPS_HXX
#define _HASHMAPS_HXX

#include <ZipEntry.hxx>
#include <vos/ref.hxx>
#include <hash_map>

struct eqFunc
{
    sal_Bool operator()( const rtl::OUString &r1,
                         const rtl::OUString &r2) const
    {
        return r1 == r2;
    }
};

class ZipPackageFolder;
namespace com { namespace sun { namespace star { namespace packages {
class ContentInfo;
} } } }

typedef std::hash_map < rtl::OUString,
                        ZipPackageFolder *,
                        ::rtl::OUStringHash,
                        eqFunc > FolderHash;

typedef std::hash_map < rtl::OUString,
                        vos::ORef < com::sun::star::packages::ContentInfo >,
                        ::rtl::OUStringHash,
                        eqFunc > ContentHash;

typedef std::hash_map < rtl::OUString,
                        ZipEntry,
                        rtl::OUStringHash,
                        eqFunc > EntryHash;

#endif
