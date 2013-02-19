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


#undef SC_DLLIMPLEMENTATION



#include "document.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include "tpstat.hrc"

#include "tpstat.hxx"


//========================================================================
// Dokumentinfo-Tabpage:
//========================================================================

SfxTabPage* ScDocStatPage::Create( Window *pParent, const SfxItemSet& rSet )
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
    aFtTables   .SetText( OUString::number( aDocStat.nTableCount ) );
    aFtCells    .SetText( OUString::number( aDocStat.nCellCount ) );
    aFtPages    .SetText( OUString::number( aDocStat.nPageCount ) );

    FreeResource();
}

//------------------------------------------------------------------------

ScDocStatPage::~ScDocStatPage()
{
}

//------------------------------------------------------------------------

sal_Bool ScDocStatPage::FillItemSet( SfxItemSet& /* rSet */ )
{
    return false;
}

//------------------------------------------------------------------------

void ScDocStatPage::Reset( const SfxItemSet& /* rSet */ )
{
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
