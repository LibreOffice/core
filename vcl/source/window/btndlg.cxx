/*************************************************************************
 *
 *  $RCSfile: btndlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_BTNDLG_CXX
#include <tools/ref.hxx>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif
#ifndef _SV_BTNDLG_HXX
#include <btndlg.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif


#pragma hdrstop

// =======================================================================

struct ImplBtnDlgItem
{
    USHORT              mnId;
    BOOL                mbOwnButton;
    BOOL                mbDummyAlign;
    long                mnSepSize;
    PushButton*         mpPushButton;
};

DECLARE_LIST( ImplBtnDlgItemList, ImplBtnDlgItem* );

// =======================================================================

void ButtonDialog::ImplInitData()
{
    mpItemList              = new ImplBtnDlgItemList( 8, 8 );
    mnButtonSize            = 0;
    mnCurButtonId           = 0;
    mnFocusButtonId         = BUTTONDIALOG_BUTTON_NOTFOUND;
    mbFormat                = TRUE;
}

// -----------------------------------------------------------------------

ButtonDialog::ButtonDialog( WindowType nType ) :
    Dialog( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

ButtonDialog::ButtonDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_TABDIALOG )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ButtonDialog::ButtonDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_BUTTONDIALOG )
{
    ImplInitData();
    rResId.SetRT( RSC_DIALOG );     // !!!!!!!!!! RSC_BUTTONDIALOG !!!!!!!!
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

ButtonDialog::~ButtonDialog()
{
    ImplBtnDlgItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mpPushButton && pItem->mbOwnButton )
            delete pItem->mpPushButton;
        delete pItem;
        pItem = mpItemList->Next();
    }

    delete mpItemList;
}

// -----------------------------------------------------------------------

PushButton* ButtonDialog::ImplCreatePushButton( USHORT nBtnFlags )
{
    PushButton* pBtn;
    WinBits     nStyle = 0;

    if ( nBtnFlags & BUTTONDIALOG_DEFBUTTON )
        nStyle |= WB_DEFBUTTON;
    if ( nBtnFlags & BUTTONDIALOG_CANCELBUTTON )
        pBtn = new CancelButton( this, nStyle );
    else if ( nBtnFlags & BUTTONDIALOG_OKBUTTON )
        pBtn = new OKButton( this, nStyle );
    else if ( nBtnFlags & BUTTONDIALOG_HELPBUTTON )
        pBtn = new HelpButton( this, nStyle );
    else
        pBtn = new PushButton( this, nStyle );

    if ( !(nBtnFlags & BUTTONDIALOG_HELPBUTTON) )
        pBtn->SetClickHdl( LINK( this, ButtonDialog, ImplClickHdl ) );

    return pBtn;
}

// -----------------------------------------------------------------------

ImplBtnDlgItem* ButtonDialog::ImplGetItem( USHORT nId ) const
{
    ImplBtnDlgItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nId )
            return pItem;

        pItem = mpItemList->Next();
    }

    return NULL;
}

// -----------------------------------------------------------------------

long ButtonDialog::ImplGetButtonSize()
{
    if ( !mbFormat )
        return mnButtonSize;

    // Calculate ButtonSize
    long    nLastSepSize = 0;
    long    nSepSize = 0;
    long    nButtonCount = 0;
    maCtrlSize = Size( IMPL_MINSIZE_BUTTON_WIDTH, IMPL_MINSIZE_BUTTON_HEIGHT );
    ImplBtnDlgItem* pItem = mpItemList->First();
    while ( pItem )
    {
        nSepSize += nLastSepSize;

        long nTxtWidth = pItem->mpPushButton->GetCtrlTextWidth( pItem->mpPushButton->GetText() );
        nTxtWidth += IMPL_EXTRA_BUTTON_WIDTH;
        if ( nTxtWidth > maCtrlSize.Width() )
            maCtrlSize.Width() = nTxtWidth;
        long nTxtHeight = pItem->mpPushButton->GetTextHeight();
        nTxtHeight += IMPL_EXTRA_BUTTON_HEIGHT;
        if ( nTxtHeight > maCtrlSize.Height() )
            maCtrlSize.Height() = nTxtHeight;

        nSepSize += pItem->mnSepSize;

        if ( GetStyle() & WB_HORZ )
            nLastSepSize = IMPL_SEP_BUTTON_X;
        else
            nLastSepSize = IMPL_SEP_BUTTON_Y;

        nButtonCount++;

        pItem = mpItemList->Next();
    }

    if ( GetStyle() & WB_HORZ )
        mnButtonSize  = nSepSize + (nButtonCount*maCtrlSize.Width());
    else
        mnButtonSize = nSepSize + (nButtonCount*maCtrlSize.Height());

    return mnButtonSize;
}

// -----------------------------------------------------------------------

void ButtonDialog::ImplPosControls()
{
    if ( !mbFormat )
        return;

    // Create PushButtons and determine Sizes
    ImplGetButtonSize();

    // determine dialog size
    ImplBtnDlgItem* pItem;
    Size            aDlgSize = maPageSize;
    long            nX;
    long            nY;
    if ( GetStyle() & WB_HORZ )
    {
        if ( mnButtonSize+(IMPL_DIALOG_OFFSET*2) > aDlgSize.Width() )
            aDlgSize.Width() = mnButtonSize+(IMPL_DIALOG_OFFSET*2);
        if ( GetStyle() & WB_LEFT )
            nX = IMPL_DIALOG_OFFSET;
        else if ( GetStyle() & WB_RIGHT )
            nX = aDlgSize.Width()-mnButtonSize-IMPL_DIALOG_OFFSET;
        else
            nX = (aDlgSize.Width()-mnButtonSize)/2;

        aDlgSize.Height() += IMPL_DIALOG_OFFSET+maCtrlSize.Height();
        nY = aDlgSize.Height()-maCtrlSize.Height()-IMPL_DIALOG_OFFSET;
    }
    else
    {
        if ( mnButtonSize+(IMPL_DIALOG_OFFSET*2) > aDlgSize.Height() )
            aDlgSize.Height() = mnButtonSize+(IMPL_DIALOG_OFFSET*2);
        if ( GetStyle() & WB_BOTTOM )
            nY = aDlgSize.Height()-mnButtonSize-IMPL_DIALOG_OFFSET;
        else if ( GetStyle() & WB_VCENTER )
            nY = (aDlgSize.Height()-mnButtonSize)/2;
        else
            nY = IMPL_DIALOG_OFFSET;

        aDlgSize.Width() += IMPL_DIALOG_OFFSET+maCtrlSize.Width();
        nX = aDlgSize.Width()-maCtrlSize.Width()-IMPL_DIALOG_OFFSET;
    }

    // Arrange PushButtons
    pItem = mpItemList->First();
    while ( pItem )
    {
        if ( GetStyle() & WB_HORZ )
            nX += pItem->mnSepSize;
        else
            nY += pItem->mnSepSize;
        pItem->mpPushButton->SetPosSizePixel( Point( nX, nY ), maCtrlSize );
        pItem->mpPushButton->Show();
        if ( GetStyle() & WB_HORZ )
            nX += maCtrlSize.Width()+IMPL_SEP_BUTTON_X;
        else
            nY += maCtrlSize.Height()+IMPL_SEP_BUTTON_Y;

        pItem = mpItemList->Next();
    }

    SetOutputSizePixel( aDlgSize );

    mbFormat = FALSE;
}

// -----------------------------------------------------------------------

IMPL_LINK( ButtonDialog, ImplClickHdl, PushButton*, pBtn )
{
    ImplBtnDlgItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mpPushButton == pBtn )
        {
            mnCurButtonId = pItem->mnId;
            Click();
            break;
        }

        pItem = mpItemList->Next();
    }

    return 0;
}

// -----------------------------------------------------------------------

void ButtonDialog::Resize()
{
}

// -----------------------------------------------------------------------

void ButtonDialog::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        ImplPosControls();

        // Focus evt. auf den entsprechenden Button setzen
        if ( mnFocusButtonId != BUTTONDIALOG_BUTTON_NOTFOUND )
        {
            ImplBtnDlgItem* pItem = mpItemList->First();
            while ( pItem )
            {
                if ( pItem->mnId == mnFocusButtonId )
                {
                    if ( pItem->mpPushButton->IsVisible() )
                        pItem->mpPushButton->GrabFocus();
                    break;
                }

                pItem = mpItemList->Next();
            }
        }
    }

    Dialog::StateChanged( nType );
}

// -----------------------------------------------------------------------

void ButtonDialog::Click()
{
    if ( !maClickHdl )
    {
        if ( IsInExecute() )
            EndDialog( GetCurButtonId() );
    }
    else
        maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ButtonDialog::AddButton( const XubString& rText, USHORT nId,
                              USHORT nBtnFlags, long nSepPixel )
{
    // PageItem anlegen
    ImplBtnDlgItem* pItem   = new ImplBtnDlgItem;
    pItem->mnId             = nId;
    pItem->mbOwnButton      = TRUE;
    pItem->mnSepSize        = nSepPixel;
    pItem->mpPushButton     = ImplCreatePushButton( nBtnFlags );
    if ( rText.Len() )
        pItem->mpPushButton->SetText( rText );

    // In die StarView-Liste eintragen
    mpItemList->Insert( pItem, LIST_APPEND );

    if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
        mnFocusButtonId = nId;

    mbFormat = TRUE;
}

// -----------------------------------------------------------------------

void ButtonDialog::AddButton( StandardButtonType eType, USHORT nId,
                              USHORT nBtnFlags, long nSepPixel )
{
    // PageItem anlegen
    ImplBtnDlgItem* pItem   = new ImplBtnDlgItem;
    pItem->mnId             = nId;
    pItem->mbOwnButton      = TRUE;
    pItem->mnSepSize        = nSepPixel;

    if ( eType == BUTTON_HELP )
        nBtnFlags |= BUTTONDIALOG_HELPBUTTON;
    else if ( (eType == BUTTON_CANCEL) || (eType == BUTTON_CLOSE) )
        nBtnFlags |= BUTTONDIALOG_CANCELBUTTON;
    pItem->mpPushButton = ImplCreatePushButton( nBtnFlags );
    pItem->mpPushButton->SetText( Button::GetStandardText( eType ) );
    pItem->mpPushButton->SetHelpText( Button::GetStandardHelpText( eType ) );

    if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
        mnFocusButtonId = nId;

    // In die StarView-Liste eintragen
    mpItemList->Insert( pItem, LIST_APPEND );

    mbFormat = TRUE;
}

// -----------------------------------------------------------------------

void ButtonDialog::AddButton( PushButton* pBtn, USHORT nId,
                              USHORT nBtnFlags, long nSepPixel )
{
    // PageItem anlegen
    ImplBtnDlgItem* pItem   = new ImplBtnDlgItem;
    pItem->mnId             = nId;
    pItem->mbOwnButton      = FALSE;
    pItem->mnSepSize        = nSepPixel;
    pItem->mpPushButton     = pBtn;

    if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
        mnFocusButtonId = nId;

    // In die StarView-Liste eintragen
    mpItemList->Insert( pItem, LIST_APPEND );

    mbFormat = TRUE;
}

// -----------------------------------------------------------------------

void ButtonDialog::RemoveButton( USHORT nId )
{
    ImplBtnDlgItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nId )
        {
            pItem->mpPushButton->Hide();
            if ( pItem->mbOwnButton )
                delete pItem->mpPushButton;
            delete pItem;
            mpItemList->Remove();
            mbFormat = TRUE;
            break;
        }

        pItem = mpItemList->Next();
    }

    DBG_ERRORFILE( "ButtonDialog::RemoveButton(): ButtonId invalid" );
}

// -----------------------------------------------------------------------

void ButtonDialog::Clear()
{
    ImplBtnDlgItem* pItem = mpItemList->First();
    while ( pItem )
    {
        pItem->mpPushButton->Hide();
        if ( pItem->mbOwnButton )
            delete pItem->mpPushButton;
        delete pItem;
        pItem = mpItemList->Next();
    }

    mpItemList->Clear();
    mbFormat = TRUE;
}

// -----------------------------------------------------------------------

USHORT ButtonDialog::GetButtonCount() const
{
    return (USHORT)mpItemList->Count();
}

// -----------------------------------------------------------------------

USHORT ButtonDialog::GetButtonId( USHORT nButton ) const
{
    if ( nButton < mpItemList->Count() )
        return (USHORT)mpItemList->GetObject( nButton )->mnId;
    else
        return BUTTONDIALOG_BUTTON_NOTFOUND;
}

// -----------------------------------------------------------------------

PushButton* ButtonDialog::GetPushButton( USHORT nId ) const
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        return pItem->mpPushButton;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ButtonDialog::SetButtonText( USHORT nId, const XubString& rText )
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
    {
        pItem->mpPushButton->SetText( rText );
        mbFormat = TRUE;
    }
}

// -----------------------------------------------------------------------

XubString ButtonDialog::GetButtonText( USHORT nId ) const
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        return pItem->mpPushButton->GetText();
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ButtonDialog::SetButtonHelpText( USHORT nId, const XubString& rText )
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        pItem->mpPushButton->SetHelpText( rText );
}

// -----------------------------------------------------------------------

XubString ButtonDialog::GetButtonHelpText( USHORT nId ) const
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        return pItem->mpPushButton->GetHelpText();
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ButtonDialog::SetButtonHelpId( USHORT nId, ULONG nHelpId )
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        pItem->mpPushButton->SetHelpId( nHelpId );
}

// -----------------------------------------------------------------------

ULONG ButtonDialog::GetButtonHelpId( USHORT nId ) const
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        return pItem->mpPushButton->GetHelpId();
    else
        return 0;
}
