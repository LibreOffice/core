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
#include "chinese_dictionarydialog.hrc"
#include "resid.hxx"
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionPropertyType.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryList.hpp>
#include <com/sun/star/linguistic2/XConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/Locale.hpp>
// header for class HeaderBar
#include <svtools/headbar.hxx>
#include <svtools/svlbitm.hxx>
#include "svtools/treelistentry.hxx"
// header for define RET_OK
#include <vcl/msgbox.hxx>
// header for class SvtLinguConfigItem
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
// header for class IntlWrapper
#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>
// header for class Application
#include <vcl/svapp.hxx>
#include "helpid.hrc"

//disable compiler warning C4355: 'this' : used in base member initializer list
#ifdef _MSC_VER
#  pragma warning (disable : 4355)
#endif

//.............................................................................
namespace textconversiondlgs
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define HEADER_BAR_BITS ( HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_FIXED | HIB_FIXEDPOS )

DictionaryList::DictionaryList( Window* pParent, const ResId& rResId)
    : SvHeaderTabListBox( pParent, rResId )
    , m_xDictionary(0)
    , m_pHeaderBar(0)
    , m_pPropertyTypeNameListBox(0)
    , m_aToBeDeleted()
    , m_nSortColumnIndex(0)
{
}

DictionaryList::DictionaryList( Window* pParent )
    : SvHeaderTabListBox( pParent, 0 )
    , m_xDictionary(0)
    , m_pHeaderBar(0)
    , m_pPropertyTypeNameListBox(0)
    , m_aToBeDeleted()
    , m_nSortColumnIndex(0)
{
}

OUString DictionaryList::getPropertyTypeName( sal_Int16 nConversionPropertyType ) const
{
    if(!m_pPropertyTypeNameListBox || !m_pPropertyTypeNameListBox->GetEntryCount())
        return String();

    sal_uInt16 nPos = static_cast<sal_uInt16>( nConversionPropertyType )-1;
    if(nPos<m_pPropertyTypeNameListBox->GetEntryCount())
        return m_pPropertyTypeNameListBox->GetEntry(nPos);
    return m_pPropertyTypeNameListBox->GetEntry(0);
}

OUString DictionaryList::makeTabString( const DictionaryEntry& rEntry ) const
{
    String aStr( rEntry.m_aTerm );
    aStr += '\t';
    aStr += String( rEntry.m_aMapping );
    aStr += '\t';
    aStr += getPropertyTypeName( rEntry.m_nConversionPropertyType );
    return aStr;
}

void DictionaryList::initDictionaryControl( const Reference< linguistic2::XConversionDictionary>& xDictionary
                                           , ListBox* pPropertyTypeNameListBox )
{
    SetStyle( WB_VSCROLL | WB_TABSTOP );
    SetSelectionMode( SINGLE_SELECTION );
    SetBorderStyle( WINDOW_BORDER_MONO );
    SetHighlightRange();

    if(m_xDictionary.is())
        return;

    m_xDictionary = xDictionary;
    m_pPropertyTypeNameListBox = pPropertyTypeNameListBox;
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
    DictionaryEntry* pRet=0;
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
    DictionaryEntry* pEntry=0;
    SvTreeListEntry* pLBEntry = GetEntryOnPos( nPos );
    if(pLBEntry)
        pEntry = (DictionaryEntry*)pLBEntry->GetUserData();
    return pEntry;
}

DictionaryEntry* DictionaryList::getTermEntry( const OUString& rTerm ) const
{
    DictionaryEntry* pE = 0;
    for( sal_Int32 nN=GetRowCount(); nN--; )
    {
        pE = getEntryOnPos( nN );
        if( pE && rTerm.equals( pE->m_aTerm ) )
            return pE;
    }
    return 0;
}

