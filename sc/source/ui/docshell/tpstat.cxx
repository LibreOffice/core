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
    ScDocShell* pDocSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );
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




