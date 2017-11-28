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

#include <dialmgr.hxx>

#include <iconcdlg.hxx>

#include <helpids.h>
#include <unotools/viewoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

using std::vector;

/**********************************************************************
|
| Ctor / Dtor
|
\**********************************************************************/

IconChoicePage::IconChoicePage( vcl::Window *pParent, const OString& rID,
                                const OUString& rUIXMLDescription,
                                const SfxItemSet* pItemSet )
:   TabPage                   ( pParent, rID, rUIXMLDescription ),
    pSet                      ( pItemSet ),
    bHasExchangeSupport       ( false )
{
    SetStyle ( GetStyle()  | WB_DIALOGCONTROL | WB_HIDE );
}


IconChoicePage::~IconChoicePage()
{
    disposeOnce();
}

/**********************************************************************
|
| Activate / Deactivate
|
\**********************************************************************/

void IconChoicePage::ActivatePage( const SfxItemSet& )
{
}


DeactivateRC IconChoicePage::DeactivatePage( SfxItemSet* )
{
    return DeactivateRC::LeavePage;
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
    vcl::Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode();
        SetParentClipMode( ParentClipMode::NoClip );
        SetPaintTransparent( true );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( false );
        SetParentClipMode();
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

    if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings();
        Invalidate();
    }
}


void IconChoicePage::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
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

VCL_BUILDER_FACTORY_ARGS(SvtIconChoiceCtrl,
                         WB_3DLOOK | WB_ICON | WB_BORDER |
                         WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME |
                         WB_NODRAGSELECTION | WB_TABSTOP);

IconChoiceDialog::IconChoiceDialog ( vcl::Window* pParent, const OUString& rID,
                                     const OUString& rUIXMLDescription )
:   SfxModalDialog         ( pParent, rID, rUIXMLDescription ),
    mnCurrentPageId ( USHRT_MAX ),

    pSet            ( nullptr ),
    pOutSet         ( nullptr ),
    pExampleSet     ( nullptr ),
    pRanges         ( nullptr )
{
    get(m_pOKBtn, "ok");
    get(m_pApplyBtn, "apply");
    get(m_pCancelBtn, "cancel");
    get(m_pHelpBtn, "help");
    get(m_pResetBtn, "reset");
    get(m_pIconCtrl, "icon_control");
    get(m_pTabContainer, "tab");

    SetCtrlStyle();
    m_pIconCtrl->SetClickHdl ( LINK ( this, IconChoiceDialog , ChosePageHdl_Impl ) );
    m_pIconCtrl->Show();
    m_pIconCtrl->SetChoiceWithCursor();
    m_pIconCtrl->SetSelectionMode( SelectionMode::Single );
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
    m_pApplyBtn->SetClickHdl   ( LINK( this, IconChoiceDialog, ApplyHdl ) );
    //m_pApplyBtn->SetHelpId( HID_ICCDIALOG_APPLY_BTN );
    m_pCancelBtn->SetHelpId( HID_ICCDIALOG_CANCEL_BTN );
    m_pResetBtn->SetClickHdl( LINK( this, IconChoiceDialog, ResetHdl ) );
    m_pResetBtn->SetHelpId( HID_ICCDIALOG_RESET_BTN );
    m_pOKBtn->Show();
    m_pApplyBtn->Show();
    m_pCancelBtn->Show();
    m_pHelpBtn->Show();
    m_pResetBtn->Show();
}

IconChoiceDialog ::~IconChoiceDialog ()
{
    disposeOnce();
}

