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

#include <osl/diagnose.h>
#include <vcl/timer.hxx>
#include <svl/slstitm.hxx>
#include <svl/itemiter.hxx>
#include <svl/style.hxx>
#include <svl/whiter.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/searchopt.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/scopeguard.hxx>
#include <svl/itempool.hxx>

#include <sfx2/app.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <svx/srchdlg.hxx>
#include <svx/strarray.hxx>

#include <svx/strings.hrc>
#include <svx/svxids.hrc>

#include <svl/srchitem.hxx>
#include <svx/pageitem.hxx>
#include "srchctrl.hxx"
#include <svx/dialmgr.hxx>
#include <editeng/brushitem.hxx>
#include <tools/resary.hxx>
#include <svx/svxdlg.hxx>
#include <vcl/toolbox.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <comphelper/lok.hxx>

#include <cstdlib>
#include <memory>

#include <findtextfield.hxx>

#include <svx/labelitemwindow.hxx>
#include <svx/xdef.hxx>
#include <officecfg/Office/Common.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace comphelper;


static bool IsMobile()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return false;
    SfxViewShell* pCurrent = SfxViewShell::Current();
    return pCurrent && pCurrent->isLOKMobilePhone();
}

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
    bool GetCheckBoxValue(const weld::CheckButton& rBox)
    {
        return rBox.get_sensitive() && rBox.get_active();
    }

    bool GetNegatedCheckBoxValue(const weld::CheckButton& rBox)
    {
        return rBox.get_sensitive() && !rBox.get_active();
    }
}

struct SearchDlg_Impl
{
    bool        bSaveToModule  : 1,
                bFocusOnSearch : 1;
    WhichRangesContainer pRanges;
    Timer       aSelectionTimer { "svx SearchDlg_Impl aSelectionTimer" };

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
};

static void ListToStrArr_Impl(sal_uInt16 nId, std::vector<OUString>& rStrLst, weld::ComboBox& rCBox, sal_uInt16 nRememberSize)
{
    const SfxStringListItem* pSrchItem =
        static_cast<const SfxStringListItem*>(SfxGetpApp()->GetItem( nId ));

    if (!pSrchItem)
        return;

    std::vector<OUString> aLst = pSrchItem->GetList();

    if (aLst.size() > nRememberSize)
        aLst.resize(nRememberSize);

    for (const OUString & s : aLst)
    {
        rStrLst.push_back(s);
        rCBox.append_text(s);
    }
}

static void StrArrToList_Impl( TypedWhichId<SfxStringListItem> nId, const std::vector<OUString>& rStrLst )
{
    DBG_ASSERT( !rStrLst.empty(), "check in advance");
    SfxGetpApp()->PutItem( SfxStringListItem( nId, &rStrLst ) );
}

SearchAttrItemList::SearchAttrItemList( SearchAttrItemList&& rList ) noexcept :
    SrchAttrInfoList(std::move(rList))
{
}

SearchAttrItemList::SearchAttrItemList( const SearchAttrItemList& rList ) :
    SrchAttrInfoList(rList)
{
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

    for (SfxItemIter aIter(rSet); !aIter.IsAtEnd(); aIter.NextItem())
    {
        const sal_uInt16 nWhich(aIter.GetCurWhich());
        const sal_uInt16 nSlot(pPool->GetSlotId(nWhich));
        emplace_back(nSlot, SfxPoolItemHolder(*pPool, aIter.GetCurItem()));
    }
}


SfxItemSet& SearchAttrItemList::Get( SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();

    for ( size_t i = 0; i < size(); ++i )
        if ( IsInvalidItem( (*this)[i].aItemPtr.getItem() ) )
            rSet.InvalidateItem( pPool->GetWhichIDFromSlotID( (*this)[i].nSlot ) );
        else
            rSet.Put( *(*this)[i].aItemPtr.getItem() );
    return rSet;
}


void SearchAttrItemList::Clear()
{
    SrchAttrInfoList::clear();
}


// Deletes the pointer to the items
void SearchAttrItemList::Remove(size_t nPos)
{
    size_t nLen = 1;
    if ( nPos + nLen > size() )
        nLen = size() - nPos;

    SrchAttrInfoList::erase( begin() + nPos, begin() + nPos + nLen );
}

SvxSearchDialog::SvxSearchDialog(weld::Window* pParent, SfxChildWindow* pChildWin, SfxBindings& rBind)
    : SfxModelessDialogController(&rBind, pChildWin, pParent,
                                  IsMobile() ? u"svx/ui/findreplacedialog-mobile.ui"_ustr : u"svx/ui/findreplacedialog.ui"_ustr,
                                  u"FindReplaceDialog"_ustr)
    , m_rBindings(rBind)
    , m_aPresentIdle("Bring SvxSearchDialog to Foreground")
    , m_bWriter(false)
    , m_bSearch(true)
    , m_bFormat(false)
    , m_bReplaceBackwards(false)
    , m_nOptions(SearchOptionFlags::ALL)
    , m_bSet(false)
    , m_bConstruct(true)
    , m_nModifyFlag(ModifyFlags::NONE)
    , m_pReplaceList(new SearchAttrItemList)
    , m_nTransliterationFlags(TransliterationFlags::NONE)
    , m_xSearchFrame(m_xBuilder->weld_frame(u"searchframe"_ustr))
    , m_xSearchLB(m_xBuilder->weld_combo_box(u"searchterm"_ustr))
    , m_xSearchTmplLB(m_xBuilder->weld_combo_box(u"searchlist"_ustr))
    , m_xSearchAttrText(m_xBuilder->weld_label(u"searchdesc"_ustr))
    , m_xSearchLabel(m_xBuilder->weld_label(u"searchlabel"_ustr))
    , m_xSearchIcon(m_xBuilder->weld_image(u"searchicon"_ustr))
    , m_xSearchBox(m_xBuilder->weld_container(u"searchbox"_ustr))
    , m_xReplaceFrame(m_xBuilder->weld_frame(u"replaceframe"_ustr))
    , m_xReplaceLB(m_xBuilder->weld_combo_box(u"replaceterm"_ustr))
    , m_xReplaceTmplLB(m_xBuilder->weld_combo_box(u"replacelist"_ustr))
    , m_xReplaceAttrText(m_xBuilder->weld_label(u"replacedesc"_ustr))
    , m_xSearchBtn(m_xBuilder->weld_button(u"search"_ustr))
    , m_xBackSearchBtn(m_xBuilder->weld_button(u"backsearch"_ustr))
    , m_xSearchAllBtn(m_xBuilder->weld_button(u"searchall"_ustr))
    , m_xReplaceBtn(m_xBuilder->weld_button(u"replace"_ustr))
    , m_xReplaceAllBtn(m_xBuilder->weld_button(u"replaceall"_ustr))
    , m_xComponentFrame(m_xBuilder->weld_frame(u"componentframe"_ustr))
    , m_xSearchComponent1PB(m_xBuilder->weld_button(u"component1"_ustr))
    , m_xSearchComponent2PB(m_xBuilder->weld_button(u"component2"_ustr))
    , m_xMatchCaseCB(m_xBuilder->weld_check_button(u"matchcase"_ustr))
    , m_xSearchFormattedCB(m_xBuilder->weld_check_button(u"searchformatted"_ustr))
    , m_xWordBtn(m_xBuilder->weld_check_button(u"wholewords"_ustr))
    , m_xCloseBtn(m_xBuilder->weld_button(u"close"_ustr))
    , m_xHelpBtn(m_xBuilder->weld_button(u"help"_ustr))
    , m_xIncludeDiacritics(m_xBuilder->weld_check_button(u"includediacritics"_ustr))
    , m_xIncludeKashida(m_xBuilder->weld_check_button(u"includekashida"_ustr))
    , m_xOtherOptionsExpander(m_xBuilder->weld_expander(u"OptionsExpander"_ustr))
    , m_xSelectionBtn(m_xBuilder->weld_check_button(u"selection"_ustr))
    , m_xRegExpBtn(m_xBuilder->weld_check_button(u"regexp"_ustr))
    , m_xWildcardBtn(m_xBuilder->weld_check_button(u"wildcard"_ustr))
    , m_xSimilarityBox(m_xBuilder->weld_check_button(u"similarity"_ustr))
    , m_xSimilarityBtn(m_xBuilder->weld_button(u"similaritybtn"_ustr))
    , m_xLayoutBtn(m_xBuilder->weld_check_button(u"layout"_ustr))
    , m_xNotesBtn(m_xBuilder->weld_check_button(u"notes"_ustr))
    , m_xJapMatchFullHalfWidthCB(m_xBuilder->weld_check_button(u"matchcharwidth"_ustr))
    , m_xJapOptionsCB(m_xBuilder->weld_check_button(u"soundslike"_ustr))
    , m_xReplaceBackwardsCB(m_xBuilder->weld_check_button(u"replace_backwards"_ustr))
    , m_xJapOptionsBtn(m_xBuilder->weld_button(u"soundslikebtn"_ustr))
    , m_xAttributeBtn(m_xBuilder->weld_button(u"attributes"_ustr))
    , m_xFormatBtn(m_xBuilder->weld_button(u"format"_ustr))
    , m_xNoFormatBtn(m_xBuilder->weld_button(u"noformat"_ustr))
    , m_xCalcGrid(m_xBuilder->weld_widget(u"calcgrid"_ustr))
    , m_xCalcSearchInFT(m_xBuilder->weld_label(u"searchinlabel"_ustr))
    , m_xCalcSearchInLB(m_xBuilder->weld_combo_box(u"calcsearchin"_ustr))
    , m_xCalcSearchDirFT(m_xBuilder->weld_label(u"searchdir"_ustr))
    , m_xRowsBtn(m_xBuilder->weld_radio_button(u"rows"_ustr))
    , m_xColumnsBtn(m_xBuilder->weld_radio_button(u"cols"_ustr))
    , m_xAllSheetsCB(m_xBuilder->weld_check_button(u"allsheets"_ustr))
    , m_xCalcStrFT(m_xBuilder->weld_label(u"entirecells"_ustr))
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        m_xCloseBtn->hide();
        m_xHelpBtn->hide();
    }

    m_aPresentIdle.SetTimeout(50);
    m_aPresentIdle.SetInvokeHandler(LINK(this, SvxSearchDialog, PresentTimeoutHdl_Impl));

    m_xSearchTmplLB->make_sorted();
    m_xSearchAttrText->hide();

    this->SetSearchLabel(u""_ustr); // hide the message but keep the box height

    //weird findreplacedialog-mobile.ui case doesn't have searchicon or searchbox
    if (m_xSearchIcon)
        m_xSearchIcon->set_size_request(24, 24); // vcl/res/infobar.png is 32x32 - too large here

    m_xReplaceTmplLB->make_sorted();
    m_xReplaceAttrText->hide();

    m_sCalcStr = m_xCalcStrFT->get_label();

    // m_xSimilarityBtn->set_height_request(m_xSimilarityBox->get_preferred_size().Height());
    // m_xJapOptionsBtn->set_height_request(m_xJapOptionsCB->get_preferred_size().Height());

    //tdf#122322
    m_nRememberSize = officecfg::Office::Common::Misc::FindReplaceRememberedSearches::get();
    if (m_nRememberSize < 1)
        m_nRememberSize = 1; //0 crashes with no results found

    auto nTermWidth = m_xSearchLB->get_approximate_digit_width() * 28;
    m_xSearchLB->set_size_request(nTermWidth, -1);
    m_xSearchTmplLB->set_size_request(nTermWidth, -1);
    m_xReplaceLB->set_size_request(nTermWidth, -1);
    m_xReplaceTmplLB->set_size_request(nTermWidth, -1);

    Construct_Impl();
}

IMPL_LINK_NOARG(SvxSearchDialog, PresentTimeoutHdl_Impl, Timer*, void)
{
    getDialog()->present();
}

