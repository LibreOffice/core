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

#include <swtypes.hxx>
#include <view.hxx>
#include <IMark.hxx>
#include <expfld.hxx>
#include <swmodule.hxx>
#include "fldref.hxx"
#include <reffld.hxx>
#include <wrtsh.hxx>

#include <fldref.hrc>
#include <globals.hrc>
#include <strings.hrc>
#include <SwNodeNum.hxx>
#include <IDocumentMarkAccess.hxx>
#include <ndtxt.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/charclass.hxx>

#include <comphelper/string.hxx>

#include <vcl/treelistentry.hxx>

#define REFFLDFLAG          0x4000
#define REFFLDFLAG_BOOKMARK 0x4800
#define REFFLDFLAG_FOOTNOTE 0x5000
#define REFFLDFLAG_ENDNOTE  0x6000
// #i83479#
#define REFFLDFLAG_HEADING  0x7100
#define REFFLDFLAG_NUMITEM  0x7200

static sal_uInt16 nFieldDlgFormatSel = 0;

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFieldRefPage::SwFieldRefPage(TabPageParent pParent, const SfxItemSet *const pCoreSet )
    : SwFieldPage(pParent, "modules/swriter/ui/fldrefpage.ui", "FieldRefPage", pCoreSet)
    , maOutlineNodes()
    , maNumItems()
    , mpSavedSelectedTextNode(nullptr)
    , mnSavedSelectedPos(0)
    , m_xTypeLB(m_xBuilder->weld_tree_view("type"))
    , m_xSelection(m_xBuilder->weld_widget("selectframe"))
    , m_xSelectionLB(m_xBuilder->weld_tree_view("select"))
    , m_xSelectionToolTipLB(m_xBuilder->weld_tree_view("selecttip"))
    , m_xFormat(m_xBuilder->weld_widget("formatframe"))
    , m_xFormatLB(m_xBuilder->weld_tree_view("format"))
    , m_xNameFT(m_xBuilder->weld_label("nameft"))
    , m_xNameED(m_xBuilder->weld_entry("name"))
    , m_xValueED(m_xBuilder->weld_entry("value"))
    , m_xFilterED(m_xBuilder->weld_entry("filter"))
{
    m_xSelectionLB->make_sorted();
    // #i83479#
    for (size_t i = 0; i < SAL_N_ELEMENTS(FLD_REF_PAGE_TYPES); ++i)
    {
        m_xTypeLB->append_text(SwResId(FLD_REF_PAGE_TYPES[i]));
        m_xFormatLB->append_text(SwResId(FLD_REF_PAGE_TYPES[i]));
    }

    sBookmarkText = m_xTypeLB->get_text(0);
    sFootnoteText = m_xTypeLB->get_text(1);
    sEndnoteText = m_xTypeLB->get_text(2);
    // #i83479#
    sHeadingText = m_xTypeLB->get_text(3);
    sNumItemText = m_xTypeLB->get_text(4);

    auto nHeight = m_xTypeLB->get_height_rows(8);
    auto nWidth = LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MapUnit::MapAppFont)).Width();
    m_xTypeLB->set_size_request(nWidth, nHeight);
    m_xFormatLB->set_size_request(nWidth, nHeight);
    m_xSelection->set_size_request(nWidth * 2, nHeight);
    nHeight = m_xTypeLB->get_height_rows(20);
    m_xSelectionToolTipLB->set_size_request(nHeight, nWidth*2);

    m_xTypeLB->clear();

    m_xNameED->connect_changed(LINK(this, SwFieldRefPage, ModifyHdl));
    m_xFilterED->connect_changed( LINK( this, SwFieldRefPage, ModifyHdl_Impl ) );

    m_xTypeLB->connect_row_activated(LINK(this, SwFieldRefPage, TreeViewInsertHdl));
    m_xTypeLB->connect_changed(LINK(this, SwFieldRefPage, TypeHdl));
    m_xSelectionLB->connect_changed(LINK(this, SwFieldRefPage, SubTypeListBoxHdl));
    m_xSelectionLB->connect_row_activated(LINK(this, SwFieldRefPage, TreeViewInsertHdl));
    m_xFormatLB->connect_row_activated(LINK(this, SwFieldRefPage, TreeViewInsertHdl));

    // #i83479#
    m_xSelectionToolTipLB->connect_changed( LINK(this, SwFieldRefPage, SubTypeTreeListBoxHdl) );
    m_xSelectionToolTipLB->connect_row_activated( LINK(this, SwFieldRefPage, TreeViewInsertHdl) );
    m_xFilterED->grab_focus();
}

SwFieldRefPage::~SwFieldRefPage()
{
    disposeOnce();
}

IMPL_LINK_NOARG(SwFieldRefPage, ModifyHdl_Impl, weld::Entry&, void)
{
    UpdateSubType(comphelper::string::strip(m_xFilterED->get_text(), ' '));
}

