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

#include <sfx2/app.hxx>
#include <tools/rc.h>
#include <tools/shl.hxx>

#include <dialmgr.hxx>

#include "iconcdlg.hxx"

#include "helpid.hrc"
#include <cuires.hrc>
#include <unotools/viewoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>

using ::std::vector;

// some stuff for easier changes for SvtViewOptions
static const sal_Char*      pViewOptDataName = "dialog data";
#define VIEWOPT_DATANAME    OUString::createFromAscii( pViewOptDataName )

static inline void SetViewOptUserItem( SvtViewOptions& rOpt, const OUString& rData )
{
    rOpt.SetUserItem( VIEWOPT_DATANAME, css::uno::makeAny( rData ) );
}

static inline OUString GetViewOptUserItem( const SvtViewOptions& rOpt )
{
    ::com::sun::star::uno::Any aAny( rOpt.GetUserItem( VIEWOPT_DATANAME ) );
    OUString aUserData;
    aAny >>= aUserData;

    return aUserData;
}




// Class IconChoicePage



/**********************************************************************
|
| Ctor / Dtor
|
\**********************************************************************/

IconChoicePage::IconChoicePage( Window *pParent, const OString& rID,
                                const OUString& rUIXMLDescription,
                                const SfxItemSet &rAttrSet )
:   TabPage                   ( pParent, rID, rUIXMLDescription ),
    pSet                      ( &rAttrSet ),
    bHasExchangeSupport       ( false ),
    pDialog                   ( NULL )
{
    SetStyle ( GetStyle()  | WB_DIALOGCONTROL | WB_HIDE );
}



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



bool IconChoicePage::IsReadOnly() const
{
    return false;
}



bool IconChoicePage::QueryClose()
{
    return true;
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
        EnableChildTransparentMode( true );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( true );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( false );
        SetParentClipMode( 0 );
        SetPaintTransparent( false );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}



void IconChoicePage::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}



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



// Class IconChoiceDialog



/**********************************************************************
|
| Ctor / Dtor
|
\**********************************************************************/

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvtIconChoiceCtrl(Window *pParent, VclBuilder::stringmap &)
{
    return new SvtIconChoiceCtrl(pParent, WB_3DLOOK | WB_ICON | WB_BORDER |
                            WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME |
                            WB_NODRAGSELECTION | WB_TABSTOP);
}

IconChoiceDialog::IconChoiceDialog ( Window* pParent, const OString& rID,
                                     const OUString& rUIXMLDescription,
                                     const EIconChoicePos ePos,
                                     const SfxItemSet *pItemSet )
:   ModalDialog         ( pParent, rID, rUIXMLDescription ),
    meChoicePos     ( ePos ),
    mnCurrentPageId ( USHRT_MAX ),

    pSet            ( pItemSet ),
    pOutSet         ( NULL ),
    pExampleSet     ( NULL ),
    pRanges         ( NULL ),
    rId             ( rID ),

    bHideResetBtn   ( false ),
    bModal          ( false ),
    bInOK           ( false ),
    bItemsReset     ( false )
{
    get(m_pOKBtn, "ok");
    get(m_pCancelBtn, "cancel");
    get(m_pHelpBtn, "help");
    get(m_pResetBtn, "back");
    get(m_pIconCtrl, "icon_control");
    get(m_pTabContainer, "tab");

    m_pIconCtrl->SetStyle (WB_3DLOOK | WB_ICON | WB_BORDER | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN );
    SetCtrlPos ( meChoicePos );
    m_pIconCtrl->SetClickHdl ( LINK ( this, IconChoiceDialog , ChosePageHdl_Impl ) );
    m_pIconCtrl->Show();
    m_pIconCtrl->SetChoiceWithCursor ( true );
    m_pIconCtrl->SetSelectionMode( SINGLE_SELECTION );
    m_pIconCtrl->SetHelpId( HID_ICCDIALOG_CHOICECTRL );

    // ItemSet
    if ( pSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }

    // Buttons
    m_pOKBtn->SetClickHdl   ( LINK( this, IconChoiceDialog, OkHdl ) );
    m_pOKBtn->SetHelpId( HID_ICCDIALOG_OK_BTN );
    m_pCancelBtn->SetHelpId( HID_ICCDIALOG_CANCEL_BTN );
    m_pResetBtn->SetClickHdl( LINK( this, IconChoiceDialog, ResetHdl ) );
    m_pResetBtn->SetText( CUI_RESSTR(RID_SVXSTR_ICONCHOICEDLG_RESETBUT) );
    m_pResetBtn->SetHelpId( HID_ICCDIALOG_RESET_BTN );
    m_pOKBtn->Show();
    m_pCancelBtn->Show();
    m_pHelpBtn->Show();
    m_pResetBtn->Show();

    SetPosSizeCtrls ( true );
}


