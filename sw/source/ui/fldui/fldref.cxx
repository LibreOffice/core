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
#include <IMark.hxx>
#include <expfld.hxx>
#include <swmodule.hxx>
#include "fldref.hxx"
#include <frmatr.hxx>
#include <reffld.hxx>
#include <wrtsh.hxx>

#include <fldref.hrc>
#include <strings.hrc>
#include <SwNodeNum.hxx>
#include <IDocumentMarkAccess.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/charclass.hxx>
#include <osl/diagnose.h>

#include <comphelper/string.hxx>
#include <editeng/frmdiritem.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <vcl/settings.hxx>


static sal_uInt16 nFieldDlgFormatSel = 0;

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

namespace {

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

}

SwFieldRefPage::SwFieldRefPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *const pCoreSet )
    : SwFieldPage(pPage, pController, "modules/swriter/ui/fldrefpage.ui", "FieldRefPage", pCoreSet)
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
    , m_xStylerefFlags(m_xBuilder->weld_widget("stylerefflagsframe"))
    , m_xStylerefFromBottomCB(m_xBuilder->weld_check_button("stylereffrombottomcheckbox"))
    , m_xStylerefHideNonNumericalCB(m_xBuilder->weld_check_button("stylerefhidenonnumericalcheckbox"))
{
    m_xSelectionLB->make_sorted();
    // #i83479#
    for (auto const& aID : FLD_REF_PAGE_TYPES)
    {
        m_xTypeLB->append_text(SwResId(aID));
        m_xFormatLB->append_text(SwResId(aID));
    }

    m_sBookmarkText = m_xTypeLB->get_text(0);
    m_sFootnoteText = m_xTypeLB->get_text(1);
    m_sEndnoteText = m_xTypeLB->get_text(2);
    // #i83479#
    m_sHeadingText = m_xTypeLB->get_text(3);
    m_sNumItemText = m_xTypeLB->get_text(4);
    m_sStyleText = m_xTypeLB->get_text(5);

    auto nHeight = m_xTypeLB->get_height_rows(8);
    auto nWidth = m_xTypeLB->get_approximate_digit_width() * FIELD_COLUMN_WIDTH;
    m_xTypeLB->set_size_request(nWidth * 1.33, nHeight);
    m_xFormatLB->set_size_request(nWidth * 1.33, nHeight);
    m_xSelection->set_size_request(nWidth * 2, nHeight);
    nHeight = m_xTypeLB->get_height_rows(8);
    m_xSelectionToolTipLB->set_size_request(nHeight, nWidth*2);

    m_xTypeLB->clear();

    m_xNameED->connect_changed(LINK(this, SwFieldRefPage, ModifyHdl));
    m_xFilterED->connect_changed( LINK( this, SwFieldRefPage, ModifyHdl_Impl ) );

    m_xTypeLB->connect_row_activated(LINK(this, SwFieldRefPage, TreeViewInsertHdl));
    m_xTypeLB->connect_changed(LINK(this, SwFieldRefPage, TypeHdl));
    m_xSelectionLB->connect_changed(LINK(this, SwFieldRefPage, SubTypeListBoxHdl));
    m_xSelectionLB->connect_row_activated(LINK(this, SwFieldRefPage, TreeViewInsertHdl));
    m_xFormatLB->connect_row_activated(LINK(this, SwFieldRefPage, TreeViewInsertHdl));
    m_xFormatLB->connect_changed(LINK(this, SwFieldRefPage, FormatHdl));

    // #i83479#
    m_xSelectionToolTipLB->connect_changed( LINK(this, SwFieldRefPage, SubTypeTreeListBoxHdl) );
    m_xSelectionToolTipLB->connect_row_activated( LINK(this, SwFieldRefPage, TreeViewInsertHdl) );
    m_xFilterED->grab_focus();

    // uitests
    m_xTypeLB->set_buildable_name(m_xTypeLB->get_buildable_name() + "-ref");
    m_xNameED->set_buildable_name(m_xNameED->get_buildable_name() + "-ref");
    m_xValueED->set_buildable_name(m_xValueED->get_buildable_name() + "-ref");
    m_xSelectionLB->set_buildable_name(m_xSelectionLB->get_buildable_name() + "-ref");
    m_xFormatLB->set_buildable_name(m_xFormatLB->get_buildable_name() + "-ref");
}

