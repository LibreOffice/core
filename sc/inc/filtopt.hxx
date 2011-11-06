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



#ifndef SC_FILTOPT_HXX
#define SC_FILTOPT_HXX

#include <unotools/configitem.hxx>
#include <tools/solar.h>
#include "scdllapi.h"

//==================================================================
// filter options
//==================================================================

class SC_DLLPUBLIC ScFilterOptions : public utl::ConfigItem
{
    sal_Bool        bWK3Flag;
    double      fExcelColScale;
    double      fExcelRowScale;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
            ScFilterOptions();

    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames );
    virtual void    Commit();

    sal_Bool        GetWK3Flag() const          { return bWK3Flag; }
    double      GetExcelColScale() const    { return fExcelColScale; }
    double      GetExcelRowScale() const    { return fExcelRowScale; }

    //  values are never modified by office
};


#endif