IconChoiceDialog ::~IconChoiceDialog ()
{
    // save configuration at INI-Manager
    // and remove pages
    //SvtViewOptions aTabDlgOpt( E_TABDIALOG, rId );
    //aTabDlgOpt.SetWindowState(OStringToOUString(GetWindowState((WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y | WINDOWSTATE_MASK_STATE | WINDOWSTATE_MASK_MINIMIZED)), RTL_TEXTENCODING_ASCII_US));
    //aTabDlgOpt.SetPageID( mnCurrentPageId );

    for ( size_t i = 0, nCount = maPageList.size(); i < nCount; ++i )
    {
        IconChoicePageData* pData = maPageList[ i ];

        if ( pData->pPage )
        {
            pData->pPage->FillUserData();
            OUString aPageData(pData->pPage->GetUserData());
            if ( !aPageData.isEmpty() )
            {
                //SvtViewOptions aTabPageOpt( E_TABPAGE, OUString::number(pData->nId) );

                //SetViewOptUserItem( aTabPageOpt, aPageData );
            }

            if ( pData->bOnDemand )
                delete (SfxItemSet*)&pData->pPage->GetItemSet();
            delete pData->pPage;
        }
        delete pData;
    }

    // remove Userdata from Icons
    for ( sal_uLong i=0; i < m_pIconCtrl->GetEntryCount(); i++)
    {
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry ( i );
        sal_uInt16* pUserData = (sal_uInt16*) pEntry->GetUserData();
        delete pUserData;
    }


    if ( pRanges )
        delete pRanges;
    if ( pOutSet )
        delete pOutSet;
}

/**********************************************************************
|
| add new page
|
\**********************************************************************/

SvxIconChoiceCtrlEntry* IconChoiceDialog::AddTabPage(
    sal_uInt16          nId,
    const OUString&   rIconText,
    const Image&    rChoiceIcon,
    CreatePage      pCreateFunc /* != 0 */,
    GetPageRanges   pRangesFunc /* darf 0 sein */,
    bool            bItemsOnDemand,
    sal_uLong           /*nPos*/
)
{
    IconChoicePageData* pData = new IconChoicePageData ( nId, pCreateFunc,
                                                         pRangesFunc,
                                                         bItemsOnDemand );
    maPageList.push_back( pData );

    pData->fnGetRanges = pRangesFunc;
    pData->bOnDemand = bItemsOnDemand;

    sal_uInt16 *pId = new sal_uInt16 ( nId );
    SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->InsertEntry( rIconText, rChoiceIcon );
    pEntry->SetUserData ( (void*) pId );
    return pEntry;
}

/**********************************************************************
|
| Paint-method
|
\**********************************************************************/

void IconChoiceDialog::Paint( const Rectangle& rRect )
{
    Dialog::Paint ( rRect );

    for ( size_t i = 0; i < maPageList.size(); i++ )
    {
        IconChoicePageData* pData = maPageList[ i ];

        if ( pData->nId == mnCurrentPageId )
        {
            ShowPageImpl ( pData );
        }
        else
        {
            HidePageImpl ( pData );
        }
    }
}

EIconChoicePos IconChoiceDialog::SetCtrlPos( const EIconChoicePos& rPos )
{
    WinBits aWinBits = m_pIconCtrl->GetStyle ();

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
    }
    m_pIconCtrl->SetStyle ( aWinBits );

    SetPosSizeCtrls();


    EIconChoicePos eOldPos = meChoicePos;
    meChoicePos = rPos;

    return eOldPos;
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



