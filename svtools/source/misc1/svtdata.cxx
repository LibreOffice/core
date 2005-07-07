/*************************************************************************
 *
 *  $RCSfile: svtdata.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:11:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

