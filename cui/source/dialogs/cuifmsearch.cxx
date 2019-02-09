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

#include <tools/debug.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <dialmgr.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/app.hxx>
#include <svx/fmsrccfg.hxx>
#include <svx/fmsrcimp.hxx>
#include <strings.hrc>
#include <cuifmsearch.hxx>
#include <svx/srchdlg.hxx>
#include <svl/cjkoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <svx/svxdlg.hxx>
#include <sal/macros.h>

using namespace css::uno;
using namespace css::i18n;
using namespace ::svxform;
using namespace css::sdbc;
using namespace css::util;

#define MAX_HISTORY_ENTRIES     50

void FmSearchDialog::initCommon( const Reference< XResultSet >& _rxCursor )
{
    // init the engine
    DBG_ASSERT( m_pSearchEngine, "FmSearchDialog::initCommon: have no engine!" );
    m_pSearchEngine->SetProgressHandler(LINK(this, FmSearchDialog, OnSearchProgress));

    // some layout changes according to available CJK options
    SvtCJKOptions aCJKOptions;
    if (!aCJKOptions.IsJapaneseFindEnabled())
    {
        // hide the options for the japanese search
        m_pSoundsLikeCJK->hide();
        m_pSoundsLikeCJKSettings->hide();
    }

    if (!aCJKOptions.IsCJKFontEnabled())
    {
        m_pHalfFullFormsCJK->hide();

        // never ignore the width (ignoring is expensive) if the option is not available at all
        m_pSearchEngine->SetIgnoreWidthCJK( false );
    }

    // some initial record texts
    m_pftRecord->set_label( OUString::number(_rxCursor->getRow()) );
    m_pbClose->set_tooltip_text(OUString());
}

