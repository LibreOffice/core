/*************************************************************************
 *
 *  $RCSfile: iconcdlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pw $ $Date: 2000-11-22 13:51:18 $
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

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_HELP_HXX
#include <sfx2/sfxhelp.hxx>
#endif

#ifndef _TOOLS_RC_H
#include <tools/rc.h>
#endif

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#include "iconcdlg.hxx"

#ifndef _SVX_HELPID_HRC
#include "helpid.hrc"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif

#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif

#ifdef WNT
int __cdecl IconcDlgCmpUS_Impl( const void* p1, const void* p2 )
#else
#if defined(OS2) && defined(ICC)
int _Optlink IconcDlgCmpUS_Impl( const void* p1, const void* p2 )
#else
int IconcDlgCmpUS_Impl( const void* p1, const void* p2 )
#endif
#endif
{
    return *(USHORT*)p1 - *(USHORT*)p2;
}

//#####################################################################
//
// Class IconChoicePage
//
//#####################################################################

/**********************************************************************
|
| Ctor / Dtor
|
\**********************************************************************/

IconChoicePage::IconChoicePage( Window *pParent, const ResId &rResId,
                                const SfxItemSet &rAttrSet )
:   TabPage                   ( pParent, rResId ),
    pSet                      ( &rAttrSet ),
    bHasExchangeSupport       ( FALSE ),
    pDialog                   ( NULL ),
    bStandard                 ( FALSE )
{
    SetStyle ( GetStyle()  | WB_DIALOGCONTROL | WB_HIDE );
}

// -----------------------------------------------------------------------

IconChoicePage::~IconChoicePage()
{
}

/**********************************************************************
|
| Activate / Deaktivate
|
\**********************************************************************/

void IconChoicePage::ActivatePage( const SfxItemSet& )
{
}

// -----------------------------------------------------------------------

int IconChoicePage::DeactivatePage( SfxItemSet* )
{
    return LEAVE_PAGE;
}

/**********************************************************************
|
| ...
|
\**********************************************************************/

void IconChoicePage::FillUserData()
{
}

// -----------------------------------------------------------------------

BOOL IconChoicePage::IsReadOnly() const
{
    return FALSE;
}

/**********************************************************************
|
| handling itemsets
|
\**********************************************************************/

const SfxPoolItem* IconChoicePage::GetItem( const SfxItemSet& rSet,
                                            USHORT nSlot )
{
    const SfxItemPool* pPool = rSet.GetPool();
    USHORT nWh = pPool->GetWhich( nSlot );
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( nWh, TRUE, &pItem );

    if ( !pItem && nWh != nSlot )
        pItem = &pPool->GetDefaultItem( nWh );

    return pItem;
}

// -----------------------------------------------------------------------

const SfxPoolItem* IconChoicePage::GetOldItem( const SfxItemSet& rSet,
                                               USHORT nSlot )
{
    const SfxItemSet& rOldSet = GetItemSet();
    USHORT nWh = GetWhich( nSlot );
    const SfxPoolItem* pItem = 0;

    if ( bStandard && rOldSet.GetParent() )
        pItem = GetItem( *rOldSet.GetParent(), nSlot );
    else if ( rSet.GetParent() && SFX_ITEM_DONTCARE == rSet.GetItemState( nWh ) )
        pItem = GetItem( *rSet.GetParent(), nSlot );
    else
        pItem = GetItem( rOldSet, nSlot );

    return pItem;
}

// -----------------------------------------------------------------------

const SfxPoolItem* IconChoicePage::GetExchangeItem( const SfxItemSet& rSet,
                                                    USHORT nSlot )
{
    if ( pDialog && !pDialog->IsInOK() && pDialog->GetExampleSet() )
        return GetItem( *pDialog->GetExampleSet(), nSlot );
    else
        return GetOldItem( rSet, nSlot );
}

/**********************************************************************
|
| window-methods
|
\**********************************************************************/

