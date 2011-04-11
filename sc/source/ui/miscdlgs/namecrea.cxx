/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
