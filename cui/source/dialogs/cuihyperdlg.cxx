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

#include <sal/config.h>

#include <osl/diagnose.h>
#include <comphelper/lok.hxx>
#include <unotools/viewoptions.hxx>
#include <cuihyperdlg.hxx>
#include <hlinettp.hxx>
#include <hlmailtp.hxx>
#include <hldoctp.hxx>
#include <hldocntp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <svx/svxids.hrc>
#include <dialmgr.hxx>
#include <strings.hrc>
#include <vector>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;


//#                                                                      #
//# Childwindow-Wrapper-Class                                            #
//#                                                                      #


SvxHlinkCtrl::SvxHlinkCtrl( sal_uInt16 _nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg )
    : SfxControllerItem ( _nId, rBindings )
    , aRdOnlyForwarder  ( SID_READONLY_MODE, *this )
{
    pParent = pDlg;
}

void SvxHlinkCtrl::dispose()
{
    pParent = nullptr;
    aRdOnlyForwarder.dispose();
    ::SfxControllerItem::dispose();
}

void SvxHlinkCtrl::StateChangedAtToolBoxControl( sal_uInt16 nSID, SfxItemState eState,
                                 const SfxPoolItem* pState )
{
    if (!(eState == SfxItemState::DEFAULT && pParent))
        return;

    switch ( nSID )
    {
        case SID_HYPERLINK_GETLINK :
        {
            pParent->SetPage( static_cast<const SvxHyperlinkItem*>(pState) );
        }
        break;
        case SID_READONLY_MODE :
        {
            pParent->SetReadOnlyMode( static_cast<const SfxBoolItem*>(pState)->GetValue() );
        }
        break;
    }
}

// tdf#90496 - remember last used view in hyperlink dialog
OUString SvxHpLinkDlg::msRememberedPageId(u"internet"_ustr);

//#                                                                      #
//# Hyperlink - Dialog                                                   #
//#                                                                      #
SvxHpLinkDlg::SvxHpLinkDlg(SfxBindings* pBindings, SfxChildWindow* pChild, weld::Window* pParent)
    : SfxModelessDialogController(pBindings, pChild, pParent, u"cui/ui/hyperlinkdialog.ui"_ustr, u"HyperlinkDialog"_ustr)
    , pSet            ( nullptr )
    , maCtrl          ( SID_HYPERLINK_GETLINK, *pBindings, this )
    , mbIsHTMLDoc     ( false )
    , m_xIconCtrl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xApplyBtn(m_xBuilder->weld_button(u"apply"_ustr))
    , m_xCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xHelpBtn(m_xBuilder->weld_button(u"help"_ustr))
    , m_xResetBtn(m_xBuilder->weld_button(u"reset"_ustr))
{
    m_xIconCtrl->connect_enter_page( LINK ( this, SvxHpLinkDlg, ChosePageHdl_Impl ) );
    m_xIconCtrl->show();

    // ItemSet
    if ( pSet )
    {
        pExampleSet.reset(new SfxItemSet( *pSet ));
        pOutSet.reset(new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() ));
    }

    // Buttons
    m_xOKBtn->show();
    m_xCancelBtn->show();

    m_xApplyBtn->set_visible(!comphelper::LibreOfficeKit::isActive());
    m_xHelpBtn->set_visible(!comphelper::LibreOfficeKit::isActive());
    m_xResetBtn->set_visible(!comphelper::LibreOfficeKit::isActive());

    mbGrabFocus = true;

    // set OK/Cancel - button
    m_xCancelBtn->set_label(CuiResId(RID_CUISTR_HYPDLG_CLOSEBUT));

    // create itemset for tabpages
    mpItemSet = std::make_unique<SfxItemSetFixed<SID_HYPERLINK_GETLINK,
                               SID_HYPERLINK_SETLINK>>( SfxGetpApp()->GetPool());

    SvxHyperlinkItem aItem(SID_HYPERLINK_GETLINK);
    mpItemSet->Put(aItem);

    SetInputSet (mpItemSet.get());

    AddTabPage(u"internet"_ustr, SvxHyperlinkInternetTp::Create);
    AddTabPage(u"mail"_ustr, SvxHyperlinkMailTp::Create);
    AddTabPage(u"document"_ustr, SvxHyperlinkDocTp::Create);

    if (!comphelper::LibreOfficeKit::isActive())
        AddTabPage(u"newdocument"_ustr, SvxHyperlinkNewDocTp::Create);

    GetBindings().Update(SID_HYPERLINK_GETLINK);
    GetBindings().Update(SID_READONLY_MODE);

    m_xResetBtn->connect_clicked( LINK( this, SvxHpLinkDlg, ResetHdl ) );
    m_xOKBtn->connect_clicked( LINK ( this, SvxHpLinkDlg, ClickOkHdl_Impl ) );
    m_xApplyBtn->connect_clicked ( LINK ( this, SvxHpLinkDlg, ClickApplyHdl_Impl ) );
}

