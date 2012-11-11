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


#include <sal/macros.h>
#include <vcl/wrkwin.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/msgbox.hxx>
#include <svl/slstitm.hxx>
#include <svl/itemiter.hxx>
#include <svl/style.hxx>
#include <unotools/searchopt.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/cjkoptions.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/itempool.hxx>
#include <svl/intitem.hxx>

#include <sfx2/app.hxx>
#include <toolkit/unohlp.hxx>

#define _SVX_SRCHDLG_CXX
#include "svx/srchdlg.hxx"

#include <svx/dialogs.hrc>
#include <svx/svxitems.hrc>
#include "srchdlg.hrc"


#define ITEMID_SETITEM      0

#include <svl/srchitem.hxx>
#include <svx/pageitem.hxx>
#include "srchctrl.hxx"
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <editeng/brshitem.hxx>
#include <tools/resary.hxx>
#include <svx/svxdlg.hxx>

using namespace com::sun::star::i18n;
using namespace com::sun::star;
using namespace comphelper;

// -----------------------------------------------------------------------

#define REMEMBER_SIZE       10

#define MODIFY_SEARCH       0x00000001
#define MODIFY_REPLACE      0x00000002
#define MODIFY_WORD         0x00000004
#define MODIFY_EXACT        0x00000008
#define MODIFY_BACKWARDS    0x00000010
#define MODIFY_SELECTION    0x00000020
#define MODIFY_REGEXP       0x00000040
#define MODIFY_LAYOUT       0x00000080
#define MODIFY_SIMILARITY   0x00000100
#define MODIFY_FORMULAS     0x00000200
#define MODIFY_VALUES       0x00000400
#define MODIFY_CALC_NOTES   0x00000800
#define MODIFY_ROWS         0x00001000
#define MODIFY_COLUMNS      0x00002000
#define MODIFY_ALLTABLES    0x00004000
#define MODIFY_NOTES        0x00008000

#define GetCheckBoxValue( rBox )                                \
    rBox.IsEnabled() ? rBox.IsChecked() : sal_False

struct SearchDlg_Impl
{
    FixedText   aSearchFormats;
    FixedText   aReplaceFormats;

    sal_Bool        bMultiLineEdit   : 1,
                bSaveToModule    : 1,
                bFocusOnSearch   : 1,
                bDeltaCalculated : 1;
    sal_uInt16*     pRanges;
    Timer       aSelectionTimer;

    uno::Reference< frame::XDispatch > xCommand1Dispatch;
    uno::Reference< frame::XDispatch > xCommand2Dispatch;
    util::URL   aCommand1URL;
    util::URL   aCommand2URL;

        SearchDlg_Impl( Window* pParent ) :
        aSearchFormats  ( pParent, SVX_RES( FT_SEARCH_FORMATS ) ),
        aReplaceFormats ( pParent, SVX_RES( FT_REPLACE_FORMATS ) ),
        bMultiLineEdit  ( sal_False ),
        bSaveToModule   ( sal_True ),
        bFocusOnSearch  ( sal_True ),
        bDeltaCalculated( sal_False ),
        pRanges         ( NULL )
        {
            aCommand1URL.Complete = aCommand1URL.Main = rtl::OUString("vnd.sun.search:SearchViaComponent1");
            aCommand1URL.Protocol = rtl::OUString("vnd.sun.search:");
            aCommand1URL.Path = rtl::OUString("SearchViaComponent1");
            aCommand2URL.Complete = aCommand2URL.Main = rtl::OUString("vnd.sun.search:SearchViaComponent2");
            aCommand2URL.Protocol = rtl::OUString("vnd.sun.search:");
            aCommand2URL.Path = rtl::OUString("SearchViaComponent2");
        }
    ~SearchDlg_Impl() { delete[] pRanges; }
};

void ListToStrArr_Impl( sal_uInt16 nId, std::vector<rtl::OUString>& rStrLst, ComboBox& rCBox )
{
    SfxStringListItem* pSrchItem =
        (SfxStringListItem*)SFX_APP()->GetItem( nId );

    if (pSrchItem)
    {
        std::vector<String> aLst = pSrchItem->GetList();

        for ( sal_uInt16 i = 0; i < aLst.size(); ++i )
        {
            rStrLst.push_back(aLst[i]);
            rCBox.InsertEntry(aLst[i]);
        }
    }
}

void StrArrToList_Impl( sal_uInt16 nId, const std::vector<rtl::OUString>& rStrLst )
{
    DBG_ASSERT( !rStrLst.empty(), "check in advance");
    std::vector<String> aLst;

    for (std::vector<rtl::OUString>::const_iterator i = rStrLst.begin(); i != rStrLst.end(); ++i)
        aLst.push_back(String(*i));

    SFX_APP()->PutItem( SfxStringListItem( nId, &aLst ) );
}

// class SearchAttrItemList ----------------------------------------------

SearchAttrItemList::SearchAttrItemList( const SearchAttrItemList& rList ) :
    SrchAttrItemList(rList)
{
    for ( sal_uInt16 i = 0; i < size(); ++i )
        if ( !IsInvalidItem( (*this)[i].pItem ) )
            (*this)[i].pItem = (*this)[i].pItem->Clone();
}

// -----------------------------------------------------------------------

SearchAttrItemList::~SearchAttrItemList()
{
    Clear();
}

// -----------------------------------------------------------------------

