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