bool DictionaryList::hasTerm( const OUString& rTerm ) const
{
    return getTermEntry(rTerm) !=0 ;
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
    sal_uIntPtr nPos = LIST_APPEND;
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

DictionaryList::~DictionaryList()
{
}

void DictionaryList::activate( HeaderBar* pHeaderBar )
{
    if(!m_pHeaderBar)
    {
        m_pHeaderBar = pHeaderBar;

        Point aPos = GetPosPixel();
        Size  aSize = GetSizePixel();
        Size aHeadSize = pHeaderBar->GetSizePixel();

        aPos.Y() += aHeadSize.Height();
        SetPosSizePixel( aPos, Size( aSize.Width(), aSize.Height() - aHeadSize.Height() ) );
        InitHeaderBar( pHeaderBar );
    }
    Show();
}

HeaderBar* DictionaryList::createHeaderBar( const OUString& rColumn1, const OUString& rColumn2, const OUString& rColumn3
                  , long nWidth1, long nWidth2, long nWidth3 )
{
    HeaderBar* pHeaderBar = new HeaderBar( Control::GetParent(), WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    pHeaderBar->SetPosSizePixel( GetPosPixel(), pHeaderBar->CalcWindowSizePixel() );

    HeaderBarItemBits nBits = HEADER_BAR_BITS;
    pHeaderBar->InsertItem( 1, rColumn1, nWidth1, nBits | HIB_UPARROW );
    pHeaderBar->InsertItem( 2, rColumn2, nWidth2, nBits );
    pHeaderBar->InsertItem( 3, rColumn3, nWidth3, nBits );

    pHeaderBar->Show();
    return pHeaderBar;
}

void DictionaryList::Resize()
{
    SvHeaderTabListBox::Resize();
    Size aBoxSize = GetOutputSizePixel();

    if ( !aBoxSize.Width() )
        return;

       Size aBarSize = m_pHeaderBar->GetSizePixel();
    aBarSize.Width() = GetSizePixel().Width();
    m_pHeaderBar->SetSizePixel( aBarSize );
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

sal_uInt16 DictionaryList::getSortColumn() const
{
    return m_nSortColumnIndex;
}

IMPL_LINK( DictionaryList, CompareHdl, SvSortData*, pData )
{
    SvTreeListEntry* pLeft = (SvTreeListEntry*)(pData->pLeft );
    SvTreeListEntry* pRight = (SvTreeListEntry*)(pData->pRight );
    return (long) ColumnCompare(pLeft,pRight);
}

StringCompare DictionaryList::ColumnCompare( SvTreeListEntry* pLeft, SvTreeListEntry* pRight )
{
    StringCompare eCompare=COMPARE_EQUAL;

    SvLBoxItem* pLeftItem = getItemAtColumn( pLeft, m_nSortColumnIndex );
    SvLBoxItem* pRightItem = getItemAtColumn( pRight, m_nSortColumnIndex );

    if(pLeftItem != NULL && pRightItem != NULL)
    {
        sal_uInt16 nLeftKind = pLeftItem->GetType();
        sal_uInt16 nRightKind = pRightItem->GetType();

        if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
            nLeftKind == SV_ITEM_ID_LBOXSTRING )
        {
            IntlWrapper aIntlWrapper( Application::GetSettings().GetLanguageTag() );
            const CollatorWrapper* pCollator = aIntlWrapper.getCaseCollator();

            eCompare=(StringCompare)pCollator->compareString( ((SvLBoxString*)pLeftItem)->GetText(),
                                    ((SvLBoxString*)pRightItem)->GetText());

            if(eCompare==COMPARE_EQUAL)
                eCompare=COMPARE_LESS;
        }
    }
    return eCompare;
}

SvLBoxItem* DictionaryList::getItemAtColumn( SvTreeListEntry* pEntry, sal_uInt16 nColumn ) const
{
    SvLBoxItem* pItem = NULL;
    if( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        nColumn++;
        if( nTreeFlags & TREEFLAG_CHKBTN )
            nColumn++;
           if( nColumn < nCount )
            pItem = pEntry->GetItem( nColumn );
    }
    return pItem;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

ChineseDictionaryDialog::ChineseDictionaryDialog( Window* pParent )
    : ModalDialog( pParent, TextConversionDlgs_ResId( DLG_CHINESEDICTIONARY ) )
    , m_nTextConversionOptions( i18n::TextConversionOption::NONE )
    , m_aRB_To_Simplified( this, TextConversionDlgs_ResId( RB_TO_SIMPLIFIED ) )
    , m_aRB_To_Traditional( this, TextConversionDlgs_ResId( RB_TO_TRADITIONAL ) )
    , m_aCB_Reverse( this, TextConversionDlgs_ResId( CB_REVERSE ) )
    , m_aFT_Term( this, TextConversionDlgs_ResId( FT_TERM ) )
    , m_aED_Term( this, TextConversionDlgs_ResId( ED_TERM ) )
    , m_aFT_Mapping( this, TextConversionDlgs_ResId( FT_MAPPING ) )
    , m_aED_Mapping( this, TextConversionDlgs_ResId( ED_MAPPING ) )
    , m_aFT_Property( this, TextConversionDlgs_ResId( FT_PROPERTY ) )
    , m_aLB_Property( this, TextConversionDlgs_ResId( LB_PROPERTY ) )
    , m_pHeaderBar( 0 )
    , m_aCT_DictionaryToSimplified( this, TextConversionDlgs_ResId( CT_MAPPINGLIST ) )
    , m_aCT_DictionaryToTraditional( this )
    , m_aPB_Add( this, TextConversionDlgs_ResId( PB_ADD ) )
    , m_aPB_Modify( this, TextConversionDlgs_ResId( PB_MODIFY ) )
    , m_aPB_Delete( this, TextConversionDlgs_ResId( PB_DELETE ) )
    , m_aFL_Bottomline( this, TextConversionDlgs_ResId( FL_BOTTOMLINE ) )
    , m_aBP_OK( this, TextConversionDlgs_ResId( PB_OK ) )
    , m_aBP_Cancel( this, TextConversionDlgs_ResId( PB_CANCEL ) )
    , m_aBP_Help( this, TextConversionDlgs_ResId( PB_HELP ) )
    , m_xContext( 0 )
{
    FreeResource();

    m_aRB_To_Simplified.SetHelpId( HID_SVX_CHINESE_DICTIONARY_RB_CONVERSION_TO_SIMPLIFIED );
    m_aRB_To_Traditional.SetHelpId( HID_SVX_CHINESE_DICTIONARY_RB_CONVERSION_TO_TRADITIONAL );

    m_aCB_Reverse.SetHelpId( HID_SVX_CHINESE_DICTIONARY_CB_REVERSE );

    m_aCT_DictionaryToSimplified.SetHelpId( HID_SVX_CHINESE_DICTIONARY_LB_TO_SIMPLIFIED );
    m_aCT_DictionaryToTraditional.SetHelpId( HID_SVX_CHINESE_DICTIONARY_LB_TO_TRADITIONAL );

    SvtLinguConfig  aLngCfg;
    sal_Bool bValue = sal_Bool();
    Any aAny( aLngCfg.GetProperty( OUString( UPN_IS_REVERSE_MAPPING ) ) );
    if( aAny >>= bValue )
        m_aCB_Reverse.Check( bValue );

    m_aLB_Property.SetDropDownLineCount( m_aLB_Property.GetEntryCount() );
    m_aLB_Property.SelectEntryPos(0);

    Reference< linguistic2::XConversionDictionary > xDictionary_To_Simplified(0);
    Reference< linguistic2::XConversionDictionary > xDictionary_To_Traditional(0);
    //get dictionaries
    {
        if(!m_xContext.is())
            m_xContext = Reference< XComponentContext >( ::cppu::defaultBootstrap_InitialComponentContext() );
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
                    aLocale.Language = OUString("zh");

                    if( xContainer->hasByName( aNameTo_Simplified ) )
                        xDictionary_To_Simplified = Reference< linguistic2::XConversionDictionary >(
                                xContainer->getByName( aNameTo_Simplified ), UNO_QUERY );
                    else
                    {
                        aLocale.Country = OUString("TW");
                        xDictionary_To_Simplified = Reference< linguistic2::XConversionDictionary >(
                                xDictionaryList->addNewDictionary( aNameTo_Simplified
                                , aLocale, linguistic2::ConversionDictionaryType::SCHINESE_TCHINESE
                                    ), UNO_QUERY );
                    }
                    if (xDictionary_To_Simplified.is())
                        xDictionary_To_Simplified->setActive( sal_True );


                    if( xContainer->hasByName( aNameTo_Traditional ) )
                        xDictionary_To_Traditional = Reference< linguistic2::XConversionDictionary >(
                                xContainer->getByName( aNameTo_Traditional ), UNO_QUERY );
                    else
                    {
                        aLocale.Country = OUString("CN");
                        xDictionary_To_Traditional = Reference< linguistic2::XConversionDictionary >(
                                xDictionaryList->addNewDictionary( aNameTo_Traditional
                                , aLocale, linguistic2::ConversionDictionaryType::SCHINESE_TCHINESE
                                    ), UNO_QUERY );
                    }
                    if (xDictionary_To_Traditional.is())
                        xDictionary_To_Traditional->setActive( sal_True );

                }
                catch( uno::Exception& )
                {
                }
            }
        }
    }

    //init HeaderBar and set tabs
    {
        String aColumn1( OutputDevice::GetNonMnemonicString( m_aFT_Term.GetText() ) );
        String aColumn2( OutputDevice::GetNonMnemonicString( m_aFT_Mapping.GetText() ) );
        String aColumn3( OutputDevice::GetNonMnemonicString( m_aFT_Property.GetText() ) );

        long nWidth1 = m_aED_Mapping.GetPosPixel().X() - m_aED_Term.GetPosPixel().X();
        long nWidth2 = m_aLB_Property.GetPosPixel().X() - m_aED_Mapping.GetPosPixel().X();
        long nWidth3 = m_aLB_Property.GetSizePixel().Width();

        m_pHeaderBar = m_aCT_DictionaryToSimplified.createHeaderBar( aColumn1, aColumn2, aColumn3, nWidth1, nWidth2, nWidth3 );
        if(m_pHeaderBar)
            m_pHeaderBar->SetHelpId( HID_SVX_CHINESE_DICTIONARY_LB_HEADER );

        long pTabs[] = { 3, 0, nWidth1, nWidth1 + nWidth2 };
        m_aCT_DictionaryToSimplified.SetTabs( &pTabs[0], MAP_PIXEL );
        m_aCT_DictionaryToTraditional.SetTabs( &pTabs[0], MAP_PIXEL );
    }

    //init dictionary controls
    m_aCT_DictionaryToTraditional.SetPosPixel( m_aCT_DictionaryToSimplified.GetPosPixel() );
    m_aCT_DictionaryToTraditional.SetSizePixel( m_aCT_DictionaryToSimplified.GetSizePixel() );

    m_aCT_DictionaryToSimplified.initDictionaryControl( xDictionary_To_Simplified, &m_aLB_Property );
    m_aCT_DictionaryToTraditional.initDictionaryControl( xDictionary_To_Traditional, &m_aLB_Property );

    //
    updateAfterDirectionChange();

    //set hdl
    if(m_pHeaderBar)
        m_pHeaderBar->SetSelectHdl( LINK( this, ChineseDictionaryDialog, HeaderBarClick ) );

    m_aED_Term.SetModifyHdl( LINK( this, ChineseDictionaryDialog, EditFieldsHdl ) );
    m_aED_Mapping.SetModifyHdl( LINK( this, ChineseDictionaryDialog, EditFieldsHdl ) );
    m_aLB_Property.SetSelectHdl( LINK( this, ChineseDictionaryDialog, EditFieldsHdl ) );

    m_aRB_To_Simplified.SetClickHdl( LINK( this, ChineseDictionaryDialog, DirectionHdl ) );
    m_aRB_To_Traditional.SetClickHdl( LINK( this, ChineseDictionaryDialog, DirectionHdl ) );

    m_aCT_DictionaryToSimplified.SetSelectHdl( LINK( this, ChineseDictionaryDialog, MappingSelectHdl ));
    m_aCT_DictionaryToTraditional.SetSelectHdl( LINK( this, ChineseDictionaryDialog, MappingSelectHdl ));

    m_aPB_Add.SetClickHdl( LINK( this, ChineseDictionaryDialog, AddHdl ) );
    m_aPB_Modify.SetClickHdl( LINK( this, ChineseDictionaryDialog, ModifyHdl ) );
    m_aPB_Delete.SetClickHdl( LINK( this, ChineseDictionaryDialog, DeleteHdl ) );
}

