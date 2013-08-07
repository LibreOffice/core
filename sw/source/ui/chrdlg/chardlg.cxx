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
#include <chrdlg.hxx>       // the dialog
#include <swmodule.hxx>
#include <poolfmt.hxx>

#include <globals.hrc>
#include <chrdlg.hrc>
#include <chrdlgmodes.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
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

SwCharDlg::SwCharDlg(Window* pParent, SwView& rVw, const SfxItemSet& rCoreSet,
    sal_uInt8 nDialogMode, const String* pStr)
    : SfxTabDialog(0, pParent, "CharacterPropertiesDialog",
        "modules/swriter/ui/characterproperties.ui", &rCoreSet, pStr != 0)
    , m_rView(rVw)
    , m_nDialogMode(nDialogMode)
{
    if(pStr)
    {
        String aTmp( GetText() );
        aTmp += SW_RESSTR(STR_TEXTCOLL_HEADER);
        aTmp += *pStr;
        aTmp += ')';
        SetText(aTmp);
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    m_nCharStdId = AddTabPage("font", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME), 0);
    m_nCharExtId = AddTabPage("fonteffects", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS), 0);
    m_nCharPosId = AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), 0 );
    m_nCharTwoId = AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), 0 );
    m_nCharUrlId = AddTabPage("hyperlink", SwCharURLPage::Create, 0);
    m_nCharBgdId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    m_nCharBrdId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), 0 );

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

/*--------------------------------------------------------------------
    Description:    set FontList
 --------------------------------------------------------------------*/

void SwCharDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nCharStdId)
    {
        SvxFontListItem aFontListItem( *( (SvxFontListItem*)
           ( m_rView.GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) ) );
        aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        if(m_nDialogMode != DLG_CHAR_DRAW && m_nDialogMode != DLG_CHAR_ANN)
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nCharExtId)
    {
        if(m_nDialogMode == DLG_CHAR_DRAW || m_nDialogMode == DLG_CHAR_ANN)
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));

        else
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER|SVX_ENABLE_FLASH));
        }
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
}

SwCharURLPage::SwCharURLPage(Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "CharURLPage", "modules/swriter/ui/charurlpage.ui", rCoreSet)
    , pINetItem(0)
    , bModified(sal_False)

{
    get(m_pURLED, "urled");
    get(m_pTextFT, "textft");
    get(m_pTextED, "texted");
    get(m_pNameED, "nameed");
    get(m_pTargetFrmLB, "targetfrmlb");
    get(m_pURLPB, "urlpb");
    get(m_pEventPB, "eventpb");
    get(m_pVisitedLB, "visitedlb");
    get(m_pNotVisitedLB, "unvisitedlb");
    get(m_pCharStyleContainer, "charstyle");

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != ( pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = ((const SfxUInt16Item*)pItem)->GetValue();
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
        size_t i;

        for ( i = 0; i < nCount; i++ )
        {
            m_pTargetFrmLB->InsertEntry( *pList->at( i ) );
        }
        for ( i = nCount; i; )
        {
            delete pList->at( --i );
        }
    }
    delete pList;
}

SwCharURLPage::~SwCharURLPage()
{
    delete pINetItem;
}

