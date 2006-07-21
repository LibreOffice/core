/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tpstat.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:46:19 $
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
        aFtTablesLbl    ( this, ScResId( FT_TABLES_LBL ) ),
        aFtTables       ( this, ScResId( FT_TABLES ) ),
        aFtCellsLbl     ( this, ScResId( FT_CELLS_LBL ) ),
        aFtCells        ( this, ScResId( FT_CELLS ) ),
        aFtPagesLbl     ( this, ScResId( FT_PAGES_LBL ) ),
        aFtPages        ( this, ScResId( FT_PAGES ) ),
        aFlInfo         ( this, ScResId( FL_INFO ) )
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

BOOL __EXPORT ScDocStatPage::FillItemSet( SfxItemSet& rSet )
{
    return FALSE;
}

//------------------------------------------------------------------------

void __EXPORT ScDocStatPage::Reset( const SfxItemSet& rSet )
{
}




