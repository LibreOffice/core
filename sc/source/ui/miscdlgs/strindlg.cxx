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

#include <tools/debug.hxx>

#include "strindlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef ScResId
#define ScResId(x) #x
#undef ModalDialog
#define ModalDialog( parent, id ) Dialog( parent, "string-input.xml", id )
#endif /* ENABLE_LAYOUT */

//==================================================================

ScStringInputDlg::ScStringInputDlg( Window*         pParent,
                                    const String&   rTitle,
                                    const String&   rEditTitle,
                                    const String&   rDefault,
                                    const rtl::OString& sHelpId, const rtl::OString& sEditHelpId    ) :
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

void ScStringInputDlg::GetInputString( String& rString ) const
{
    rString = aEdInput.GetText();
}

ScStringInputDlg::~ScStringInputDlg()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