void IconChoiceDialog::HidePageImpl ( IconChoicePageData* pData )
{
    if ( pData->pPage )
        pData->pPage->Hide();
}



void IconChoiceDialog::ShowPage( sal_uInt16 nId )
{
    bool bInvalidate = GetCurPageId() != nId;
    SetCurPageId( nId );
    ActivatePageImpl( );
    if(bInvalidate)
        Invalidate();

    // IA2 CWS. MT: I guess we want the event now, and not in Paint()?
    IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
    if(pData)
        ShowPageImpl ( pData );
}

/**********************************************************************
|
| Resize Dialog
|
\**********************************************************************/

#define ICONCTRL_WIDTH_PIXEL       110
#define ICONCTRL_HEIGHT_PIXEL       75

void IconChoiceDialog::Resize()
{
    Dialog::Resize ();

    if ( IsReallyVisible() )
    {
        SetPosSizeCtrls ();
    }
}

void IconChoiceDialog::SetPosSizeCtrls ( bool bInit )
{
    const Point aCtrlOffset ( LogicToPixel( Point( CTRLS_OFFSET, CTRLS_OFFSET ), MAP_APPFONT ) );
    Size aOutSize ( GetOutputSizePixel() );


    // Button-Defaults

    Size aDefaultButtonSize = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );

    // Reset-Button
    Size aResetButtonSize ( bInit ? aDefaultButtonSize :
                                    m_pResetBtn->GetSizePixel () );


    // IconChoiceCtrl resizen & positionieren

    SvtTabAppearanceCfg aCfg;
    const long nDefaultWidth = (aCfg.GetScaleFactor() * ICONCTRL_WIDTH_PIXEL) / 100;
    const long nDefaultHeight = (aCfg.GetScaleFactor() * ICONCTRL_HEIGHT_PIXEL) / 100;

    Size aNewIconCtrlSize  ( nDefaultWidth,
                             aOutSize.Height()-(2*aCtrlOffset.X()) );
    Point aIconCtrlPos;
    switch ( meChoicePos )
    {
        case PosLeft :
            aIconCtrlPos = aCtrlOffset;
            aNewIconCtrlSize = Size ( nDefaultWidth,
                                      aOutSize.Height()-(2*aCtrlOffset.X()) );
            break;
        case PosRight :
            aIconCtrlPos = Point ( aOutSize.Width() - nDefaultWidth -
                                   aCtrlOffset.X(), aCtrlOffset.X() );
            aNewIconCtrlSize = Size ( nDefaultWidth,
                                      aOutSize.Height()-(2*aCtrlOffset.X()) );
            break;
        case PosTop :
            aIconCtrlPos = aCtrlOffset;
            aNewIconCtrlSize = Size ( aOutSize.Width()-(2*aCtrlOffset.X()),
                                      nDefaultHeight );
            break;
        case PosBottom :
            aIconCtrlPos = Point ( aCtrlOffset.X(), aOutSize.Height() -
                                   aResetButtonSize.Height() - (2*aCtrlOffset.X()) -
                                   nDefaultHeight );
            aNewIconCtrlSize = Size ( aOutSize.Width()-(2*aCtrlOffset.X()),
                                      nDefaultHeight );
            break;
    }
    m_pIconCtrl->SetPosSizePixel ( aIconCtrlPos, aNewIconCtrlSize );
    m_pIconCtrl->ArrangeIcons();


    // resize & position the pages

    for ( size_t i = 0; i < maPageList.size(); i++ )
    {
        IconChoicePageData* pData = maPageList[ i ];

        Point aNewPagePos;
        Size aNewPageSize;
        switch ( meChoicePos )
        {
            case PosLeft :
                aNewPagePos = Point ( aNewIconCtrlSize.Width() + (2*CTRLS_OFFSET),
                                      CTRLS_OFFSET );
                aNewPageSize = Size ( aOutSize.Width() - aNewIconCtrlSize.Width() -
                                      (3*CTRLS_OFFSET),
                                      aOutSize.Height() - m_pOKBtn->GetSizePixel().Height() -
                                      (3*CTRLS_OFFSET) );
                break;
            case PosRight :
                aNewPagePos = aCtrlOffset;
                aNewPageSize = Size ( aOutSize.Width() - aNewIconCtrlSize.Width() -
                                      (3*aCtrlOffset.X()),
                                      aOutSize.Height() - m_pOKBtn->GetSizePixel().Height() -
                                      (3*aCtrlOffset.X()) );
                break;
            case PosTop :
                aNewPagePos = Point ( aCtrlOffset.X(), aNewIconCtrlSize.Height() +
                                      (2*aCtrlOffset.X()) );
                aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
                                      aOutSize.Height() - m_pOKBtn->GetSizePixel().Height() -
                                      aNewIconCtrlSize.Height() - (4*aCtrlOffset.X()) );
                break;
            case PosBottom :
                aNewPagePos = aCtrlOffset;
                aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
                                      aOutSize.Height() - m_pOKBtn->GetSizePixel().Height() -
                                      aNewIconCtrlSize.Height() - (4*aCtrlOffset.X()) );
                break;
        }

        if ( pData->pPage )
            pData->pPage->SetPosSizePixel ( aNewPagePos, aNewPageSize );
    }


    // position the buttons

    sal_uLong nXOffset=0;
    if ( meChoicePos == PosRight )
        nXOffset = aNewIconCtrlSize.Width()+(2*aCtrlOffset.X());

    m_pResetBtn->SetPosSizePixel ( Point( aOutSize.Width() - nXOffset -
                                       aResetButtonSize.Width()-aCtrlOffset.X(),
                                       aOutSize.Height()-aResetButtonSize.Height()-
                                       aCtrlOffset.X() ),
                               aResetButtonSize );
    // Help-Button
    Size aHelpButtonSize ( bInit ? aDefaultButtonSize :
                                   m_pHelpBtn->GetSizePixel () );
    m_pHelpBtn->SetPosSizePixel ( Point( aOutSize.Width()-aResetButtonSize.Width()-
                                      aHelpButtonSize.Width()- nXOffset -
                                      (2*aCtrlOffset.X()),
                                      aOutSize.Height()-aHelpButtonSize.Height()-
                                      aCtrlOffset.X() ),
                               aHelpButtonSize );
    // Cancel-Button
    Size aCancelButtonSize ( bInit ? aDefaultButtonSize :
                                     m_pCancelBtn->GetSizePixel () );
    m_pCancelBtn->SetPosSizePixel ( Point( aOutSize.Width()-aCancelButtonSize.Width()-
                                        aResetButtonSize.Width()-aHelpButtonSize.Width()-
                                        (3*aCtrlOffset.X()) -  nXOffset,
                                        aOutSize.Height()-aCancelButtonSize.Height()-
                                        aCtrlOffset.X() ),
                                aCancelButtonSize );
    // OK-Button
    Size aOKButtonSize ( bInit ? aDefaultButtonSize : m_pOKBtn->GetSizePixel () );
    m_pOKBtn->SetPosSizePixel ( Point( aOutSize.Width()-aOKButtonSize.Width()-
                                    aCancelButtonSize.Width()-aResetButtonSize.Width()-
                                    aHelpButtonSize.Width()-(4*aCtrlOffset.X())-  nXOffset,
                                    aOutSize.Height()-aOKButtonSize.Height()-aCtrlOffset.X() ),
                            aOKButtonSize );

    Invalidate();
}