// #i83479#
void SwFieldRefPage::SaveSelectedTextNode()
{
    mpSavedSelectedTextNode = nullptr;
    mnSavedSelectedPos = 0;
    if ( m_xSelectionToolTipLB->get_visible() )
    {
        int nEntry = m_xSelectionToolTipLB->get_selected_index();
        if (nEntry != -1)
        {
            const sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

            if ( nTypeId == REFFLDFLAG_HEADING )
            {
                mnSavedSelectedPos = m_xSelectionToolTipLB->get_id(nEntry).toUInt32();
                if ( mnSavedSelectedPos < maOutlineNodes.size() )
                {
                    mpSavedSelectedTextNode = maOutlineNodes[mnSavedSelectedPos];
                }
            }
            else if ( nTypeId == REFFLDFLAG_NUMITEM )
            {
                mnSavedSelectedPos = m_xSelectionToolTipLB->get_id(nEntry).toUInt32();
                if ( mnSavedSelectedPos < maNumItems.size() )
                {
                    mpSavedSelectedTextNode = maNumItems[mnSavedSelectedPos]->GetTextNode();
                }
            }
        }
    }
}

void SwFieldRefPage::Reset(const SfxItemSet* )
{
    if (!IsFieldEdit())
    {
        SavePos(*m_xTypeLB);
        // #i83479#
        SaveSelectedTextNode();
    }
    SetSelectionSel(-1);
    SetTypeSel(-1);
    Init(); // general initialisation

    // initialise TypeListBox
    m_xTypeLB->freeze();
    m_xTypeLB->clear();

    // fill Type-Listbox

    // set/insert reference
    const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

    for (short i = rRg.nStart; i < rRg.nEnd; ++i)
    {
        const sal_uInt16 nTypeId = SwFieldMgr::GetTypeId(i);

        if (!IsFieldEdit() || nTypeId != TYP_SETREFFLD)
        {
            m_xTypeLB->append(OUString::number(nTypeId), SwFieldMgr::GetTypeStr(i));
        }
    }

    // #i83479#
    // entries for headings and numbered items
    m_xTypeLB->append(OUString::number(REFFLDFLAG_HEADING), sHeadingText);
    m_xTypeLB->append(OUString::number(REFFLDFLAG_NUMITEM), sNumItemText);

    // fill up with the sequence types
    SwWrtShell *pSh = GetWrtShell();
    if (!pSh)
        pSh = ::GetActiveWrtShell();

    if (!pSh)
        return;

    const size_t nFieldTypeCnt = pSh->GetFieldTypeCount(SwFieldIds::SetExp);

    OSL_ENSURE( nFieldTypeCnt < static_cast<size_t>(REFFLDFLAG), "<SwFieldRefPage::Reset> - Item index will overlap flags!" );

    for (size_t n = 0; n < nFieldTypeCnt; ++n)
    {
        SwSetExpFieldType* pType = static_cast<SwSetExpFieldType*>(pSh->GetFieldType(n, SwFieldIds::SetExp));

        if ((nsSwGetSetExpType::GSE_SEQ & pType->GetType()) && pType->HasWriterListeners() && pSh->IsUsed(*pType))
        {
            m_xTypeLB->append(OUString::number(REFFLDFLAG | n), pType->GetName());
        }
    }

    // text marks - now always (because of globaldocuments)
    m_xTypeLB->append(OUString::number(REFFLDFLAG_BOOKMARK), sBookmarkText);

    // footnotes:
    if( pSh->HasFootnotes() )
    {
        m_xTypeLB->append(OUString::number(REFFLDFLAG_FOOTNOTE), sFootnoteText);
    }

    // endnotes:
    if ( pSh->HasFootnotes(true) )
    {
        m_xTypeLB->append(OUString::number(REFFLDFLAG_ENDNOTE), sEndnoteText);
    }

    m_xTypeLB->thaw();

    // select old Pos
    if (!IsFieldEdit())
        RestorePos(*m_xTypeLB);

    nFieldDlgFormatSel = 0;

    sal_uInt16 nFormatBoxPosition = USHRT_MAX;
    if( !IsRefresh() )
    {
        sal_Int32 nIdx{ 0 };
        const OUString sUserData = GetUserData();
        if(!IsRefresh() && sUserData.getToken(0, ';', nIdx).
                                equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            const sal_uInt16 nVal = static_cast< sal_uInt16 >(sUserData.getToken(0, ';', nIdx).toInt32());
            if(nVal != USHRT_MAX)
            {
                for(sal_Int32 i = 0, nEntryCount = m_xTypeLB->n_children(); i < nEntryCount; ++i)
                {
                    if (nVal == m_xTypeLB->get_id(i).toUInt32())
                    {
                        m_xTypeLB->select(i);
                        break;
                    }
                }
                if (nIdx>=0 && nIdx<sUserData.getLength())
                {
                    nFormatBoxPosition = static_cast< sal_uInt16 >(sUserData.getToken(0, ';', nIdx).toInt32());
                }
            }
        }
    }
    TypeHdl(*m_xTypeLB);
    if(nFormatBoxPosition != USHRT_MAX)
    {
        m_xFormatLB->select(nFormatBoxPosition);
    }
    if (IsFieldEdit())
    {
        m_xTypeLB->save_value();
        m_xSelectionLB->save_value();
        m_xFormatLB->save_value();
        m_xNameED->save_value();
        m_xValueED->save_value();
        m_xFilterED->set_text(OUString());
    }
}

