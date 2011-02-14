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
#include "precompiled_svtools.hxx"

#include <map>
#include <tools/resmgr.hxx>
#include <tools/shl.hxx>
#include <vos/process.hxx>
#include <svtools/svtdata.hxx>
#include <vcl/svapp.hxx>

//============================================================================
//
//  ImpSvtData
//
//============================================================================

ImpSvtData::~ImpSvtData()
{
    delete pResMgr;
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

ResMgr * ImpSvtData::GetResMgr()
{
    return GetResMgr(Application::GetSettings().GetUILocale());
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

SvtResId::SvtResId(sal_uInt16 nId, const ::com::sun::star::lang::Locale aLocale):
        ResId(nId, *ImpSvtData::GetSvtData().GetResMgr(aLocale)) {}

SvtResId::SvtResId(sal_uInt16 nId): ResId(nId, *ImpSvtData::GetSvtData().GetResMgr()) {}