void SearchAttrItemList::Put( const SfxItemSet& rSet )
{
    if ( !rSet.Count() )
        return;

    SfxItemPool* pPool = rSet.GetPool();
    SfxItemIter aIter( rSet );
    SearchAttrItem aItem;
    const SfxPoolItem* pItem = aIter.GetCurItem();
    sal_uInt16 nWhich;

    while ( sal_True )
    {
        // only test that it is available?
        if( IsInvalidItem( pItem ) )
        {
            nWhich = rSet.GetWhichByPos( aIter.GetCurPos() );
            aItem.pItem = (SfxPoolItem*)pItem;
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

// -----------------------------------------------------------------------

SfxItemSet& SearchAttrItemList::Get( SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();

    for ( sal_uInt16 i = 0; i < size(); ++i )
        if ( IsInvalidItem( (*this)[i].pItem ) )
            rSet.InvalidateItem( pPool->GetWhich( (*this)[i].nSlot ) );
        else
            rSet.Put( *(*this)[i].pItem );
    return rSet;
}

// -----------------------------------------------------------------------

void SearchAttrItemList::Clear()
{
    for ( sal_uInt16 i = 0; i < size(); ++i )
        if ( !IsInvalidItem( (*this)[i].pItem ) )
            delete (*this)[i].pItem;
    SrchAttrItemList::clear();
}

// -----------------------------------------------------------------------

// Deletes the pointer to the items
void SearchAttrItemList::Remove(size_t nPos, size_t nLen)
{
    if ( nPos + nLen > size() )
        nLen = size() - nPos;

    for ( sal_uInt16 i = nPos; i < nPos + nLen; ++i )
        if ( !IsInvalidItem( (*this)[i].pItem ) )
            delete (*this)[i].pItem;

    SrchAttrItemList::erase( begin() + nPos, begin() + nPos + nLen );
}

#undef INI_LIST
#define INI_LIST() \
    aSearchText     ( this, SVX_RES( FT_SEARCH ) ),                         \
    aSearchLB       ( this, SVX_RES( ED_SEARCH ) ),                         \
    aSearchTmplLB   ( this, SVX_RES( LB_SEARCH ) ),                         \
    aSearchAttrText ( this, SVX_RES( FT_SEARCH_ATTR ) ),                        \
    aReplaceText    ( this, SVX_RES( FT_REPLACE ) ),                            \
    aReplaceLB      ( this, SVX_RES( ED_REPLACE ) ),                            \
    aReplaceTmplLB  ( this, SVX_RES( LB_REPLACE ) ),                            \
    aReplaceAttrText( this, SVX_RES( FT_REPLACE_ATTR ) ),                       \
    aSearchBtn      ( this, SVX_RES( BTN_SEARCH ) ),                          \
    aSearchAllBtn   ( this, SVX_RES( BTN_SEARCH_ALL ) ),                        \
    aSearchCmdLine  ( this, SVX_RES( FL_SEARCH_COMMAND ) ),                   \
    aReplaceBtn     ( this, SVX_RES( BTN_REPLACE ) ),                         \
    aReplaceAllBtn  ( this, SVX_RES( BTN_REPLACE_ALL ) ),                     \
    aSearchComponentFL( this, SVX_RES( FL_SEARCH_COMPONENT ) ), \
    aSearchComponent1PB( this, SVX_RES( BTN_COMPONENT_1 ) ), \
    aSearchComponent2PB( this, SVX_RES( BTN_COMPONENT_2 ) ), \
    aMatchCaseCB    ( this, SVX_RES( CB_MATCH_CASE ) ),                       \
    aWordBtn        ( this, SVX_RES( CB_WHOLE_WORDS ) ),                      \
    aButtonsFL      ( this, SVX_RES( FL_BUTTONS ) ),                          \
    pMoreBtn        ( new MoreButton( this, SVX_RES( BTN_MORE ) ) ),          \
    aHelpBtn        ( this, SVX_RES( BTN_HELP ) ),                            \
    aCloseBtn       ( this, SVX_RES( BTN_CLOSE ) ),                           \
    aOptionsFL      ( this, SVX_RES( FL_OPTIONS ) ),                          \
    aSelectionBtn   ( this, SVX_RES( CB_SELECTIONS ) ),                       \
    aBackwardsBtn   ( this, SVX_RES( CB_BACKWARDS ) ),                        \
    aRegExpBtn      ( this, SVX_RES( CB_REGEXP ) ),                           \
    aSimilarityBox  ( this, SVX_RES( CB_SIMILARITY) ),                        \
    aSimilarityBtn  ( this, SVX_RES( PB_SIMILARITY) ),                        \
    aLayoutBtn      ( this, SVX_RES( CB_LAYOUTS ) ),                          \
    aNotesBtn       ( this, SVX_RES( CB_COMMENTS ) ),                            \
    aJapMatchFullHalfWidthCB( this, SVX_RES( CB_JAP_MATCH_FULL_HALF_WIDTH ) ),\
    aJapOptionsCB   ( this, SVX_RES( CB_JAP_SOUNDS_LIKE ) ),                  \
    aJapOptionsBtn  ( this, SVX_RES( PB_JAP_OPTIONS ) ),                      \
    aAttributeBtn   ( this, SVX_RES( BTN_ATTRIBUTE ) ),                       \
    aFormatBtn      ( this, SVX_RES( BTN_FORMAT ) ),                            \
    aNoFormatBtn    ( this, SVX_RES( BTN_NOFORMAT ) ),                      \
    aCalcFL         ( this, SVX_RES( FL_CALC ) ),                             \
    aCalcSearchInFT ( this, SVX_RES( FT_CALC_SEARCHIN ) ),                    \
    aCalcSearchInLB ( this, SVX_RES( LB_CALC_SEARCHIN ) ),                    \
    aCalcSearchDirFT( this, SVX_RES( FT_CALC_SEARCHDIR ) ),                   \
    aRowsBtn        ( this, SVX_RES( RB_CALC_ROWS ) ),                        \
    aColumnsBtn     ( this, SVX_RES( RB_CALC_COLUMNS ) ),                     \
    aAllSheetsCB    ( this, SVX_RES( CB_ALL_SHEETS ) ),                       \
    rBindings       ( rBind ),                                              \
    bWriter         ( sal_False ),                                              \
    bSearch         ( sal_True ),                                               \
    bFormat         ( sal_False ),                                              \
    nOptions        ( USHRT_MAX ),                                          \
    bSet            ( sal_False ),                                              \
    bReadOnly       ( sal_False ),                                              \
    bConstruct      ( sal_True ),                                               \
    nModifyFlag     ( 0 ),                                                  \
    aCalcStr        ( SVX_RES( STR_WORDCALC ) ),                       \
    pImpl           ( NULL ),                                               \
    pSearchList     ( NULL ),                                               \
    pReplaceList    ( new SearchAttrItemList ),                             \
    pSearchItem     ( NULL ),                                               \
    pSearchController       ( NULL ),                                       \
    pOptionsController      ( NULL ),                                       \
    pFamilyController       ( NULL ),                                       \
    pSearchSetController    ( NULL ),                                       \
    pReplaceSetController   ( NULL ),                                       \
    nTransliterationFlags   ( 0x00000000 )

// class SvxSearchDialog -------------------------------------------------

SvxSearchDialog::SvxSearchDialog( Window* pParent, SfxChildWindow* pChildWin, SfxBindings& rBind ) :

    SfxModelessDialog( &rBind, pChildWin, pParent, SVX_RES( RID_SVXDLG_SEARCH ) ),

    INI_LIST()

{
    Construct_Impl();
}

#undef INI_LIST

// -----------------------------------------------------------------------

SvxSearchDialog::~SvxSearchDialog()
{
    Hide();

    rBindings.EnterRegistrations();
    delete pSearchController;
    delete pOptionsController;
    delete pFamilyController;
    delete pSearchSetController;
    delete pReplaceSetController;
    rBindings.LeaveRegistrations();

    delete pSearchItem;
    delete pImpl;
    delete pSearchList;
    delete pReplaceList;
    delete pMoreBtn;
}

static void lcl_MoveDown( Window& rWindow, sal_Int32 nOffset )
{
    Point aPos(rWindow.GetPosPixel());
    aPos.Y() += nOffset;
    rWindow.SetPosPixel(aPos);
}

void SvxSearchDialog::Construct_Impl()
{
    // temporary to avoid incompatibility
    pImpl = new SearchDlg_Impl( this );
    pImpl->aSelectionTimer.SetTimeout( 500 );
    pImpl->aSelectionTimer.SetTimeoutHdl(
        LINK( this, SvxSearchDialog, TimeoutHdl_Impl ) );
    EnableControls_Impl( 0 );

    // Store old Text from aWordBtn
    aCalcStr += sal_Unicode('#');
    aCalcStr += aWordBtn.GetText();

    aLayoutStr = SVX_RESSTR( RID_SVXSTR_SEARCH_STYLES );
    aStylesStr = aLayoutBtn.GetText();

    // Get stored search-strings from the application
    ListToStrArr_Impl( SID_SEARCHDLG_SEARCHSTRINGS,
                       aSearchStrings, aSearchLB    );
    ListToStrArr_Impl( SID_SEARCHDLG_REPLACESTRINGS,
                       aReplaceStrings, aReplaceLB  );

    pMoreBtn->SetMoreText( String( SVX_RES( STR_MORE_BTN ) ) );
    pMoreBtn->SetLessText( String( SVX_RES( STR_LESS_BTN ) ) );

    FreeResource();
    InitControls_Impl();

    // Get attribut sets only once in construtor()
    const SfxPoolItem* ppArgs[] = { pSearchItem, 0 };
    const SvxSetItem* pSrchSetItem =
        (const SvxSetItem*) rBindings.GetDispatcher()->Execute( FID_SEARCH_SEARCHSET, SFX_CALLMODE_SLOT, ppArgs );

    if ( pSrchSetItem )
        InitAttrList_Impl( &pSrchSetItem->GetItemSet(), 0 );

    const SvxSetItem* pReplSetItem =
        (const SvxSetItem*)rBindings.GetDispatcher()->Execute( FID_SEARCH_REPLACESET, SFX_CALLMODE_SLOT, ppArgs );

    if ( pReplSetItem )
        InitAttrList_Impl( 0, &pReplSetItem->GetItemSet() );

    // Create controller and update at once
    rBindings.EnterRegistrations();
    pSearchController =
        new SvxSearchController( SID_SEARCH_ITEM, rBindings, *this );
    pOptionsController =
        new SvxSearchController( SID_SEARCH_OPTIONS, rBindings, *this );
    rBindings.LeaveRegistrations();
    rBindings.GetDispatcher()->Execute( FID_SEARCH_ON, SFX_CALLMODE_SLOT, ppArgs );
    pImpl->aSelectionTimer.Start();


    SvtCJKOptions aCJKOptions;
    if(!aCJKOptions.IsJapaneseFindEnabled())
    {
        aJapOptionsCB.Check( sal_False );
        aJapOptionsCB.Hide();
        aJapOptionsBtn.Hide();
    }
    if(!aCJKOptions.IsCJKFontEnabled())
    {
        aJapMatchFullHalfWidthCB.Hide();
    }

    aSimilarityBtn.SetAccessibleRelationLabeledBy(&aSimilarityBox);
    aSimilarityBtn.SetAccessibleRelationMemberOf(&aOptionsFL);
    aJapOptionsBtn.SetAccessibleRelationLabeledBy(&aJapOptionsCB);
    aJapOptionsBtn.SetAccessibleRelationMemberOf(&aOptionsFL);
    aRowsBtn.SetAccessibleRelationMemberOf(&aCalcSearchDirFT);
    aColumnsBtn.SetAccessibleRelationMemberOf(&aCalcSearchDirFT);

    //component extension - show component search buttons if the commands
    // vnd.sun.star::SearchViaComponent1 and 2 are supported
    const uno::Reference< frame::XFrame >xFrame = rBindings.GetActiveFrame();
    const uno::Reference< frame::XDispatchProvider > xDispatchProv(xFrame, uno::UNO_QUERY);
    rtl::OUString sTarget("_self");

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
            uno::Reference< lang::XMultiServiceFactory >  xMgr = getProcessServiceFactory();
            uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider =
                    configuration::theDefaultProvider::get( comphelper::getComponentContext(xMgr) );
            uno::Sequence< uno::Any > aArgs(1);
            ::rtl::OUString sPath( "/org.openoffice.Office.Common/SearchOptions/");
            aArgs[0] <<= sPath;

            uno::Reference< uno::XInterface > xIFace = xConfigurationProvider->createInstanceWithArguments(
                        ::rtl::OUString( "com.sun.star.configuration.ConfigurationUpdateAccess"),
                        aArgs);
            uno::Reference< container::XNameAccess> xDirectAccess(xIFace, uno::UNO_QUERY);
            if(xDirectAccess.is())
            {
                ::rtl::OUString sTemp;
                ::rtl::OUString sProperty( "ComponentSearchGroupLabel");
                uno::Any aRet = xDirectAccess->getByName(sProperty);
                aRet >>= sTemp;
                aSearchComponentFL.SetText( sTemp );
                aRet = xDirectAccess->getByName(::rtl::OUString( "ComponentSearchCommandLabel1"));
                aRet >>= sTemp;
                aSearchComponent1PB.SetText( sTemp );
                aRet = xDirectAccess->getByName(::rtl::OUString( "ComponentSearchCommandLabel2"));
                aRet >>= sTemp;
                aSearchComponent2PB.SetText( sTemp );
            }
        }
        catch(uno::Exception&){}

        if(aSearchComponent1PB.GetText().Len() && bSearchComponent1 )
        {
            aSearchComponentFL.Show();
            aSearchComponent1PB.Show();
        }
        if( aSearchComponent2PB.GetText().Len() )
        {
            if(!aSearchComponent1PB.IsVisible())
            {
                aSearchComponent2PB.SetPosPixel(aSearchComponent1PB.GetPosPixel());
            }
            aSearchComponentFL.Show();
            aSearchComponent2PB.Show();
        }
        if( aSearchComponentFL.IsVisible() && aSearchComponent1PB.IsVisible() )
        {

            //dialog must be resized
            Size aDlgSize(GetSizePixel());
            sal_Int32 nOffset = aSearchCmdLine.GetPosPixel().Y() - aSearchAllBtn.GetPosPixel().Y()
                - aButtonsFL.GetPosPixel().Y() + aSearchComponent2PB.GetPosPixel().Y();

            aDlgSize.Height() += nOffset;
            Window* aWindows[] =
            {
                &aOptionsFL,
                &aSelectionBtn,
                &aBackwardsBtn,
                &aRegExpBtn,
                &aSimilarityBox,
                &aSimilarityBtn,
                &aLayoutBtn,
                &aNotesBtn,
                &aJapMatchFullHalfWidthCB,
                &aJapOptionsCB,
                &aJapOptionsBtn,
                &aAttributeBtn,
                &aFormatBtn,
                &aNoFormatBtn,
                &aCalcFL,
                &aCalcSearchInFT,
                &aCalcSearchInLB,
                &aCalcSearchDirFT,
                &aRowsBtn,
                &aColumnsBtn,
                &aAllSheetsCB,
                &aButtonsFL,
                &aHelpBtn,
                &aCloseBtn,
                pMoreBtn,
                0
            };
            sal_Int32 nWindow = 0;
            do
            {
                lcl_MoveDown( *aWindows[nWindow], nOffset );
            }
            while(aWindows[++nWindow]);

            SetSizePixel(aDlgSize);
        }
    }
}

// -----------------------------------------------------------------------

sal_Bool SvxSearchDialog::Close()
{
    // remember strings speichern
    if (!aSearchStrings.empty())
        StrArrToList_Impl( SID_SEARCHDLG_SEARCHSTRINGS, aSearchStrings );

    if (!aReplaceStrings.empty())
        StrArrToList_Impl( SID_SEARCHDLG_REPLACESTRINGS, aReplaceStrings );

    // save settings to configuration
    SvtSearchOptions aOpt;
    aOpt.SetWholeWordsOnly          ( aWordBtn                .IsChecked() );
    aOpt.SetBackwards               ( aBackwardsBtn           .IsChecked() );
    aOpt.SetUseRegularExpression    ( aRegExpBtn              .IsChecked() );
    aOpt.SetSearchForStyles         ( aLayoutBtn              .IsChecked() );
    aOpt.SetSimilaritySearch        ( aSimilarityBox          .IsChecked() );
    aOpt.SetUseAsianOptions         ( aJapOptionsCB           .IsChecked() );
        aOpt.SetNotes                   ( aNotesBtn               .IsChecked() );

    const SfxPoolItem* ppArgs[] = { pSearchItem, 0 };
    rBindings.GetDispatcher()->Execute( FID_SEARCH_OFF, SFX_CALLMODE_SLOT, ppArgs );
    rBindings.Execute( SID_SEARCH_DLG );

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Int32 SvxSearchDialog::GetTransliterationFlags() const
{
    if (!aMatchCaseCB.IsChecked())
        nTransliterationFlags |=  TransliterationModules_IGNORE_CASE;
    else
        nTransliterationFlags &= ~TransliterationModules_IGNORE_CASE;
    if ( !aJapMatchFullHalfWidthCB.IsChecked())
        nTransliterationFlags |=  TransliterationModules_IGNORE_WIDTH;
    else
        nTransliterationFlags &= ~TransliterationModules_IGNORE_WIDTH;
    return nTransliterationFlags;
}

void SvxSearchDialog::SetSaveToModule(bool b)
{
    pImpl->bSaveToModule = b;
}

// -----------------------------------------------------------------------

void SvxSearchDialog::ApplyTransliterationFlags_Impl( sal_Int32 nSettings )
{
    nTransliterationFlags = nSettings;
    sal_Bool bVal = 0 != (nSettings & TransliterationModules_IGNORE_CASE);
    aMatchCaseCB            .Check(!bVal );
    bVal = 0 != (nSettings & TransliterationModules_IGNORE_WIDTH);
    aJapMatchFullHalfWidthCB.Check( !bVal );
}

// -----------------------------------------------------------------------

void SvxSearchDialog::Activate()
{
    // apply possible transliteration changes of the SvxSearchItem member
    DBG_ASSERT( pSearchItem, "SearchItem missing" );
    if (pSearchItem)
    {
        aMatchCaseCB            .Check( pSearchItem->GetExact() );
        aJapMatchFullHalfWidthCB.Check( !pSearchItem->IsMatchFullHalfWidthForms() );
    }
}

// -----------------------------------------------------------------------

void SvxSearchDialog::InitControls_Impl()
{
    // CaseSensitives AutoComplete
    aSearchLB.EnableAutocomplete( sal_True, sal_True );
    aSearchLB.Show();
    aReplaceLB.EnableAutocomplete( sal_True, sal_True );
    aReplaceLB.Show();

    aFormatBtn.Disable();
    aAttributeBtn.Disable();

    aSearchLB.SetModifyHdl( LINK( this, SvxSearchDialog, ModifyHdl_Impl ) );
    aReplaceLB.SetModifyHdl( LINK( this, SvxSearchDialog, ModifyHdl_Impl ) );

    Link aLink = LINK( this, SvxSearchDialog, FocusHdl_Impl );
    aSearchLB.SetGetFocusHdl( aLink );
    pImpl->aSearchFormats.SetGetFocusHdl( aLink );

    aReplaceLB.SetGetFocusHdl( aLink );
    pImpl->aReplaceFormats.SetGetFocusHdl( aLink );

    aLink = LINK( this, SvxSearchDialog, LoseFocusHdl_Impl );
    aSearchLB.SetLoseFocusHdl( aLink );
    aReplaceLB.SetLoseFocusHdl( aLink );

    aSearchTmplLB.SetLoseFocusHdl( aLink );
    aReplaceTmplLB.SetLoseFocusHdl( aLink );

    aLink = LINK( this, SvxSearchDialog, CommandHdl_Impl );
    aSearchBtn.SetClickHdl( aLink );
    aSearchAllBtn.SetClickHdl( aLink );
    aReplaceBtn.SetClickHdl( aLink );
    aReplaceAllBtn.SetClickHdl( aLink );
    aCloseBtn.SetClickHdl( aLink );
    aSimilarityBtn.SetClickHdl( aLink );
    aJapOptionsBtn.SetClickHdl( aLink );
    aSearchComponent1PB.SetClickHdl( aLink );
    aSearchComponent2PB.SetClickHdl( aLink );

    aLink = LINK( this, SvxSearchDialog, FlagHdl_Impl );
    aWordBtn.SetClickHdl( aLink );
    aSelectionBtn.SetClickHdl( aLink );
    aMatchCaseCB.SetClickHdl( aLink );
    aRegExpBtn.SetClickHdl( aLink );
    aBackwardsBtn.SetClickHdl( aLink );
    aNotesBtn.SetClickHdl( aLink );
    aSimilarityBox.SetClickHdl( aLink );
    aJapOptionsCB.SetClickHdl( aLink );
    aJapMatchFullHalfWidthCB.SetClickHdl( aLink );

    aLayoutBtn.SetClickHdl( LINK( this, SvxSearchDialog, TemplateHdl_Impl ) );
    aFormatBtn.SetClickHdl( LINK( this, SvxSearchDialog, FormatHdl_Impl ) );
    aNoFormatBtn.SetClickHdl(
        LINK( this, SvxSearchDialog, NoFormatHdl_Impl ) );
    aAttributeBtn.SetClickHdl(
        LINK( this, SvxSearchDialog, AttributeHdl_Impl ) );

    // check if buttontext is to wide
    long nTxtW = Max( pMoreBtn->GetCtrlTextWidth( pMoreBtn->GetMoreText() ),
                      pMoreBtn->GetCtrlTextWidth( pMoreBtn->GetLessText() ) );
    nTxtW += ( pMoreBtn->GetTextHeight() * 2 ); // add image size + offset
    long nBtnW = pMoreBtn->GetSizePixel().Width();
    if ( nTxtW > nBtnW )
    {
        // broaden the button
        const long nMinDelta = 10;
        long nDelta = Max( nTxtW - nBtnW, nMinDelta );
        Size aNewSize = pMoreBtn->GetSizePixel();
        aNewSize.Width() += nDelta;
        pMoreBtn->SetSizePixel( aNewSize );
    }
}

// -----------------------------------------------------------------------

void SvxSearchDialog::CalculateDelta_Impl()
{
    DBG_ASSERT( pSearchItem, "no search item" );

    bool bDrawApp = false;
    bool bCalcApp = false;
    bool bWriterApp = false;
    bool bImpressApp = false;
    const uno::Reference< frame::XFrame > xFrame = rBindings.GetActiveFrame();
    uno::Reference< frame::XModuleManager2 > xModuleManager( frame::ModuleManager::create(::comphelper::getProcessComponentContext()) );
    try
    {
        ::rtl::OUString aModuleIdentifier = xModuleManager->identify( xFrame );
        bCalcApp = aModuleIdentifier == "com.sun.star.sheet.SpreadsheetDocument";
        bDrawApp = aModuleIdentifier == "com.sun.star.drawing.DrawingDocument";
        bImpressApp = aModuleIdentifier == "com.sun.star.presentation.PresentationDocument";
        bWriterApp = aModuleIdentifier == "com.sun.star.text.TextDocument";
    }
    catch ( uno::Exception& )
    {
    }

    if ( pImpl->bDeltaCalculated )
        return;
    else
        pImpl->bDeltaCalculated = sal_True;

    sal_uIntPtr nDelta = 187, nOffset = 0;
    SvtCJKOptions aCJKOptions;

    pMoreBtn->AddWindow( &aOptionsFL );
    if ( !bDrawApp )
        pMoreBtn->AddWindow( &aLayoutBtn );
    if ( bWriterApp )
        pMoreBtn->AddWindow( &aNotesBtn );
    else
    {
        aNotesBtn.Hide();
        nOffset = !bDrawApp ? 13 : 0;
    }
    pMoreBtn->AddWindow( &aBackwardsBtn );
    if ( !bDrawApp )
        pMoreBtn->AddWindow( &aRegExpBtn );
    pMoreBtn->AddWindow( &aSimilarityBox );
    pMoreBtn->AddWindow( &aSimilarityBtn );
    pMoreBtn->AddWindow( &aSelectionBtn );

    if ( aCJKOptions.IsCJKFontEnabled() )
        pMoreBtn->AddWindow( &aJapMatchFullHalfWidthCB );
    else
        nOffset += 13;
    if ( aCJKOptions.IsJapaneseFindEnabled() )
    {
        pMoreBtn->AddWindow( &aJapOptionsCB );
        pMoreBtn->AddWindow( &aJapOptionsBtn );
    }
    else
        nOffset += 17;

    if ( bWriter )
    {
        pMoreBtn->AddWindow( &aAttributeBtn );
        pMoreBtn->AddWindow( &aFormatBtn );
        pMoreBtn->AddWindow( &aNoFormatBtn );
    }

    if (bDrawApp || bImpressApp)
    {
        // "Find All" button is hidden--align "Find" vertically to the
        // search listbox
        Point aNewPt(aSearchBtn.GetPosPixel());
        const Size aBtnSz(aSearchBtn.GetSizePixel());
        const Size aLBSz(aSearchLB.GetSizePixel());
        const int nOff((aLBSz.Height() - aBtnSz.Height()) / 2);
        aNewPt.Y() = aSearchLB.GetPosPixel().Y() + nOff;
        aSearchBtn.SetPosPixel(aNewPt);
    }

    if ( bDrawApp )
    {
        // Draw App: "Regular expressions" and "Search for Styles" check boxes are hidden
        // so align the other buttons
        const long nAppFontHeight = 13; // checkbox height + space between in APPFONT
        long nH = LogicToPixel( Size( 0, nAppFontHeight ), MAP_APPFONT ).Height();

        Point aNewPos = aSimilarityBox.GetPosPixel();
        aNewPos.Y() -= nH;
        aSimilarityBox.SetPosPixel( aNewPos );
        aNewPos = aSimilarityBtn.GetPosPixel();
        aNewPos.Y() -= nH;
        aSimilarityBtn.SetPosPixel( aNewPos );
        nH *= 3;
        nOffset += ( 3 * nAppFontHeight );
        if ( aCJKOptions.IsCJKFontEnabled() )
        {
            aNewPos = aJapMatchFullHalfWidthCB.GetPosPixel();
            aNewPos.Y() -= nH;
            aJapMatchFullHalfWidthCB.SetPosPixel( aNewPos );
        }
        if ( aCJKOptions.IsJapaneseFindEnabled() )
        {
            aNewPos = aJapOptionsCB.GetPosPixel();
            aNewPos.Y() -= nH;
            aJapOptionsCB.SetPosPixel( aNewPos );
            aNewPos = aJapOptionsBtn.GetPosPixel();
            aNewPos.Y() -= nH;
            aJapOptionsBtn.SetPosPixel( aNewPos );
        }
    }

    if ( bCalcApp || bImpressApp )
    {
        Window* pWins[] =
        {
            &aCalcFL, &aCalcSearchInFT, &aCalcSearchInLB, &aCalcSearchDirFT,
            &aRowsBtn, &aColumnsBtn, &aAllSheetsCB, &aJapMatchFullHalfWidthCB,
            &aJapOptionsCB, &aJapOptionsBtn
        };
        Window** pCurrent = pWins;
        sal_uInt32 i = 0;
        const sal_uInt32 nCalcCtrlCount = 7;
        if ( nOffset > 0 )
        {
            long nH = LogicToPixel( Size( 0, nOffset ), MAP_APPFONT ).Height();
            for ( i = 0; i < sizeof( pWins ) / sizeof( pWins[ 0 ] ); ++i, ++pCurrent )
            {
                if ( ( bCalcApp && i < nCalcCtrlCount )
                    || ( i == nCalcCtrlCount && aCJKOptions.IsCJKFontEnabled() )
                    || ( i > nCalcCtrlCount && aCJKOptions.IsJapaneseFindEnabled() ) )
                {
                    Point aNewPos = (*pCurrent)->GetPosPixel();
                    aNewPos.Y() -= nH;
                    (*pCurrent)->SetPosPixel( aNewPos );
                }
            }
        }

        if ( bCalcApp)
        {
            pCurrent = pWins;
            for ( i = 0; i < nCalcCtrlCount; ++i, ++pCurrent )
                pMoreBtn->AddWindow( *pCurrent );
        }
        else
            nOffset += 64;
    }
    else
        nOffset += 64;

    pMoreBtn->SetDelta( nDelta - nOffset );
    pMoreBtn->Show();
    pMoreBtn->Enable();
}

// -----------------------------------------------------------------------

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

void SvxSearchDialog::Init_Impl( int bSearchPattern )
{
    DBG_ASSERT( pSearchItem, "SearchItem == 0" );

    // We don't want to save any intermediate state to the module while the
    // dialog is being initialized.
    ToggleSaveToModule aNoModuleSave(*this, false);

    bWriter = ( pSearchItem->GetAppFlag() == SVX_SEARCHAPP_WRITER );

    pImpl->bMultiLineEdit = sal_False;

    if ( !pImpl->bMultiLineEdit )
    {
        pImpl->aSearchFormats.Hide();
        aSearchAttrText.Show();
        pImpl->aReplaceFormats.Hide();
        aReplaceAttrText.Show();
    }
    else
    {
        String aText = aSearchAttrText.GetText();
        aSearchAttrText.Hide();

        if ( aText.Len() )
            pImpl->aSearchFormats.SetText( aText );
        pImpl->aSearchFormats.Show();
        aText = aReplaceAttrText.GetText();
        aReplaceAttrText.Hide();

        if ( aText.Len() )
            pImpl->aReplaceFormats.SetText( aText );
        pImpl->aReplaceFormats.Show();
    }

    if ( ( nModifyFlag & MODIFY_WORD ) == 0 )
         aWordBtn.Check( pSearchItem->GetWordOnly() );
    if ( ( nModifyFlag & MODIFY_EXACT ) == 0 )
        aMatchCaseCB.Check( pSearchItem->GetExact() );
    if ( ( nModifyFlag & MODIFY_BACKWARDS ) == 0 )
        aBackwardsBtn.Check( pSearchItem->GetBackward() );
    if ( ( nModifyFlag & MODIFY_NOTES ) == 0 )
        aNotesBtn.Check( pSearchItem->GetNotes() );
    if ( ( nModifyFlag & MODIFY_SELECTION ) == 0 )
        aSelectionBtn.Check( pSearchItem->GetSelection() );
    if ( ( nModifyFlag & MODIFY_REGEXP ) == 0 )
        aRegExpBtn.Check( pSearchItem->GetRegExp() );
    if ( ( nModifyFlag & MODIFY_LAYOUT ) == 0 )
        aLayoutBtn.Check( pSearchItem->GetPattern() );
    if (aNotesBtn.IsChecked())
        aLayoutBtn.Disable();
    aSimilarityBox.Check( pSearchItem->IsLevenshtein() );
    if( aJapOptionsCB.IsVisible() )
        aJapOptionsCB.Check( pSearchItem->IsUseAsianOptions() );
    ApplyTransliterationFlags_Impl( pSearchItem->GetTransliterationFlags() );

    CalculateDelta_Impl();

    bool bDraw = false;
    if ( pSearchItem->GetAppFlag() == SVX_SEARCHAPP_CALC )
    {
        Link aLink = LINK( this, SvxSearchDialog, FlagHdl_Impl );
        aCalcSearchInLB.SetSelectHdl( aLink );
        aRowsBtn.SetClickHdl( aLink );
        aColumnsBtn.SetClickHdl( aLink );
        aAllSheetsCB.SetClickHdl( aLink );

        switch ( pSearchItem->GetCellType() )
        {
            case SVX_SEARCHIN_FORMULA:
                if ( ( nModifyFlag & MODIFY_FORMULAS ) == 0 )
                    aCalcSearchInLB.SelectEntryPos( SVX_SEARCHIN_FORMULA );
                break;

            case SVX_SEARCHIN_VALUE:
                if ( ( nModifyFlag & MODIFY_VALUES ) == 0 )
                    aCalcSearchInLB.SelectEntryPos( SVX_SEARCHIN_VALUE );
                break;

            case SVX_SEARCHIN_NOTE:
                if ( ( nModifyFlag & MODIFY_CALC_NOTES ) == 0 )
                    aCalcSearchInLB.SelectEntryPos( SVX_SEARCHIN_NOTE );
                break;
        }
        aWordBtn.SetText( aCalcStr.GetToken( 0, '#' ) );

        if ( pSearchItem->GetRowDirection() &&
             ( nModifyFlag & MODIFY_ROWS ) == 0 )
            aRowsBtn.Check();
        else if ( !pSearchItem->GetRowDirection() &&
                  ( nModifyFlag & MODIFY_COLUMNS ) == 0 )
            aColumnsBtn.Check();

        if ( ( nModifyFlag & MODIFY_ALLTABLES ) == 0 )
            aAllSheetsCB.Check( pSearchItem->IsAllTables() );

        // only look for formatting in Writer
        aFormatBtn.Hide();
        aNoFormatBtn.Hide();
        aAttributeBtn.Hide();
    }
    else
    {
        aWordBtn.SetText( aCalcStr.GetToken( 1, '#' ) );

        if ( pSearchItem->GetAppFlag() == SVX_SEARCHAPP_DRAW )
        {
            aSearchAllBtn.Hide();

            aRegExpBtn.Hide();
            aLayoutBtn.Hide();

            // only look for formatting in Writer
            aFormatBtn.Hide();
            aNoFormatBtn.Hide();
            aAttributeBtn.Hide();
            bDraw = sal_True;
        }
        else
        {
            if ( !pSearchList )
            {
                // Get attribute sets, if it not has been done already
                const SfxPoolItem* ppArgs[] = { pSearchItem, 0 };
                const SvxSetItem* pSrchSetItem =
                (const SvxSetItem*)rBindings.GetDispatcher()->Execute( FID_SEARCH_SEARCHSET, SFX_CALLMODE_SLOT, ppArgs );

                if ( pSrchSetItem )
                    InitAttrList_Impl( &pSrchSetItem->GetItemSet(), 0 );

                const SvxSetItem* pReplSetItem =
                (const SvxSetItem*)rBindings.GetDispatcher()->Execute( FID_SEARCH_REPLACESET, SFX_CALLMODE_SLOT, ppArgs );

                if ( pReplSetItem )
                    InitAttrList_Impl( 0, &pReplSetItem->GetItemSet() );
            }
        }
    }

    if ( 0 && !bDraw ) //!!!!!
    {
        aRegExpBtn.Show();
        aLayoutBtn.Show();
    }

    // similarity search?
    if ( ( nModifyFlag & MODIFY_SIMILARITY ) == 0 )
        aSimilarityBox.Check( pSearchItem->IsLevenshtein() );
    bSet = sal_True;

    FlagHdl_Impl( &aSimilarityBox );
    FlagHdl_Impl( &aJapOptionsCB );

    bool bDisableSearch = false;
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if ( pViewShell )
    {
        sal_Bool bText = !bSearchPattern;

        if ( pViewShell->HasSelection( bText ) )
            EnableControl_Impl( &aSelectionBtn );
        else
        {
            aSelectionBtn.Check( sal_False );
            aSelectionBtn.Disable();
        }
    }

    // Pattern Search and there were no AttrSets given
    if ( bSearchPattern )
    {
        SfxObjectShell* pShell = SfxObjectShell::Current();

        if ( pShell && pShell->GetStyleSheetPool() )
        {
            // Templates designed
            aSearchTmplLB .Clear();
            aReplaceTmplLB.Clear();
            SfxStyleSheetBasePool* pStylePool = pShell->GetStyleSheetPool();
            pStylePool->SetSearchMask( pSearchItem->GetFamily(),
                                       SFXSTYLEBIT_ALL );
            SfxStyleSheetBase* pBase = pStylePool->First();

            while ( pBase )
            {
                if ( pBase->IsUsed() )
                    aSearchTmplLB.InsertEntry( pBase->GetName() );
                aReplaceTmplLB.InsertEntry( pBase->GetName() );
                pBase = pStylePool->Next();
            }
            aSearchTmplLB.SelectEntry( pSearchItem->GetSearchString() );
            aReplaceTmplLB.SelectEntry( pSearchItem->GetReplaceString() );

        }
        aSearchTmplLB.Show();

        if ( bConstruct )
            // Grab focus only after creating
            aSearchTmplLB.GrabFocus();
        aReplaceTmplLB.Show();
        aSearchLB.Hide();
        aReplaceLB.Hide();

        aWordBtn.Disable();
        aRegExpBtn.Disable();
        aMatchCaseCB.Disable();

        bDisableSearch = !aSearchTmplLB.GetEntryCount();
    }
    else
    {
        bool bSetSearch = ( ( nModifyFlag & MODIFY_SEARCH ) == 0 );
        bool bSetReplace = ( ( nModifyFlag & MODIFY_REPLACE ) == 0 );

        if ( pSearchItem->GetSearchString().Len() && bSetSearch )
            aSearchLB.SetText( pSearchItem->GetSearchString() );
        else if (!aSearchStrings.empty())
        {
            bool bAttributes =
                ( ( pSearchList && pSearchList->Count() ) ||
                  ( pReplaceList && pReplaceList->Count() ) );

            if ( bSetSearch && !bAttributes )
                aSearchLB.SetText(aSearchStrings[0]);

            String aReplaceTxt = pSearchItem->GetReplaceString();

            if (!aReplaceStrings.empty())
                aReplaceTxt = aReplaceStrings[0];

            if ( bSetReplace && !bAttributes )
                aReplaceLB.SetText( aReplaceTxt );
        }
        aSearchLB.Show();

        if ( bConstruct )
            // Grab focus only after creating
            aSearchLB.GrabFocus();
        aReplaceLB.Show();
        aSearchTmplLB.Hide();
        aReplaceTmplLB.Hide();

        EnableControl_Impl( &aRegExpBtn );
        EnableControl_Impl( &aMatchCaseCB );

        if ( aRegExpBtn.IsChecked() )
            aWordBtn.Disable();
        else
            EnableControl_Impl( &aWordBtn );

        String aSrchAttrTxt;

        if ( pImpl->bMultiLineEdit )
            aSrchAttrTxt = pImpl->aSearchFormats.GetText();
        else
            aSrchAttrTxt = aSearchAttrText.GetText();

        bDisableSearch = !aSearchLB.GetText().Len() && !aSrchAttrTxt.Len();
    }
    FocusHdl_Impl( &aSearchLB );

    if ( bDisableSearch )
    {
        aSearchBtn.Disable();
        aSearchAllBtn.Disable();
        aReplaceBtn.Disable();
        aReplaceAllBtn.Disable();
        aSearchComponentFL.Enable(sal_False);
        aSearchComponent1PB.Enable(sal_False);
        aSearchComponent2PB.Enable(sal_False);
    }
    else
    {
        EnableControl_Impl( &aSearchBtn );
        EnableControl_Impl( &aReplaceBtn );
        if (!bWriter || (bWriter && !aNotesBtn.IsChecked()))
        {
            EnableControl_Impl( &aSearchAllBtn );
            EnableControl_Impl( &aReplaceAllBtn );
        }
        if (bWriter && pSearchItem->GetNotes())
        {
            aSearchAllBtn.Disable();
            aReplaceAllBtn.Disable();
        }
    }

    if ( ( !pImpl->bMultiLineEdit && aSearchAttrText.GetText().Len() ) ||
            ( pImpl->bMultiLineEdit && pImpl->aSearchFormats.GetText().Len() ) )
        EnableControl_Impl( &aNoFormatBtn );
    else
        aNoFormatBtn.Disable();

    if ( !pSearchList )
    {
        aAttributeBtn.Disable();
        aFormatBtn.Disable();
    }

    if ( aLayoutBtn.IsChecked() )
    {
        pImpl->bSaveToModule = sal_False;
        TemplateHdl_Impl( &aLayoutBtn );
        pImpl->bSaveToModule = sal_True;
    }
}

// -----------------------------------------------------------------------

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
        pImpl->pRanges = new sal_uInt16[nCnt];
        memcpy( pImpl->pRanges, pTmp, sizeof(sal_uInt16) * nCnt );
    }

    // See to it that are the texts of the attributes are correct
    String aDesc;

    if ( pSSet )
    {
        delete pSearchList;
        pSearchList = new SearchAttrItemList;

        if ( pSSet->Count() )
        {
            pSearchList->Put( *pSSet );

            if ( !pImpl->bMultiLineEdit )
                aSearchAttrText.SetText( BuildAttrText_Impl( aDesc, sal_True ) );
            else
                pImpl->aSearchFormats.SetText( BuildAttrText_Impl( aDesc, sal_True ) );

            if ( aDesc.Len() )
                bFormat |= sal_True;
        }
    }

    if ( pRSet )
    {
        delete pReplaceList;
        pReplaceList = new SearchAttrItemList;

        if ( pRSet->Count() )
        {
            pReplaceList->Put( *pRSet );

            if ( !pImpl->bMultiLineEdit )
                aReplaceAttrText.SetText( BuildAttrText_Impl( aDesc, sal_False ) );
            else
                pImpl->aReplaceFormats.SetText( BuildAttrText_Impl( aDesc, sal_False ) );

            if ( aDesc.Len() )
                bFormat |= sal_True;
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, FlagHdl_Impl, Control *, pCtrl )
{
    if ( pCtrl && !bSet )
        SetModifyFlag_Impl( pCtrl );
    else
        bSet = sal_False;

    if ( pCtrl == &aSimilarityBox )
    {
        sal_Bool bIsChecked = aSimilarityBox.IsChecked();

        if ( bIsChecked )
        {
            aSimilarityBtn.Enable();
            aRegExpBtn.Check( sal_False );
            aRegExpBtn.Disable();
            EnableControl_Impl( &aWordBtn );

            if ( aLayoutBtn.IsChecked() )
            {
                EnableControl_Impl( &aMatchCaseCB );
                aLayoutBtn.Check( sal_False );
            }
            aRegExpBtn.Disable();
            aLayoutBtn.Disable();
            aFormatBtn.Disable();
            aNoFormatBtn.Disable();
            aAttributeBtn.Disable();
        }
        else
        {
            EnableControl_Impl( &aRegExpBtn );
            if (!aNotesBtn.IsChecked())
                EnableControl_Impl( &aLayoutBtn );
            EnableControl_Impl( &aFormatBtn );
            EnableControl_Impl( &aAttributeBtn );
            aSimilarityBtn.Disable();
        }
        pSearchItem->SetLevenshtein( bIsChecked );
    }
    else
    if ( pCtrl == &aNotesBtn)
    {
        if (aNotesBtn.IsChecked())
        {
            aLayoutBtn.Disable();
            aSearchAllBtn.Disable();
            aReplaceAllBtn.Disable();
        }
        else
        {
            EnableControl_Impl( &aLayoutBtn );
            ModifyHdl_Impl( &aSearchLB );
        }
    }
    else
    {
        if ( aLayoutBtn.IsChecked() && !bFormat )
        {
            aWordBtn.Check( sal_False );
            aWordBtn.Disable();
            aRegExpBtn.Check( sal_False );
            aRegExpBtn.Disable();
            aMatchCaseCB.Check( sal_False );
            aMatchCaseCB.Disable();
            aNotesBtn.Disable();

            if ( aSearchTmplLB.GetEntryCount() )
            {
                EnableControl_Impl( &aSearchBtn );
                EnableControl_Impl( &aSearchAllBtn );
                EnableControl_Impl( &aReplaceBtn );
                EnableControl_Impl( &aReplaceAllBtn );
            }
        }
        else
        {
            EnableControl_Impl( &aRegExpBtn );
            EnableControl_Impl( &aMatchCaseCB );
            EnableControl_Impl( &aNotesBtn );

            if ( aRegExpBtn.IsChecked() )
            {
                aWordBtn.Check( sal_False );
                aWordBtn.Disable();
                aSimilarityBox.Disable();
                aSimilarityBtn.Disable();
            }
            else
            {
                EnableControl_Impl( &aWordBtn );
                EnableControl_Impl( &aSimilarityBox );
            }

            // Search-string in place? then enable Buttons
            bSet = sal_True;
            ModifyHdl_Impl( &aSearchLB );
        }
    }

    if ( &aAllSheetsCB == pCtrl )
    {
        if ( aAllSheetsCB.IsChecked() )
            aSearchAllBtn.Disable();
        else
        {
            bSet = sal_True;
            ModifyHdl_Impl( &aSearchLB );
        }
    }

    if ( &aJapOptionsCB == pCtrl )
    {
        sal_Bool bEnableJapOpt = aJapOptionsCB.IsChecked();
        aMatchCaseCB            .Enable(!bEnableJapOpt );
        aJapMatchFullHalfWidthCB.Enable(!bEnableJapOpt );
        aJapOptionsBtn          .Enable( bEnableJapOpt );
    }

    if ( pImpl->bSaveToModule )
        SaveToModule_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, CommandHdl_Impl, Button *, pBtn )
{
    bool bInclusive = ( aLayoutBtn.GetText() == aLayoutStr );

    if ( ( pBtn == &aSearchBtn )    ||
         ( pBtn == &aSearchAllBtn ) ||
         ( pBtn == &aReplaceBtn )   ||
         ( pBtn == &aReplaceAllBtn ) )
    {
        if ( aLayoutBtn.IsChecked() && !bInclusive )
        {
            pSearchItem->SetSearchString ( aSearchTmplLB.GetSelectEntry() );
            pSearchItem->SetReplaceString( aReplaceTmplLB.GetSelectEntry() );
        }
        else
        {
            pSearchItem->SetSearchString ( aSearchLB.GetText() );
            pSearchItem->SetReplaceString( aReplaceLB.GetText() );

            if ( pBtn == &aReplaceBtn )
                Remember_Impl( aReplaceLB.GetText(), sal_False );
            else
            {
                Remember_Impl( aSearchLB.GetText(), sal_True );

                if ( pBtn == &aReplaceAllBtn )
                    Remember_Impl( aReplaceLB.GetText(), sal_False );
            }
        }

        pSearchItem->SetRegExp( sal_False );
        pSearchItem->SetLevenshtein( sal_False );
        if (GetCheckBoxValue( aRegExpBtn ))
            pSearchItem->SetRegExp( sal_True );
        else if (GetCheckBoxValue( aSimilarityBox ))
            pSearchItem->SetLevenshtein( sal_True );

        pSearchItem->SetWordOnly( GetCheckBoxValue( aWordBtn ) );
        pSearchItem->SetBackward( GetCheckBoxValue( aBackwardsBtn ) );
        pSearchItem->SetNotes( GetCheckBoxValue( aNotesBtn ) );
        pSearchItem->SetPattern( GetCheckBoxValue( aLayoutBtn ) );
        pSearchItem->SetSelection( GetCheckBoxValue( aSelectionBtn ) );

        pSearchItem->SetUseAsianOptions( GetCheckBoxValue( aJapOptionsCB ) );
        sal_Int32 nFlags = GetTransliterationFlags();
        if( !pSearchItem->IsUseAsianOptions())
            nFlags &= (TransliterationModules_IGNORE_CASE |
                       TransliterationModules_IGNORE_WIDTH );
        pSearchItem->SetTransliterationFlags( nFlags );

        if ( !bWriter )
        {
            if ( aCalcSearchInLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
                pSearchItem->SetCellType( aCalcSearchInLB.GetSelectEntryPos() );

            pSearchItem->SetRowDirection( aRowsBtn.IsChecked() );
            pSearchItem->SetAllTables( aAllSheetsCB.IsChecked() );
        }

        if ( pBtn == &aSearchBtn )
            pSearchItem->SetCommand( SVX_SEARCHCMD_FIND );
        else if ( pBtn == &aSearchAllBtn )
            pSearchItem->SetCommand( SVX_SEARCHCMD_FIND_ALL );
        else if ( pBtn == &aReplaceBtn )
            pSearchItem->SetCommand( SVX_SEARCHCMD_REPLACE );
        else if ( pBtn == &aReplaceAllBtn )
            pSearchItem->SetCommand( SVX_SEARCHCMD_REPLACE_ALL );

        // when looking for templates, delete format lists
        if ( !bFormat && pSearchItem->GetPattern() )
        {
            if ( pSearchList )
                pSearchList->Clear();

            if ( pReplaceList )
                pReplaceList->Clear();
        }
        nModifyFlag = 0;
        const SfxPoolItem* ppArgs[] = { pSearchItem, 0 };
        rBindings.ExecuteSynchron( FID_SEARCH_NOW, ppArgs, 0L );
    }
    else if ( pBtn == &aCloseBtn )
    {
        if ( !aLayoutBtn.IsChecked() || bInclusive )
        {
            String aStr( aSearchLB.GetText() );

            if ( aStr.Len() )
                Remember_Impl( aStr, sal_True );
            aStr = aReplaceLB.GetText();

            if ( aStr.Len() )
                Remember_Impl( aStr, sal_False );
        }
        SaveToModule_Impl();
        Close();
    }
    else if ( pBtn == &aSimilarityBtn )
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            AbstractSvxSearchSimilarityDialog* pDlg = pFact->CreateSvxSearchSimilarityDialog( this,
                                                                        pSearchItem->IsLEVRelaxed(),
                                                                        pSearchItem->GetLEVOther(),
                                                                        pSearchItem->GetLEVShorter(),
                                                                        pSearchItem->GetLEVLonger() );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            if ( pDlg && pDlg->Execute() == RET_OK )
            {
                pSearchItem->SetLEVRelaxed( pDlg->IsRelaxed() );
                pSearchItem->SetLEVOther( pDlg->GetOther() );
                pSearchItem->SetLEVShorter( pDlg->GetShorter() );
                pSearchItem->SetLEVLonger( pDlg->GetLonger() );
                SaveToModule_Impl();
            }
            delete pDlg;
        }
    }
    else if ( pBtn == &aJapOptionsBtn )
    {
        SfxItemSet aSet( SFX_APP()->GetPool() );
        pSearchItem->SetTransliterationFlags( GetTransliterationFlags() );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            AbstractSvxJSearchOptionsDialog* aDlg = pFact->CreateSvxJSearchOptionsDialog( this, aSet,
                    pSearchItem->GetTransliterationFlags() );
            DBG_ASSERT(aDlg, "Dialogdiet fail!");
            int nRet = aDlg->Execute();
            if (RET_OK == nRet) //! true only if FillItemSet of SvxJSearchOptionsPage returns true
            {
                sal_Int32 nFlags = aDlg->GetTransliterationFlags();
                pSearchItem->SetTransliterationFlags( nFlags );
                ApplyTransliterationFlags_Impl( nFlags );
            }
            delete aDlg;
        }
    }
    else if(pBtn == &aSearchComponent1PB || pBtn == &aSearchComponent2PB )
    {
        uno::Sequence < beans::PropertyValue > aArgs(2);
        beans::PropertyValue* pArgs = aArgs.getArray();
        pArgs[0].Name = ::rtl::OUString("SearchString");
        pArgs[0].Value <<= ::rtl::OUString(aSearchLB.GetText());
        pArgs[1].Name = ::rtl::OUString("ParentWindow");
        pArgs[1].Value <<= VCLUnoHelper::GetInterface( this );
        if(pBtn == &aSearchComponent1PB)
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

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, ModifyHdl_Impl, ComboBox *, pEd )
{
    if ( !bSet )
        SetModifyFlag_Impl( pEd );
    else
        bSet = sal_False;

    // Calc allows searching for empty cells.
    bool bAllowEmptySearch = (pSearchItem->GetAppFlag() == SVX_SEARCHAPP_CALC);

    if ( pEd == &aSearchLB || pEd == &aReplaceLB )
    {
        xub_StrLen nSrchTxtLen = aSearchLB.GetText().Len();
        xub_StrLen nReplTxtLen = 0;
        if (bAllowEmptySearch)
            nReplTxtLen = aReplaceLB.GetText().Len();
        xub_StrLen nAttrTxtLen = 0;

        if ( !pImpl->bMultiLineEdit )
           nAttrTxtLen = aSearchAttrText.GetText().Len();
        else
            nAttrTxtLen = pImpl->aSearchFormats.GetText().Len();

        if (nSrchTxtLen || nReplTxtLen || nAttrTxtLen)
        {
            EnableControl_Impl( &aSearchBtn );
            EnableControl_Impl( &aReplaceBtn );
            if (!bWriter || (bWriter && !aNotesBtn.IsChecked()))
            {
                EnableControl_Impl( &aSearchAllBtn );
                EnableControl_Impl( &aReplaceAllBtn );
            }
        }
        else
        {
            aSearchComponentFL.Enable(sal_False);
            aSearchComponent1PB.Enable(sal_False);
            aSearchComponent2PB.Enable(sal_False);
            aSearchBtn.Disable();
            aSearchAllBtn.Disable();
            aReplaceBtn.Disable();
            aReplaceAllBtn.Disable();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxSearchDialog, TemplateHdl_Impl)
{
    if ( pImpl->bSaveToModule )
        SaveToModule_Impl();

    if ( bFormat )
        return 0;
    String sDesc;

    if ( aLayoutBtn.IsChecked() )
    {
        if ( !pFamilyController )
        {
            sal_uInt16 nId = 0;

            // Enable templates controller
            switch ( pSearchItem->GetFamily() )
            {
                case SFX_STYLE_FAMILY_CHAR:
                    nId = SID_STYLE_FAMILY1; break;

                case SFX_STYLE_FAMILY_PARA:
                    nId = SID_STYLE_FAMILY2; break;

                case SFX_STYLE_FAMILY_FRAME:
                    nId = SID_STYLE_FAMILY3; break;

                case SFX_STYLE_FAMILY_PAGE:
                    nId = SID_STYLE_FAMILY4; break;

                case SFX_STYLE_FAMILY_ALL:
                    break;

                default:
                    OSL_FAIL( "StyleSheetFamily was changed?" );
            }

            rBindings.EnterRegistrations();
            pFamilyController =
                new SvxSearchController( nId, rBindings, *this );
            rBindings.LeaveRegistrations();
            aSearchTmplLB.Clear();
            aReplaceTmplLB.Clear();

            aSearchTmplLB.Show();
            aReplaceTmplLB.Show();
            aSearchLB.Hide();
            aReplaceLB.Hide();

            if ( !pImpl->bMultiLineEdit )
            {
                aSearchAttrText.SetText( sDesc );
                aReplaceAttrText.SetText( sDesc );
            }
            else
            {
                pImpl->aSearchFormats.SetText( sDesc );
                pImpl->aReplaceFormats.SetText( sDesc );
            }
        }
        aFormatBtn.Disable();
        aNoFormatBtn.Disable();
        aAttributeBtn.Disable();
        aSimilarityBox.Disable();
        aSimilarityBtn.Disable();
    }
    else
    {
        // Disable templates controller
        rBindings.EnterRegistrations();
        DELETEZ( pFamilyController );
        rBindings.LeaveRegistrations();

        aSearchLB.Show();
        aReplaceLB.Show();
        aSearchTmplLB.Hide();
        aReplaceTmplLB.Hide();

        if ( !pImpl->bMultiLineEdit )
        {
            aSearchAttrText.SetText( BuildAttrText_Impl( sDesc, sal_True ) );
            aReplaceAttrText.SetText( BuildAttrText_Impl( sDesc, sal_False ) );
        }
        else
        {
            pImpl->aSearchFormats.SetText( BuildAttrText_Impl( sDesc, sal_True ) );
            pImpl->aReplaceFormats.SetText( BuildAttrText_Impl( sDesc, sal_False ) );
        }

        EnableControl_Impl( &aFormatBtn );
        EnableControl_Impl( &aAttributeBtn );
        EnableControl_Impl( &aSimilarityBox );

        FocusHdl_Impl( bSearch ? &aSearchLB : &aReplaceLB );
    }
    bSet = sal_True;
    pImpl->bSaveToModule = sal_False;
    FlagHdl_Impl( &aLayoutBtn );
    pImpl->bSaveToModule = sal_True;
    return 0;
}

// -----------------------------------------------------------------------

void SvxSearchDialog::Remember_Impl( const String &rStr,sal_Bool _bSearch )
{
    if ( !rStr.Len() )
        return;

    std::vector<rtl::OUString>* pArr = _bSearch ? &aSearchStrings : &aReplaceStrings;
    ComboBox* pListBox = _bSearch ? &aSearchLB : &aReplaceLB;

    // ignore identical strings
    for (std::vector<rtl::OUString>::const_iterator i = pArr->begin(); i != pArr->end(); ++i)
    {
        if ((*i).equals(rStr))
            return;
    }

    // delete oldest entry at maximum occupancy (ListBox and Array)
    if(REMEMBER_SIZE < pArr->size())
    {
        pListBox->RemoveEntry( sal_uInt16(REMEMBER_SIZE - 1) );
        (*pArr)[REMEMBER_SIZE - 1] = rStr;
        pArr->erase(pArr->begin() + REMEMBER_SIZE - 1);
    }

    pArr->insert(pArr->begin(), rStr);
    pListBox->InsertEntry(rStr, 0);
}

// -----------------------------------------------------------------------

void SvxSearchDialog::TemplatesChanged_Impl( SfxStyleSheetBasePool& rPool )
{
    String aOldSrch( aSearchTmplLB .GetSelectEntry() );
    String aOldRepl( aReplaceTmplLB.GetSelectEntry() );
    aSearchTmplLB .Clear();
    aReplaceTmplLB.Clear();
    rPool.SetSearchMask( pSearchItem->GetFamily(), SFXSTYLEBIT_ALL );
    aSearchTmplLB.SetUpdateMode( sal_False );
    aReplaceTmplLB.SetUpdateMode( sal_False );
    SfxStyleSheetBase* pBase = rPool.First();

    while ( pBase )
    {
        if ( pBase->IsUsed() )
            aSearchTmplLB.InsertEntry( pBase->GetName() );
        aReplaceTmplLB.InsertEntry( pBase->GetName() );
        pBase = rPool.Next();
    }
    aSearchTmplLB.SetUpdateMode( sal_True );
    aReplaceTmplLB.SetUpdateMode( sal_True );
    aSearchTmplLB.SelectEntryPos(0);

    if ( aOldSrch.Len() )
        aSearchTmplLB .SelectEntry( aOldSrch );
    aReplaceTmplLB.SelectEntryPos(0);

    if ( aOldRepl.Len() )
        aReplaceTmplLB.SelectEntry( aOldRepl );

    if ( aSearchTmplLB.GetEntryCount() )
    {
        EnableControl_Impl( &aSearchBtn );
        EnableControl_Impl( &aSearchAllBtn );
        EnableControl_Impl( &aReplaceBtn );
        EnableControl_Impl( &aReplaceAllBtn );
    }
}

// -----------------------------------------------------------------------

void SvxSearchDialog::EnableControls_Impl( const sal_uInt16 nFlags )
{
    if ( nFlags == nOptions )
        return;
    else
        nOptions = nFlags;

    if ( !nOptions )
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

    sal_Bool bEnableSearch = ( SEARCH_OPTIONS_SEARCH & nOptions ) != 0;
    aSearchBtn.Enable(bEnableSearch);

    if( bEnableSearch )
        bNoSearch = false;


    if ( ( SEARCH_OPTIONS_SEARCH_ALL & nOptions ) != 0 )
    {
        aSearchAllBtn.Enable();
        bNoSearch = false;
    }
    else
        aSearchAllBtn.Disable();
    if ( ( SEARCH_OPTIONS_REPLACE & nOptions ) != 0 )
    {
        aReplaceBtn.Enable();
        aReplaceText.Enable();
        aReplaceLB.Enable();
        aReplaceTmplLB.Enable();
        bNoSearch = false;
    }
    else
    {
        aReplaceBtn.Disable();
        aReplaceText.Disable();
        aReplaceLB.Disable();
        aReplaceTmplLB.Disable();
    }
    if ( ( SEARCH_OPTIONS_REPLACE_ALL & nOptions ) != 0 )
    {
        aReplaceAllBtn.Enable();
        bNoSearch = false;
    }
    else
        aReplaceAllBtn.Disable();
    aSearchComponentFL.Enable(!bNoSearch);
    aSearchComponent1PB.Enable(!bNoSearch);
    aSearchComponent2PB.Enable(!bNoSearch);
    aSearchBtn.Enable( !bNoSearch );
    aSearchText.Enable( !bNoSearch );
    aSearchLB.Enable( !bNoSearch );

    if ( ( SEARCH_OPTIONS_WHOLE_WORDS & nOptions ) != 0 )
        aWordBtn.Enable();
    else
        aWordBtn.Disable();
    if ( ( SEARCH_OPTIONS_BACKWARDS & nOptions ) != 0 )
        aBackwardsBtn.Enable();
    else
        aBackwardsBtn.Disable();
        aNotesBtn.Enable();
    if ( ( SEARCH_OPTIONS_REG_EXP & nOptions ) != 0 )
        aRegExpBtn.Enable();
    else
        aRegExpBtn.Disable();
    if ( ( SEARCH_OPTIONS_EXACT & nOptions ) != 0 )
        aMatchCaseCB.Enable();
    else
        aMatchCaseCB.Disable();
    if ( ( SEARCH_OPTIONS_SELECTION & nOptions ) != 0 )
        aSelectionBtn.Enable();
    else
        aSelectionBtn.Disable();
    if ( ( SEARCH_OPTIONS_FAMILIES & nOptions ) != 0 )
        aLayoutBtn.Enable();
    else
        aLayoutBtn.Disable();
    if ( ( SEARCH_OPTIONS_FORMAT & nOptions ) != 0 )
    {
        aAttributeBtn.Enable();
        aFormatBtn.Enable();
        aNoFormatBtn.Enable();
    }
    else
    {
        aAttributeBtn.Disable();
        aFormatBtn.Disable();
        aNoFormatBtn.Disable();
    }

    if ( ( SEARCH_OPTIONS_SIMILARITY & nOptions ) != 0 )
    {
        aSimilarityBox.Enable();
        aSimilarityBtn.Enable();
    }
    else
    {
        aSimilarityBox.Disable();
        aSimilarityBtn.Disable();
    }

    if ( pSearchItem )
        Init_Impl( pSearchItem->GetPattern() &&
                   ( !pSearchList || !pSearchList->Count() ) );
}

// -----------------------------------------------------------------------

void SvxSearchDialog::EnableControl_Impl( Control* pCtrl )
{
    if ( &aSearchBtn == pCtrl && ( SEARCH_OPTIONS_SEARCH & nOptions ) != 0 )
    {
        aSearchComponentFL.Enable();
        aSearchComponent1PB.Enable();
        aSearchComponent2PB.Enable();
        aSearchBtn.Enable();
        return;
    }
    if ( &aSearchAllBtn == pCtrl &&
         ( SEARCH_OPTIONS_SEARCH_ALL & nOptions ) != 0 )
    {
        aSearchAllBtn.Enable( ( bWriter || !aAllSheetsCB.IsChecked() ) );
        return;
    }
    if ( &aReplaceBtn == pCtrl && ( SEARCH_OPTIONS_REPLACE & nOptions ) != 0 )
    {
        aReplaceBtn.Enable();
        return;
    }
    if ( &aReplaceAllBtn == pCtrl &&
         ( SEARCH_OPTIONS_REPLACE_ALL & nOptions ) != 0 )
    {
        aReplaceAllBtn.Enable();
        return;
    }
    if ( &aWordBtn == pCtrl && ( SEARCH_OPTIONS_WHOLE_WORDS & nOptions ) != 0 )
    {
        aWordBtn.Enable();
        return;
    }
    if ( &aBackwardsBtn == pCtrl && ( SEARCH_OPTIONS_BACKWARDS & nOptions ) != 0 )
    {
        aBackwardsBtn.Enable();
        return;
    }
    if ( &aNotesBtn == pCtrl )
    {
        aNotesBtn.Enable();
        return;
    }
    if ( &aRegExpBtn == pCtrl && ( SEARCH_OPTIONS_REG_EXP & nOptions ) != 0
        && !aSimilarityBox.IsChecked())
    {
        aRegExpBtn.Enable();
        return;
    }
    if ( &aMatchCaseCB == pCtrl && ( SEARCH_OPTIONS_EXACT & nOptions ) != 0 )
    {
        if (!aJapOptionsCB.IsChecked())
            aMatchCaseCB.Enable();
        return;
    }
    if ( &aSelectionBtn == pCtrl && ( SEARCH_OPTIONS_SELECTION & nOptions ) != 0 )
    {
        aSelectionBtn.Enable();
        return;
    }
    if ( &aLayoutBtn == pCtrl && ( SEARCH_OPTIONS_FAMILIES & nOptions ) != 0 )
    {
        aLayoutBtn.Enable();
        return;
    }
    if (    &aAttributeBtn == pCtrl
         && ( SEARCH_OPTIONS_FORMAT & nOptions ) != 0
         && pSearchList )
    {
        aAttributeBtn.Enable( pImpl->bFocusOnSearch );
    }
    if ( &aFormatBtn == pCtrl && ( SEARCH_OPTIONS_FORMAT & nOptions ) != 0 )
    {
        aFormatBtn.Enable();
        return;
    }
    if ( &aNoFormatBtn == pCtrl && ( SEARCH_OPTIONS_FORMAT & nOptions ) != 0 )
    {
        aNoFormatBtn.Enable();
        return;
    }
    if ( &aSimilarityBox == pCtrl &&
         ( SEARCH_OPTIONS_SIMILARITY & nOptions ) != 0 )
    {
        aSimilarityBox.Enable();

        if ( aSimilarityBox.IsChecked() )
            aSimilarityBtn.Enable();
    }
}

// -----------------------------------------------------------------------

void SvxSearchDialog::SetItem_Impl( const SvxSearchItem* pItem )
{
    if ( pItem )
    {
        delete pSearchItem;
        pSearchItem = (SvxSearchItem*)pItem->Clone();
        Init_Impl( pSearchItem->GetPattern() &&
                   ( !pSearchList || !pSearchList->Count() ) );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, FocusHdl_Impl, Control *, pCtrl )
{
    xub_StrLen nTxtLen;

    if ( !pImpl->bMultiLineEdit )
        nTxtLen = aSearchAttrText.GetText().Len();
    else
        nTxtLen = pImpl->aSearchFormats.GetText().Len();

    if ( pCtrl == &aSearchLB || pCtrl == &pImpl->aSearchFormats )
    {
        if ( pCtrl->HasChildPathFocus() )
            pImpl->bFocusOnSearch = sal_True;
        pCtrl = &aSearchLB;
        bSearch = sal_True;

        if( nTxtLen )
            EnableControl_Impl( &aNoFormatBtn );
        else
            aNoFormatBtn.Disable();
        EnableControl_Impl( &aAttributeBtn );
    }
    else
    {
        pImpl->bFocusOnSearch = sal_False;
        pCtrl = &aReplaceLB;
        bSearch = sal_False;

        if ( ( !pImpl->bMultiLineEdit && aReplaceAttrText.GetText().Len() ) ||
                ( pImpl->bMultiLineEdit && pImpl->aReplaceFormats.GetText().Len() ) )
            EnableControl_Impl( &aNoFormatBtn );
        else
            aNoFormatBtn.Disable();
        aAttributeBtn.Disable();
    }
    bSet = sal_True;

    aSearchLB.SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );

    ModifyHdl_Impl( (ComboBox*)pCtrl );

    aLayoutBtn.SetText( bFormat && nTxtLen ? aLayoutStr : aStylesStr );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxSearchDialog, LoseFocusHdl_Impl)
{
    SaveToModule_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxSearchDialog, FormatHdl_Impl)
{
    SfxObjectShell* pSh = SfxObjectShell::Current();

    DBG_ASSERT( pSh, "no DocShell" );

    if ( !pSh || !pImpl->pRanges )
        return 0;

    sal_sSize nCnt = 0;
    const sal_uInt16* pPtr = pImpl->pRanges;
    const sal_uInt16* pTmp = pPtr;

    while( *pTmp )
        pTmp++;
    nCnt = pTmp - pPtr + 7;
    sal_uInt16* pWhRanges = new sal_uInt16[nCnt];
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
    pWhRanges[nPos++] = SID_PARA_BACKGRND_DESTINATION;
    pWhRanges[nPos++] = SID_PARA_BACKGRND_DESTINATION;
    pWhRanges[nPos] = 0;
    SfxItemPool& rPool = pSh->GetPool();
    SfxItemSet aSet( rPool, pWhRanges );
    String aTxt;

    aSet.InvalidateAllItems();
    aSet.Put(SvxBrushItem(nBrushWhich));
    aSet.Put(SfxUInt16Item(SID_PARA_BACKGRND_DESTINATION, PARA_DEST_CHAR));

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
        SfxAbstractTabDialog* pDlg = pFact->CreateTabItemDialog( this, aSet, RID_SVXDLG_SEARCHFORMAT );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");
        aTxt.Insert( pDlg->GetText(), 0 );
        pDlg->SetText( aTxt );

        if ( pDlg->Execute() == RET_OK )
        {
            DBG_ASSERT( pDlg->GetOutputItemSet(), "invalid Output-Set" );
            SfxItemSet aOutSet( *pDlg->GetOutputItemSet() );

            SearchAttrItemList* pList = bSearch ? pSearchList : pReplaceList;

            SearchAttrItem* pAItem;
            const SfxPoolItem* pItem;
            for( sal_uInt16 n = 0; n < pList->Count(); ++n )
                if( !IsInvalidItem( (pAItem = &pList->GetObject(n))->pItem ) &&
                    SFX_ITEM_SET == aOutSet.GetItemState(
                        pAItem->pItem->Which(), sal_False, &pItem ) )
                {
                    delete pAItem->pItem;
                    pAItem->pItem = pItem->Clone();
                    aOutSet.ClearItem( pAItem->pItem->Which() );
                }

            if( aOutSet.Count() )
                pList->Put( aOutSet );

            PaintAttrText_Impl(); // Set AttributText in GroupBox
        }
        delete pDlg;
    }
    delete[] pWhRanges;
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxSearchDialog, NoFormatHdl_Impl)
{
    aLayoutBtn.SetText( aStylesStr );
    bFormat = sal_False;
    aLayoutBtn.Check( sal_False );

    if ( bSearch )
    {
        if ( !pImpl->bMultiLineEdit )
            aSearchAttrText.SetText( String() );
        else
            pImpl->aSearchFormats.SetText( String() );
        pSearchList->Clear();
    }
    else
    {
        if ( !pImpl->bMultiLineEdit )
            aReplaceAttrText.SetText( String() );
        else
            pImpl->aReplaceFormats.SetText( String() );
        pReplaceList->Clear();
    }
    pImpl->bSaveToModule = sal_False;
    TemplateHdl_Impl( &aLayoutBtn );
    pImpl->bSaveToModule = sal_True;
    aNoFormatBtn.Disable();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxSearchDialog, AttributeHdl_Impl)
{
    if ( !pSearchList || !pImpl->pRanges )
        return 0;

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        VclAbstractDialog* pDlg = pFact->CreateSvxSearchAttributeDialog( this, *pSearchList, pImpl->pRanges );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");
        pDlg->Execute();
        delete pDlg;
    }
    PaintAttrText_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, TimeoutHdl_Impl, Timer *, pTimer )
{
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if ( pViewShell )
    {
        if ( pViewShell->HasSelection( aSearchLB.IsVisible() ) )
            EnableControl_Impl( &aSelectionBtn );
        else
        {
            aSelectionBtn.Check( sal_False );
            aSelectionBtn.Disable();
        }
    }

    pTimer->Start();
    return 0;
}

// -----------------------------------------------------------------------

String& SvxSearchDialog::BuildAttrText_Impl( String& rStr,
                                             sal_Bool bSrchFlag ) const
{
    if ( rStr.Len() )
        rStr.Erase();

    SfxObjectShell* pSh = SfxObjectShell::Current();
    DBG_ASSERT( pSh, "no DocShell" );

    if ( !pSh )
        return rStr;

    SfxItemPool& rPool = pSh->GetPool();
    SearchAttrItemList* pList = bSrchFlag ? pSearchList : pReplaceList;

    if ( !pList )
        return rStr;

    // Metric query
    SfxMapUnit eMapUnit = SFX_MAPUNIT_CM;
    FieldUnit eFieldUnit = pSh->GetModule()->GetFieldUnit();
    switch ( eFieldUnit )
    {
        case FUNIT_MM:          eMapUnit = SFX_MAPUNIT_MM; break;
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:          eMapUnit = SFX_MAPUNIT_CM; break;
        case FUNIT_TWIP:        eMapUnit = SFX_MAPUNIT_TWIP; break;
        case FUNIT_POINT:
        case FUNIT_PICA:        eMapUnit = SFX_MAPUNIT_POINT; break;
        case FUNIT_INCH:
        case FUNIT_FOOT:
        case FUNIT_MILE:        eMapUnit = SFX_MAPUNIT_INCH; break;
        case FUNIT_100TH_MM:    eMapUnit = SFX_MAPUNIT_100TH_MM; break;
        default: ;//prevent warning
    }

    ResStringArray aAttrNames( SVX_RES( RID_ATTR_NAMES ) );

    for ( sal_uInt16 i = 0; i < pList->Count(); ++i )
    {
        const SearchAttrItem& rItem = pList->GetObject(i);

        if ( rStr.Len() )
            rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );

        if ( !IsInvalidItem( rItem.pItem ) )
        {
            String aStr;
            rPool.GetPresentation( *rItem.pItem,
                                    SFX_ITEM_PRESENTATION_COMPLETE,
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

// -----------------------------------------------------------------------

void SvxSearchDialog::PaintAttrText_Impl()
{
    String aDesc;
    BuildAttrText_Impl( aDesc, bSearch );

    if ( !bFormat && aDesc.Len() )
        bFormat = sal_True;

    if ( bSearch )
    {
        if ( !pImpl->bMultiLineEdit )
            aSearchAttrText.SetText( aDesc );
        else
            pImpl->aSearchFormats.SetText( aDesc );
        FocusHdl_Impl( &aSearchLB );
    }
    else
    {
        if ( !pImpl->bMultiLineEdit )
            aReplaceAttrText.SetText( aDesc );
        else
            pImpl->aReplaceFormats.SetText( aDesc );
        FocusHdl_Impl( &aReplaceLB );
    }
}

// -----------------------------------------------------------------------

void SvxSearchDialog::SetModifyFlag_Impl( const Control* pCtrl )
{
    if ( &aSearchLB == (ComboBox*)pCtrl )
        nModifyFlag |= MODIFY_SEARCH;
    else if ( &aReplaceLB == (ComboBox*)pCtrl )
        nModifyFlag |= MODIFY_REPLACE;
    else if ( &aWordBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_WORD;
    else if ( &aMatchCaseCB == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_EXACT;
    else if ( &aBackwardsBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_BACKWARDS;
    else if ( &aNotesBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_NOTES;
    else if ( &aSelectionBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_SELECTION;
    else if ( &aRegExpBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_REGEXP;
    else if ( &aLayoutBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_LAYOUT;
    else if ( &aSimilarityBox == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_SIMILARITY;
    else if ( &aCalcSearchInLB == (ListBox*)pCtrl )
    {
        nModifyFlag |= MODIFY_FORMULAS;
        nModifyFlag |= MODIFY_VALUES;
        nModifyFlag |= MODIFY_CALC_NOTES;
    }
    else if ( &aRowsBtn == (RadioButton*)pCtrl )
        nModifyFlag |= MODIFY_ROWS;
    else if ( &aColumnsBtn == (RadioButton*)pCtrl )
        nModifyFlag |= MODIFY_COLUMNS;
    else if ( &aAllSheetsCB == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_ALLTABLES;
}

// -----------------------------------------------------------------------

void SvxSearchDialog::SaveToModule_Impl()
{
    if ( !pSearchItem )
        return;

    if ( aLayoutBtn.IsChecked() )
    {
        pSearchItem->SetSearchString ( aSearchTmplLB.GetSelectEntry() );
        pSearchItem->SetReplaceString( aReplaceTmplLB.GetSelectEntry() );
    }
    else
    {
        pSearchItem->SetSearchString ( aSearchLB.GetText() );
        pSearchItem->SetReplaceString( aReplaceLB.GetText() );
        Remember_Impl( aSearchLB.GetText(), sal_True );
    }

    pSearchItem->SetRegExp( sal_False );
    pSearchItem->SetLevenshtein( sal_False );
    if (GetCheckBoxValue( aRegExpBtn ))
        pSearchItem->SetRegExp( sal_True );
    else if (GetCheckBoxValue( aSimilarityBox ))
        pSearchItem->SetLevenshtein( sal_True );

    pSearchItem->SetWordOnly( GetCheckBoxValue( aWordBtn ) );
    pSearchItem->SetBackward( GetCheckBoxValue( aBackwardsBtn ) );
    pSearchItem->SetNotes( GetCheckBoxValue( aNotesBtn ) );
    pSearchItem->SetPattern( GetCheckBoxValue( aLayoutBtn ) );
    pSearchItem->SetSelection( GetCheckBoxValue( aSelectionBtn ) );

    pSearchItem->SetUseAsianOptions( GetCheckBoxValue( aJapOptionsCB ) );
    sal_Int32 nFlags = GetTransliterationFlags();
    if( !pSearchItem->IsUseAsianOptions())
        nFlags &= (TransliterationModules_IGNORE_CASE |
                   TransliterationModules_IGNORE_WIDTH );
    pSearchItem->SetTransliterationFlags( nFlags );

    if ( !bWriter )
    {
        if ( aCalcSearchInLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
            pSearchItem->SetCellType( aCalcSearchInLB.GetSelectEntryPos() );

        pSearchItem->SetRowDirection( aRowsBtn.IsChecked() );
        pSearchItem->SetAllTables( aAllSheetsCB.IsChecked() );
    }

    pSearchItem->SetCommand( SVX_SEARCHCMD_FIND );
    nModifyFlag = 0;
    const SfxPoolItem* ppArgs[] = { pSearchItem, 0 };
    rBindings.GetDispatcher()->Execute( SID_SEARCH_ITEM, SFX_CALLMODE_SLOT, ppArgs );
}

// class SvxSearchDialogWrapper ------------------------------------------

SFX_IMPL_CHILDWINDOW_WITHID(SvxSearchDialogWrapper, SID_SEARCH_DLG);

// -----------------------------------------------------------------------

SvxSearchDialogWrapper::SvxSearchDialogWrapper( Window* _pParent, sal_uInt16 nId,
                                                SfxBindings* pBindings,
                                                SfxChildWinInfo* pInfo )
    : SfxChildWindow( _pParent, nId )
    , dialog (new SvxSearchDialog (_pParent, this, *pBindings))
{
    pWindow = dialog;
    dialog->Initialize( pInfo );

    pBindings->Update( SID_SEARCH_ITEM );
    pBindings->Update( SID_SEARCH_OPTIONS );
    pBindings->Update( SID_SEARCH_SEARCHSET );
    pBindings->Update( SID_SEARCH_REPLACESET );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    dialog->bConstruct = sal_False;
}

SvxSearchDialogWrapper::~SvxSearchDialogWrapper ()
{
}

SvxSearchDialog *SvxSearchDialogWrapper::getDialog ()
{
    return dialog;
}

// -----------------------------------------------------------------------

SfxChildWinInfo SvxSearchDialogWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    aInfo.bVisible = sal_False;
    return aInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