void IconChoiceDialog::dispose()
{
    // save configuration at INI-Manager
    // and remove pages
    //SvtViewOptions aTabDlgOpt( EViewType::TabDialog, rId );
    //aTabDlgOpt.SetWindowState(OStringToOUString(GetWindowState((WindowStateMask::X | WindowStateMask::Y | WindowStateMask::State | WindowStateMask::Minimized)), RTL_TEXTENCODING_ASCII_US));
    //aTabDlgOpt.SetPageID( mnCurrentPageId );

    for (IconChoicePageData* pData : maPageList)
    {
        if ( pData->pPage )
        {
            OUString aPageData(pData->pPage->GetUserData());
            if ( !aPageData.isEmpty() )
            {
                //SvtViewOptions aTabPageOpt( EViewType::TabPage, OUString::number(pData->nId) );

                //SetViewOptUserItem( aTabPageOpt, aPageData );
            }

            pData->pPage.disposeAndClear();
        }
        delete pData;
    }
    maPageList.clear();

    if (m_pIconCtrl)
    {
        // remove Userdata from Icons
        for ( sal_Int32 i=0; i < m_pIconCtrl->GetEntryCount(); i++)
        {
            SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( i );
            delete static_cast<sal_uInt16*>(pEntry->GetUserData());
        }
    }

    delete pRanges;
    pRanges = nullptr;
    delete pOutSet;
    pOutSet = nullptr;

    m_pIconCtrl.clear();
    m_pOKBtn.clear();
    m_pApplyBtn.clear();
    m_pCancelBtn.clear();
    m_pHelpBtn.clear();
    m_pResetBtn.clear();
    m_pTabContainer.clear();
    SfxModalDialog::dispose();
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
    CreatePage      pCreateFunc /* != 0 */
)
{
    IconChoicePageData* pData = new IconChoicePageData ( nId, pCreateFunc );
    maPageList.push_back( pData );

    sal_uInt16 *pId = new sal_uInt16 ( nId );
    SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->InsertEntry( rIconText, rChoiceIcon );
    pEntry->SetUserData ( static_cast<void*>(pId) );
    return pEntry;
}

void IconChoiceDialog::SetCtrlStyle()
{
    WinBits const aWinBits = WB_3DLOOK | WB_ICON | WB_BORDER | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN | WB_ALIGN_LEFT | WB_NOHSCROLL;
    m_pIconCtrl->SetStyle(aWinBits);
    m_pIconCtrl->ArrangeIcons();
}

/**********************************************************************
|
| Show / Hide page or button
|
\**********************************************************************/

void IconChoiceDialog::ShowPageImpl ( IconChoicePageData const * pData )
{
    if ( pData->pPage )
        pData->pPage->Show();
}


void IconChoiceDialog::HidePageImpl ( IconChoicePageData const * pData )
{
    if ( pData->pPage )
        pData->pPage->Hide();
}

void IconChoiceDialog::ShowPage(sal_uInt16 nId)
{
    sal_uInt16 nOldPageId = GetCurPageId();
    bool bInvalidate = nOldPageId != nId;
    if (bInvalidate)
    {
        IconChoicePageData* pOldData = GetPageData(nOldPageId);
        if (pOldData && pOldData->pPage)
        {
            DeActivatePageImpl();
            HidePageImpl(pOldData);
        }

        Invalidate();
    }
    SetCurPageId(nId);
    ActivatePageImpl();
    IconChoicePageData* pNewData = GetPageData(nId);
    if (pNewData && pNewData->pPage)
        ShowPageImpl(pNewData);
}

/**********************************************************************
|
| select a page
|
\**********************************************************************/
IMPL_LINK_NOARG(IconChoiceDialog , ChosePageHdl_Impl, SvtIconChoiceCtrl*, void)
{
    SvxIconChoiceCtrlEntry *pEntry = m_pIconCtrl->GetSelectedEntry();
    if ( !pEntry )
        pEntry = m_pIconCtrl->GetCursor( );

    sal_uInt16 *pId = static_cast<sal_uInt16*>(pEntry->GetUserData());

    if( *pId != mnCurrentPageId )
    {
        ShowPage(*pId);
    }
}

/**********************************************************************
|
| Button-handler
|
\**********************************************************************/

IMPL_LINK_NOARG(IconChoiceDialog, OkHdl, Button*, void)
{
    if ( OK_Impl() )
    {
        Ok();
        Close();
    }
}


IMPL_LINK_NOARG(IconChoiceDialog, ApplyHdl, Button*, void)
{
    if ( OK_Impl() )
    {
        Ok();
        Close();
    }
}

IMPL_LINK_NOARG(IconChoiceDialog, ResetHdl, Button*, void)
{
    ResetPageImpl ();

    IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
    DBG_ASSERT( pData, "ID not known" );

    pData->pPage->Reset( *pSet );
}


IMPL_LINK_NOARG(IconChoiceDialog, CancelHdl, Button*, void)
{
    Close();
}

