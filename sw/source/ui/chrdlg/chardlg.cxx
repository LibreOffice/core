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

#include <hintids.hxx>

#include <comphelper/fileurl.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svl/cjkoptions.hxx>

#include <cmdid.h>
#include <helpid.h>
#include <swtypes.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <uitool.hxx>
#include <fmtinfmt.hxx>
#include <macassgn.hxx>
#include <chrdlg.hxx>
#include <swmodule.hxx>
#include <poolfmt.hxx>

#include <globals.hrc>
#include <chrdlg.hrc>
#include <chrdlgmodes.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/viewfrm.hxx>

#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <svx/dialogs.hrc>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::sfx2;

SwCharDlg::SwCharDlg(vcl::Window* pParent, SwView& rVw, const SfxItemSet& rCoreSet,
    sal_uInt8 nDialogMode, const OUString* pStr)
    : SfxTabDialog(pParent, "CharacterPropertiesDialog",
        "modules/swriter/ui/characterproperties.ui", &rCoreSet, pStr != nullptr)
    , m_rView(rVw)
    , m_nDialogMode(nDialogMode)
{
    if(pStr)
    {
        SetText(GetText() + SW_RESSTR(STR_TEXTCOLL_HEADER) + *pStr + ")");
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");
    m_nCharStdId = AddTabPage("font", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME), nullptr);
    m_nCharExtId = AddTabPage("fonteffects", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS), nullptr);
    m_nCharPosId = AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), nullptr );
    m_nCharTwoId = AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), nullptr );
    m_nCharUrlId = AddTabPage("hyperlink", SwCharURLPage::Create, nullptr);
    m_nCharBgdId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), nullptr );
    m_nCharBrdId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), nullptr );

    SvtCJKOptions aCJKOptions;
    if(m_nDialogMode == DLG_CHAR_DRAW || m_nDialogMode == DLG_CHAR_ANN)
    {
        RemoveTabPage(m_nCharUrlId);
        RemoveTabPage(m_nCharBgdId);
        RemoveTabPage(m_nCharTwoId);
    }
    else if(!aCJKOptions.IsDoubleLinesEnabled())
        RemoveTabPage(m_nCharTwoId);

    if(m_nDialogMode != DLG_CHAR_STD)
        RemoveTabPage(m_nCharBrdId);
}

SwCharDlg::~SwCharDlg()
{
}

// set FontList
void SwCharDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nCharStdId)
    {
        SvxFontListItem aFontListItem( *static_cast<const SvxFontListItem*>(
           ( m_rView.GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) ) );
        aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        if(m_nDialogMode != DLG_CHAR_DRAW && m_nDialogMode != DLG_CHAR_ANN)
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nCharExtId)
    {
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER|SVX_ENABLE_FLASH));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nCharPosId)
    {
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nCharTwoId)
    {
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nCharBgdId)
    {
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_HIGHLIGHTING)));
        rPage.PageCreated(aSet);
    }
}

SwCharURLPage::SwCharURLPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "CharURLPage", "modules/swriter/ui/charurlpage.ui", &rCoreSet)
    , pINetItem(nullptr)
    , bModified(false)

{
    get(m_pURLED, "urled");
    get(m_pTextFT, "textft");
    get(m_pTextED, "texted");
    get(m_pNameED, "nameed");
    get(m_pTargetFrameLB, "targetfrmlb");
    get(m_pURLPB, "urlpb");
    get(m_pEventPB, "eventpb");
    get(m_pVisitedLB, "visitedlb");
    get(m_pNotVisitedLB, "unvisitedlb");
    get(m_pCharStyleContainer, "charstyle");

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SfxItemState::SET == rCoreSet.GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( nullptr != ( pShell = SfxObjectShell::Current()) &&
                    nullptr != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        if(HTMLMODE_ON & nHtmlMode)
            m_pCharStyleContainer->Hide();
    }

    m_pURLPB->SetClickHdl  (LINK( this, SwCharURLPage, InsertFileHdl));
    m_pEventPB->SetClickHdl(LINK( this, SwCharURLPage, EventHdl ));

    SwView *pView = ::GetActiveView();
    ::FillCharStyleListBox(*m_pVisitedLB, pView->GetDocShell());
    ::FillCharStyleListBox(*m_pNotVisitedLB, pView->GetDocShell());

    TargetList* pList = new TargetList;
    const SfxFrame& rFrame = pView->GetViewFrame()->GetTopFrame();
    rFrame.GetTargetList(*pList);
    if ( !pList->empty() )
    {
        size_t nCount = pList->size();

        for ( size_t i = 0; i < nCount; i++ )
        {
            m_pTargetFrameLB->InsertEntry( pList->at( i ) );
        }
    }
    delete pList;
}

SwCharURLPage::~SwCharURLPage()
{
    disposeOnce();
}

void SwCharURLPage::dispose()
{
    delete pINetItem;
    m_pURLED.clear();
    m_pTextFT.clear();
    m_pTextED.clear();
    m_pNameED.clear();
    m_pTargetFrameLB.clear();
    m_pURLPB.clear();
    m_pEventPB.clear();
    m_pVisitedLB.clear();
    m_pNotVisitedLB.clear();
    m_pCharStyleContainer.clear();
    SfxTabPage::dispose();
}