SwFieldRefPage::~SwFieldRefPage()
{
}

IMPL_LINK_NOARG(SwFieldRefPage, ModifyHdl_Impl, weld::Entry&, void)
{
    UpdateSubType(comphelper::string::strip(m_xFilterED->get_text(), ' '));
    // tdf#135938 - refresh cross-reference name after filter selection has changed
    SubTypeHdl();
}

// #i83479#
void SwFieldRefPage::SaveSelectedTextNode()
{
    mpSavedSelectedTextNode = nullptr;
    mnSavedSelectedPos = 0;
    if ( !m_xSelectionToolTipLB->get_visible() )
        return;

    int nEntry = m_xSelectionToolTipLB->get_selected_index();
    if (nEntry == -1)
        return;

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
        const SwFieldTypesEnum nTypeId = SwFieldMgr::GetTypeId(i);

        if (!IsFieldEdit() || nTypeId != SwFieldTypesEnum::SetRef)
        {
            m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldMgr::GetTypeStr(i));
        }
    }

    // #i83479#
    // entries for headings and numbered items
    m_xTypeLB->append(OUString::number(REFFLDFLAG_HEADING), m_sHeadingText);
    m_xTypeLB->append(OUString::number(REFFLDFLAG_NUMITEM), m_sNumItemText);

    // fill up with the sequence types
    SwWrtShell *pSh = GetWrtShell();
    if (!pSh)
        pSh = ::GetActiveWrtShell();
    if (!pSh)
        return;

    // tdf#148432 in LTR UI override the navigator treeview direction based on
    // the first page directionality
    if (!AllSettings::GetLayoutRTL())
    {
        const SwPageDesc& rDesc = pSh->GetPageDesc(0);
        const SvxFrameDirectionItem& rFrameDir = rDesc.GetMaster().GetFrameDir();
        m_xSelectionToolTipLB->set_direction(rFrameDir.GetValue() == SvxFrameDirection::Horizontal_RL_TB);
    }

    const size_t nFieldTypeCnt = pSh->GetFieldTypeCount(SwFieldIds::SetExp);

    OSL_ENSURE( nFieldTypeCnt < o3tl::make_unsigned(REFFLDFLAG), "<SwFieldRefPage::Reset> - Item index will overlap flags!" );

    for (size_t n = 0; n < nFieldTypeCnt; ++n)
    {
        SwSetExpFieldType* pType = static_cast<SwSetExpFieldType*>(pSh->GetFieldType(n, SwFieldIds::SetExp));

        if ((nsSwGetSetExpType::GSE_SEQ & pType->GetType()) && pType->HasWriterListeners() && pSh->IsUsed(*pType))
        {
            m_xTypeLB->append(OUString::number(REFFLDFLAG | n), pType->GetName());
        }
    }

    // text marks - now always (because of globaldocuments)
    m_xTypeLB->append(OUString::number(REFFLDFLAG_BOOKMARK), m_sBookmarkText);

    // footnotes:
    if( pSh->HasFootnotes() )
    {
        m_xTypeLB->append(OUString::number(REFFLDFLAG_FOOTNOTE), m_sFootnoteText);
    }

    // endnotes:
    if ( pSh->HasFootnotes(true) )
    {
        m_xTypeLB->append(OUString::number(REFFLDFLAG_ENDNOTE), m_sEndnoteText);
    }

    // stylerefs
    m_xTypeLB->append(OUString::number(REFFLDFLAG_STYLE), m_sStyleText);

    m_xTypeLB->thaw();

    // select old Pos
    if (!IsFieldEdit())
        RestorePos(*m_xTypeLB);

    nFieldDlgFormatSel = 0;

    sal_uInt16 nFormatBoxPosition = USHRT_MAX;
    if( !IsFieldEdit() )
    {
        sal_Int32 nIdx{ 0 };
        const OUString sUserData = GetUserData();
        if(!IsRefresh() && o3tl::equalsIgnoreAsciiCase(o3tl::getToken(sUserData, 0, ';', nIdx),
                                u"" USER_DATA_VERSION_1))
        {
            const sal_uInt16 nVal = static_cast< sal_uInt16 >(o3tl::toInt32(o3tl::getToken(sUserData, 0, ';', nIdx)));
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
                    nFormatBoxPosition = static_cast< sal_uInt16 >(o3tl::toInt32(o3tl::getToken(sUserData, 0, ';', nIdx)));
                }
            }
        }
    }
    TypeHdl(*m_xTypeLB);
    if (!IsFieldEdit() && nFormatBoxPosition < m_xFormatLB->n_children())
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
        m_xStylerefFromBottomCB->save_state();
        m_xStylerefHideNonNumericalCB->save_state();
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
                        sName = m_sHeadingText;
                        nFlag = REFFLDFLAG_HEADING;
                    }
                    else if ( pRefField &&
                              pRefField->IsRefToNumItemCrossRefBookmark() )
                    {
                        sName = m_sNumItemText;
                        nFlag = REFFLDFLAG_NUMITEM;
                    }
                    else
                    {
                        sName = m_sBookmarkText;
                        nFlag = REFFLDFLAG_BOOKMARK;
                    }
                }
                break;

                case REF_FOOTNOTE:
                    sName = m_sFootnoteText;
                    nFlag = REFFLDFLAG_FOOTNOTE;
                    break;

                case REF_ENDNOTE:
                    sName = m_sEndnoteText;
                    nFlag = REFFLDFLAG_ENDNOTE;
                    break;

                case REF_SETREFATTR:
                    sName = SwResId(STR_GETREFFLD);
                    nFlag = REF_SETREFATTR;
                    break;

                case REF_SEQUENCEFLD:
                {
                    SwGetRefField const*const pRefField(dynamic_cast<SwGetRefField*>(GetCurField()));
                    if (pRefField)
                    {
                        sName = pRefField->GetSetRefName();
                    }
                    nFlag = REFFLDFLAG;
                    break;
                }

                case REF_STYLE:
                {
                    SwGetRefField const*const pRefField(dynamic_cast<SwGetRefField*>(GetCurField()));
                    if (pRefField)
                    {
                        sName = pRefField->GetPar1();
                    }
                    nFlag = REFFLDFLAG_STYLE;
                    break;
                }
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

    bool bName = false;
    nFieldDlgFormatSel = 0;

    if ( ( !IsFieldEdit() || m_xSelectionLB->n_children() ) &&
         nOld != -1 )
    {
        m_xNameED->set_text(OUString());
        m_xValueED->set_text(OUString());
        m_xFilterED->set_text(OUString());
    }

    // fill selection-ListBox
    UpdateSubType(comphelper::string::strip(m_xFilterED->get_text(), ' '));

    switch (nTypeId)
    {
        case static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef):
            if (nOld != -1 && REFFLDFLAG & m_xTypeLB->get_id(nOld).toUInt32())
                // the old one stays
                nFieldDlgFormatSel = m_xFormatLB->get_selected_index();
            bName = true;
            break;

        case static_cast<sal_uInt16>(SwFieldTypesEnum::SetRef):
            bName = true;
            break;

        case REFFLDFLAG_BOOKMARK:
            bName = true;
            [[fallthrough]];
        default:
            if( REFFLDFLAG & nTypeId )
            {
                const sal_uInt16 nOldId = nOld != -1 ? m_xTypeLB->get_id(nOld).toUInt32() : 0;
                if( nOldId & REFFLDFLAG || nOldId == static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef) )
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