void IconChoiceDialog::SetPosSizePages ( sal_uInt16 nId )
{
    const Point aCtrlOffset ( LogicToPixel( Point( CTRLS_OFFSET, CTRLS_OFFSET ), MAP_APPFONT ) );
    IconChoicePageData* pData = GetPageData ( nId );

    if ( pData->pPage )
    {
        Size aOutSize ( GetOutputSizePixel() );
        Size aIconCtrlSize ( m_pIconCtrl->GetSizePixel() );

        Point aNewPagePos;
        Size aNewPageSize;
        switch ( meChoicePos )
        {
            case PosLeft :
                aNewPagePos = Point ( aIconCtrlSize.Width() + (2*aCtrlOffset.X()),
                                      aCtrlOffset.X() );
                aNewPageSize = Size ( aOutSize.Width() - m_pIconCtrl->GetSizePixel().Width() -
                                      (3*aCtrlOffset.X()),
                                      aOutSize.Height() - m_pOKBtn->GetSizePixel().Height() -
                                      (3*aCtrlOffset.X()) );
                break;
            case PosRight :
                aNewPagePos = aCtrlOffset;
                aNewPageSize = Size ( aOutSize.Width() - m_pIconCtrl->GetSizePixel().Width() -
                                      (3*aCtrlOffset.X()),
                                      aOutSize.Height() - m_pOKBtn->GetSizePixel().Height() -
                                      (3*aCtrlOffset.X()) );
                break;
            case PosTop :
                aNewPagePos = Point ( aCtrlOffset.X(), aIconCtrlSize.Height() +
                                      (2*aCtrlOffset.X()) );
                aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
                                      aOutSize.Height() - m_pOKBtn->GetSizePixel().Height() -
                                      m_pIconCtrl->GetSizePixel().Height() - (4*aCtrlOffset.X()) );
                break;
            case PosBottom :
                aNewPagePos = aCtrlOffset;
                aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
                                      aOutSize.Height() - m_pOKBtn->GetSizePixel().Height() -
                                      m_pIconCtrl->GetSizePixel().Height() - (4*aCtrlOffset.X()) );
                break;
        }

        pData->pPage->SetPosSizePixel ( aNewPagePos, aNewPageSize );
    }
}

