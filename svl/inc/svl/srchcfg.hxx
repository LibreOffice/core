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


#ifndef _SVX_SRCHCFG_HXX
#define _SVX_SRCHCFG_HXX

#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/svldllapi.h>

//-----------------------------------------------------------------------------
struct SvxSearchConfig_Impl;
struct SvxSearchEngineData
{
    rtl::OUString   sEngineName;

    rtl::OUString   sAndPrefix;
    rtl::OUString   sAndSuffix;
    rtl::OUString   sAndSeparator;
    sal_Int32       nAndCaseMatch;

    rtl::OUString   sOrPrefix;
    rtl::OUString   sOrSuffix;
    rtl::OUString   sOrSeparator;
    sal_Int32       nOrCaseMatch;

    rtl::OUString   sExactPrefix;
    rtl::OUString   sExactSuffix;
    rtl::OUString   sExactSeparator;
    sal_Int32       nExactCaseMatch;

    SvxSearchEngineData() :
        nAndCaseMatch(0),
        nOrCaseMatch(0),
        nExactCaseMatch(0){}

    sal_Bool operator==(const SvxSearchEngineData& rData);
};
class SVL_DLLPUBLIC SvxSearchConfig : public utl::ConfigItem
{
    SvxSearchConfig_Impl* pImpl;

public:
    SvxSearchConfig(sal_Bool bEnableNotify = sal_True);
    virtual ~SvxSearchConfig();

    void            Load();
    virtual void    Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

    sal_uInt16                  Count();
    const SvxSearchEngineData&  GetData(sal_uInt16 nPos);
    const SvxSearchEngineData*  GetData(const rtl::OUString& rEngineName);
    void                        SetData(const SvxSearchEngineData& rData);
    void                        RemoveData(const rtl::OUString& rEngineName);
};

#endif

