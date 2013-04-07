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

#include "tabbgcolordlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"

#include <tools/color.hxx>
#include <sfx2/objsh.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/resid.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>

#include <boost/scoped_ptr.hpp>

//==================================================================

#define HDL(hdl) LINK(this,ScTabBgColorDlg,hdl)

ScTabBgColorDlg::ScTabBgColorDlg( Window*         pParent,
                                  const String&   rTitle,
                                  const String&   rTabBgColorNoColorText,
                                  const Color&    rDefaultColor,
                                  const OString& sHelpId ) :
    ModalDialog    ( pParent, ScResId( RID_SCDLG_TAB_BG_COLOR ) ),
    aBorderWin              ( this, ScResId( TAB_BG_COLOR_CT_BORDER ) ),
    aTabBgColorSet          ( &aBorderWin, ScResId( TAB_BG_COLOR_SET_BGDCOLOR ), this ),
    aBtnOk                  ( this, ScResId( BTN_OK ) ),
    aBtnCancel              ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp                ( this, ScResId( BTN_HELP ) ),
    aTabBgColor             ( rDefaultColor ),
    aTabBgColorNoColorText  ( rTabBgColorNoColorText ),
    msHelpId                ( sHelpId )

{
    SetHelpId( sHelpId );
    this->SetText( rTitle );
    this->SetStyle(GetStyle() | WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK | WB_DIALOGCONTROL | WB_SYSTEMWINDOW | WB_STANDALONE | WB_HIDE);

    FillColorValueSets_Impl();
    aTabBgColorSet.SetDoubleClickHdl( HDL(TabBgColorDblClickHdl_Impl) );
    aBtnOk.SetClickHdl( HDL(TabBgColorOKHdl_Impl) );
    FreeResource();
}

//------------------------------------------------------------------------

void ScTabBgColorDlg::GetSelectedColor( Color& rColor ) const
{
    rColor = this->aTabBgColor;
}

ScTabBgColorDlg::~ScTabBgColorDlg()
{
}

void ScTabBgColorDlg::FillColorValueSets_Impl()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    XColorListRef pColorList;

    const Size aSize15x15 = Size( 15, 15 );
    sal_uInt16 nSelectedItem = 0;

    OSL_ENSURE( pDocSh, "DocShell not found!" );

    if ( pDocSh && ( 0 != ( pItem = pDocSh->GetItem(SID_COLOR_TABLE) ) ) )
        pColorList = ( (SvxColorListItem*)pItem )->GetColorList();
    if ( !pColorList.is() )
        pColorList = XColorList::CreateStdColorList();

    if ( pColorList.is() )
    {
        sal_uInt16 i = 0;
        long nCount = pColorList->Count();
        XColorEntry* pEntry = NULL;
        Color aColWhite( COL_WHITE );
        String aStrWhite( EditResId( RID_SVXITEMS_COLOR_WHITE ) );
        WinBits nBits = ( aTabBgColorSet.GetStyle() | WB_NAMEFIELD | WB_ITEMBORDER | WB_NONEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_NOPOINTERFOCUS);
        aTabBgColorSet.SetText( aTabBgColorNoColorText );
        aTabBgColorSet.SetStyle( nBits );
        for ( i = 0; i < nCount; i++ )
        {
            pEntry = pColorList->GetColor(i);
            aTabBgColorSet.InsertItem( i + 1, pEntry->GetColor(), pEntry->GetName() );
            if (pEntry->GetColor() == aTabBgColor)
                nSelectedItem = (i + 1);
        }

        while ( i < 80 )
        {
            aTabBgColorSet.InsertItem( i + 1, aColWhite, aStrWhite );
            i++;
        }

        if ( nCount > 80 )
        {
            aTabBgColorSet.SetStyle( nBits | WB_VSCROLL );
        }
    }
    aTabBgColorSet.SetColCount( 10 );
    aTabBgColorSet.SetLineCount( 10 );
    aTabBgColorSet.CalcWindowSizePixel( aSize15x15 );
    aTabBgColorSet.Format();
    aTabBgColorSet.SelectItem(nSelectedItem);
    aTabBgColorSet.Resize();
}

IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorDblClickHdl_Impl)
/*
    Handler, called when color selection is changed
*/
{
    sal_uInt16 nItemId = aTabBgColorSet.GetSelectItemId();
    Color aColor = nItemId ? ( aTabBgColorSet.GetItemColor( nItemId ) ) : Color( COL_AUTO );
    aTabBgColor = aColor;
    EndDialog( sal_True );
    return 0;
}

IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorOKHdl_Impl)
{

//    Handler, called when the OK button is pushed

    sal_uInt16 nItemId = aTabBgColorSet.GetSelectItemId();
    Color aColor = nItemId ? ( aTabBgColorSet.GetItemColor( nItemId ) ) : Color( COL_AUTO );
    aTabBgColor = aColor;
    EndDialog( sal_True );
    return 0;
}

ScTabBgColorDlg::ScTabBgColorValueSet::ScTabBgColorValueSet( Control* pParent, const ResId& rResId, ScTabBgColorDlg* pTabBgColorDlg ) :
    ValueSet(pParent, rResId)
{
    aTabBgColorDlg = pTabBgColorDlg;
}

void ScTabBgColorDlg::ScTabBgColorValueSet::KeyInput( const KeyEvent& rKEvt )
{
    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_SPACE:
        case KEY_RETURN:
        {
            sal_uInt16 nItemId = GetSelectItemId();
            const Color& aColor = nItemId ? ( GetItemColor( nItemId ) ) : Color( COL_AUTO );
            aTabBgColorDlg->aTabBgColor = aColor;
            aTabBgColorDlg->EndDialog(sal_True);
        }
        break;
    }
    ValueSet::KeyInput(rKEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