FmSearchDialog::FmSearchDialog(weld::Window* pParent, const OUString& sInitialText, const std::vector< OUString >& _rContexts, sal_Int16 nInitialContext,
    const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier)
    : GenericDialogController(pParent, "cui/ui/fmsearchdialog.ui", "RecordSearchDialog")
    , m_sCancel( Button::GetStandardText( StandardButtonType::Cancel ) )
    , m_lnkContextSupplier(lnkContextSupplier)
    , m_prbSearchForText(m_xBuilder->weld_radio_button("rbSearchForText"))
    , m_prbSearchForNull(m_xBuilder->weld_radio_button("rbSearchForNull"))
    , m_prbSearchForNotNull(m_xBuilder->weld_radio_button("rbSearchForNotNull"))
    , m_pcmbSearchText(m_xBuilder->weld_combo_box("cmbSearchText"))
    , m_pftForm(m_xBuilder->weld_label("ftForm"))
    , m_plbForm(m_xBuilder->weld_combo_box("lbForm"))
    , m_prbAllFields(m_xBuilder->weld_radio_button("rbAllFields"))
    , m_prbSingleField(m_xBuilder->weld_radio_button("rbSingleField"))
    , m_plbField(m_xBuilder->weld_combo_box("lbField"))
    , m_pftPosition(m_xBuilder->weld_label("ftPosition"))
    , m_plbPosition(m_xBuilder->weld_combo_box("lbPosition"))
    , m_pcbUseFormat(m_xBuilder->weld_check_button("cbUseFormat"))
    , m_pcbCase(m_xBuilder->weld_check_button("cbCase"))
    , m_pcbBackwards(m_xBuilder->weld_check_button("cbBackwards"))
    , m_pcbStartOver(m_xBuilder->weld_check_button("cbStartOver"))
    , m_pcbWildCard(m_xBuilder->weld_check_button("cbWildCard"))
    , m_pcbRegular(m_xBuilder->weld_check_button("cbRegular"))
    , m_pcbApprox(m_xBuilder->weld_check_button("cbApprox"))
    , m_ppbApproxSettings(m_xBuilder->weld_button("pbApproxSettings"))
    , m_pHalfFullFormsCJK(m_xBuilder->weld_check_button("HalfFullFormsCJK"))
    , m_pSoundsLikeCJK(m_xBuilder->weld_check_button("SoundsLikeCJK"))
    , m_pSoundsLikeCJKSettings(m_xBuilder->weld_button("SoundsLikeCJKSettings"))
    , m_pftRecord(m_xBuilder->weld_label("ftRecord"))
    , m_pftHint(m_xBuilder->weld_label("ftHint"))
    , m_pbSearchAgain(m_xBuilder->weld_button("pbSearchAgain"))
    , m_pbClose(m_xBuilder->weld_button("close"))
{
    m_pcmbSearchText->set_size_request(m_pcmbSearchText->get_approximate_digit_width() * 38, -1);
    m_plbForm->set_size_request(m_plbForm->get_approximate_digit_width() * 38, -1);
    m_sSearch = m_pbSearchAgain->get_label();

    DBG_ASSERT(m_lnkContextSupplier.IsSet(), "FmSearchDialog::FmSearchDialog : have no ContextSupplier !");

    FmSearchContext fmscInitial;
    fmscInitial.nContext = nInitialContext;
    m_lnkContextSupplier.Call(fmscInitial);
    DBG_ASSERT(fmscInitial.xCursor.is(), "FmSearchDialog::FmSearchDialog : invalid data supplied by ContextSupplier !");
    DBG_ASSERT(comphelper::string::getTokenCount(fmscInitial.strUsedFields, ';') == static_cast<sal_Int32>(fmscInitial.arrFields.size()),
        "FmSearchDialog::FmSearchDialog : invalid data supplied by ContextSupplied !");
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
    for (const Reference<XInterface> & arrField : fmscInitial.arrFields)
    {
        DBG_ASSERT(arrField.is(), "FmSearchDialog::FmSearchDialog : invalid data supplied by ContextSupplier !");
    }
#endif // (OSL_DEBUG_LEVEL > 1) || DBG_UTIL

    for (   std::vector< OUString >::const_iterator context = _rContexts.begin();
            context != _rContexts.end();
            ++context
        )
    {
        m_arrContextFields.emplace_back();
        m_plbForm->append_text(*context);
    }
    m_plbForm->set_active(nInitialContext);

    m_plbForm->connect_changed(LINK(this, FmSearchDialog, OnContextSelection));

    if (m_arrContextFields.size() == 1)
    {
        // hide dispensable controls
        m_pftForm->hide();
        m_plbForm->hide();
    }

    m_pSearchEngine.reset( new FmSearchEngine(
        ::comphelper::getProcessComponentContext(), fmscInitial.xCursor, fmscInitial.strUsedFields, fmscInitial.arrFields ) );
    initCommon( fmscInitial.xCursor );

    if ( !fmscInitial.sFieldDisplayNames.isEmpty() )
    {   // use the display names if supplied
        DBG_ASSERT(comphelper::string::getTokenCount(fmscInitial.sFieldDisplayNames, ';') == comphelper::string::getTokenCount(fmscInitial.strUsedFields, ';'),
            "FmSearchDialog::FmSearchDialog : invalid initial context description !");
        Init(fmscInitial.sFieldDisplayNames, sInitialText);
    }
    else
        Init(fmscInitial.strUsedFields, sInitialText);
}

FmSearchDialog::~FmSearchDialog()
{
    SaveParams();

    m_pConfig.reset();
    m_pSearchEngine.reset();
}