SvxHpLinkDlg::~SvxHpLinkDlg()
{
    mbGrabFocus = false; // don't do any grab if tear-down moves focus around during destruction

    // delete config item, so the base class (SfxModelessDialogController) can not load it on the next start
    SvtViewOptions aViewOpt( EViewType::TabDialog, OUString::number(SID_HYPERLINK_DIALOG) );
    aViewOpt.Delete();

    mpItemSet.reset();

    maCtrl.dispose();

    maPageList.clear();

    pRanges.reset();
    pOutSet.reset();
}

IconChoicePage* SvxHpLinkDlg::GetTabPage( std::u16string_view rPageId )
{
    const IconChoicePageData* p = GetPageData(rPageId);
    return (p == nullptr) ? nullptr : p->xPage.get();
}

void SvxHpLinkDlg::Activate()
{
    if (mbGrabFocus) {
        static_cast<SvxHyperlinkTabPageBase *>(GetTabPage(GetCurPageId()))->SetInitFocus();
        mbGrabFocus = false;
    }
    SfxModelessDialogController::Activate();
}

void SvxHpLinkDlg::Close()
{
    if (IsClosing())
        return;
    if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
        pViewFrame->ToggleChildWindow(SID_HYPERLINK_DIALOG);
}

void SvxHpLinkDlg::Apply()
{
    SfxItemSetFixed<SID_HYPERLINK_GETLINK, SID_HYPERLINK_SETLINK> aItemSet( SfxGetpApp()->GetPool() );

    SvxHyperlinkTabPageBase* pCurrentPage = static_cast<SvxHyperlinkTabPageBase*>(
                                                GetTabPage( GetCurPageId() ) );

    pCurrentPage->FillItemSet( &aItemSet );

    const SvxHyperlinkItem *aItem = aItemSet.GetItem(SID_HYPERLINK_SETLINK);
    if ( !aItem->GetURL().isEmpty() )
        GetDispatcher()->ExecuteList(SID_HYPERLINK_SETLINK,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { aItem });

    static_cast<SvxHyperlinkTabPageBase*>( GetTabPage( GetCurPageId() ) )->DoApply();
}

/// Click on OK button
IMPL_LINK_NOARG(SvxHpLinkDlg, ClickOkHdl_Impl, weld::Button&, void)
{
    Apply();
    m_xDialog->response(RET_OK);
}

/*************************************************************************
|*
|* Click on Apply-button
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHpLinkDlg, ClickApplyHdl_Impl, weld::Button&, void)
{
    Apply();
}

/*************************************************************************
|*
|* Set Page
|*
|************************************************************************/
void SvxHpLinkDlg::SetPage ( SvxHyperlinkItem const * pItem )
{
    mpItemSet->Put(*pItem);

    const OUString& aStrURL(pItem->GetURL());
    INetURLObject aURL(aStrURL);
    INetProtocol eProtocolTyp = aURL.GetProtocol();

    OUString sPageId(msRememberedPageId);

    if (comphelper::LibreOfficeKit::isActive()) {
        sPageId = "internet";
    }

    if (eProtocolTyp == INetProtocol::Http || eProtocolTyp == INetProtocol::Https || eProtocolTyp == INetProtocol::Ftp) {
        sPageId = "internet";
    } else if (eProtocolTyp == INetProtocol::Mailto) {
        sPageId = "mail";
    } else if (!comphelper::LibreOfficeKit::isActive() &&
        (eProtocolTyp == INetProtocol::File || aStrURL.startsWith("#"))) {
        sPageId = "document";
    }

    IconChoicePage* pPage = GetTabPage(sPageId);

    // Switching to tab that doesn't exist should not crash
    if (pPage == nullptr) { return; }

    ShowPage (sPageId);
    mbIsHTMLDoc = (pItem->GetInsertMode() & HLINK_HTMLMODE) != 0;

    SfxItemSet& aPageSet = const_cast<SfxItemSet&>(pPage->GetItemSet ());
    aPageSet.Put ( *pItem );

    for (std::unique_ptr<IconChoicePageData>& page: maPageList) {
        page->xPage->Reset(*pSet);
    }
}

/*************************************************************************
|*
|* Enable/Disable ReadOnly mode
|*
|************************************************************************/
void SvxHpLinkDlg::SetReadOnlyMode( bool bRdOnly )
{
    m_xOKBtn->set_sensitive(!bRdOnly);
}

/*************************************************************************
|*
|* late-initialization of newly created pages
|*
|************************************************************************/
void SvxHpLinkDlg::PageCreated(IconChoicePage& rPage)
{
    SvxHyperlinkTabPageBase& rHyperlinkPage = dynamic_cast< SvxHyperlinkTabPageBase& >( rPage );
    Reference< XFrame > xDocumentFrame = GetBindings().GetActiveFrame();
    OSL_ENSURE( xDocumentFrame.is(), "SvxHpLinkDlg::PageCreated: macro assignment functionality won't work with a proper frame!" );
    rHyperlinkPage.SetDocumentFrame( xDocumentFrame );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
