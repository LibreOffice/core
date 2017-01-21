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


#include <sal/macros.h>
#include <vcl/wrkwin.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/timer.hxx>
#include <svl/slstitm.hxx>
#include <svl/itemiter.hxx>
#include <svl/style.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/searchopt.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/itempool.hxx>
#include <svl/intitem.hxx>

#include <sfx2/app.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include "svx/srchdlg.hxx"

#include <svx/dialogs.hrc>
#include <svx/svxitems.hrc>

#include <svl/srchitem.hxx>
#include <AccessibleSvxFindReplaceDialog.hxx>
#include <svx/pageitem.hxx>
#include "srchctrl.hxx"
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <editeng/brushitem.hxx>
#include <tools/resary.hxx>
#include <svx/svxdlg.hxx>
#include <vcl/toolbox.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <cstdlib>
#include <memory>

using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;
using namespace com::sun::star;
using namespace comphelper;


#define REMEMBER_SIZE       10

enum class ModifyFlags {
    NONE         = 0x000000,
    Search       = 0x000001,
    Replace      = 0x000002,
    Word         = 0x000004,
    Exact        = 0x000008,
    Backwards    = 0x000010,
    Selection    = 0x000020,
    Regexp       = 0x000040,
    Layout       = 0x000080,
    Similarity   = 0x000100,
    Formulas     = 0x000200,
    Values       = 0x000400,
    CalcNotes    = 0x000800,
    Rows         = 0x001000,
    Columns      = 0x002000,
    AllTables    = 0x004000,
    Notes        = 0x008000,
    Wildcard     = 0x010000
};
namespace o3tl {
    template<> struct typed_flags<ModifyFlags> : is_typed_flags<ModifyFlags, 0x01ffff> {};
}

namespace
{
    bool GetCheckBoxValue(const CheckBox *pBox)
    {
        return pBox->IsEnabled() && pBox->IsChecked();
    }
}

struct SearchDlg_Impl
{
    bool        bSaveToModule  : 1,
                bFocusOnSearch : 1;
    std::unique_ptr<sal_uInt16[]> pRanges;
    Timer       aSelectionTimer;

    uno::Reference< frame::XDispatch > xCommand1Dispatch;
    uno::Reference< frame::XDispatch > xCommand2Dispatch;
    util::URL   aCommand1URL;
    util::URL   aCommand2URL;

    SearchDlg_Impl()
        : bSaveToModule(true)
        , bFocusOnSearch(true)
    {
        aCommand1URL.Complete = aCommand1URL.Main = "vnd.sun.search:SearchViaComponent1";
        aCommand1URL.Protocol = "vnd.sun.search:";
        aCommand1URL.Path = "SearchViaComponent1";
        aCommand2URL.Complete = aCommand2URL.Main = "vnd.sun.search:SearchViaComponent2";
        aCommand2URL.Protocol = "vnd.sun.search:";
        aCommand2URL.Path = "SearchViaComponent2";
    }
    ~SearchDlg_Impl() {}
};

void ListToStrArr_Impl( sal_uInt16 nId, std::vector<OUString>& rStrLst, ComboBox& rCBox )
{
    const SfxStringListItem* pSrchItem =
        static_cast<const SfxStringListItem*>(SfxGetpApp()->GetItem( nId ));

    if (pSrchItem)
    {
        std::vector<OUString> aLst = pSrchItem->GetList();

        for (const OUString & s : aLst)
        {
            rStrLst.push_back(s);
            rCBox.InsertEntry(s);
        }
    }
}

void StrArrToList_Impl( sal_uInt16 nId, const std::vector<OUString>& rStrLst )
{
    DBG_ASSERT( !rStrLst.empty(), "check in advance");
    SfxGetpApp()->PutItem( SfxStringListItem( nId, &rStrLst ) );
}

SearchAttrItemList::SearchAttrItemList( const SearchAttrItemList& rList ) :
    SrchAttrItemList(rList)
{
    for ( size_t i = 0; i < size(); ++i )
        if ( !IsInvalidItem( (*this)[i].pItem ) )
            (*this)[i].pItem = (*this)[i].pItem->Clone();
}


SearchAttrItemList::~SearchAttrItemList()
{
    Clear();
}


