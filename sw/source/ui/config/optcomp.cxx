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

#include "optcomp.hrc"
#include "globals.hrc"
#include <unotools/configmgr.hxx>
#include <vcl/layout.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include "svtools/treelistentry.hxx"
#include <IDocumentSettingAccess.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::std;

#define DEFAULT_ENTRY       COMPATIBILITY_DEFAULT_NAME
#define USER_ENTRY          "_user"
#define BUTTON_BORDER       2

// struct CompatibilityItem ----------------------------------------------

struct CompatibilityItem
{
    String      m_sName;
    String      m_sModule;
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
    bool        m_bIsUser;

    CompatibilityItem( const String& _rName, const String& _rModule,
                       bool _bUsePrtMetrics, bool _bAddSpacing, bool _bAddSpacingAtPages,
                       bool _bUseOurTabStops, bool _bNoExtLeading, bool _bUseLineSpacing,
                       bool _bAddTableSpacing, bool _bUseObjPos, bool _bUseOurTextWrapping,
                       bool _bConsiderWrappingStyle, bool _bExpandWordSpace,
                       bool _bIsDefault, bool _bIsUser ) :

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
        m_bIsDefault            ( _bIsDefault ),
        m_bIsUser               ( _bIsUser ) {}
};

#include <vector>

struct SwCompatibilityOptPage_Impl
{
    typedef vector< CompatibilityItem > SwCompatibilityItemList;

    SwCompatibilityItemList     m_aList;
};

// class SwCompatibilityOptPage ------------------------------------------

SwCompatibilityOptPage::SwCompatibilityOptPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SW_RES( TP_OPTCOMPATIBILITY_PAGE ), rSet ),

    m_aMainFL           ( this, SW_RES( FL_MAIN ) ),
    m_aFormattingFT     ( this, SW_RES( FT_FORMATTING ) ),
    m_aFormattingLB     ( this, SW_RES( LB_FORMATTING ) ),
    m_aOptionsFT        ( this, SW_RES( FT_OPTIONS ) ),
    m_aOptionsLB        ( this, SW_RES( LB_OPTIONS ) ),
    m_aResetPB          ( this, SW_RES( PB_RESET ) ),
    m_aDefaultPB        ( this, SW_RES( PB_DEFAULT ) ),
    m_sUserEntry        (       SW_RES( STR_USERENTRY ) ),
    m_pWrtShell         ( NULL ),
    m_pImpl             ( new SwCompatibilityOptPage_Impl ),
    m_nSavedOptions     ( 0 )

{
    // init options strings with local resource ids -> so do it before FreeResource()
    for ( sal_uInt16 nResId = STR_COMP_OPTIONS_START; nResId < STR_COMP_OPTIONS_END; ++nResId )
    {
        String sEntry = String( SW_RES( nResId ) );
        if ( STR_TAB_ALIGNMENT == nResId ||
             STR_LINE_SPACING == nResId ||
             STR_USE_OBJPOSITIONING == nResId ||
             STR_USE_OURTEXTWRAPPING == nResId )
            ReplaceFormatName( sEntry );
        SvTreeListEntry* pEntry = m_aOptionsLB.SvTreeListBox::InsertEntry( sEntry );
        if ( pEntry )
        {
            m_aOptionsLB.SetCheckButtonState( pEntry, SV_BUTTON_UNCHECKED );
            pEntry->SetUserData( (void*)(sal_uLong)nResId );
        }
    }
    m_aOptionsLB.SetStyle( m_aOptionsLB.GetStyle() | WB_HSCROLL | WB_HIDESELECTION );
    m_aOptionsLB.SetHighlightRange();

    FreeResource();

    InitControls( rSet );

    // set handler
    m_aFormattingLB.SetSelectHdl( LINK( this, SwCompatibilityOptPage, SelectHdl ) );
    m_aDefaultPB.SetClickHdl( LINK( this, SwCompatibilityOptPage, UseAsDefaultHdl ) );

    // hide some controls, will be implemented later!!!
    m_aFormattingFT.Hide();
    m_aFormattingLB.Hide();
    m_aResetPB.Hide();
    // so move and resize the other controls
    Point aMovePnt = m_aFormattingFT.GetPosPixel();
    Point aNewPnt = m_aOptionsFT.GetPosPixel();
    aNewPnt.Y() = aMovePnt.Y();
    m_aOptionsFT.SetPosPixel( aNewPnt );
    aMovePnt = m_aFormattingLB.GetPosPixel();
    aNewPnt = m_aOptionsLB.GetPosPixel();
    long nDelta = aNewPnt.Y() - aMovePnt.Y();
    aNewPnt.Y() = aMovePnt.Y();
    m_aOptionsLB.SetPosPixel( aNewPnt );
    Size aNewSz = m_aOptionsLB.GetSizePixel();
    aNewSz.Height() += nDelta;
    m_aOptionsLB.SetSizePixel( aNewSz );
}

