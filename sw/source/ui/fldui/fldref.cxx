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

#include "swtypes.hxx"
#include <view.hxx>
#include <IMark.hxx>
#include <expfld.hxx>
#include <swmodule.hxx>
#include <fldref.hxx>
#include <reffld.hxx>
#include <wrtsh.hxx>

#include <fldui.hrc>
#include <globals.hrc>
#include <SwNodeNum.hxx>
#include <IDocumentMarkAccess.hxx>
#include <ndtxt.hxx>

#include <svtools/treelistentry.hxx>

#define REFFLDFLAG          0x4000
#define REFFLDFLAG_BOOKMARK 0x4800
#define REFFLDFLAG_FOOTNOTE 0x5000
#define REFFLDFLAG_ENDNOTE  0x6000
// #i83479#
#define REFFLDFLAG_HEADING  0x7100
#define REFFLDFLAG_NUMITEM  0x7200

static sal_uInt16 nFldDlgFmtSel = 0;

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFldRefPage::SwFldRefPage(vcl::Window* pParent, const SfxItemSet& rCoreSet )
    : SwFldPage(pParent, "FldRefPage",
        "modules/swriter/ui/fldrefpage.ui", rCoreSet)
    , maOutlineNodes()
    , maNumItems()
    , mpSavedSelectedTxtNode(0)
    , mnSavedSelectedPos(0)
{
    get(m_pTypeLB, "type");
    get(m_pSelection, "selectframe");
    get(m_pSelectionLB, "select");
    m_pSelectionLB->SetStyle(m_pSelectionLB->GetStyle() | WB_SORT);
    // #i83479#
    get(m_pSelectionToolTipLB, "selecttip");
    get(m_pFormat, "formatframe");
    get(m_pFormatLB, "format");
    get(m_pNameFT, "nameft");
    get(m_pNameED, "name");
    get(m_pValueED, "value");

    sBookmarkTxt = m_pTypeLB->GetEntry(0);
    sFootnoteTxt = m_pTypeLB->GetEntry(1);
    sEndnoteTxt = m_pTypeLB->GetEntry(2);
    // #i83479#
    sHeadingTxt = m_pTypeLB->GetEntry(3);
    sNumItemTxt = m_pTypeLB->GetEntry(4);
    m_pTypeLB->Clear();

    long nHeight = m_pTypeLB->GetTextHeight() * 20;
    m_pSelection->set_height_request(nHeight);
    m_pSelectionToolTipLB->set_height_request(nHeight);

    nHeight = m_pTypeLB->GetTextHeight() * 8;
    m_pTypeLB->set_height_request(nHeight);
    m_pFormatLB->set_height_request(nHeight);

    long nWidth = m_pTypeLB->LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MAP_APPFONT)).Width();
    m_pTypeLB->set_width_request(nWidth);
    m_pFormatLB->set_width_request(nWidth);
    m_pSelection->set_width_request(nWidth*2);
    m_pSelectionToolTipLB->set_width_request(nWidth*2);

    m_pNameED->SetModifyHdl(LINK(this, SwFldRefPage, ModifyHdl));

    m_pTypeLB->SetDoubleClickHdl       (LINK(this, SwFldRefPage, InsertHdl));
    m_pTypeLB->SetSelectHdl            (LINK(this, SwFldRefPage, TypeHdl));
    m_pSelectionLB->SetSelectHdl       (LINK(this, SwFldRefPage, SubTypeHdl));
    m_pSelectionLB->SetDoubleClickHdl  (LINK(this, SwFldRefPage, InsertHdl));
    m_pFormatLB->SetDoubleClickHdl     (LINK(this, SwFldRefPage, InsertHdl));

    // #i83479#
    m_pSelectionToolTipLB->SetSelectHdl( LINK(this, SwFldRefPage, SubTypeHdl) );
    m_pSelectionToolTipLB->SetDoubleClickHdl( LINK(this, SwFldRefPage, InsertHdl) );
    m_pSelectionToolTipLB->SetStyle( m_pSelectionToolTipLB->GetStyle() | WB_HSCROLL );
    m_pSelectionToolTipLB->SetSpaceBetweenEntries(1);
    m_pSelectionToolTipLB->SetHighlightRange();
}

SwFldRefPage::~SwFldRefPage()
{
}