void FmSearchDialog::Init(const OUString& strVisibleFields, const OUString& sInitialText)
{
    //the initialization of all the Controls
    m_prbSearchForText->connect_clicked(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_prbSearchForNull->connect_clicked(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_prbSearchForNotNull->connect_clicked(LINK(this, FmSearchDialog, OnClickedFieldRadios));

    m_prbAllFields->connect_clicked(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_prbSingleField->connect_clicked(LINK(this, FmSearchDialog, OnClickedFieldRadios));

    m_pbSearchAgain->connect_clicked(LINK(this, FmSearchDialog, OnClickedSearchAgain));
    m_ppbApproxSettings->connect_clicked(LINK(this, FmSearchDialog, OnClickedSpecialSettings));
    m_pSoundsLikeCJKSettings->connect_clicked(LINK(this, FmSearchDialog, OnClickedSpecialSettings));

    m_plbPosition->connect_changed(LINK(this, FmSearchDialog, OnPositionSelected));
    m_plbField->connect_changed(LINK(this, FmSearchDialog, OnFieldSelected));

    m_pcmbSearchText->connect_changed(LINK(this, FmSearchDialog, OnSearchTextModified));
    m_pcmbSearchText->set_entry_completion(false);
    m_pcmbSearchText->connect_focus_in(LINK(this, FmSearchDialog, OnFocusGrabbed));

    m_pcbUseFormat->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbBackwards->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbStartOver->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbCase->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbWildCard->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbRegular->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbApprox->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pHalfFullFormsCJK->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pSoundsLikeCJK->connect_toggled(LINK(this, FmSearchDialog, OnCheckBoxToggled));

    // fill the listboxes
    // method of field comparison
    const char* const aResIds[] = {
        RID_STR_SEARCH_ANYWHERE,
        RID_STR_SEARCH_BEGINNING,
        RID_STR_SEARCH_END,
        RID_STR_SEARCH_WHOLE
    };
    for (auto pResId : aResIds)
        m_plbPosition->append_text(CuiResId(pResId));
    m_plbPosition->set_active(MATCHING_ANYWHERE);

    // the field listbox
    if (!strVisibleFields.isEmpty())
    {
        sal_Int32 nPos {0};
        do {
            m_plbField->append_text(strVisibleFields.getToken(0, ';', nPos));
        } while (nPos>=0);
    }


    m_pConfig.reset( new FmSearchConfigItem );
    LoadParams();

    m_pcmbSearchText->set_entry_text(sInitialText);
    // if the Edit-line has changed the text (e.g. because it contains
    // control characters, as can be the case with memo fields), I use
    // an empty OUString.
    OUString sRealSetText = m_pcmbSearchText->get_active_text();
    if (sRealSetText != sInitialText)
        m_pcmbSearchText->set_entry_text(OUString());
    OnSearchTextModified(*m_pcmbSearchText);

    // initial
    EnableSearchUI(true);

    if ( m_prbSearchForText->get_active() )
        m_pcmbSearchText->grab_focus();

}

short FmSearchDialog::run()
{
    short nRet = weld::GenericDialogController::run();
    m_pSearchEngine->CancelSearch();
    return nRet;
}

IMPL_LINK(FmSearchDialog, OnClickedFieldRadios, weld::Button&, rButton, void)
{
    if ((&rButton == m_prbSearchForText.get()) || (&rButton == m_prbSearchForNull.get()) || (&rButton == m_prbSearchForNotNull.get()))
    {
        EnableSearchForDependees(true);
    }
    else
        // en- or disable field list box accordingly
        if (&rButton == m_prbSingleField.get())
        {
            m_plbField->set_sensitive(true);
            m_pSearchEngine->RebuildUsedFields(m_plbField->get_active());
        }
        else
        {
            m_plbField->set_sensitive(false);
            m_pSearchEngine->RebuildUsedFields(-1);
        }
}

IMPL_LINK_NOARG(FmSearchDialog, OnClickedSearchAgain, weld::Button&, void)
{
    if (m_pbClose->get_sensitive())
    {   // the button has the function 'search'
        OUString strThisRoundText = m_pcmbSearchText->get_active_text();
        // to history
        m_pcmbSearchText->remove_text(strThisRoundText);
        m_pcmbSearchText->insert_text(0, strThisRoundText);
        // the remove/insert makes sure that a) the OUString does not appear twice and
        // that b) the last searched strings are at the beginning and limit the list length
        while (m_pcmbSearchText->get_count() > MAX_HISTORY_ENTRIES)
            m_pcmbSearchText->remove(m_pcmbSearchText->get_count()-1);

        // take out the 'overflow' hint
        m_pftHint->set_label(OUString());

        if (m_pcbStartOver->get_active())
        {
            m_pcbStartOver->set_active(false);
            EnableSearchUI(false);
            if (m_prbSearchForText->get_active())
                m_pSearchEngine->StartOver(strThisRoundText);
            else
                m_pSearchEngine->StartOverSpecial(m_prbSearchForNull->get_active());
        }
        else
        {
            EnableSearchUI(false);
            if (m_prbSearchForText->get_active())
                m_pSearchEngine->SearchNext(strThisRoundText);
            else
                m_pSearchEngine->SearchNextSpecial(m_prbSearchForNull->get_active());
        }
    }
    else
    {   // the button has the function 'cancel'
            // the CancelButton is usually only disabled, when working in a thread or with reschedule
        m_pSearchEngine->CancelSearch();
            // the ProgressHandler is called when it's really finished, here it's only a demand
    }
}

IMPL_LINK(FmSearchDialog, OnClickedSpecialSettings, weld::Button&, rButton, void)
{
    if (m_ppbApproxSettings.get() == &rButton)
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxSearchSimilarityDialog> pDlg(pFact->CreateSvxSearchSimilarityDialog(m_xDialog.get(), m_pSearchEngine->GetLevRelaxed(), m_pSearchEngine->GetLevOther(),
                    m_pSearchEngine->GetLevShorter(), m_pSearchEngine->GetLevLonger() ));
        if (pDlg->Execute() == RET_OK)
        {
            m_pSearchEngine->SetLevRelaxed( pDlg->IsRelaxed() );
            m_pSearchEngine->SetLevOther( pDlg->GetOther() );
            m_pSearchEngine->SetLevShorter(pDlg->GetShorter() );
            m_pSearchEngine->SetLevLonger( pDlg->GetLonger() );
        }
    }
    else if (m_pSoundsLikeCJKSettings.get() == &rButton)
    {
        SfxItemSet aSet( SfxGetpApp()->GetPool() );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxJSearchOptionsDialog> aDlg(pFact->CreateSvxJSearchOptionsDialog(m_xDialog.get(), aSet, m_pSearchEngine->GetTransliterationFlags() ));
        aDlg->Execute();

        TransliterationFlags nFlags = aDlg->GetTransliterationFlags();
        m_pSearchEngine->SetTransliterationFlags(nFlags);

        m_pcbCase->set_active(m_pSearchEngine->GetCaseSensitive());
        OnCheckBoxToggled( *m_pcbCase );
        m_pHalfFullFormsCJK->set_active( !m_pSearchEngine->GetIgnoreWidthCJK() );
        OnCheckBoxToggled( *m_pHalfFullFormsCJK );
    }
}

IMPL_LINK_NOARG(FmSearchDialog, OnSearchTextModified, weld::ComboBox&, void)
{
    if ((!m_pcmbSearchText->get_active_text().isEmpty()) || !m_prbSearchForText->get_active())
        m_pbSearchAgain->set_sensitive(true);
    else
        m_pbSearchAgain->set_sensitive(false);

    m_pSearchEngine->InvalidatePreviousLoc();
}

IMPL_LINK_NOARG(FmSearchDialog, OnFocusGrabbed, weld::Widget&, void)
{
    m_pcmbSearchText->select_entry_region(0, -1);
}

IMPL_LINK_NOARG(FmSearchDialog, OnPositionSelected, weld::ComboBox&, void)
{
    m_pSearchEngine->SetPosition(m_plbPosition->get_active());
}

IMPL_LINK_NOARG(FmSearchDialog, OnFieldSelected, weld::ComboBox&, void)
{
    m_pSearchEngine->RebuildUsedFields(m_prbAllFields->get_active() ? -1 : m_plbField->get_active());
    // calls m_pSearchEngine->InvalidatePreviousLoc too

    int nCurrentContext = m_plbForm->get_active();
    if (nCurrentContext != -1)
        m_arrContextFields[nCurrentContext] = m_plbField->get_active_text();
}

IMPL_LINK(FmSearchDialog, OnCheckBoxToggled, weld::ToggleButton&, rBox, void)
{
    bool bChecked = rBox.get_active();

    // formatter or case -> pass on to the engine
    if (&rBox == m_pcbUseFormat.get())
        m_pSearchEngine->SetFormatterUsing(bChecked);
    else if (&rBox == m_pcbCase.get())
        m_pSearchEngine->SetCaseSensitive(bChecked);
    // direction -> pass on and reset the checkbox-text for StartOver
    else if (&rBox == m_pcbBackwards.get())
    {
        m_pcbStartOver->set_label( CuiResId( bChecked ? RID_STR_FROM_BOTTOM : RID_STR_FROM_TOP ) );
        m_pSearchEngine->SetDirection(!bChecked);
    }
    // similarity-search or regular expression
    else if ((&rBox == m_pcbApprox.get()) || (&rBox == m_pcbRegular.get()) || (&rBox == m_pcbWildCard.get()))
    {
        weld::CheckButton* pBoxes[] = { m_pcbWildCard.get(), m_pcbRegular.get(), m_pcbApprox.get() };
        for (weld::CheckButton* pBoxe : pBoxes)
        {
            if (pBoxe != &rBox)
            {
                if (bChecked)
                    pBoxe->set_sensitive(false);
                else
                    pBoxe->set_sensitive(true);
            }
        }

        // pass on to the engine
        m_pSearchEngine->SetWildcard(m_pcbWildCard->get_sensitive() && m_pcbWildCard->get_active());
        m_pSearchEngine->SetRegular(m_pcbRegular->get_sensitive() && m_pcbRegular->get_active());
        m_pSearchEngine->SetLevenshtein(m_pcbApprox->get_sensitive() && m_pcbApprox->get_active());
            // (disabled boxes have to be passed to the engine as sal_False)

        // adjust the Position-Listbox (which is not allowed during Wildcard-search)
        if (&rBox == m_pcbWildCard.get())
        {
            if (bChecked)
            {
                m_pftPosition->set_sensitive(false);
                m_plbPosition->set_sensitive(false);
            }
            else
            {
                m_pftPosition->set_sensitive(true);
                m_plbPosition->set_sensitive(true);
            }
        }

        // and the button for similarity-search
        if (&rBox == m_pcbApprox.get())
        {
            if (bChecked)
                m_ppbApproxSettings->set_sensitive(true);
            else
                m_ppbApproxSettings->set_sensitive(false);
        }
    }
    else if (&rBox == m_pHalfFullFormsCJK.get())
    {
        // forward to the search engine
        m_pSearchEngine->SetIgnoreWidthCJK( !bChecked );
    }
    else if (&rBox == m_pSoundsLikeCJK.get())
    {
        m_pSoundsLikeCJKSettings->set_sensitive(bChecked);

        // two other buttons which depend on this one
        bool bEnable =  (   m_prbSearchForText->get_active()
                            &&  !m_pSoundsLikeCJK->get_active()
                            )
                         || !SvtCJKOptions().IsJapaneseFindEnabled();
        m_pcbCase->set_sensitive(bEnable);
        m_pHalfFullFormsCJK->set_sensitive(bEnable);

        // forward to the search engine
        m_pSearchEngine->SetTransliteration( bChecked );
    }
}

void FmSearchDialog::InitContext(sal_Int16 nContext)
{
    FmSearchContext fmscContext;
    fmscContext.nContext = nContext;

    sal_uInt32 nResult = m_lnkContextSupplier.Call(fmscContext);
    DBG_ASSERT(nResult > 0, "FmSearchDialog::InitContext : ContextSupplier didn't give me any controls !");

    // put the field names into the respective listbox
    m_plbField->clear();

    if (!fmscContext.sFieldDisplayNames.isEmpty())
    {
        // use the display names if supplied
        DBG_ASSERT(comphelper::string::getTokenCount(fmscContext.sFieldDisplayNames, ';') == comphelper::string::getTokenCount(fmscContext.strUsedFields, ';'),
            "FmSearchDialog::InitContext : invalid context description supplied !");
        sal_Int32 nPos {0};
        do {
            m_plbField->append_text(fmscContext.sFieldDisplayNames.getToken(0, ';', nPos));
        } while (nPos>=0);
    }
    else if (!fmscContext.strUsedFields.isEmpty())
    {
        // else use the field names
        sal_Int32 nPos {0};
        do {
            m_plbField->append_text(fmscContext.strUsedFields.getToken(0, ';', nPos));
        } while (nPos>=0);
    }

    if (nContext < static_cast<sal_Int32>(m_arrContextFields.size()) && !m_arrContextFields[nContext].isEmpty())
    {
        m_plbField->set_active_text(m_arrContextFields[nContext]);
    }
    else
    {
        m_plbField->set_active(0);
        if (m_prbSingleField->get_active() && (m_plbField->get_count() > 1))
            m_plbField->grab_focus();
    }

    m_pSearchEngine->SwitchToContext(fmscContext.xCursor, fmscContext.strUsedFields, fmscContext.arrFields,
        m_prbAllFields->get_active() ? -1 : 0);

    m_pftRecord->set_label(OUString::number(fmscContext.xCursor->getRow()));
}

IMPL_LINK(FmSearchDialog, OnContextSelection, weld::ComboBox&, rBox, void)
{
    InitContext(rBox.get_active());
}

void FmSearchDialog::EnableSearchUI(bool bEnable)
{
    // the search button has two functions -> adjust its text accordingly
    OUString sButtonText( bEnable ? m_sSearch : m_sCancel );
    m_pbSearchAgain->set_label(sButtonText);

    m_prbSearchForText->set_sensitive(bEnable);
    m_prbSearchForNull->set_sensitive(bEnable);
    m_prbSearchForNotNull->set_sensitive(bEnable);
    m_plbForm->set_sensitive(bEnable);
    m_prbAllFields->set_sensitive(bEnable);
    m_prbSingleField->set_sensitive(bEnable);
    m_plbField->set_sensitive(bEnable && m_prbSingleField->get_active());
    m_pcbBackwards->set_sensitive(bEnable);
    m_pcbStartOver->set_sensitive(bEnable);
    m_pbClose->set_sensitive(bEnable);
    EnableSearchForDependees(bEnable);

    if ( !bEnable )
    {   // this means we're preparing for starting a search
        // In this case, EnableSearchForDependees disabled the search button
        // But as we're about to use it for cancelling the search, we really need to enable it, again
        m_pbSearchAgain->set_sensitive(true);
    }
}

void FmSearchDialog::EnableSearchForDependees(bool bEnable)
{
    bool bSearchingForText = m_prbSearchForText->get_active();
    m_pbSearchAgain->set_sensitive(bEnable && (!bSearchingForText || (!m_pcmbSearchText->get_active_text().isEmpty())));

    bEnable = bEnable && bSearchingForText;

    bool bEnableRedundants = !m_pSoundsLikeCJK->get_active() || !SvtCJKOptions().IsJapaneseFindEnabled();

    m_pcmbSearchText->set_sensitive(bEnable);
    m_pftPosition->set_sensitive(bEnable && !m_pcbWildCard->get_active());
    m_pcbWildCard->set_sensitive(bEnable && !m_pcbRegular->get_active() && !m_pcbApprox->get_active());
    m_pcbRegular->set_sensitive(bEnable && !m_pcbWildCard->get_active() && !m_pcbApprox->get_active());
    m_pcbApprox->set_sensitive(bEnable && !m_pcbWildCard->get_active() && !m_pcbRegular->get_active());
    m_ppbApproxSettings->set_sensitive(bEnable && m_pcbApprox->get_active());
    m_pHalfFullFormsCJK->set_sensitive(bEnable && bEnableRedundants);
    m_pSoundsLikeCJK->set_sensitive(bEnable);
    m_pSoundsLikeCJKSettings->set_sensitive(bEnable && m_pSoundsLikeCJK->get_active());
    m_plbPosition->set_sensitive(bEnable && !m_pcbWildCard->get_active());
    m_pcbUseFormat->set_sensitive(bEnable);
    m_pcbCase->set_sensitive(bEnable && bEnableRedundants);
}

void FmSearchDialog::OnFound(const css::uno::Any& aCursorPos, sal_Int16 nFieldPos)
{
    FmFoundRecordInformation friInfo;
    friInfo.nContext = m_plbForm->get_active();
    // if I don't do a search in a context, this has an invalid value - but then it doesn't matter anyway
    friInfo.aPosition = aCursorPos;
    if (m_prbAllFields->get_active())
        friInfo.nFieldPos = nFieldPos;
    else
        friInfo.nFieldPos = m_plbField->get_active();
        // this of course implies that I have really searched in the field that is selected in the listbox,
        // which is made sure in RebuildUsedFields

    m_lnkFoundHandler.Call(friInfo);

    m_pcmbSearchText->grab_focus();
}

IMPL_LINK(FmSearchDialog, OnSearchProgress, const FmSearchProgress*, pProgress, void)
{
    SolarMutexGuard aGuard;
        // make this single method thread-safe (it's an overkill to block the whole application for this,
        // but we don't have another safety concept at the moment)

    switch (pProgress->aSearchState)
    {
        case FmSearchProgress::State::Progress:
            if (pProgress->bOverflow)
            {
                OUString sHint( CuiResId( m_pcbBackwards->get_active() ? RID_STR_OVERFLOW_BACKWARD : RID_STR_OVERFLOW_FORWARD ) );
                m_pftHint->set_label( sHint );
            }

            m_pftRecord->set_label(OUString::number(1 + pProgress->nCurrentRecord));
            break;

        case FmSearchProgress::State::ProgressCounting:
            m_pftHint->set_label(CuiResId(RID_STR_SEARCH_COUNTING));
            m_pftRecord->set_label(OUString::number(pProgress->nCurrentRecord));
            break;

        case FmSearchProgress::State::Successful:
            OnFound(pProgress->aBookmark, static_cast<sal_Int16>(pProgress->nFieldIndex));
            EnableSearchUI(true);
            break;

        case FmSearchProgress::State::Error:
        case FmSearchProgress::State::NothingFound:
        {
            const char* pErrorId = (FmSearchProgress::State::Error == pProgress->aSearchState)
                ? RID_STR_SEARCH_GENERAL_ERROR
                : RID_STR_SEARCH_NORECORD;
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Warning, VclButtonsType::Ok, CuiResId(pErrorId)));
            xBox->run();
            [[fallthrough]];
        }
        case FmSearchProgress::State::Canceled:
            EnableSearchUI(true);
            if (m_lnkCanceledNotFoundHdl.IsSet())
            {
                FmFoundRecordInformation friInfo;
                friInfo.nContext = m_plbForm->get_active();
                // if I don't do a search in a context, this has an invalid value - but then it doesn't matter anyway
                friInfo.aPosition = pProgress->aBookmark;
                m_lnkCanceledNotFoundHdl.Call(friInfo);
            }
            break;
    }

    m_pftRecord->set_label(OUString::number(1 + pProgress->nCurrentRecord));
}

