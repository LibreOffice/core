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

#include <iconcdlg.hxx>
#include <cuihyperdlg.hxx>

#include <sal/log.hxx>
#include <unotools/viewoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

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


/**********************************************************************
|
| Ctor / Dtor
|
\**********************************************************************/

VCL_BUILDER_FACTORY_ARGS(SvtIconChoiceCtrl,
                         WB_3DLOOK | WB_ICON | WB_BORDER |
                         WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME |
                         WB_NODRAGSELECTION | WB_TABSTOP);

/**********************************************************************
|
| add new page
|
\**********************************************************************/

SvxIconChoiceCtrlEntry* SvxHpLinkDlg::AddTabPage(
    HyperLinkPageType nId,
    const OUString&   rIconText,
    const Image&    rChoiceIcon,
    CreatePage      pCreateFunc /* != 0 */
)
{
    maPageList.emplace_back( new IconChoicePageData ( nId, pCreateFunc ) );

    SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->InsertEntry( rIconText, rChoiceIcon );
    pEntry->SetUserData ( reinterpret_cast<void*>(nId) );
    return pEntry;
}

void SvxHpLinkDlg::SetCtrlStyle()
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

void SvxHpLinkDlg::ShowPageImpl ( IconChoicePageData const * pData )
{
    if ( pData->pPage )
        pData->pPage->Show();
}


void SvxHpLinkDlg::HidePageImpl ( IconChoicePageData const * pData )
{
    if ( pData->pPage )
        pData->pPage->Hide();
}

void SvxHpLinkDlg::ShowPage(HyperLinkPageType nId)
{
    HyperLinkPageType nOldPageId = GetCurPageId();
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
IMPL_LINK_NOARG(SvxHpLinkDlg, ChosePageHdl_Impl, SvtIconChoiceCtrl*, void)
{
    SvxIconChoiceCtrlEntry *pEntry = m_pIconCtrl->GetSelectedEntry();
    if ( !pEntry )
        pEntry = m_pIconCtrl->GetCursor( );

    HyperLinkPageType nId = static_cast<HyperLinkPageType>(reinterpret_cast<sal_uIntPtr>(pEntry->GetUserData()));

    if( nId != mnCurrentPageId )
    {
        ShowPage(nId);
    }
}

/**********************************************************************
|
| Button-handler
|
\**********************************************************************/

IMPL_LINK_NOARG(SvxHpLinkDlg, OkHdl, Button*, void)
{
    if ( OK_Impl() )
    {
        Ok();
        Close();
    }
}


IMPL_LINK_NOARG(SvxHpLinkDlg, ApplyHdl, Button*, void)
{
    if ( OK_Impl() )
    {
        Ok();
        Close();
    }
}

IMPL_LINK_NOARG(SvxHpLinkDlg, ResetHdl, Button*, void)
{
    ResetPageImpl ();

    IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
    DBG_ASSERT( pData, "ID not known" );

    pData->pPage->Reset( *pSet );
}


IMPL_LINK_NOARG(SvxHpLinkDlg, CancelHdl, Button*, void)
{
    Close();
}

/**********************************************************************
|
| call page
|
\**********************************************************************/

void SvxHpLinkDlg::ActivatePageImpl ()
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


void SvxHpLinkDlg::DeActivatePageImpl ()
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
            for (auto & pObj : maPageList)
            {
                if ( pObj->pPage.get() != pPage )
                    pObj->bRefresh = true;
                else
                    pObj->bRefresh = false;
            }
        }
    }
}


void SvxHpLinkDlg::ResetPageImpl ()
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

const sal_uInt16* SvxHpLinkDlg::GetInputRanges( const SfxItemPool& )
{
    if ( pSet )
    {
        SAL_WARN( "cui.dialogs", "Set does already exist!" );
        return pSet->GetRanges();
    }

    if ( pRanges )
        return pRanges.get();

    pRanges.reset(new sal_uInt16[1]);
    pRanges[0] = 0;

    return pRanges.get();
}


void SvxHpLinkDlg::SetInputSet( const SfxItemSet* pInSet )
{
    bool bSet = ( pSet != nullptr );

    pSet = pInSet;

    if ( !bSet && !pExampleSet && !pOutSet )
    {
        pExampleSet = new SfxItemSet( *pSet );
        pOutSet.reset(new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() ));
    }
}


/**********************************************************************
|
| start dialog
|
\**********************************************************************/

short SvxHpLinkDlg::Execute()
{
    if ( maPageList.empty() )
        return RET_CANCEL;

    Start_Impl();

    return Dialog::Execute();
}


void SvxHpLinkDlg::Start()
{
    m_pCancelBtn->SetClickHdl( LINK( this, SvxHpLinkDlg, CancelHdl ) );

    Start_Impl();
}


bool SvxHpLinkDlg::QueryClose()
{
    bool bRet = true;
    for (auto & pData : maPageList)
    {
        if ( pData->pPage && !pData->pPage->QueryClose() )
        {
            bRet = false;
            break;
        }
    }
    return bRet;
}

void SvxHpLinkDlg::Start_Impl()
{
    FocusOnIcon( mnCurrentPageId );
    ActivatePageImpl();
}

/**********************************************************************
|
| tool-methods
|
\**********************************************************************/

IconChoicePageData* SvxHpLinkDlg::GetPageData ( HyperLinkPageType nId )
{
    IconChoicePageData *pRet = nullptr;
    for (auto & pData : maPageList)
    {
        if ( pData->nId == nId )
        {
            pRet = pData.get();
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

bool SvxHpLinkDlg::OK_Impl()
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


void SvxHpLinkDlg::Ok()
{
    if ( !pOutSet )
    {
        if ( !pExampleSet && pSet )
            pOutSet = pSet->Clone( false ); // without items
        else if ( pExampleSet )
            pOutSet.reset(new SfxItemSet( *pExampleSet ));
    }

    for ( size_t i = 0, nCount = maPageList.size(); i < nCount; ++i )
    {
        IconChoicePageData* pData = GetPageData ( maPageList[i]->nId );

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

void SvxHpLinkDlg::FocusOnIcon( HyperLinkPageType nId )
{
    // set focus to icon for the current visible page
    for ( sal_Int32 i=0; i<m_pIconCtrl->GetEntryCount(); i++)
    {
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry ( i );
        HyperLinkPageType nUserData = static_cast<HyperLinkPageType>(reinterpret_cast<sal_uIntPtr>(pEntry->GetUserData()));

        if ( nUserData == nId )
        {
            m_pIconCtrl->SetCursor( pEntry );
            break;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