// #i83479#
void SwFldRefPage::SaveSelectedTxtNode()
{
    mpSavedSelectedTxtNode = 0;
    mnSavedSelectedPos = 0;
    if ( m_pSelectionToolTipLB->IsVisible() )
    {
        SvTreeListEntry* pEntry = m_pSelectionToolTipLB->GetCurEntry();
        if ( pEntry )
        {
            const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

            if ( nTypeId == REFFLDFLAG_HEADING )
            {
                mnSavedSelectedPos = static_cast<size_t>(reinterpret_cast<sal_uLong>(pEntry->GetUserData()));
                if ( mnSavedSelectedPos < maOutlineNodes.size() )
                {
                    mpSavedSelectedTxtNode = maOutlineNodes[mnSavedSelectedPos];
                }
            }
            else if ( nTypeId == REFFLDFLAG_NUMITEM )
            {
                mnSavedSelectedPos = static_cast<size_t>(reinterpret_cast<sal_uLong>(pEntry->GetUserData()));
                if ( mnSavedSelectedPos < maNumItems.size() )
                {
                    mpSavedSelectedTxtNode = maNumItems[mnSavedSelectedPos]->GetTxtNode();
                }
            }
        }
    }
}

void SwFldRefPage::Reset(const SfxItemSet* )
{
    if (!IsFldEdit())
    {
        SavePos(m_pTypeLB);
        // #i83479#
        SaveSelectedTxtNode();
    }
    SetSelectionSel(LISTBOX_ENTRY_NOTFOUND);
    SetTypeSel(LISTBOX_ENTRY_NOTFOUND);
    Init(); // general initialisation

    // initialise TypeListBox
    m_pTypeLB->SetUpdateMode(false);
    m_pTypeLB->Clear();

    // fill Type-Listbox

    sal_Int32 nPos;
    // set/insert reference
    const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

    for (short i = rRg.nStart; i < rRg.nEnd; ++i)
    {
        const sal_uInt16 nTypeId = SwFldMgr::GetTypeId(i);

        if (!IsFldEdit() || nTypeId != TYP_SETREFFLD)
        {
            nPos = m_pTypeLB->InsertEntry(SwFldMgr::GetTypeStr(i), i - rRg.nStart);
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }

    // #i83479#
    // entries for headings and numbered items
    nPos = m_pTypeLB->InsertEntry(sHeadingTxt);
    m_pTypeLB->SetEntryData(nPos, (void*)REFFLDFLAG_HEADING);
    nPos = m_pTypeLB->InsertEntry(sNumItemTxt);
    m_pTypeLB->SetEntryData(nPos, (void*)REFFLDFLAG_NUMITEM);

    // fill up with the sequence types
    SwWrtShell *pSh = GetWrtShell();
    if(!pSh)
        pSh = ::GetActiveWrtShell();

    const sal_uInt16 nFldTypeCnt = pSh->GetFldTypeCount(RES_SETEXPFLD);

    for (sal_uInt16 n = 0; n < nFldTypeCnt; ++n)
    {
        SwSetExpFieldType* pType = (SwSetExpFieldType*)pSh->GetFldType(n, RES_SETEXPFLD);

        if ((nsSwGetSetExpType::GSE_SEQ & pType->GetType()) && pType->GetDepends() && pSh->IsUsed(*pType))
        {
            nPos = m_pTypeLB->InsertEntry(pType->GetName());
            m_pTypeLB->SetEntryData(nPos, (void*)(sal_uIntPtr)(REFFLDFLAG | n));
        }
    }

    // text marks - now always (because of globaldocuments)
    nPos = m_pTypeLB->InsertEntry(sBookmarkTxt);
    m_pTypeLB->SetEntryData(nPos, (void*)REFFLDFLAG_BOOKMARK);

    // footnotes:
    if( pSh->HasFtns() )
    {
        nPos = m_pTypeLB->InsertEntry(sFootnoteTxt);
        m_pTypeLB->SetEntryData(nPos, (void*)REFFLDFLAG_FOOTNOTE);
    }

    // endnotes:
    if ( pSh->HasFtns(true) )
    {
        nPos = m_pTypeLB->InsertEntry(sEndnoteTxt);
        m_pTypeLB->SetEntryData(nPos, (void*)REFFLDFLAG_ENDNOTE);
    }

    // select old Pos
    if (!IsFldEdit())
        RestorePos(m_pTypeLB);

    m_pTypeLB->SetUpdateMode(true);

    nFldDlgFmtSel = 0;

    if( !IsRefresh() )
    {
        OUString sUserData = GetUserData();
        if(!IsRefresh() && sUserData.getToken(0, ';').
                                equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            OUString sVal = sUserData.getToken(1, ';');
            const sal_uInt16 nVal = static_cast< sal_uInt16 >(sVal.toInt32());
            if(nVal != USHRT_MAX)
            {
                for(sal_Int32 i = 0; i < m_pTypeLB->GetEntryCount(); i++)
                    if(nVal == (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(i))
                    {
                        m_pTypeLB->SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    if (IsFldEdit())
    {
        m_pTypeLB->SaveValue();
        m_pSelectionLB->SaveValue();
        m_pFormatLB->SaveValue();
        m_pNameED->SaveValue();
        m_pValueED->SaveValue();
    }
}

IMPL_LINK_NOARG(SwFldRefPage, TypeHdl)
{
    // save old ListBoxPos
    const sal_Int32 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_pTypeLB->GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        if (IsFldEdit())
        {
            // select positions
            OUString sName;
            sal_uInt16 nFlag = 0;

            switch( GetCurField()->GetSubType() )
            {
                case REF_BOOKMARK:
                {
                    // #i83479#
                    SwGetRefField* pRefFld = dynamic_cast<SwGetRefField*>(GetCurField());
                    if ( pRefFld &&
                         pRefFld->IsRefToHeadingCrossRefBookmark() )
                    {
                        sName = sHeadingTxt;
                        nFlag = REFFLDFLAG_HEADING;
                    }
                    else if ( pRefFld &&
                              pRefFld->IsRefToNumItemCrossRefBookmark() )
                    {
                        sName = sNumItemTxt;
                        nFlag = REFFLDFLAG_NUMITEM;
                    }
                    else
                    {
                        sName = sBookmarkTxt;
                        nFlag = REFFLDFLAG_BOOKMARK;
                    }
                }
                break;

                case REF_FOOTNOTE:
                    sName = sFootnoteTxt;
                    nFlag = REFFLDFLAG_FOOTNOTE;
                    break;

                case REF_ENDNOTE:
                    sName = sEndnoteTxt;
                    nFlag = REFFLDFLAG_ENDNOTE;
                    break;

                case REF_SETREFATTR:
                    sName = SW_RESSTR(STR_GETREFFLD);
                    nFlag = REF_SETREFATTR;
                    break;

                case REF_SEQUENCEFLD:
                    sName = ((SwGetRefField*)GetCurField())->GetSetRefName();
                    nFlag = REFFLDFLAG;
                    break;
            }

            if (m_pTypeLB->GetEntryPos(sName) == LISTBOX_ENTRY_NOTFOUND)   // reference to deleted mark
            {
                sal_Int32 nPos = m_pTypeLB->InsertEntry(sName);
                m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nFlag));
            }

            m_pTypeLB->SelectEntry(sName);
            SetTypeSel(m_pTypeLB->GetSelectEntryPos());
        }
        else
        {
            SetTypeSel(0);
            m_pTypeLB->SelectEntryPos(0);
        }
    }

    if (nOld != GetTypeSel())
    {
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        // fill selection-ListBox
        UpdateSubType();

        bool bName = false;
        nFldDlgFmtSel = 0;

        if ( ( !IsFldEdit() || m_pSelectionLB->GetEntryCount() ) &&
             nOld != LISTBOX_ENTRY_NOTFOUND )
        {
            m_pNameED->SetText(aEmptyOUStr);
            m_pValueED->SetText(aEmptyOUStr);
        }

        switch (nTypeId)
        {
            case TYP_GETREFFLD:
                if (REFFLDFLAG & (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(nOld))
                    // the old one stays
                    nFldDlgFmtSel = m_pFormatLB->GetSelectEntryPos();
                bName = true;
                break;

            case TYP_SETREFFLD:
                bName = true;
                break;

            case REFFLDFLAG_BOOKMARK:
                bName = true;
                // no break!!!
            default:
                if( REFFLDFLAG & nTypeId )
                {
                    const sal_uInt16 nOldId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(nOld);
                    if( nOldId & REFFLDFLAG || nOldId == TYP_GETREFFLD )
                        // then the old one stays
                        nFldDlgFmtSel = m_pFormatLB->GetSelectEntryPos();
                }
                break;
        }

        m_pNameED->Enable(bName);
        m_pNameFT->Enable(bName);

        // fill Format-Listbox
        sal_Int32 nSize = FillFormatLB(nTypeId);
        bool bFormat = nSize != 0;
        m_pFormat->Enable(bFormat);

        SubTypeHdl();
        ModifyHdl();
    }

    return 0;
}

IMPL_LINK_NOARG(SwFldRefPage, SubTypeHdl)
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    switch(nTypeId)
    {
        case TYP_GETREFFLD:
            if (!IsFldEdit() || m_pSelectionLB->GetSelectEntryCount())
            {
                m_pNameED->SetText(m_pSelectionLB->GetSelectEntry());
                ModifyHdl(m_pNameED);
            }
            break;

        case TYP_SETREFFLD:
        {
            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                m_pValueED->SetText(pSh->GetSelTxt());
            }

        }
        break;
        // #i83479#
        case REFFLDFLAG_HEADING:
        case REFFLDFLAG_NUMITEM:
        {
            if ( m_pSelectionToolTipLB->GetCurEntry() )
            {
                m_pNameED->SetText( m_pSelectionToolTipLB->GetEntryText(
                                        m_pSelectionToolTipLB->GetCurEntry() ) );
            }
        }
        break;

        default:
            if (!IsFldEdit() || m_pSelectionLB->GetSelectEntryCount())
                m_pNameED->SetText(m_pSelectionLB->GetSelectEntry());
            break;
    }

    return 0;
}

// renew types in SelectionLB
void SwFldRefPage::UpdateSubType()
{
    SwWrtShell *pSh = GetWrtShell();
    if(!pSh)
        pSh = ::GetActiveWrtShell();
    SwGetRefField* pRefFld = (SwGetRefField*)GetCurField();
    const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    OUString sOldSel;
    // #i83479#
    if ( m_pSelectionLB->IsVisible() )
    {
        const sal_Int32 nSelectionSel = m_pSelectionLB->GetSelectEntryPos();
        if (nSelectionSel != LISTBOX_ENTRY_NOTFOUND)
        {
            sOldSel = m_pSelectionLB->GetEntry(nSelectionSel);
        }
    }
    if (IsFldEdit() && sOldSel.isEmpty())
        sOldSel = OUString::number( pRefFld->GetSeqNo() + 1 );

    m_pSelectionLB->SetUpdateMode(false);
    m_pSelectionLB->Clear();
    // #i83479#
    m_pSelectionToolTipLB->SetUpdateMode(false);
    m_pSelectionToolTipLB->Clear();
    bool bShowSelectionToolTipLB( false );

    if( REFFLDFLAG & nTypeId )
    {
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // text marks!
        {
            m_pSelectionLB->SetStyle(m_pSelectionLB->GetStyle()|WB_SORT);
            // get all text marks
            IDocumentMarkAccess* const pMarkAccess = pSh->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getBookmarksBegin();
                ppMark != pMarkAccess->getBookmarksEnd();
                ++ppMark)
            {
                const ::sw::mark::IMark* pBkmk = ppMark->get();
                if(IDocumentMarkAccess::BOOKMARK == IDocumentMarkAccess::GetType(*pBkmk))
                    m_pSelectionLB->InsertEntry( pBkmk->GetName() );
            }
            if (IsFldEdit())
                sOldSel = pRefFld->GetSetRefName();
        }
        else if (nTypeId == REFFLDFLAG_FOOTNOTE)
        {
            m_pSelectionLB->SetStyle(m_pSelectionLB->GetStyle() & ~WB_SORT);
            SwSeqFldList aArr;
            const size_t nCnt = pSh->GetSeqFtnList( aArr );

            for( size_t n = 0; n < nCnt; ++n )
            {
                m_pSelectionLB->InsertEntry( aArr[ n ]->sDlgEntry );
                if (IsFldEdit() && pRefFld->GetSeqNo() == aArr[ n ]->nSeqNo)
                    sOldSel = aArr[n]->sDlgEntry;
            }
        }
        else if (nTypeId == REFFLDFLAG_ENDNOTE)
        {
            m_pSelectionLB->SetStyle(m_pSelectionLB->GetStyle() & ~WB_SORT);
            SwSeqFldList aArr;
            const size_t nCnt = pSh->GetSeqFtnList( aArr, true );

            for( size_t n = 0; n < nCnt; ++n )
            {
                m_pSelectionLB->InsertEntry( aArr[ n ]->sDlgEntry );
                if (IsFldEdit() && pRefFld->GetSeqNo() == aArr[ n ]->nSeqNo)
                    sOldSel = aArr[n]->sDlgEntry;
            }
        }
        // #i83479#
        else if ( nTypeId == REFFLDFLAG_HEADING )
        {
            bShowSelectionToolTipLB = true;

            const IDocumentOutlineNodes* pIDoc( pSh->getIDocumentOutlineNodesAccess() );
            pIDoc->getOutlineNodes( maOutlineNodes );
            bool bCertainTxtNodeSelected( false );
            SvTreeListEntry* pEntry = 0;
            for ( size_t nOutlIdx = 0; nOutlIdx < maOutlineNodes.size(); ++nOutlIdx )
            {
                pEntry = m_pSelectionToolTipLB->InsertEntry(
                                pIDoc->getOutlineText( nOutlIdx, true, true, false ) );
                pEntry->SetUserData( reinterpret_cast<void*>(nOutlIdx) );
                if ( ( IsFldEdit() &&
                       pRefFld->GetReferencedTxtNode() == maOutlineNodes[nOutlIdx] ) ||
                     mpSavedSelectedTxtNode == maOutlineNodes[nOutlIdx] )
                {
                    m_pSelectionToolTipLB->Select( pEntry );
                    sOldSel = "";
                    bCertainTxtNodeSelected = true;
                }
                else if ( !bCertainTxtNodeSelected && mnSavedSelectedPos == nOutlIdx )
                {
                    m_pSelectionToolTipLB->Select( pEntry );
                    sOldSel = "";
                }
            }
        }
        else if ( nTypeId == REFFLDFLAG_NUMITEM )
        {
            bShowSelectionToolTipLB = true;

            const IDocumentListItems* pIDoc( pSh->getIDocumentListItemsAccess() );
            pIDoc->getNumItems( maNumItems );
            bool bCertainTxtNodeSelected( false );
            SvTreeListEntry* pEntry = 0;
            for ( size_t nNumItemIdx = 0; nNumItemIdx < maNumItems.size(); ++nNumItemIdx )
            {
                pEntry = m_pSelectionToolTipLB->InsertEntry(
                            pIDoc->getListItemText( *maNumItems[nNumItemIdx], true, true ) );
                pEntry->SetUserData( reinterpret_cast<void*>(nNumItemIdx) );
                if ( ( IsFldEdit() &&
                       pRefFld->GetReferencedTxtNode() == maNumItems[nNumItemIdx]->GetTxtNode() ) ||
                     mpSavedSelectedTxtNode == maNumItems[nNumItemIdx]->GetTxtNode() )
                {
                    m_pSelectionToolTipLB->Select( pEntry );
                    sOldSel = "";
                    bCertainTxtNodeSelected = true;
                }
                else if ( !bCertainTxtNodeSelected && mnSavedSelectedPos == nNumItemIdx )
                {
                    m_pSelectionToolTipLB->Select( pEntry );
                    sOldSel = "";
                }
            }
        }
        else
        {
            m_pSelectionLB->SetStyle(m_pSelectionLB->GetStyle()|WB_SORT);
            // get the fields to Seq-FieldType:

            SwSetExpFieldType* pType = (SwSetExpFieldType*)pSh->GetFldType(
                                nTypeId & ~REFFLDFLAG, RES_SETEXPFLD );
            if( pType )
            {
                SwSeqFldList aArr;
                // old selection should be kept in non-edit mode
                if(IsFldEdit())
                    sOldSel = "";

                const size_t nCnt = pType->GetSeqFldList( aArr );
                for( size_t n = 0; n < nCnt; ++n )
                {
                    m_pSelectionLB->InsertEntry( aArr[ n ]->sDlgEntry );
                    if (IsFldEdit() && sOldSel.isEmpty() &&
                        aArr[ n ]->nSeqNo == pRefFld->GetSeqNo())
                        sOldSel = aArr[ n ]->sDlgEntry;
                }

                if (IsFldEdit() && sOldSel.isEmpty())
                    sOldSel = OUString::number( pRefFld->GetSeqNo() + 1);
            }
        }
    }
    else
    {
        std::vector<OUString> aLst;
        GetFldMgr().GetSubTypes(nTypeId, aLst);
        for(size_t i = 0; i < aLst.size(); ++i)
            m_pSelectionLB->InsertEntry(aLst[i]);

        if (IsFldEdit())
            sOldSel = pRefFld->GetSetRefName();
    }

    // #i83479#
    m_pSelectionToolTipLB->Show( bShowSelectionToolTipLB );
    m_pSelectionLB->Show( !bShowSelectionToolTipLB );
    if ( bShowSelectionToolTipLB )
    {
        m_pSelectionToolTipLB->SetUpdateMode(true);

        bool bEnable = m_pSelectionToolTipLB->GetEntryCount() != 0;
        m_pSelection->Enable( bEnable );

        if ( m_pSelectionToolTipLB->GetCurEntry() != 0 )
        {
            m_pSelectionToolTipLB->MakeVisible( m_pSelectionToolTipLB->GetCurEntry() );
        }

        if ( IsFldEdit() && m_pSelectionToolTipLB->GetCurEntry() == 0 )
        {
            m_pNameED->SetText(sOldSel);
        }
    }
    else
    {
        m_pSelectionLB->SetUpdateMode(true);

        // enable or disable
        bool bEnable = m_pSelectionLB->GetEntryCount() != 0;
        m_pSelection->Enable( bEnable );

        if ( bEnable )
        {
            m_pSelectionLB->SelectEntry(sOldSel);
            if (!m_pSelectionLB->GetSelectEntryCount() && !IsFldEdit())
                m_pSelectionLB->SelectEntryPos(0);
        }

        if (IsFldEdit() && !m_pSelectionLB->GetSelectEntryCount()) // in case the reference was already deleted...
            m_pNameED->SetText(sOldSel);
    }
}

sal_Int32 SwFldRefPage::FillFormatLB(sal_uInt16 nTypeId)
{
    OUString sOldSel;

    sal_Int32 nFormatSel = m_pFormatLB->GetSelectEntryPos();
    if (nFormatSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = m_pFormatLB->GetEntry(nFormatSel);

    // fill Format-Listbox
    m_pFormatLB->Clear();

    // reference has less that the annotation
    sal_uInt16 nSize( 0 );
    bool bAddCrossRefFormats( false );
    switch (nTypeId)
    {
        // #i83479#
        case REFFLDFLAG_HEADING:
        case REFFLDFLAG_NUMITEM:
            bAddCrossRefFormats = true;
            // intentional no break here

        case TYP_GETREFFLD:
        case REFFLDFLAG_BOOKMARK:
        case REFFLDFLAG_FOOTNOTE:
        case REFFLDFLAG_ENDNOTE:
            nSize = FMT_REF_PAGE_PGDSC - FMT_REF_BEGIN + 1;
            break;

        default:
            // #i83479#

            if ( REFFLDFLAG & nTypeId )
            {
                nSize = FMT_REF_ONLYSEQNO - FMT_REF_BEGIN + 1;
            }
            else
            {
                nSize = GetFldMgr().GetFormatCount( nTypeId, false, IsFldDlgHtmlMode() );
            }
            break;
    }

    if (REFFLDFLAG & nTypeId)
        nTypeId = TYP_GETREFFLD;

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        sal_Int32 nPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr( nTypeId, i ));
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, i )));
    }
    // #i83479#
    if ( bAddCrossRefFormats )
    {
        sal_uInt16 nFormat = FMT_REF_NUMBER - FMT_REF_BEGIN;
        sal_Int32 nPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr( nTypeId, nFormat ));
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, nFormat )));
        nFormat = FMT_REF_NUMBER_NO_CONTEXT - FMT_REF_BEGIN;
        nPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr( nTypeId, nFormat ));
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, nFormat )));
        nFormat = FMT_REF_NUMBER_FULL_CONTEXT - FMT_REF_BEGIN;
        nPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr( nTypeId, nFormat ));
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, nFormat )));
        nSize += 3;
    }

    // select a certain entry
    if (nSize)
    {
        if (!IsFldEdit())
            m_pFormatLB->SelectEntry(sOldSel);
        else
            m_pFormatLB->SelectEntry(SW_RESSTR(FMT_REF_BEGIN + GetCurField()->GetFormat()));

        if (!m_pFormatLB->GetSelectEntryCount())
        {
            m_pFormatLB->SelectEntryPos(nFldDlgFmtSel);
            if (!m_pFormatLB->GetSelectEntryCount())
                m_pFormatLB->SelectEntryPos(0);
        }
    }

    return nSize;
}