void SvxSearchDialog::Present()
{
    PresentTimeoutHdl_Impl(nullptr);
    // tdf#133807 try again in a short timeout
    m_aPresentIdle.Start();
}

void SvxSearchDialog::ChildWinDispose()
{
    m_rBindings.EnterRegistrations();
    m_pSearchController.reset();
    m_pOptionsController.reset();
    m_pFamilyController.reset();
    m_rBindings.LeaveRegistrations();
    SfxModelessDialogController::ChildWinDispose();
}

SvxSearchDialog::~SvxSearchDialog()
{
    m_aPresentIdle.Stop();
    m_pSearchItem.reset();
    m_pImpl.reset();
}

void SvxSearchDialog::Construct_Impl()
{
    m_pImpl.reset(new SearchDlg_Impl());
    m_pImpl->aSelectionTimer.SetTimeout(500);
    m_pImpl->aSelectionTimer.SetInvokeHandler(LINK(this, SvxSearchDialog, TimeoutHdl_Impl));
    EnableControls_Impl( SearchOptionFlags::NONE );

    // Store old Text from m_xWordBtn
    m_sCalcStr += "#";
    m_sCalcStr += m_xWordBtn->get_label();

    m_sLayoutStr = SvxResId(RID_SVXSTR_SEARCH_STYLES);
    m_sLayoutWriterStr = SvxResId(RID_SVXSTR_WRITER_STYLES);
    m_sLayoutCalcStr = SvxResId(RID_SVXSTR_CALC_STYLES);
    m_sStylesStr = m_xLayoutBtn->get_label();

    // Get stored search-strings from the application
    ListToStrArr_Impl(SID_SEARCHDLG_SEARCHSTRINGS, m_aSearchStrings, *m_xSearchLB, m_nRememberSize);
    ListToStrArr_Impl(SID_SEARCHDLG_REPLACESTRINGS, m_aReplaceStrings, *m_xReplaceLB,
                      m_nRememberSize);

    InitControls_Impl();

    // Get attribute sets only once in constructor()
    const SfxPoolItem* ppArgs[] = { m_pSearchItem.get(), nullptr };
    SfxPoolItemHolder aResult(
        m_rBindings.GetDispatcher()->Execute(FID_SEARCH_SEARCHSET, SfxCallMode::SLOT, ppArgs));
    const SvxSetItem* pSrchSetItem(static_cast<const SvxSetItem*>(aResult.getItem()));

    if ( pSrchSetItem )
        InitAttrList_Impl( &pSrchSetItem->GetItemSet(), nullptr );

    aResult
        = m_rBindings.GetDispatcher()->Execute(FID_SEARCH_REPLACESET, SfxCallMode::SLOT, ppArgs);
    const SvxSetItem* pReplSetItem(static_cast<const SvxSetItem*>(aResult.getItem()));

    if ( pReplSetItem )
        InitAttrList_Impl( nullptr, &pReplSetItem->GetItemSet() );

    // Create controller and update at once
    m_rBindings.EnterRegistrations();
    m_pSearchController.reset(new SvxSearchController(SID_SEARCH_ITEM, m_rBindings, *this));
    m_pOptionsController.reset(new SvxSearchController(SID_SEARCH_OPTIONS, m_rBindings, *this));
    m_rBindings.LeaveRegistrations();
    m_rBindings.GetDispatcher()->Execute(FID_SEARCH_ON, SfxCallMode::SLOT, ppArgs);
    m_pImpl->aSelectionTimer.Start();

    if(!SvtCJKOptions::IsJapaneseFindEnabled())
    {
        m_xJapOptionsCB->set_active( false );
        m_xJapOptionsCB->hide();
        m_xJapOptionsBtn->hide();
    }
    if(!SvtCJKOptions::IsCJKFontEnabled())
    {
        m_xJapMatchFullHalfWidthCB->hide();
    }
    // Do not disable and hide the m_xIncludeDiacritics button.
    // Include Diacritics == Not Ignore Diacritics => A does not match A-Umlaut (Diaeresis).
    // Confusingly these have negated names (following the UI) but the actual
    // transliteration is to *ignore* diacritics if "included" (sensitive) is
    // _not_ checked.
    if(!SvtCTLOptions::IsCTLFontEnabled())
    {
        m_xIncludeDiacritics->set_active( true );
        m_xIncludeKashida->set_active( true );
        m_xIncludeKashida->hide();
    }
    //component extension - show component search buttons if the commands
    // vnd.sun.star::SearchViaComponent1 and 2 are supported
    const uno::Reference<frame::XFrame> xFrame = m_rBindings.GetActiveFrame();
    const uno::Reference< frame::XDispatchProvider > xDispatchProv(xFrame, uno::UNO_QUERY);

    bool bSearchComponent1 = false;
    bool bSearchComponent2 = false;
    if(xDispatchProv.is())
    {
        OUString sTarget(u"_self"_ustr);
        m_pImpl->xCommand1Dispatch
            = xDispatchProv->queryDispatch(m_pImpl->aCommand1URL, sTarget, 0);
        if (m_pImpl->xCommand1Dispatch.is())
            bSearchComponent1 = true;
        m_pImpl->xCommand2Dispatch
            = xDispatchProv->queryDispatch(m_pImpl->aCommand2URL, sTarget, 0);
        if (m_pImpl->xCommand2Dispatch.is())
            bSearchComponent2 = true;
    }

    if( !(bSearchComponent1 || bSearchComponent2) )
        return;

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider =
                configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() );
        uno::Sequence< uno::Any > aArgs {
                    Any(u"/org.openoffice.Office.Common/SearchOptions/"_ustr) };

        uno::Reference< uno::XInterface > xIFace = xConfigurationProvider->createInstanceWithArguments(
                    u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr,
                    aArgs);
        uno::Reference< container::XNameAccess> xDirectAccess(xIFace, uno::UNO_QUERY);
        if(xDirectAccess.is())
        {
            OUString sTemp;
            uno::Any aRet = xDirectAccess->getByName(u"ComponentSearchGroupLabel"_ustr);
            aRet >>= sTemp;
            m_xComponentFrame->set_label(sTemp);
            aRet = xDirectAccess->getByName(u"ComponentSearchCommandLabel1"_ustr);
            aRet >>= sTemp;
            m_xSearchComponent1PB->set_label( sTemp );
            aRet = xDirectAccess->getByName(u"ComponentSearchCommandLabel2"_ustr);
            aRet >>= sTemp;
            m_xSearchComponent2PB->set_label( sTemp );
        }
    }
    catch(uno::Exception&){}

    if(!m_xSearchComponent1PB->get_label().isEmpty() && bSearchComponent1 )
    {
        m_xComponentFrame->show();
        m_xSearchComponent1PB->show();
    }
    if( !m_xSearchComponent2PB->get_label().isEmpty() )
    {
        m_xComponentFrame->show();
        m_xSearchComponent2PB->show();
    }
}

void SvxSearchDialog::Close()
{
    // remember strings
    if (!m_aSearchStrings.empty())
        StrArrToList_Impl(SID_SEARCHDLG_SEARCHSTRINGS, m_aSearchStrings);

    if (!m_aReplaceStrings.empty())
        StrArrToList_Impl(SID_SEARCHDLG_REPLACESTRINGS, m_aReplaceStrings);

    // save settings to configuration
    SvtSearchOptions aOpt;
    aOpt.SetWholeWordsOnly          ( m_xWordBtn->get_active() );
    aOpt.SetBackwards               ( m_xReplaceBackwardsCB->get_active() );
    aOpt.SetUseRegularExpression    ( m_xRegExpBtn->get_active() );
    aOpt.SetUseWildcard             ( m_xWildcardBtn->get_active() );
    aOpt.SetSearchForStyles         ( m_xLayoutBtn->get_active() );
    aOpt.SetSimilaritySearch        ( m_xSimilarityBox->get_active() );
    aOpt.SetUseAsianOptions         ( m_xJapOptionsCB->get_active() );
    aOpt.SetNotes                   ( m_xNotesBtn->get_active() );
    aOpt.SetIgnoreDiacritics_CTL    ( !m_xIncludeDiacritics->get_active() );
    aOpt.SetIgnoreKashida_CTL       ( !m_xIncludeKashida->get_active() );
    aOpt.SetSearchFormatted         ( m_xSearchFormattedCB->get_active() );
    aOpt.Commit();

    if (IsClosing())
        return;

    const SfxPoolItem* ppArgs[] = { m_pSearchItem.get(), nullptr };
    m_rBindings.GetDispatcher()->Execute(FID_SEARCH_OFF, SfxCallMode::SLOT, ppArgs);
    m_rBindings.Invalidate(SID_SEARCH_DLG);

    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (pViewFrame)
        pViewFrame->ToggleChildWindow(SID_SEARCH_DLG);
}

TransliterationFlags SvxSearchDialog::GetTransliterationFlags() const
{
    if (!m_xMatchCaseCB->get_active())
        m_nTransliterationFlags |= TransliterationFlags::IGNORE_CASE;
    else
        m_nTransliterationFlags &= ~TransliterationFlags::IGNORE_CASE;
    if ( !m_xJapMatchFullHalfWidthCB->get_active())
        m_nTransliterationFlags |= TransliterationFlags::IGNORE_WIDTH;
    else
        m_nTransliterationFlags &= ~TransliterationFlags::IGNORE_WIDTH;
    return m_nTransliterationFlags;
}

void SvxSearchDialog::SetSaveToModule(bool b) { m_pImpl->bSaveToModule = b; }

void SvxSearchDialog::SetSearchLabel(const OUString& rStr)
{
    m_xSearchLabel->set_label(rStr);
    if (!rStr.isEmpty())
    {
        m_xSearchLabel->show();
        if (m_xSearchIcon)
        {
            m_xSearchIcon->show();
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            if (rStyleSettings.GetDialogColor().IsDark())
                m_xSearchBox->set_background(Color(0x00, 0x56, 0x80));
            else
                m_xSearchBox->set_background(Color(0xBD, 0xE5, 0xF8)); // same as InfobarType::INFO
        }
    }
    else
    {
        m_xSearchLabel->hide();
        if (m_xSearchIcon)
        {
            const Size aSize = m_xSearchBox->get_preferred_size();
            m_xSearchIcon->hide();
            m_xSearchBox->set_size_request(-1, aSize.Height());
            m_xSearchBox->set_background(COL_TRANSPARENT);
        }
    }

    if (rStr == SvxResId(RID_SVXSTR_SEARCH_NOT_FOUND))
        m_xSearchLB->set_entry_message_type(weld::EntryMessageType::Error);
}

void SvxSearchDialog::ApplyTransliterationFlags_Impl( TransliterationFlags nSettings )
{
    m_nTransliterationFlags = nSettings;
    bool bVal(nSettings & TransliterationFlags::IGNORE_CASE);
    m_xMatchCaseCB->set_active( !bVal );
    bVal = bool(nSettings & TransliterationFlags::IGNORE_WIDTH);
    m_xJapMatchFullHalfWidthCB->set_active( !bVal );
}


bool SvxSearchDialog::IsOtherOptionsExpanded() const
{
    return m_xReplaceBackwardsCB->get_active() ||
           m_xSelectionBtn->get_active() ||
           m_xRegExpBtn->get_active() ||
           m_xLayoutBtn->get_active() ||
           m_xSimilarityBox->get_active() ||
           m_xJapMatchFullHalfWidthCB->get_active() ||
           m_xJapOptionsCB->get_active() ||
           m_xWildcardBtn->get_active() ||
           m_xNotesBtn->get_active() ||
           m_xIncludeKashida->get_active() ||
           !m_xIncludeDiacritics->get_active();//tdf#138173
}

void SvxSearchDialog::Activate()
{
    // apply possible transliteration changes of the SvxSearchItem member
    if (m_pSearchItem)
    {
        m_xMatchCaseCB->set_active(m_pSearchItem->GetExact());
        m_xJapMatchFullHalfWidthCB->set_active(!m_pSearchItem->IsMatchFullHalfWidthForms());
    }

    SfxModelessDialogController::Activate();
}

