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

#include "optcomp.hxx"

#include "docsh.hxx"
#include "swmodule.hxx"
#include "swtypes.hxx"
#include "uiitems.hxx"
#include "view.hxx"
#include "wrtsh.hxx"

#include "globals.hrc"
#include <unotools/configmgr.hxx>
#include <vcl/layout.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/treelistentry.hxx>
#include <IDocumentSettingAccess.hxx>
#include <vector>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::std;

struct SwCompatibilityOptPage_Impl
{
    typedef vector< SvtCompatibilityEntry > SwCompatibilityItemList;

    SwCompatibilityItemList m_aList;
};

SwCompatibilityOptPage::SwCompatibilityOptPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptCompatPage", "modules/swriter/ui/optcompatpage.ui", &rSet)
    , m_pWrtShell(nullptr)
    , m_pImpl(new SwCompatibilityOptPage_Impl)
    , m_nSavedOptions(0)
{
    get(m_pMain, "compatframe");
    get(m_pFormattingLB, "format");
    get(m_pOptionsLB, "options");
    get(m_pDefaultPB, "default");

    for ( int i = (static_cast<int>(SvtCompatibilityEntry::Index::Module) + 1); i < static_cast<int>(SvtCompatibilityEntry::Index::INVALID); ++i )
    {
        int nCoptIdx = i - 2; /* Do not consider "Name" & "Module" indexes */

        const OUString sEntry = m_pFormattingLB->GetEntry( nCoptIdx );
        SvTreeListEntry* pEntry = m_pOptionsLB->SvTreeListBox::InsertEntry( sEntry );
        if ( pEntry )
        {
            m_pOptionsLB->SetCheckButtonState( pEntry, SvButtonState::Unchecked );
        }
    }

    m_sUserEntry = m_pFormattingLB->GetEntry(m_pFormattingLB->GetEntryCount()-1);

    m_pFormattingLB->Clear();

    m_pOptionsLB->SetStyle( m_pOptionsLB->GetStyle() | WB_HSCROLL | WB_HIDESELECTION );
    m_pOptionsLB->SetHighlightRange();

    InitControls( rSet );

    // set handler
    m_pFormattingLB->SetSelectHdl( LINK( this, SwCompatibilityOptPage, SelectHdl ) );
    m_pDefaultPB->SetClickHdl( LINK( this, SwCompatibilityOptPage, UseAsDefaultHdl ) );
}

SwCompatibilityOptPage::~SwCompatibilityOptPage()
{
    disposeOnce();
}

void SwCompatibilityOptPage::dispose()
{
    m_pImpl.reset();
    m_pMain.clear();
    m_pFormattingLB.clear();
    m_pOptionsLB.clear();
    m_pDefaultPB.clear();
    SfxTabPage::dispose();
}

sal_uLong convertBools2Ulong_Impl
(
    bool _bUsePrtMetrics,
    bool _bAddSpacing,
    bool _bAddSpacingAtPages,
    bool _bUseOurTabStops,
    bool _bNoExtLeading,
    bool _bUseLineSpacing,
    bool _bAddTableSpacing,
    bool _bUseObjPos,
    bool _bUseOurTextWrapping,
    bool _bConsiderWrappingStyle,
    bool _bExpandWordSpace,
    bool _bProtectForm,
    bool _bMsWordCompTrailingBlanks,
    bool bSubtractFlysAnchoredAtFlys
)
{
    sal_uLong nRet = 0;
    sal_uLong nSetBit = 1;

    if ( _bUsePrtMetrics )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bAddSpacing )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bAddSpacingAtPages )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bUseOurTabStops )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bNoExtLeading )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bUseLineSpacing )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bAddTableSpacing )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bUseObjPos )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bUseOurTextWrapping )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bConsiderWrappingStyle )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bExpandWordSpace )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bProtectForm )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if ( _bMsWordCompTrailingBlanks )
        nRet |= nSetBit;
    nSetBit = nSetBit << 1;
    if (bSubtractFlysAnchoredAtFlys)
        nRet |= nSetBit;

    return nRet;
}