ChineseDictionaryDialog::~ChineseDictionaryDialog()
{
    m_xContext=0;
    delete m_pHeaderBar;
}

void ChineseDictionaryDialog::setDirectionAndTextConversionOptions( bool bDirectionToSimplified, sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ )
{
    if( bDirectionToSimplified == bool(m_aRB_To_Simplified.IsChecked())
        && nTextConversionOptions == m_nTextConversionOptions )
        return;

    m_nTextConversionOptions = nTextConversionOptions;

    if( bDirectionToSimplified )
        m_aRB_To_Simplified.Check();
    else
        m_aRB_To_Traditional.Check();
    updateAfterDirectionChange();
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, DirectionHdl)
{
    updateAfterDirectionChange();
    return 0;
}

void ChineseDictionaryDialog::updateAfterDirectionChange()
{
    Reference< linguistic2::XConversionDictionary > xDictionary(0);

    if( m_aRB_To_Simplified.IsChecked() )
    {
        m_aCT_DictionaryToSimplified.activate( m_pHeaderBar );
        m_aCT_DictionaryToTraditional.Hide();
        xDictionary = m_aCT_DictionaryToSimplified.m_xDictionary;
    }
    else
    {
        m_aCT_DictionaryToTraditional.activate( m_pHeaderBar );
        m_aCT_DictionaryToSimplified.Hide();
        xDictionary = m_aCT_DictionaryToTraditional.m_xDictionary;
    }

    updateButtons();
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, EditFieldsHdl)
{
    updateButtons();
    return 0;
}
IMPL_LINK_NOARG(ChineseDictionaryDialog, MappingSelectHdl)
{
    DictionaryEntry* pE = getActiveDictionary().getFirstSelectedEntry();
    if(pE)
    {
        m_aED_Term.SetText( pE->m_aTerm );
        m_aED_Mapping.SetText( pE->m_aMapping );
        sal_Int16 nPos = pE->m_nConversionPropertyType-1;
        if( nPos<0 || nPos>=m_aLB_Property.GetEntryCount() )
            nPos=0;
        if( m_aLB_Property.GetEntryCount() )
            m_aLB_Property.SelectEntryPos(nPos);
    }

    updateButtons();
    return 0;
}

