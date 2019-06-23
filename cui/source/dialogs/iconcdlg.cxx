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

IconChoicePage::IconChoicePage(weld::Container* pParent,
                               const OUString& rUIXMLDescription, const OString& rID,
                               const SfxItemSet* pItemSet)
    : xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , xContainer(xBuilder->weld_container(rID))
    , pSet(pItemSet)
    , bHasExchangeSupport(false)
{
}

IconChoicePage::~IconChoicePage()
{
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
| add new page
|
\**********************************************************************/
void SvxHpLinkDlg::AddTabPage(const OString& rId, CreatePage pCreateFunc /* != 0 */)
{
    weld::Container* pPage = m_xIconCtrl->get_page(rId);
    maPageList.emplace_back(new IconChoicePageData(rId, pCreateFunc(pPage, this, pSet)));
    maPageList.back()->xPage->Reset(*pSet);
    PageCreated(rId, *maPageList.back()->xPage);
}

/**********************************************************************
|
| Show / Hide page or button
|
\**********************************************************************/
void SvxHpLinkDlg::ShowPage(const OString& rId)
{
    OString sOldPageId = GetCurPageId();
    bool bInvalidate = sOldPageId != rId;
    if (bInvalidate)
    {
        IconChoicePageData* pOldData = GetPageData(sOldPageId);
        if (pOldData && pOldData->xPage)
        {
            DeActivatePageImpl();
        }
    }
    SetCurPageId(rId);
    ActivatePageImpl();
}

/**********************************************************************
|
| select a page
|
\**********************************************************************/
IMPL_LINK(SvxHpLinkDlg, ChosePageHdl_Impl, const OString&, rId, void)
{
    if (rId != msCurrentPageId)
    {
        ShowPage(rId);
    }
}

/**********************************************************************
|
| Button-handler
|
\**********************************************************************/
IMPL_LINK_NOARG(SvxHpLinkDlg, ResetHdl, weld::Button&, void)
{
    ResetPageImpl ();

    IconChoicePageData* pData = GetPageData ( msCurrentPageId );
    DBG_ASSERT( pData, "ID not known" );

    pData->xPage->Reset( *pSet );
}

/**********************************************************************
|
| call page
|
\**********************************************************************/
void SvxHpLinkDlg::ActivatePageImpl()
{
    DBG_ASSERT( !maPageList.empty(), "no Pages registered" );
    IconChoicePageData* pData = GetPageData ( msCurrentPageId );
    DBG_ASSERT( pData, "ID not known" );
    if ( pData )
    {
        if ( pData->bRefresh )
        {
            pData->xPage->Reset( *pSet );
            pData->bRefresh = false;
        }

        if ( pExampleSet )
            pData->xPage->ActivatePage( *pExampleSet );
        m_xDialog->set_help_id(pData->xPage->GetHelpId());
    }

    m_xResetBtn->show();
}

void SvxHpLinkDlg::DeActivatePageImpl ()
{
    IconChoicePageData *pData = GetPageData ( msCurrentPageId );

    DeactivateRC nRet = DeactivateRC::LeavePage;

    if ( pData )
    {
        IconChoicePage * pPage = pData->xPage.get();

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
                if ( pObj->xPage.get() != pPage )
                    pObj->bRefresh = true;
                else
                    pObj->bRefresh = false;
            }
        }
    }
}


void SvxHpLinkDlg::ResetPageImpl ()
{
    IconChoicePageData *pData = GetPageData ( msCurrentPageId );

    DBG_ASSERT( pData, "ID not known" );

    pData->xPage->Reset( *pSet );
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

void SvxHpLinkDlg::Start()
{
    Start_Impl();
}

bool SvxHpLinkDlg::QueryClose()
{
    bool bRet = true;
    for (auto & pData : maPageList)
    {
        if ( pData->xPage && !pData->xPage->QueryClose() )
        {
            bRet = false;
            break;
        }
    }
    return bRet;
}

void SvxHpLinkDlg::Start_Impl()
{
    SwitchPage(msCurrentPageId);
    ActivatePageImpl();
}

/**********************************************************************
|
| tool-methods
|
\**********************************************************************/

IconChoicePageData* SvxHpLinkDlg::GetPageData ( const OString& rId )
{
    IconChoicePageData *pRet = nullptr;
    for (auto & pData : maPageList)
    {
        if ( pData->sId == rId )
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
    IconChoicePage* pPage = GetPageData ( msCurrentPageId )->xPage.get();

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
        IconChoicePageData* pData = GetPageData ( maPageList[i]->sId );

        IconChoicePage* pPage = pData->xPage.get();

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

void SvxHpLinkDlg::SwitchPage( const OString& rId )
{
    m_xIconCtrl->set_current_page(rId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