void IconChoicePage::ImplInitSettings()
{
    Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode( TRUE );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( TRUE );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( FALSE );
        SetParentClipMode( 0 );
        SetPaintTransparent( FALSE );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

// -----------------------------------------------------------------------

void IconChoicePage::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void IconChoicePage::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

/**********************************************************************
|
| D & D
|
\**********************************************************************/

BOOL IconChoicePage::QueryDrop( DropEvent& rDEvt )
{
    if( pDialog )
        return pDialog->QueryDrop( rDEvt );
    else
        return FALSE;
}

// -----------------------------------------------------------------------

BOOL IconChoicePage::Drop( const DropEvent& rDEvt )
{
    if( pDialog )
        return pDialog->Drop( rDEvt );
    else
        return Window::Drop( rDEvt );
}

//#####################################################################
//
// Class IconChoiceDialog
//
//#####################################################################

/**********************************************************************
|
| Ctor / Dtor
|
\**********************************************************************/

IconChoiceDialog::IconChoiceDialog ( Window* pParent, const ResId &rResId,
                                     const EIconChoicePos ePos,
                                     const SfxItemSet *pItemSet )//, BOOL bEditFmt, const String *pUserButtonText = 0 )
:   ModalDialog         ( pParent, rResId ),
    aOKBtn          ( this, WB_DEFBUTTON ),
    aCancelBtn      ( this, WB_DEFBUTTON ),
    aHelpBtn        ( this ),
    aResetBtn       ( this ),
    //pUserBtn      ( pUserButtonText? new PushButton(this): 0 ),
    //aBaseFmtBtn       ( this ),
    maIconCtrl      ( this, WB_3DLOOK | WB_ICON | WB_BORDER |
                            WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME |
                            WB_NOSELECTION | WB_NODRAGSELECTION | WB_TABSTOP ),
    meChoicePos     ( ePos ),
    mnCurrentPageId ( USHRT_MAX ),
    pSet            ( pItemSet ),
    pExampleSet     ( NULL ),
    pOutSet         ( NULL ),
    pRanges         ( NULL ),
    bHideResetBtn   ( FALSE ),
    bModal          ( FALSE ),
    bInOK           ( FALSE ),
    bModified       ( FALSE ),
    bItemsReset     ( FALSE ),
    nResId          ( rResId.GetId() )
{
    // IconChoiceCtrl-Settings
    //maIconCtrl.SetBackground ( Wallpaper( Color (146, 146, 186) ) );

    maIconCtrl.SetStyle (WB_3DLOOK | WB_ICON | WB_BORDER | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | WB_NOSELECTION | WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN );
    SetCtrlPos ( meChoicePos );
    maIconCtrl.SetClickHdl ( LINK ( this, IconChoiceDialog , ChosePageHdl_Impl ) );
    maIconCtrl.Show();
    maIconCtrl.SetChoiceWithCursor ( TRUE );
    maIconCtrl.SetHelpId( HID_ICCDIALOG_CHOICECTRL );

    // ItemSet
    if ( pSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }

    // Buttons
    aOKBtn.SetClickHdl   ( LINK( this, IconChoiceDialog, OkHdl ) );
    aOKBtn.SetHelpId( HID_ICCDIALOG_OK_BTN );
    aCancelBtn.SetHelpId( HID_ICCDIALOG_CANCEL_BTN );
    aResetBtn.SetClickHdl( LINK( this, IconChoiceDialog, ResetHdl ) );
    aResetBtn.SetText( SVX_RESSTR(RID_SVXSTR_ICONCHOICEDLG_RESETBUT) );
    aResetBtn.SetHelpId( HID_ICCDIALOG_RESET_BTN );
    aOKBtn.Show();
    aCancelBtn.Show();
    aHelpBtn.Show();
    aResetBtn.Show();

    SetPosSizeCtrls ( TRUE );
}

// -----------------------------------------------------------------------

/*
IconChoiceDialog ::IconChoiceDialog ( SfxViewFrame *pViewFrame, Window* pParent, const ResId &rResId,
                   const SfxItemSet * = 0, BOOL bEditFmt = FALSE,
                   const String *pUserButtonText = 0 )
:   meChoicePos     ( PosLeft ),    // Default erst ma Links
    maIconCtrl      ( this, Die_Winbits ),
    aOKBtn          ( this ),
    pUserBtn        ( pUserButtonText? new PushButton(this): 0 ),
    aCancelBtn      ( this ),
    aHelpBtn        ( this ),
    aResetBtn       ( this ),
    aBaseFmtBtn     ( this ),
    mnCurrentPageId ( 0 )
{
    FreeResource();
}
*/

// -----------------------------------------------------------------------

IconChoiceDialog ::~IconChoiceDialog ()
{
    ULONG i;

    // save configuration at INI-Manager
    // and remove pages
    SvtViewOptions aTabDlgOpt( E_TABDIALOG, String::CreateFromInt32( nResId ) );
    aTabDlgOpt.SetPosition( GetPosPixel().X(), GetPosPixel().Y() );
    aTabDlgOpt.SetPageID( mnCurrentPageId );

    const USHORT nCount = maPageList.Count();

    for ( i = 0; i < nCount; ++i )
    {
        IconChoicePageData* pData = maPageList.GetObject(i);

        if ( pData->pPage )
        {
            pData->pPage->FillUserData();
            String aPageData(pData->pPage->GetUserData());
            if ( aPageData.Len() )
            {
                SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pData->nId ) );
                aTabPageOpt.SetUserData( aPageData );
            }

            if ( pData->bOnDemand )
                delete (SfxItemSet*)&pData->pPage->GetItemSet();
            delete pData->pPage;
        }
        delete pData;
    }

    // remove Pagelist