void SvxSearchDialog::InitControls_Impl()
{
    // CaseSensitives AutoComplete
    m_xSearchLB->set_entry_completion( true, true );
    m_xSearchLB->show();
    m_xReplaceLB->set_entry_completion( true, true );
    m_xReplaceLB->show();

    m_xFormatBtn->set_sensitive(false);
    m_xAttributeBtn->set_sensitive(false);

    m_xSearchLB->connect_changed( LINK( this, SvxSearchDialog, ModifyHdl_Impl ) );
    m_xReplaceLB->connect_changed( LINK( this, SvxSearchDialog, ModifyHdl_Impl ) );

    Link<weld::Widget&,void> aLink = LINK( this, SvxSearchDialog, FocusHdl_Impl );
    m_xSearchLB->connect_focus_in( aLink );
    m_xReplaceLB->connect_focus_in( aLink );

    aLink = LINK( this, SvxSearchDialog, LoseFocusHdl_Impl );
    m_xSearchLB->connect_focus_out( aLink );
    m_xReplaceLB->connect_focus_out( aLink );

    m_xSearchTmplLB->connect_focus_out( aLink );
    m_xReplaceTmplLB->connect_focus_out( aLink );

    Link<weld::Button&,void> aLink2 = LINK( this, SvxSearchDialog, CommandHdl_Impl );
    m_xSearchBtn->connect_clicked( aLink2 );
    m_xBackSearchBtn->connect_clicked( aLink2 );
    m_xSearchAllBtn->connect_clicked( aLink2 );
    m_xReplaceBtn->connect_clicked( aLink2 );
    m_xReplaceAllBtn->connect_clicked( aLink2 );
    m_xCloseBtn->connect_clicked( aLink2 );
    m_xSimilarityBtn->connect_clicked( aLink2 );
    m_xJapOptionsBtn->connect_clicked( aLink2 );
    m_xSearchComponent1PB->connect_clicked( aLink2 );
    m_xSearchComponent2PB->connect_clicked( aLink2 );

    Link<weld::Toggleable&,void> aLink3 = LINK( this, SvxSearchDialog, FlagHdl_Impl );
    m_xReplaceBackwardsCB->connect_toggled( aLink3 );
    m_xWordBtn->connect_toggled( aLink3 );
    m_xSelectionBtn->connect_toggled( aLink3 );
    m_xMatchCaseCB->connect_toggled( aLink3 );
    m_xRegExpBtn->connect_toggled( aLink3 );
    m_xWildcardBtn->connect_toggled( aLink3 );
    m_xNotesBtn->connect_toggled( aLink3 );
    m_xSimilarityBox->connect_toggled( aLink3 );
    m_xJapOptionsCB->connect_toggled( aLink3 );
    m_xJapMatchFullHalfWidthCB->connect_toggled( aLink3 );
    m_xIncludeDiacritics->connect_toggled( aLink3 );
    m_xIncludeKashida->connect_toggled( aLink3 );
    m_xLayoutBtn->connect_toggled( LINK( this, SvxSearchDialog, TemplateHdl_Impl ) );
    m_xFormatBtn->connect_clicked( LINK( this, SvxSearchDialog, FormatHdl_Impl ) );
    m_xNoFormatBtn->connect_clicked(
        LINK( this, SvxSearchDialog, NoFormatHdl_Impl ) );
    m_xAttributeBtn->connect_clicked(
        LINK( this, SvxSearchDialog, AttributeHdl_Impl ) );
}

namespace
{
    SvtModuleOptions::EFactory getModule(SfxBindings const & rBindings)
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
    DBG_ASSERT(m_pSearchItem, "no search item");

    SvtModuleOptions::EFactory eFactory = getModule(m_rBindings);
    bool bDrawApp = eFactory == SvtModuleOptions::EFactory::DRAW;
    bool bWriterApp =
        eFactory == SvtModuleOptions::EFactory::WRITER ||
        eFactory == SvtModuleOptions::EFactory::WRITERWEB ||
        eFactory == SvtModuleOptions::EFactory::WRITERGLOBAL;
    bool bCalcApp = eFactory == SvtModuleOptions::EFactory::CALC;

    m_xLayoutBtn->set_visible(!bDrawApp);
    m_xNotesBtn->set_visible(bWriterApp);
    m_xRegExpBtn->set_visible(!bDrawApp);
    m_xWildcardBtn->set_visible(bCalcApp); /* TODO:WILDCARD enable for other apps if hey handle it */
    m_xReplaceBackwardsCB->show();
    m_xSimilarityBox->show();
    m_xSimilarityBtn->show();
    m_xSelectionBtn->show();
    m_xIncludeDiacritics->show();
    m_xIncludeKashida->set_visible(SvtCTLOptions::IsCTLFontEnabled());
    m_xJapMatchFullHalfWidthCB->set_visible(SvtCJKOptions::IsCJKFontEnabled());
    m_xJapOptionsCB->set_visible(SvtCJKOptions::IsJapaneseFindEnabled());
    m_xJapOptionsBtn->set_visible(SvtCJKOptions::IsJapaneseFindEnabled());

    if (m_bWriter)
    {
        m_xAttributeBtn->show();
        m_xFormatBtn->show();
        m_xNoFormatBtn->show();
    }

