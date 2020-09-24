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

#include <comphelper/processfactory.hxx>
#include <svl/zforlist.hxx>
#include <editeng/editeng.hxx>
#include <osl/diagnose.h>

#include <poolhelp.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include <stlpool.hxx>

ScPoolHelper::ScPoolHelper( ScDocument& rSourceDoc )
    : pEditPool(nullptr)
    , pEnginePool(nullptr)
    , m_rSourceDoc(rSourceDoc)
{
    pDocPool = new ScDocumentPool;
    pDocPool->FreezeIdRanges();

    mxStylePool = new ScStyleSheetPool( *pDocPool, &rSourceDoc );
}

ScPoolHelper::~ScPoolHelper()
{
    SfxItemPool::Free(pEnginePool);
    SfxItemPool::Free(pEditPool);
    pFormTable.reset();
    mxStylePool.clear();
    SfxItemPool::Free(pDocPool);
}
SfxItemPool*        ScPoolHelper::GetEditPool() const
{
    if ( !pEditPool )
    {
        pEditPool = EditEngine::CreatePool();
        pEditPool->SetDefaultMetric( MapUnit::Map100thMM );
        pEditPool->FreezeIdRanges();
    }
    return pEditPool;
}
SfxItemPool*        ScPoolHelper::GetEnginePool() const
{
    if ( !pEnginePool )
    {
        pEnginePool = EditEngine::CreatePool();
        pEnginePool->SetDefaultMetric( MapUnit::Map100thMM );
        pEnginePool->FreezeIdRanges();
    } // ifg ( pEnginePool )
    return pEnginePool;
}
SvNumberFormatter*  ScPoolHelper::GetFormTable() const
{
    if (!pFormTable)
        pFormTable = CreateNumberFormatter();
    return pFormTable.get();
}

void ScPoolHelper::SetFormTableOpt(const ScDocOptions& rOpt)
{
    aOpt = rOpt;
    // #i105512# if the number formatter exists, update its settings
    if (pFormTable)
    {
        sal_uInt16 d,m;
        sal_Int16 y;
        aOpt.GetDate( d,m,y );
        pFormTable->ChangeNullDate( d,m,y );
        pFormTable->ChangeStandardPrec( aOpt.GetStdPrecision() );
        pFormTable->SetYear2000( aOpt.GetYear2000() );
    }
}

std::unique_ptr<SvNumberFormatter> ScPoolHelper::CreateNumberFormatter() const
{
    std::unique_ptr<SvNumberFormatter> p;
    {
        osl::MutexGuard aGuard(&maMtxCreateNumFormatter);
        p.reset(new SvNumberFormatter(comphelper::getProcessComponentContext(), LANGUAGE_SYSTEM));
    }
    p->SetColorLink( LINK(&m_rSourceDoc, ScDocument, GetUserDefinedColor) );
    p->SetEvalDateFormat(NF_EVALDATEFORMAT_INTL_FORMAT);

    sal_uInt16 d,m;
    sal_Int16 y;
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