void SearchAttrItemList::Put( const SfxItemSet& rSet )
{
    if ( !rSet.Count() )
        return;

    SfxItemPool* pPool = rSet.GetPool();
    SfxItemIter aIter( rSet );
    SearchAttrItem aItem;
    const SfxPoolItem* pItem = aIter.GetCurItem();
    sal_uInt16 nWhich;

    while ( true )
    {
        // only test that it is available?
        if( IsInvalidItem( pItem ) )
        {
            nWhich = aIter.GetCurWhich();
            aItem.pItem = const_cast<SfxPoolItem*>(pItem);
        }
        else
        {
            nWhich = pItem->Which();
            aItem.pItem = pItem->Clone();
        }

        aItem.nSlot = pPool->GetSlotId( nWhich );
        Insert( aItem );

        if ( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
}


SfxItemSet& SearchAttrItemList::Get( SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();

    for ( size_t i = 0; i < size(); ++i )
        if ( IsInvalidItem( (*this)[i].pItem ) )
            rSet.InvalidateItem( pPool->GetWhich( (*this)[i].nSlot ) );
        else
            rSet.Put( *(*this)[i].pItem );
    return rSet;
}


void SearchAttrItemList::Clear()
{
    for ( size_t i = 0; i < size(); ++i )
        if ( !IsInvalidItem( (*this)[i].pItem ) )
            delete (*this)[i].pItem;
    SrchAttrItemList::clear();
}


// Deletes the pointer to the items
void SearchAttrItemList::Remove(size_t nPos)
{
    size_t nLen = 1;
    if ( nPos + nLen > size() )
        nLen = size() - nPos;

    for ( sal_uInt16 i = nPos; i < nPos + nLen; ++i )
        if ( !IsInvalidItem( (*this)[i].pItem ) )
            delete (*this)[i].pItem;

    SrchAttrItemList::erase( begin() + nPos, begin() + nPos + nLen );
}

SvxSearchDialog::SvxSearchDialog( vcl::Window* pParent, SfxChildWindow* pChildWin, SfxBindings& rBind )
    : SfxModelessDialog(&rBind, pChildWin, pParent, "FindReplaceDialog",
        "svx/ui/findreplacedialog.ui")
    , mpDocWin(nullptr)
    , mbSuccess(false)
    , rBindings(rBind)
    , bWriter(false)
    , bSearch(true)
    , bFormat(false)
    , bReplaceBackwards(false)
    , nOptions(SearchOptionFlags::ALL)
    , bSet(false)
    , bConstruct(true)
    , nModifyFlag(ModifyFlags::NONE)
    , pSearchList(nullptr)
    , pReplaceList(new SearchAttrItemList)
    , pSearchItem(nullptr)
    , pSearchController(nullptr)
    , pOptionsController(nullptr)
    , pFamilyController(nullptr)
    , pSearchSetController(nullptr)
    , pReplaceSetController(nullptr)
    , nTransliterationFlags(0x00000000)
{
    get(m_pSearchFrame, "searchframe");
    get(m_pSearchLB, "searchterm");
    get(m_pSearchTmplLB, "searchlist");
    m_pSearchTmplLB->SetStyle(m_pSearchTmplLB->GetStyle() | WB_SORT);
    get(m_pSearchBtn, "search");
    get(m_pBackSearchBtn, "backsearch");
    get(m_pSearchAllBtn, "searchall");
    get(m_pSearchAttrText, "searchdesc");
    m_pSearchAttrText->SetStyle(m_pSearchAttrText->GetStyle() | WB_PATHELLIPSIS);
    m_pSearchAttrText->Hide();
    get(m_pSearchLabel, "searchlabel");
    m_pSearchLabel->SetStyle(m_pSearchLabel->GetStyle() | WB_PATHELLIPSIS);
    m_pSearchLabel->Show();

    get(m_pReplaceFrame, "replaceframe");
    get(m_pReplaceLB, "replaceterm");
    get(m_pReplaceTmplLB, "replacelist");
    m_pReplaceTmplLB->SetStyle(m_pReplaceTmplLB->GetStyle() | WB_SORT);
    get(m_pReplaceBtn, "replace");
    get(m_pReplaceAllBtn, "replaceall");
    get(m_pReplaceAttrText, "replacedesc");
    m_pReplaceAttrText->SetStyle(m_pReplaceAttrText->GetStyle() | WB_PATHELLIPSIS);
    m_pReplaceAttrText->Hide();

    get(m_pComponentFrame, "componentframe");
    get(m_pSearchComponent1PB, "component1");
    get(m_pSearchComponent2PB, "component2");

    get(m_pMatchCaseCB, "matchcase");
    get(m_pSearchFormattedCB, "searchformatted");
    get(m_pWordBtn, "wholewords");
    aCalcStr = get<FixedText>("entirecells")->GetText();

    get(m_pCloseBtn, "close");

    get(m_pIgnoreDiacritics, "ignorediacritics");
    get(m_pIgnoreKashida, "ignorekashida");
    get(m_pSelectionBtn, "selection");
    get(m_pReplaceBackwardsCB, "replace_backwards");
    get(m_pRegExpBtn, "regexp");
    get(m_pWildcardBtn, "wildcard");
    get(m_pSimilarityBox, "similarity");
    get(m_pSimilarityBtn, "similaritybtn");
    get(m_pLayoutBtn, "layout");
    get(m_pNotesBtn, "notes");
    get(m_pJapMatchFullHalfWidthCB, "matchcharwidth");
    get(m_pJapOptionsCB, "soundslike");
    get(m_pJapOptionsBtn, "soundslikebtn");

    get(m_pAttributeBtn, "attributes");
    get(m_pFormatBtn, "format");
    get(m_pNoFormatBtn, "noformat");

    get(m_pCalcGrid, "calcgrid");
    get(m_pCalcSearchInFT, "searchinlabel");
    get(m_pCalcSearchInLB, "calcsearchin");
    get(m_pCalcSearchDirFT, "searchdir");
    get(m_pRowsBtn, "rows");
    get(m_pColumnsBtn, "cols");
    get(m_pAllSheetsCB, "allsheets");

    //align find and replace inputs without a predefined with for the labels
    get(m_pFindLabel, "label4"); //"Find:"
    get(m_pReplaceLabel, "label5"); //"Replace:"
    long nFindWidth = m_pFindLabel->get_preferred_size().Width();
    long nReplaceWidth = m_pReplaceLabel->get_preferred_size().Width();
    if ( nFindWidth > nReplaceWidth)
       m_pReplaceLabel->set_width_request(nFindWidth);
    else
       m_pFindLabel->set_width_request(nReplaceWidth);

    // m_pSimilarityBtn->set_height_request(m_pSimilarityBox->get_preferred_size().Height());
    // m_pJapOptionsBtn->set_height_request(m_pJapOptionsCB->get_preferred_size().Height());

    long nTermWidth = approximate_char_width() * 32;
    m_pSearchLB->set_width_request(nTermWidth);
    m_pSearchTmplLB->set_width_request(nTermWidth);
    m_pReplaceLB->set_width_request(nTermWidth);
    m_pReplaceTmplLB->set_width_request(nTermWidth);

    Construct_Impl();
}


SvxSearchDialog::~SvxSearchDialog()
{
    disposeOnce();
}

void SvxSearchDialog::dispose()
{
    Hide();

    rBindings.EnterRegistrations();
    DELETEZ( pSearchController );
    DELETEZ( pOptionsController );
    DELETEZ( pFamilyController );
    DELETEZ( pSearchSetController );
    DELETEZ( pReplaceSetController );
    rBindings.LeaveRegistrations();

    delete pSearchItem;
    pImpl.reset();
    delete pSearchList;
    delete pReplaceList;
    mpDocWin.clear();
    m_pSearchFrame.clear();
    m_pSearchLB.clear();
    m_pSearchTmplLB.clear();
    m_pSearchAttrText.clear();
    m_pSearchLabel.clear();
    m_pReplaceFrame.clear();
    m_pReplaceLB.clear();
    m_pReplaceTmplLB.clear();
    m_pReplaceAttrText.clear();
    m_pSearchBtn.clear();
    m_pBackSearchBtn.clear();
    m_pSearchAllBtn.clear();
    m_pReplaceBtn.clear();
    m_pReplaceAllBtn.clear();
    m_pComponentFrame.clear();
    m_pSearchComponent1PB.clear();
    m_pSearchComponent2PB.clear();
    m_pMatchCaseCB.clear();
    m_pSearchFormattedCB.clear();
    m_pWordBtn.clear();
    m_pCloseBtn.clear();
    m_pIgnoreDiacritics.clear();
    m_pIgnoreKashida.clear();
    m_pSelectionBtn.clear();
    m_pReplaceBackwardsCB.clear();
    m_pRegExpBtn.clear();
    m_pWildcardBtn.clear();
    m_pSimilarityBox.clear();
    m_pSimilarityBtn.clear();
    m_pLayoutBtn.clear();
    m_pNotesBtn.clear();
    m_pJapMatchFullHalfWidthCB.clear();
    m_pJapOptionsCB.clear();
    m_pJapOptionsBtn.clear();
    m_pAttributeBtn.clear();
    m_pFormatBtn.clear();
    m_pNoFormatBtn.clear();
    m_pCalcGrid.clear();
    m_pCalcSearchInFT.clear();
    m_pCalcSearchInLB.clear();
    m_pCalcSearchDirFT.clear();
    m_pRowsBtn.clear();
    m_pColumnsBtn.clear();
    m_pAllSheetsCB.clear();
    m_pFindLabel.clear();
    m_pReplaceLabel.clear();
    SfxModelessDialog::dispose();
}

void SvxSearchDialog::Construct_Impl()
{
    // temporary to avoid incompatibility
    pImpl.reset( new SearchDlg_Impl() );
    pImpl->aSelectionTimer.SetTimeout( 500 );
    pImpl->aSelectionTimer.SetTimeoutHdl(
        LINK( this, SvxSearchDialog, TimeoutHdl_Impl ) );
    EnableControls_Impl( SearchOptionFlags::NONE );

    // Store old Text from m_pWordBtn
    aCalcStr += "#";
    aCalcStr += m_pWordBtn->GetText();

    aLayoutStr = SVX_RESSTR( RID_SVXSTR_SEARCH_STYLES );
    aLayoutWriterStr = SVX_RESSTR( RID_SVXSTR_WRITER_STYLES );
    aLayoutCalcStr = SVX_RESSTR( RID_SVXSTR_CALC_STYLES );
    aStylesStr = m_pLayoutBtn->GetText();

    // Get stored search-strings from the application
    ListToStrArr_Impl(SID_SEARCHDLG_SEARCHSTRINGS,
                       aSearchStrings, *m_pSearchLB);
    ListToStrArr_Impl(SID_SEARCHDLG_REPLACESTRINGS,
                       aReplaceStrings, *m_pReplaceLB);

    InitControls_Impl();

    // Get attribute sets only once in constructor()
    const SfxPoolItem* ppArgs[] = { pSearchItem, nullptr };
    const SvxSetItem* pSrchSetItem =
        static_cast<const SvxSetItem*>( rBindings.GetDispatcher()->Execute( FID_SEARCH_SEARCHSET, SfxCallMode::SLOT, ppArgs ) );

    if ( pSrchSetItem )
        InitAttrList_Impl( &pSrchSetItem->GetItemSet(), nullptr );

    const SvxSetItem* pReplSetItem =
        static_cast<const SvxSetItem*>( rBindings.GetDispatcher()->Execute( FID_SEARCH_REPLACESET, SfxCallMode::SLOT, ppArgs ) );

    if ( pReplSetItem )
        InitAttrList_Impl( nullptr, &pReplSetItem->GetItemSet() );

    // Create controller and update at once
    rBindings.EnterRegistrations();
    pSearchController =
        new SvxSearchController( SID_SEARCH_ITEM, rBindings, *this );
    pOptionsController =
        new SvxSearchController( SID_SEARCH_OPTIONS, rBindings, *this );
    rBindings.LeaveRegistrations();
    rBindings.GetDispatcher()->Execute( FID_SEARCH_ON, SfxCallMode::SLOT, ppArgs );
    pImpl->aSelectionTimer.Start();


    SvtCJKOptions aCJKOptions;
    if(!aCJKOptions.IsJapaneseFindEnabled())
    {
        m_pJapOptionsCB->Check( false );
        m_pJapOptionsCB->Hide();
        m_pJapOptionsBtn->Hide();
    }
    if(!aCJKOptions.IsCJKFontEnabled())
    {
        m_pJapMatchFullHalfWidthCB->Hide();
    }
    SvtCTLOptions aCTLOptions;
    if(!aCTLOptions.IsCTLFontEnabled())
    {
        m_pIgnoreDiacritics->Check( false );
        m_pIgnoreDiacritics->Hide();
        m_pIgnoreKashida->Check( false );
        m_pIgnoreKashida->Hide();
    }
    //component extension - show component search buttons if the commands
    // vnd.sun.star::SearchViaComponent1 and 2 are supported
    const uno::Reference< frame::XFrame >xFrame = rBindings.GetActiveFrame();
    const uno::Reference< frame::XDispatchProvider > xDispatchProv(xFrame, uno::UNO_QUERY);
    OUString sTarget("_self");

    bool bSearchComponent1 = false;
    bool bSearchComponent2 = false;
    if(xDispatchProv.is() &&
            (pImpl->xCommand1Dispatch = xDispatchProv->queryDispatch(pImpl->aCommand1URL, sTarget, 0)).is())
    {
        bSearchComponent1 = true;
    }
    if(xDispatchProv.is() &&
            (pImpl->xCommand2Dispatch = xDispatchProv->queryDispatch(pImpl->aCommand2URL, sTarget, 0)).is())
    {
        bSearchComponent2 = true;
    }

    if( bSearchComponent1 || bSearchComponent2 )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider =
                    configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() );
            uno::Sequence< uno::Any > aArgs(1);
            OUString sPath( "/org.openoffice.Office.Common/SearchOptions/");
            aArgs[0] <<= sPath;

            uno::Reference< uno::XInterface > xIFace = xConfigurationProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationUpdateAccess",
                        aArgs);
            uno::Reference< container::XNameAccess> xDirectAccess(xIFace, uno::UNO_QUERY);
            if(xDirectAccess.is())
            {
                OUString sTemp;
                OUString sProperty( "ComponentSearchGroupLabel");
                uno::Any aRet = xDirectAccess->getByName(sProperty);
                aRet >>= sTemp;
                m_pComponentFrame->get_label_widget()->SetText(sTemp);
                aRet = xDirectAccess->getByName("ComponentSearchCommandLabel1");
                aRet >>= sTemp;
                m_pSearchComponent1PB->SetText( sTemp );
                aRet = xDirectAccess->getByName("ComponentSearchCommandLabel2");
                aRet >>= sTemp;
                m_pSearchComponent2PB->SetText( sTemp );
            }
        }
        catch(uno::Exception&){}

        if(!m_pSearchComponent1PB->GetText().isEmpty() && bSearchComponent1 )
        {
            m_pComponentFrame->Show();
            m_pSearchComponent1PB->Show();
        }
        if( !m_pSearchComponent2PB->GetText().isEmpty() )
        {
            m_pComponentFrame->Show();
            m_pSearchComponent2PB->Show();
        }
    }
}


bool SvxSearchDialog::Close()
{
    // remember strings speichern
    if (!aSearchStrings.empty())
        StrArrToList_Impl( SID_SEARCHDLG_SEARCHSTRINGS, aSearchStrings );

    if (!aReplaceStrings.empty())
        StrArrToList_Impl( SID_SEARCHDLG_REPLACESTRINGS, aReplaceStrings );

    // save settings to configuration
    SvtSearchOptions aOpt;
    aOpt.SetWholeWordsOnly          ( m_pWordBtn->IsChecked() );
    aOpt.SetBackwards               ( m_pReplaceBackwardsCB->IsChecked() );
    aOpt.SetUseRegularExpression    ( m_pRegExpBtn->IsChecked() );
    aOpt.SetUseWildcard             ( m_pWildcardBtn->IsChecked() );
    aOpt.SetSearchForStyles         ( m_pLayoutBtn->IsChecked() );
    aOpt.SetSimilaritySearch        ( m_pSimilarityBox->IsChecked() );
    aOpt.SetUseAsianOptions         ( m_pJapOptionsCB->IsChecked() );
    aOpt.SetNotes                   ( m_pNotesBtn->IsChecked() );
    aOpt.SetIgnoreDiacritics_CTL    ( m_pIgnoreDiacritics->IsChecked() );
    aOpt.SetIgnoreKashida_CTL       ( m_pIgnoreKashida->IsChecked() );
    aOpt.SetSearchFormatted         ( m_pSearchFormattedCB->IsChecked() );
    aOpt.Commit();

    const SfxPoolItem* ppArgs[] = { pSearchItem, nullptr };
    rBindings.GetDispatcher()->Execute( FID_SEARCH_OFF, SfxCallMode::SLOT, ppArgs );
    rBindings.Execute( SID_SEARCH_DLG );

    return true;
}


sal_Int32 SvxSearchDialog::GetTransliterationFlags() const
{
    if (!m_pMatchCaseCB->IsChecked())
        nTransliterationFlags |=  TransliterationModules_IGNORE_CASE;
    else
        nTransliterationFlags &= ~TransliterationModules_IGNORE_CASE;
    if ( !m_pJapMatchFullHalfWidthCB->IsChecked())
        nTransliterationFlags |=  TransliterationModules_IGNORE_WIDTH;
    else
        nTransliterationFlags &= ~TransliterationModules_IGNORE_WIDTH;
    return nTransliterationFlags;
}

void SvxSearchDialog::SetSaveToModule(bool b)
{
    pImpl->bSaveToModule = b;
}


void SvxSearchDialog::ApplyTransliterationFlags_Impl( sal_Int32 nSettings )
{
    nTransliterationFlags = nSettings;
    bool bVal = 0 != (nSettings & TransliterationModules_IGNORE_CASE);
    m_pMatchCaseCB->Check(!bVal );
    bVal = 0 != (nSettings & TransliterationModules_IGNORE_WIDTH);
    m_pJapMatchFullHalfWidthCB->Check( !bVal );
}


void SvxSearchDialog::Activate()
{
    // apply possible transliteration changes of the SvxSearchItem member
    DBG_ASSERT( pSearchItem, "SearchItem missing" );
    if (pSearchItem)
    {
        m_pMatchCaseCB->Check( pSearchItem->GetExact() );
        m_pJapMatchFullHalfWidthCB->Check( !pSearchItem->IsMatchFullHalfWidthForms() );
    }
}


