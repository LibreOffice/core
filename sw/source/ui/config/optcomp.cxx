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

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::std;

struct CompatibilityItem
{
    OUString    m_sName;
    OUString    m_sModule;
    bool        m_bUsePrtMetrics;
    bool        m_bAddSpacing;
    bool        m_bAddSpacingAtPages;
    bool        m_bUseOurTabStops;
    bool        m_bNoExtLeading;
    bool        m_bUseLineSpacing;
    bool        m_bAddTableSpacing;
    bool        m_bUseObjPos;
    bool        m_bUseOurTextWrapping;
    bool        m_bConsiderWrappingStyle;
    bool        m_bExpandWordSpace;
    bool        m_bIsDefault;

    CompatibilityItem( const OUString& _rName, const OUString& _rModule,
                       bool _bUsePrtMetrics, bool _bAddSpacing, bool _bAddSpacingAtPages,
                       bool _bUseOurTabStops, bool _bNoExtLeading, bool _bUseLineSpacing,
                       bool _bAddTableSpacing, bool _bUseObjPos, bool _bUseOurTextWrapping,
                       bool _bConsiderWrappingStyle, bool _bExpandWordSpace,
                       bool _bIsDefault ) :

        m_sName                 ( _rName ),
        m_sModule               ( _rModule ),
        m_bUsePrtMetrics        ( _bUsePrtMetrics ),
        m_bAddSpacing           ( _bAddSpacing ),
        m_bAddSpacingAtPages    ( _bAddSpacingAtPages ),
        m_bUseOurTabStops       ( _bUseOurTabStops ),
        m_bNoExtLeading         ( _bNoExtLeading ),
        m_bUseLineSpacing       ( _bUseLineSpacing ),
        m_bAddTableSpacing      ( _bAddTableSpacing ),
        m_bUseObjPos            ( _bUseObjPos ),
        m_bUseOurTextWrapping   ( _bUseOurTextWrapping ),
        m_bConsiderWrappingStyle( _bConsiderWrappingStyle ),
        m_bExpandWordSpace      ( _bExpandWordSpace ),
        m_bIsDefault            ( _bIsDefault ) {}
};

#include <vector>

struct SwCompatibilityOptPage_Impl
{
    typedef vector< CompatibilityItem > SwCompatibilityItemList;

    SwCompatibilityItemList     m_aList;
};

SwCompatibilityOptPage::SwCompatibilityOptPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptCompatPage",
        "modules/swriter/ui/optcompatpage.ui", &rSet)
    , m_pWrtShell(nullptr)
    , m_pImpl(new SwCompatibilityOptPage_Impl)
    , m_nSavedOptions(0)
{
    get(m_pMain, "compatframe");
    get(m_pFormattingLB, "format");
    get(m_pOptionsLB, "options");
    get(m_pDefaultPB, "default");

    for (sal_Int32 nId = COPT_USE_PRINTERDEVICE; nId <= COPT_EXPAND_WORDSPACE; ++nId)
    {
        const OUString sEntry = m_pFormattingLB->GetEntry(nId);
        SvTreeListEntry* pEntry = m_pOptionsLB->SvTreeListBox::InsertEntry( sEntry );
        if ( pEntry )
        {
            m_pOptionsLB->SetCheckButtonState( pEntry, SV_BUTTON_UNCHECKED );
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
    delete m_pImpl;
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
    bool _bExpandWordSpace
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
    OUString sName;
    OUString sModule;
    bool bUsePrtMetrics = false;
    bool bAddSpacing = false;
    bool bAddSpacingAtPages = false;
    bool bUseOurTabStops = false;
    bool bNoExtLeading = false;
    bool bUseLineSpacing = false;
    bool bAddTableSpacing = false;
    bool bUseObjPos = false;
    bool bUseOurTextWrapping = false;
    bool bConsiderWrappingStyle = false;
    bool bExpandWordSpace = false;
    const sal_Int32 nCount = aList.getLength();
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        const Sequence< PropertyValue >& rEntry = aList[i];
        const sal_Int32 nEntries = rEntry.getLength();
        for ( sal_Int32 j = 0; j < nEntries; j++ )
        {
            PropertyValue aValue = rEntry[j];
            if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_NAME )
                aValue.Value >>= sName;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_MODULE )
                aValue.Value >>= sModule;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_USEPRTMETRICS )
                aValue.Value >>= bUsePrtMetrics;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_ADDSPACING )
                aValue.Value >>= bAddSpacing;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_ADDSPACINGATPAGES )
                aValue.Value >>= bAddSpacingAtPages;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_USEOURTABSTOPS )
                aValue.Value >>= bUseOurTabStops;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_NOEXTLEADING )
                aValue.Value >>= bNoExtLeading;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_USELINESPACING )
                aValue.Value >>= bUseLineSpacing;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_ADDTABLESPACING )
                aValue.Value >>= bAddTableSpacing;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_USEOBJECTPOSITIONING )
                aValue.Value >>= bUseObjPos;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_USEOURTEXTWRAPPING )
                aValue.Value >>= bUseOurTextWrapping;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_CONSIDERWRAPPINGSTYLE )
                aValue.Value >>= bConsiderWrappingStyle;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_EXPANDWORDSPACE )
                aValue.Value >>= bExpandWordSpace;
        }

        const bool bIsUserEntry = sName == "_user";
        const bool bIsDefaultEntry = sName == COMPATIBILITY_DEFAULT_NAME;

        CompatibilityItem aItem(
            sName, sModule, bUsePrtMetrics, bAddSpacing,
            bAddSpacingAtPages, bUseOurTabStops, bNoExtLeading,
            bUseLineSpacing, bAddTableSpacing, bUseObjPos,
            bUseOurTextWrapping, bConsiderWrappingStyle, bExpandWordSpace,
            bIsDefaultEntry );
        m_pImpl->m_aList.push_back( aItem );

        if ( aItem.m_bIsDefault )
            continue;

        OUString sNewEntry;
        if ( bIsUserEntry )
            sNewEntry = m_sUserEntry;
        else if ( pObjShell && !sName.isEmpty() )
        {
            SfxFilterContainer* pFacCont = pObjShell->GetFactory().GetFilterContainer();
            const SfxFilter* pFilter = pFacCont->GetFilter4FilterName( sName );
            if ( pFilter )
                sNewEntry = pFilter->GetUIName();
        }

        if ( sNewEntry.isEmpty() )
            sNewEntry = sName;

        const sal_Int32 nPos = m_pFormattingLB->InsertEntry( sNewEntry );
        sal_uLong nOptions = convertBools2Ulong_Impl(
            bUsePrtMetrics, bAddSpacing, bAddSpacingAtPages,
            bUseOurTabStops, bNoExtLeading, bUseLineSpacing,
            bAddTableSpacing, bUseObjPos, bUseOurTextWrapping,
            bConsiderWrappingStyle, bExpandWordSpace );
        m_pFormattingLB->SetEntryData( nPos, reinterpret_cast<void*>((sal_IntPtr)nOptions) );
    }

    m_pFormattingLB->SetDropDownLineCount( m_pFormattingLB->GetEntryCount() );
}