/**********************************************************************
|
| select a page
|
\**********************************************************************/

IMPL_LINK_NOARG(IconChoiceDialog , ChosePageHdl_Impl)
{
    sal_uLong nPos;

    SvxIconChoiceCtrlEntry *pEntry = m_pIconCtrl->GetSelectedEntry ( nPos );
    if ( !pEntry )
        pEntry = m_pIconCtrl->GetCursor( );

    sal_uInt16 *pId = (sal_uInt16*)pEntry->GetUserData ();

    if( *pId != mnCurrentPageId )
    {
        IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
        if ( pData->pPage )
            DeActivatePageImpl();

        SetCurPageId ( *pId );

        ActivatePageImpl();
        Invalidate();
    }

    return 0L;
}

/**********************************************************************
|
| Button-handler
|
\**********************************************************************/

IMPL_LINK_NOARG(IconChoiceDialog, OkHdl)
{
    bInOK = true;

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



IMPL_LINK_NOARG(IconChoiceDialog, ResetHdl)
{
    ResetPageImpl ();

    IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
    DBG_ASSERT( pData, "Id nicht bekannt" );

    if ( pData->bOnDemand )
    {
        // CSet on AIS has problems here, therefore separated
        const SfxItemSet* _pSet = &( pData->pPage->GetItemSet() );
        pData->pPage->Reset( *(SfxItemSet*)_pSet );
    }
    else
        pData->pPage->Reset( *pSet );


    return 0;
}



IMPL_LINK_NOARG(IconChoiceDialog, CancelHdl)
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
    DBG_ASSERT( !maPageList.empty(), "keine Pages angemeldet" );
    IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
    DBG_ASSERT( pData, "Id nicht bekannt" );
    bool bReadOnly = false;
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
                pData->pPage = (pData->fnCreatePage)( m_pTabContainer, this, *pTmpSet );
            else
                pData->pPage = (pData->fnCreatePage)( m_pTabContainer, this, *CreateInputItemSet( mnCurrentPageId ) );

            //SvtViewOptions aTabPageOpt( E_TABPAGE, OUString::number(pData->nId) );
            //pData->pPage->SetUserData( GetViewOptUserItem( aTabPageOpt ) );
            SetPosSizePages ( pData->nId );

            if ( pData->bOnDemand )
                pData->pPage->Reset( (SfxItemSet &)pData->pPage->GetItemSet() );
            else
                pData->pPage->Reset( *pSet );

            PageCreated( mnCurrentPageId, *pData->pPage );
        }
        else if ( pData->bRefresh )
        {
            pData->pPage->Reset( *pSet );
        }

        pData->bRefresh = false;

        if ( pExampleSet )
            pData->pPage->ActivatePage( *pExampleSet );
        SetHelpId( pData->pPage->GetHelpId() );
        bReadOnly = pData->pPage->IsReadOnly();
    }


    if ( bReadOnly || bHideResetBtn )
        m_pResetBtn->Hide();
    else
        m_pResetBtn->Show();

}