/**********************************************************************
|
| call page
|
\**********************************************************************/

void IconChoiceDialog::ActivatePageImpl ()
{
    DBG_ASSERT( !maPageList.empty(), "no Pages registered" );
    IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
    DBG_ASSERT( pData, "ID not known" );
    if ( pData )
    {
        if ( !pData->pPage )
        {
            pData->pPage = (pData->fnCreatePage)( m_pTabContainer, this, pSet );
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
    }

    m_pResetBtn->Show();
}


void IconChoiceDialog::DeActivatePageImpl ()
{
    IconChoicePageData *pData = GetPageData ( mnCurrentPageId );

    DeactivateRC nRet = DeactivateRC::LeavePage;

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

            if ( ( DeactivateRC::LeavePage & nRet ) &&
                 aTmp.Count() )
            {
                if (pExampleSet)
                    pExampleSet->Put(aTmp);
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
                nRet = pPage->DeactivatePage( nullptr );
        }

        if ( nRet & DeactivateRC::RefreshSet )
        {
            // TODO refresh input set
            // flag all pages to be newly initialized
            for (IconChoicePageData* pObj : maPageList)
            {
                if ( pObj->pPage.get() != pPage )
                    pObj->bRefresh = true;
                else
                    pObj->bRefresh = false;
            }
        }
    }
}


void IconChoiceDialog::ResetPageImpl ()
{
    IconChoicePageData *pData = GetPageData ( mnCurrentPageId );

    DBG_ASSERT( pData, "ID not known" );

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
    bool bSet = ( pSet != nullptr );

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


void IconChoiceDialog::Start()
{
    m_pCancelBtn->SetClickHdl( LINK( this, IconChoiceDialog, CancelHdl ) );

    Start_Impl();
}


bool IconChoiceDialog::QueryClose()
{
    bool bRet = true;
    for (IconChoicePageData* pData : maPageList)
    {
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
    FocusOnIcon( mnCurrentPageId );
    ActivatePageImpl();
}

/**********************************************************************
|
| tool-methods
|
\**********************************************************************/

IconChoicePageData* IconChoiceDialog::GetPageData ( sal_uInt16 nId )
{
    IconChoicePageData *pRet = nullptr;
    for (IconChoicePageData* pData : maPageList)
    {
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
        DeactivateRC nRet = DeactivateRC::LeavePage;
        if ( pSet )
        {
            SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

            if ( pPage->HasExchangeSupport() )
                nRet = pPage->DeactivatePage( &aTmp );

            if ( ( DeactivateRC::LeavePage & nRet )
                 && aTmp.Count() )
            {
                pExampleSet->Put( aTmp );
                pOutSet->Put( aTmp );
            }
        }
        else
            nRet = pPage->DeactivatePage( nullptr );
        bEnd = nRet != DeactivateRC::KeepPage;
    }

    return bEnd;
}


void IconChoiceDialog::Ok()
{
    if ( !pOutSet )
    {
        if ( !pExampleSet && pSet )
            pOutSet = pSet->Clone( false ); // without items
        else if ( pExampleSet )
            pOutSet = new SfxItemSet( *pExampleSet );
    }

    for ( size_t i = 0, nCount = maPageList.size(); i < nCount; ++i )
    {
        IconChoicePageData* pData = GetPageData ( i );

        IconChoicePage* pPage = pData->pPage;

        if ( pPage )
        {
            if ( pSet && !pPage->HasExchangeSupport() )
            {
                SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

                if ( pPage->FillItemSet( &aTmp ) )
                {
                    if (pExampleSet)
                        pExampleSet->Put(aTmp);
                    pOutSet->Put( aTmp );
                }
            }
        }
    }
}

void IconChoiceDialog::FocusOnIcon( sal_uInt16 nId )
{
    // set focus to icon for the current visible page
    for ( sal_Int32 i=0; i<m_pIconCtrl->GetEntryCount(); i++)
    {
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry ( i );
        sal_uInt16* pUserData = static_cast<sal_uInt16*>(pEntry->GetUserData());

        if ( pUserData && *pUserData == nId )
        {
            m_pIconCtrl->SetCursor( pEntry );
            break;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
