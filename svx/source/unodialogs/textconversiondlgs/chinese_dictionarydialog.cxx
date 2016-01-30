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


#include "chinese_dictionarydialog.hxx"
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionPropertyType.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryList.hpp>
#include <com/sun/star/linguistic2/XConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <svtools/headbar.hxx>
#include <svtools/svlbitm.hxx>
#include "svtools/treelistentry.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include "helpid.hrc"


namespace textconversiondlgs
{


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define HEADER_BAR_BITS ( HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER | HeaderBarItemBits::CLICKABLE | HeaderBarItemBits::FIXED | HeaderBarItemBits::FIXEDPOS )

DictionaryList::DictionaryList(SvSimpleTableContainer& rParent, WinBits nBits)
    : SvSimpleTable(rParent, nBits)
    , m_xDictionary(nullptr)
    , m_pED_Term(nullptr)
    , m_pED_Mapping(nullptr)
    , m_pLB_Property(nullptr)
    , m_aToBeDeleted()
    , m_nSortColumnIndex(0)
{
}

DictionaryList::~DictionaryList()
{
    disposeOnce();
}

void DictionaryList::dispose()
{
    m_pED_Term.clear();
    m_pED_Mapping.clear();
    m_pLB_Property.clear();
    SvSimpleTable::dispose();
}

OUString DictionaryList::getPropertyTypeName( sal_Int16 nConversionPropertyType ) const
{
    if(!m_pLB_Property || !m_pLB_Property->GetEntryCount())
        return OUString();

    sal_uInt16 nPos = static_cast<sal_uInt16>( nConversionPropertyType )-1;
    if(nPos<m_pLB_Property->GetEntryCount())
        return m_pLB_Property->GetEntry(nPos);
    return m_pLB_Property->GetEntry(0);
}

OUString DictionaryList::makeTabString( const DictionaryEntry& rEntry ) const
{
    OUString aStr( rEntry.m_aTerm );
    aStr += "\t";
    aStr += rEntry.m_aMapping;
    aStr += "\t";
    aStr += getPropertyTypeName( rEntry.m_nConversionPropertyType );
    return aStr;
}

void DictionaryList::save()
{
    if( !m_xDictionary.is() )
        return;

    Reference< linguistic2::XConversionPropertyType > xPropertyType( m_xDictionary, uno::UNO_QUERY );

    sal_Int32 nN;
    DictionaryEntry* pE;

    for( nN = m_aToBeDeleted.size(); nN--; )
    {
        pE = m_aToBeDeleted[nN];
        m_xDictionary->removeEntry( pE->m_aTerm, pE->m_aMapping );
    }
    for( nN = GetRowCount(); nN--; )
    {
        pE = getEntryOnPos( nN );
        if(pE->m_bNewEntry)
        {
            try
            {
                m_xDictionary->addEntry( pE->m_aTerm, pE->m_aMapping );
                xPropertyType->setPropertyType( pE->m_aTerm, pE->m_aMapping, pE->m_nConversionPropertyType );
            }
            catch( uno::Exception& )
            {

            }
        }
    }
    Reference< util::XFlushable > xFlush( m_xDictionary, uno::UNO_QUERY );
    if( xFlush.is() )
        xFlush->flush();
}

void DictionaryList::deleteAll()
{
    sal_Int32 nN;
    for( nN = GetRowCount(); nN--; )
        deleteEntryOnPos( nN  );
    for( nN = m_aToBeDeleted.size(); nN--; )
    {
        DictionaryEntry* pE = m_aToBeDeleted[nN];
        delete pE;
    }
    m_aToBeDeleted.clear();
}

void DictionaryList::refillFromDictionary( sal_Int32 nTextConversionOptions )
{
    deleteAll();

    if(!m_xDictionary.is())
        return;

    Sequence< OUString > aLeftList(  m_xDictionary->getConversionEntries( linguistic2::ConversionDirection_FROM_LEFT ) );
    sal_Int32 nCount = aLeftList.getLength();

    Reference< linguistic2::XConversionPropertyType > xPropertyType( m_xDictionary, uno::UNO_QUERY );

    OUString aLeft, aRight;
    sal_Int16 nConversionPropertyType;

    for(sal_Int32 nN=0; nN<nCount; nN++)
    {
        aLeft  = aLeftList[nN];
        Sequence< OUString > aRightList( m_xDictionary->getConversions(
            aLeft, 0, aLeft.getLength()
            , linguistic2::ConversionDirection_FROM_LEFT, nTextConversionOptions ) );

        if(aRightList.getLength()!=1)
        {
            OSL_FAIL("The Chinese Translation Dictionary should have exactly one Mapping for each term.");
            continue;
        }

        aRight = aRightList[0];
        nConversionPropertyType = linguistic2::ConversionPropertyType::OTHER;
        if(xPropertyType.is())
            nConversionPropertyType = xPropertyType->getPropertyType(aLeft, aRight);

        DictionaryEntry* pEntry = new DictionaryEntry( aLeft, aRight, nConversionPropertyType );
        SvTreeListEntry* pLBEntry = InsertEntry( makeTabString( *pEntry ) );
        pLBEntry->SetUserData( pEntry );
    }

    if( GetEntryCount() > 0 )
        SelectRow( 0 );
}

DictionaryEntry* DictionaryList::getFirstSelectedEntry() const
{
    DictionaryEntry* pRet=nullptr;
    for( sal_Int32 nN=GetRowCount(); nN--; )
    {
        if( IsRowSelected( nN ) )
        {
            pRet = getEntryOnPos( nN );
            break;
        }
    }
    return pRet;
}

DictionaryEntry* DictionaryList::getEntryOnPos( sal_Int32 nPos ) const
{
    DictionaryEntry* pEntry=nullptr;
    SvTreeListEntry* pLBEntry = GetEntryOnPos( nPos );
    if(pLBEntry)
        pEntry = static_cast<DictionaryEntry*>(pLBEntry->GetUserData());
    return pEntry;
}

DictionaryEntry* DictionaryList::getTermEntry( const OUString& rTerm ) const
{
    for( sal_Int32 nN=GetRowCount(); nN--; )
    {
        DictionaryEntry* pE = getEntryOnPos( nN );
        if( pE && rTerm.equals( pE->m_aTerm ) )
            return pE;
    }
    return nullptr;
}

bool DictionaryList::hasTerm( const OUString& rTerm ) const
{
    return getTermEntry(rTerm) !=nullptr ;
}

void DictionaryList::addEntry( const OUString& rTerm, const OUString& rMapping
                              , sal_Int16 nConversionPropertyType, sal_uIntPtr nPos )
{
    if( hasTerm( rTerm ) )
        return;

    DictionaryEntry* pEntry = new DictionaryEntry( rTerm, rMapping, nConversionPropertyType, true );
    SvTreeListEntry* pLBEntry = InsertEntryToColumn( makeTabString( *pEntry ), nPos );
    pLBEntry->SetUserData( pEntry );
    SelectRow( GetEntryPos( pLBEntry ) );
}

void DictionaryList::deleteEntryOnPos( sal_Int32 nPos  )
{
    SvTreeListEntry* pLBEntry = GetEntryOnPos( nPos );
    DictionaryEntry* pEntry = getEntryOnPos( nPos );
    if( pLBEntry )
        RemoveParentKeepChildren( pLBEntry );
    if( pEntry )
    {
        if( pEntry->m_bNewEntry )
            delete pEntry;
        else
            m_aToBeDeleted.push_back( pEntry );
    }
}

sal_uIntPtr DictionaryList::deleteEntries( const OUString& rTerm )
{
    sal_uIntPtr nPos = TREELIST_APPEND;
    for( sal_Int32 nN=GetRowCount(); nN--; )
    {
        DictionaryEntry* pCurEntry = getEntryOnPos( nN );
        if( rTerm.equals( pCurEntry->m_aTerm ) )
        {
            nPos = nN;
            SvTreeListEntry* pCurLBEntry = GetEntryOnPos( nN );
            RemoveParentKeepChildren( pCurLBEntry );
            if( pCurEntry->m_bNewEntry )
                delete pCurEntry;
            else
                m_aToBeDeleted.push_back( pCurEntry );
        }
    }
    return nPos;
}

void DictionaryList::sortByColumn( sal_uInt16 nSortColumnIndex, bool bSortAtoZ )
{
    m_nSortColumnIndex=nSortColumnIndex;
    if( nSortColumnIndex<3 )
    {
        if(bSortAtoZ)
            GetModel()->SetSortMode(SortAscending);
        else
            GetModel()->SetSortMode(SortDescending);

        GetModel()->SetCompareHdl( LINK( this, DictionaryList, CompareHdl));
        GetModel()->Resort();
    }
    else
        GetModel()->SetSortMode(SortNone);
}


IMPL_LINK_TYPED( DictionaryList, CompareHdl, const SvSortData&, rData, sal_Int32 )
{
    SvTreeListEntry* pLeft = const_cast<SvTreeListEntry*>(rData.pLeft);
    SvTreeListEntry* pRight = const_cast<SvTreeListEntry*>(rData.pRight);
    return ColumnCompare(pLeft,pRight);
}

sal_Int32 DictionaryList::ColumnCompare( SvTreeListEntry* pLeft, SvTreeListEntry* pRight )
{
    sal_Int32 nCompare = 0;

    SvLBoxItem* pLeftItem = getItemAtColumn( pLeft, m_nSortColumnIndex );
    SvLBoxItem* pRightItem = getItemAtColumn( pRight, m_nSortColumnIndex );

    if(pLeftItem != nullptr && pRightItem != nullptr)
    {
        sal_uInt16 nLeftKind = pLeftItem->GetType();
        sal_uInt16 nRightKind = pRightItem->GetType();

        if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
            nLeftKind == SV_ITEM_ID_LBOXSTRING )
        {
            IntlWrapper aIntlWrapper( Application::GetSettings().GetLanguageTag() );
            const CollatorWrapper* pCollator = aIntlWrapper.getCaseCollator();

            nCompare = pCollator->compareString( static_cast<SvLBoxString*>(pLeftItem)->GetText(),
                                    static_cast<SvLBoxString*>(pRightItem)->GetText());

            if (nCompare == 0)
                nCompare = -1;
        }
    }
    return nCompare;
}