bool IconChoiceDialog::DeActivatePageImpl ()
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
            // flag all pages to be newly initialized
            for ( size_t i = 0, nCount = maPageList.size(); i < nCount; ++i )
            {
                IconChoicePageData* pObj = maPageList[ i ];
                if ( pObj->pPage != pPage )
                    pObj->bRefresh = true;
                else
                    pObj->bRefresh = false;
            }
        }
    }

    if ( nRet & IconChoicePage::LEAVE_PAGE )
        return true;
    else
        return false;
}



void IconChoiceDialog::ResetPageImpl ()
{
    IconChoicePageData *pData = GetPageData ( mnCurrentPageId );

    DBG_ASSERT( pData, "Id nicht bekannt" );

    if ( pData->bOnDemand )
    {
        // CSet on AIS has problems here, therefore separated
        const SfxItemSet* _pSet = &pData->pPage->GetItemSet();
        pData->pPage->Reset( *(SfxItemSet*)_pSet );
    }
    else
        pData->pPage->Reset( *pSet );
}

/**********************************************************************
|
| handling itemsets
|
\**********************************************************************/

const sal_uInt16* IconChoiceDialog::GetInputRanges( const SfxItemPool& rPool )
{
    if ( pSet )
    {
        SAL_WARN( "cui.dialogs", "Set does already exist!" );
        return pSet->GetRanges();
    }

    if ( pRanges )
        return pRanges;
    std::vector<sal_uInt16> aUS;

    size_t nCount = maPageList.size();
    for ( size_t i = 0; i < nCount; ++i )
    {
        IconChoicePageData* pData = maPageList[ i ];
        if ( pData->fnGetRanges )
        {
            const sal_uInt16* pTmpRanges = (pData->fnGetRanges)();
            const sal_uInt16* pIter = pTmpRanges;

            sal_uInt16 nLen;
            for( nLen = 0; *pIter; ++nLen, ++pIter )
                ;
            aUS.insert( aUS.end(), pTmpRanges, pTmpRanges + nLen );
        }
    }

    // remove double Id's
    {
        nCount = aUS.size();
        for ( size_t i = 0; i < nCount; ++i )
            aUS[i] = rPool.GetWhich( aUS[i] );
    }

    if ( aUS.size() > 1 )
    {
        std::sort( aUS.begin(), aUS.end() );
    }

    pRanges = new sal_uInt16[aUS.size() + 1];
    std::copy( aUS.begin(), aUS.end(), pRanges );
    pRanges[aUS.size()] = 0;

    return pRanges;
}



void IconChoiceDialog::SetInputSet( const SfxItemSet* pInSet )
{
    bool bSet = ( pSet != NULL );

    pSet = pInSet;

    if ( !bSet && !pExampleSet && !pOutSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
    }
}



void IconChoiceDialog::PageCreated( sal_uInt16 /*nId*/, IconChoicePage& /*rPage*/ )
{
    // not interested in
}



SfxItemSet* IconChoiceDialog::CreateInputItemSet( sal_uInt16 )
{
    SAL_INFO( "cui.dialogs", "CreateInputItemSet not implemented" );

    return 0;
}

/**********************************************************************
|
| start dialog
|
\**********************************************************************/

short IconChoiceDialog::Execute()
{
    if ( maPageList.empty() )
        return RET_CANCEL;

    Start_Impl();

    return Dialog::Execute();
}



void IconChoiceDialog::Start( bool bShow )
{

    m_pCancelBtn->SetClickHdl( LINK( this, IconChoiceDialog, CancelHdl ) );
    bModal = false;

    Start_Impl();

    if ( bShow )
        Window::Show();

}



bool IconChoiceDialog::QueryClose()
{
    bool bRet = true;
    for ( size_t i = 0, nCount = maPageList.size(); i < nCount; ++i )
    {
        IconChoicePageData* pData = maPageList[i ];
        if ( pData->pPage && !pData->pPage->QueryClose() )
        {
            bRet = false;
            break;
        }
    }
    return bRet;
}



