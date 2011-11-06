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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <map>
#include <tools/resmgr.hxx>
#include <tools/shl.hxx>
#include <vos/process.hxx>
#include <svl/svldata.hxx>

namespace unnamed_svl_svldata {}
using namespace unnamed_svl_svldata;
    // unnamed namespaces don't work well yet

//============================================================================
namespace unnamed_svl_svldata {

typedef std::map< rtl::OUString, SimpleResMgr * > SimpleResMgrMap;

}

//============================================================================
//
//  ImpSvlData
//
//============================================================================

static ImpSvlData* pSvlData = 0;

ImpSvlData::~ImpSvlData()
{
    for (SimpleResMgrMap::iterator t
             = static_cast< SimpleResMgrMap * >(m_pThreadsafeRMs)->begin();
         t != static_cast< SimpleResMgrMap * >(m_pThreadsafeRMs)->end(); ++t)
        delete t->second;
    delete static_cast< SimpleResMgrMap * >(m_pThreadsafeRMs);
}

//============================================================================
SimpleResMgr* ImpSvlData::GetSimpleRM(const ::com::sun::star::lang::Locale& rLocale)
{
    if (!m_pThreadsafeRMs)
        m_pThreadsafeRMs = new SimpleResMgrMap;
    rtl::OUString aISOcode = rLocale.Language;
    aISOcode += rtl::OStringToOUString("-", RTL_TEXTENCODING_UTF8);
    aISOcode += rLocale.Country;

    SimpleResMgr *& rResMgr
        = (*static_cast< SimpleResMgrMap * >(m_pThreadsafeRMs))[aISOcode];
    if (!rResMgr)
    {
        rResMgr = new SimpleResMgr(CREATEVERSIONRESMGR_NAME(svl), rLocale );
    }
    return rResMgr;
}

//============================================================================
// static
ImpSvlData & ImpSvlData::GetSvlData()
{
    if (!pSvlData)
        pSvlData= new ImpSvlData;
    return *pSvlData;
}