void SvxSearchDialog::InitControls_Impl()
{
    // CaseSensitives AutoComplete
    m_pSearchLB->EnableAutocomplete( true, true );
    m_pSearchLB->Show();
    m_pReplaceLB->EnableAutocomplete( true, true );
    m_pReplaceLB->Show();

    m_pFormatBtn->Disable();
    m_pAttributeBtn->Disable();

    m_pSearchLB->SetModifyHdl( LINK( this, SvxSearchDialog, ModifyHdl_Impl ) );
    m_pReplaceLB->SetModifyHdl( LINK( this, SvxSearchDialog, ModifyHdl_Impl ) );

    Link<Control&,void> aLink = LINK( this, SvxSearchDialog, FocusHdl_Impl );
    m_pSearchLB->SetGetFocusHdl( aLink );
    m_pReplaceLB->SetGetFocusHdl( aLink );

    aLink = LINK( this, SvxSearchDialog, LoseFocusHdl_Impl );
    m_pSearchLB->SetLoseFocusHdl( aLink );
    m_pReplaceLB->SetLoseFocusHdl( aLink );

    m_pSearchTmplLB->SetLoseFocusHdl( aLink );
    m_pReplaceTmplLB->SetLoseFocusHdl( aLink );

    Link<Button*,void> aLink2 = LINK( this, SvxSearchDialog, CommandHdl_Impl );
    m_pSearchBtn->SetClickHdl( aLink2 );
    m_pBackSearchBtn->SetClickHdl( aLink2 );
    m_pSearchAllBtn->SetClickHdl( aLink2 );
    m_pReplaceBtn->SetClickHdl( aLink2 );
    m_pReplaceAllBtn->SetClickHdl( aLink2 );
    m_pCloseBtn->SetClickHdl( aLink2 );
    m_pSimilarityBtn->SetClickHdl( aLink2 );
    m_pJapOptionsBtn->SetClickHdl( aLink2 );
    m_pSearchComponent1PB->SetClickHdl( aLink2 );
    m_pSearchComponent2PB->SetClickHdl( aLink2 );

    aLink2 = LINK( this, SvxSearchDialog, FlagHdl_Impl );
    m_pReplaceBackwardsCB->SetClickHdl( aLink2 );
    m_pWordBtn->SetClickHdl( aLink2 );
    m_pSelectionBtn->SetClickHdl( aLink2 );
    m_pMatchCaseCB->SetClickHdl( aLink2 );
    m_pRegExpBtn->SetClickHdl( aLink2 );
    m_pWildcardBtn->SetClickHdl( aLink2 );
    m_pNotesBtn->SetClickHdl( aLink2 );
    m_pSimilarityBox->SetClickHdl( aLink2 );
    m_pJapOptionsCB->SetClickHdl( aLink2 );
    m_pJapMatchFullHalfWidthCB->SetClickHdl( aLink2 );
    m_pIgnoreDiacritics->SetClickHdl( aLink2 );
    m_pIgnoreKashida->SetClickHdl( aLink2 );
    m_pLayoutBtn->SetClickHdl( LINK( this, SvxSearchDialog, TemplateHdl_Impl ) );
    m_pFormatBtn->SetClickHdl( LINK( this, SvxSearchDialog, FormatHdl_Impl ) );
    m_pNoFormatBtn->SetClickHdl(
        LINK( this, SvxSearchDialog, NoFormatHdl_Impl ) );
    m_pAttributeBtn->SetClickHdl(
        LINK( this, SvxSearchDialog, AttributeHdl_Impl ) );
}

namespace
{
    SvtModuleOptions::EFactory getModule(SfxBindings& rBindings)
    {
        SvtModuleOptions::EFactory eFactory(SvtModuleOptions::EFactory::UNKNOWN_FACTORY);
        try
        {
            const uno::Reference< frame::XFrame > xFrame =
                rBindings.GetActiveFrame();
            uno::Reference< frame::XModuleManager2 > xModuleManager(
                frame::ModuleManager::create(::comphelper::getProcessComponentContext()));

            OUString aModuleIdentifier = xModuleManager->identify( xFrame );
            eFactory = SvtModuleOptions::ClassifyFactoryByServiceName(aModuleIdentifier);
        }
        catch (const uno::Exception&)
        {
        }
        return eFactory;
    }
}

void SvxSearchDialog::ShowOptionalControls_Impl()
{
    DBG_ASSERT( pSearchItem, "no search item" );

    SvtCJKOptions aCJKOptions;
    SvtCTLOptions aCTLOptions;
    SvtModuleOptions::EFactory eFactory = getModule(rBindings);
    bool bDrawApp = eFactory == SvtModuleOptions::EFactory::DRAW;
    bool bWriterApp =
        eFactory == SvtModuleOptions::EFactory::WRITER ||
        eFactory == SvtModuleOptions::EFactory::WRITERWEB ||
        eFactory == SvtModuleOptions::EFactory::WRITERGLOBAL;
    bool bCalcApp = eFactory == SvtModuleOptions::EFactory::CALC;

    m_pLayoutBtn->Show(!bDrawApp);
    m_pNotesBtn->Show(bWriterApp);
    m_pRegExpBtn->Show(!bDrawApp);
    m_pWildcardBtn->Show(bCalcApp); /* TODO:WILDCARD enable for other apps if hey handle it */
    m_pReplaceBackwardsCB->Show();
    m_pSimilarityBox->Show();
    m_pSimilarityBtn->Show();
    m_pSelectionBtn->Show();
    m_pIgnoreDiacritics->Show(aCTLOptions.IsCTLFontEnabled());
    m_pIgnoreKashida->Show(aCTLOptions.IsCTLFontEnabled());
    m_pJapMatchFullHalfWidthCB->Show(aCJKOptions.IsCJKFontEnabled());
    m_pJapOptionsCB->Show(aCJKOptions.IsJapaneseFindEnabled());
    m_pJapOptionsBtn->Show(aCJKOptions.IsJapaneseFindEnabled());

    if (bWriter)
    {
        m_pAttributeBtn->Show();
        m_pFormatBtn->Show();
        m_pNoFormatBtn->Show();
    }

    if (bCalcApp)
    {
        m_pCalcSearchInFT->Show();
        m_pCalcSearchInLB->Show();
        m_pCalcSearchDirFT->Show();
        m_pRowsBtn->Show();
        m_pColumnsBtn->Show();
        m_pAllSheetsCB->Show();
        m_pSearchFormattedCB->Show();
    }
}


namespace {

class ToggleSaveToModule
{
public:
    ToggleSaveToModule(SvxSearchDialog& rDialog, bool bValue) :
        mrDialog(rDialog), mbValue(bValue)
    {
        mrDialog.SetSaveToModule(mbValue);
    }

    ~ToggleSaveToModule()
    {
        mrDialog.SetSaveToModule(!mbValue);
    }
private:
    SvxSearchDialog& mrDialog;
    bool mbValue;
};

}