IMPL_LINK_NOARG(SwFieldRefPage, TypeHdl, weld::TreeView&, void)
{
    // save old ListBoxPos
    const sal_Int32 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_xTypeLB->get_selected_index());

    if(GetTypeSel() == -1)
    {
        if (IsFieldEdit())
        {
            // select positions
            OUString sName;
            sal_uInt16 nFlag = 0;

            switch( GetCurField()->GetSubType() )
            {
                case REF_BOOKMARK:
                {
                    // #i83479#
                    SwGetRefField* pRefField = dynamic_cast<SwGetRefField*>(GetCurField());
                    if ( pRefField &&
                         pRefField->IsRefToHeadingCrossRefBookmark() )
                    {
                        sName = sHeadingText;
                        nFlag = REFFLDFLAG_HEADING;
                    }
                    else if ( pRefField &&
                              pRefField->IsRefToNumItemCrossRefBookmark() )
                    {
                        sName = sNumItemText;
                        nFlag = REFFLDFLAG_NUMITEM;
                    }
                    else
                    {
                        sName = sBookmarkText;
                        nFlag = REFFLDFLAG_BOOKMARK;
                    }
                }
                break;

                case REF_FOOTNOTE:
                    sName = sFootnoteText;
                    nFlag = REFFLDFLAG_FOOTNOTE;
                    break;

                case REF_ENDNOTE:
                    sName = sEndnoteText;
                    nFlag = REFFLDFLAG_ENDNOTE;
                    break;

                case REF_SETREFATTR:
                    sName = SwResId(STR_GETREFFLD);
                    nFlag = REF_SETREFATTR;
                    break;

                case REF_SEQUENCEFLD:
                    sName = static_cast<SwGetRefField*>(GetCurField())->GetSetRefName();
                    nFlag = REFFLDFLAG;
                    break;
            }

            if (m_xTypeLB->find_text(sName) == -1)   // reference to deleted mark
            {
                m_xTypeLB->append(OUString::number(nFlag), sName);
            }

            m_xTypeLB->select_text(sName);
            SetTypeSel(m_xTypeLB->get_selected_index());
        }
        else
        {
            SetTypeSel(0);
            m_xTypeLB->select(0);
        }
    }

    if (nOld == GetTypeSel())
        return;

    sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    // fill selection-ListBox
    UpdateSubType(comphelper::string::strip(m_xFilterED->get_text(), ' '));

    bool bName = false;
    nFieldDlgFormatSel = 0;

    if ( ( !IsFieldEdit() || m_xSelectionLB->n_children() ) &&
         nOld != -1 )
    {
        m_xNameED->set_text(OUString());
        m_xValueED->set_text(OUString());
        m_xFilterED->set_text(OUString());
    }

    switch (nTypeId)
    {
        case TYP_GETREFFLD:
            if (REFFLDFLAG & m_xTypeLB->get_id(nOld).toUInt32())
                // the old one stays
                nFieldDlgFormatSel = m_xFormatLB->get_selected_index();
            bName = true;
            break;

        case TYP_SETREFFLD:
            bName = true;
            break;

        case REFFLDFLAG_BOOKMARK:
            bName = true;
            [[fallthrough]];
        default:
            if( REFFLDFLAG & nTypeId )
            {
                const sal_uInt16 nOldId = m_xTypeLB->get_id(nOld).toUInt32();
                if( nOldId & REFFLDFLAG || nOldId == TYP_GETREFFLD )
                    // then the old one stays
                    nFieldDlgFormatSel = m_xFormatLB->get_selected_index();
            }
            break;
    }

    m_xNameED->set_sensitive(bName);
    m_xNameFT->set_sensitive(bName);

    // fill Format-Listbox
    sal_Int32 nSize = FillFormatLB(nTypeId);
    bool bFormat = nSize != 0;
    m_xFormat->set_sensitive(bFormat);

    SubTypeHdl();
    ModifyHdl(*m_xNameED);
    ModifyHdl(*m_xFilterED);
}

IMPL_LINK_NOARG(SwFieldRefPage, SubTypeTreeListBoxHdl, weld::TreeView&, void)
{
    SubTypeHdl();
}

IMPL_LINK_NOARG(SwFieldRefPage, SubTypeListBoxHdl, weld::TreeView&, void)
{
    SubTypeHdl();
}

void SwFieldRefPage::SubTypeHdl()
{
    sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    switch(nTypeId)
    {
        case TYP_GETREFFLD:
            if (!IsFieldEdit() || m_xSelectionLB->get_selected_index() != -1)
            {
                m_xNameED->set_text(m_xSelectionLB->get_selected_text());
                ModifyHdl(*m_xNameED);
            }
            break;

        case TYP_SETREFFLD:
        {
            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                m_xValueED->set_text(pSh->GetSelText());
            }

        }
        break;
        // #i83479#
        case REFFLDFLAG_HEADING:
        case REFFLDFLAG_NUMITEM:
        {
            int nEntry = m_xSelectionToolTipLB->get_selected_index();
            if (nEntry != -1)
                m_xNameED->set_text(m_xSelectionToolTipLB->get_text(nEntry));
        }
        break;

        default:
            if (!IsFieldEdit() || m_xSelectionLB->get_selected_index() != -1)
                m_xNameED->set_text(m_xSelectionLB->get_selected_text());
            break;
    }
}