SvLBoxItem* DictionaryList::getItemAtColumn( SvTreeListEntry* pEntry, sal_uInt16 nColumn ) const
{
    SvLBoxItem* pItem = nullptr;
    if( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        nColumn++;
        if( nTreeFlags & SvTreeFlags::CHKBTN )
            nColumn++;
           if( nColumn < nCount )
            pItem = &pEntry->GetItem( nColumn );
    }
    return pItem;
}


DictionaryEntry::DictionaryEntry( const OUString& rTerm, const OUString& rMapping
                    , sal_Int16 nConversionPropertyType
                    , bool bNewEntry )
        : m_aTerm( rTerm )
        , m_aMapping( rMapping )
        , m_nConversionPropertyType( nConversionPropertyType )
        , m_bNewEntry( bNewEntry )
{
    if( m_nConversionPropertyType == 0 )
        m_nConversionPropertyType = 1;
}

DictionaryEntry::~DictionaryEntry()
{
}

bool DictionaryEntry::operator==( const DictionaryEntry& rE ) const
{
    return m_aTerm == rE.m_aTerm
            && m_aMapping == rE.m_aMapping
            && m_nConversionPropertyType == rE.m_nConversionPropertyType;
}

void DictionaryList::setColSizes()
{
    HeaderBar &rBar = GetTheHeaderBar();
    if (rBar.GetItemCount() < 3)
        return;

    long nWidth1 = m_pED_Term->get_preferred_size().Width();
    long nWidth2 = m_pED_Mapping->get_preferred_size().Width();
    long nWidth3 = m_pLB_Property->get_preferred_size().Width();

    long nWidth = GetSizePixel().Width();
    long nPos3 = nWidth - nWidth3;
    long nRemainder = nWidth - (nWidth1 + nWidth2 + nWidth3);

    long aStaticTabs[] = { 3, 0, nWidth1 + (nRemainder/2), nPos3 };
    SvSimpleTable::SetTabs(aStaticTabs, MAP_PIXEL);
}

