/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svl/zforlist.hxx>
#include <editeng/editeng.hxx>

#include "poolhelp.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"

ScPoolHelper::ScPoolHelper( ScDocument* pSourceDoc )
:pFormTable(nullptr)
,pEditPool(nullptr)
,pEnginePool(nullptr)
,m_pSourceDoc(pSourceDoc)
{
    OSL_ENSURE( pSourceDoc, "ScPoolHelper: no document" );
    pDocPool = new ScDocumentPool;
    pDocPool->FreezeIdRanges();

    mxStylePool = new ScStyleSheetPool( *pDocPool, pSourceDoc );
}

ScPoolHelper::~ScPoolHelper()
{
    SfxItemPool::Free(pEnginePool);
    SfxItemPool::Free(pEditPool);
    delete pFormTable;
    mxStylePool.clear();
    SfxItemPool::Free(pDocPool);
}
SfxItemPool*        ScPoolHelper::GetEditPool() const
{
    if ( !pEditPool )
    {
        pEditPool = EditEngine::CreatePool();
        pEditPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
        pEditPool->FreezeIdRanges();
        pEditPool->SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );   // used in ScGlobal::EETextObjEqual
    }
    return pEditPool;
}
SfxItemPool*        ScPoolHelper::GetEnginePool() const
{
    if ( !pEnginePool )
    {
        pEnginePool = EditEngine::CreatePool();
        pEnginePool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
        pEnginePool->FreezeIdRanges();
    } // ifg ( pEnginePool )
    return pEnginePool;
}
SvNumberFormatter*  ScPoolHelper::GetFormTable() const
{
    if (!pFormTable)
        pFormTable = CreateNumberFormatter();
    return pFormTable;
}

void ScPoolHelper::UseDocOptions() const
{
    if (pFormTable)
    {
        sal_uInt16 d,m,y;
        aOpt.GetDate( d,m,y );
        pFormTable->ChangeNullDate( d,m,y );
        pFormTable->ChangeStandardPrec( (sal_uInt16)aOpt.GetStdPrecision() );
        pFormTable->SetYear2000( aOpt.GetYear2000() );
    }
}

void ScPoolHelper::SetFormTableOpt(const ScDocOptions& rOpt)
{
    aOpt = rOpt;
    UseDocOptions();        // #i105512# if the number formatter exists, update its settings
}

SvNumberFormatter* ScPoolHelper::CreateNumberFormatter() const
{
    SvNumberFormatter* p = nullptr;
    {
        osl::MutexGuard aGuard(&maMtxCreateNumFormatter);
        p = new SvNumberFormatter(comphelper::getProcessComponentContext(), ScGlobal::eLnge);
    }
    p->SetColorLink( LINK(m_pSourceDoc, ScDocument, GetUserDefinedColor) );
    p->SetEvalDateFormat(NF_EVALDATEFORMAT_INTL_FORMAT);

    sal_uInt16 d,m,y;
    aOpt.GetDate(d, m, y);
    p->ChangeNullDate(d, m, y);
    p->ChangeStandardPrec(aOpt.GetStdPrecision());
    p->SetYear2000(aOpt.GetYear2000());
    return p;
}

void ScPoolHelper::SourceDocumentGone()
{
    //  reset all pointers to the source document
    mxStylePool->SetDocument( nullptr );
    if ( pFormTable )
        pFormTable->SetColorLink( Link<sal_uInt16,Color*>() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