/*  for ( i=0; i<maPageList.Count(); i++ )
    {
        IconChoicePageData* pData = (IconChoicePageData*)maPageList.GetObject ( i );

        if ( pData->bOnDemand )
            delete ( SfxItemSet * )&( pData->pPage->GetItemSet() );

        delete pData->pPage;
        delete pData;
    }*/

    // remove Userdata from Icons
    for ( i=0; i<maIconCtrl.GetEntryCount(); i++)
    {
        SvxIconChoiceCtrlEntry* pEntry = maIconCtrl.GetEntry ( i );
        USHORT* pUserData = (USHORT*) pEntry->GetUserData();
        delete pUserData;
    }

    //
    if ( pRanges )
        delete pRanges;
    if ( pOutSet )
        delete pOutSet;

    SfxHelpPI *pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->ResetTopic();
}

/**********************************************************************
|
| add new page
|
\**********************************************************************/

void IconChoiceDialog::AddTabPage( USHORT nId, const String& rIconText,
                                   const Image& rChoiceIcon,
                                   CreatePage pCreateFunc /* != 0 */,
                                   GetPageRanges pRangesFunc /* darf 0 sein */,
                                   BOOL bItemsOnDemand, ULONG nPos )
{
    IconChoicePageData* pData = new IconChoicePageData ( nId, pCreateFunc,
                                                         pRangesFunc,
                                                         bItemsOnDemand );
    maPageList.Insert ( pData, LIST_APPEND );

    pData->fnGetRanges = pRangesFunc;
    pData->bOnDemand = bItemsOnDemand;

    USHORT *pId = new USHORT ( nId );
    SvxIconChoiceCtrlEntry* pEntry = maIconCtrl.InsertEntry( rIconText, rChoiceIcon );
    pEntry->SetUserData ( (void*) pId );
}

/**********************************************************************
|
| remove page
|
\**********************************************************************/

void IconChoiceDialog::RemoveTabPage( USHORT nId )
{
    IconChoicePageData* pData = GetPageData ( nId );

    // remove page from list
    if ( pData )
    {
        maPageList.Remove ( pData );

        // save settings
        if ( pData->pPage )
        {
            pData->pPage->FillUserData();
            String aPageData(pData->pPage->GetUserData());
            if ( aPageData.Len() )
            {
                SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pData->nId ) );
                aTabPageOpt.SetUserData( aPageData );
            }
        }

        if ( pData->bOnDemand )
            delete ( SfxItemSet * )&( pData->pPage->GetItemSet() );

        delete pData->pPage;
        delete pData;
    }

    // remove Icon
    BOOL bFound=FALSE;
    for ( ULONG i=0; i<maIconCtrl.GetEntryCount() && !bFound; i++)
    {
        SvxIconChoiceCtrlEntry* pEntry = maIconCtrl.GetEntry ( i );
        USHORT* pUserData = (USHORT*) pEntry->GetUserData();

        if ( *pUserData == nId )
        {
            delete pUserData;
            maIconCtrl.RemoveEntry ( pEntry );
            bFound = TRUE;
        }
    }

    // was it the current page ?
    if ( nId = mnCurrentPageId )
    {
        mnCurrentPageId = maPageList.First()->nId;
    }

    Invalidate ();
}

/**********************************************************************
|
| Paint-method
|
\**********************************************************************/

void IconChoiceDialog::Paint( const Rectangle& rRect )
{
    Dialog::Paint ( rRect );

    for ( ULONG i=0; i<maPageList.Count(); i++ )
    {
        IconChoicePageData* pData = (IconChoicePageData*)maPageList.GetObject ( i );

        if ( pData->nId == mnCurrentPageId )
        {
            ShowPageImpl ( pData );
            pData->pPage->Invalidate();
        }
        else
        {
            HidePageImpl ( pData );
        }
    }
}

EIconChoicePos IconChoiceDialog::SetCtrlPos( const EIconChoicePos& rPos )
{
    WinBits aWinBits = maIconCtrl.GetStyle ();

    switch ( meChoicePos )
    {
        case PosLeft :
            aWinBits &= ~WB_ALIGN_TOP & ~WB_NOVSCROLL;
            aWinBits |= WB_ALIGN_LEFT | WB_NOHSCROLL;
            break;
        case PosRight :
            aWinBits &= ~WB_ALIGN_TOP & ~WB_NOVSCROLL;
            aWinBits |= WB_ALIGN_LEFT | WB_NOHSCROLL;
            break;
        case PosTop :
            aWinBits &= ~WB_ALIGN_LEFT & ~WB_NOHSCROLL;
            aWinBits |= WB_ALIGN_TOP | WB_NOVSCROLL;
            break;
        case PosBottom :
            aWinBits &= ~WB_ALIGN_LEFT & ~WB_NOHSCROLL;
            aWinBits |= WB_ALIGN_TOP | WB_NOVSCROLL;
            break;
    };
    maIconCtrl.SetStyle ( aWinBits );

    SetPosSizeCtrls();
    Invalidate ();

    EIconChoicePos eOldPos = meChoicePos;
    meChoicePos = rPos;

    return eOldPos;
}

void IconChoiceDialog::SetCtrlColor ( const Color& rColor )
{
    Wallpaper aWallpaper ( rColor );
    maIconCtrl.SetBackground( aWallpaper );
    maIconCtrl.SetFontColorToBackground ();
}