void DictionaryList::Resize()
{
    SvSimpleTable::Resize();
    setColSizes();
}

void DictionaryList::init(const Reference< linguistic2::XConversionDictionary>& xDictionary,
    vcl::Window *pED_Term, vcl::Window *pED_Mapping, ListBox *pLB_Property,
    vcl::Window *pFT_Term, vcl::Window *pFT_Mapping, vcl::Window *pFT_Property)
{
    SetStyle( WB_VSCROLL | WB_TABSTOP );
    SetSelectionMode( SINGLE_SELECTION );
    SetBorderStyle( WindowBorderStyle::MONO );
    SetHighlightRange();

    if (m_xDictionary.is())
        return;

    m_xDictionary = xDictionary;

    m_pED_Term = pED_Term;
    m_pED_Mapping = pED_Mapping;
    m_pLB_Property = pLB_Property;

    HeaderBar& rHeaderBar = GetTheHeaderBar();

    OUString aColumn1( OutputDevice::GetNonMnemonicString( pFT_Term->GetText() ) );
    OUString aColumn2( OutputDevice::GetNonMnemonicString( pFT_Mapping->GetText() ) );
    OUString aColumn3( OutputDevice::GetNonMnemonicString( pFT_Property->GetText() ) );

    long nWidth1 = m_pED_Term->get_preferred_size().Width();
    long nWidth2 = m_pED_Mapping->get_preferred_size().Width();
    long nWidth3 = m_pLB_Property->get_preferred_size().Width();

    HeaderBarItemBits nBits = HEADER_BAR_BITS;
    rHeaderBar.InsertItem( 1, aColumn1, nWidth1, nBits | HeaderBarItemBits::UPARROW );
    rHeaderBar.InsertItem( 2, aColumn2, nWidth2, nBits );
    rHeaderBar.InsertItem( 3, aColumn3, nWidth3, nBits );

    long pTabs[] = { 3, 0, nWidth1, nWidth1 + nWidth2 };
    SetTabs( &pTabs[0], MAP_PIXEL );
}