void FmSearchDialog::LoadParams()
{
    FmSearchParams aParams(m_pConfig->getParams());

    const OUString* pHistory     =                   aParams.aHistory.getConstArray();
    const OUString* pHistoryEnd  =   pHistory    +   aParams.aHistory.getLength();
    for (; pHistory != pHistoryEnd; ++pHistory)
        m_pcmbSearchText->append_text( *pHistory );

    // I do the settings at my UI-elements and then I simply call the respective change-handler,
    // that way the data is handed on to the SearchEngine and all dependent settings are done

    // current field
    int nInitialField = m_plbField->find_text( aParams.sSingleSearchField );
    if (nInitialField == -1)
        nInitialField = 0;
    m_plbField->set_active(nInitialField);
    OnFieldSelected(*m_plbField);
    // all fields/single field (AFTER selecting the field because OnClickedFieldRadios expects a valid value there)
    if (aParams.bAllFields)
    {
        m_prbSingleField->set_active(false);
        m_prbAllFields->set_active(true);
        OnClickedFieldRadios(*m_prbAllFields);
        // OnClickedFieldRadios also calls to RebuildUsedFields
    }
    else
    {
        m_prbAllFields->set_active(false);
        m_prbSingleField->set_active(true);
        OnClickedFieldRadios(*m_prbSingleField);
    }

    m_plbPosition->set_active(aParams.nPosition);
    OnPositionSelected(*m_plbPosition);

    // field formatting/case sensitivity/direction
    m_pcbUseFormat->set_active(aParams.bUseFormatter);
    m_pcbCase->set_active( aParams.isCaseSensitive() );
    m_pcbBackwards->set_active(aParams.bBackwards);
    OnCheckBoxToggled(*m_pcbUseFormat);
    OnCheckBoxToggled(*m_pcbCase);
    OnCheckBoxToggled(*m_pcbBackwards);

    m_pHalfFullFormsCJK->set_active( !aParams.isIgnoreWidthCJK( ) );  // BEWARE: this checkbox has a inverse semantics!
    m_pSoundsLikeCJK->set_active( aParams.bSoundsLikeCJK );
    OnCheckBoxToggled(*m_pHalfFullFormsCJK);
    OnCheckBoxToggled(*m_pSoundsLikeCJK);

    m_pcbWildCard->set_active(false);
    m_pcbRegular->set_active(false);
    m_pcbApprox->set_active(false);
    OnCheckBoxToggled(*m_pcbWildCard);
    OnCheckBoxToggled(*m_pcbRegular);
    OnCheckBoxToggled(*m_pcbApprox);

    weld::CheckButton* pToCheck = nullptr;
    if (aParams.bWildcard)
        pToCheck = m_pcbWildCard.get();
    if (aParams.bRegular)
        pToCheck = m_pcbRegular.get();
    if (aParams.bApproxSearch)
        pToCheck = m_pcbApprox.get();
    if (aParams.bSoundsLikeCJK)
        pToCheck = m_pSoundsLikeCJK.get();
    if (pToCheck)
    {
        pToCheck->set_active(true);
        OnCheckBoxToggled(*pToCheck);
    }

    // set Levenshtein-parameters directly at the SearchEngine
    m_pSearchEngine->SetLevRelaxed(aParams.bLevRelaxed);
    m_pSearchEngine->SetLevOther(aParams.nLevOther);
    m_pSearchEngine->SetLevShorter(aParams.nLevShorter);
    m_pSearchEngine->SetLevLonger(aParams.nLevLonger);

    m_pSearchEngine->SetTransliterationFlags( aParams.getTransliterationFlags( ) );

    m_prbSearchForText->set_active(false);
    m_prbSearchForNull->set_active(false);
    m_prbSearchForNotNull->set_active(false);
    switch (aParams.nSearchForType)
    {
        case 1: m_prbSearchForNull->set_active(true); break;
        case 2: m_prbSearchForNotNull->set_active(true); break;
        default: m_prbSearchForText->set_active(true); break;
    }
    OnClickedFieldRadios(*m_prbSearchForText);
}