bool ChineseDictionaryDialog::isEditFieldsHaveContent() const
{
    return !m_aED_Term.GetText().isEmpty() && !m_aED_Mapping.GetText().isEmpty();
}

bool ChineseDictionaryDialog::isEditFieldsContentEqualsSelectedListContent() const
{
    DictionaryEntry* pE = getActiveDictionary().getFirstSelectedEntry();
    if( pE )
    {
        if( pE->m_aTerm != OUString( m_aED_Term.GetText() ) )
            return false;
        if( pE->m_aMapping != OUString( m_aED_Mapping.GetText() ) )
            return false;
        if( pE->m_nConversionPropertyType != m_aLB_Property.GetSelectEntryPos()+1 )
            return false;
        return true;
    }
    return false;
}

const DictionaryList& ChineseDictionaryDialog::getActiveDictionary() const
{
    if( m_aRB_To_Traditional.IsChecked() )
        return m_aCT_DictionaryToTraditional;
    return m_aCT_DictionaryToSimplified;
}

DictionaryList& ChineseDictionaryDialog::getActiveDictionary()
{
    if( m_aRB_To_Traditional.IsChecked() )
        return m_aCT_DictionaryToTraditional;
    return m_aCT_DictionaryToSimplified;
}

const DictionaryList& ChineseDictionaryDialog::getReverseDictionary() const
{
    if( m_aRB_To_Traditional.IsChecked() )
        return m_aCT_DictionaryToSimplified;
    return m_aCT_DictionaryToTraditional;
}