void ChineseDictionaryDialog::initDictionaryControl(DictionaryList *pList,
    const Reference< linguistic2::XConversionDictionary>& xDictionary)
{
    //init HeaderBar and set tabs
    HeaderBar& rHeaderBar = pList->GetTheHeaderBar();
    //set hdl
    rHeaderBar.SetSelectHdl( LINK( this, ChineseDictionaryDialog, HeaderBarClick ) );
    //set widgets to track the width of for columns
    pList->init(xDictionary,
        m_pED_Term, m_pED_Mapping, m_pLB_Property,
        m_pFT_Term, m_pFT_Mapping, m_pFT_Property);
}

ChineseDictionaryDialog::ChineseDictionaryDialog( vcl::Window* pParent )
    : ModalDialog(pParent, "ChineseDictionaryDialog",
         "svx/ui/chinesedictionary.ui")
    , m_nTextConversionOptions(i18n::TextConversionOption::NONE)
    , m_xContext(nullptr)
{
    get(m_pRB_To_Simplified, "tradtosimple");
    get(m_pRB_To_Traditional, "simpletotrad");
    get(m_pCB_Reverse, "reverse");
    get(m_pFT_Term, "termft");
    get(m_pED_Term, "term");
    get(m_pFT_Mapping, "mappingft");
    get(m_pED_Mapping, "mapping");
    get(m_pFT_Property, "propertyft");
    get(m_pLB_Property, "property");

    get(m_pPB_Add, "add");
    get(m_pPB_Modify, "modify");
    get(m_pPB_Delete, "delete");

    get(mpToSimplifiedContainer, "tradtosimpleview");
    mpToSimplifiedContainer->set_height_request(mpToSimplifiedContainer->GetTextHeight() * 8);
    m_pCT_DictionaryToSimplified = VclPtr<DictionaryList>::Create(*mpToSimplifiedContainer, 0);
    get(mpToTraditionalContainer, "simpletotradview");
    mpToTraditionalContainer->set_height_request(mpToTraditionalContainer->GetTextHeight() * 8);
    m_pCT_DictionaryToTraditional = VclPtr<DictionaryList>::Create(*mpToTraditionalContainer, 0);

    SvtLinguConfig  aLngCfg;
    bool bValue;
    Any aAny( aLngCfg.GetProperty( OUString( UPN_IS_REVERSE_MAPPING ) ) );
    if( aAny >>= bValue )
        m_pCB_Reverse->Check( bValue );

    m_pLB_Property->SetDropDownLineCount( m_pLB_Property->GetEntryCount() );
    m_pLB_Property->SelectEntryPos(0);

    Reference< linguistic2::XConversionDictionary > xDictionary_To_Simplified(nullptr);
    Reference< linguistic2::XConversionDictionary > xDictionary_To_Traditional(nullptr);
    //get dictionaries
    {
        if(!m_xContext.is())
            m_xContext.set( ::cppu::defaultBootstrap_InitialComponentContext() );
        if(m_xContext.is())
        {
            Reference< linguistic2::XConversionDictionaryList > xDictionaryList = linguistic2::ConversionDictionaryList::create(m_xContext);
            Reference< container::XNameContainer > xContainer( xDictionaryList->getDictionaryContainer() );
            if(xContainer.is())
            {
                try
                {
                    OUString aNameTo_Simplified("ChineseT2S");
                    OUString aNameTo_Traditional("ChineseS2T");
                    lang::Locale aLocale;
                    aLocale.Language = "zh";

                    if( xContainer->hasByName( aNameTo_Simplified ) )
                        xDictionary_To_Simplified.set(
                                xContainer->getByName( aNameTo_Simplified ), UNO_QUERY );
                    else
                    {
                        aLocale.Country = "TW";
                        xDictionary_To_Simplified.set(
                                xDictionaryList->addNewDictionary( aNameTo_Simplified
                                    , aLocale, linguistic2::ConversionDictionaryType::SCHINESE_TCHINESE
                                ), UNO_QUERY );
                    }
                    if (xDictionary_To_Simplified.is())
                        xDictionary_To_Simplified->setActive( sal_True );


                    if( xContainer->hasByName( aNameTo_Traditional ) )
                        xDictionary_To_Traditional.set(
                                xContainer->getByName( aNameTo_Traditional ), UNO_QUERY );
                    else
                    {
                        aLocale.Country = "CN";
                        xDictionary_To_Traditional.set(
                                xDictionaryList->addNewDictionary( aNameTo_Traditional
                                    ,aLocale, linguistic2::ConversionDictionaryType::SCHINESE_TCHINESE),
                                UNO_QUERY );
                    }
                    if (xDictionary_To_Traditional.is())
                        xDictionary_To_Traditional->setActive( sal_True );

                }
                catch(const uno::Exception& )
                {
                }
            }
        }
    }

    //init dictionary controls
    initDictionaryControl(m_pCT_DictionaryToSimplified, xDictionary_To_Simplified);
    initDictionaryControl(m_pCT_DictionaryToTraditional, xDictionary_To_Traditional);

    updateAfterDirectionChange();

    m_pED_Term->SetModifyHdl( LINK( this, ChineseDictionaryDialog, EditFieldsHdl ) );
    m_pED_Mapping->SetModifyHdl( LINK( this, ChineseDictionaryDialog, EditFieldsHdl ) );
    m_pLB_Property->SetSelectHdl( LINK( this, ChineseDictionaryDialog, EditFieldsListBoxHdl ) );

    m_pRB_To_Simplified->SetClickHdl( LINK( this, ChineseDictionaryDialog, DirectionHdl ) );
    m_pRB_To_Traditional->SetClickHdl( LINK( this, ChineseDictionaryDialog, DirectionHdl ) );

    m_pCT_DictionaryToSimplified->SetSelectHdl( LINK( this, ChineseDictionaryDialog, MappingSelectHdl ));
    m_pCT_DictionaryToTraditional->SetSelectHdl( LINK( this, ChineseDictionaryDialog, MappingSelectHdl ));

    m_pPB_Add->SetClickHdl( LINK( this, ChineseDictionaryDialog, AddHdl ) );
    m_pPB_Modify->SetClickHdl( LINK( this, ChineseDictionaryDialog, ModifyHdl ) );
    m_pPB_Delete->SetClickHdl( LINK( this, ChineseDictionaryDialog, DeleteHdl ) );
}

