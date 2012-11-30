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

#include "namecrea.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScNameCreateDlg::ScNameCreateDlg( Window * pParent, sal_uInt16 nFlags )
    : ModalDialog( pParent, ScResId(RID_SCDLG_NAMES_CREATE) ),
    aFixedLine      ( this, ScResId( FL_FRAME ) ),
    aTopBox         ( this, ScResId( BTN_TOP ) ),
    aLeftBox        ( this, ScResId( BTN_LEFT ) ),
    aBottomBox      ( this, ScResId( BTN_BOTTOM ) ),
    aRightBox       ( this, ScResId( BTN_RIGHT ) ),
    aOKButton       ( this, ScResId( BTN_OK ) ),
    aCancelButton   ( this, ScResId( BTN_CANCEL ) ),
    aHelpButton     ( this, ScResId( BTN_HELP ) )
{
    aTopBox.Check   ( (nFlags & NAME_TOP)   ? sal_True : false );
    aLeftBox.Check  ( (nFlags & NAME_LEFT)  ? sal_True : false );
    aBottomBox.Check( (nFlags & NAME_BOTTOM)? sal_True : false );
    aRightBox.Check ( (nFlags & NAME_RIGHT) ? sal_True : false );

    FreeResource();
}

//------------------------------------------------------------------

sal_uInt16 ScNameCreateDlg::GetFlags() const
{
    sal_uInt16  nResult = 0;

    nResult |= aTopBox.IsChecked()      ? NAME_TOP:     0 ;
    nResult |= aLeftBox.IsChecked()     ? NAME_LEFT:    0 ;
    nResult |= aBottomBox.IsChecked()   ? NAME_BOTTOM:  0 ;
    nResult |= aRightBox.IsChecked()    ? NAME_RIGHT:   0 ;

    return nResult;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