DictionaryList& ChineseDictionaryDialog::getReverseDictionary()
{
    if( m_aRB_To_Traditional.IsChecked() )
        return m_aCT_DictionaryToSimplified;
    return m_aCT_DictionaryToTraditional;
}

void ChineseDictionaryDialog::updateButtons()
{
    bool bAdd = isEditFieldsHaveContent() && !getActiveDictionary().hasTerm( m_aED_Term.GetText() );
    m_aPB_Add.Enable( bAdd );

    m_aPB_Delete.Enable( !bAdd && getActiveDictionary().GetSelectedRowCount()>0 );

//    DictionaryEntry* pFirstSelectedEntry = getActiveDictionary().getFirstSelectedEntry();

    bool bModify = false;
    {
        DictionaryEntry* pFirstSelectedEntry = getActiveDictionary().getFirstSelectedEntry();
        bModify = !bAdd && getActiveDictionary().GetSelectedRowCount()==1
                        && pFirstSelectedEntry && pFirstSelectedEntry->m_aTerm.equals( m_aED_Term.GetText() );
        if( bModify && isEditFieldsContentEqualsSelectedListContent() )
            bModify = false;
    }
    m_aPB_Modify.Enable( bModify );
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, AddHdl)
{
    if( !isEditFieldsHaveContent() )
        return 0;

    sal_Int16 nConversionPropertyType = m_aLB_Property.GetSelectEntryPos()+1;

    getActiveDictionary().addEntry( m_aED_Term.GetText(), m_aED_Mapping.GetText(), nConversionPropertyType );

    if( m_aCB_Reverse.IsChecked() )
    {
        getReverseDictionary().deleteEntries( m_aED_Mapping.GetText() );
        getReverseDictionary().addEntry( m_aED_Mapping.GetText(), m_aED_Term.GetText(), nConversionPropertyType );
    }

    updateButtons();
    return 0;
}
IMPL_LINK_NOARG(ChineseDictionaryDialog, ModifyHdl)
{
    OUString aTerm( m_aED_Term.GetText() );
    OUString aMapping( m_aED_Mapping.GetText() );
    sal_Int16 nConversionPropertyType = m_aLB_Property.GetSelectEntryPos()+1;

    DictionaryList& rActive  = getActiveDictionary();
    DictionaryList& rReverse = getReverseDictionary();

    DictionaryEntry* pE = rActive.getFirstSelectedEntry();
    if( pE->m_aTerm != aTerm )
        return 0;

    if( pE )
    {
        if( pE->m_aMapping != aMapping || pE->m_nConversionPropertyType != nConversionPropertyType )
        {
            if( m_aCB_Reverse.IsChecked() )
            {
                sal_uIntPtr nPos = rReverse.deleteEntries( pE->m_aMapping );
                nPos = rReverse.deleteEntries( aMapping );
                rReverse.addEntry( aMapping, aTerm, nConversionPropertyType, nPos );
            }

            sal_uIntPtr nPos = rActive.deleteEntries( aTerm );
            rActive.addEntry( aTerm, aMapping, nConversionPropertyType, nPos );
        }
    }

    updateButtons();
    return 0;
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, DeleteHdl)
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
                    if( m_aCB_Reverse.IsChecked() )
                        rReverse.deleteEntries( aMapping  );
                }
                break;
            }
        }
    }

    updateButtons();
    return 0;
}