ChineseDictionaryDialog::~ChineseDictionaryDialog()
{
    disposeOnce();
}

void ChineseDictionaryDialog::dispose()
{
    m_xContext=nullptr;
    m_pCT_DictionaryToSimplified.disposeAndClear();
    m_pCT_DictionaryToTraditional.disposeAndClear();
    m_pRB_To_Simplified.clear();
    m_pRB_To_Traditional.clear();
    m_pCB_Reverse.clear();
    m_pFT_Term.clear();
    m_pED_Term.clear();
    m_pFT_Mapping.clear();
    m_pED_Mapping.clear();
    m_pFT_Property.clear();
    m_pLB_Property.clear();
    mpToSimplifiedContainer.clear();
    mpToTraditionalContainer.clear();
    m_pPB_Add.clear();
    m_pPB_Modify.clear();
    m_pPB_Delete.clear();
    ModalDialog::dispose();
}

void ChineseDictionaryDialog::setDirectionAndTextConversionOptions( bool bDirectionToSimplified, sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ )
{
    if( bDirectionToSimplified == bool(m_pRB_To_Simplified->IsChecked())
        && nTextConversionOptions == m_nTextConversionOptions )
        return;

    m_nTextConversionOptions = nTextConversionOptions;

    if( bDirectionToSimplified )
        m_pRB_To_Simplified->Check();
    else
        m_pRB_To_Traditional->Check();
    updateAfterDirectionChange();
}