    if (bCalcApp)
    {
        m_xCalcSearchInFT->show();
        m_xCalcSearchInLB->show();
        m_xCalcSearchDirFT->show();
        m_xRowsBtn->show();
        m_xColumnsBtn->show();
        m_xAllSheetsCB->show();
        m_xSearchFormattedCB->show();
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
    DBG_ASSERT(m_pSearchItem, "SearchItem == 0");

    // We don't want to save any intermediate state to the module while the
    // dialog is being initialized.
    ToggleSaveToModule aNoModuleSave(*this, false);
    SvtSearchOptions aOpt;

    m_bWriter = (m_pSearchItem->GetAppFlag() == SvxSearchApp::WRITER);

    if (!(m_nModifyFlag & ModifyFlags::Word))
        m_xWordBtn->set_active(m_pSearchItem->GetWordOnly());
    if (!(m_nModifyFlag & ModifyFlags::Exact))
        m_xMatchCaseCB->set_active(m_pSearchItem->GetExact());
    if (!(m_nModifyFlag & ModifyFlags::Backwards))
        m_xReplaceBackwardsCB->set_active(m_bReplaceBackwards); //adjustment to replace backwards
    if (!(m_nModifyFlag & ModifyFlags::Notes))
        m_xNotesBtn->set_active(m_pSearchItem->GetNotes());
    if (!(m_nModifyFlag & ModifyFlags::Selection))
        m_xSelectionBtn->set_active(m_pSearchItem->GetSelection());
    if (!(m_nModifyFlag & ModifyFlags::Regexp))
        m_xRegExpBtn->set_active(m_pSearchItem->GetRegExp());
    if (!(m_nModifyFlag & ModifyFlags::Wildcard))
        m_xWildcardBtn->set_active(m_pSearchItem->GetWildcard());
    if (!(m_nModifyFlag & ModifyFlags::Layout))
        m_xLayoutBtn->set_active(m_pSearchItem->GetPattern());
    if (m_xNotesBtn->get_active())
        m_xLayoutBtn->set_sensitive(false);
    m_xSimilarityBox->set_active(m_pSearchItem->IsLevenshtein());
    if ( m_xJapOptionsCB->get_visible() )
        m_xJapOptionsCB->set_active(m_pSearchItem->IsUseAsianOptions());
    m_xIncludeDiacritics->set_active( !aOpt.IsIgnoreDiacritics_CTL() );
    if ( m_xIncludeKashida->get_visible() )
        m_xIncludeKashida->set_active( !aOpt.IsIgnoreKashida_CTL() );
    ApplyTransliterationFlags_Impl(m_pSearchItem->GetTransliterationFlags());

    ShowOptionalControls_Impl();

    if (m_pSearchItem->GetAppFlag() == SvxSearchApp::CALC)
    {
        m_xCalcGrid->show();
        m_xSearchFormattedCB->set_active( aOpt.IsSearchFormatted() );
        Link<weld::Toggleable&,void> aLink = LINK( this, SvxSearchDialog, FlagHdl_Impl );
        m_xCalcSearchInLB->connect_changed( LINK( this, SvxSearchDialog, LBSelectHdl_Impl ) );
        m_xRowsBtn->connect_toggled( aLink );
        m_xColumnsBtn->connect_toggled( aLink );
        m_xAllSheetsCB->connect_toggled( aLink );
        m_xSearchFormattedCB->connect_toggled( aLink );

        ModifyFlags nModifyFlagCheck;
        switch (m_pSearchItem->GetCellType())
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
        if (!(m_nModifyFlag & nModifyFlagCheck))
            m_xCalcSearchInLB->set_active(static_cast<sal_Int32>(m_pSearchItem->GetCellType()));

        m_xWordBtn->set_label(m_sCalcStr.getToken(0, '#'));

        if (m_pSearchItem->GetRowDirection() && !(m_nModifyFlag & ModifyFlags::Rows))
            m_xRowsBtn->set_active(true);
        else if (!m_pSearchItem->GetRowDirection() && !(m_nModifyFlag & ModifyFlags::Columns))
            m_xColumnsBtn->set_active(true);

        if (!(m_nModifyFlag & ModifyFlags::AllTables))
            m_xAllSheetsCB->set_active(m_pSearchItem->IsAllTables());

        // only look for formatting in Writer
        m_xFormatBtn->hide();
        m_xNoFormatBtn->hide();
        m_xAttributeBtn->hide();
    }
    else
    {
        m_xSearchFormattedCB->hide();
        m_xWordBtn->set_label(m_sCalcStr.getToken(1, '#'));

        if (m_pSearchItem->GetAppFlag() == SvxSearchApp::DRAW)
        {
            m_xSearchAllBtn->hide();

            m_xRegExpBtn->hide();
            m_xWildcardBtn->hide();
            m_xLayoutBtn->hide();

            // only look for formatting in Writer
            m_xFormatBtn->hide();
            m_xNoFormatBtn->hide();
            m_xAttributeBtn->hide();
        }
        else
        {
            m_xWildcardBtn->hide(); /* TODO:WILDCARD do not hide for other apps if they handle it */

            if (!m_pSearchList)
            {
                // Get attribute sets, if it not has been done already
                const SfxPoolItem* ppArgs[] = { m_pSearchItem.get(), nullptr };
                SfxPoolItemHolder aResult(m_rBindings.GetDispatcher()->Execute(
                    FID_SEARCH_SEARCHSET, SfxCallMode::SLOT, ppArgs));
                const SvxSetItem* pSrchSetItem(static_cast<const SvxSetItem*>(aResult.getItem()));

                if ( pSrchSetItem )
                    InitAttrList_Impl( &pSrchSetItem->GetItemSet(), nullptr );

                aResult = m_rBindings.GetDispatcher()->Execute(FID_SEARCH_REPLACESET,
                                                               SfxCallMode::SLOT, ppArgs);
                const SvxSetItem* pReplSetItem(static_cast<const SvxSetItem*>(aResult.getItem()));

                if ( pReplSetItem )
                    InitAttrList_Impl( nullptr, &pReplSetItem->GetItemSet() );
            }
        }
    }

    // similarity search?
    if (!(m_nModifyFlag & ModifyFlags::Similarity))
        m_xSimilarityBox->set_active(m_pSearchItem->IsLevenshtein());
    m_bSet = true;

    FlagHdl_Impl(*m_xSimilarityBox);
    FlagHdl_Impl(*m_xJapOptionsCB);

    bool bDisableSearch = false;
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if ( pViewShell )
    {
        bool bText = !bSearchPattern;

        if ( pViewShell->HasSelection( bText ) )
            EnableControl_Impl(*m_xSelectionBtn);
        else
        {
            m_xSelectionBtn->set_active( false );
            m_xSelectionBtn->set_sensitive(false);
        }
    }

    // Pattern Search and there were no AttrSets given
    if ( bSearchPattern )
    {
        SfxObjectShell* pShell = SfxObjectShell::Current();

        if ( pShell && pShell->GetStyleSheetPool() )
        {
            // Templates designed
            m_xSearchTmplLB->clear();
            m_xReplaceTmplLB->clear();
            SfxStyleSheetBasePool* pStylePool = pShell->GetStyleSheetPool();
            SfxStyleSheetBase* pBase = pStylePool->First(m_pSearchItem->GetFamily());

            while ( pBase )
            {
                if ( pBase->IsUsed() )
                    m_xSearchTmplLB->append_text( pBase->GetName() );
                m_xReplaceTmplLB->append_text( pBase->GetName() );
                pBase = pStylePool->Next();
            }
            m_xSearchTmplLB->set_active_text(m_pSearchItem->GetSearchString());
            m_xReplaceTmplLB->set_active_text(m_pSearchItem->GetReplaceString());
        }
        m_xSearchTmplLB->show();

        if (m_bConstruct)
            // Grab focus only after creating
            m_xSearchTmplLB->grab_focus();
        m_xReplaceTmplLB->show();
        m_xSearchLB->hide();
        m_xReplaceLB->hide();

        m_xWordBtn->set_sensitive(false);
        m_xRegExpBtn->set_sensitive(false);
        m_xWildcardBtn->set_sensitive(false);
        m_xMatchCaseCB->set_sensitive(false);

        bDisableSearch = !m_xSearchTmplLB->get_count();
    }
    else
    {
        bool bSetSearch = !(m_nModifyFlag & ModifyFlags::Search);
        bool bSetReplace = !(m_nModifyFlag & ModifyFlags::Replace);

        if (!(m_pSearchItem->GetSearchString().isEmpty()) && bSetSearch)
            m_xSearchLB->set_entry_text(m_pSearchItem->GetSearchString());
        else if (!m_aSearchStrings.empty())
        {
            bool bAttributes = ((m_pSearchList && m_pSearchList->Count())
                                || (m_pReplaceList && m_pReplaceList->Count()));

            if ( bSetSearch && !bAttributes )
                m_xSearchLB->set_entry_text(m_aSearchStrings[0]);

            OUString aReplaceTxt = m_pSearchItem->GetReplaceString();

            if (!m_aReplaceStrings.empty())
                aReplaceTxt = m_aReplaceStrings[0];

            if ( bSetReplace && !bAttributes )
                m_xReplaceLB->set_entry_text( aReplaceTxt );
        }
        m_xSearchLB->show();

        if (m_bConstruct)
            // Grab focus only after creating
            m_xSearchLB->grab_focus();
        m_xReplaceLB->show();
        m_xSearchTmplLB->hide();
        m_xReplaceTmplLB->hide();

        EnableControl_Impl(*m_xRegExpBtn);
        EnableControl_Impl(*m_xWildcardBtn);
        EnableControl_Impl(*m_xMatchCaseCB);

        if ( m_xRegExpBtn->get_active() )
            m_xWordBtn->set_sensitive(false);
        else
            EnableControl_Impl(*m_xWordBtn);

        bDisableSearch = m_xSearchLB->get_active_text().isEmpty() &&
            m_xSearchAttrText->get_label().isEmpty();
    }
    FocusHdl_Impl(*m_xSearchLB);

    if ( bDisableSearch )
    {
        m_xSearchBtn->set_sensitive(false);
        m_xBackSearchBtn->set_sensitive(false);
        m_xSearchAllBtn->set_sensitive(false);
        m_xReplaceBtn->set_sensitive(false);
        m_xReplaceAllBtn->set_sensitive(false);
        m_xComponentFrame->set_sensitive(false);
    }
    else
    {
        EnableControl_Impl(*m_xSearchBtn);
        EnableControl_Impl(*m_xBackSearchBtn);
        EnableControl_Impl(*m_xReplaceBtn);
        if (!m_bWriter || !m_xNotesBtn->get_active())
        {
            EnableControl_Impl(*m_xSearchAllBtn);
            EnableControl_Impl(*m_xReplaceAllBtn);
        }
        if (m_bWriter && m_pSearchItem->GetNotes())
        {
            m_xSearchAllBtn->set_sensitive(false);
            m_xReplaceAllBtn->set_sensitive(false);
        }
    }

    if (!m_xSearchAttrText->get_label().isEmpty())
        EnableControl_Impl(*m_xNoFormatBtn);
    else
        m_xNoFormatBtn->set_sensitive(false);

    if (!m_pSearchList)
    {
        m_xAttributeBtn->set_sensitive(false);
        m_xFormatBtn->set_sensitive(false);
    }

    if ( m_xLayoutBtn->get_active() )
    {
        m_pImpl->bSaveToModule = false;
        TemplateHdl_Impl(*m_xLayoutBtn);
        m_pImpl->bSaveToModule = true;
    }
}


void SvxSearchDialog::InitAttrList_Impl( const SfxItemSet* pSSet,
                                         const SfxItemSet* pRSet )
{
    if ( !pSSet && !pRSet )
        return;

    if (m_pImpl->pRanges.empty() && pSSet)
        m_pImpl->pRanges = pSSet->GetRanges();

    bool bSetOptimalLayoutSize = false;

    // See to it that are the texts of the attributes are correct
    OUString aDesc;

    if ( pSSet )
    {
        m_pSearchList.reset(new SearchAttrItemList);

        if ( pSSet->Count() )
        {
            m_pSearchList->Put(*pSSet);

            m_xSearchAttrText->set_label( BuildAttrText_Impl( aDesc, true ) );

            if ( !aDesc.isEmpty() )
            {
                if (!m_xSearchAttrText->get_visible())
                {
                    m_xSearchAttrText->show();
                    bSetOptimalLayoutSize = true;
                }
                m_bFormat |= true;
            }
        }
    }

    if ( pRSet )
    {
        m_pReplaceList.reset(new SearchAttrItemList);

        if ( pRSet->Count() )
        {
            m_pReplaceList->Put(*pRSet);

            m_xReplaceAttrText->set_label( BuildAttrText_Impl( aDesc, false ) );

            if ( !aDesc.isEmpty() )
            {
                if (!m_xReplaceAttrText->get_visible())
                {
                    m_xReplaceAttrText->show();
                    bSetOptimalLayoutSize = true;
                }
                m_bFormat |= true;
            }
        }
    }

    if (bSetOptimalLayoutSize)
        m_xDialog->resize_to_request();
}

IMPL_LINK( SvxSearchDialog, LBSelectHdl_Impl, weld::ComboBox&, rCtrl, void )
{
    ClickHdl_Impl(&rCtrl);
}

IMPL_LINK( SvxSearchDialog, FlagHdl_Impl, weld::Toggleable&, rCtrl, void )
{
    ClickHdl_Impl(&rCtrl);
}

void SvxSearchDialog::ClickHdl_Impl(const weld::Widget* pCtrl)
{
    if (pCtrl && !m_bSet)
        SetModifyFlag_Impl(pCtrl);
    else
        m_bSet = false;

    if (pCtrl == m_xSimilarityBox.get())
    {
        bool bIsChecked = m_xSimilarityBox->get_active();

        if ( bIsChecked )
        {
            m_xSimilarityBtn->set_sensitive(true);
            m_xRegExpBtn->set_active( false );
            m_xRegExpBtn->set_sensitive(false);
            m_xWildcardBtn->set_active( false );
            m_xWildcardBtn->set_sensitive(false);
            EnableControl_Impl(*m_xWordBtn);

            if ( m_xLayoutBtn->get_active() )
            {
                EnableControl_Impl(*m_xMatchCaseCB);
                m_xLayoutBtn->set_active( false );
            }
            m_xRegExpBtn->set_sensitive(false);
            m_xWildcardBtn->set_sensitive(false);
            m_xLayoutBtn->set_sensitive(false);
            m_xFormatBtn->set_sensitive(false);
            m_xNoFormatBtn->set_sensitive(false);
            m_xAttributeBtn->set_sensitive(false);
        }
        else
        {
            EnableControl_Impl(*m_xRegExpBtn);
            EnableControl_Impl(*m_xWildcardBtn);
            if (!m_xNotesBtn->get_active())
                EnableControl_Impl(*m_xLayoutBtn);
            EnableControl_Impl(*m_xFormatBtn);
            EnableControl_Impl(*m_xAttributeBtn);
            m_xSimilarityBtn->set_sensitive(false);
        }
        m_pSearchItem->SetLevenshtein(bIsChecked);
    }
    else if (pCtrl == m_xNotesBtn.get())
    {
        if (m_xNotesBtn->get_active())
        {
            m_xLayoutBtn->set_sensitive(false);
            m_xSearchAllBtn->set_sensitive(false);
            m_xReplaceAllBtn->set_sensitive(false);
        }
        else
        {
            EnableControl_Impl(*m_xLayoutBtn);
            ModifyHdl_Impl(*m_xSearchLB);
        }
    }
    else
    {
        if (m_xLayoutBtn->get_active() && !m_bFormat)
        {
            m_xWordBtn->set_active( false );
            m_xWordBtn->set_sensitive(false);
            m_xRegExpBtn->set_active( false );
            m_xRegExpBtn->set_sensitive(false);
            m_xWildcardBtn->set_active( false );
            m_xWildcardBtn->set_sensitive(false);
            m_xMatchCaseCB->set_active( false );
            m_xMatchCaseCB->set_sensitive(false);
            m_xNotesBtn->set_sensitive(false);

            if ( m_xSearchTmplLB->get_count() )
            {
                EnableControl_Impl(*m_xSearchBtn);
                EnableControl_Impl(*m_xBackSearchBtn);
                EnableControl_Impl(*m_xSearchAllBtn);
                EnableControl_Impl(*m_xReplaceBtn);
                EnableControl_Impl(*m_xReplaceAllBtn);
            }
        }
        else
        {
            EnableControl_Impl(*m_xRegExpBtn);
            EnableControl_Impl(*m_xWildcardBtn);
            EnableControl_Impl(*m_xMatchCaseCB);
            EnableControl_Impl(*m_xNotesBtn);

            if ( m_xRegExpBtn->get_active() )
            {
                m_xWordBtn->set_active( false );
                m_xWordBtn->set_sensitive(false);
                m_xWildcardBtn->set_active( false );
                m_xWildcardBtn->set_sensitive(false);
                m_xSimilarityBox->set_active( false );
                m_xSimilarityBox->set_sensitive(false);
                m_xSimilarityBtn->set_sensitive(false);
            }
            else if ( m_xWildcardBtn->get_active() )
            {
                m_xRegExpBtn->set_active( false );
                m_xRegExpBtn->set_sensitive(false);
                m_xSimilarityBox->set_active( false );
                m_xSimilarityBox->set_sensitive(false);
                m_xSimilarityBtn->set_sensitive(false);
            }
            else
            {
                EnableControl_Impl(*m_xWordBtn);
                EnableControl_Impl(*m_xSimilarityBox);
            }

            // Search-string in place? then enable Buttons
            m_bSet = true;
            ModifyHdl_Impl(*m_xSearchLB);
        }
    }

    if (pCtrl == m_xAllSheetsCB.get())
    {
        m_bSet = true;
        ModifyHdl_Impl(*m_xSearchLB);
    }

    if (pCtrl == m_xJapOptionsCB.get())
    {
        bool bEnableJapOpt = m_xJapOptionsCB->get_active();
        m_xMatchCaseCB->set_sensitive(!bEnableJapOpt );
        m_xJapMatchFullHalfWidthCB->set_sensitive(!bEnableJapOpt );
        m_xJapOptionsBtn->set_sensitive( bEnableJapOpt );
    }

    if (m_pImpl->bSaveToModule)
        SaveToModule_Impl();
}

IMPL_LINK(SvxSearchDialog, CommandHdl_Impl, weld::Button&, rBtn, void)
{
    bool bInclusive = (m_xLayoutBtn->get_label() == m_sLayoutStr);

    if ( ( &rBtn == m_xSearchBtn.get() )   ||
         (&rBtn == m_xBackSearchBtn.get())  ||
         ( &rBtn == m_xSearchAllBtn.get() )||
         ( &rBtn == m_xReplaceBtn.get() )  ||
         ( &rBtn == m_xReplaceAllBtn.get() ) )
    {
        if ( m_xLayoutBtn->get_active() && !bInclusive )
        {
            m_pSearchItem->SetSearchString(m_xSearchTmplLB->get_active_text());
            m_pSearchItem->SetReplaceString(m_xReplaceTmplLB->get_active_text());
        }
        else
        {
            m_pSearchItem->SetSearchString(m_xSearchLB->get_active_text());
            m_pSearchItem->SetReplaceString(m_xReplaceLB->get_active_text());

            if ( &rBtn == m_xReplaceBtn.get() )
                Remember_Impl(false);
            else
            {
                Remember_Impl(true);

                if ( &rBtn == m_xReplaceAllBtn.get() )
                    Remember_Impl(false);
            }
        }

        m_pSearchItem->SetRegExp(false);
        m_pSearchItem->SetWildcard(false);
        m_pSearchItem->SetLevenshtein(false);
        if (GetCheckBoxValue(*m_xRegExpBtn))
            m_pSearchItem->SetRegExp(true);
        else if (GetCheckBoxValue(*m_xWildcardBtn))
            m_pSearchItem->SetWildcard(true);
        else if (GetCheckBoxValue(*m_xSimilarityBox))
            m_pSearchItem->SetLevenshtein(true);

        m_pSearchItem->SetWordOnly(GetCheckBoxValue(*m_xWordBtn));

        bool bSetBackwards = false;
        if( &rBtn == m_xBackSearchBtn.get())
        {
            bSetBackwards = true;
        }
        else if( &rBtn == m_xReplaceBtn.get())
        {
            bSetBackwards = GetCheckBoxValue(*m_xReplaceBackwardsCB);
            m_bReplaceBackwards = GetCheckBoxValue(*m_xReplaceBackwardsCB);
        }

        m_pSearchItem->SetBackward(bSetBackwards);

        m_pSearchItem->SetNotes(GetCheckBoxValue(*m_xNotesBtn));
        m_pSearchItem->SetPattern(GetCheckBoxValue(*m_xLayoutBtn));
        m_pSearchItem->SetSelection(GetCheckBoxValue(*m_xSelectionBtn));
        m_pSearchItem->SetUseAsianOptions(GetCheckBoxValue(*m_xJapOptionsCB));
        TransliterationFlags nFlags = GetTransliterationFlags();
        if (!m_pSearchItem->IsUseAsianOptions())
            nFlags &= TransliterationFlags::IGNORE_CASE |
                      TransliterationFlags::IGNORE_WIDTH;
        if (GetNegatedCheckBoxValue(*m_xIncludeDiacritics))
            nFlags |= TransliterationFlags::IGNORE_DIACRITICS_CTL;
        if (GetNegatedCheckBoxValue(*m_xIncludeKashida))
            nFlags |= TransliterationFlags::IGNORE_KASHIDA_CTL;
        m_pSearchItem->SetTransliterationFlags(nFlags);

        if (!m_bWriter)
        {
            if ( m_xCalcSearchInLB->get_active() != -1)
                m_pSearchItem->SetCellType(
                    static_cast<SvxSearchCellType>(m_xCalcSearchInLB->get_active()));

            m_pSearchItem->SetRowDirection(m_xRowsBtn->get_active());
            m_pSearchItem->SetAllTables(m_xAllSheetsCB->get_active());
            m_pSearchItem->SetSearchFormatted(m_xSearchFormattedCB->get_active());
        }

        if ((&rBtn == m_xSearchBtn.get()) ||  (&rBtn == m_xBackSearchBtn.get()))
            m_pSearchItem->SetCommand(SvxSearchCmd::FIND);
        else if ( &rBtn == m_xSearchAllBtn.get() )
            m_pSearchItem->SetCommand(SvxSearchCmd::FIND_ALL);
        else if ( &rBtn == m_xReplaceBtn.get() )
            m_pSearchItem->SetCommand(SvxSearchCmd::REPLACE);
        else if ( &rBtn == m_xReplaceAllBtn.get() )
            m_pSearchItem->SetCommand(SvxSearchCmd::REPLACE_ALL);

        // when looking for templates, delete format lists
        if (!m_bFormat && m_pSearchItem->GetPattern())
        {
            if (m_pSearchList)
                m_pSearchList->Clear();

            if (m_pReplaceList)
                m_pReplaceList->Clear();
        }
        m_nModifyFlag = ModifyFlags::NONE;
        const SfxPoolItem* ppArgs[] = { m_pSearchItem.get(), nullptr };
        m_rBindings.ExecuteSynchron(FID_SEARCH_NOW, ppArgs);
    }
    else if ( &rBtn == m_xCloseBtn.get() )
    {
        if ( !m_xLayoutBtn->get_active() || bInclusive )
        {
            Remember_Impl(true);
            Remember_Impl(false);
        }
        SaveToModule_Impl();
        Close();
    }
    else if (&rBtn == m_xSimilarityBtn.get())
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxSearchSimilarityDialog> pDlg(pFact->CreateSvxSearchSimilarityDialog(
            m_xDialog.get(), m_pSearchItem->IsLEVRelaxed(), m_pSearchItem->GetLEVOther(),
            m_pSearchItem->GetLEVShorter(), m_pSearchItem->GetLEVLonger()));
        if ( executeSubDialog(pDlg.get()) == RET_OK )
        {
            m_pSearchItem->SetLEVRelaxed(pDlg->IsRelaxed());
            m_pSearchItem->SetLEVOther(pDlg->GetOther());
            m_pSearchItem->SetLEVShorter(pDlg->GetShorter());
            m_pSearchItem->SetLEVLonger(pDlg->GetLonger());
            SaveToModule_Impl();
        }
    }
    else if (&rBtn == m_xJapOptionsBtn.get())
    {
        SfxItemSet aSet( SfxGetpApp()->GetPool() );
        m_pSearchItem->SetTransliterationFlags(GetTransliterationFlags());
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxJSearchOptionsDialog> aDlg(pFact->CreateSvxJSearchOptionsDialog(
            m_xDialog.get(), aSet, m_pSearchItem->GetTransliterationFlags()));
        int nRet = executeSubDialog(aDlg.get());
        if (RET_OK == nRet) //! true only if FillItemSet of SvxJSearchOptionsPage returns true
        {
            TransliterationFlags nFlags = aDlg->GetTransliterationFlags();
            m_pSearchItem->SetTransliterationFlags(nFlags);
            ApplyTransliterationFlags_Impl( nFlags );
        }
    }
    else if (&rBtn == m_xSearchComponent1PB.get() || &rBtn == m_xSearchComponent2PB.get())
    {
        uno::Sequence < beans::PropertyValue > aArgs(2);
        beans::PropertyValue* pArgs = aArgs.getArray();
        pArgs[0].Name = "SearchString";
        pArgs[0].Value <<= m_xSearchLB->get_active_text();
        pArgs[1].Name = "ParentWindow";
        pArgs[1].Value <<= m_xDialog->GetXWindow();
        if (&rBtn == m_xSearchComponent1PB.get())
        {
            if (m_pImpl->xCommand1Dispatch.is())
                m_pImpl->xCommand1Dispatch->dispatch(m_pImpl->aCommand1URL, aArgs);
        }
        else
        {
            if (m_pImpl->xCommand2Dispatch.is())
                m_pImpl->xCommand2Dispatch->dispatch(m_pImpl->aCommand2URL, aArgs);
        }
    }
}


