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


#ifndef CONNECTIVITY_STDTYPEDEFS_HXX
#define CONNECTIVITY_STDTYPEDEFS_HXX

#include "sal/config.h"

#include <map>
#include <vector>
#include "sal/types.h"

namespace rtl { class OUString; }

namespace connectivity
{
    typedef ::std::vector< ::rtl::OUString>         TStringVector;
    typedef ::std::vector< sal_Int32>               TIntVector;
    typedef ::std::map<sal_Int32,sal_Int32>         TInt2IntMap;
    typedef ::std::map< ::rtl::OUString,sal_Int32>  TString2IntMap;
    typedef ::std::map< sal_Int32,::rtl::OUString>  TInt2StringMap;
}

#endif // CONNECTIVITY_STDTYPEDEFS_HXX