void SwCompatibilityOptPage::InitControls( const SfxItemSet& rSet )
{
    // init objectshell and detect document name
    OUString sDocTitle;
    const SfxPoolItem* pItem = nullptr;
    SfxObjectShell* pObjShell = nullptr;
    if ( SfxItemState::SET == rSet.GetItemState( FN_PARAM_WRTSHELL, false, &pItem ) )
        m_pWrtShell = static_cast<SwWrtShell*>(static_cast<const SwPtrItem*>(pItem)->GetValue());
    if ( m_pWrtShell )
    {
        pObjShell = m_pWrtShell->GetView().GetDocShell();
        if ( pObjShell )
            sDocTitle = pObjShell->GetTitle();
    }
    else
    {
        m_pMain->Disable();
    }
    const OUString& rText = m_pMain->get_label();
    m_pMain->set_label(rText.replaceAll("%DOCNAME", sDocTitle));

    // loading file formats
    Sequence< Sequence< PropertyValue > > aList = m_aConfigItem.GetList();

    SvtCompatibilityEntry aEntry;
    aEntry.setValue<bool>( SvtCompatibilityEntry::Index::ExpandWordSpace, false );

    const sal_Int32 nCount = aList.getLength();
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        const Sequence< PropertyValue >& rEntry = aList[i];
        const sal_Int32 nEntries = rEntry.getLength();
        for ( sal_Int32 j = 0; j < nEntries; j++ )
        {
            PropertyValue aValue = rEntry[j];
            aEntry.setValue( SvtCompatibilityEntry::getIndex(aValue.Name), aValue.Value );
        }

        const OUString sEntryName = aEntry.getValue<OUString>( SvtCompatibilityEntry::Index::Name );

        const bool bIsUserEntry    = ( sEntryName == SvtCompatibilityEntry::getUserEntryName() );
        const bool bIsDefaultEntry = ( sEntryName == SvtCompatibilityEntry::getDefaultEntryName() );

        aEntry.setDefaultEntry( bIsDefaultEntry );

        m_pImpl->m_aList.push_back( aEntry );

        if ( aEntry.isDefaultEntry() )
            continue;

        OUString sNewEntry;
        if ( bIsUserEntry )
            sNewEntry = m_sUserEntry;

        else if ( pObjShell && !sEntryName.isEmpty() )
        {
            SfxFilterContainer* pFacCont = pObjShell->GetFactory().GetFilterContainer();
            std::shared_ptr<const SfxFilter> pFilter = pFacCont->GetFilter4FilterName( sEntryName );
            if ( pFilter )
                sNewEntry = pFilter->GetUIName();
        }

        if ( sNewEntry.isEmpty() )
            sNewEntry = sEntryName;

        const sal_Int32 nPos = m_pFormattingLB->InsertEntry( sNewEntry );
        sal_uLong nOptions = convertBools2Ulong_Impl(
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::UsePrtMetrics ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::AddSpacing ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::AddSpacingAtPages ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::UseOurTabStops ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::NoExtLeading ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::UseLineSpacing ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::AddTableSpacing ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::UseObjectPositioning ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::UseOurTextWrapping ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::ConsiderWrappingStyle ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::ExpandWordSpace ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::ProtectForm ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::MsWordTrailingBlanks ),
            aEntry.getValue<bool>( SvtCompatibilityEntry::Index::SubtractFlysAnchoredAtFlys ) );
        m_pFormattingLB->SetEntryData( nPos, reinterpret_cast<void*>((sal_IntPtr)nOptions) );
    }

    m_pFormattingLB->SetDropDownLineCount( m_pFormattingLB->GetEntryCount() );
}

IMPL_LINK_NOARG(SwCompatibilityOptPage, SelectHdl, ListBox&, void)
{
    const sal_Int32 nPos = m_pFormattingLB->GetSelectEntryPos();
    sal_uLong nOptions = reinterpret_cast<sal_uLong>(m_pFormattingLB->GetEntryData( nPos ));
    SetCurrentOptions( nOptions );
}

IMPL_LINK_NOARG(SwCompatibilityOptPage, UseAsDefaultHdl, Button*, void)
{
    ScopedVclPtrInstance<MessageDialog> aQuery(this, "QueryDefaultCompatDialog",
                                               "modules/swriter/ui/querydefaultcompatdialog.ui");
    if (aQuery->Execute() == RET_YES)
    {
        for ( vector< SvtCompatibilityEntry >::iterator pItem = m_pImpl->m_aList.begin();
              pItem != m_pImpl->m_aList.end(); ++pItem )
        {
            if ( pItem->isDefaultEntry() )
            {
                const sal_Int32 nCount = m_pOptionsLB->GetEntryCount();
                for ( sal_Int32 i = 0; i < nCount; ++i )
                {
                    bool bChecked = m_pOptionsLB->IsChecked(static_cast< sal_uLong >( i ));

                    int nCoptIdx = i + 2; /* Consider "Name" & "Module" indexes */
                    pItem->setValue<bool>( SvtCompatibilityEntry::Index(nCoptIdx), bChecked );
                }
                break;
            }
        }

        WriteOptions();
    }
}

void SwCompatibilityOptPage::SetCurrentOptions( sal_uLong nOptions )
{
    const sal_uLong nCount = m_pOptionsLB->GetEntryCount();
    OSL_ENSURE( nCount <= 32, "SwCompatibilityOptPage::Reset(): entry overflow" );
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        bool bChecked = ( ( nOptions & 0x00000001 ) == 0x00000001 );
        m_pOptionsLB->CheckEntryPos( i, bChecked );
        nOptions = nOptions >> 1;
    }
}