void SvxSearchDialog::Init_Impl( bool bSearchPattern )
{
    DBG_ASSERT( pSearchItem, "SearchItem == 0" );

    // We don't want to save any intermediate state to the module while the
    // dialog is being initialized.
    ToggleSaveToModule aNoModuleSave(*this, false);
    SvtSearchOptions aOpt;

    bWriter = ( pSearchItem->GetAppFlag() == SvxSearchApp::WRITER );

    if ( !( nModifyFlag & ModifyFlags::Word ) )
         m_pWordBtn->Check( pSearchItem->GetWordOnly() );
    if ( !( nModifyFlag & ModifyFlags::Exact ) )
        m_pMatchCaseCB->Check( pSearchItem->GetExact() );
    if ( !( nModifyFlag & ModifyFlags::Backwards ) )
        m_pReplaceBackwardsCB->Check( bReplaceBackwards ); //adjustment to replace backwards
    if ( !( nModifyFlag & ModifyFlags::Notes ) )
        m_pNotesBtn->Check( pSearchItem->GetNotes() );
    if ( !( nModifyFlag & ModifyFlags::Selection ) )
        m_pSelectionBtn->Check( pSearchItem->GetSelection() );
    if ( !( nModifyFlag & ModifyFlags::Regexp ) )
        m_pRegExpBtn->Check( pSearchItem->GetRegExp() );
    if ( !( nModifyFlag & ModifyFlags::Wildcard ) )
        m_pWildcardBtn->Check( pSearchItem->GetWildcard() );
    if ( !( nModifyFlag & ModifyFlags::Layout ) )
        m_pLayoutBtn->Check( pSearchItem->GetPattern() );
    if (m_pNotesBtn->IsChecked())
        m_pLayoutBtn->Disable();
    m_pSimilarityBox->Check( pSearchItem->IsLevenshtein() );
    if( m_pJapOptionsCB->IsVisible() )
        m_pJapOptionsCB->Check( pSearchItem->IsUseAsianOptions() );
    if (m_pIgnoreDiacritics->IsVisible())
        m_pIgnoreDiacritics->Check( aOpt.IsIgnoreDiacritics_CTL() );
    if (m_pIgnoreKashida->IsVisible())
        m_pIgnoreKashida->Check( aOpt.IsIgnoreKashida_CTL() );
    ApplyTransliterationFlags_Impl( pSearchItem->GetTransliterationFlags() );

    ShowOptionalControls_Impl();

    if ( pSearchItem->GetAppFlag() == SvxSearchApp::CALC )
    {
        m_pCalcGrid->Show();
        m_pSearchFormattedCB->Check( aOpt.IsSearchFormatted() );
        Link<Button*,void> aLink = LINK( this, SvxSearchDialog, FlagHdl_Impl );
        m_pCalcSearchInLB->SetSelectHdl( LINK( this, SvxSearchDialog, LBSelectHdl_Impl ) );
        m_pRowsBtn->SetClickHdl( aLink );
        m_pColumnsBtn->SetClickHdl( aLink );
        m_pAllSheetsCB->SetClickHdl( aLink );
        m_pSearchFormattedCB->SetClickHdl( aLink );

        ModifyFlags nModifyFlagCheck;
        switch ( pSearchItem->GetCellType() )
        {
            case SvxSearchCellType::FORMULA:
                nModifyFlagCheck = ModifyFlags::Formulas;
                break;

            case SvxSearchCellType::VALUE:
                nModifyFlagCheck = ModifyFlags::Values;
                break;

            case SvxSearchCellType::NOTE:
                nModifyFlagCheck = ModifyFlags::CalcNotes;
                break;

            default:
                std::abort(); // cannot happen
        }
        if ( !(nModifyFlag & nModifyFlagCheck) )
            m_pCalcSearchInLB->SelectEntryPos( static_cast<sal_Int32>(pSearchItem->GetCellType()) );

        m_pWordBtn->SetText( aCalcStr.getToken( 0, '#' ) );

        if ( pSearchItem->GetRowDirection() &&
             !( nModifyFlag & ModifyFlags::Rows ) )
            m_pRowsBtn->Check();
        else if ( !pSearchItem->GetRowDirection() &&
                  !( nModifyFlag & ModifyFlags::Columns ) )
            m_pColumnsBtn->Check();

        if ( !( nModifyFlag & ModifyFlags::AllTables ) )
            m_pAllSheetsCB->Check( pSearchItem->IsAllTables() );

        // only look for formatting in Writer
        m_pFormatBtn->Hide();
        m_pNoFormatBtn->Hide();
        m_pAttributeBtn->Hide();
    }
    else
    {
        m_pSearchFormattedCB->Hide();
        m_pWordBtn->SetText( aCalcStr.getToken( 1, '#' ) );

        if ( pSearchItem->GetAppFlag() == SvxSearchApp::DRAW )
        {
            m_pSearchAllBtn->Hide();

            m_pRegExpBtn->Hide();
            m_pWildcardBtn->Hide();
            m_pLayoutBtn->Hide();

            // only look for formatting in Writer
            m_pFormatBtn->Hide();
            m_pNoFormatBtn->Hide();
            m_pAttributeBtn->Hide();
        }
        else
        {
            m_pWildcardBtn->Hide(); /* TODO:WILDCARD do not hide for other apps if they handle it */

            if ( !pSearchList )
            {
                // Get attribute sets, if it not has been done already
                const SfxPoolItem* ppArgs[] = { pSearchItem, nullptr };
                const SvxSetItem* pSrchSetItem =
                    static_cast<const SvxSetItem*>(rBindings.GetDispatcher()->Execute( FID_SEARCH_SEARCHSET, SfxCallMode::SLOT, ppArgs ));

                if ( pSrchSetItem )
                    InitAttrList_Impl( &pSrchSetItem->GetItemSet(), nullptr );

                const SvxSetItem* pReplSetItem =
                    static_cast<const SvxSetItem*>( rBindings.GetDispatcher()->Execute( FID_SEARCH_REPLACESET, SfxCallMode::SLOT, ppArgs ) );

                if ( pReplSetItem )
                    InitAttrList_Impl( nullptr, &pReplSetItem->GetItemSet() );
            }
        }
    }

    // similarity search?
    if ( !( nModifyFlag & ModifyFlags::Similarity ) )
        m_pSimilarityBox->Check( pSearchItem->IsLevenshtein() );
    bSet = true;

    FlagHdl_Impl(m_pSimilarityBox);
    FlagHdl_Impl(m_pJapOptionsCB);

    bool bDisableSearch = false;
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if ( pViewShell )
    {
        bool bText = !bSearchPattern;

        if ( pViewShell->HasSelection( bText ) )
            EnableControl_Impl(m_pSelectionBtn);
        else
        {
            m_pSelectionBtn->Check( false );
            m_pSelectionBtn->Disable();
        }
    }

    // Pattern Search and there were no AttrSets given
    if ( bSearchPattern )
    {
        SfxObjectShell* pShell = SfxObjectShell::Current();

        if ( pShell && pShell->GetStyleSheetPool() )
        {
            // Templates designed
            m_pSearchTmplLB->Clear();
            m_pReplaceTmplLB->Clear();
            SfxStyleSheetBasePool* pStylePool = pShell->GetStyleSheetPool();
            pStylePool->SetSearchMask( pSearchItem->GetFamily() );
            SfxStyleSheetBase* pBase = pStylePool->First();

            while ( pBase )
            {
                if ( pBase->IsUsed() )
                    m_pSearchTmplLB->InsertEntry( pBase->GetName() );
                m_pReplaceTmplLB->InsertEntry( pBase->GetName() );
                pBase = pStylePool->Next();
            }
            m_pSearchTmplLB->SelectEntry( pSearchItem->GetSearchString() );
            m_pReplaceTmplLB->SelectEntry( pSearchItem->GetReplaceString() );

        }
        m_pSearchTmplLB->Show();

        if ( bConstruct )
            // Grab focus only after creating
            m_pSearchTmplLB->GrabFocus();
        m_pReplaceTmplLB->Show();
        m_pSearchLB->Hide();
        m_pReplaceLB->Hide();

        m_pWordBtn->Disable();
        m_pRegExpBtn->Disable();
        m_pWildcardBtn->Disable();
        m_pMatchCaseCB->Disable();

        bDisableSearch = !m_pSearchTmplLB->GetEntryCount();
    }
    else
    {
        bool bSetSearch = !( nModifyFlag & ModifyFlags::Search );
        bool bSetReplace = !( nModifyFlag & ModifyFlags::Replace );

        if ( !(pSearchItem->GetSearchString().isEmpty()) && bSetSearch )
            m_pSearchLB->SetText( pSearchItem->GetSearchString() );
        else if (!aSearchStrings.empty())
        {
            bool bAttributes =
                ( ( pSearchList && pSearchList->Count() ) ||
                  ( pReplaceList && pReplaceList->Count() ) );

            if ( bSetSearch && !bAttributes )
                m_pSearchLB->SetText(aSearchStrings[0]);

            OUString aReplaceTxt = pSearchItem->GetReplaceString();

            if (!aReplaceStrings.empty())
                aReplaceTxt = aReplaceStrings[0];

            if ( bSetReplace && !bAttributes )
                m_pReplaceLB->SetText( aReplaceTxt );
        }
        m_pSearchLB->Show();

        if ( bConstruct )
            // Grab focus only after creating
            m_pSearchLB->GrabFocus();
        m_pReplaceLB->Show();
        m_pSearchTmplLB->Hide();
        m_pReplaceTmplLB->Hide();

        EnableControl_Impl(m_pRegExpBtn);
        EnableControl_Impl(m_pWildcardBtn);
        EnableControl_Impl(m_pMatchCaseCB);

        if ( m_pRegExpBtn->IsChecked() )
            m_pWordBtn->Disable();
        else
            EnableControl_Impl(m_pWordBtn);

        bDisableSearch = m_pSearchLB->GetText().isEmpty() &&
            m_pSearchAttrText->GetText().isEmpty();
    }
    FocusHdl_Impl(*m_pSearchLB);

    if ( bDisableSearch )
    {
        m_pSearchBtn->Disable();
        m_pBackSearchBtn->Disable();
        m_pSearchAllBtn->Disable();
        m_pReplaceBtn->Disable();
        m_pReplaceAllBtn->Disable();
        m_pComponentFrame->Enable(false);
    }
    else
    {
        EnableControl_Impl(m_pSearchBtn);
        EnableControl_Impl(m_pBackSearchBtn);
        EnableControl_Impl(m_pReplaceBtn);
        if (!bWriter || !m_pNotesBtn->IsChecked())
        {
            EnableControl_Impl(m_pSearchAllBtn);
            EnableControl_Impl(m_pReplaceAllBtn);
        }
        if (bWriter && pSearchItem->GetNotes())
        {
            m_pSearchAllBtn->Disable();
            m_pReplaceAllBtn->Disable();
        }
    }

    if (!m_pSearchAttrText->GetText().isEmpty())
        EnableControl_Impl(m_pNoFormatBtn);
    else
        m_pNoFormatBtn->Disable();

    if ( !pSearchList )
    {
        m_pAttributeBtn->Disable();
        m_pFormatBtn->Disable();
    }

    if ( m_pLayoutBtn->IsChecked() )
    {
        pImpl->bSaveToModule = false;
        TemplateHdl_Impl(m_pLayoutBtn);
        pImpl->bSaveToModule = true;
    }
}


void SvxSearchDialog::InitAttrList_Impl( const SfxItemSet* pSSet,
                                         const SfxItemSet* pRSet )
{
    if ( !pSSet && !pRSet )
        return;

    if ( !pImpl->pRanges && pSSet )
    {
        sal_sSize nCnt = 0;
        const sal_uInt16* pPtr = pSSet->GetRanges();
        const sal_uInt16* pTmp = pPtr;

        while( *pPtr )
        {
            nCnt += ( *(pPtr+1) - *pPtr ) + 1;
            pPtr += 2;
        }
        nCnt = pPtr - pTmp + 1;
        pImpl->pRanges.reset( new sal_uInt16[nCnt] );
        memcpy( pImpl->pRanges.get(), pTmp, sizeof(sal_uInt16) * nCnt );
    }

    // See to it that are the texts of the attributes are correct
    OUString aDesc;

    if ( pSSet )
    {
        delete pSearchList;
        pSearchList = new SearchAttrItemList;

        if ( pSSet->Count() )
        {
            pSearchList->Put( *pSSet );

            m_pSearchAttrText->SetText( BuildAttrText_Impl( aDesc, true ) );

            if ( !aDesc.isEmpty() )
            {
                m_pSearchAttrText->Show();
                bFormat |= true;
            }
        }
    }

    if ( pRSet )
    {
        delete pReplaceList;
        pReplaceList = new SearchAttrItemList;

        if ( pRSet->Count() )
        {
            pReplaceList->Put( *pRSet );

            m_pReplaceAttrText->SetText( BuildAttrText_Impl( aDesc, false ) );

            if ( !aDesc.isEmpty() )
            {
                m_pReplaceAttrText->Show();
                bFormat |= true;
            }
        }
    }
}


IMPL_LINK( SvxSearchDialog, LBSelectHdl_Impl, ListBox&, rCtrl, void )
{
    ClickHdl_Impl(&rCtrl);
}

IMPL_LINK( SvxSearchDialog, FlagHdl_Impl, Button *, pCtrl, void )
{
    ClickHdl_Impl(pCtrl);
}

void SvxSearchDialog::ClickHdl_Impl(void* pCtrl)
{
    if ( pCtrl && !bSet )
        SetModifyFlag_Impl( static_cast<Control*>(pCtrl) );
    else
        bSet = false;

    if (pCtrl == m_pSimilarityBox)
    {
        bool bIsChecked = m_pSimilarityBox->IsChecked();

        if ( bIsChecked )
        {
            m_pSimilarityBtn->Enable();
            m_pRegExpBtn->Check( false );
            m_pRegExpBtn->Disable();
            m_pWildcardBtn->Check( false );
            m_pWildcardBtn->Disable();
            EnableControl_Impl(m_pWordBtn);

            if ( m_pLayoutBtn->IsChecked() )
            {
                EnableControl_Impl(m_pMatchCaseCB);
                m_pLayoutBtn->Check( false );
            }
            m_pRegExpBtn->Disable();
            m_pWildcardBtn->Disable();
            m_pLayoutBtn->Disable();
            m_pFormatBtn->Disable();
            m_pNoFormatBtn->Disable();
            m_pAttributeBtn->Disable();
        }
        else
        {
            EnableControl_Impl(m_pRegExpBtn);
            EnableControl_Impl(m_pWildcardBtn);
            if (!m_pNotesBtn->IsChecked())
                EnableControl_Impl(m_pLayoutBtn);
            EnableControl_Impl(m_pFormatBtn);
            EnableControl_Impl(m_pAttributeBtn);
            m_pSimilarityBtn->Disable();
        }
        pSearchItem->SetLevenshtein( bIsChecked );
    }
    else if (pCtrl == m_pNotesBtn)
    {
        if (m_pNotesBtn->IsChecked())
        {
            m_pLayoutBtn->Disable();
            m_pSearchAllBtn->Disable();
            m_pReplaceAllBtn->Disable();
        }
        else
        {
            EnableControl_Impl(m_pLayoutBtn);
            ModifyHdl_Impl(*m_pSearchLB);
        }
    }
    else
    {
        if ( m_pLayoutBtn->IsChecked() && !bFormat )
        {
            m_pWordBtn->Check( false );
            m_pWordBtn->Disable();
            m_pRegExpBtn->Check( false );
            m_pRegExpBtn->Disable();
            m_pWildcardBtn->Check( false );
            m_pWildcardBtn->Disable();
            m_pMatchCaseCB->Check( false );
            m_pMatchCaseCB->Disable();
            m_pNotesBtn->Disable();

            if ( m_pSearchTmplLB->GetEntryCount() )
            {
                EnableControl_Impl(m_pSearchBtn);
                EnableControl_Impl(m_pBackSearchBtn);
                EnableControl_Impl(m_pSearchAllBtn);
                EnableControl_Impl(m_pReplaceBtn);
                EnableControl_Impl(m_pReplaceAllBtn);
            }
        }
        else
        {
            EnableControl_Impl(m_pRegExpBtn);
            EnableControl_Impl(m_pWildcardBtn);
            EnableControl_Impl(m_pMatchCaseCB);
            EnableControl_Impl(m_pNotesBtn);

            if ( m_pRegExpBtn->IsChecked() )
            {
                m_pWordBtn->Check( false );
                m_pWordBtn->Disable();
                m_pWildcardBtn->Check( false );
                m_pWildcardBtn->Disable();
                m_pSimilarityBox->Check( false );
                m_pSimilarityBox->Disable();
                m_pSimilarityBtn->Disable();
            }
            else if ( m_pWildcardBtn->IsChecked() )
            {
                m_pRegExpBtn->Check( false );
                m_pRegExpBtn->Disable();
                m_pSimilarityBox->Check( false );
                m_pSimilarityBox->Disable();
                m_pSimilarityBtn->Disable();
            }
            else
            {
                EnableControl_Impl(m_pWordBtn);
                EnableControl_Impl(m_pSimilarityBox);
            }

            // Search-string in place? then enable Buttons
            bSet = true;
            ModifyHdl_Impl(*m_pSearchLB);
        }
    }

    if (pCtrl == m_pAllSheetsCB)
    {
        bSet = true;
        ModifyHdl_Impl(*m_pSearchLB);
    }

    if (pCtrl == m_pJapOptionsCB)
    {
        bool bEnableJapOpt = m_pJapOptionsCB->IsChecked();
        m_pMatchCaseCB->Enable(!bEnableJapOpt );
        m_pJapMatchFullHalfWidthCB->Enable(!bEnableJapOpt );
        m_pJapOptionsBtn->Enable( bEnableJapOpt );
    }

    if ( pImpl->bSaveToModule )
        SaveToModule_Impl();
}


