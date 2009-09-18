/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tabbgcolordlg.cxx,v $
 * $Revision: 1.0 $
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
#include <tools/color.hxx>
#include <sfx2/objsh.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <svtools/pathoptions.hxx>
#include <tools/resid.hxx>
#include "tabbgcolordlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"
#include <svx/svxitems.hrc>

//==================================================================

#define HDL(hdl) LINK(this,ScTabBgColorDlg,hdl)

__EXPORT ScTabBgColorDlg::ScTabBgColorDlg( Window*         pParent,
                                    const String&   rTitle,
                                    const String&   rTabBgColorNoColorText,
                                    const Color&    rDefaultColor,
                                    ULONG nHelpId ) :
    ModalDialog    ( pParent, ScResId( RID_SCDLG_TAB_BG_COLOR ) ),
    aBorderWin              ( this, ScResId( TAB_BG_COLOR_CT_BORDER ) ),
    aTabBgColorSet          ( &aBorderWin, ScResId( TAB_BG_COLOR_SET_BGDCOLOR ), this ),
    aTabBgColorBox          ( this, ScResId( TAB_BG_COLOR_GB_BGDCOLOR ) ),
    aBtnOk                  ( this, ScResId( BTN_OK ) ),
    aBtnCancel              ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp                ( this, ScResId( BTN_HELP ) ),
    aTabBgColorNoColorText  ( rTabBgColorNoColorText ),
    aTabBgColor             ( rDefaultColor ),
    aHelpId                 ( nHelpId )

{
    SetHelpId( nHelpId );
    this->SetText( rTitle );
    this->SetStyle(GetStyle() | WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK | WB_DIALOGCONTROL | WB_SYSTEMWINDOW | WB_STANDALONE | WB_HIDE);
    //TODO: Assign Help ID's to all controls...
    aTabBgColorBox.SetText(rTitle);
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

__EXPORT ScTabBgColorDlg::~ScTabBgColorDlg()
{
}

void ScTabBgColorDlg::FillColorValueSets_Impl()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    XColorTable* pColorTable = NULL;
    const Size aSize15x15 = Size( 15, 15 );
    const Size aSize10x10 = Size( 10, 10 );
    const Size aSize5x5 = Size( 5, 5 );
    USHORT nSelectedItem = 0;
    FASTBOOL bOwn = FALSE;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh && ( 0 != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) ) )
        pColorTable = ( (SvxColorTableItem*)pItem )->GetColorTable();
    if ( !pColorTable )
    {
        bOwn = TRUE;
        pColorTable = new XColorTable( SvtPathOptions().GetPalettePath() );
    }
    if ( pColorTable )
    {
        short i = 0;
        long nCount = pColorTable->Count();
        XColorEntry* pEntry = NULL;
        Color aColWhite( COL_WHITE );
        String aStrWhite( ScResId( RID_SVXITEMS_COLOR_WHITE ) );
        WinBits nBits = ( aTabBgColorSet.GetStyle() | WB_NAMEFIELD | WB_ITEMBORDER | WB_NONEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_NOPOINTERFOCUS);
        aTabBgColorSet.SetText( aTabBgColorNoColorText );
        aTabBgColorSet.SetStyle( nBits );
        for ( i = 0; i < nCount; i++ )
        {
            pEntry = pColorTable->GetColor(i);
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
    if ( bOwn )
        delete pColorTable;
}

IMPL_LINK( ScTabBgColorDlg, TabBgColorDblClickHdl_Impl, ValueSet*, EMPTYARG )
/*
    Handler, called when color selection is changed
*/
{
    USHORT nItemId = aTabBgColorSet.GetSelectItemId();
    Color aColor = nItemId ? ( aTabBgColorSet.GetItemColor( nItemId ) ) : Color( COL_AUTO );
    aTabBgColor = aColor;
    EndDialog( TRUE );
    return 0;
}

IMPL_LINK( ScTabBgColorDlg, TabBgColorOKHdl_Impl, OKButton*, EMPTYARG )
{

//    Handler, called when the OK button is pushed

    USHORT nItemId = aTabBgColorSet.GetSelectItemId();
    Color aColor = nItemId ? ( aTabBgColorSet.GetItemColor( nItemId ) ) : Color( COL_AUTO );
    aTabBgColor = aColor;
    EndDialog( TRUE );
    return 0;
}

__EXPORT ScTabBgColorDlg::ScTabBgColorValueSet::ScTabBgColorValueSet( Control* pParent, const ResId& rResId, ScTabBgColorDlg* pTabBgColorDlg ) :
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
            USHORT nItemId = GetSelectItemId();
            const Color& aColor = nItemId ? ( GetItemColor( nItemId ) ) : Color( COL_AUTO );
            aTabBgColorDlg->aTabBgColor = aColor;
            aTabBgColorDlg->EndDialog(TRUE);
        }
        break;
    }
    ValueSet::KeyInput(rKEvt);
}