sal_uLong SwCompatibilityOptPage::GetDocumentOptions() const
{
    sal_uLong nRet = 0;
    if ( m_pWrtShell )
    {
        const IDocumentSettingAccess& rIDocumentSettingAccess = m_pWrtShell->getIDocumentSettingAccess();
        nRet = convertBools2Ulong_Impl(
            !rIDocumentSettingAccess.get( DocumentSettingId::USE_VIRTUAL_DEVICE ),
            rIDocumentSettingAccess.get( DocumentSettingId::PARA_SPACE_MAX ),
            rIDocumentSettingAccess.get( DocumentSettingId::PARA_SPACE_MAX_AT_PAGES ),
            !rIDocumentSettingAccess.get( DocumentSettingId::TAB_COMPAT ),
            !rIDocumentSettingAccess.get( DocumentSettingId::ADD_EXT_LEADING ),
            rIDocumentSettingAccess.get( DocumentSettingId::OLD_LINE_SPACING ),
            rIDocumentSettingAccess.get( DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS ),
            rIDocumentSettingAccess.get( DocumentSettingId::USE_FORMER_OBJECT_POS ),
            rIDocumentSettingAccess.get( DocumentSettingId::USE_FORMER_TEXT_WRAPPING ),
            rIDocumentSettingAccess.get( DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION ),
            !rIDocumentSettingAccess.get( DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK ),
            rIDocumentSettingAccess.get( DocumentSettingId::PROTECT_FORM ),
            rIDocumentSettingAccess.get( DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS ),
            rIDocumentSettingAccess.get( DocumentSettingId::SUBTRACT_FLYS ) );
    }
    return nRet;
}

void SwCompatibilityOptPage::WriteOptions()
{
    m_aConfigItem.Clear();
    for ( vector< SvtCompatibilityEntry >::const_iterator pItem = m_pImpl->m_aList.begin();
          pItem != m_pImpl->m_aList.end(); ++pItem )
        m_aConfigItem.AppendItem(*pItem);
}

VclPtr<SfxTabPage> SwCompatibilityOptPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SwCompatibilityOptPage>::Create( pParent, *rAttrSet );
}

bool SwCompatibilityOptPage::FillItemSet( SfxItemSet*  )
{
    bool bModified = false;
    if ( m_pWrtShell )
    {
        sal_uLong nSavedOptions = m_nSavedOptions;
        const sal_uLong nCount = m_pOptionsLB->GetEntryCount();
        OSL_ENSURE( nCount <= 32, "SwCompatibilityOptPage::Reset(): entry overflow" );

        for ( sal_uLong i = 0; i < nCount; ++i )
        {
            bool bChecked = m_pOptionsLB->IsChecked(i);
            bool bSavedChecked = ( ( nSavedOptions & 0x00000001 ) == 0x00000001 );
            if ( bChecked != bSavedChecked )
            {
                int nCoptIdx = i + 2; /* Consider "Name" & "Module" indexes */
                switch ( SvtCompatibilityEntry::Index(nCoptIdx) )
                {
                    case SvtCompatibilityEntry::Index::UsePrtMetrics:
                        m_pWrtShell->SetUseVirDev( !bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::AddSpacing:
                        m_pWrtShell->SetParaSpaceMax( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::AddSpacingAtPages:
                        m_pWrtShell->SetParaSpaceMaxAtPages( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::UseOurTabStops:
                        m_pWrtShell->SetTabCompat( !bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::NoExtLeading:
                        m_pWrtShell->SetAddExtLeading( !bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::UseLineSpacing:
                        m_pWrtShell->SetUseFormerLineSpacing( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::AddTableSpacing:
                        m_pWrtShell->SetAddParaSpacingToTableCells( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::UseObjectPositioning:
                        m_pWrtShell->SetUseFormerObjectPositioning( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::UseOurTextWrapping:
                        m_pWrtShell->SetUseFormerTextWrapping( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::ConsiderWrappingStyle:
                        m_pWrtShell->SetConsiderWrapOnObjPos( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::ExpandWordSpace:
                        m_pWrtShell->SetDoNotJustifyLinesWithManualBreak( !bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::ProtectForm:
                        m_pWrtShell->SetProtectForm( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::MsWordTrailingBlanks:
                        m_pWrtShell->SetMsWordCompTrailingBlanks( bChecked );
                        break;

                    case SvtCompatibilityEntry::Index::SubtractFlysAnchoredAtFlys:
                        m_pWrtShell->SetSubtractFlysAnchoredAtFlys(bChecked);
                        break;

                    default:
                        break;
                }
                bModified = true;
            }

            nSavedOptions = nSavedOptions >> 1;
        }
    }

    if ( bModified )
        WriteOptions();

    return bModified;
}

void SwCompatibilityOptPage::Reset( const SfxItemSet*  )
{
    m_pOptionsLB->SelectEntryPos( 0 );

    sal_uLong nOptions = GetDocumentOptions();
    SetCurrentOptions( nOptions );
    m_nSavedOptions = nOptions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