IMPL_LINK( SvxSearchDialog, ModifyHdl_Impl, weld::ComboBox&, rEd, void )
{
    if (!m_bSet)
        SetModifyFlag_Impl( &rEd );
    else
        m_bSet = false;

    // Calc allows searching for empty cells.
    bool bAllowEmptySearch = (m_pSearchItem->GetAppFlag() == SvxSearchApp::CALC);

    if (&rEd != m_xSearchLB.get() && &rEd != m_xReplaceLB.get())
        return;

    sal_Int32 nSrchTxtLen = m_xSearchLB->get_active_text().getLength();
    sal_Int32 nReplTxtLen = 0;
    if (bAllowEmptySearch)
        nReplTxtLen = m_xReplaceLB->get_active_text().getLength();
    sal_Int32 nAttrTxtLen = m_xSearchAttrText->get_label().getLength();

    if (nSrchTxtLen || nReplTxtLen || nAttrTxtLen)
    {
        EnableControl_Impl(*m_xSearchBtn);
        EnableControl_Impl(*m_xBackSearchBtn);
        EnableControl_Impl(*m_xReplaceBtn);
        if (!m_bWriter || !m_xNotesBtn->get_active())
        {
            EnableControl_Impl(*m_xSearchAllBtn);
            EnableControl_Impl(*m_xReplaceAllBtn);
        }
    }
    else
    {
        m_xComponentFrame->set_sensitive(false);
        m_xSearchBtn->set_sensitive(false);
        m_xBackSearchBtn->set_sensitive(false);
        m_xSearchAllBtn->set_sensitive(false);
        m_xReplaceBtn->set_sensitive(false);
        m_xReplaceAllBtn->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxSearchDialog, TemplateHdl_Impl, weld::Toggleable&, void)
{
    if (m_pImpl->bSaveToModule)
        SaveToModule_Impl();

    if (m_bFormat)
        return;
    OUString sDesc;

    if ( m_xLayoutBtn->get_active() )
    {
        if (!m_pFamilyController)
        {
            sal_uInt16 nId = 0;

            // Enable templates controller
            switch (m_pSearchItem->GetFamily())
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

            m_rBindings.EnterRegistrations();
            m_pFamilyController.reset(new SvxSearchController(nId, m_rBindings, *this));
            m_rBindings.LeaveRegistrations();
            m_xSearchTmplLB->clear();
            m_xReplaceTmplLB->clear();

            m_xSearchTmplLB->show();
            m_xReplaceTmplLB->show();
            m_xSearchLB->hide();
            m_xReplaceLB->hide();

            m_xSearchAttrText->set_label( sDesc );
            m_xReplaceAttrText->set_label( sDesc );

            if(!sDesc.isEmpty())
            {
                if (!m_xSearchAttrText->get_visible() || !m_xReplaceAttrText->get_visible())
                {
                    m_xSearchAttrText->show();
                    m_xReplaceAttrText->show();
                    m_xDialog->resize_to_request();
                }
            }
        }
        m_xFormatBtn->set_sensitive(false);
        m_xNoFormatBtn->set_sensitive(false);
        m_xAttributeBtn->set_sensitive(false);
        m_xSimilarityBox->set_sensitive(false);
        m_xSimilarityBtn->set_sensitive(false);
    }
    else
    {
        // Disable templates controller
        m_rBindings.EnterRegistrations();
        m_pFamilyController.reset();
        m_rBindings.LeaveRegistrations();

        m_xSearchLB->show();
        m_xReplaceLB->show();
        m_xSearchTmplLB->hide();
        m_xReplaceTmplLB->hide();

        m_xSearchAttrText->set_label( BuildAttrText_Impl( sDesc, true ) );
        m_xReplaceAttrText->set_label( BuildAttrText_Impl( sDesc, false ) );

        if(!sDesc.isEmpty())
        {
            if (!m_xSearchAttrText->get_visible() || !m_xReplaceAttrText->get_visible())
            {
                m_xSearchAttrText->show();
                m_xReplaceAttrText->show();
                m_xDialog->resize_to_request();
            }
        }

        EnableControl_Impl(*m_xFormatBtn);
        EnableControl_Impl(*m_xAttributeBtn);
        EnableControl_Impl(*m_xSimilarityBox);

        FocusHdl_Impl(m_bSearch ? *m_xSearchLB : *m_xReplaceLB);
    }
    m_bSet = true;
    m_pImpl->bSaveToModule = false;
    FlagHdl_Impl(*m_xLayoutBtn);
    m_pImpl->bSaveToModule = true;
}

void SvxSearchDialog::Remember_Impl(bool _bSearch)
{
    std::vector<OUString>& rArr = _bSearch ? m_aSearchStrings : m_aReplaceStrings;
    weld::ComboBox& rListBox = _bSearch ? *m_xSearchLB : *m_xReplaceLB;

    const OUString sText = rListBox.get_active_text();
    if (sText.isEmpty())
        return;

    // tdf#154818 - rearrange the search items
    const int nPos = rListBox.find_text(sText);
    if (nPos == 0 && !rArr.empty() && rArr.at(0) == sText)
        // nothing to do, is already the first item
        return;

    if (nPos != -1)
    {
        rListBox.remove(nPos);
        rArr.erase(rArr.begin() + nPos);
    }
    else if (rListBox.get_count() >= m_nRememberSize)
    {
        // delete oldest entry at maximum occupancy (ListBox and Array)
        rListBox.remove(m_nRememberSize - 1);
        rArr.erase(rArr.begin() + m_nRememberSize - 1);
    }

    rArr.insert(rArr.begin(), sText);
    rListBox.insert_text(0, sText);
}

void SvxSearchDialog::TemplatesChanged_Impl( SfxStyleSheetBasePool& rPool )
{
    OUString aOldSrch( m_xSearchTmplLB->get_active_text() );
    OUString aOldRepl( m_xReplaceTmplLB->get_active_text() );
    m_xSearchTmplLB->clear();
    m_xReplaceTmplLB->clear();
    m_xSearchTmplLB->freeze();
    m_xReplaceTmplLB->freeze();
    SfxStyleSheetBase* pBase = rPool.First(m_pSearchItem->GetFamily());

    while ( pBase )
    {
        if ( pBase->IsUsed() )
            m_xSearchTmplLB->append_text( pBase->GetName() );
        m_xReplaceTmplLB->append_text( pBase->GetName() );
        pBase = rPool.Next();
    }
    m_xSearchTmplLB->thaw();
    m_xReplaceTmplLB->thaw();
    m_xSearchTmplLB->set_active(0);

    if ( !aOldSrch.isEmpty() )
        m_xSearchTmplLB->set_active_text( aOldSrch );
    m_xReplaceTmplLB->set_active(0);

    if ( !aOldRepl.isEmpty() )
        m_xReplaceTmplLB->set_active_text( aOldRepl );

    if ( m_xSearchTmplLB->get_count() )
    {
        EnableControl_Impl(*m_xSearchBtn);
        EnableControl_Impl(*m_xBackSearchBtn);
        EnableControl_Impl(*m_xSearchAllBtn);
        EnableControl_Impl(*m_xReplaceBtn);
        EnableControl_Impl(*m_xReplaceAllBtn);
    }
}


void SvxSearchDialog::EnableControls_Impl( const SearchOptionFlags nFlags )
{
    if (nFlags == m_nOptions)
        return;
    else
        m_nOptions = nFlags;

    bool bNoSearch = true;

    bool bEnableSearch = bool(SearchOptionFlags::SEARCH & m_nOptions);
    m_xSearchBtn->set_sensitive(bEnableSearch);
    m_xBackSearchBtn->set_sensitive(bEnableSearch);

    if( bEnableSearch )
        bNoSearch = false;

    if (SearchOptionFlags::SEARCHALL & m_nOptions)
    {
        m_xSearchAllBtn->set_sensitive(true);
        bNoSearch = false;
    }
    else
        m_xSearchAllBtn->set_sensitive(false);
    if (SearchOptionFlags::REPLACE & m_nOptions)
    {
        m_xReplaceBtn->set_sensitive(true);
        m_xReplaceFrame->set_sensitive(true);
        m_xReplaceLB->set_sensitive(true);
        m_xReplaceTmplLB->set_sensitive(true);
        bNoSearch = false;
    }
    else
    {
        m_xReplaceBtn->set_sensitive(false);
        m_xReplaceFrame->set_sensitive(false);
        m_xReplaceLB->set_sensitive(false);
        m_xReplaceTmplLB->set_sensitive(false);
    }
    if (SearchOptionFlags::REPLACE_ALL & m_nOptions)
    {
        m_xReplaceAllBtn->set_sensitive(true);
        bNoSearch = false;
    }
    else
        m_xReplaceAllBtn->set_sensitive(false);
    m_xComponentFrame->set_sensitive(!bNoSearch);
    m_xSearchBtn->set_sensitive( !bNoSearch );
    m_xBackSearchBtn->set_sensitive( !bNoSearch );
    m_xSearchFrame->set_sensitive( !bNoSearch );
    m_xSearchLB->set_sensitive( !bNoSearch );
    m_xNotesBtn->set_sensitive(true);

    if (SearchOptionFlags::WHOLE_WORDS & m_nOptions)
        m_xWordBtn->set_sensitive(true);
    else
        m_xWordBtn->set_sensitive(false);
    if (SearchOptionFlags::BACKWARDS & m_nOptions)
    {
        m_xBackSearchBtn->set_sensitive(true);
        m_xReplaceBackwardsCB->set_sensitive(true);
    }
    else
    {
        m_xBackSearchBtn->set_sensitive(false);
        m_xReplaceBackwardsCB->set_sensitive(false);
    }
    if (SearchOptionFlags::REG_EXP & m_nOptions)
        m_xRegExpBtn->set_sensitive(true);
    else
        m_xRegExpBtn->set_sensitive(false);
    if (SearchOptionFlags::WILDCARD & m_nOptions)
        m_xWildcardBtn->set_sensitive(true);
    else
        m_xWildcardBtn->set_sensitive(false);
    if (SearchOptionFlags::EXACT & m_nOptions)
        m_xMatchCaseCB->set_sensitive(true);
    else
        m_xMatchCaseCB->set_sensitive(false);
    if (SearchOptionFlags::SELECTION & m_nOptions)
        m_xSelectionBtn->set_sensitive(true);
    else
        m_xSelectionBtn->set_sensitive(false);
    if (SearchOptionFlags::FAMILIES & m_nOptions)
        m_xLayoutBtn->set_sensitive(true);
    else
        m_xLayoutBtn->set_sensitive(false);
    if (SearchOptionFlags::FORMAT & m_nOptions)
    {
        m_xAttributeBtn->set_sensitive(true);
        m_xFormatBtn->set_sensitive(true);
        m_xNoFormatBtn->set_sensitive(true);
    }
    else
    {
        m_xAttributeBtn->set_sensitive(false);
        m_xFormatBtn->set_sensitive(false);
        m_xNoFormatBtn->set_sensitive(false);
    }

    if (SearchOptionFlags::SIMILARITY & m_nOptions)
    {
        m_xSimilarityBox->set_sensitive(true);
        m_xSimilarityBtn->set_sensitive(true);
    }
    else
    {
        m_xSimilarityBox->set_sensitive(false);
        m_xSimilarityBtn->set_sensitive(false);
    }

    if (m_pSearchItem)
    {
        Init_Impl(m_pSearchItem->GetPattern() && (!m_pSearchList || !m_pSearchList->Count()));
        if (SvxSearchDialog::IsOtherOptionsExpanded())
            m_xOtherOptionsExpander->set_expanded(true);
    }
}

void SvxSearchDialog::EnableControl_Impl(const weld::Widget& rCtrl)
{
    if (m_xSearchBtn.get() == &rCtrl && (SearchOptionFlags::SEARCH & m_nOptions))
    {
        m_xComponentFrame->set_sensitive(true);
        m_xSearchBtn->set_sensitive(true);
        return;
    }
    if (m_xSearchAllBtn.get() == &rCtrl && (SearchOptionFlags::SEARCHALL & m_nOptions))
    {
        m_xSearchAllBtn->set_sensitive(true);
        return;
    }
    if (m_xReplaceBtn.get() == &rCtrl && (SearchOptionFlags::REPLACE & m_nOptions))
    {
        m_xReplaceBtn->set_sensitive(true);
        return;
    }
    if (m_xReplaceAllBtn.get() == &rCtrl && (SearchOptionFlags::REPLACE_ALL & m_nOptions))
    {
        m_xReplaceAllBtn->set_sensitive(true);
        return;
    }
    if (m_xWordBtn.get() == &rCtrl && (SearchOptionFlags::WHOLE_WORDS & m_nOptions))
    {
        m_xWordBtn->set_sensitive(true);
        return;
    }
    if (SearchOptionFlags::BACKWARDS & m_nOptions)
    {
        if( m_xBackSearchBtn.get() == &rCtrl )
        {
            m_xBackSearchBtn->set_sensitive(true);
            return;
        }
        else if ( m_xReplaceBackwardsCB.get() == &rCtrl )
        {
            m_xReplaceBackwardsCB->set_sensitive(true);
            return;
        }
    }
    if (m_xNotesBtn.get() == &rCtrl)
    {
        m_xNotesBtn->set_sensitive(true);
        return;
    }
    if (m_xRegExpBtn.get() == &rCtrl && (SearchOptionFlags::REG_EXP & m_nOptions)
        && !m_xSimilarityBox->get_active() && !m_xWildcardBtn->get_active())
    {
        m_xRegExpBtn->set_sensitive(true);
        return;
    }
    if (m_xWildcardBtn.get() == &rCtrl && (SearchOptionFlags::WILDCARD & m_nOptions)
        && !m_xSimilarityBox->get_active() && !m_xRegExpBtn->get_active())
    {
        m_xWildcardBtn->set_sensitive(true);
        return;
    }
    if (m_xMatchCaseCB.get() == &rCtrl && (SearchOptionFlags::EXACT & m_nOptions))
    {
        if (!m_xJapOptionsCB->get_active())
            m_xMatchCaseCB->set_sensitive(true);
        return;
    }
    if (m_xSelectionBtn.get() == &rCtrl && (SearchOptionFlags::SELECTION & m_nOptions))
    {
        m_xSelectionBtn->set_sensitive(true);
        return;
    }
    if (m_xLayoutBtn.get() == &rCtrl && (SearchOptionFlags::FAMILIES & m_nOptions))
    {
        m_xLayoutBtn->set_sensitive(true);
        return;
    }
    if (m_xAttributeBtn.get() == &rCtrl && (SearchOptionFlags::FORMAT & m_nOptions)
        && m_pSearchList)
    {
        m_xAttributeBtn->set_sensitive(m_pImpl->bFocusOnSearch);
    }
    if (m_xFormatBtn.get() == &rCtrl && (SearchOptionFlags::FORMAT & m_nOptions))
    {
        m_xFormatBtn->set_sensitive(true);
        return;
    }
    if (m_xNoFormatBtn.get() == &rCtrl && (SearchOptionFlags::FORMAT & m_nOptions))
    {
        m_xNoFormatBtn->set_sensitive(true);
        return;
    }
    if (m_xSimilarityBox.get() == &rCtrl && (SearchOptionFlags::SIMILARITY & m_nOptions)
        && !m_xRegExpBtn->get_active() && !m_xWildcardBtn->get_active())
    {
        m_xSimilarityBox->set_sensitive(true);

        if ( m_xSimilarityBox->get_active() )
            m_xSimilarityBtn->set_sensitive(true);
    }
}

void SvxSearchDialog::SetItem_Impl( const SvxSearchItem* pItem )
{
    //TODO: save pItem and process later if m_executingSubDialog?
    if ( pItem && !m_executingSubDialog )
    {
        m_pSearchItem.reset(pItem->Clone());
        Init_Impl(m_pSearchItem->GetPattern() && (!m_pSearchList || !m_pSearchList->Count()));
    }
}

IMPL_LINK(SvxSearchDialog, FocusHdl_Impl, weld::Widget&, rControl, void)
{
    sal_Int32 nTxtLen = m_xSearchAttrText->get_label().getLength();
    weld::Widget* pCtrl = &rControl;
    if (pCtrl == m_xSearchLB.get())
    {
        if (pCtrl->has_focus())
            m_pImpl->bFocusOnSearch = true;
        pCtrl = m_xSearchLB.get();
        m_bSearch = true;

        if( nTxtLen )
            EnableControl_Impl(*m_xNoFormatBtn);
        else
            m_xNoFormatBtn->set_sensitive(false);
        EnableControl_Impl(*m_xAttributeBtn);
    }
    else
    {
        m_pImpl->bFocusOnSearch = false;
        pCtrl = m_xReplaceLB.get();
        m_bSearch = false;

        if (!m_xReplaceAttrText->get_label().isEmpty())
            EnableControl_Impl(*m_xNoFormatBtn);
        else
            m_xNoFormatBtn->set_sensitive(false);
        m_xAttributeBtn->set_sensitive(false);
    }
    m_bSet = true;

    weld::ComboBox &rComboBox = dynamic_cast<weld::ComboBox&>(*pCtrl);
    rComboBox.select_entry_region(0, -1);
    ModifyHdl_Impl(rComboBox);

    if (m_bFormat && nTxtLen)
        m_xLayoutBtn->set_label(m_sLayoutStr);
    else
    {
        SvtModuleOptions::EFactory eFactory = getModule(m_rBindings);
        bool bWriterApp =
            eFactory == SvtModuleOptions::EFactory::WRITER ||
            eFactory == SvtModuleOptions::EFactory::WRITERWEB ||
            eFactory == SvtModuleOptions::EFactory::WRITERGLOBAL;
        bool bCalcApp = eFactory == SvtModuleOptions::EFactory::CALC;

        if (bWriterApp)
            m_xLayoutBtn->set_label(m_sLayoutWriterStr);
        else
        {
            if (bCalcApp)
                m_xLayoutBtn->set_label(m_sLayoutCalcStr);
            else
                m_xLayoutBtn->set_label(m_sStylesStr);
        }
    }
}

IMPL_LINK_NOARG(SvxSearchDialog, LoseFocusHdl_Impl, weld::Widget&, void)
{
    SaveToModule_Impl();
}

IMPL_LINK_NOARG(SvxSearchDialog, FormatHdl_Impl, weld::Button&, void)
{
    SfxObjectShell* pSh = SfxObjectShell::Current();

    DBG_ASSERT( pSh, "no DocShell" );

    if (!pSh || m_pImpl->pRanges.empty())
        return;

    SfxItemPool& rPool = pSh->GetPool();
    SfxItemSet aSet(rPool, m_pImpl->pRanges);

    aSet.MergeRange(SID_ATTR_PARA_MODEL, SID_ATTR_PARA_MODEL);

    sal_uInt16 nBrushWhich = pSh->GetPool().GetWhichIDFromSlotID(SID_ATTR_BRUSH);
    aSet.MergeRange(nBrushWhich, nBrushWhich);

    aSet.MergeRange(XATTR_FILL_FIRST, XATTR_FILL_LAST);

    OUString aTxt;

    // ITEM: here we have a problem due to the hand-made ItemSet-like
    // SearchAttrItemList. The state 'invalid' seems to be used as unused
    // marker. It should be changed to use SfxPoolItemHolder and not need
    // that. For now, set by using an own loop to set to that state
    SfxWhichIter aIter(aSet);
    for ( sal_uInt16 nWh = aIter.FirstWhich(); nWh != 0; nWh = aIter.NextWhich() )
        aSet.InvalidateItem(nWh);
    aSet.Put(SvxBrushItem(nBrushWhich));

    if (m_bSearch)
    {
        aTxt = SvxResId( RID_SVXSTR_SEARCH );
        m_pSearchList->Get(aSet);
    }
    else
    {
        aTxt = SvxResId( RID_SVXSTR_REPLACE );
        m_pReplaceList->Get(aSet);
    }
    aSet.DisableItem(SID_ATTR_PARA_MODEL);
    aSet.DisableItem(rPool.GetWhichIDFromSlotID(SID_ATTR_PARA_PAGEBREAK));
    aSet.DisableItem(rPool.GetWhichIDFromSlotID(SID_ATTR_PARA_KEEP));


    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateTabItemDialog(m_xDialog.get(), aSet));
    pDlg->SetText( aTxt );

    if ( executeSubDialog(pDlg.get()) != RET_OK )
        return;

    DBG_ASSERT( pDlg->GetOutputItemSet(), "invalid Output-Set" );
    SfxItemSet aOutSet( *pDlg->GetOutputItemSet() );

    SearchAttrItemList* pList = m_bSearch ? m_pSearchList.get() : m_pReplaceList.get();

    const SfxPoolItem* pItem;
    for( sal_uInt16 n = 0; n < pList->Count(); ++n )
    {
        SearchAttrInfo* pAItem = &pList->GetObject(n);
        if( !IsInvalidItem( pAItem->aItemPtr.getItem() ) &&
            SfxItemState::SET == aOutSet.GetItemState(
                pAItem->aItemPtr.Which(), false, &pItem ) )
        {
            pAItem->aItemPtr = SfxPoolItemHolder(*aOutSet.GetPool(), pItem);
            aOutSet.ClearItem( pAItem->aItemPtr.Which() );
        }
    }

    if( aOutSet.Count() )
        pList->Put( aOutSet );

    PaintAttrText_Impl(); // Set AttributeText in GroupBox
}

