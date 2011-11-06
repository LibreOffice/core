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



#ifndef _XMLOFF_I18NMAP_HXX
#define _XMLOFF_I18NMAP_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <tools/solar.h>


namespace rtl
{
    class OUString;
}

class SvI18NMap_Impl;
class SvI18NMapEntry_Impl;

class XMLOFF_DLLPUBLIC SvI18NMap
{
    SvI18NMap_Impl      *pImpl;

    SAL_DLLPRIVATE SvI18NMapEntry_Impl *_Find( sal_uInt16 nKind,
                                const ::rtl::OUString& rName ) const;

public:

    SvI18NMap();
    ~SvI18NMap();

    // Add a name mapping
    void Add( sal_uInt16 nKind, const ::rtl::OUString& rName,
              const ::rtl::OUString& rNewName );

    // Return a mapped name. If the name could not be found, return the
    // original name.
    const ::rtl::OUString& Get( sal_uInt16 nKind,
                                const ::rtl::OUString& rName ) const;
};


#endif  //  _XMLOFF_I18NMAP_HXX

