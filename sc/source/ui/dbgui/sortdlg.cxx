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


#include <vcl/msgbox.hxx>
#include "tpsort.hxx"
#include "sortdlg.hxx"
#include "scresid.hxx"
#include "sortdlg.hrc"

ScSortDlg::ScSortDlg( Window*           pParent,
                      const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( RID_SCDLG_SORT ),
                      pArgSet ),
        bIsHeaders  ( false ),
        bIsByRows   ( false )
{
    AddTabPage( TP_FIELDS,  ScTabPageSortFields::Create,  0 );
    AddTabPage( TP_OPTIONS, ScTabPageSortOptions::Create, 0 );
    FreeResource();
}

ScSortDlg::~ScSortDlg()
{
}

//==================================================================
ScSortWarningDlg::ScSortWarningDlg( Window* pParent,
                                   const String& rExtendText,
                                   const String& rCurrentText ):
    ModalDialog     ( pParent, "SortWarning", "modules/scalc/ui/sortwarning.ui" )
{
    get(aFtText, "sorttext" );
    get( aBtnExtSort, "extend" );
    get( aBtnCurSort, "current" );

    String sTextName = aFtText->GetText();
    sTextName.SearchAndReplaceAscii("%1", rExtendText);
    sTextName.SearchAndReplaceAscii("%2", rCurrentText);
    aFtText->SetText( sTextName );

    aBtnExtSort->SetClickHdl( LINK( this, ScSortWarningDlg, BtnHdl ) );
    aBtnCurSort->SetClickHdl( LINK( this, ScSortWarningDlg, BtnHdl ) );
}

ScSortWarningDlg::~ScSortWarningDlg()
{
}

IMPL_LINK( ScSortWarningDlg, BtnHdl, PushButton*, pBtn )
{
    if ( pBtn == aBtnExtSort )
    {
        EndDialog( BTN_EXTEND_RANGE );
    }
    else if( pBtn == aBtnCurSort )
    {
        EndDialog( BTN_CURRENT_SELECTION );
    }
    return 0;
}
//========================================================================//

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