IMPL_LINK_NOARG(SvxSearchDialog, NoFormatHdl_Impl, weld::Button&, void)
{
    SvtModuleOptions::EFactory eFactory = getModule(m_rBindings);
    bool bWriterApp =
        eFactory == SvtModuleOptions::EFactory::WRITER ||
        eFactory == SvtModuleOptions::EFactory::WRITERWEB ||
        eFactory == SvtModuleOptions::EFactory::WRITERGLOBAL;
    bool bCalcApp = eFactory == SvtModuleOptions::EFactory::CALC;

    if (bCalcApp)
        m_xLayoutBtn->set_label(m_sLayoutCalcStr);
    else
    {
        if (bWriterApp)
            m_xLayoutBtn->set_label(m_sLayoutWriterStr);
        else
            m_xLayoutBtn->set_label(m_sStylesStr);
    }

    m_bFormat = false;
    m_xLayoutBtn->set_active( false );

    bool bSetOptimalLayoutSize = false;

    if (m_bSearch)
    {
        m_pSearchList->Clear();
        m_xSearchAttrText->set_label( u""_ustr );
        if (m_xSearchAttrText->get_visible())
        {
            m_xSearchAttrText->hide();
            bSetOptimalLayoutSize = true;
        }
    }
    else
    {
        m_pReplaceList->Clear();
        m_xReplaceAttrText->set_label( u""_ustr );
        if (m_xReplaceAttrText->get_visible())
        {
            m_xReplaceAttrText->hide();
            bSetOptimalLayoutSize = true;
        }
    }

    if (bSetOptimalLayoutSize)
        m_xDialog->resize_to_request();

    m_pImpl->bSaveToModule = false;
    TemplateHdl_Impl(*m_xLayoutBtn);
    m_pImpl->bSaveToModule = true;
    m_xNoFormatBtn->set_sensitive(false);
}