IMPL_LINK_NOARG_TYPED(ChineseDictionaryDialog, DirectionHdl, Button*, void)
{
    updateAfterDirectionChange();
}

void ChineseDictionaryDialog::updateAfterDirectionChange()
{
    Reference< linguistic2::XConversionDictionary > xDictionary(nullptr);

    if( m_pRB_To_Simplified->IsChecked() )
    {
        mpToTraditionalContainer->Hide();
        mpToSimplifiedContainer->Show();
        xDictionary = m_pCT_DictionaryToSimplified->m_xDictionary;
    }
    else
    {
        mpToSimplifiedContainer->Hide();
        mpToTraditionalContainer->Show();
        xDictionary = m_pCT_DictionaryToTraditional->m_xDictionary;
    }

    updateButtons();
}

IMPL_LINK_NOARG_TYPED(ChineseDictionaryDialog, EditFieldsListBoxHdl, ListBox&, void)
{
    updateButtons();
}
IMPL_LINK_NOARG_TYPED(ChineseDictionaryDialog, EditFieldsHdl, Edit&, void)
{
    updateButtons();
}
IMPL_LINK_NOARG_TYPED(ChineseDictionaryDialog, MappingSelectHdl, SvTreeListBox*, void)
{
    DictionaryEntry* pE = getActiveDictionary().getFirstSelectedEntry();
    if(pE)
    {
        m_pED_Term->SetText( pE->m_aTerm );
        m_pED_Mapping->SetText( pE->m_aMapping );
        sal_Int16 nPos = pE->m_nConversionPropertyType-1;
        if( nPos<0 || nPos>=m_pLB_Property->GetEntryCount() )
            nPos=0;
        if( m_pLB_Property->GetEntryCount() )
            m_pLB_Property->SelectEntryPos(nPos);
    }

    updateButtons();
}