IMPL_LINK_NOARG_TYPED(SwCompatibilityOptPage, SelectHdl, ListBox&, void)
{
    const sal_Int32 nPos = m_pFormattingLB->GetSelectEntryPos();
    sal_uLong nOptions = reinterpret_cast<sal_uLong>(m_pFormattingLB->GetEntryData( nPos ));
    SetCurrentOptions( nOptions );
}

IMPL_LINK_NOARG_TYPED(SwCompatibilityOptPage, UseAsDefaultHdl, Button*, void)
{
    ScopedVclPtrInstance<MessageDialog> aQuery(this, "QueryDefaultCompatDialog",
                                               "modules/swriter/ui/querydefaultcompatdialog.ui");
    if (aQuery->Execute() == RET_YES)
    {
        for ( vector< CompatibilityItem >::iterator pItem = m_pImpl->m_aList.begin();
              pItem != m_pImpl->m_aList.end(); ++pItem )
        {
            if ( pItem->m_bIsDefault )
            {
                const sal_Int32 nCount = m_pOptionsLB->GetEntryCount();
                for ( sal_Int32 i = 0; i < nCount; ++i )
                {
                    bool bChecked = m_pOptionsLB->IsChecked(static_cast< sal_uLong >( i ));
                    CompatibilityOptions eOption = static_cast< CompatibilityOptions >(i);
                    switch ( eOption )
                    {
                        case COPT_USE_PRINTERDEVICE : pItem->m_bUsePrtMetrics = bChecked; break;
                        case COPT_ADD_SPACING : pItem->m_bAddSpacing = bChecked; break;
                        case COPT_ADD_SPACING_AT_PAGES : pItem->m_bAddSpacingAtPages = bChecked; break;
                        case COPT_USE_OUR_TABSTOPS : pItem->m_bUseOurTabStops = bChecked; break;
                        case COPT_NO_EXTLEADING : pItem->m_bNoExtLeading = bChecked; break;
                        case COPT_USE_LINESPACING : pItem->m_bUseLineSpacing = bChecked; break;
                        case COPT_ADD_TABLESPACING : pItem->m_bAddTableSpacing = bChecked; break;
                        case COPT_USE_OBJECTPOSITIONING: pItem->m_bUseObjPos = bChecked; break;
                        case COPT_USE_OUR_TEXTWRAPPING: pItem->m_bUseOurTextWrapping = bChecked; break;
                        case COPT_CONSIDER_WRAPPINGSTYLE: pItem->m_bConsiderWrappingStyle = bChecked; break;
                        case COPT_EXPAND_WORDSPACE:  pItem->m_bExpandWordSpace = bChecked; break;
                        default:
                        {
                            OSL_FAIL("SwCompatibilityOptPage::UseAsDefaultHdl(): wrong option" );
                        }
                    }
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
                !rIDocumentSettingAccess.get(DocumentSettingId::USE_VIRTUAL_DEVICE),
                rIDocumentSettingAccess.get(DocumentSettingId::PARA_SPACE_MAX),
                rIDocumentSettingAccess.get(DocumentSettingId::PARA_SPACE_MAX_AT_PAGES),
                !rIDocumentSettingAccess.get(DocumentSettingId::TAB_COMPAT),
                !rIDocumentSettingAccess.get(DocumentSettingId::ADD_EXT_LEADING),
                rIDocumentSettingAccess.get(DocumentSettingId::OLD_LINE_SPACING),
                rIDocumentSettingAccess.get(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS),
                rIDocumentSettingAccess.get(DocumentSettingId::USE_FORMER_OBJECT_POS),
                rIDocumentSettingAccess.get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING),
                rIDocumentSettingAccess.get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION),
                !rIDocumentSettingAccess.get(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK) );
    }
    return nRet;
}