IMPL_LINK_NOARG(SvxSearchDialog, AttributeHdl_Impl, weld::Button&, void)
{
    if (!m_pSearchList || m_pImpl->pRanges.empty())
        return;

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<VclAbstractDialog> pDlg(
        pFact->CreateSvxSearchAttributeDialog(m_xDialog.get(), *m_pSearchList, m_pImpl->pRanges));
    executeSubDialog(pDlg.get());
    PaintAttrText_Impl();
}

IMPL_LINK( SvxSearchDialog, TimeoutHdl_Impl, Timer *, pTimer, void )
{
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if ( pViewShell )
    {
        if ( pViewShell->HasSelection( m_xSearchLB->get_visible() ) )
            EnableControl_Impl(*m_xSelectionBtn);
        else
        {
            m_xSelectionBtn->set_active( false );
            m_xSelectionBtn->set_sensitive(false);
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
    SearchAttrItemList* pList = bSrchFlag ? m_pSearchList.get() : m_pReplaceList.get();

    if ( !pList )
        return rStr;

    // Metric query
    MapUnit eMapUnit = MapUnit::MapCM;
    FieldUnit eFieldUnit = pSh->GetModule()->GetFieldUnit();
    switch ( eFieldUnit )
    {
        case FieldUnit::MM:          eMapUnit = MapUnit::MapMM; break;
        case FieldUnit::CM:
        case FieldUnit::M:
        case FieldUnit::KM:          eMapUnit = MapUnit::MapCM; break;
        case FieldUnit::TWIP:        eMapUnit = MapUnit::MapTwip; break;
        case FieldUnit::POINT:
        case FieldUnit::PICA:        eMapUnit = MapUnit::MapPoint; break;
        case FieldUnit::INCH:
        case FieldUnit::FOOT:
        case FieldUnit::MILE:        eMapUnit = MapUnit::MapInch; break;
        case FieldUnit::MM_100TH:    eMapUnit = MapUnit::Map100thMM; break;
        default: ;//prevent warning
    }

    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    for ( sal_uInt16 i = 0; i < pList->Count(); ++i )
    {
        const SearchAttrInfo& rItem = pList->GetObject(i);

        if ( !rStr.isEmpty() )
            rStr += ", ";

        if ( !IsInvalidItem( rItem.aItemPtr.getItem() ) )
        {
            OUString aStr;
            rPool.GetPresentation(*rItem.aItemPtr.getItem(), eMapUnit, aStr, aIntlWrapper);
            if (aStr.isEmpty())
            {
                if (rStr.endsWith(", "))
                    rStr = rStr.copy(0, rStr.lastIndexOf(","));
            }
            else
                rStr += aStr;
        }
        else if ( rItem.nSlot == SID_ATTR_BRUSH_CHAR )
        {
            // Special treatment for text background
            rStr += SvxResId( RID_SVXITEMS_BRUSH_CHAR );
        }
        else
        {
            sal_uInt32 nId = SvxAttrNameTable::FindIndex(rItem.nSlot);
            if ( RESARRAY_INDEX_NOTFOUND != nId )
                rStr += SvxAttrNameTable::GetString(nId);
        }
    }
    return rStr;
}


void SvxSearchDialog::PaintAttrText_Impl()
{
    OUString aDesc;
    BuildAttrText_Impl(aDesc, m_bSearch);

    if (!m_bFormat && !aDesc.isEmpty())
        m_bFormat = true;

    bool bSetOptimalLayoutSize = false;

    if (m_bSearch)
    {
        m_xSearchAttrText->set_label( aDesc );
        if (!aDesc.isEmpty() && !m_xSearchAttrText->get_visible())
        {
            m_xSearchAttrText->show();
            bSetOptimalLayoutSize = true;
        }

        FocusHdl_Impl(*m_xSearchLB);
    }
    else
    {
        m_xReplaceAttrText->set_label( aDesc );
        if (!aDesc.isEmpty() && !m_xReplaceAttrText->get_visible())
        {
            m_xReplaceAttrText->show();
            bSetOptimalLayoutSize = true;
        }

        FocusHdl_Impl(*m_xReplaceLB);
    }

    if (bSetOptimalLayoutSize)
        m_xDialog->resize_to_request();
}

void SvxSearchDialog::SetModifyFlag_Impl( const weld::Widget* pCtrl )
{
    if (m_xSearchLB.get() == pCtrl)
    {
        m_nModifyFlag |= ModifyFlags::Search;
        m_xSearchLB->set_entry_message_type(weld::EntryMessageType::Normal);
        if (!SvxSearchDialogWrapper::GetSearchLabel().isEmpty())
            SvxSearchDialogWrapper::SetSearchLabel(u""_ustr);
    }
    else if ( m_xReplaceLB.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Replace;
    else if ( m_xWordBtn.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Word;
    else if ( m_xMatchCaseCB.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Exact;
    else if ( m_xReplaceBackwardsCB.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Backwards;
    else if ( m_xNotesBtn.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Notes;
    else if ( m_xSelectionBtn.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Selection;
    else if ( m_xRegExpBtn.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Regexp;
    else if ( m_xWildcardBtn.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Wildcard;
    else if ( m_xLayoutBtn.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Layout;
    else if ( m_xSimilarityBox.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Similarity;
    else if ( m_xCalcSearchInLB.get() == pCtrl )
    {
        m_nModifyFlag |= ModifyFlags::Formulas;
        m_nModifyFlag |= ModifyFlags::Values;
        m_nModifyFlag |= ModifyFlags::CalcNotes;
    }
    else if ( m_xRowsBtn.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Rows;
    else if ( m_xColumnsBtn.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::Columns;
    else if ( m_xAllSheetsCB.get() == pCtrl )
        m_nModifyFlag |= ModifyFlags::AllTables;
}

void SvxSearchDialog::SaveToModule_Impl()
{
    if (!m_pSearchItem)
        return;

    if ( m_xLayoutBtn->get_active() )
    {
        m_pSearchItem->SetSearchString(m_xSearchTmplLB->get_active_text());
        m_pSearchItem->SetReplaceString(m_xReplaceTmplLB->get_active_text());
    }
    else
    {
        m_pSearchItem->SetSearchString(m_xSearchLB->get_active_text());
        m_pSearchItem->SetReplaceString(m_xReplaceLB->get_active_text());
        Remember_Impl(true);
    }

    m_pSearchItem->SetRegExp(false);
    m_pSearchItem->SetWildcard(false);
    m_pSearchItem->SetLevenshtein(false);
    if (GetCheckBoxValue(*m_xRegExpBtn))
        m_pSearchItem->SetRegExp(true);
    else if (GetCheckBoxValue(*m_xWildcardBtn))
        m_pSearchItem->SetWildcard(true);
    else if (GetCheckBoxValue(*m_xSimilarityBox))
        m_pSearchItem->SetLevenshtein(true);

    m_pSearchItem->SetWordOnly(GetCheckBoxValue(*m_xWordBtn));
    m_pSearchItem->SetBackward(GetCheckBoxValue(*m_xReplaceBackwardsCB));
    m_pSearchItem->SetNotes(GetCheckBoxValue(*m_xNotesBtn));
    m_pSearchItem->SetPattern(GetCheckBoxValue(*m_xLayoutBtn));
    m_pSearchItem->SetSelection(GetCheckBoxValue(*m_xSelectionBtn));
    m_pSearchItem->SetUseAsianOptions(GetCheckBoxValue(*m_xJapOptionsCB));

    SvtSearchOptions aOpt;
    aOpt.SetIgnoreDiacritics_CTL(GetNegatedCheckBoxValue(*m_xIncludeDiacritics));
    aOpt.SetIgnoreKashida_CTL(GetNegatedCheckBoxValue(*m_xIncludeKashida));
    aOpt.Commit();

    TransliterationFlags nFlags = GetTransliterationFlags();
    if (!m_pSearchItem->IsUseAsianOptions())
        nFlags &= TransliterationFlags::IGNORE_CASE |
                   TransliterationFlags::IGNORE_WIDTH;
    if (GetNegatedCheckBoxValue(*m_xIncludeDiacritics))
        nFlags |= TransliterationFlags::IGNORE_DIACRITICS_CTL;
    if (GetNegatedCheckBoxValue(*m_xIncludeKashida))
        nFlags |= TransliterationFlags::IGNORE_KASHIDA_CTL;
    m_pSearchItem->SetTransliterationFlags(nFlags);

    if (!m_bWriter)
    {
        if (m_xCalcSearchInLB->get_active() != -1)
            m_pSearchItem->SetCellType(
                static_cast<SvxSearchCellType>(m_xCalcSearchInLB->get_active()));

        m_pSearchItem->SetRowDirection(m_xRowsBtn->get_active());
        m_pSearchItem->SetAllTables(m_xAllSheetsCB->get_active());
        m_pSearchItem->SetSearchFormatted(m_xSearchFormattedCB->get_active());
    }

    m_pSearchItem->SetCommand(SvxSearchCmd::FIND);
    m_nModifyFlag = ModifyFlags::NONE;
    const SfxPoolItem* ppArgs[] = { m_pSearchItem.get(), nullptr };
    m_rBindings.GetDispatcher()->Execute(SID_SEARCH_ITEM, SfxCallMode::SLOT, ppArgs);
}

short SvxSearchDialog::executeSubDialog(VclAbstractDialog * dialog) {
    assert(!m_executingSubDialog);
    comphelper::ScopeGuard g([this] { m_executingSubDialog = false; });
    m_executingSubDialog = true;
    return dialog->Execute();
}

SFX_IMPL_CHILDWINDOW_WITHID(SvxSearchDialogWrapper, SID_SEARCH_DLG);


SvxSearchDialogWrapper::SvxSearchDialogWrapper( vcl::Window* _pParent, sal_uInt16 nId,
                                                SfxBindings* pBindings,
                                                SfxChildWinInfo const * pInfo )
    : SfxChildWindow( _pParent, nId )
    , dialog(std::make_shared<SvxSearchDialog>(_pParent->GetFrameWeld(), this, *pBindings))
{
    SetController(dialog);
    dialog->Initialize( pInfo );

    pBindings->Update( SID_SEARCH_ITEM );
    pBindings->Update( SID_SEARCH_OPTIONS );
    pBindings->Update( SID_SEARCH_SEARCHSET );
    pBindings->Update( SID_SEARCH_REPLACESET );
    dialog->m_bConstruct = false;
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

static void lcl_SetSearchLabelWindow(const OUString& rStr, const SfxViewFrame& rViewFrame)
{
    css::uno::Reference< css::beans::XPropertySet > xPropSet(
            rViewFrame.GetFrame().GetFrameInterface(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    xPropSet->getPropertyValue(u"LayoutManager"_ustr) >>= xLayoutManager;
    css::uno::Reference< css::ui::XUIElement > xUIElement =
        xLayoutManager->getElement(u"private:resource/toolbar/findbar"_ustr);
    if (!xUIElement.is())
        return;
    css::uno::Reference< css::awt::XWindow > xWindow(
            xUIElement->getRealInterface(), css::uno::UNO_QUERY_THROW);
    VclPtr< ToolBox > pToolBox = static_cast<ToolBox*>( VCLUnoHelper::GetWindow(xWindow) );
    for (ToolBox::ImplToolItems::size_type i = 0; pToolBox && i < pToolBox->GetItemCount(); ++i)
    {
        ToolBoxItemId id = pToolBox->GetItemId(i);
        if (pToolBox->GetItemCommand(id) == ".uno:SearchLabel")
        {
            LabelItemWindow* pSearchLabel = dynamic_cast<LabelItemWindow*>(pToolBox->GetItemWindow(id));
            assert(pSearchLabel);
            pSearchLabel->set_label(rStr, LabelItemWindowType::Info);
            pSearchLabel->SetOptimalSize();
        }

        if (pToolBox->IsFloatingMode() && pToolBox->GetItemCommand(id) == ".uno:FindText")
        {
            FindTextFieldControl* pFindText = dynamic_cast<FindTextFieldControl*>(pToolBox->GetItemWindow(id));
            assert(pFindText);
            if (rStr == SvxResId(RID_SVXSTR_SEARCH_NOT_FOUND))
                pFindText->set_entry_message_type(weld::EntryMessageType::Error);
            else if (rStr == SvxResId(RID_SVXSTR_SEARCH_END) || rStr == SvxResId(RID_SVXSTR_SEARCH_START))
                pFindText->set_entry_message_type(weld::EntryMessageType::Warning);
            else
                pFindText->set_entry_message_type(weld::EntryMessageType::Normal);
        }
    }
    xLayoutManager->doLayout();
    pToolBox->Resize();
}

OUString SvxSearchDialogWrapper::GetSearchLabel()
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return OUString();

    css::uno::Reference< css::beans::XPropertySet > xPropSet(
            pViewFrame->GetFrame().GetFrameInterface(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    xPropSet->getPropertyValue(u"LayoutManager"_ustr) >>= xLayoutManager;
    if (!xLayoutManager.is())
        return OUString();
    css::uno::Reference< css::ui::XUIElement > xUIElement =
        xLayoutManager->getElement(u"private:resource/toolbar/findbar"_ustr);
    if (!xUIElement.is())
        return OUString();
    css::uno::Reference< css::awt::XWindow > xWindow(
            xUIElement->getRealInterface(), css::uno::UNO_QUERY_THROW);
    VclPtr< ToolBox > pToolBox = static_cast<ToolBox*>( VCLUnoHelper::GetWindow(xWindow) );
    for (ToolBox::ImplToolItems::size_type i = 0; pToolBox && i < pToolBox->GetItemCount(); ++i)
    {
        ToolBoxItemId id = pToolBox->GetItemId(i);
        if (pToolBox->GetItemCommand(id) == ".uno:SearchLabel")
        {
            LabelItemWindow* pSearchLabel = dynamic_cast<LabelItemWindow*>(pToolBox->GetItemWindow(id));
            return pSearchLabel ? pSearchLabel->get_label() : OUString();
        }
    }
    return OUString();
}

void SvxSearchDialogWrapper::SetSearchLabel(const SearchLabel& rSL)
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return;

    OUString sStr;
    if (rSL == SearchLabel::End)
        sStr = SvxResId(RID_SVXSTR_SEARCH_END);
    else if (rSL == SearchLabel::Start)
        sStr = SvxResId(RID_SVXSTR_SEARCH_START);
    else if (rSL == SearchLabel::EndWrapped)
        sStr = SvxResId(RID_SVXSTR_SEARCH_END_WRAPPED);
    else if (rSL == SearchLabel::StartWrapped)
        sStr = SvxResId(RID_SVXSTR_SEARCH_START_WRAPPED);
    else if (rSL == SearchLabel::EndSheet)
        sStr = SvxResId(RID_SVXSTR_SEARCH_END_SHEET);
    else if (rSL == SearchLabel::NotFound)
        sStr = SvxResId(RID_SVXSTR_SEARCH_NOT_FOUND);
    else if (rSL == SearchLabel::NavElementNotFound)
        sStr = SvxResId(RID_SVXSTR_SEARCH_NAV_ELEMENT_NOT_FOUND);
    else if (rSL == SearchLabel::ReminderEndWrapped)
        sStr = SvxResId(RID_SVXSTR_SEARCH_REMINDER_END_WRAPPED);
    else if (rSL == SearchLabel::ReminderStartWrapped)
        sStr = SvxResId(RID_SVXSTR_SEARCH_REMINDER_START_WRAPPED);

    lcl_SetSearchLabelWindow(sStr, *pViewFrame);

    if (SvxSearchDialogWrapper *pWrp = static_cast<SvxSearchDialogWrapper*>( pViewFrame->
            GetChildWindow( SvxSearchDialogWrapper::GetChildWindowId() )))
        pWrp->getDialog()->SetSearchLabel(sStr);
}

void SvxSearchDialogWrapper::SetSearchLabel(const OUString& sStr)
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return;

    lcl_SetSearchLabelWindow(sStr, *pViewFrame);
    if (SvxSearchDialogWrapper *pWrp = static_cast<SvxSearchDialogWrapper*>( pViewFrame->
            GetChildWindow( SvxSearchDialogWrapper::GetChildWindowId() )))
        pWrp->getDialog()->SetSearchLabel(sStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
