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


#ifndef _LABELCFG_HXX
#define _LABELCFG_HXX

#include <unotools/configitem.hxx>
#include "swdllapi.h"

class SwLabRecs;
class SwLabRec;

class SW_DLLPUBLIC SwLabelConfig : public utl::ConfigItem
{
    com::sun::star::uno::Sequence<rtl::OUString> aNodeNames;

    SW_DLLPRIVATE com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
    SwLabelConfig();
    virtual ~SwLabelConfig();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );

    void    FillLabels(const rtl::OUString& rManufacturer, SwLabRecs& rLabArr);
    const com::sun::star::uno::Sequence<rtl::OUString>&
            GetManufacturers() const {return aNodeNames;}

    sal_Bool    HasLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType);
    void        SaveLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType,
                            const SwLabRec& rRec);
};

#endif