IMPL_LINK( SvxSearchDialog, CommandHdl_Impl, Button *, pBtn, void )
{
    bool bInclusive = ( m_pLayoutBtn->GetText() == aLayoutStr );

    if ( ( pBtn == m_pSearchBtn )   ||
        (pBtn == m_pBackSearchBtn)  ||
         ( pBtn == m_pSearchAllBtn )||
         ( pBtn == m_pReplaceBtn )  ||
         ( pBtn == m_pReplaceAllBtn ) )
    {
        if ( m_pLayoutBtn->IsChecked() && !bInclusive )
        {
            pSearchItem->SetSearchString ( m_pSearchTmplLB->GetSelectEntry() );
            pSearchItem->SetReplaceString( m_pReplaceTmplLB->GetSelectEntry() );
        }
        else
        {
            pSearchItem->SetSearchString ( m_pSearchLB->GetText() );
            pSearchItem->SetReplaceString( m_pReplaceLB->GetText() );

            if ( pBtn == m_pReplaceBtn )
                Remember_Impl( m_pReplaceLB->GetText(), false );
            else
            {
                Remember_Impl( m_pSearchLB->GetText(), true );

                if ( pBtn == m_pReplaceAllBtn )
                    Remember_Impl( m_pReplaceLB->GetText(), false );
            }
        }

        pSearchItem->SetRegExp( false );
        pSearchItem->SetWildcard( false );
        pSearchItem->SetLevenshtein( false );
        if (GetCheckBoxValue(m_pRegExpBtn))
            pSearchItem->SetRegExp( true );
        else if (GetCheckBoxValue(m_pWildcardBtn))
            pSearchItem->SetWildcard( true );
        else if (GetCheckBoxValue(m_pSimilarityBox))
            pSearchItem->SetLevenshtein( true );

        pSearchItem->SetWordOnly(GetCheckBoxValue(m_pWordBtn));

        bool bSetBackwards = false;
        if( pBtn == m_pBackSearchBtn)
        {
            bSetBackwards = true;
        }
        else if( pBtn == m_pReplaceBtn)
        {
            bSetBackwards = GetCheckBoxValue(m_pReplaceBackwardsCB);
            bReplaceBackwards = GetCheckBoxValue(m_pReplaceBackwardsCB);
        }

        pSearchItem->SetBackward(bSetBackwards);

        pSearchItem->SetNotes(GetCheckBoxValue(m_pNotesBtn));
        pSearchItem->SetPattern(GetCheckBoxValue(m_pLayoutBtn));
        pSearchItem->SetSelection(GetCheckBoxValue(m_pSelectionBtn));
        pSearchItem->SetUseAsianOptions(GetCheckBoxValue(m_pJapOptionsCB));
        sal_Int32 nFlags = GetTransliterationFlags();
        if( !pSearchItem->IsUseAsianOptions())
            nFlags &= (TransliterationModules_IGNORE_CASE |
                       TransliterationModules_IGNORE_WIDTH );
        if (GetCheckBoxValue(m_pIgnoreDiacritics))
            nFlags |= TransliterationModulesExtra::IGNORE_DIACRITICS_CTL;
        if (GetCheckBoxValue(m_pIgnoreKashida))
            nFlags |= TransliterationModulesExtra::IGNORE_KASHIDA_CTL;
        pSearchItem->SetTransliterationFlags( nFlags );

        if ( !bWriter )
        {
            if ( m_pCalcSearchInLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
                pSearchItem->SetCellType( static_cast<SvxSearchCellType>(m_pCalcSearchInLB->GetSelectEntryPos()) );

            pSearchItem->SetRowDirection( m_pRowsBtn->IsChecked() );
            pSearchItem->SetAllTables( m_pAllSheetsCB->IsChecked() );
            pSearchItem->SetSearchFormatted( m_pSearchFormattedCB->IsChecked() );
        }

        if ((pBtn == m_pSearchBtn) ||  (pBtn == m_pBackSearchBtn))
            pSearchItem->SetCommand( SvxSearchCmd::FIND );
        else if ( pBtn == m_pSearchAllBtn )
            pSearchItem->SetCommand( SvxSearchCmd::FIND_ALL );
        else if ( pBtn == m_pReplaceBtn )
            pSearchItem->SetCommand( SvxSearchCmd::REPLACE );
        else if ( pBtn == m_pReplaceAllBtn )
            pSearchItem->SetCommand( SvxSearchCmd::REPLACE_ALL );

        // when looking for templates, delete format lists
        if ( !bFormat && pSearchItem->GetPattern() )
        {
            if ( pSearchList )
                pSearchList->Clear();

            if ( pReplaceList )
                pReplaceList->Clear();
        }
        nModifyFlag = ModifyFlags::NONE;
        const SfxPoolItem* ppArgs[] = { pSearchItem, nullptr };
        rBindings.ExecuteSynchron( FID_SEARCH_NOW, ppArgs );
    }
    else if ( pBtn == m_pCloseBtn )
    {
        if ( !m_pLayoutBtn->IsChecked() || bInclusive )
        {
            OUString aStr( m_pSearchLB->GetText() );

            if ( !aStr.isEmpty() )
                Remember_Impl( aStr, true );
            aStr = m_pReplaceLB->GetText();

            if ( !aStr.isEmpty() )
                Remember_Impl( aStr, false );
        }
        SaveToModule_Impl();
        Close();
    }
    else if (pBtn == m_pSimilarityBtn)
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            ScopedVclPtr<AbstractSvxSearchSimilarityDialog> pDlg(pFact->CreateSvxSearchSimilarityDialog( this,
                                                                        pSearchItem->IsLEVRelaxed(),
                                                                        pSearchItem->GetLEVOther(),
                                                                        pSearchItem->GetLEVShorter(),
                                                                        pSearchItem->GetLEVLonger() ));
            DBG_ASSERT(pDlg, "Dialog creation failed!");
            if ( pDlg && pDlg->Execute() == RET_OK )
            {
                pSearchItem->SetLEVRelaxed( pDlg->IsRelaxed() );
                pSearchItem->SetLEVOther( pDlg->GetOther() );
                pSearchItem->SetLEVShorter( pDlg->GetShorter() );
                pSearchItem->SetLEVLonger( pDlg->GetLonger() );
                SaveToModule_Impl();
            }
        }
    }
    else if (pBtn == m_pJapOptionsBtn)
    {
        SfxItemSet aSet( SfxGetpApp()->GetPool() );
        pSearchItem->SetTransliterationFlags( GetTransliterationFlags() );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            ScopedVclPtr<AbstractSvxJSearchOptionsDialog> aDlg(pFact->CreateSvxJSearchOptionsDialog( this, aSet,
                    pSearchItem->GetTransliterationFlags() ));
            DBG_ASSERT(aDlg, "Dialog creation failed!");
            int nRet = aDlg->Execute();
            if (RET_OK == nRet) //! true only if FillItemSet of SvxJSearchOptionsPage returns true
            {
                sal_Int32 nFlags = aDlg->GetTransliterationFlags();
                pSearchItem->SetTransliterationFlags( nFlags );
                ApplyTransliterationFlags_Impl( nFlags );
            }
        }
    }
    else if (pBtn == m_pSearchComponent1PB || pBtn == m_pSearchComponent2PB)
    {
        uno::Sequence < beans::PropertyValue > aArgs(2);
        beans::PropertyValue* pArgs = aArgs.getArray();
        pArgs[0].Name = "SearchString";
        pArgs[0].Value <<= OUString(m_pSearchLB->GetText());
        pArgs[1].Name = "ParentWindow";
        pArgs[1].Value <<= VCLUnoHelper::GetInterface( this );
        if(pBtn == m_pSearchComponent1PB)
        {
            if ( pImpl->xCommand1Dispatch.is() )
                pImpl->xCommand1Dispatch->dispatch(pImpl->aCommand1URL, aArgs);
        }
        else
        {
            if ( pImpl->xCommand2Dispatch.is() )
                pImpl->xCommand2Dispatch->dispatch(pImpl->aCommand2URL, aArgs);
        }
    }
}


IMPL_LINK( SvxSearchDialog, ModifyHdl_Impl, Edit&, rEd, void )
{
    if ( !bSet )
        SetModifyFlag_Impl( &rEd );
    else
        bSet = false;

    // Calc allows searching for empty cells.
    bool bAllowEmptySearch = (pSearchItem->GetAppFlag() == SvxSearchApp::CALC);

    if ( &rEd == m_pSearchLB || &rEd == m_pReplaceLB )
    {
        sal_Int32 nSrchTxtLen = m_pSearchLB->GetText().getLength();
        sal_Int32 nReplTxtLen = 0;
        if (bAllowEmptySearch)
            nReplTxtLen = m_pReplaceLB->GetText().getLength();
        sal_Int32 nAttrTxtLen = m_pSearchAttrText->GetText().getLength();

        if (nSrchTxtLen || nReplTxtLen || nAttrTxtLen)
        {
            EnableControl_Impl(m_pSearchBtn);
            EnableControl_Impl(m_pBackSearchBtn);
            EnableControl_Impl(m_pReplaceBtn);
            if (!bWriter || !m_pNotesBtn->IsChecked())
            {
                EnableControl_Impl(m_pSearchAllBtn);
                EnableControl_Impl(m_pReplaceAllBtn);
            }
        }
        else
        {
            m_pComponentFrame->Enable(false);
            m_pSearchBtn->Disable();
            m_pBackSearchBtn->Disable();
            m_pSearchAllBtn->Disable();
            m_pReplaceBtn->Disable();
            m_pReplaceAllBtn->Disable();
        }
    }
}


