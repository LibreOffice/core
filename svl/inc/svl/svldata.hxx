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



#ifndef _SVL_SVLDATA_HXX
#define _SVL_SVLDATA_HXX

#include <tools/simplerm.hxx>

class SfxItemPool;

//============================================================================
class ImpSvlData
{
public:
    const SfxItemPool * pStoringPool;
    void*           m_pThreadsafeRMs;
        // one SimpleResMgr for each language for which a resource was requested
        // (When using the 'non-simple' resmgr, the first request for any language wins, any
        // further request for any other language supply the resmgr of the first call.
        // For the simple resmgr we have a mgr for each language ever requested).

private:
    ImpSvlData():
        pStoringPool(0), m_pThreadsafeRMs(NULL)
    {}

    ~ImpSvlData();

public:
    SimpleResMgr * GetSimpleRM(const ::com::sun::star::lang::Locale& rLocale);
    static ImpSvlData & GetSvlData();
};

//============================================================================
class SvtSimpleResId
{
    String  m_sValue;

public:
    SvtSimpleResId(sal_uInt16 nId, const ::com::sun::star::lang::Locale aLocale) : m_sValue(ImpSvlData::GetSvlData().GetSimpleRM(aLocale)->ReadString(nId)) { };

    operator String () const { return m_sValue; }
};



#endif //  _SVL_SVLDATA_HXX