/**********************************************************************
|
| Show / Hide page or button
|
\**********************************************************************/

void IconChoiceDialog::ShowPageImpl ( IconChoicePageData* pData )
{
    if ( pData->pPage )
        pData->pPage->Show();
}

// -----------------------------------------------------------------------

void IconChoiceDialog::HidePageImpl ( IconChoicePageData* pData )
{
    if ( pData->pPage )
        pData->pPage->Hide();
}

// -----------------------------------------------------------------------

void IconChoiceDialog::RemoveResetButton()
{
    aResetBtn.Hide();
    bHideResetBtn = TRUE;
}

// -----------------------------------------------------------------------

void IconChoiceDialog::ShowPage( USHORT nId )
{
    SetCurPageId( nId );
    ActivatePageImpl( );
}

/**********************************************************************
|
| Resize Dialog
|
\**********************************************************************/

#define ICONCTRL_WIDTH_PIXEL       110
#define ICONCTRL_HEIGHT_PIXEL       75
#define MINSIZE_BUTTON_WIDTH        70
#define MINSIZE_BUTTON_HEIGHT       22

void IconChoiceDialog::Resize()
{
    Dialog::Resize ();

    if ( IsReallyVisible() )
    {
        SetPosSizeCtrls ();
    }
}

void IconChoiceDialog::SetPosSizeCtrls ( BOOL bInit )
{
    const Point aCtrlOffset ( LogicToPixel( Point( CTRLS_OFFSET, CTRLS_OFFSET ), MAP_APPFONT ) );
    Size aOutSize ( GetOutputSizePixel() );

    ////////////////////////////////////////
    // Button-Defaults
    //
    Size aDefaultButtonSize = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );

    // Reset-Button
    Size aResetButtonSize ( bInit ? aDefaultButtonSize :
                                    aResetBtn.GetSizePixel () );

    ////////////////////////////////////////
    // IconChoiceCtrl resizen & positionieren
    //
    Size aNewIconCtrlSize  ( ICONCTRL_WIDTH_PIXEL,
                             aOutSize.Height()-(2*aCtrlOffset.X()) );
    Point aIconCtrlPos;
    switch ( meChoicePos )
    {
        case PosLeft :
            aIconCtrlPos = aCtrlOffset;
            aNewIconCtrlSize = Size ( ICONCTRL_WIDTH_PIXEL,
                                      aOutSize.Height()-(2*aCtrlOffset.X()) );
            break;
        case PosRight :
            aIconCtrlPos = Point ( aOutSize.Width() - ICONCTRL_WIDTH_PIXEL -
                                   aCtrlOffset.X(), aCtrlOffset.X() );
            aNewIconCtrlSize = Size ( ICONCTRL_WIDTH_PIXEL,
                                      aOutSize.Height()-(2*aCtrlOffset.X()) );
            break;
        case PosTop :
            aIconCtrlPos = aCtrlOffset;
            aNewIconCtrlSize = Size ( aOutSize.Width()-(2*aCtrlOffset.X()),
                                      ICONCTRL_HEIGHT_PIXEL );
            break;
        case PosBottom :
            aIconCtrlPos = Point ( aCtrlOffset.X(), aOutSize.Height() -
                                   aResetButtonSize.Height() - (2*aCtrlOffset.X()) -
                                   ICONCTRL_HEIGHT_PIXEL );
            aNewIconCtrlSize = Size ( aOutSize.Width()-(2*aCtrlOffset.X()),
                                      ICONCTRL_HEIGHT_PIXEL );
            break;
    };
    maIconCtrl.SetPosSizePixel ( aIconCtrlPos, aNewIconCtrlSize );
    maIconCtrl.ArrangeIcons();

    ////////////////////////////////////////
    // Pages resizen & positionieren
    //
    for ( ULONG i=0; i<maPageList.Count(); i++ )
    {
        IconChoicePageData* pData = (IconChoicePageData*)maPageList.GetObject ( i );

        Point aNewPagePos;
        Size aNewPageSize;
        switch ( meChoicePos )
        {
            case PosLeft :
                aNewPagePos = Point ( aNewIconCtrlSize.Width() + (2*CTRLS_OFFSET),
                                      CTRLS_OFFSET );
                aNewPageSize = Size ( aOutSize.Width() - aNewIconCtrlSize.Width() -
                                      (3*CTRLS_OFFSET),
                                      aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
                                      (3*CTRLS_OFFSET) );
                break;
            case PosRight :
                aNewPagePos = aCtrlOffset;
                aNewPageSize = Size ( aOutSize.Width() - aNewIconCtrlSize.Width() -
                                      (3*aCtrlOffset.X()),
                                      aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
                                      (3*aCtrlOffset.X()) );
                break;
            case PosTop :
                aNewPagePos = Point ( aCtrlOffset.X(), aNewIconCtrlSize.Height() +
                                      (2*aCtrlOffset.X()) );
                aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
                                      aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
                                      aNewIconCtrlSize.Height() - (4*aCtrlOffset.X()) );
                break;
            case PosBottom :
                aNewPagePos = aCtrlOffset;
                aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
                                      aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
                                      aNewIconCtrlSize.Height() - (4*aCtrlOffset.X()) );
                break;
        };

        if ( pData->pPage )
            pData->pPage->SetPosSizePixel ( aNewPagePos, aNewPageSize );
    }

    ////////////////////////////////////////
    // Buttons positionieren
    //
    ULONG nXOffset=0;
    if ( meChoicePos == PosRight )
        nXOffset = aNewIconCtrlSize.Width()+(2*aCtrlOffset.X());

    aResetBtn.SetPosSizePixel ( Point( aOutSize.Width() - nXOffset -
                                       aResetButtonSize.Width()-aCtrlOffset.X(),
                                       aOutSize.Height()-aResetButtonSize.Height()-
                                       aCtrlOffset.X() ),
                               aResetButtonSize );
    // Help-Button
    Size aHelpButtonSize ( bInit ? aDefaultButtonSize :
                                   aHelpBtn.GetSizePixel () );
    aHelpBtn.SetPosSizePixel ( Point( aOutSize.Width()-aResetButtonSize.Width()-
                                      aHelpButtonSize.Width()- nXOffset -
                                      (2*aCtrlOffset.X()),
                                      aOutSize.Height()-aHelpButtonSize.Height()-
                                      aCtrlOffset.X() ),
                               aHelpButtonSize );
    // Cancel-Button
    Size aCancelButtonSize ( bInit ? aDefaultButtonSize :
                                     aCancelBtn.GetSizePixel () );
    aCancelBtn.SetPosSizePixel ( Point( aOutSize.Width()-aCancelButtonSize.Width()-
                                        aResetButtonSize.Width()-aHelpButtonSize.Width()-
                                        (3*aCtrlOffset.X()) -  nXOffset,
                                        aOutSize.Height()-aCancelButtonSize.Height()-
                                        aCtrlOffset.X() ),
                                aCancelButtonSize );
    // OK-Button
    Size aOKButtonSize ( bInit ? aDefaultButtonSize : aOKBtn.GetSizePixel () );
    aOKBtn.SetPosSizePixel ( Point( aOutSize.Width()-aOKButtonSize.Width()-
                                    aCancelButtonSize.Width()-aResetButtonSize.Width()-
                                    aHelpButtonSize.Width()-(4*aCtrlOffset.X())-  nXOffset,
                                    aOutSize.Height()-aOKButtonSize.Height()-aCtrlOffset.X() ),
                            aOKButtonSize );

    Invalidate();
}

