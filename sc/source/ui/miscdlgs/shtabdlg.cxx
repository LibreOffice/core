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



//------------------------------------------------------------------

#include <vcl/msgbox.hxx>

#include "shtabdlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScShowTabDlg::ScShowTabDlg( Window* pParent ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_SHOW_TAB ) ),
    aFtLbTitle      ( this, ScResId( FT_LABEL ) ),
    aLb             ( this, ScResId( LB_ENTRYLIST ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    aLb.Clear();
    aLb.SetDoubleClickHdl( LINK( this, ScShowTabDlg, DblClkHdl ) );

    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

void ScShowTabDlg::SetDescription(
        const String& rTitle, const String& rFixedText,
        const OString& rDlgHelpId, const OString& sLbHelpId )
{
    SetText( rTitle );
    aFtLbTitle.SetText( rFixedText );
    SetHelpId( rDlgHelpId );
    aLb.SetHelpId( sLbHelpId );
}

void ScShowTabDlg::Insert( const String& rString, sal_Bool bSelected )
{
    aLb.InsertEntry( rString );
    if( bSelected )
        aLb.SelectEntryPos( aLb.GetEntryCount() - 1 );
}

//------------------------------------------------------------------------

sal_uInt16 ScShowTabDlg::GetSelectEntryCount() const
{
    return aLb.GetSelectEntryCount();
}

String ScShowTabDlg::GetSelectEntry(sal_uInt16 nPos) const
{
    return aLb.GetSelectEntry(nPos);
}

sal_uInt16 ScShowTabDlg::GetSelectEntryPos(sal_uInt16 nPos) const
{
    return aLb.GetSelectEntryPos(nPos);
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(ScShowTabDlg, DblClkHdl)
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ScShowTabDlg, DblClkHdl)

ScShowTabDlg::~ScShowTabDlg()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