IMPL_LINK_NOARG(SvxSearchDialog, TemplateHdl_Impl, Button*, void)
{
    if ( pImpl->bSaveToModule )
        SaveToModule_Impl();

    if ( bFormat )
        return;
    OUString sDesc;

    if ( m_pLayoutBtn->IsChecked() )
    {
        if ( !pFamilyController )
        {
            sal_uInt16 nId = 0;

            // Enable templates controller
            switch ( pSearchItem->GetFamily() )
            {
                case SfxStyleFamily::Char:
                    nId = SID_STYLE_FAMILY1; break;

                case SfxStyleFamily::Para:
                    nId = SID_STYLE_FAMILY2; break;

                case SfxStyleFamily::Frame:
                    nId = SID_STYLE_FAMILY3; break;

                case SfxStyleFamily::Page:
                    nId = SID_STYLE_FAMILY4; break;

                case SfxStyleFamily::All:
                    break;

                default:
                    OSL_FAIL( "StyleSheetFamily was changed?" );
            }

            rBindings.EnterRegistrations();
            pFamilyController =
                new SvxSearchController( nId, rBindings, *this );
            rBindings.LeaveRegistrations();
            m_pSearchTmplLB->Clear();
            m_pReplaceTmplLB->Clear();

            m_pSearchTmplLB->Show();
            m_pReplaceTmplLB->Show();
            m_pSearchLB->Hide();
            m_pReplaceLB->Hide();

            m_pSearchAttrText->SetText( sDesc );
            m_pReplaceAttrText->SetText( sDesc );

            if(!sDesc.isEmpty())
            {
                m_pSearchAttrText->Show();
                m_pReplaceAttrText->Show();
            }
        }
        m_pFormatBtn->Disable();
        m_pNoFormatBtn->Disable();
        m_pAttributeBtn->Disable();
        m_pSimilarityBox->Disable();
        m_pSimilarityBtn->Disable();
    }
    else
    {
        // Disable templates controller
        rBindings.EnterRegistrations();
        DELETEZ( pFamilyController );
        rBindings.LeaveRegistrations();

        m_pSearchLB->Show();
        m_pReplaceLB->Show();
        m_pSearchTmplLB->Hide();
        m_pReplaceTmplLB->Hide();

        m_pSearchAttrText->SetText( BuildAttrText_Impl( sDesc, true ) );
        m_pReplaceAttrText->SetText( BuildAttrText_Impl( sDesc, false ) );

        if(!sDesc.isEmpty())
        {
            m_pSearchAttrText->Show();
            m_pReplaceAttrText->Show();
        }

        EnableControl_Impl(m_pFormatBtn);
        EnableControl_Impl(m_pAttributeBtn);
        EnableControl_Impl(m_pSimilarityBox);

        FocusHdl_Impl( bSearch ? *m_pSearchLB : *m_pReplaceLB );
    }
    bSet = true;
    pImpl->bSaveToModule = false;
    FlagHdl_Impl(m_pLayoutBtn);
    pImpl->bSaveToModule = true;
}


void SvxSearchDialog::Remember_Impl( const OUString &rStr, bool _bSearch )
{
    if ( rStr.isEmpty() )
        return;

    std::vector<OUString>* pArr = _bSearch ? &aSearchStrings : &aReplaceStrings;
    ComboBox* pListBox = _bSearch ? m_pSearchLB.get() : m_pReplaceLB.get();

    // ignore identical strings
    for (std::vector<OUString>::const_iterator i = pArr->begin(); i != pArr->end(); ++i)
    {
        if ((*i).equals(rStr))
            return;
    }

    // delete oldest entry at maximum occupancy (ListBox and Array)
    if(REMEMBER_SIZE < pArr->size())
    {
        pListBox->RemoveEntryAt(static_cast<sal_uInt16>(REMEMBER_SIZE - 1));
        (*pArr)[REMEMBER_SIZE - 1] = rStr;
        pArr->erase(pArr->begin() + REMEMBER_SIZE - 1);
    }

    pArr->insert(pArr->begin(), rStr);
    pListBox->InsertEntry(rStr, 0);
}


void SvxSearchDialog::TemplatesChanged_Impl( SfxStyleSheetBasePool& rPool )
{
    OUString aOldSrch( m_pSearchTmplLB->GetSelectEntry() );
    OUString aOldRepl( m_pReplaceTmplLB->GetSelectEntry() );
    m_pSearchTmplLB->Clear();
    m_pReplaceTmplLB->Clear();
    rPool.SetSearchMask( pSearchItem->GetFamily() );
    m_pSearchTmplLB->SetUpdateMode( false );
    m_pReplaceTmplLB->SetUpdateMode( false );
    SfxStyleSheetBase* pBase = rPool.First();

    while ( pBase )
    {
        if ( pBase->IsUsed() )
            m_pSearchTmplLB->InsertEntry( pBase->GetName() );
        m_pReplaceTmplLB->InsertEntry( pBase->GetName() );
        pBase = rPool.Next();
    }
    m_pSearchTmplLB->SetUpdateMode( true );
    m_pReplaceTmplLB->SetUpdateMode( true );
    m_pSearchTmplLB->SelectEntryPos(0);

    if ( !aOldSrch.isEmpty() )
        m_pSearchTmplLB->SelectEntry( aOldSrch );
    m_pReplaceTmplLB->SelectEntryPos(0);

    if ( !aOldRepl.isEmpty() )
        m_pReplaceTmplLB->SelectEntry( aOldRepl );

    if ( m_pSearchTmplLB->GetEntryCount() )
    {
        EnableControl_Impl(m_pSearchBtn);
        EnableControl_Impl(m_pBackSearchBtn);
        EnableControl_Impl(m_pSearchAllBtn);
        EnableControl_Impl(m_pReplaceBtn);
        EnableControl_Impl(m_pReplaceAllBtn);
    }
}


void SvxSearchDialog::EnableControls_Impl( const SearchOptionFlags nFlags )
{
    if ( nFlags == nOptions )
        return;
    else
        nOptions = nFlags;

    if ( nOptions == SearchOptionFlags::NONE )
    {
        if ( IsVisible() )
        {
            Hide();
            return;
        }
    }
    else if ( !IsVisible() )
        Show();
    bool bNoSearch = true;

    bool bEnableSearch = bool( SearchOptionFlags::SEARCH & nOptions );
    m_pSearchBtn->Enable(bEnableSearch);
    m_pBackSearchBtn->Enable(bEnableSearch);

    if( bEnableSearch )
        bNoSearch = false;


    if ( ( SearchOptionFlags::SEARCHALL & nOptions ) )
    {
        m_pSearchAllBtn->Enable();
        bNoSearch = false;
    }
    else
        m_pSearchAllBtn->Disable();
    if ( ( SearchOptionFlags::REPLACE & nOptions ) )
    {
        m_pReplaceBtn->Enable();
        m_pReplaceFrame->get_label_widget()->Enable();
        m_pReplaceLB->Enable();
        m_pReplaceTmplLB->Enable();
        bNoSearch = false;
    }
    else
    {
        m_pReplaceBtn->Disable();
        m_pReplaceFrame->get_label_widget()->Disable();
        m_pReplaceLB->Disable();
        m_pReplaceTmplLB->Disable();
    }
    if ( ( SearchOptionFlags::REPLACE_ALL & nOptions ) )
    {
        m_pReplaceAllBtn->Enable();
        bNoSearch = false;
    }
    else
        m_pReplaceAllBtn->Disable();
    m_pComponentFrame->Enable(!bNoSearch);
    m_pSearchBtn->Enable( !bNoSearch );
    m_pBackSearchBtn->Enable( !bNoSearch );
    m_pSearchFrame->get_label_widget()->Enable( !bNoSearch );
    m_pSearchLB->Enable( !bNoSearch );
    m_pNotesBtn->Enable();

    if ( ( SearchOptionFlags::WHOLE_WORDS & nOptions ) )
        m_pWordBtn->Enable();
    else
        m_pWordBtn->Disable();
    if ( ( SearchOptionFlags::BACKWARDS & nOptions ) )
    {
        m_pBackSearchBtn->Enable();
        m_pReplaceBackwardsCB->Enable();
    }
    else
    {
        m_pBackSearchBtn->Disable();
        m_pReplaceBackwardsCB->Disable();
    }
    if ( ( SearchOptionFlags::REG_EXP & nOptions ) )
        m_pRegExpBtn->Enable();
    else
        m_pRegExpBtn->Disable();
    if ( ( SearchOptionFlags::WILDCARD & nOptions ) )
        m_pWildcardBtn->Enable();
    else
        m_pWildcardBtn->Disable();
    if ( ( SearchOptionFlags::EXACT & nOptions ) )
        m_pMatchCaseCB->Enable();
    else
        m_pMatchCaseCB->Disable();
    if ( ( SearchOptionFlags::SELECTION & nOptions ) )
        m_pSelectionBtn->Enable();
    else
        m_pSelectionBtn->Disable();
    if ( ( SearchOptionFlags::FAMILIES & nOptions ) )
        m_pLayoutBtn->Enable();
    else
        m_pLayoutBtn->Disable();
    if ( ( SearchOptionFlags::FORMAT & nOptions ) )
    {
        m_pAttributeBtn->Enable();
        m_pFormatBtn->Enable();
        m_pNoFormatBtn->Enable();
    }
    else
    {
        m_pAttributeBtn->Disable();
        m_pFormatBtn->Disable();
        m_pNoFormatBtn->Disable();
    }

    if ( ( SearchOptionFlags::SIMILARITY & nOptions ) )
    {
        m_pSimilarityBox->Enable();
        m_pSimilarityBtn->Enable();
    }
    else
    {
        m_pSimilarityBox->Disable();
        m_pSimilarityBtn->Disable();
    }

    if ( pSearchItem )
        Init_Impl( pSearchItem->GetPattern() &&
                   ( !pSearchList || !pSearchList->Count() ) );
}


