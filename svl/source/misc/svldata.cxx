/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

