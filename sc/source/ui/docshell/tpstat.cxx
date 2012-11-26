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

#undef SC_DLLIMPLEMENTATION



#include "document.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include "tpstat.hrc"

#include "tpstat.hxx"


//========================================================================
// Dokumentinfo-Tabpage:
//========================================================================

SfxTabPage* __EXPORT ScDocStatPage::Create( Window *pParent, const SfxItemSet& rSet )
{
    return new ScDocStatPage( pParent, rSet );
}

//------------------------------------------------------------------------

ScDocStatPage::ScDocStatPage( Window *pParent, const SfxItemSet& rSet )
    :   SfxTabPage( pParent, ScResId(RID_SCPAGE_STAT), rSet ),
        aFlInfo         ( this, ScResId( FL_INFO ) ),
        aFtTablesLbl    ( this, ScResId( FT_TABLES_LBL ) ),
        aFtTables       ( this, ScResId( FT_TABLES ) ),
        aFtCellsLbl     ( this, ScResId( FT_CELLS_LBL ) ),
        aFtCells        ( this, ScResId( FT_CELLS ) ),
        aFtPagesLbl     ( this, ScResId( FT_PAGES_LBL ) ),
        aFtPages        ( this, ScResId( FT_PAGES ) )
{
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( SfxObjectShell::Current() );
    ScDocStat   aDocStat;

    if ( pDocSh )
        pDocSh->GetDocStat( aDocStat );

    String aInfo = aFlInfo.GetText();
    aInfo += aDocStat.aDocName;
    aFlInfo     .SetText( aInfo );
    aFtTables   .SetText( String::CreateFromInt32( aDocStat.nTableCount ) );
    aFtCells    .SetText( String::CreateFromInt32( aDocStat.nCellCount ) );
    aFtPages    .SetText( String::CreateFromInt32( aDocStat.nPageCount ) );

    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScDocStatPage::~ScDocStatPage()
{
}

//------------------------------------------------------------------------

sal_Bool __EXPORT ScDocStatPage::FillItemSet( SfxItemSet& /* rSet */ )
{
    return sal_False;
}

//------------------------------------------------------------------------

void __EXPORT ScDocStatPage::Reset( const SfxItemSet& /* rSet */ )
{
}




