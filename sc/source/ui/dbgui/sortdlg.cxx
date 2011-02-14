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


#include <vcl/msgbox.hxx>
#include "tpsort.hxx"
#include "sortdlg.hxx"
#include "scresid.hxx"
#include "sortdlg.hrc"

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <layout/layout-pre.hxx>
#endif

ScSortDlg::ScSortDlg( Window*           pParent,
                      const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( RID_SCDLG_SORT ),
                      pArgSet ),
        bIsHeaders  ( sal_False ),
        bIsByRows   ( sal_False )

{
#if LAYOUT_SFX_TABDIALOG_BROKEN
    AddTabPage( TP_FIELDS,  ScTabPageSortFields::Create,  0 );
    AddTabPage( TP_OPTIONS, ScTabPageSortOptions::Create, 0 );
#else
    String fields = rtl::OUString::createFromAscii ("fields");
    AddTabPage( TP_FIELDS, fields, ScTabPageSortFields::Create, 0, sal_False, TAB_APPEND);
    String options = rtl::OUString::createFromAscii ("options");
    AddTabPage( TP_OPTIONS, options, ScTabPageSortOptions::Create, 0, sal_False, TAB_APPEND);
#endif
    FreeResource();
}

__EXPORT ScSortDlg::~ScSortDlg()
{
}

//==================================================================
ScSortWarningDlg::ScSortWarningDlg( Window* pParent,
                                   const String& rExtendText,
                                   const String& rCurrentText ):
        ModalDialog     ( pParent, ScResId( RID_SCDLG_SORT_WARNING ) ),
        aFtText         ( this, ScResId( FT_TEXT ) ),
        aFtTip          ( this, ScResId( FT_TIP ) ),
        aBtnExtSort     ( this, ScResId( BTN_EXTSORT ) ),
        aBtnCurSort     ( this, ScResId( BTN_CURSORT ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) )
{
    String sTextName = aFtText.GetText();
    sTextName.SearchAndReplaceAscii("%1", rExtendText);
    sTextName.SearchAndReplaceAscii("%2", rCurrentText);
    aFtText.SetText( sTextName );

    aBtnExtSort .SetClickHdl( LINK( this, ScSortWarningDlg, BtnHdl ) );
    aBtnCurSort .SetClickHdl( LINK( this, ScSortWarningDlg, BtnHdl ) );

    FreeResource();
}

ScSortWarningDlg::~ScSortWarningDlg()
{
}

IMPL_LINK( ScSortWarningDlg, BtnHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnExtSort )
    {
        EndDialog( BTN_EXTEND_RANGE );
    }
    else if( pBtn == &aBtnCurSort )
    {
        EndDialog( BTN_CURRENT_SELECTION );
    }
    return 0;
}
//========================================================================//