short ChineseDictionaryDialog::Execute()
{
    sal_Int32 nTextConversionOptions = m_nTextConversionOptions;
    if(m_nTextConversionOptions | i18n::TextConversionOption::USE_CHARACTER_VARIANTS )
        nTextConversionOptions = nTextConversionOptions^i18n::TextConversionOption::USE_CHARACTER_VARIANTS ;

    m_aCT_DictionaryToSimplified.refillFromDictionary( nTextConversionOptions );
    m_aCT_DictionaryToTraditional.refillFromDictionary( m_nTextConversionOptions );

    short nRet = ModalDialog::Execute();

    if( nRet == RET_OK )
    {
        //save settings to configuration
        SvtLinguConfig  aLngCfg;
        Any aAny;
        aAny <<= sal_Bool( !!m_aCB_Reverse.IsChecked() );
        aLngCfg.SetProperty( OUString( UPN_IS_REVERSE_MAPPING ), aAny );

        m_aCT_DictionaryToSimplified.save();
        m_aCT_DictionaryToTraditional.save();
    }

    m_aCT_DictionaryToSimplified.deleteAll();
    m_aCT_DictionaryToTraditional.deleteAll();

    return nRet;
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, HeaderBarClick)
{
    if(m_pHeaderBar)
    {
        sal_uInt16 nId = m_pHeaderBar->GetCurItemId();
        HeaderBarItemBits nBits = m_pHeaderBar->GetItemBits(nId);
        if( nBits & HIB_CLICKABLE )
        {
            //set new arrow positions in headerbar
            m_pHeaderBar->SetItemBits( getActiveDictionary().getSortColumn()+1, HEADER_BAR_BITS );
            if( nBits & HIB_UPARROW )
                m_pHeaderBar->SetItemBits( nId, HEADER_BAR_BITS | HIB_DOWNARROW );
            else
                m_pHeaderBar->SetItemBits( nId, HEADER_BAR_BITS | HIB_UPARROW );

            //sort lists
            nBits = m_pHeaderBar->GetItemBits(nId);
            bool bSortAtoZ = nBits & HIB_UPARROW;
            getActiveDictionary().sortByColumn(nId-1,bSortAtoZ);
            getReverseDictionary().sortByColumn(nId-1,bSortAtoZ);
        }
    }
    return 0;
}

//.............................................................................
} //end namespace
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
