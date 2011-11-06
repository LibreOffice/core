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


#ifndef _DBCONFIG_HXX
#define _DBCONFIG_HXX

#include <unotools/configitem.hxx>
#include "swdllapi.h"

struct SwDBData;

class SW_DLLPUBLIC SwDBConfig : public utl::ConfigItem
{
    SW_DLLPRIVATE const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();

    SwDBData*       pAdrImpl;
    SwDBData*       pBibImpl;

public:
    SwDBConfig();
    virtual ~SwDBConfig();

    void                    Load();
    const SwDBData&         GetAddressSource();
    const SwDBData&         GetBibliographySource();
    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};

#endif