void IconChoiceDialog::Start_Impl()
{
    sal_uInt16 nActPage;

    if ( mnCurrentPageId == 0 || mnCurrentPageId == USHRT_MAX )
        nActPage = maPageList.front()->nId;
    else
        nActPage = mnCurrentPageId;

    // configuration existing?
    //SvtViewOptions aTabDlgOpt( E_TABDIALOG, rId );

    /*if ( aTabDlgOpt.Exists() )
    {
        // possibly position from config
        SetWindowState(OUStringToOString(aTabDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US));

        // initial TabPage from program/help/config
        nActPage = (sal_uInt16)aTabDlgOpt.GetPageID();

        if ( USHRT_MAX != mnCurrentPageId )
            nActPage = mnCurrentPageId;

        if ( GetPageData ( nActPage ) == NULL )
            nActPage = maPageList.front()->nId;
    }*/
    //else if ( USHRT_MAX != mnCurrentPageId && GetPageData ( mnCurrentPageId ) != NULL )
        nActPage = mnCurrentPageId;

    mnCurrentPageId = nActPage;

    FocusOnIcon( mnCurrentPageId );

    ActivatePageImpl();
}



const SfxItemSet* IconChoiceDialog::GetRefreshedSet()
{
    SAL_WARN( "cui.dialogs", "GetRefreshedSet not implemented" );
    return 0;
}

/**********************************************************************
|
| tool-methods
|
\**********************************************************************/

IconChoicePageData* IconChoiceDialog::GetPageData ( sal_uInt16 nId )
{
    IconChoicePageData *pRet = NULL;
    for ( size_t i=0; i < maPageList.size(); i++ )
    {
        IconChoicePageData* pData = maPageList[ i ];
        if ( pData->nId == nId )
        {
            pRet = pData;
            break;
        }
    }
    return pRet;
}

/**********************************************************************
|
| OK-Status
|
\**********************************************************************/

bool IconChoiceDialog::OK_Impl()
{
    IconChoicePage* pPage = GetPageData ( mnCurrentPageId )->pPage;

    bool bEnd = !pPage;
    if ( pPage )
    {
        int nRet = IconChoicePage::LEAVE_PAGE;
        if ( pSet )
        {
            SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );

            if ( ( IconChoicePage::LEAVE_PAGE & nRet ) == IconChoicePage::LEAVE_PAGE
                 && aTmp.Count() )
            {
                pExampleSet->Put( aTmp );
                pOutSet->Put( aTmp );
            }
        }
        else
            nRet = pPage->DeactivatePage( NULL );
        bEnd = nRet;
    }

    return bEnd;
}



short IconChoiceDialog::Ok()
{
    bInOK = true;

    if ( !pOutSet )
    {
        if ( !pExampleSet && pSet )
            pOutSet = pSet->Clone( false ); // without items
        else if ( pExampleSet )
            pOutSet = new SfxItemSet( *pExampleSet );
    }
    bool _bModified = false;

    for ( size_t i = 0, nCount = maPageList.size(); i < nCount; ++i )
    {
        IconChoicePageData* pData = GetPageData ( i );

        IconChoicePage* pPage = pData->pPage;

        if ( pPage )
        {
            if ( pData->bOnDemand )
            {
                SfxItemSet& rSet = (SfxItemSet&)pPage->GetItemSet();
                rSet.ClearItem();
                _bModified |= pPage->FillItemSet( &rSet );
            }
            else if ( pSet && !pPage->HasExchangeSupport() )
            {
                SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

                if ( pPage->FillItemSet( &aTmp ) )
                {
                    _bModified |= true;
                    pExampleSet->Put( aTmp );
                    pOutSet->Put( aTmp );
                }
            }
        }
    }

    if ( _bModified || ( pOutSet && pOutSet->Count() > 0 ) )
        _bModified |= true;

    return _bModified ? RET_OK : RET_CANCEL;
}



void IconChoiceDialog::FocusOnIcon( sal_uInt16 nId )
{
    // set focus to icon for the current visible page
    for ( sal_uInt16 i=0; i<m_pIconCtrl->GetEntryCount(); i++)
    {
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry ( i );
        sal_uInt16* pUserData = (sal_uInt16*) pEntry->GetUserData();

        if ( pUserData && *pUserData == nId )
        {
            m_pIconCtrl->SetCursor( pEntry );
            break;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
