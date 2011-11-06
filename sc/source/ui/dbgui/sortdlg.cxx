/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