void SvxSearchDialog::EnableControl_Impl( Control* pCtrl )
{
    if (m_pSearchBtn == pCtrl && ( SearchOptionFlags::SEARCH & nOptions ) )
    {
        m_pComponentFrame->Enable();
        m_pSearchBtn->Enable();
        return;
    }
    if ( m_pSearchAllBtn == pCtrl &&
         ( SearchOptionFlags::SEARCHALL & nOptions )  )
    {
        m_pSearchAllBtn->Enable();
        return;
    }
    if ( m_pReplaceBtn == pCtrl && ( SearchOptionFlags::REPLACE & nOptions )  )
    {
        m_pReplaceBtn->Enable();
        return;
    }
    if ( m_pReplaceAllBtn == pCtrl &&
         ( SearchOptionFlags::REPLACE_ALL & nOptions ) )
    {
        m_pReplaceAllBtn->Enable();
        return;
    }
    if ( m_pWordBtn == pCtrl && ( SearchOptionFlags::WHOLE_WORDS & nOptions ) )
    {
        m_pWordBtn->Enable();
        return;
    }
    if ( SearchOptionFlags::BACKWARDS & nOptions )
    {
        if( m_pBackSearchBtn == pCtrl )
        {
            m_pBackSearchBtn->Enable();
            return;
        }
        else if ( m_pReplaceBackwardsCB == pCtrl )
        {
            m_pReplaceBackwardsCB->Enable();
            return;
        }
    }
    if (m_pNotesBtn == pCtrl)
    {
        m_pNotesBtn->Enable();
        return;
    }
    if ( m_pRegExpBtn == pCtrl && ( SearchOptionFlags::REG_EXP & nOptions )
        && !m_pSimilarityBox->IsChecked() && !m_pWildcardBtn->IsChecked())
    {
        m_pRegExpBtn->Enable();
        return;
    }
    if ( m_pWildcardBtn == pCtrl && ( SearchOptionFlags::WILDCARD & nOptions )
        && !m_pSimilarityBox->IsChecked() && !m_pRegExpBtn->IsChecked())
    {
        m_pWildcardBtn->Enable();
        return;
    }
    if ( m_pMatchCaseCB == pCtrl && ( SearchOptionFlags::EXACT & nOptions ) )
    {
        if (!m_pJapOptionsCB->IsChecked())
            m_pMatchCaseCB->Enable();
        return;
    }
    if ( m_pSelectionBtn == pCtrl && ( SearchOptionFlags::SELECTION & nOptions ) )
    {
        m_pSelectionBtn->Enable();
        return;
    }
    if ( m_pLayoutBtn == pCtrl && ( SearchOptionFlags::FAMILIES & nOptions ) )
    {
        m_pLayoutBtn->Enable();
        return;
    }
    if ( m_pAttributeBtn == pCtrl
         && ( SearchOptionFlags::FORMAT & nOptions )
         && pSearchList )
    {
        m_pAttributeBtn->Enable( pImpl->bFocusOnSearch );
    }
    if ( m_pFormatBtn == pCtrl && ( SearchOptionFlags::FORMAT & nOptions ) )
    {
        m_pFormatBtn->Enable();
        return;
    }
    if ( m_pNoFormatBtn == pCtrl && ( SearchOptionFlags::FORMAT & nOptions ) )
    {
        m_pNoFormatBtn->Enable();
        return;
    }
    if ( m_pSimilarityBox == pCtrl && ( SearchOptionFlags::SIMILARITY & nOptions )
        && !m_pRegExpBtn->IsChecked() && !m_pWildcardBtn->IsChecked())
    {
        m_pSimilarityBox->Enable();

        if ( m_pSimilarityBox->IsChecked() )
            m_pSimilarityBtn->Enable();
    }
}


void SvxSearchDialog::SetItem_Impl( const SvxSearchItem* pItem )
{
    if ( pItem )
    {
        delete pSearchItem;
        pSearchItem = static_cast<SvxSearchItem*>(pItem->Clone());
        Init_Impl( pSearchItem->GetPattern() &&
                   ( !pSearchList || !pSearchList->Count() ) );
    }
}


IMPL_LINK( SvxSearchDialog, FocusHdl_Impl, Control&, rControl, void )
{
    sal_Int32 nTxtLen = m_pSearchAttrText->GetText().getLength();
    Control* pCtrl = &rControl;
    if ( pCtrl == m_pSearchLB )
    {
        if ( pCtrl->HasChildPathFocus() )
            pImpl->bFocusOnSearch = true;
        pCtrl = m_pSearchLB;
        bSearch = true;

        if( nTxtLen )
            EnableControl_Impl(m_pNoFormatBtn);
        else
            m_pNoFormatBtn->Disable();
        EnableControl_Impl(m_pAttributeBtn);
    }
    else
    {
        pImpl->bFocusOnSearch = false;
        pCtrl = m_pReplaceLB;
        bSearch = false;

        if (!m_pReplaceAttrText->GetText().isEmpty())
            EnableControl_Impl(m_pNoFormatBtn);
        else
            m_pNoFormatBtn->Disable();
        m_pAttributeBtn->Disable();
    }
    bSet = true;

    static_cast<ComboBox*>(pCtrl)->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );

    ModifyHdl_Impl( static_cast<Edit&>(*pCtrl) );

    if (bFormat && nTxtLen)
        m_pLayoutBtn->SetText(aLayoutStr);
    else
    {
        SvtModuleOptions::EFactory eFactory = getModule(rBindings);
        bool bWriterApp =
            eFactory == SvtModuleOptions::EFactory::WRITER ||
            eFactory == SvtModuleOptions::EFactory::WRITERWEB ||
            eFactory == SvtModuleOptions::EFactory::WRITERGLOBAL;
        bool bCalcApp = eFactory == SvtModuleOptions::EFactory::CALC;

        if (bWriterApp)
            m_pLayoutBtn->SetText(aLayoutWriterStr);
        else
        {
            if (bCalcApp)
                m_pLayoutBtn->SetText(aLayoutCalcStr);
            else
                m_pLayoutBtn->SetText(aStylesStr);
        }
    }
}


IMPL_LINK_NOARG(SvxSearchDialog, LoseFocusHdl_Impl, Control&, void)
{
    SaveToModule_Impl();
}


IMPL_LINK_NOARG(SvxSearchDialog, FormatHdl_Impl, Button*, void)
{
    SfxObjectShell* pSh = SfxObjectShell::Current();

    DBG_ASSERT( pSh, "no DocShell" );

    if ( !pSh || !pImpl->pRanges )
        return;

    sal_sSize nCnt = 0;
    const sal_uInt16* pPtr = pImpl->pRanges.get();
    const sal_uInt16* pTmp = pPtr;

    while( *pTmp )
        pTmp++;
    nCnt = pTmp - pPtr + 7;
    std::unique_ptr<sal_uInt16[]> pWhRanges(new sal_uInt16[nCnt]);
    sal_uInt16 nPos = 0;

    while( *pPtr )
    {
        pWhRanges[nPos++] = *pPtr++;
    }

    pWhRanges[nPos++] = SID_ATTR_PARA_MODEL;
    pWhRanges[nPos++] = SID_ATTR_PARA_MODEL;

    sal_uInt16 nBrushWhich = pSh->GetPool().GetWhich(SID_ATTR_BRUSH);
    pWhRanges[nPos++] = nBrushWhich;
    pWhRanges[nPos++] = nBrushWhich;
    pWhRanges[nPos] = 0;
    SfxItemPool& rPool = pSh->GetPool();
    SfxItemSet aSet( rPool, pWhRanges.get() );
    OUString aTxt;

    aSet.InvalidateAllItems();
    aSet.Put(SvxBrushItem(nBrushWhich));

    if ( bSearch )
    {
        aTxt = SVX_RESSTR( RID_SVXSTR_SEARCH );
        pSearchList->Get( aSet );
    }
    else
    {
        aTxt = SVX_RESSTR( RID_SVXSTR_REPLACE );
        pReplaceList->Get( aSet );
    }
    aSet.DisableItem(SID_ATTR_PARA_MODEL);
    aSet.DisableItem(rPool.GetWhich(SID_ATTR_PARA_PAGEBREAK));
    aSet.DisableItem(rPool.GetWhich(SID_ATTR_PARA_KEEP));


    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateTabItemDialog(this, aSet));
        DBG_ASSERT(pDlg, "Dialog creation failed!");
        aTxt = pDlg->GetText() + aTxt;
        pDlg->SetText( aTxt );

        if ( pDlg->Execute() == RET_OK )
        {
            DBG_ASSERT( pDlg->GetOutputItemSet(), "invalid Output-Set" );
            SfxItemSet aOutSet( *pDlg->GetOutputItemSet() );

            SearchAttrItemList* pList = bSearch ? pSearchList : pReplaceList;

            const SfxPoolItem* pItem;
            for( sal_uInt16 n = 0; n < pList->Count(); ++n )
            {
                SearchAttrItem* pAItem;
                if( !IsInvalidItem( (pAItem = &pList->GetObject(n))->pItem ) &&
                    SfxItemState::SET == aOutSet.GetItemState(
                        pAItem->pItem->Which(), false, &pItem ) )
                {
                    delete pAItem->pItem;
                    pAItem->pItem = pItem->Clone();
                    aOutSet.ClearItem( pAItem->pItem->Which() );
                }
            }

            if( aOutSet.Count() )
                pList->Put( aOutSet );

            PaintAttrText_Impl(); // Set AttributText in GroupBox
        }
    }
}


IMPL_LINK_NOARG(SvxSearchDialog, NoFormatHdl_Impl, Button*, void)
{
    SvtModuleOptions::EFactory eFactory = getModule(rBindings);
    bool bWriterApp =
        eFactory == SvtModuleOptions::EFactory::WRITER ||
        eFactory == SvtModuleOptions::EFactory::WRITERWEB ||
        eFactory == SvtModuleOptions::EFactory::WRITERGLOBAL;
    bool bCalcApp = eFactory == SvtModuleOptions::EFactory::CALC;

    if (bCalcApp)
        m_pLayoutBtn->SetText( aLayoutCalcStr );
    else
    {
        if (bWriterApp)
            m_pLayoutBtn->SetText( aLayoutWriterStr);
        else
            m_pLayoutBtn->SetText( aStylesStr );
    }

    bFormat = false;
    m_pLayoutBtn->Check( false );

    if ( bSearch )
    {
        pSearchList->Clear();
        m_pSearchAttrText->SetText( "" );
        m_pSearchAttrText->Hide();
    }
    else
    {
        pReplaceList->Clear();
        m_pReplaceAttrText->SetText( "" );
        m_pReplaceAttrText->Hide();
    }

    pImpl->bSaveToModule = false;
    TemplateHdl_Impl(m_pLayoutBtn);
    pImpl->bSaveToModule = true;
    m_pNoFormatBtn->Disable();
}


IMPL_LINK_NOARG(SvxSearchDialog, AttributeHdl_Impl, Button*, void)
{
    if ( !pSearchList || !pImpl->pRanges )
        return;

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSvxSearchAttributeDialog( this, *pSearchList, pImpl->pRanges.get() ));
        DBG_ASSERT(pDlg, "Dialog creation failed!");
        pDlg->Execute();
    }
    PaintAttrText_Impl();
}


IMPL_LINK( SvxSearchDialog, TimeoutHdl_Impl, Timer *, pTimer, void )
{
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if ( pViewShell )
    {
        if ( pViewShell->HasSelection( m_pSearchLB->IsVisible() ) )
            EnableControl_Impl(m_pSelectionBtn);
        else
        {
            m_pSelectionBtn->Check( false );
            m_pSelectionBtn->Disable();
        }
    }

    pTimer->Start();
}


OUString& SvxSearchDialog::BuildAttrText_Impl( OUString& rStr,
                                             bool bSrchFlag ) const
{
    rStr.clear();

    SfxObjectShell* pSh = SfxObjectShell::Current();
    DBG_ASSERT( pSh, "no DocShell" );

    if ( !pSh )
        return rStr;

    SfxItemPool& rPool = pSh->GetPool();
    SearchAttrItemList* pList = bSrchFlag ? pSearchList : pReplaceList;

    if ( !pList )
        return rStr;

    // Metric query
    MapUnit eMapUnit = MapUnit::MapCM;
    FieldUnit eFieldUnit = pSh->GetModule()->GetFieldUnit();
    switch ( eFieldUnit )
    {
        case FUNIT_MM:          eMapUnit = MapUnit::MapMM; break;
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:          eMapUnit = MapUnit::MapCM; break;
        case FUNIT_TWIP:        eMapUnit = MapUnit::MapTwip; break;
        case FUNIT_POINT:
        case FUNIT_PICA:        eMapUnit = MapUnit::MapPoint; break;
        case FUNIT_INCH:
        case FUNIT_FOOT:
        case FUNIT_MILE:        eMapUnit = MapUnit::MapInch; break;
        case FUNIT_100TH_MM:    eMapUnit = MapUnit::Map100thMM; break;
        default: ;//prevent warning
    }

    ResStringArray aAttrNames( SVX_RES( RID_ATTR_NAMES ) );

    for ( sal_uInt16 i = 0; i < pList->Count(); ++i )
    {
        const SearchAttrItem& rItem = pList->GetObject(i);

        if ( !rStr.isEmpty() )
            rStr += ", ";

        if ( !IsInvalidItem( rItem.pItem ) )
        {
            OUString aStr;
            rPool.GetPresentation( *rItem.pItem,
                                    eMapUnit, aStr );
            rStr += aStr;
        }
        else if ( rItem.nSlot == SID_ATTR_BRUSH_CHAR )
        {
            // Special treatment for text background
            rStr += SVX_RESSTR( RID_SVXITEMS_BRUSH_CHAR );
        }
        else
        {
            sal_uInt32 nId  = aAttrNames.FindIndex( rItem.nSlot );
            if ( RESARRAY_INDEX_NOTFOUND != nId )
                rStr += aAttrNames.GetString( nId );
        }
    }
    return rStr;
}