// renew types in SelectionLB after filtering
void SwFieldRefPage::UpdateSubType(const OUString& filterString)
{
    SwWrtShell *pSh = GetWrtShell();
    if(!pSh)
        pSh = ::GetActiveWrtShell();
    SwGetRefField* pRefField = static_cast<SwGetRefField*>(GetCurField());
    const sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    OUString sOldSel;
    // #i83479#
    if ( m_xSelectionLB->get_visible() )
    {
        const sal_Int32 nSelectionSel = m_xSelectionLB->get_selected_index();
        if (nSelectionSel != -1)
            sOldSel = m_xSelectionLB->get_text(nSelectionSel);
    }
    if (IsFieldEdit() && sOldSel.isEmpty())
        sOldSel = OUString::number( pRefField->GetSeqNo() + 1 );

    m_xSelectionLB->freeze();
    m_xSelectionLB->clear();
    // #i83479#
    m_xSelectionToolTipLB->freeze();
    m_xSelectionToolTipLB->clear();
    OUString m_sSelectionToolTipLBId;
    bool bShowSelectionToolTipLB( false );

    if( REFFLDFLAG & nTypeId )
    {
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // text marks!
        {
            m_xSelectionLB->make_sorted();
            // get all text marks
            IDocumentMarkAccess* const pMarkAccess = pSh->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getBookmarksBegin();
                ppMark != pMarkAccess->getBookmarksEnd();
                ++ppMark)
            {
                const ::sw::mark::IMark* pBkmk = ppMark->get();
                if(IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(*pBkmk))
                {
                    bool isSubstring = MatchSubstring(pBkmk->GetName(), filterString);
                    if(isSubstring)
                    {
                        m_xSelectionLB->append_text( pBkmk->GetName() );
                    }
                }
            }
            if (IsFieldEdit())
                sOldSel = pRefField->GetSetRefName();
        }
        else if (nTypeId == REFFLDFLAG_FOOTNOTE)
        {
            m_xSelectionLB->make_unsorted();
            SwSeqFieldList aArr;
            const size_t nCnt = pSh->GetSeqFootnoteList( aArr );

            for( size_t n = 0; n < nCnt; ++n )
            {
                bool isSubstring = MatchSubstring(aArr[ n ].sDlgEntry, filterString);
                if(isSubstring)
                {
                    m_xSelectionLB->append_text( aArr[ n ].sDlgEntry );
                }
                if (IsFieldEdit() && pRefField->GetSeqNo() == aArr[ n ].nSeqNo)
                    sOldSel = aArr[n].sDlgEntry;
            }
        }
        else if (nTypeId == REFFLDFLAG_ENDNOTE)
        {
            m_xSelectionLB->make_unsorted();
            SwSeqFieldList aArr;
            const size_t nCnt = pSh->GetSeqFootnoteList( aArr, true );

            for( size_t n = 0; n < nCnt; ++n )
            {
                bool isSubstring = MatchSubstring(aArr[ n ].sDlgEntry, filterString);
                if(isSubstring)
                {
                    m_xSelectionLB->append_text( aArr[ n ].sDlgEntry );
                }
                if (IsFieldEdit() && pRefField->GetSeqNo() == aArr[ n ].nSeqNo)
                    sOldSel = aArr[n].sDlgEntry;
            }
        }
        // #i83479#
        else if ( nTypeId == REFFLDFLAG_HEADING )
        {
            bShowSelectionToolTipLB = true;

            const IDocumentOutlineNodes* pIDoc( pSh->getIDocumentOutlineNodesAccess() );
            pIDoc->getOutlineNodes( maOutlineNodes );
            bool bCertainTextNodeSelected( false );
            for ( size_t nOutlIdx = 0; nOutlIdx < maOutlineNodes.size(); ++nOutlIdx )
            {
                if (!pIDoc->isOutlineInLayout(nOutlIdx, *pSh->GetLayout()))
                {
                    continue; // skip it
                }
                bool isSubstring = MatchSubstring(pIDoc->getOutlineText(nOutlIdx, pSh->GetLayout(), true, true, false), filterString);
                if(isSubstring)
                {
                    OUString sId(OUString::number(nOutlIdx));
                    m_xSelectionToolTipLB->append(sId,
                        pIDoc->getOutlineText(nOutlIdx, pSh->GetLayout(), true, true, false));
                    if ( ( IsFieldEdit() &&
                       pRefField->GetReferencedTextNode() == maOutlineNodes[nOutlIdx] ) ||
                        mpSavedSelectedTextNode == maOutlineNodes[nOutlIdx] )
                    {
                        m_sSelectionToolTipLBId = sId;
                        sOldSel.clear();
                        bCertainTextNodeSelected = true;
                    }
                    else if ( !bCertainTextNodeSelected && mnSavedSelectedPos == nOutlIdx )
                    {
                        m_sSelectionToolTipLBId = sId;
                        sOldSel.clear();
                    }
                }
            }
        }
        else if ( nTypeId == REFFLDFLAG_NUMITEM )
        {
            bShowSelectionToolTipLB = true;

            const IDocumentListItems* pIDoc( pSh->getIDocumentListItemsAccess() );
            pIDoc->getNumItems( maNumItems );
            bool bCertainTextNodeSelected( false );
            for ( size_t nNumItemIdx = 0; nNumItemIdx < maNumItems.size(); ++nNumItemIdx )
            {
                if (!pIDoc->isNumberedInLayout(*maNumItems[nNumItemIdx], *pSh->GetLayout()))
                {
                    continue; // skip it
                }
                bool isSubstring = MatchSubstring(pIDoc->getListItemText(*maNumItems[nNumItemIdx], *pSh->GetLayout()), filterString);
                if(isSubstring)
                {
                    OUString sId(OUString::number(nNumItemIdx));
                    m_xSelectionToolTipLB->append(sId,
                        pIDoc->getListItemText(*maNumItems[nNumItemIdx], *pSh->GetLayout()));
                    if ( ( IsFieldEdit() &&
                           pRefField->GetReferencedTextNode() == maNumItems[nNumItemIdx]->GetTextNode() ) ||
                        mpSavedSelectedTextNode == maNumItems[nNumItemIdx]->GetTextNode() )
                    {
                        m_sSelectionToolTipLBId = sId;
                        sOldSel.clear();
                        bCertainTextNodeSelected = true;
                    }
                    else if ( !bCertainTextNodeSelected && mnSavedSelectedPos == nNumItemIdx )
                    {
                        m_sSelectionToolTipLBId = sId;
                        sOldSel.clear();
                    }
                }
            }
        }
        else
        {
            m_xSelectionLB->make_sorted();
            // get the fields to Seq-FieldType:

            SwSetExpFieldType* pType = static_cast<SwSetExpFieldType*>(pSh->GetFieldType(
                                nTypeId & ~REFFLDFLAG, SwFieldIds::SetExp ));
            if( pType )
            {
                SwSeqFieldList aArr;
                // old selection should be kept in non-edit mode
                if(IsFieldEdit())
                    sOldSel.clear();

                const size_t nCnt = pType->GetSeqFieldList(aArr, pSh->GetLayout());
                for( size_t n = 0; n < nCnt; ++n )
                {
                    bool isSubstring = MatchSubstring(aArr[ n ].sDlgEntry, filterString);
                    if(isSubstring)
                    {
                        m_xSelectionLB->append_text( aArr[ n ].sDlgEntry );
                    }
                    if (IsFieldEdit() && sOldSel.isEmpty() &&
                        aArr[ n ].nSeqNo == pRefField->GetSeqNo())
                        sOldSel = aArr[ n ].sDlgEntry;
                }

                if (IsFieldEdit() && sOldSel.isEmpty())
                    sOldSel = OUString::number( pRefField->GetSeqNo() + 1);
            }
        }
    }
    else
    {
        std::vector<OUString> aLst;
        GetFieldMgr().GetSubTypes(nTypeId, aLst);
        for(const OUString & i : aLst)
        {
            bool isSubstring = MatchSubstring( i , filterString );
            if(isSubstring)
            {
                m_xSelectionLB->append_text(i);
            }
        }

        if (IsFieldEdit())
            sOldSel = pRefField->GetSetRefName();
    }

    // #i83479#
    m_xSelectionLB->thaw();
    m_xSelectionToolTipLB->thaw();
    if (!m_sSelectionToolTipLBId.isEmpty())
        m_xSelectionToolTipLB->select_id(m_sSelectionToolTipLBId);
    m_xSelectionToolTipLB->set_visible( bShowSelectionToolTipLB );
    m_xSelectionLB->set_visible( !bShowSelectionToolTipLB );
    if ( bShowSelectionToolTipLB )
    {
        bool bEnable = m_xSelectionToolTipLB->n_children() != 0;
        m_xSelection->set_sensitive( bEnable );

        int nEntry = m_xSelectionToolTipLB->get_selected_index();
        if (nEntry != -1)
            m_xSelectionToolTipLB->scroll_to_row(nEntry);

        if (IsFieldEdit() && nEntry == -1)
        {
            m_xNameED->set_text(sOldSel);
        }
    }
    else
    {
        // enable or disable
        bool bEnable = m_xSelectionLB->n_children() != 0;
        m_xSelection->set_sensitive( bEnable );

        if ( bEnable )
        {
            m_xSelectionLB->select_text(sOldSel);
            if (m_xSelectionLB->get_selected_index() == -1 && !IsFieldEdit())
                m_xSelectionLB->select(0);
        }

        if (IsFieldEdit() && m_xSelectionLB->get_selected_index() == -1) // in case the reference was already deleted...
            m_xNameED->set_text(sOldSel);
    }
}

