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

#include "lbseldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScSelEntryDlg::ScSelEntryDlg(      Window*  pParent,
                                   sal_uInt16   nResId,
                             const String&  aTitle,
                             const String&  aLbTitle,
                             const std::vector<String> &rEntryList ) :
    ModalDialog     ( pParent, ScResId( nResId ) ),
    //
    aFlLbTitle      ( this, ScResId( FL_ENTRYLIST ) ),
    aLb             ( this, ScResId( LB_ENTRYLIST ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    SetText( aTitle );
    aFlLbTitle.SetText( aLbTitle );
    aLb.Clear();
    aLb.SetDoubleClickHdl( LINK( this, ScSelEntryDlg, DblClkHdl ) );

    std::vector<String>::const_iterator pIter;
    for ( pIter = rEntryList.begin(); pIter != rEntryList.end(); ++pIter )
        aLb.InsertEntry(*pIter);

    if ( aLb.GetEntryCount() > 0 )
        aLb.SelectEntryPos( 0 );

    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

String ScSelEntryDlg::GetSelectEntry() const
{
    return aLb.GetSelectEntry();
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(ScSelEntryDlg, DblClkHdl)
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ScSelEntryDlg, DblClkHdl)

//------------------------------------------------------------------------

ScSelEntryDlg::~ScSelEntryDlg()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