SwCompatibilityOptPage::~SwCompatibilityOptPage()
{
    delete m_pImpl;
}

void SwCompatibilityOptPage::ReplaceFormatName( String& rEntry )
{
    OUString sFormatName(utl::ConfigManager::getProductName());
    OUString sFormatVersion;
    bool bOpenOffice = ( sFormatName == "OpenOffice.org" );
    if ( bOpenOffice )
        sFormatVersion = OUString("1.1");
    else
        sFormatVersion = OUString("6.0/7");
    if ( !bOpenOffice && ( sFormatName != "StarSuite" ) )
        sFormatName = OUString("StarOffice");

    rEntry.SearchAndReplace( OUString("%FORMATNAME"), sFormatName );
    rEntry.SearchAndReplace( OUString("%FORMATVERSION"), sFormatVersion );
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
    String sDocTitle;
    const SfxPoolItem* pItem = NULL;
    SfxObjectShell* pObjShell = NULL;
    if ( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_WRTSHELL, sal_False, &pItem ) )
        m_pWrtShell = (SwWrtShell*)( (const SwPtrItem*)pItem )->GetValue();
    if ( m_pWrtShell )
    {
        pObjShell = m_pWrtShell->GetView().GetDocShell();
        if ( pObjShell )
            sDocTitle = pObjShell->GetTitle( SFX_TITLE_TITLE );
    }
    else
    {
        m_aMainFL.Disable();
        m_aFormattingFT.Disable();
        m_aFormattingLB.Disable();
        m_aOptionsFT.Disable();
        m_aOptionsLB.Disable();
        m_aResetPB.Disable();
        m_aDefaultPB.Disable();
    }
    String sText = m_aMainFL.GetText();
    sText.SearchAndReplace( OUString("%DOCNAME"), sDocTitle );
    m_aMainFL.SetText( sText );

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
    int i, j, nCount = aList.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        String sNewEntry;
        const Sequence< PropertyValue >& rEntry = aList[i];
        for ( j = 0; j < rEntry.getLength(); j++ )
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

        CompatibilityItem aItem(
            sName, sModule, bUsePrtMetrics, bAddSpacing,
            bAddSpacingAtPages, bUseOurTabStops, bNoExtLeading,
            bUseLineSpacing, bAddTableSpacing, bUseObjPos,
            bUseOurTextWrapping, bConsiderWrappingStyle, bExpandWordSpace,
            ( sName.equals( DEFAULT_ENTRY ) != sal_False ),
            ( sName.equals( USER_ENTRY ) != sal_False ) );
        m_pImpl->m_aList.push_back( aItem );

        if ( aItem.m_bIsDefault )
            continue;

        if ( sName.equals( USER_ENTRY ) )
            sNewEntry = m_sUserEntry;
        else if ( pObjShell && !sName.isEmpty() )
        {
            SfxFilterContainer* pFacCont = pObjShell->GetFactory().GetFilterContainer();
            const SfxFilter* pFilter = pFacCont->GetFilter4FilterName( sName );
            if ( pFilter )
                sNewEntry = pFilter->GetUIName();
        }

        if ( sNewEntry.Len() == 0 )
            sNewEntry = sName;

        sal_uInt16 nPos = m_aFormattingLB.InsertEntry( sNewEntry );
        sal_uLong nOptions = convertBools2Ulong_Impl(
            bUsePrtMetrics, bAddSpacing, bAddSpacingAtPages,
            bUseOurTabStops, bNoExtLeading, bUseLineSpacing,
            bAddTableSpacing, bUseObjPos, bUseOurTextWrapping,
            bConsiderWrappingStyle, bExpandWordSpace );
        m_aFormattingLB.SetEntryData( nPos, (void*)(sal_IntPtr)nOptions );
    }

    m_aFormattingLB.SetDropDownLineCount( m_aFormattingLB.GetEntryCount() );

    // check if the default button text is not too wide otherwise we have to stretch the button
    // and move its position and the position of the reset button
    long nTxtWidth = m_aDefaultPB.GetTextWidth( m_aDefaultPB.GetText() );
    Size aBtnSz = m_aDefaultPB.GetSizePixel();
    if ( nTxtWidth > aBtnSz.Width() )
    {
        long nDelta = nTxtWidth - aBtnSz.Width() + 2 * BUTTON_BORDER;
        aBtnSz.Width() += nDelta;
        Point aBtnPnt = m_aDefaultPB.GetPosPixel();
        aBtnPnt.X() -= nDelta;
        m_aDefaultPB.SetPosSizePixel( aBtnPnt, aBtnSz );
        aBtnPnt = m_aResetPB.GetPosPixel();
        aBtnPnt.X() -= 2 * nDelta;
        m_aResetPB.SetPosSizePixel( aBtnPnt, aBtnSz );
    }
}