bool SwFieldRefPage::MatchSubstring( const OUString& rListString, const OUString& rSubstr )
{
    if(rSubstr.isEmpty())
        return true;
    OUString aListString = GetAppCharClass().lowercase(rListString);
    OUString aSubstr = GetAppCharClass().lowercase(rSubstr);
    return aListString.indexOf(aSubstr) >= 0;
}

enum FMT_REF_IDX
{
    FMT_REF_PAGE_IDX                = 0,
    FMT_REF_CHAPTER_IDX             = 1,
    FMT_REF_TEXT_IDX                = 2,
    FMT_REF_UPDOWN_IDX              = 3,
    FMT_REF_PAGE_PGDSC_IDX          = 4,
    FMT_REF_ONLYNUMBER_IDX          = 5,
    FMT_REF_ONLYCAPTION_IDX         = 6,
    FMT_REF_ONLYSEQNO_IDX           = 7,
    FMT_REF_NUMBER_IDX              = 8,
    FMT_REF_NUMBER_NO_CONTEXT_IDX   = 9,
    FMT_REF_NUMBER_FULL_CONTEXT_IDX = 10
};

static const char* FMT_REF_ARY[] =
{
    FMT_REF_PAGE,
    FMT_REF_CHAPTER,
    FMT_REF_TEXT,
    FMT_REF_UPDOWN,
    FMT_REF_PAGE_PGDSC,
    FMT_REF_ONLYNUMBER,
    FMT_REF_ONLYCAPTION,
    FMT_REF_ONLYSEQNO,
    FMT_REF_NUMBER,
    FMT_REF_NUMBER_NO_CONTEXT,
    FMT_REF_NUMBER_FULL_CONTEXT
};