void IconChoiceDialog::SetPosSizePages ( USHORT nId )
{
    const Point aCtrlOffset ( LogicToPixel( Point( CTRLS_OFFSET, CTRLS_OFFSET ), MAP_APPFONT ) );
    IconChoicePageData* pData = GetPageData ( nId );

    if ( pData->pPage )
    {
        Size aOutSize ( GetOutputSizePixel() );
        Size aIconCtrlSize ( maIconCtrl.GetSizePixel() );

        Point aNewPagePos;
        Size aNewPageSize;
        switch ( meChoicePos )
        {
            case PosLeft :
                aNewPagePos = Point ( aIconCtrlSize.Width() + (2*aCtrlOffset.X()),
                                      aCtrlOffset.X() );
                aNewPageSize = Size ( aOutSize.Width() - maIconCtrl.GetSizePixel().Width() -
                                      (3*aCtrlOffset.X()),
                                      aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
                                      (3*aCtrlOffset.X()) );
                break;
            case PosRight :
                aNewPagePos = aCtrlOffset;
                aNewPageSize = Size ( aOutSize.Width() - maIconCtrl.GetSizePixel().Width() -
                                      (3*aCtrlOffset.X()),
                                      aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
                                      (3*aCtrlOffset.X()) );
                break;
            case PosTop :
                aNewPagePos = Point ( aCtrlOffset.X(), aIconCtrlSize.Height() +
                                      (2*aCtrlOffset.X()) );
                aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
                                      aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
                                      maIconCtrl.GetSizePixel().Height() - (4*aCtrlOffset.X()) );
                break;
            case PosBottom :
                aNewPagePos = aCtrlOffset;
                aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
                                      aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
                                      maIconCtrl.GetSizePixel().Height() - (4*aCtrlOffset.X()) );
                break;
        };

        pData->pPage->SetPosSizePixel ( aNewPagePos, aNewPageSize );
    }
}

/**********************************************************************
|
| select a page
|
\**********************************************************************/