void SwCharURLPage::Reset(const SfxItemSet* rSet)
{
    const SfxPoolItem* pItem;
    if ( SfxItemState::SET == rSet->GetItemState( RES_TXTATR_INETFMT, false, &pItem ) )
    {
        const SwFormatINetFormat* pINetFormat = static_cast<const SwFormatINetFormat*>( pItem);
        m_pURLED->SetText(INetURLObject::decode(pINetFormat->GetValue(),
            INetURLObject::DECODE_UNAMBIGUOUS));
        m_pURLED->SaveValue();
        m_pURLED->SetText(pINetFormat->GetName());

        OUString sEntry = pINetFormat->GetVisitedFormat();
        if (sEntry.isEmpty())
        {
            OSL_ENSURE( false, "<SwCharURLPage::Reset(..)> - missing visited character format at hyperlink attribute" );
            SwStyleNameMapper::FillUIName(RES_POOLCHR_INET_VISIT, sEntry);
        }
        m_pVisitedLB->SelectEntry( sEntry );

        sEntry = pINetFormat->GetINetFormat();
        if (sEntry.isEmpty())
        {
            OSL_ENSURE( false, "<SwCharURLPage::Reset(..)> - missing unvisited character format at hyperlink attribute" );
            SwStyleNameMapper::FillUIName(RES_POOLCHR_INET_NORMAL, sEntry);
        }
        m_pNotVisitedLB->SelectEntry(sEntry);

        m_pTargetFrameLB->SetText(pINetFormat->GetTargetFrame());
        m_pVisitedLB->   SaveValue();
        m_pNotVisitedLB->SaveValue();
        m_pTargetFrameLB-> SaveValue();
        pINetItem = new SvxMacroItem(FN_INET_FIELD_MACRO);

        if( pINetFormat->GetMacroTable() )
            pINetItem->SetMacroTable( *pINetFormat->GetMacroTable() );
    }
    if(SfxItemState::SET == rSet->GetItemState(FN_PARAM_SELECTION, false, &pItem))
    {
        m_pTextED->SetText(static_cast<const SfxStringItem*>(pItem)->GetValue());
        m_pTextFT->Enable( false );
        m_pTextED->Enable( false );
    }
}

bool SwCharURLPage::FillItemSet(SfxItemSet* rSet)
{
    OUString sURL = m_pURLED->GetText();
    if(!sURL.isEmpty())
    {
        sURL = URIHelper::SmartRel2Abs(INetURLObject(), sURL, Link<OUString *, bool>(), false );
        // #i100683# file URLs should be normalized in the UI
        if ( comphelper::isFileUrl(sURL) )
            sURL = URIHelper::simpleNormalizedMakeRelative(OUString(), sURL);
    }

    SwFormatINetFormat aINetFormat(sURL, m_pTargetFrameLB->GetText());
    aINetFormat.SetName(m_pNameED->GetText());
    bool bURLModified = m_pURLED->IsValueChangedFromSaved();
    bool bNameModified = m_pNameED->IsModified();
    bool bTargetModified = m_pTargetFrameLB->IsValueChangedFromSaved();
    bModified = bURLModified || bNameModified || bTargetModified;

    // set valid settings first
    OUString sEntry = m_pVisitedLB->GetSelectEntry();
    sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
    aINetFormat.SetVisitedFormatAndId( sEntry, nId );

    sEntry = m_pNotVisitedLB->GetSelectEntry();
    nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
    aINetFormat.SetINetFormatAndId( sEntry, nId );

    if( pINetItem && !pINetItem->GetMacroTable().empty() )
        aINetFormat.SetMacroTable( &pINetItem->GetMacroTable() );

    if(m_pVisitedLB->IsValueChangedFromSaved())
        bModified = true;

    if(m_pNotVisitedLB->IsValueChangedFromSaved())
        bModified = true;

    if(m_pTextED->IsModified())
    {
        bModified = true;
        rSet->Put(SfxStringItem(FN_PARAM_SELECTION, m_pTextED->GetText()));
    }
    if(bModified)
        rSet->Put(aINetFormat);
    return bModified;
}

VclPtr<SfxTabPage> SwCharURLPage::Create(  vcl::Window* pParent,
                                           const SfxItemSet* rAttrSet )
{
    return VclPtr<SwCharURLPage>::Create( pParent, *rAttrSet );
}

IMPL_LINK_NOARG_TYPED(SwCharURLPage, InsertFileHdl, Button*, void)
{
    FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        Reference < XFilePicker2 > xFP = aDlgHelper.GetFilePicker();
        m_pURLED->SetText(xFP->getSelectedFiles().getConstArray()[0]);
    }
}

IMPL_LINK_NOARG_TYPED(SwCharURLPage, EventHdl, Button*, void)
{
    bModified |= SwMacroAssignDlg::INetFormatDlg( this,
                    ::GetActiveView()->GetWrtShell(), pINetItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