// Modify
IMPL_LINK_NOARG(SwFldRefPage, ModifyHdl)
{
    OUString aName(m_pNameED->GetText());
    const bool bEmptyName = aName.isEmpty();

    bool bEnable = true;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    if ((nTypeId == TYP_SETREFFLD && !GetFldMgr().CanInsertRefMark(aName)) ||
        (bEmptyName && (nTypeId == TYP_GETREFFLD || nTypeId == TYP_SETREFFLD ||
                       nTypeId == REFFLDFLAG_BOOKMARK)))
        bEnable = false;

    EnableInsert(bEnable);

    m_pSelectionLB->SelectEntry(aName);

    return 0;
}

bool SwFldRefPage::FillItemSet(SfxItemSet* )
{
    bool bModified = false;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    sal_uInt16 nSubType = 0;
    const sal_Int32 nEntryPos = m_pFormatLB->GetSelectEntryPos();
    const sal_uLong nFormat = (nEntryPos == LISTBOX_ENTRY_NOTFOUND)
        ? 0 : (sal_uLong)m_pFormatLB->GetEntryData(nEntryPos);

    OUString aVal(m_pValueED->GetText());
    OUString aName(m_pNameED->GetText());

    switch(nTypeId)
    {
        case TYP_GETREFFLD:
            nSubType = REF_SETREFATTR;
            break;

        case TYP_SETREFFLD:
        {
            SwFieldType* pType = GetFldMgr().GetFldType(RES_SETEXPFLD, aName);

            if(!pType)  // Only insert when the name doesn't exist yet
            {
                m_pSelectionLB->InsertEntry(aName);
                m_pSelection->Enable();
            }
            break;
        }
    }

    SwGetRefField* pRefFld = (SwGetRefField*)GetCurField();

    if (REFFLDFLAG & nTypeId)
    {
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
        {
            pSh = ::GetActiveWrtShell();
        }
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // text marks!
        {
            aName = m_pNameED->GetText();
            nTypeId = TYP_GETREFFLD;
            nSubType = REF_BOOKMARK;
        }
        else if (REFFLDFLAG_FOOTNOTE == nTypeId)        // footnotes
        {
            SwSeqFldList aArr;
            _SeqFldLstElem aElem( m_pSelectionLB->GetSelectEntry(), 0 );

            size_t nPos = 0;

            nTypeId = TYP_GETREFFLD;
            nSubType = REF_FOOTNOTE;
            aName = "";

            if (pSh->GetSeqFtnList(aArr) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = OUString::number( aArr[nPos]->nSeqNo );

                if (IsFldEdit() && aArr[nPos]->nSeqNo == pRefFld->GetSeqNo())
                    bModified = true; // can happen with fields of which the references were deleted
            }
            else if (IsFldEdit())
                aVal = OUString::number( pRefFld->GetSeqNo() );
        }
        else if (REFFLDFLAG_ENDNOTE == nTypeId)         // endnotes
        {
            SwSeqFldList aArr;
            _SeqFldLstElem aElem( m_pSelectionLB->GetSelectEntry(), 0 );

            size_t nPos = 0;

            nTypeId = TYP_GETREFFLD;
            nSubType = REF_ENDNOTE;
            aName = "";

            if (pSh->GetSeqFtnList(aArr, true) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = OUString::number( aArr[nPos]->nSeqNo );

                if (IsFldEdit() && aArr[nPos]->nSeqNo == pRefFld->GetSeqNo())
                    bModified = true; // can happen with fields of which the reference was deleted
            }
            else if (IsFldEdit())
                aVal = OUString::number( pRefFld->GetSeqNo() );
        }
        // #i83479#
        else if ( nTypeId == REFFLDFLAG_HEADING )
        {
            SvTreeListEntry* pEntry = m_pSelectionToolTipLB->GetCurEntry();
            OSL_ENSURE( pEntry,
                    "<SwFldRefPage::FillItemSet(..)> - no entry selected in selection tool tip listbox!" );
            if ( pEntry )
            {
                const size_t nOutlIdx( static_cast<size_t>
                    (reinterpret_cast<sal_uLong>(pEntry->GetUserData())) );
                pSh->getIDocumentOutlineNodesAccess()->getOutlineNodes( maOutlineNodes );
                if ( nOutlIdx < maOutlineNodes.size() )
                {
                    ::sw::mark::IMark const * const pMark = pSh->getIDocumentMarkAccess()->getMarkForTxtNode(
                        *(maOutlineNodes[nOutlIdx]),
                        IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK);
                    aName = pMark->GetName();
                    nTypeId = TYP_GETREFFLD;
                    nSubType = REF_BOOKMARK;
                }
            }
        }
        else if ( nTypeId == REFFLDFLAG_NUMITEM )
        {
            SvTreeListEntry* pEntry = m_pSelectionToolTipLB->GetCurEntry();
            OSL_ENSURE( pEntry,
                    "<SwFldRefPage::FillItemSet(..)> - no entry selected in selection tool tip listbox!" );
            if ( pEntry )
            {
                const size_t nNumItemIdx( static_cast<size_t>
                    (reinterpret_cast<sal_uLong>(pEntry->GetUserData())) );
                pSh->getIDocumentListItemsAccess()->getNumItems( maNumItems );
                if ( nNumItemIdx < maNumItems.size() )
                {
                    ::sw::mark::IMark const * const pMark = pSh->getIDocumentMarkAccess()->getMarkForTxtNode(
                        *(maNumItems[nNumItemIdx]->GetTxtNode()),
                        IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK);
                    aName = pMark->GetName();
                    nTypeId = TYP_GETREFFLD;
                    nSubType = REF_BOOKMARK;
                }
            }
        }
        else                                // SeqenceFields
        {
            // get fields for Seq-FeldType:
            SwSetExpFieldType* pType = (SwSetExpFieldType*)pSh->GetFldType(
                                    nTypeId & ~REFFLDFLAG, RES_SETEXPFLD );
            if( pType )
            {
                SwSeqFldList aArr;
                _SeqFldLstElem aElem( m_pSelectionLB->GetSelectEntry(), 0 );

                size_t nPos = 0;

                nTypeId = TYP_GETREFFLD;
                nSubType = REF_SEQUENCEFLD;
                aName = pType->GetName();

                if (pType->GetSeqFldList(aArr) && aArr.SeekEntry(aElem, &nPos))
                {
                    aVal = OUString::number( aArr[nPos]->nSeqNo );

                    if (IsFldEdit() && aArr[nPos]->nSeqNo == pRefFld->GetSeqNo())
                        bModified = true; // can happen with fields of which the reference was deleted
                }
                else if (IsFldEdit())
                    aVal = OUString::number( pRefFld->GetSeqNo() );
            }
        }
    }

    if (IsFldEdit() && nTypeId == TYP_GETREFFLD)
    {
        aVal = OUString::number(nSubType) + "|" + aVal;
    }

    if (!IsFldEdit() || bModified ||
        m_pNameED->IsValueChangedFromSaved() ||
        m_pValueED->IsValueChangedFromSaved() ||
        m_pTypeLB->IsValueChangedFromSaved() ||
        m_pSelectionLB->IsValueChangedFromSaved() ||
        m_pFormatLB->IsValueChangedFromSaved())
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat );
    }

    ModifyHdl();    // enable/disable insert if applicable

    return false;
}

SfxTabPage* SwFldRefPage::Create(   vcl::Window* pParent,
                        const SfxItemSet* rAttrSet )
{
    return ( new SwFldRefPage( pParent, *rAttrSet ) );
}

sal_uInt16 SwFldRefPage::GetGroup()
{
    return GRP_REF;
}

void    SwFldRefPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_pTypeLB->GetSelectEntryPos();
    const sal_uInt16 nTypeSel = ( LISTBOX_ENTRY_NOTFOUND == nEntryPos )
        ? USHRT_MAX
        : sal::static_int_cast< sal_uInt16 >
            (reinterpret_cast< sal_uIntPtr >(m_pTypeLB->GetEntryData( nEntryPos )));
    SetUserData( USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