void FmSearchDialog::SaveParams() const
{
    if (!m_pConfig)
        return;

    FmSearchParams aCurrentSettings;

    int nCount = m_pcmbSearchText->get_count();
    aCurrentSettings.aHistory.realloc(nCount);
    OUString* pHistory = aCurrentSettings.aHistory.getArray();
    for (int i = 0; i < nCount; ++i, ++pHistory)
        *pHistory = m_pcmbSearchText->get_text(i);

    aCurrentSettings.sSingleSearchField         = m_plbField->get_active_text();
    aCurrentSettings.bAllFields                 = m_prbAllFields->get_active();
    aCurrentSettings.nPosition                  = m_pSearchEngine->GetPosition();
    aCurrentSettings.bUseFormatter              = m_pSearchEngine->GetFormatterUsing();
    aCurrentSettings.setCaseSensitive           ( m_pSearchEngine->GetCaseSensitive() );
    aCurrentSettings.bBackwards                 = !m_pSearchEngine->GetDirection();
    aCurrentSettings.bWildcard                  = m_pSearchEngine->GetWildcard();
    aCurrentSettings.bRegular                   = m_pSearchEngine->GetRegular();
    aCurrentSettings.bApproxSearch              = m_pSearchEngine->GetLevenshtein();
    aCurrentSettings.bLevRelaxed                = m_pSearchEngine->GetLevRelaxed();
    aCurrentSettings.nLevOther                  = m_pSearchEngine->GetLevOther();
    aCurrentSettings.nLevShorter                = m_pSearchEngine->GetLevShorter();
    aCurrentSettings.nLevLonger                 = m_pSearchEngine->GetLevLonger();

    aCurrentSettings.bSoundsLikeCJK             = m_pSearchEngine->GetTransliteration();
    aCurrentSettings.setTransliterationFlags    ( m_pSearchEngine->GetTransliterationFlags() );

    if (m_prbSearchForNull->get_active())
        aCurrentSettings.nSearchForType = 1;
    else if (m_prbSearchForNotNull->get_active())
        aCurrentSettings.nSearchForType = 2;
    else
        aCurrentSettings.nSearchForType = 0;

    m_pConfig->setParams( aCurrentSettings );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
