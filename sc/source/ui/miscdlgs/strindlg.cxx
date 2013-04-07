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

#include "strindlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"

//==================================================================

ScStringInputDlg::ScStringInputDlg( Window*         pParent,
                                    const String&   rTitle,
                                    const String&   rEditTitle,
                                    const String&   rDefault,
                                    const OString& sHelpId, const OString& sEditHelpId    ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_STRINPUT ) ),
    //
    aFtEditTitle    ( this, ScResId( FT_LABEL ) ),
    aEdInput        ( this, ScResId( ED_INPUT ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    SetHelpId( sHelpId );
    SetText( rTitle );
    aFtEditTitle.SetText( rEditTitle );
    aEdInput.SetText( rDefault );
    aEdInput.SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    aEdInput.SetHelpId( sEditHelpId );
    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

void ScStringInputDlg::GetInputString( OUString& rString ) const
{
    rString = aEdInput.GetText();
}

ScStringInputDlg::~ScStringInputDlg()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