sal_Int32 SwFieldRefPage::FillFormatLB(sal_uInt16 nTypeId)
{
    OUString sOldSel;

    sal_Int32 nFormatSel = m_xFormatLB->get_selected_index();
    if (nFormatSel != -1)
        sOldSel = m_xFormatLB->get_text(nFormatSel);

    // fill Format-Listbox
    m_xFormatLB->clear();

    // reference has less that the annotation
    sal_uInt16 nSize( 0 );
    bool bAddCrossRefFormats( false );
    switch (nTypeId)
    {
        // #i83479#
        case REFFLDFLAG_HEADING:
        case REFFLDFLAG_NUMITEM:
            bAddCrossRefFormats = true;
            [[fallthrough]];

        case TYP_GETREFFLD:
        case REFFLDFLAG_BOOKMARK:
        case REFFLDFLAG_FOOTNOTE:
        case REFFLDFLAG_ENDNOTE:
            nSize = FMT_REF_PAGE_PGDSC_IDX + 1;
            break;

        default:
            // #i83479#

            if ( REFFLDFLAG & nTypeId )
            {
                nSize = FMT_REF_ONLYSEQNO_IDX + 1;
            }
            else
            {
                nSize = GetFieldMgr().GetFormatCount( nTypeId, IsFieldDlgHtmlMode() );
            }
            break;
    }

    if (REFFLDFLAG & nTypeId)
        nTypeId = TYP_GETREFFLD;

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        OUString sId(OUString::number(GetFieldMgr().GetFormatId( nTypeId, i )));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr(nTypeId, i));
    }
    // #i83479#

    sal_uInt16 nExtraSize( 0 );
    if ( bAddCrossRefFormats )
    {
        sal_uInt16 nFormat = FMT_REF_NUMBER_IDX;
        OUString sId(OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat)));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr( nTypeId, nFormat ));
        nFormat = FMT_REF_NUMBER_NO_CONTEXT_IDX;
        sId = OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr( nTypeId, nFormat ));
        nFormat = FMT_REF_NUMBER_FULL_CONTEXT_IDX;
        sId = OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr( nTypeId, nFormat ));
        nExtraSize = 3;
    }

    // extra list items optionally, depending from reference-language
    SvtSysLocaleOptions aSysLocaleOptions;
    static const LanguageTag& rLang = aSysLocaleOptions.GetRealLanguageTag();

    if (rLang.getLanguage() == "hu")
    {
        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            OUString sId(OUString::number(GetFieldMgr().GetFormatId( nTypeId, i + SAL_N_ELEMENTS(FMT_REF_ARY))));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_LOWERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nTypeId, i ));
        }
        nExtraSize += nSize;

        if ( bAddCrossRefFormats )
        {
            sal_uInt16 nFormat = FMT_REF_NUMBER_IDX + SAL_N_ELEMENTS(FMT_REF_ARY);
            OUString sId(OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat)));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_LOWERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nTypeId, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nFormat = FMT_REF_NUMBER_NO_CONTEXT_IDX + SAL_N_ELEMENTS(FMT_REF_ARY);
            sId = OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_LOWERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nTypeId, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nFormat = FMT_REF_NUMBER_FULL_CONTEXT_IDX + SAL_N_ELEMENTS(FMT_REF_ARY);
            sId = OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_LOWERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nTypeId, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nExtraSize += 3;
        }
        // uppercase article
        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            OUString sId(OUString::number(GetFieldMgr().GetFormatId( nTypeId, i + 2 * SAL_N_ELEMENTS(FMT_REF_ARY))));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_UPPERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nTypeId, i ));
        }
        nExtraSize += nSize;
        if ( bAddCrossRefFormats )
        {
            sal_uInt16 nFormat = FMT_REF_NUMBER_IDX + 2 * SAL_N_ELEMENTS(FMT_REF_ARY);
            OUString sId(OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat)));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_UPPERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nTypeId, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nFormat = FMT_REF_NUMBER_NO_CONTEXT_IDX + 2 * SAL_N_ELEMENTS(FMT_REF_ARY);
            sId = OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_UPPERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nTypeId, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nFormat = FMT_REF_NUMBER_FULL_CONTEXT_IDX + 2 * SAL_N_ELEMENTS(FMT_REF_ARY);
            sId = OUString::number(GetFieldMgr().GetFormatId(nTypeId, nFormat));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_UPPERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nTypeId, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nExtraSize += 3;
        }
    }

    nSize += nExtraSize;

    // select a certain entry
    if (nSize)
    {
        if (!IsFieldEdit())
            m_xFormatLB->select_text(sOldSel);
        else
            m_xFormatLB->select_text(SwResId(FMT_REF_ARY[GetCurField()->GetFormat() % SAL_N_ELEMENTS(FMT_REF_ARY)]));

        if (m_xFormatLB->get_selected_index() == -1)
        {
            if (nFieldDlgFormatSel < m_xFormatLB->n_children())
                m_xFormatLB->select(nFieldDlgFormatSel);
            else
                m_xFormatLB->select(0);
        }
    }

    return nSize;
}