void SwCompatibilityOptPage::WriteOptions()
{
    m_aConfigItem.Clear();
    for ( vector< CompatibilityItem >::const_iterator pItem = m_pImpl->m_aList.begin();
          pItem != m_pImpl->m_aList.end(); ++pItem )
        m_aConfigItem.AppendItem(
            pItem->m_sName, pItem->m_sModule, pItem->m_bUsePrtMetrics, pItem->m_bAddSpacing,
            pItem->m_bAddSpacingAtPages, pItem->m_bUseOurTabStops,
            pItem->m_bNoExtLeading, pItem->m_bUseLineSpacing,
            pItem->m_bAddTableSpacing, pItem->m_bUseObjPos,
            pItem->m_bUseOurTextWrapping, pItem->m_bConsiderWrappingStyle,
            pItem->m_bExpandWordSpace );
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

        bool bSetParaSpaceMax = false;

        for ( sal_uLong i = 0; i < nCount; ++i )
        {
            CompatibilityOptions nOption = static_cast< CompatibilityOptions >(i);
            bool bChecked = m_pOptionsLB->IsChecked(i);
            bool bSavedChecked = ( ( nSavedOptions & 0x00000001 ) == 0x00000001 );
            if ( bChecked != bSavedChecked )
            {
                if ( COPT_USE_PRINTERDEVICE == nOption )
                {
                    m_pWrtShell->SetUseVirDev( !bChecked );
                    bModified = true;
                }
                else if ( ( COPT_ADD_SPACING == nOption || COPT_ADD_SPACING_AT_PAGES == nOption ) && !bSetParaSpaceMax )
                    bSetParaSpaceMax = true;
                else if ( COPT_USE_OUR_TABSTOPS == nOption )
                {
                    m_pWrtShell->SetTabCompat( !bChecked );
                    bModified = true;
                }
                else if ( COPT_NO_EXTLEADING == nOption )
                {
                    m_pWrtShell->SetAddExtLeading( !bChecked );
                    bModified = true;
                }
                else if ( COPT_USE_LINESPACING == nOption )
                {
                       m_pWrtShell->SetUseFormerLineSpacing( bChecked );
                    bModified = true;
                }
                else if ( COPT_ADD_TABLESPACING == nOption )
                {
                    m_pWrtShell->SetAddParaSpacingToTableCells( bChecked );
                    bModified = true;
                }
                else if ( COPT_USE_OBJECTPOSITIONING == nOption )
                {
                    m_pWrtShell->SetUseFormerObjectPositioning( bChecked );
                    bModified = true;
                }
                else if ( COPT_USE_OUR_TEXTWRAPPING == nOption )
                {
                    m_pWrtShell->SetUseFormerTextWrapping( bChecked );
                    bModified = true;
                }
                else if ( COPT_CONSIDER_WRAPPINGSTYLE == nOption )
                {
                    m_pWrtShell->SetConsiderWrapOnObjPos( bChecked );
                    bModified = true;
                }
                else if ( COPT_EXPAND_WORDSPACE == nOption )
                {
                    m_pWrtShell->SetDoNotJustifyLinesWithManualBreak( !bChecked );
                    bModified = true;
                }
            }

            nSavedOptions = nSavedOptions >> 1;
        }

        if ( bSetParaSpaceMax )
        {
            m_pWrtShell->SetParaSpaceMax( m_pOptionsLB->IsChecked( (sal_uLong)COPT_ADD_SPACING ) );
            m_pWrtShell->SetParaSpaceMaxAtPages( m_pOptionsLB->IsChecked( (sal_uLong)COPT_ADD_SPACING_AT_PAGES ) );
            bModified = true;
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