IMPL_LINK_NOARG(SwFieldRefPage, FormatHdl, weld::TreeView&, void)
{
    SubTypeHdl();
}

void SwFieldRefPage::SubTypeHdl()
{
    sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    sal_uInt16 nFormat = m_xFormatLB->get_selected_id().toUInt32();
    m_xStylerefHideNonNumericalCB->set_visible(nFormat == FMT_REF_NUMBER_IDX
                                               || nFormat == FMT_REF_NUMBER_NO_CONTEXT_IDX
                                               || nFormat == FMT_REF_NUMBER_FULL_CONTEXT_IDX);
    m_xStylerefFlags->set_visible(nTypeId == REFFLDFLAG_STYLE);

    switch(nTypeId)
    {
        case static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef):
            if (!IsFieldEdit() || m_xSelectionLB->get_selected_index() != -1)
            {
                m_xNameED->set_text(m_xSelectionLB->get_selected_text());
                ModifyHdl(*m_xNameED);
            }
            break;

        case static_cast<sal_uInt16>(SwFieldTypesEnum::SetRef):
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
    if (!pSh)
        pSh = ::GetActiveWrtShell();
    if (!pSh)
        return;

    SwGetRefField const*const pRefField(dynamic_cast<SwGetRefField*>(GetCurField()));
    const sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    OUString sOldSel;
    // #i83479#
    if ( m_xSelectionLB->get_visible() )
    {
        const sal_Int32 nSelectionSel = m_xSelectionLB->get_selected_index();
        if (nSelectionSel != -1)
            sOldSel = m_xSelectionLB->get_text(nSelectionSel);
    }
    if (IsFieldEdit() && pRefField && sOldSel.isEmpty())
        sOldSel = OUString::number( pRefField->GetSeqNo() + 1 );

    m_xSelectionLB->freeze();
    m_xSelectionLB->clear();

    if (REFFLDFLAG & nTypeId)
    {
        if (nTypeId == REFFLDFLAG_FOOTNOTE || nTypeId == REFFLDFLAG_ENDNOTE)
        {
            m_xSelectionLB->thaw();
            m_xSelectionLB->make_unsorted();
            m_xSelectionLB->freeze();
        }
        // #i83479#
        else if (nTypeId != REFFLDFLAG_HEADING && nTypeId != REFFLDFLAG_NUMITEM)
        {
            m_xSelectionLB->thaw();
            m_xSelectionLB->make_sorted();
            m_xSelectionLB->freeze();
        }
    }

    // #i83479#
    m_xSelectionToolTipLB->freeze();
    m_xSelectionToolTipLB->clear();
    OUString m_sSelectionToolTipLBId;
    bool bShowSelectionToolTipLB( false );

    if( REFFLDFLAG & nTypeId )
    {
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // text marks!
        {
            // get all text marks
            IDocumentMarkAccess* const pMarkAccess = pSh->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getBookmarksBegin();
                ppMark != pMarkAccess->getBookmarksEnd();
                ++ppMark)
            {
                const ::sw::mark::IMark* pBkmk = *ppMark;
                if(IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(*pBkmk))
                {
                    bool isSubstring = MatchSubstring(pBkmk->GetName(), filterString);
                    if(isSubstring)
                    {
                        m_xSelectionLB->append_text( pBkmk->GetName() );
                    }
                }
            }
            if (IsFieldEdit() && pRefField)
                sOldSel = pRefField->GetSetRefName();
        }
        else if (nTypeId == REFFLDFLAG_FOOTNOTE)
        {
            SwSeqFieldList aArr;
            const size_t nCnt = pSh->GetSeqFootnoteList( aArr );

            for( size_t n = 0; n < nCnt; ++n )
            {
                bool isSubstring = MatchSubstring(aArr[ n ].sDlgEntry, filterString);
                if(isSubstring)
                {
                    m_xSelectionLB->append_text( aArr[ n ].sDlgEntry );
                }
                if (IsFieldEdit() && pRefField && pRefField->GetSeqNo() == aArr[ n ].nSeqNo)
                    sOldSel = aArr[n].sDlgEntry;
            }
        }
        else if (nTypeId == REFFLDFLAG_ENDNOTE)
        {
            SwSeqFieldList aArr;
            const size_t nCnt = pSh->GetSeqFootnoteList( aArr, true );

            for( size_t n = 0; n < nCnt; ++n )
            {
                bool isSubstring = MatchSubstring(aArr[ n ].sDlgEntry, filterString);
                if(isSubstring)
                {
                    m_xSelectionLB->append_text( aArr[ n ].sDlgEntry );
                }
                if (IsFieldEdit() && pRefField && pRefField->GetSeqNo() == aArr[ n ].nSeqNo)
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
                    if ((IsFieldEdit() && pRefField
                            && pRefField->GetReferencedTextNode(nullptr, nullptr) == maOutlineNodes[nOutlIdx])
                        || mpSavedSelectedTextNode == maOutlineNodes[nOutlIdx])
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
                    if ((IsFieldEdit() && pRefField
                            && pRefField->GetReferencedTextNode(nullptr, nullptr) == maNumItems[nNumItemIdx]->GetTextNode())
                        || mpSavedSelectedTextNode == maNumItems[nNumItemIdx]->GetTextNode())
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
        else if (nTypeId == REFFLDFLAG_STYLE)
        {
            const IDocumentOutlineNodes* pIDoc(pSh->getIDocumentOutlineNodesAccess());
            pIDoc->getOutlineNodes(maOutlineNodes);

            SfxStyleSheetBasePool* pStyleSheetPool
                = pSh->GetDoc()->GetDocShell()->GetStyleSheetPool();
            auto stylesheetIterator
                = pStyleSheetPool->CreateIterator(SfxStyleFamily::Para, SfxStyleSearchBits::Used);

            SfxStyleSheetBase* pStyle = stylesheetIterator->First();
            while (pStyle != nullptr)
            {
                bool isSubstring = MatchSubstring(pStyle->GetName(), filterString);

                if (isSubstring)
                {
                    m_xSelectionLB->append_text(pStyle->GetName());
                }

                pStyle = stylesheetIterator->Next();
            }

            if (IsFieldEdit() && pRefField) {
                sOldSel = pRefField->GetPar1();
                m_xStylerefFromBottomCB->set_active((pRefField->GetFlags() & REFFLDFLAG_STYLE_FROM_BOTTOM) == REFFLDFLAG_STYLE_FROM_BOTTOM);
                m_xStylerefHideNonNumericalCB->set_active((pRefField->GetFlags() & REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL) == REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL);
            }
        }
        else
        {
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
                    if (IsFieldEdit() && pRefField && sOldSel.isEmpty() &&
                        aArr[ n ].nSeqNo == pRefField->GetSeqNo())
                        sOldSel = aArr[ n ].sDlgEntry;
                }

                if (IsFieldEdit() && pRefField && sOldSel.isEmpty())
                    sOldSel = OUString::number( pRefField->GetSeqNo() + 1);
            }
        }
    }
    else
    {
        std::vector<OUString> aLst;
        GetFieldMgr().GetSubTypes(static_cast<SwFieldTypesEnum>(nTypeId), aLst);
        for(const OUString & i : aLst)
        {
            bool isSubstring = MatchSubstring( i , filterString );
            if(isSubstring)
            {
                m_xSelectionLB->append_text(i);
            }
        }

        if (IsFieldEdit() && pRefField)
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

const TranslateId FMT_REF_ARY[] =
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
    sal_uInt16 nOffset( 0 );
    bool bAddCrossRefFormats( false );
    switch (nTypeId)
    {
        // #i83479#
        case REFFLDFLAG_HEADING:
        case REFFLDFLAG_NUMITEM:
            bAddCrossRefFormats = true;
            [[fallthrough]];

        case static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef):
        case REFFLDFLAG_BOOKMARK:
        case REFFLDFLAG_FOOTNOTE:
        case REFFLDFLAG_ENDNOTE:
            nSize = FMT_REF_PAGE_PGDSC_IDX + 1;
            break;
        case REFFLDFLAG_STYLE:
            nOffset = FMT_REF_TEXT_IDX;
            nSize = FMT_REF_UPDOWN_IDX + 1 - nOffset;
            bAddCrossRefFormats = true;
            break;

        default:
            // #i83479#

            if ( REFFLDFLAG & nTypeId )
            {
                nSize = FMT_REF_ONLYSEQNO_IDX + 1;
            }
            else
            {
                nSize = GetFieldMgr().GetFormatCount( static_cast<SwFieldTypesEnum>(nTypeId), IsFieldDlgHtmlMode() );
            }
            break;
    }

    if (REFFLDFLAG & nTypeId)
        nTypeId = static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef);

    SwFieldTypesEnum nFieldType = static_cast<SwFieldTypesEnum>(nTypeId);
    for (sal_uInt16 i = nOffset; i < nSize + nOffset; i++)
    {
        OUString sId(OUString::number(GetFieldMgr().GetFormatId( nFieldType, i )));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr(nFieldType, i));
    }
    // #i83479#

    sal_uInt16 nExtraSize( 0 );
    if ( bAddCrossRefFormats )
    {
        sal_uInt16 nFormat = FMT_REF_NUMBER_IDX;
        OUString sId(OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat)));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr( nFieldType, nFormat ));
        nFormat = FMT_REF_NUMBER_NO_CONTEXT_IDX;
        sId = OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr( nFieldType, nFormat ));
        nFormat = FMT_REF_NUMBER_FULL_CONTEXT_IDX;
        sId = OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr( nFieldType, nFormat ));
        nExtraSize = 3;
    }

    // extra list items optionally, depending from reference-language
    SvtSysLocaleOptions aSysLocaleOptions;
    static const LanguageTag& rLang = aSysLocaleOptions.GetRealLanguageTag();

    if (rLang.getLanguage() == "hu")
    {
        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            OUString sId(OUString::number(GetFieldMgr().GetFormatId( nFieldType, i + SAL_N_ELEMENTS(FMT_REF_ARY))));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_LOWERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nFieldType, i ));
        }
        nExtraSize += nSize;

        if ( bAddCrossRefFormats )
        {
            sal_uInt16 nFormat = FMT_REF_NUMBER_IDX + SAL_N_ELEMENTS(FMT_REF_ARY);
            OUString sId(OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat)));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_LOWERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nFieldType, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nFormat = FMT_REF_NUMBER_NO_CONTEXT_IDX + SAL_N_ELEMENTS(FMT_REF_ARY);
            sId = OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_LOWERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nFieldType, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nFormat = FMT_REF_NUMBER_FULL_CONTEXT_IDX + SAL_N_ELEMENTS(FMT_REF_ARY);
            sId = OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_LOWERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nFieldType, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nExtraSize += 3;
        }
        // uppercase article
        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            OUString sId(OUString::number(GetFieldMgr().GetFormatId( nFieldType, i + 2 * SAL_N_ELEMENTS(FMT_REF_ARY))));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_UPPERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nFieldType, i ));
        }
        nExtraSize += nSize;
        if ( bAddCrossRefFormats )
        {
            sal_uInt16 nFormat = FMT_REF_NUMBER_IDX + 2 * SAL_N_ELEMENTS(FMT_REF_ARY);
            OUString sId(OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat)));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_UPPERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nFieldType, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nFormat = FMT_REF_NUMBER_NO_CONTEXT_IDX + 2 * SAL_N_ELEMENTS(FMT_REF_ARY);
            sId = OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_UPPERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nFieldType, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
            nFormat = FMT_REF_NUMBER_FULL_CONTEXT_IDX + 2 * SAL_N_ELEMENTS(FMT_REF_ARY);
            sId = OUString::number(GetFieldMgr().GetFormatId(nFieldType, nFormat));
            m_xFormatLB->append(sId, SwResId(FMT_REF_WITH_UPPERCASE_HU_ARTICLE) + GetFieldMgr().GetFormatStr( nFieldType, nFormat % SAL_N_ELEMENTS(FMT_REF_ARY)));
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

    if ((nTypeId == static_cast<sal_uInt16>(SwFieldTypesEnum::SetRef) && !GetFieldMgr().CanInsertRefMark(aName)) ||
        (bEmptyName && (nTypeId == static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef) || nTypeId == static_cast<sal_uInt16>(SwFieldTypesEnum::SetRef) ||
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
    const sal_uInt32 nFormat = (nEntryPos == -1)
        ? 0 : m_xFormatLB->get_id(nEntryPos).toUInt32();

    OUString aVal(m_xValueED->get_text());
    OUString aName(m_xNameED->get_text());

    switch(nTypeId)
    {
        case static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef):
            nSubType = REF_SETREFATTR;
            break;

        case static_cast<sal_uInt16>(SwFieldTypesEnum::SetRef):
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

    SwGetRefField const*const pRefField(dynamic_cast<SwGetRefField*>(GetCurField()));

    SwWrtShell *pSh = GetWrtShell();
    if(!pSh)
        pSh = ::GetActiveWrtShell();

    if (pSh && REFFLDFLAG & nTypeId)
    {
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // text marks!
        {
            aName = m_xNameED->get_text();
            nTypeId = static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef);
            nSubType = REF_BOOKMARK;
        }
        else if (REFFLDFLAG_FOOTNOTE == nTypeId)        // footnotes
        {
            SwSeqFieldList aArr;
            SeqFieldLstElem aElem( m_xSelectionLB->get_selected_text(), 0 );

            size_t nPos = 0;

            nTypeId = static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef);
            nSubType = REF_FOOTNOTE;
            aName.clear();

            if (pSh->GetSeqFootnoteList(aArr) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = OUString::number( aArr[nPos].nSeqNo );

                if (IsFieldEdit() && pRefField && aArr[nPos].nSeqNo == pRefField->GetSeqNo())
                    bModified = true; // can happen with fields of which the references were deleted
            }
            else if (IsFieldEdit() && pRefField)
                aVal = OUString::number( pRefField->GetSeqNo() );
        }
        else if (REFFLDFLAG_ENDNOTE == nTypeId)         // endnotes
        {
            SwSeqFieldList aArr;
            SeqFieldLstElem aElem( m_xSelectionLB->get_selected_text(), 0 );

            size_t nPos = 0;

            nTypeId = static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef);
            nSubType = REF_ENDNOTE;
            aName.clear();

            if (pSh->GetSeqFootnoteList(aArr, true) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = OUString::number( aArr[nPos].nSeqNo );

                if (IsFieldEdit() && pRefField && aArr[nPos].nSeqNo == pRefField->GetSeqNo())
                    bModified = true; // can happen with fields of which the reference was deleted
            }
            else if (IsFieldEdit() && pRefField)
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
                    nTypeId = static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef);
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
                    nTypeId = static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef);
                    nSubType = REF_BOOKMARK;
                }
            }
        }
        else if (nTypeId == REFFLDFLAG_STYLE)
        {
            int nEntry = m_xSelectionLB->get_selected_index();
            if (nEntry != -1)
            {
                aName = m_xSelectionLB->get_text(nEntry);
                nTypeId = static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef);
                nSubType = REF_STYLE;
                sal_uInt16 nVal = REFFLDFLAG_STYLE;

                if (m_xStylerefFromBottomCB->get_active()) {
                    nVal |= REFFLDFLAG_STYLE_FROM_BOTTOM;
                }

                if (m_xStylerefHideNonNumericalCB->get_active()) {
                    nVal |= REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL;
                }

                aVal = OUString::number(nVal);
            } else {
                SAL_WARN("sw.ui", "<SwFieldRefPage::FillItemSet(..)> no entry selected in selection listbox!");
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

                nTypeId = static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef);
                nSubType = REF_SEQUENCEFLD;
                aName = pType->GetName();

                if (pType->GetSeqFieldList(aArr, pSh->GetLayout())
                    && aArr.SeekEntry(aElem, &nPos))
                {
                    aVal = OUString::number( aArr[nPos].nSeqNo );

                    if (IsFieldEdit() && pRefField && aArr[nPos].nSeqNo == pRefField->GetSeqNo())
                        bModified = true; // can happen with fields of which the reference was deleted
                }
                else if (IsFieldEdit() && pRefField)
                    aVal = OUString::number( pRefField->GetSeqNo() );
            }
        }
    }

    if (IsFieldEdit() && nTypeId == static_cast<sal_uInt16>(SwFieldTypesEnum::GetRef))
    {
        aVal = OUString::number(nSubType) + "|" + aVal;
    }

    if (!IsFieldEdit() || bModified ||
        m_xNameED->get_value_changed_from_saved() ||
        m_xValueED->get_value_changed_from_saved() ||
        m_xTypeLB->get_value_changed_from_saved() ||
        m_xSelectionLB->get_value_changed_from_saved() ||
        m_xFormatLB->get_value_changed_from_saved() ||
        (nSubType == REF_STYLE
         && (m_xStylerefFromBottomCB->get_state_changed_from_saved() || m_xStylerefHideNonNumericalCB->get_state_changed_from_saved())))
    {
        InsertField( static_cast<SwFieldTypesEnum>(nTypeId), nSubType, aName, aVal, nFormat );
    }

    ModifyHdl(*m_xNameED);    // enable/disable insert if applicable

    return false;
}

std::unique_ptr<SfxTabPage> SwFieldRefPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                         const SfxItemSet *const pAttrSet)
{
    return std::make_unique<SwFieldRefPage>(pPage, pController, pAttrSet);
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