IMPL_LINK ( IconChoiceDialog , ChosePageHdl_Impl, void *, EMPTYARG )
{
    ULONG nPos;

    SvxIconChoiceCtrlEntry *pEntry = maIconCtrl.GetSelectedEntry ( nPos );

    USHORT *pId = (USHORT*)pEntry->GetUserData ();

    if( *pId != mnCurrentPageId )
    {
        IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
        if ( pData->pPage )
            DeActivatePageImpl();

        SetCurPageId ( *pId );

        ActivatePageImpl();
    }

    return 0L;
}

/**********************************************************************
|
| Button-handler
|
\**********************************************************************/

IMPL_LINK( IconChoiceDialog, OkHdl, Button *, EMPTYARG )
{
    bInOK = TRUE;

    if ( OK_Impl() )
    {
        if ( bModal )
            EndDialog( Ok() );
        else
        {
            Ok();
            Close();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( IconChoiceDialog, ResetHdl, Button *, EMPTYARG )
{
    ResetPageImpl ();

    IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
    DBG_ASSERT( pData, "Id nicht bekannt" );

    if ( pData->bOnDemand )
    {
        // CSet auf AIS hat hier Probleme, daher getrennt
        const SfxItemSet* pSet = &( pData->pPage->GetItemSet() );
        pData->pPage->Reset( *(SfxItemSet*)pSet );
    }
    else
        pData->pPage->Reset( *pSet );


    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( IconChoiceDialog, CancelHdl, Button*, pButton )
{
    Close();

    return 0;
}

/**********************************************************************
|
| call page
|
\**********************************************************************/

void IconChoiceDialog::ActivatePageImpl ()
{
    DBG_ASSERT( maPageList.Count(), "keine Pages angemeldet" );
    IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
    DBG_ASSERT( pData, "Id nicht bekannt" );
    if ( pData )
    {
        if ( !pData->pPage )
        {
            const SfxItemSet* pTmpSet = 0;

            if ( pSet )
            {
                if ( bItemsReset && pSet->GetParent() )
                    pTmpSet = pSet->GetParent();
                else
                    pTmpSet = pSet;
            }

            if ( pTmpSet && !pData->bOnDemand )
                pData->pPage = (pData->fnCreatePage)( this, *pTmpSet );
            else
                pData->pPage = (pData->fnCreatePage)( this, *CreateInputItemSet( mnCurrentPageId ) );

            SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pData->nId ) );
            pData->pPage->SetUserData( aTabPageOpt.GetUserData() );
            SetPosSizePages ( pData->nId );
            PageCreated( mnCurrentPageId, *(pData->pPage) );

            if ( pData->bOnDemand )
                pData->pPage->Reset( (SfxItemSet &)pData->pPage->GetItemSet() );
            else
                pData->pPage->Reset( *pSet );
        }
        else if ( pData->bRefresh )
        {
            pData->pPage->Reset( *pSet );
        }

        pData->bRefresh = FALSE;

        if ( pExampleSet )
            pData->pPage->ActivatePage( *pExampleSet );
    }

    SfxHelpPI *pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->LoadTopic( pData->pPage->GetHelpId() );

    SetHelpId( pData->pPage->GetHelpId() );

    BOOL bReadOnly = pData->pPage->IsReadOnly();
    if ( bReadOnly || bHideResetBtn )
        aResetBtn.Hide();
    else
        aResetBtn.Show();

    Invalidate ();
}

// -----------------------------------------------------------------------

BOOL IconChoiceDialog::DeActivatePageImpl ()
{
    IconChoicePageData *pData = GetPageData ( mnCurrentPageId );

    int nRet = IconChoicePage::LEAVE_PAGE;

    if ( pData )
    {
        IconChoicePage * pPage = pData->pPage;

        if ( !pExampleSet && pPage->HasExchangeSupport() && pSet )
            pExampleSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );

        if ( pSet )
        {
            SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );

            if ( ( IconChoicePage::LEAVE_PAGE & nRet ) == IconChoicePage::LEAVE_PAGE &&
                 aTmp.Count() )
            {
                pExampleSet->Put( aTmp );
                pOutSet->Put( aTmp );
            }
        }
        else
        {
            if ( pPage->HasExchangeSupport() ) //!!!
            {
                if ( !pExampleSet )
                {
                    SfxItemPool* pPool = pPage->GetItemSet().GetPool();
                    pExampleSet =
                        new SfxItemSet( *pPool, GetInputRanges( *pPool ) );
                }
                nRet = pPage->DeactivatePage( pExampleSet );
            }
            else
                nRet = pPage->DeactivatePage( NULL );
        }

        if ( nRet & IconChoicePage::REFRESH_SET )
        {
            pSet = GetRefreshedSet();
            DBG_ASSERT( pSet, "GetRefreshedSet() liefert NULL" );
            // alle Pages als neu zu initialsieren flaggen
            const USHORT nCount = maPageList.Count();

            for ( USHORT i = 0; i < nCount; ++i )
            {
                IconChoicePageData* pObj = (IconChoicePageData*)maPageList.GetObject(i);

                if ( pObj->pPage != pPage ) // eigene Page nicht mehr refreshen
                    pObj->bRefresh = TRUE;
                else
                    pObj->bRefresh = FALSE;
            }
        }
    }

    if ( nRet & IconChoicePage::LEAVE_PAGE )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void IconChoiceDialog::ResetPageImpl ()
{
    IconChoicePageData *pData = GetPageData ( mnCurrentPageId );

    DBG_ASSERT( pData, "Id nicht bekannt" );

    if ( pData->bOnDemand )
    {
        // CSet auf AIS hat hier Probleme, daher getrennt
        const SfxItemSet* pSet = &pData->pPage->GetItemSet();
        pData->pPage->Reset( *(SfxItemSet*)pSet );
    }
    else
        pData->pPage->Reset( *pSet );
}

// -----------------------------------------------------------------------

void IconChoiceDialog::PageCreated( USHORT nId, IconChoicePage &rPage )
{
}

/**********************************************************************
|
| handling itemsets
|
\**********************************************************************/

const USHORT* IconChoiceDialog::GetInputRanges( const SfxItemPool& rPool )
{
    if ( pSet )
    {
        DBG_ERRORFILE( "Set bereits vorhanden!" );
        return pSet->GetRanges();
    }

    if ( pRanges )
        return pRanges;
    SvUShorts aUS( 16, 16 );
    ULONG nCount = maPageList.Count();

    ULONG i;
    for ( i = 0; i < nCount; ++i )
    {
        IconChoicePageData* pData = maPageList.GetObject (i);

        if ( pData->fnGetRanges )
        {
            const USHORT* pTmpRanges = (pData->fnGetRanges)();
            const USHORT* pIter = pTmpRanges;

            USHORT nLen;
            for( nLen = 0; *pIter; ++nLen, ++pIter )
                ;
            aUS.Insert( pTmpRanges, nLen, aUS.Count() );
        }
    }

    // remove double Id's
#ifndef TF_POOLABLE
    if ( rPool.HasMap() )
#endif
    {
        nCount = aUS.Count();

        for ( i = 0; i < nCount; ++i )
            aUS[i] = (USHORT) rPool.GetWhich( aUS[i] );
    }

    // sortieren
    if ( aUS.Count() > 1 )
        qsort( (void*)aUS.GetData(),
               aUS.Count(), sizeof(USHORT), IconcDlgCmpUS_Impl );

    pRanges = new USHORT[aUS.Count() + 1];
    memcpy(pRanges, aUS.GetData(), sizeof(USHORT) * aUS.Count());
    pRanges[aUS.Count()] = 0;

    return pRanges;
}

// -----------------------------------------------------------------------

void IconChoiceDialog::SetInputSet( const SfxItemSet* pInSet )
{
    FASTBOOL bSet = ( pSet != NULL );

    pSet = pInSet;

    if ( !bSet && !pExampleSet && !pOutSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }
}

// -----------------------------------------------------------------------

//  Liefert die Pages, die ihre Sets onDemand liefern, das OutputItemSet.
const SfxItemSet* IconChoiceDialog::GetOutputItemSet ( USHORT nId )
{
    IconChoicePageData * pData = GetPageData ( nId );
    DBG_ASSERT( pData, "TabPage nicht gefunden" );

    if ( pData )
    {
        if ( !pData->pPage )
            return NULL;

        if ( pData->bOnDemand )
            return &pData->pPage->GetItemSet();

        return pOutSet;
    }

    return NULL;
}

// -----------------------------------------------------------------------

int IconChoiceDialog::FillOutputItemSet()
{
    int nRet = IconChoicePage::LEAVE_PAGE;
    if ( OK_Impl() )
        Ok();
    else
        nRet = IconChoicePage::KEEP_PAGE;
    return nRet;
}

// -----------------------------------------------------------------------

SfxItemSet* IconChoiceDialog::CreateInputItemSet( USHORT )
{
    DBG_WARNINGFILE( "CreateInputItemSet nicht implementiert" );

    return 0;
}

/**********************************************************************
|
| start dialog
|
\**********************************************************************/

short IconChoiceDialog::Execute()
{
    if ( !maPageList.Count() )
        return RET_CANCEL;

    Start_Impl();

    return Dialog::Execute();
}

// -----------------------------------------------------------------------

void IconChoiceDialog::Start( BOOL bShow )
{

    aCancelBtn.SetClickHdl( LINK( this, IconChoiceDialog, CancelHdl ) );
    bModal = FALSE;

    Start_Impl();

    if ( bShow )
        Window::Show();

}

// -----------------------------------------------------------------------

void IconChoiceDialog::Start_Impl()
{
    Point aPos;
    USHORT nActPage;

    if ( mnCurrentPageId == 0 || mnCurrentPageId == USHRT_MAX )
        nActPage = maPageList.GetObject(0)->nId;//First()->nId;
    else
        nActPage = mnCurrentPageId;

    // Konfiguration vorhanden?
    SvtViewOptions aTabDlgOpt( E_TABDIALOG, String::CreateFromInt32( nResId ) );

    if ( aTabDlgOpt.Exists() )
    {
        // ggf. Position aus Konfig
        aTabDlgOpt.GetPosition( aPos.X(), aPos.Y() );
        SetPosPixel( aPos );

        // initiale TabPage aus Programm/Hilfe/Konfig
        nActPage = (USHORT)aTabDlgOpt.GetPageID();

        if ( USHRT_MAX != mnCurrentPageId )
            nActPage = mnCurrentPageId;

        if ( GetPageData ( nActPage ) == NULL )
            nActPage = ( (IconChoicePageData*)maPageList.GetObject(0) )->nId;
    }
    else if ( USHRT_MAX != mnCurrentPageId && GetPageData ( mnCurrentPageId ) != NULL )
        nActPage = mnCurrentPageId;

    mnCurrentPageId = nActPage;

    FocusOnIcon( mnCurrentPageId );

    ActivatePageImpl();
}

// -----------------------------------------------------------------------

const SfxItemSet* IconChoiceDialog::GetRefreshedSet()
{
    DBG_ERRORFILE( "GetRefreshedSet nicht implementiert" );
    return 0;
}

/**********************************************************************
|
| tool-methods
|
\**********************************************************************/

IconChoicePageData* IconChoiceDialog::GetPageData ( USHORT nId )
{
    IconChoicePageData *pRet = NULL;
    BOOL bFound = FALSE;

    for ( ULONG i=0; i<maPageList.Count() && !bFound; i++ )
    {
        IconChoicePageData* pData = (IconChoicePageData*)maPageList.GetObject ( i );

        if ( pData->nId == nId )
        {
            pRet = pData;
        }
    }

    return pRet;
}

/**********************************************************************
|
| OK-Status
|
\**********************************************************************/

BOOL IconChoiceDialog::OK_Impl()
{
    IconChoicePage* pPage = GetPageData ( mnCurrentPageId )->pPage;

    BOOL bEnd = !pPage;
    if ( pPage )
    {
        int nRet = IconChoicePage::LEAVE_PAGE;
        if ( pSet )
        {
            SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );
            BOOL bRet = FALSE;

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );

            if ( ( IconChoicePage::LEAVE_PAGE & nRet ) == IconChoicePage::LEAVE_PAGE
                 && aTmp.Count() )
            {
                pExampleSet->Put( aTmp );
                pOutSet->Put( aTmp );
            }
            else if ( bRet )
                bModified |= TRUE;
        }
        else
            nRet = pPage->DeactivatePage( NULL );
        bEnd = nRet;
    }

    return bEnd;
}