// Modify
IMPL_LINK_NOARG(SwFieldRefPage, ModifyHdl, weld::Entry&, void)
{
    OUString aName(m_xNameED->get_text());
    const bool bEmptyName = aName.isEmpty();

    bool bEnable = true;
    sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    if ((nTypeId == TYP_SETREFFLD && !GetFieldMgr().CanInsertRefMark(aName)) ||
        (bEmptyName && (nTypeId == TYP_GETREFFLD || nTypeId == TYP_SETREFFLD ||
                       nTypeId == REFFLDFLAG_BOOKMARK)))
        bEnable = false;

    EnableInsert(bEnable);

    m_xSelectionLB->select_text(aName);
}

bool SwFieldRefPage::FillItemSet(SfxItemSet* )
{
    bool bModified = false;
    sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    sal_uInt16 nSubType = 0;
    const sal_Int32 nEntryPos = m_xFormatLB->get_selected_index();
    const sal_uLong nFormat = (nEntryPos == -1)
        ? 0 : m_xFormatLB->get_id(nEntryPos).toUInt32();

    OUString aVal(m_xValueED->get_text());
    OUString aName(m_xNameED->get_text());

    switch(nTypeId)
    {
        case TYP_GETREFFLD:
            nSubType = REF_SETREFATTR;
            break;

        case TYP_SETREFFLD:
        {
            SwFieldType* pType = GetFieldMgr().GetFieldType(SwFieldIds::SetExp, aName);

            if(!pType)  // Only insert when the name doesn't exist yet
            {
                m_xSelectionLB->append_text(aName);
                m_xSelection->set_sensitive(true);
            }
            break;
        }
    }

    SwGetRefField* pRefField = static_cast<SwGetRefField*>(GetCurField());

    if (REFFLDFLAG & nTypeId)
    {
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
        {
            pSh = ::GetActiveWrtShell();
        }
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // text marks!
        {
            aName = m_xNameED->get_text();
            nTypeId = TYP_GETREFFLD;
            nSubType = REF_BOOKMARK;
        }
        else if (REFFLDFLAG_FOOTNOTE == nTypeId)        // footnotes
        {
            SwSeqFieldList aArr;
            SeqFieldLstElem aElem( m_xSelectionLB->get_selected_text(), 0 );

            size_t nPos = 0;

            nTypeId = TYP_GETREFFLD;
            nSubType = REF_FOOTNOTE;
            aName.clear();

            if (pSh->GetSeqFootnoteList(aArr) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = OUString::number( aArr[nPos].nSeqNo );

                if (IsFieldEdit() && aArr[nPos].nSeqNo == pRefField->GetSeqNo())
                    bModified = true; // can happen with fields of which the references were deleted
            }
            else if (IsFieldEdit())
                aVal = OUString::number( pRefField->GetSeqNo() );
        }
        else if (REFFLDFLAG_ENDNOTE == nTypeId)         // endnotes
        {
            SwSeqFieldList aArr;
            SeqFieldLstElem aElem( m_xSelectionLB->get_selected_text(), 0 );

            size_t nPos = 0;

            nTypeId = TYP_GETREFFLD;
            nSubType = REF_ENDNOTE;
            aName.clear();

            if (pSh->GetSeqFootnoteList(aArr, true) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = OUString::number( aArr[nPos].nSeqNo );

                if (IsFieldEdit() && aArr[nPos].nSeqNo == pRefField->GetSeqNo())
                    bModified = true; // can happen with fields of which the reference was deleted
            }
            else if (IsFieldEdit())
                aVal = OUString::number( pRefField->GetSeqNo() );
        }
        // #i83479#
        else if ( nTypeId == REFFLDFLAG_HEADING )
        {
            int nEntry = m_xSelectionToolTipLB->get_selected_index();
            OSL_ENSURE( nEntry != -1,
                    "<SwFieldRefPage::FillItemSet(..)> - no entry selected in selection tool tip listbox!" );
            if (nEntry != -1)
            {
                const size_t nOutlIdx(m_xSelectionToolTipLB->get_id(nEntry).toUInt32());
                pSh->getIDocumentOutlineNodesAccess()->getOutlineNodes( maOutlineNodes );
                if ( nOutlIdx < maOutlineNodes.size() )
                {
                    ::sw::mark::IMark const * const pMark = pSh->getIDocumentMarkAccess()->getMarkForTextNode(
                        *(maOutlineNodes[nOutlIdx]),
                        IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK);
                    aName = pMark->GetName();
                    nTypeId = TYP_GETREFFLD;
                    nSubType = REF_BOOKMARK;
                }
            }
        }
        else if ( nTypeId == REFFLDFLAG_NUMITEM )
        {
            int nEntry = m_xSelectionToolTipLB->get_selected_index();
            OSL_ENSURE( nEntry != -1,
                    "<SwFieldRefPage::FillItemSet(..)> - no entry selected in selection tool tip listbox!" );
            if (nEntry != -1)
            {
                const size_t nNumItemIdx(m_xSelectionToolTipLB->get_id(nEntry).toUInt32());
                pSh->getIDocumentListItemsAccess()->getNumItems(maNumItems);
                if ( nNumItemIdx < maNumItems.size() )
                {
                    ::sw::mark::IMark const * const pMark = pSh->getIDocumentMarkAccess()->getMarkForTextNode(
                        *(maNumItems[nNumItemIdx]->GetTextNode()),
                        IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK);
                    aName = pMark->GetName();
                    nTypeId = TYP_GETREFFLD;
                    nSubType = REF_BOOKMARK;
                }
            }
        }
        else                                // SequenceFields
        {
            // get fields for Seq-FieldType:
            SwSetExpFieldType* pType = static_cast<SwSetExpFieldType*>(pSh->GetFieldType(
                                    nTypeId & ~REFFLDFLAG, SwFieldIds::SetExp ));
            if( pType )
            {
                SwSeqFieldList aArr;
                SeqFieldLstElem aElem( m_xSelectionLB->get_selected_text(), 0 );

                size_t nPos = 0;

                nTypeId = TYP_GETREFFLD;
                nSubType = REF_SEQUENCEFLD;
                aName = pType->GetName();

                if (pType->GetSeqFieldList(aArr, pSh->GetLayout())
                    && aArr.SeekEntry(aElem, &nPos))
                {
                    aVal = OUString::number( aArr[nPos].nSeqNo );

                    if (IsFieldEdit() && aArr[nPos].nSeqNo == pRefField->GetSeqNo())
                        bModified = true; // can happen with fields of which the reference was deleted
                }
                else if (IsFieldEdit())
                    aVal = OUString::number( pRefField->GetSeqNo() );
            }
        }
    }

    if (IsFieldEdit() && nTypeId == TYP_GETREFFLD)
    {
        aVal = OUString::number(nSubType) + "|" + aVal;
    }

    if (!IsFieldEdit() || bModified ||
        m_xNameED->get_value_changed_from_saved() ||
        m_xValueED->get_value_changed_from_saved() ||
        m_xTypeLB->get_value_changed_from_saved() ||
        m_xSelectionLB->get_value_changed_from_saved() ||
        m_xFormatLB->get_value_changed_from_saved())
    {
        InsertField( nTypeId, nSubType, aName, aVal, nFormat );
    }

    ModifyHdl(*m_xNameED);    // enable/disable insert if applicable

    return false;
}

VclPtr<SfxTabPage> SwFieldRefPage::Create( TabPageParent pParent,
                                         const SfxItemSet *const pAttrSet)
{
    return VclPtr<SwFieldRefPage>::Create( pParent, pAttrSet );
}

sal_uInt16 SwFieldRefPage::GetGroup()
{
    return GRP_REF;
}

void    SwFieldRefPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_xTypeLB->get_selected_index();
    const sal_uInt16 nTypeSel = ( -1 == nEntryPos )
        ? USHRT_MAX
        : m_xTypeLB->get_id(nEntryPos).toUInt32();
    const sal_Int32 nFormatEntryPos = m_xFormatLB->get_selected_index();
    const sal_uInt32 nFormatSel = -1 == nFormatEntryPos ? USHRT_MAX : nFormatEntryPos;
    SetUserData( USER_DATA_VERSION ";" +
        OUString::number( nTypeSel ) + ";" +
        OUString::number( nFormatSel ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