void SwCharURLPage::Reset(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(RES_TXTATR_INETFMT, sal_False, &pItem))
    {
        const SwFmtINetFmt* pINetFmt = (const SwFmtINetFmt*)pItem;
        m_pURLED->SetText( INetURLObject::decode( pINetFmt->GetValue(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ));
        m_pURLED->SaveValue();
        m_pNameED->SetText(pINetFmt->GetName());
        OUString sEntry = pINetFmt->GetVisitedFmt();
        if (sEntry.isEmpty())
            SwStyleNameMapper::FillUIName( RES_POOLCHR_INET_VISIT, sEntry );
        m_pVisitedLB->SelectEntry(sEntry);
        sEntry = pINetFmt->GetINetFmt();
        if (sEntry.isEmpty())
            SwStyleNameMapper::FillUIName( RES_POOLCHR_INET_NORMAL, sEntry );
        m_pNotVisitedLB->SelectEntry(sEntry);

        m_pTargetFrmLB->SetText(pINetFmt->GetTargetFrame());
        m_pVisitedLB->   SaveValue();
        m_pNotVisitedLB->SaveValue();
        m_pTargetFrmLB-> SaveValue();
        pINetItem = new SvxMacroItem(FN_INET_FIELD_MACRO);

        if( pINetFmt->GetMacroTbl() )
            pINetItem->SetMacroTable( *pINetFmt->GetMacroTbl() );
    }
    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_SELECTION, sal_False, &pItem))
    {
        m_pTextED->SetText(((const SfxStringItem*)pItem)->GetValue());
        m_pTextFT->Enable( sal_False );
        m_pTextED->Enable( sal_False );
    }
}

sal_Bool SwCharURLPage::FillItemSet(SfxItemSet& rSet)
{
   OUString sURL = m_pURLED->GetText();
   if(!sURL.isEmpty())
    {
        sURL = URIHelper::SmartRel2Abs(INetURLObject(), sURL, Link(), false );
        // #i100683# file URLs should be normalized in the UI
        static const sal_Char* pFile = "file:";
       sal_Int32 nLength = ((sal_Int32)sizeof(pFile)-1);
       if( sURL.copy(0, nLength ).equalsAsciiL( pFile, nLength ))
            sURL = URIHelper::simpleNormalizedMakeRelative(OUString(), sURL);
    }

    SwFmtINetFmt aINetFmt(sURL, m_pTargetFrmLB->GetText());
    aINetFmt.SetName(m_pNameED->GetText());
    bModified |= m_pURLED->GetText() != m_pURLED->GetSavedValue();
    bModified |= m_pNameED->IsModified();
    bModified |= m_pTargetFrmLB->GetSavedValue() != m_pTargetFrmLB->GetText();

    // set valid settings first
    String sEntry = m_pVisitedLB->GetSelectEntry();
    sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
    aINetFmt.SetVisitedFmtId(nId);
    aINetFmt.SetVisitedFmt(nId == RES_POOLCHR_INET_VISIT ? aEmptyStr : sEntry);

    sEntry = m_pNotVisitedLB->GetSelectEntry();
    nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
    aINetFmt.SetINetFmtId( nId );
    aINetFmt.SetINetFmt(nId == RES_POOLCHR_INET_NORMAL ? aEmptyStr : sEntry);

    if( pINetItem && !pINetItem->GetMacroTable().empty() )
        aINetFmt.SetMacroTbl( &pINetItem->GetMacroTable() );

    if(m_pVisitedLB->GetSavedValue() != m_pVisitedLB->GetSelectEntryPos())
        bModified = sal_True;

    if(m_pNotVisitedLB->GetSavedValue() != m_pNotVisitedLB->GetSelectEntryPos())
        bModified = sal_True;

    if(m_pTextED->IsModified())
    {
        bModified = sal_True;
        rSet.Put(SfxStringItem(FN_PARAM_SELECTION, m_pTextED->GetText()));
    }
    if(bModified)
        rSet.Put(aINetFmt);
    return bModified;
}

SfxTabPage* SwCharURLPage::Create(  Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwCharURLPage( pParent, rAttrSet ) );
}

IMPL_LINK_NOARG(SwCharURLPage, InsertFileHdl)
{
    FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();
        m_pURLED->SetText(xFP->getFiles().getConstArray()[0]);
    }
    return 0;
}

IMPL_LINK_NOARG(SwCharURLPage, EventHdl)
{
    bModified |= SwMacroAssignDlg::INetFmtDlg( this,
                    ::GetActiveView()->GetWrtShell(), pINetItem );
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