// -----------------------------------------------------------------------

short IconChoiceDialog::Ok()
{
    bInOK = TRUE;

    if ( !pOutSet )
    {
        if ( !pExampleSet && pSet )
            pOutSet = pSet->Clone( FALSE ); // ohne Items
        else if ( pExampleSet )
            pOutSet = new SfxItemSet( *pExampleSet );
    }
    BOOL bModified = FALSE;

    const ULONG nCount = maPageList.Count();

    for ( ULONG i = 0; i < nCount; ++i )
    {
        IconChoicePageData* pData = GetPageData ( i );

        IconChoicePage* pPage = pData->pPage;

        if ( pPage )
        {
            if ( pData->bOnDemand )
            {
                SfxItemSet& rSet = (SfxItemSet&)pPage->GetItemSet();
                rSet.ClearItem();
                bModified |= pPage->FillItemSet( rSet );
            }
            else if ( pSet && !pPage->HasExchangeSupport() )
            {
                SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

                if ( pPage->FillItemSet( aTmp ) )
                {
                    bModified |= TRUE;
                    pExampleSet->Put( aTmp );
                    pOutSet->Put( aTmp );
                }
            }
        }
    }

    if ( bModified || ( pOutSet && pOutSet->Count() > 0 ) )
        bModified |= TRUE;

    return bModified ? RET_OK : RET_CANCEL;
}

// -----------------------------------------------------------------------

BOOL IconChoiceDialog::IsInOK() const
{
    return bInOK;
}

// -----------------------------------------------------------------------

void IconChoiceDialog::FocusOnIcon( USHORT nId )
{
    // set focus to icon for the current visible page
    for ( int i=0; i<maIconCtrl.GetEntryCount(); i++)
    {
        SvxIconChoiceCtrlEntry* pEntry = maIconCtrl.GetEntry ( i );
        USHORT* pUserData = (USHORT*) pEntry->GetUserData();

        if ( pUserData && *pUserData == nId )
            pEntry->SetFocus( TRUE );
        else
            pEntry->SetFocus( FALSE );
    }
}

/**********************************************************************
|
| D & D
|
\**********************************************************************/

BOOL IconChoiceDialog::QueryDrop( DropEvent& rDEvt )
{
    return ModalDialog::QueryDrop( rDEvt );
}

// -----------------------------------------------------------------------

BOOL IconChoiceDialog::Drop( const DropEvent& rDEvt )
{
    return ModalDialog::Drop( rDEvt );
}