bool ChineseDictionaryDialog::isEditFieldsHaveContent() const
{
    return !m_pED_Term->GetText().isEmpty() && !m_pED_Mapping->GetText().isEmpty();
}

bool ChineseDictionaryDialog::isEditFieldsContentEqualsSelectedListContent() const
{
    DictionaryEntry* pE = getActiveDictionary().getFirstSelectedEntry();
    if( pE )
    {
        if( pE->m_aTerm != m_pED_Term->GetText() )
            return false;
        if( pE->m_aMapping != m_pED_Mapping->GetText() )
            return false;
        if( pE->m_nConversionPropertyType != m_pLB_Property->GetSelectEntryPos()+1 )
            return false;
        return true;
    }
    return false;
}

const DictionaryList& ChineseDictionaryDialog::getActiveDictionary() const
{
    if( m_pRB_To_Traditional->IsChecked() )
        return *m_pCT_DictionaryToTraditional;
    return *m_pCT_DictionaryToSimplified;
}

DictionaryList& ChineseDictionaryDialog::getActiveDictionary()
{
    if( m_pRB_To_Traditional->IsChecked() )
        return *m_pCT_DictionaryToTraditional;
    return *m_pCT_DictionaryToSimplified;
}

const DictionaryList& ChineseDictionaryDialog::getReverseDictionary() const
{
    if( m_pRB_To_Traditional->IsChecked() )
        return *m_pCT_DictionaryToSimplified;
    return *m_pCT_DictionaryToTraditional;
}

DictionaryList& ChineseDictionaryDialog::getReverseDictionary()
{
    if( m_pRB_To_Traditional->IsChecked() )
        return *m_pCT_DictionaryToSimplified;
    return *m_pCT_DictionaryToTraditional;
}

void ChineseDictionaryDialog::updateButtons()
{
    bool bAdd = isEditFieldsHaveContent() && !getActiveDictionary().hasTerm( m_pED_Term->GetText() );
    m_pPB_Add->Enable( bAdd );

    m_pPB_Delete->Enable( !bAdd && getActiveDictionary().GetSelectedRowCount()>0 );

//    DictionaryEntry* pFirstSelectedEntry = getActiveDictionary().getFirstSelectedEntry();

    bool bModify = false;
    {
        DictionaryEntry* pFirstSelectedEntry = getActiveDictionary().getFirstSelectedEntry();
        bModify = !bAdd && getActiveDictionary().GetSelectedRowCount()==1
                        && pFirstSelectedEntry && pFirstSelectedEntry->m_aTerm.equals( m_pED_Term->GetText() );
        if( bModify && isEditFieldsContentEqualsSelectedListContent() )
            bModify = false;
    }
    m_pPB_Modify->Enable( bModify );
}

IMPL_LINK_NOARG_TYPED(ChineseDictionaryDialog, AddHdl, Button*, void)
{
    if( !isEditFieldsHaveContent() )
        return;

    sal_Int16 nConversionPropertyType = m_pLB_Property->GetSelectEntryPos()+1;

    getActiveDictionary().addEntry( m_pED_Term->GetText(), m_pED_Mapping->GetText(), nConversionPropertyType );

    if( m_pCB_Reverse->IsChecked() )
    {
        getReverseDictionary().deleteEntries( m_pED_Mapping->GetText() );
        getReverseDictionary().addEntry( m_pED_Mapping->GetText(), m_pED_Term->GetText(), nConversionPropertyType );
    }

    updateButtons();
}

