/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <hintids.hxx>

#include <vcl/msgbox.hxx>
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <editeng/flstitem.hxx>
#include <svx/htmlmode.hxx>
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
                     const String* pStr, sal_Bool bIsDrwTxtDlg) :
    SfxTabDialog(pParent, SW_RES(DLG_CHAR), &rCoreSet, pStr != 0),
    rView(rVw),
    bIsDrwTxtMode(bIsDrwTxtDlg)
{
    FreeResource();

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
    AddTabPage(TP_CHAR_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 );
    AddTabPage(TP_CHAR_EXT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 );
    AddTabPage(TP_CHAR_POS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), 0 );
    AddTabPage(TP_CHAR_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), 0 );
    AddTabPage(TP_CHAR_URL, SwCharURLPage::Create, 0);
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );

    SvtCJKOptions aCJKOptions;
    if(bIsDrwTxtMode)
    {
        RemoveTabPage( TP_CHAR_URL );
        RemoveTabPage( TP_BACKGROUND );
        RemoveTabPage( TP_CHAR_TWOLN );
    }
    else if(!aCJKOptions.IsDoubleLinesEnabled())
        RemoveTabPage( TP_CHAR_TWOLN );
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
    switch( nId )
    {
        case TP_CHAR_STD:
            {
            SvxFontListItem aFontListItem( *( (SvxFontListItem*)
               ( rView.GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) ) );
            aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
                if(!bIsDrwTxtMode)
                    aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
            }
            break;
        case TP_CHAR_EXT:
            if(bIsDrwTxtMode)
                aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));

            else
            {
                aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER|SVX_ENABLE_FLASH));
            }
            rPage.PageCreated(aSet);
            break;
        case TP_CHAR_POS:
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        break;
        case TP_CHAR_TWOLN:
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        break;
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
        String sEntry = pINetFmt->GetVisitedFmt();
        if( !sEntry.Len() )
            SwStyleNameMapper::FillUIName( RES_POOLCHR_INET_VISIT, sEntry );
        m_pVisitedLB->SelectEntry(sEntry);
        sEntry = pINetFmt->GetINetFmt();
        if(!sEntry.Len())
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
   ::rtl::OUString sURL = m_pURLED->GetText();
   if(!sURL.isEmpty())
    {
        sURL = URIHelper::SmartRel2Abs(INetURLObject(), sURL, Link(), false );
        // #i100683# file URLs should be normalized in the UI
        static const sal_Char* pFile = "file:";
       sal_Int32 nLength = ((sal_Int32)sizeof(pFile)-1);
       if( sURL.copy(0, nLength ).equalsAsciiL( pFile, nLength ))
            sURL = URIHelper::simpleNormalizedMakeRelative(::rtl::OUString(), sURL);
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