void SvxSearchDialog::PaintAttrText_Impl()
{
    OUString aDesc;
    BuildAttrText_Impl( aDesc, bSearch );

    if ( !bFormat && !aDesc.isEmpty() )
        bFormat = true;

    if ( bSearch )
    {
        m_pSearchAttrText->SetText( aDesc );
        if(!aDesc.isEmpty())
            m_pSearchAttrText->Show();

        FocusHdl_Impl(*m_pSearchLB);
    }
    else
    {
        m_pReplaceAttrText->SetText( aDesc );
        if(!aDesc.isEmpty())
            m_pReplaceAttrText->Show();

        FocusHdl_Impl(*m_pReplaceLB);
    }
}


void SvxSearchDialog::SetModifyFlag_Impl( const Control* pCtrl )
{
    if ( m_pSearchLB == pCtrl )
        nModifyFlag |= ModifyFlags::Search;
    else if ( m_pReplaceLB == pCtrl )
        nModifyFlag |= ModifyFlags::Replace;
    else if ( m_pWordBtn == pCtrl )
        nModifyFlag |= ModifyFlags::Word;
    else if ( m_pMatchCaseCB == pCtrl )
        nModifyFlag |= ModifyFlags::Exact;
    else if ( m_pReplaceBackwardsCB == pCtrl )
        nModifyFlag |= ModifyFlags::Backwards;
    else if ( m_pNotesBtn == pCtrl )
        nModifyFlag |= ModifyFlags::Notes;
    else if ( m_pSelectionBtn == pCtrl )
        nModifyFlag |= ModifyFlags::Selection;
    else if ( m_pRegExpBtn == pCtrl )
        nModifyFlag |= ModifyFlags::Regexp;
    else if ( m_pWildcardBtn == pCtrl )
        nModifyFlag |= ModifyFlags::Wildcard;
    else if ( m_pLayoutBtn == pCtrl )
        nModifyFlag |= ModifyFlags::Layout;
    else if ( m_pSimilarityBox == pCtrl )
        nModifyFlag |= ModifyFlags::Similarity;
    else if ( m_pCalcSearchInLB == pCtrl )
    {
        nModifyFlag |= ModifyFlags::Formulas;
        nModifyFlag |= ModifyFlags::Values;
        nModifyFlag |= ModifyFlags::CalcNotes;
    }
    else if ( m_pRowsBtn == pCtrl )
        nModifyFlag |= ModifyFlags::Rows;
    else if ( m_pColumnsBtn == pCtrl )
        nModifyFlag |= ModifyFlags::Columns;
    else if ( m_pAllSheetsCB == pCtrl )
        nModifyFlag |= ModifyFlags::AllTables;
}


void SvxSearchDialog::SaveToModule_Impl()
{
    if ( !pSearchItem )
        return;

    if ( m_pLayoutBtn->IsChecked() )
    {
        pSearchItem->SetSearchString ( m_pSearchTmplLB->GetSelectEntry() );
        pSearchItem->SetReplaceString( m_pReplaceTmplLB->GetSelectEntry() );
    }
    else
    {
        pSearchItem->SetSearchString ( m_pSearchLB->GetText() );
        pSearchItem->SetReplaceString( m_pReplaceLB->GetText() );
        Remember_Impl( m_pSearchLB->GetText(), true );
    }

    pSearchItem->SetRegExp( false );
    pSearchItem->SetWildcard( false );
    pSearchItem->SetLevenshtein( false );
    if (GetCheckBoxValue(m_pRegExpBtn))
        pSearchItem->SetRegExp( true );
    else if (GetCheckBoxValue(m_pWildcardBtn))
        pSearchItem->SetWildcard( true );
    else if (GetCheckBoxValue(m_pSimilarityBox))
        pSearchItem->SetLevenshtein( true );

    pSearchItem->SetWordOnly(GetCheckBoxValue(m_pWordBtn));
    pSearchItem->SetBackward(GetCheckBoxValue(m_pReplaceBackwardsCB));
    pSearchItem->SetNotes(GetCheckBoxValue(m_pNotesBtn));
    pSearchItem->SetPattern(GetCheckBoxValue(m_pLayoutBtn));
    pSearchItem->SetSelection(GetCheckBoxValue(m_pSelectionBtn));
    pSearchItem->SetUseAsianOptions(GetCheckBoxValue(m_pJapOptionsCB));

    SvtSearchOptions aOpt;
    aOpt.SetIgnoreDiacritics_CTL(GetCheckBoxValue(m_pIgnoreDiacritics));
    aOpt.SetIgnoreKashida_CTL(GetCheckBoxValue(m_pIgnoreKashida));
    aOpt.Commit();

    sal_Int32 nFlags = GetTransliterationFlags();
    if( !pSearchItem->IsUseAsianOptions())
        nFlags &= (TransliterationModules_IGNORE_CASE |
                   TransliterationModules_IGNORE_WIDTH );
    if (GetCheckBoxValue(m_pIgnoreDiacritics))
        nFlags |= TransliterationModulesExtra::IGNORE_DIACRITICS_CTL;
    if (GetCheckBoxValue(m_pIgnoreKashida))
        nFlags |= TransliterationModulesExtra::IGNORE_KASHIDA_CTL;
    pSearchItem->SetTransliterationFlags( nFlags );

    if ( !bWriter )
    {
        if ( m_pCalcSearchInLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
            pSearchItem->SetCellType( static_cast<SvxSearchCellType>(m_pCalcSearchInLB->GetSelectEntryPos()) );

        pSearchItem->SetRowDirection( m_pRowsBtn->IsChecked() );
        pSearchItem->SetAllTables( m_pAllSheetsCB->IsChecked() );
        pSearchItem->SetSearchFormatted( m_pSearchFormattedCB->IsChecked() );
    }

    pSearchItem->SetCommand( SvxSearchCmd::FIND );
    nModifyFlag = ModifyFlags::NONE;
    const SfxPoolItem* ppArgs[] = { pSearchItem, nullptr };
    rBindings.GetDispatcher()->Execute( SID_SEARCH_ITEM, SfxCallMode::SLOT, ppArgs );
}

css::uno::Reference< css::awt::XWindowPeer >
        SvxSearchDialog::GetComponentInterface( bool bCreate )
{
    css::uno::Reference< css::awt::XWindowPeer > xPeer( Window::GetComponentInterface(false) );
    if ( !xPeer.is() && bCreate )
    {
        css::awt::XWindowPeer* pPeer = new VCLXSvxFindReplaceDialog(this);
        SetComponentInterface(pPeer);
        return pPeer;
    }
    else
        return xPeer;
}

SFX_IMPL_CHILDWINDOW_WITHID(SvxSearchDialogWrapper, SID_SEARCH_DLG);


SvxSearchDialogWrapper::SvxSearchDialogWrapper( vcl::Window* _pParent, sal_uInt16 nId,
                                                SfxBindings* pBindings,
                                                SfxChildWinInfo* pInfo )
    : SfxChildWindow( _pParent, nId )
    , dialog (VclPtr<SvxSearchDialog>::Create(_pParent, this, *pBindings))
{
    SetWindow(dialog);
    dialog->Initialize( pInfo );

    pBindings->Update( SID_SEARCH_ITEM );
    pBindings->Update( SID_SEARCH_OPTIONS );
    pBindings->Update( SID_SEARCH_SEARCHSET );
    pBindings->Update( SID_SEARCH_REPLACESET );
    dialog->bConstruct = false;
}

SvxSearchDialogWrapper::~SvxSearchDialogWrapper ()
{
}


SfxChildWinInfo SvxSearchDialogWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    aInfo.bVisible = false;
    return aInfo;
}


static vcl::Window* lcl_GetSearchLabelWindow()
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return nullptr;

    css::uno::Reference< css::beans::XPropertySet > xPropSet(
            pViewFrame->GetFrame().GetFrameInterface(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    xPropSet->getPropertyValue("LayoutManager") >>= xLayoutManager;
    css::uno::Reference< css::ui::XUIElement > xUIElement =
        xLayoutManager->getElement("private:resource/toolbar/findbar");
    if (!xUIElement.is())
        return nullptr;
    css::uno::Reference< css::awt::XWindow > xWindow(
            xUIElement->getRealInterface(), css::uno::UNO_QUERY_THROW);
    VclPtr< ToolBox > pToolBox = static_cast<ToolBox*>( VCLUnoHelper::GetWindow(xWindow).get() );
    for (size_t i = 0; pToolBox && i < pToolBox->GetItemCount(); ++i)
        if (pToolBox->GetItemCommand(i) == ".uno:SearchLabel")
            return pToolBox->GetItemWindow(i);
    return nullptr;
}

void SvxSearchDialogWrapper::SetSearchLabel(const SearchLabel& rSL)
{
    OUString sStr;
    if (rSL == SearchLabel::End)
        sStr = SVX_RESSTR(RID_SVXSTR_SEARCH_END);
    else if (rSL == SearchLabel::Start)
        sStr = SVX_RESSTR(RID_SVXSTR_SEARCH_START);
    else if (rSL == SearchLabel::EndSheet)
        sStr = SVX_RESSTR(RID_SVXSTR_SEARCH_END_SHEET);
    else if (rSL == SearchLabel::NotFound)
        sStr = SVX_RESSTR(RID_SVXSTR_SEARCH_NOT_FOUND);

    if (vcl::Window *pSearchLabel = lcl_GetSearchLabelWindow())
    {
        if (sStr.isEmpty())
            pSearchLabel->Hide();
        else
        {
            pSearchLabel->SetText(sStr);
            pSearchLabel->Show();
        }
    }
    if (SvxSearchDialogWrapper *pWrp = static_cast<SvxSearchDialogWrapper*>( SfxViewFrame::Current()->
            GetChildWindow( SvxSearchDialogWrapper::GetChildWindowId() )))
        pWrp->getDialog()->SetSearchLabel(sStr);
}

void SvxSearchDialogWrapper::SetSearchLabel(const OUString& sStr)
{

    if (vcl::Window *pSearchLabel = lcl_GetSearchLabelWindow())
    {
        if (sStr.isEmpty())
            pSearchLabel->Hide();
        else
        {
            pSearchLabel->SetText(sStr);
            pSearchLabel->Show();
        }
    }
    if (SvxSearchDialogWrapper *pWrp = static_cast<SvxSearchDialogWrapper*>( SfxViewFrame::Current()->
            GetChildWindow( SvxSearchDialogWrapper::GetChildWindowId() )))
        pWrp->getDialog()->SetSearchLabel(sStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