IMPL_LINK_NOARG(SwCompatibilityOptPage, SelectHdl)
{
    sal_uInt16 nPos = m_aFormattingLB.GetSelectEntryPos();
    sal_uLong nOptions = (sal_uLong)(void*)m_aFormattingLB.GetEntryData( nPos );
    SetCurrentOptions( nOptions );

    return 0;
}

IMPL_LINK_NOARG(SwCompatibilityOptPage, UseAsDefaultHdl)
{
    MessageDialog aQuery(this, "QueryDefaultCompatDialog",
        "modules/swriter/ui/querydefaultcompatdialog.ui");
    if (aQuery.Execute() == RET_YES)
    {
        for ( vector< CompatibilityItem >::iterator pItem = m_pImpl->m_aList.begin();
              pItem != m_pImpl->m_aList.end(); ++pItem )
        {
            if ( pItem->m_bIsDefault )
            {
                sal_uInt16 nCount = static_cast< sal_uInt16 >( m_aOptionsLB.GetEntryCount() );
                for ( sal_uInt16 i = 0; i < nCount; ++i )
                {
                    bool bChecked = ( m_aOptionsLB.IsChecked(i) != sal_False );
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

    return 0;
}

void SwCompatibilityOptPage::SetCurrentOptions( sal_uLong nOptions )
{
    sal_uLong nCount = m_aOptionsLB.GetEntryCount();
    OSL_ENSURE( nCount <= 32, "SwCompatibilityOptPage::Reset(): entry overflow" );
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        sal_Bool bChecked = ( ( nOptions & 0x00000001 ) == 0x00000001 );
        m_aOptionsLB.CheckEntryPos( i, bChecked );
        nOptions = nOptions >> 1;
    }
}

sal_uLong SwCompatibilityOptPage::GetDocumentOptions() const
{
    sal_uLong nRet = 0;
    if ( m_pWrtShell )
    {
        const IDocumentSettingAccess& rIDocumentSettingAccess = *m_pWrtShell->getIDocumentSettingAccess();
        nRet = convertBools2Ulong_Impl(
                rIDocumentSettingAccess.get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) == sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::PARA_SPACE_MAX) != sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES) != sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::TAB_COMPAT) == sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::ADD_EXT_LEADING) == sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::OLD_LINE_SPACING) != sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) != sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::USE_FORMER_OBJECT_POS) != sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) != sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) != sal_False,
                rIDocumentSettingAccess.get(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK) != sal_True );
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