IMPL_LINK_NOARG_TYPED(ChineseDictionaryDialog, ModifyHdl, Button*, void)
{
    OUString aTerm( m_pED_Term->GetText() );
    OUString aMapping( m_pED_Mapping->GetText() );
    sal_Int16 nConversionPropertyType = m_pLB_Property->GetSelectEntryPos()+1;

    DictionaryList& rActive  = getActiveDictionary();
    DictionaryList& rReverse = getReverseDictionary();

    DictionaryEntry* pE = rActive.getFirstSelectedEntry();
    if( pE && pE->m_aTerm != aTerm )
        return;

    if( pE )
    {
        if( pE->m_aMapping != aMapping || pE->m_nConversionPropertyType != nConversionPropertyType )
        {
            if( m_pCB_Reverse->IsChecked() )
            {
                rReverse.deleteEntries( pE->m_aMapping );
                sal_uIntPtr nPos = rReverse.deleteEntries( aMapping );
                rReverse.addEntry( aMapping, aTerm, nConversionPropertyType, nPos );
            }

            sal_uIntPtr nPos = rActive.deleteEntries( aTerm );
            rActive.addEntry( aTerm, aMapping, nConversionPropertyType, nPos );
        }
    }

    updateButtons();
}

IMPL_LINK_NOARG_TYPED(ChineseDictionaryDialog, DeleteHdl, Button*, void)
{
    DictionaryList& rActive  = getActiveDictionary();
    DictionaryList& rReverse = getReverseDictionary();

    if( rActive.GetSelectedRowCount()>0)
    {
        DictionaryEntry* pEntry;

        OUString aMapping;
        for( sal_Int32 nN=rActive.GetRowCount(); nN--; )
        {
            if( rActive.IsRowSelected( nN ) )
            {
                pEntry = rActive.getEntryOnPos( nN );
                if(pEntry)
                {
                    aMapping = pEntry->m_aMapping;
                    rActive.deleteEntryOnPos( nN );
                    if( m_pCB_Reverse->IsChecked() )
                        rReverse.deleteEntries( aMapping  );
                }
                break;
            }
        }
    }

    updateButtons();
}

short ChineseDictionaryDialog::Execute()
{
    sal_Int32 nTextConversionOptions = m_nTextConversionOptions;
    if(m_nTextConversionOptions & i18n::TextConversionOption::USE_CHARACTER_VARIANTS )
        nTextConversionOptions = nTextConversionOptions^i18n::TextConversionOption::USE_CHARACTER_VARIANTS ;

    m_pCT_DictionaryToSimplified->refillFromDictionary( nTextConversionOptions );
    m_pCT_DictionaryToTraditional->refillFromDictionary( m_nTextConversionOptions );

    short nRet = ModalDialog::Execute();

    if( nRet == RET_OK )
    {
        //save settings to configuration
        SvtLinguConfig  aLngCfg;
        Any aAny;
        aAny <<= m_pCB_Reverse->IsChecked();
        aLngCfg.SetProperty( OUString( UPN_IS_REVERSE_MAPPING ), aAny );

        m_pCT_DictionaryToSimplified->save();
        m_pCT_DictionaryToTraditional->save();
    }

    m_pCT_DictionaryToSimplified->deleteAll();
    m_pCT_DictionaryToTraditional->deleteAll();

    return nRet;
}

IMPL_LINK_TYPED(ChineseDictionaryDialog, HeaderBarClick, HeaderBar*, pHeaderBar, void)
{
    sal_uInt16 nId = pHeaderBar->GetCurItemId();
    HeaderBarItemBits nBits = pHeaderBar->GetItemBits(nId);
    if( nBits & HeaderBarItemBits::CLICKABLE )
    {
        //set new arrow positions in headerbar
        pHeaderBar->SetItemBits( getActiveDictionary().getSortColumn()+1, HEADER_BAR_BITS );
        if( nBits & HeaderBarItemBits::UPARROW )
            pHeaderBar->SetItemBits( nId, HEADER_BAR_BITS | HeaderBarItemBits::DOWNARROW );
        else
            pHeaderBar->SetItemBits( nId, HEADER_BAR_BITS | HeaderBarItemBits::UPARROW );

        //sort lists
        nBits = pHeaderBar->GetItemBits(nId);
        bool bSortAtoZ = bool(nBits & HeaderBarItemBits::UPARROW);
        getActiveDictionary().sortByColumn(nId-1,bSortAtoZ);
        getReverseDictionary().sortByColumn(nId-1,bSortAtoZ);
    }
}


} //end namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
