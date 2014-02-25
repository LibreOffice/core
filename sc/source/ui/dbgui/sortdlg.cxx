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

ScSortDlg::ScSortDlg(Window* pParent, const SfxItemSet* pArgSet)
    : SfxTabDialog(pParent, "SortDialog", "modules/scalc/ui/sortdialog.ui", pArgSet)
    , bIsHeaders(false)
    , bIsByRows(false)
{
    AddTabPage("criteria",  ScTabPageSortFields::Create,  0);
    AddTabPage("options", ScTabPageSortOptions::Create, 0);
}


ScSortWarningDlg::ScSortWarningDlg(Window* pParent,
    const OUString& rExtendText, const OUString& rCurrentText)
    : ModalDialog(pParent, "SortWarning", "modules/scalc/ui/sortwarning.ui")
{
    get(aFtText, "sorttext" );
    get( aBtnExtSort, "extend" );
    get( aBtnCurSort, "current" );

    OUString sTextName = aFtText->GetText();
    sTextName = sTextName.replaceFirst("%1", rExtendText);
    sTextName = sTextName.replaceFirst("%2", rCurrentText);
    aFtText->SetText( sTextName );

    aBtnExtSort->SetClickHdl( LINK( this, ScSortWarningDlg, BtnHdl ) );
    aBtnCurSort->SetClickHdl( LINK( this, ScSortWarningDlg, BtnHdl ) );
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
