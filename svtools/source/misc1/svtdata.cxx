/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svtdata.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:17:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <map>

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _SVTOOLS_SVTDATA_HXX
#include <svtdata.hxx>
#endif

namespace unnamed_svtools_svtdata {}
using namespace unnamed_svtools_svtdata;
    // unnamed namespaces don't work well yet

//============================================================================
namespace unnamed_svtools_svtdata {

typedef std::map< rtl::OUString, SimpleResMgr * > SimpleResMgrMap;

}

//============================================================================
//
//  ImpSvtData
//
//============================================================================

ImpSvtData::~ImpSvtData()
{
    delete pResMgr;
    for (SimpleResMgrMap::iterator t
             = static_cast< SimpleResMgrMap * >(m_pThreadsafeRMs)->begin();
         t != static_cast< SimpleResMgrMap * >(m_pThreadsafeRMs)->end(); ++t)
        delete t->second;
    delete static_cast< SimpleResMgrMap * >(m_pThreadsafeRMs);
}

//============================================================================
ResMgr * ImpSvtData::GetResMgr(const ::com::sun::star::lang::Locale aLocale)
{
    if (!pResMgr)
    {
        pResMgr = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(svt), aLocale );
    }
    return pResMgr;
}

//============================================================================
SimpleResMgr* ImpSvtData::GetSimpleRM(const ::com::sun::star::lang::Locale& rLocale)
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
        rResMgr = new SimpleResMgr(CREATEVERSIONRESMGR_NAME(svs), rLocale );
    }
    return rResMgr;
}

ResMgr * ImpSvtData::GetPatchResMgr(const ::com::sun::star::lang::Locale& aLocale)
{
    if (!pPatchResMgr)
    {
        pPatchResMgr = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(svp), aLocale);
    }
    return pPatchResMgr;
}

//============================================================================
// static
ImpSvtData & ImpSvtData::GetSvtData()
{
    void ** pAppData = GetAppData(SHL_SVT);
    if (!*pAppData)
        *pAppData= new ImpSvtData;
    return *static_cast<ImpSvtData *>(*pAppData);
}