SfxTabPage* SwCompatibilityOptPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SwCompatibilityOptPage( pParent, rAttrSet );
}

sal_Bool SwCompatibilityOptPage::FillItemSet( SfxItemSet&  )
{
    sal_Bool bModified = sal_False;
    if ( m_pWrtShell )
    {
        sal_uLong nSavedOptions = m_nSavedOptions;
        sal_uLong nCount = m_aOptionsLB.GetEntryCount();
        OSL_ENSURE( nCount <= 32, "SwCompatibilityOptPage::Reset(): entry overflow" );

        bool bSetParaSpaceMax = false;

        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            CompatibilityOptions nOption = static_cast< CompatibilityOptions >(i);
            sal_Bool bChecked = m_aOptionsLB.IsChecked(i);
            sal_Bool bSavedChecked = ( ( nSavedOptions & 0x00000001 ) == 0x00000001 );
            if ( bChecked != bSavedChecked )
            {
                if ( COPT_USE_PRINTERDEVICE == nOption )
                {
                    m_pWrtShell->SetUseVirDev( !bChecked );
                    bModified = sal_True;
                }
                else if ( ( COPT_ADD_SPACING == nOption || COPT_ADD_SPACING_AT_PAGES == nOption ) && !bSetParaSpaceMax )
                    bSetParaSpaceMax = true;
                else if ( COPT_USE_OUR_TABSTOPS == nOption )
                {
                    m_pWrtShell->SetTabCompat( !bChecked );
                    bModified = sal_True;
                }
                else if ( COPT_NO_EXTLEADING == nOption )
                {
                    m_pWrtShell->SetAddExtLeading( !bChecked );
                    bModified = sal_True;
                }
                else if ( COPT_USE_LINESPACING == nOption )
                {
                       m_pWrtShell->SetUseFormerLineSpacing( bChecked );
                    bModified = sal_True;
                }
                else if ( COPT_ADD_TABLESPACING == nOption )
                {
                    m_pWrtShell->SetAddParaSpacingToTableCells( bChecked );
                    bModified = sal_True;
                }
                else if ( COPT_USE_OBJECTPOSITIONING == nOption )
                {
                    m_pWrtShell->SetUseFormerObjectPositioning( bChecked );
                    bModified = sal_True;
                }
                else if ( COPT_USE_OUR_TEXTWRAPPING == nOption )
                {
                    m_pWrtShell->SetUseFormerTextWrapping( bChecked );
                    bModified = sal_True;
                }
                else if ( COPT_CONSIDER_WRAPPINGSTYLE == nOption )
                {
                    m_pWrtShell->SetConsiderWrapOnObjPos( bChecked );
                    bModified = sal_True;
                }
                else if ( COPT_EXPAND_WORDSPACE == nOption )
                {
                    m_pWrtShell->SetDoNotJustifyLinesWithManualBreak( !bChecked );
                    bModified = sal_True;
                }
            }

            nSavedOptions = nSavedOptions >> 1;
        }

        if ( bSetParaSpaceMax )
        {
            m_pWrtShell->SetParaSpaceMax( m_aOptionsLB.IsChecked( (sal_uInt16)COPT_ADD_SPACING ) );
            m_pWrtShell->SetParaSpaceMaxAtPages( m_aOptionsLB.IsChecked( (sal_uInt16)COPT_ADD_SPACING_AT_PAGES ) );
            bModified = sal_True;
        }
    }

    if ( bModified )
        WriteOptions();

    return bModified;
}

void SwCompatibilityOptPage::Reset( const SfxItemSet&  )
{
    m_aOptionsLB.SelectEntryPos( 0 );

    sal_uLong nOptions = GetDocumentOptions();
    SetCurrentOptions( nOptions );
    m_nSavedOptions = nOptions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
