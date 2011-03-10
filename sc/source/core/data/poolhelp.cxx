/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svl/zforlist.hxx>
#include <editeng/editeng.hxx>

#include "poolhelp.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"

// -----------------------------------------------------------------------

ScPoolHelper::ScPoolHelper( ScDocument* pSourceDoc )
:pFormTable(NULL)
,pEditPool(NULL)
,pEnginePool(NULL)
,m_pSourceDoc(pSourceDoc)
{
    DBG_ASSERT( pSourceDoc, "ScPoolHelper: no document" );
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
    if ( !pFormTable )
    {
        pFormTable = new SvNumberFormatter( m_pSourceDoc->GetServiceManager(), ScGlobal::eLnge );
        pFormTable->SetColorLink( LINK( m_pSourceDoc, ScDocument, GetUserDefinedColor ) );
        pFormTable->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );

        UseDocOptions();        // null date, year2000, std precision
    }
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

void ScPoolHelper::SourceDocumentGone()
{
    //  reset all pointers to the source document
    mxStylePool->SetDocument( NULL );
    if ( pFormTable )
        pFormTable->SetColorLink( Link() );
}

// -----------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
