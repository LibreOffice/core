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
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <dialmgr.hxx>
#include <sfx2/tabdlg.hxx>
#include <osl/mutex.hxx>
#include <sfx2/app.hxx>
#include <cuires.hrc>
#include <svx/fmsrccfg.hxx>
#include <svx/fmsrcimp.hxx>
#include "fmsearch.hrc"
#include "cuifmsearch.hxx"
#include <svx/srchdlg.hxx>
#include <svl/cjkoptions.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
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
        m_pSoundsLikeCJK->Hide();
        m_pSoundsLikeCJKSettings->Hide();
    }

    if (!aCJKOptions.IsCJKFontEnabled())
    {
        m_pHalfFullFormsCJK->Hide();

        // never ignore the width (ignoring is expensive) if the option is not available at all
        m_pSearchEngine->SetIgnoreWidthCJK( false );
    }

    // some initial record texts
    m_pftRecord->SetText( OUString::number(_rxCursor->getRow()) );
    m_pbClose->SetHelpText(OUString());
}

FmSearchDialog::FmSearchDialog(vcl::Window* pParent, const OUString& sInitialText, const ::std::vector< OUString >& _rContexts, sal_Int16 nInitialContext,
    const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier)
    :ModalDialog(pParent, "RecordSearchDialog", "cui/ui/fmsearchdialog.ui")
    ,m_sCancel( Button::GetStandardText( StandardButtonType::Cancel ) )
    ,m_pPreSearchFocus( nullptr )
    ,m_lnkContextSupplier(lnkContextSupplier)
    ,m_pConfig( nullptr )
{
    get(m_prbSearchForText,"rbSearchForText");
    get(m_prbSearchForNull,"rbSearchForNull");
    get(m_prbSearchForNotNull,"rbSearchForNotNull");
    get(m_pcmbSearchText,"cmbSearchText");
    m_pcmbSearchText->set_width_request(m_pcmbSearchText->approximate_char_width() * 42);
    get(m_plbForm,"lbForm");
    m_plbForm->set_width_request(m_plbForm->approximate_char_width() * 42);
    get(m_prbAllFields,"rbAllFields");
    get(m_prbSingleField,"rbSingleField");
    get(m_plbField,"lbField");
    get(m_plbPosition,"lbPosition");
    get(m_pcbUseFormat,"cbUseFormat");
    get(m_pcbCase,"cbCase");
    get(m_pcbBackwards,"cbBackwards");
    get(m_pcbStartOver,"cbStartOver");
    get(m_pcbWildCard,"cbWildCard");
    get(m_pcbRegular,"cbRegular");
    get(m_pcbApprox,"cbApprox");
    get(m_ppbApproxSettings,"pbApproxSettings");
    get(m_pHalfFullFormsCJK,"HalfFullFormsCJK");
    get(m_pSoundsLikeCJK,"SoundsLikeCJK");
    get(m_pSoundsLikeCJKSettings,"SoundsLikeCJKSettings");
    get(m_pbSearchAgain,"pbSearchAgain");
    m_sSearch = m_pbSearchAgain->GetText();
    get(m_pftRecord,"ftRecord");
    get(m_pftHint,"ftHint");
    get(m_pftPosition,"ftPosition");
    get(m_pftForm,"ftForm");
    get(m_pbClose,"close");

    DBG_ASSERT(m_lnkContextSupplier.IsSet(), "FmSearchDialog::FmSearchDialog : have no ContextSupplier !");

    FmSearchContext fmscInitial;
    fmscInitial.nContext = nInitialContext;
    m_lnkContextSupplier.Call(fmscInitial);
    DBG_ASSERT(fmscInitial.xCursor.is(), "FmSearchDialog::FmSearchDialog : invalid data supplied by ContextSupplier !");
    DBG_ASSERT(comphelper::string::getTokenCount(fmscInitial.strUsedFields, ';') == (sal_Int32)fmscInitial.arrFields.size(),
        "FmSearchDialog::FmSearchDialog : invalid data supplied by ContextSupplied !");
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
    for (sal_Int32 i=0; i<(sal_Int32)fmscInitial.arrFields.size(); ++i)
        DBG_ASSERT(fmscInitial.arrFields.at(i).is(), "FmSearchDialog::FmSearchDialog : invalid data supplied by ContextSupplier !");
#endif // (OSL_DEBUG_LEVEL > 1) || DBG_UTIL

    for (   ::std::vector< OUString >::const_iterator context = _rContexts.begin();
            context != _rContexts.end();
            ++context
        )
    {
        m_arrContextFields.push_back(OUString());
        m_plbForm->InsertEntry(*context);
    }
    m_plbForm->SelectEntryPos(nInitialContext);

    m_plbForm->SetSelectHdl(LINK(this, FmSearchDialog, OnContextSelection));

    if (m_arrContextFields.size() == 1)
    {
        // hide dispensable controls
        m_pftForm->Hide();
        m_plbForm->Hide();
    }

    m_pSearchEngine = new FmSearchEngine(
        ::comphelper::getProcessComponentContext(), fmscInitial.xCursor, fmscInitial.strUsedFields, fmscInitial.arrFields, SM_ALLOWSCHEDULE );
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
    disposeOnce();
}

void FmSearchDialog::dispose()
{
    if (m_aDelayedPaint.IsActive())
        m_aDelayedPaint.Stop();

    SaveParams();

    delete m_pConfig;
    m_pConfig = nullptr;

    delete m_pSearchEngine;
    m_pSearchEngine = nullptr;

    m_prbSearchForText.clear();
    m_prbSearchForNull.clear();
    m_prbSearchForNotNull.clear();
    m_pcmbSearchText.clear();
    m_pftForm.clear();
    m_plbForm.clear();
    m_prbAllFields.clear();
    m_prbSingleField.clear();
    m_plbField.clear();
    m_pftPosition.clear();
    m_plbPosition.clear();
    m_pcbUseFormat.clear();
    m_pcbCase.clear();
    m_pcbBackwards.clear();
    m_pcbStartOver.clear();
    m_pcbWildCard.clear();
    m_pcbRegular.clear();
    m_pcbApprox.clear();
    m_ppbApproxSettings.clear();
    m_pHalfFullFormsCJK.clear();
    m_pSoundsLikeCJK.clear();
    m_pSoundsLikeCJKSettings.clear();
    m_pftRecord.clear();
    m_pftHint.clear();
    m_pbSearchAgain.clear();
    m_pbClose.clear();
    m_pPreSearchFocus.clear();

    ModalDialog::dispose();
}

void FmSearchDialog::Init(const OUString& strVisibleFields, const OUString& sInitialText)
{
    //the initialization of all the Controls
    m_prbSearchForText->SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_prbSearchForNull->SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_prbSearchForNotNull->SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));

    m_prbAllFields->SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_prbSingleField->SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));

    m_pbSearchAgain->SetClickHdl(LINK(this, FmSearchDialog, OnClickedSearchAgain));
    m_ppbApproxSettings->SetClickHdl(LINK(this, FmSearchDialog, OnClickedSpecialSettings));
    m_pSoundsLikeCJKSettings->SetClickHdl(LINK(this, FmSearchDialog, OnClickedSpecialSettings));

    m_plbPosition->SetSelectHdl(LINK(this, FmSearchDialog, OnPositionSelected));
    m_plbField->SetSelectHdl(LINK(this, FmSearchDialog, OnFieldSelected));

    m_pcmbSearchText->SetModifyHdl(LINK(this, FmSearchDialog, OnSearchTextModified));
    m_pcmbSearchText->EnableAutocomplete(false);
    m_pcmbSearchText->SetGetFocusHdl(LINK(this, FmSearchDialog, OnFocusGrabbed));

    m_pcbUseFormat->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbBackwards->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbStartOver->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbCase->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbWildCard->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbRegular->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pcbApprox->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pHalfFullFormsCJK->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_pSoundsLikeCJK->SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));

    // fill the listboxes
    // method of field comparison
    sal_uInt16 nResIds[] = {
        RID_STR_SEARCH_ANYWHERE,
        RID_STR_SEARCH_BEGINNING,
        RID_STR_SEARCH_END,
        RID_STR_SEARCH_WHOLE
    };
    for ( size_t i=0; i<SAL_N_ELEMENTS(nResIds); ++i )
        m_plbPosition->InsertEntry( OUString( CUI_RES( nResIds[i] ) ) );
    m_plbPosition->SelectEntryPos(MATCHING_ANYWHERE);

    // the field listbox
    for (sal_Int32 i=0; i < comphelper::string::getTokenCount(strVisibleFields, ';'); ++i)
        m_plbField->InsertEntry(strVisibleFields.getToken(i, ';'));


    m_pConfig = new FmSearchConfigItem;
    LoadParams();

    m_pcmbSearchText->SetText(sInitialText);
    // if the Edit-line has changed the text (e.g. because it contains
    // control characters, as can be the case with memo fields), I use
    // an empty OUString.
    OUString sRealSetText = m_pcmbSearchText->GetText();
    if (!sRealSetText.equals(sInitialText))
        m_pcmbSearchText->SetText(OUString());
    LINK(this, FmSearchDialog, OnSearchTextModified).Call(*m_pcmbSearchText);

    // initial
    m_aDelayedPaint.SetTimeoutHdl(LINK(this, FmSearchDialog, OnDelayedPaint));
    m_aDelayedPaint.SetTimeout(500);
    EnableSearchUI(true);

    if ( m_prbSearchForText->IsChecked() )
        m_pcmbSearchText->GrabFocus();

}

bool FmSearchDialog::Close()
{
    // If the close button is disabled and ESC is pressed in a dialog,
    // then Frame will call Close anyway, which I don't want to happen
    // while I'm in the middle of a search (maybe one that's running
    // in its own thread)
    if (!m_pbClose->IsEnabled())
        return false;
    return ModalDialog::Close();
}

IMPL_LINK_TYPED(FmSearchDialog, OnClickedFieldRadios, Button*, pButton, void)
{
    if ((pButton == m_prbSearchForText) || (pButton == m_prbSearchForNull) || (pButton == m_prbSearchForNotNull))
    {
        EnableSearchForDependees(true);
    }
    else
        // en- or disable field list box accordingly
        if (pButton == m_prbSingleField)
        {
            m_plbField->Enable();
            m_pSearchEngine->RebuildUsedFields(m_plbField->GetSelectEntryPos());
        }
        else
        {
            m_plbField->Disable();
            m_pSearchEngine->RebuildUsedFields(-1);
        }
}

IMPL_LINK_NOARG_TYPED(FmSearchDialog, OnClickedSearchAgain, Button*, void)
{
    if (m_pbClose->IsEnabled())
    {   // the button has the function 'search'
        OUString strThisRoundText = m_pcmbSearchText->GetText();
        // to history
        m_pcmbSearchText->RemoveEntry(strThisRoundText);
        m_pcmbSearchText->InsertEntry(strThisRoundText, 0);
        // the remove/insert makes sure that a) the OUString does not appear twice and
        // that b) the last searched strings are at the beginning and limit the list length
        while (m_pcmbSearchText->GetEntryCount() > MAX_HISTORY_ENTRIES)
            m_pcmbSearchText->RemoveEntryAt(m_pcmbSearchText->GetEntryCount()-1);

        // take out the 'overflow' hint
        m_pftHint->SetText(OUString());
        m_pftHint->Invalidate();

        if (m_pcbStartOver->IsChecked())
        {
            m_pcbStartOver->Check(false);
            EnableSearchUI(false);
            if (m_prbSearchForText->IsChecked())
                m_pSearchEngine->StartOver(strThisRoundText);
            else
                m_pSearchEngine->StartOverSpecial(m_prbSearchForNull->IsChecked());
        }
        else
        {
            EnableSearchUI(false);
            if (m_prbSearchForText->IsChecked())
                m_pSearchEngine->SearchNext(strThisRoundText);
            else
                m_pSearchEngine->SearchNextSpecial(m_prbSearchForNull->IsChecked());
        }
    }
    else
    {   // the button has the function 'cancel'
        DBG_ASSERT(m_pSearchEngine->GetSearchMode() != SM_BRUTE, "FmSearchDialog, OnClickedSearchAgain : falscher Modus !");
            // the CancelButton is usually only disabled, when working in a thread or with reschedule
        m_pSearchEngine->CancelSearch();
            // the ProgressHandler is called when it's really finished, here it's only a demand
    }
}

IMPL_LINK_TYPED(FmSearchDialog, OnClickedSpecialSettings, Button*, pButton, void )
{
    if (m_ppbApproxSettings == pButton)
    {
        std::unique_ptr<AbstractSvxSearchSimilarityDialog> pDlg;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if ( pFact )
            pDlg.reset(pFact->CreateSvxSearchSimilarityDialog( this, m_pSearchEngine->GetLevRelaxed(), m_pSearchEngine->GetLevOther(),
                        m_pSearchEngine->GetLevShorter(), m_pSearchEngine->GetLevLonger() ));
        DBG_ASSERT( pDlg, "FmSearchDialog, OnClickedSpecialSettings: could not load the dialog!" );

        if ( pDlg && pDlg->Execute() == RET_OK )
        {
            m_pSearchEngine->SetLevRelaxed( pDlg->IsRelaxed() );
            m_pSearchEngine->SetLevOther( pDlg->GetOther() );
            m_pSearchEngine->SetLevShorter(pDlg->GetShorter() );
            m_pSearchEngine->SetLevLonger( pDlg->GetLonger() );
        }
    }
    else if (m_pSoundsLikeCJKSettings == pButton)
    {
        SfxItemSet aSet( SfxGetpApp()->GetPool() );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            std::unique_ptr<AbstractSvxJSearchOptionsDialog> aDlg(pFact->CreateSvxJSearchOptionsDialog( this, aSet, m_pSearchEngine->GetTransliterationFlags() ));
            DBG_ASSERT(aDlg, "Dialog creation failed!");
            aDlg->Execute();


            sal_Int32 nFlags = aDlg->GetTransliterationFlags();
            m_pSearchEngine->SetTransliterationFlags(nFlags);

            m_pcbCase->Check(m_pSearchEngine->GetCaseSensitive());
            OnCheckBoxToggled( *m_pcbCase );
            m_pHalfFullFormsCJK->Check( !m_pSearchEngine->GetIgnoreWidthCJK() );
            OnCheckBoxToggled( *m_pHalfFullFormsCJK );
        }
    }
}

IMPL_LINK_NOARG_TYPED(FmSearchDialog, OnSearchTextModified, Edit&, void)
{
    if ((!m_pcmbSearchText->GetText().isEmpty()) || !m_prbSearchForText->IsChecked())
        m_pbSearchAgain->Enable();
    else
        m_pbSearchAgain->Disable();

    m_pSearchEngine->InvalidatePreviousLoc();
}

IMPL_LINK_NOARG_TYPED(FmSearchDialog, OnFocusGrabbed, Control&, void)
{
    m_pcmbSearchText->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
}

IMPL_LINK_TYPED(FmSearchDialog, OnPositionSelected, ListBox&, rBox, void)
{
    DBG_ASSERT(rBox.GetSelectEntryCount() == 1, "FmSearchDialog::OnMethodSelected : unerwartet : nicht genau ein Eintrag selektiert !");

    m_pSearchEngine->SetPosition(m_plbPosition->GetSelectEntryPos());
}

IMPL_LINK_TYPED(FmSearchDialog, OnFieldSelected, ListBox&, rBox, void)
{
    DBG_ASSERT(rBox.GetSelectEntryCount() == 1, "FmSearchDialog::OnFieldSelected : unerwartet : nicht genau ein Eintrag selektiert !");

    m_pSearchEngine->RebuildUsedFields(m_prbAllFields->IsChecked() ? -1 : (sal_Int16)m_plbField->GetSelectEntryPos());
        // calls m_pSearchEngine->InvalidatePreviousLoc too

    sal_Int32 nCurrentContext = m_plbForm->GetSelectEntryPos();
    if (nCurrentContext != LISTBOX_ENTRY_NOTFOUND)
        m_arrContextFields[nCurrentContext] = m_plbField->GetSelectEntry();
}

IMPL_LINK_TYPED(FmSearchDialog, OnCheckBoxToggled, CheckBox&, rBox, void)
{
    bool bChecked = rBox.IsChecked();

    // formatter or case -> pass on to the engine
    if (&rBox == m_pcbUseFormat)
        m_pSearchEngine->SetFormatterUsing(bChecked);
    else if (&rBox == m_pcbCase)
        m_pSearchEngine->SetCaseSensitive(bChecked);
    // direction -> pass on and reset the checkbox-text for StartOver
    else if (&rBox == m_pcbBackwards)
    {
        m_pcbStartOver->SetText( OUString( CUI_RES( bChecked ? RID_STR_FROM_BOTTOM : RID_STR_FROM_TOP ) ) );
        m_pSearchEngine->SetDirection(!bChecked);
    }
    // similarity-search or regular expression
    else if ((&rBox == m_pcbApprox) || (&rBox == m_pcbRegular) || (&rBox == m_pcbWildCard))
    {
        CheckBox* pBoxes[] = { m_pcbWildCard, m_pcbRegular, m_pcbApprox };
        for (sal_uInt32 i=0; i< SAL_N_ELEMENTS(pBoxes); ++i)
        {
            if (pBoxes[i] != &rBox)
            {
                if (bChecked)
                    pBoxes[i]->Disable();
                else
                    pBoxes[i]->Enable();
            }
        }

        // pass on to the engine
        m_pSearchEngine->SetWildcard(m_pcbWildCard->IsEnabled() && m_pcbWildCard->IsChecked());
        m_pSearchEngine->SetRegular(m_pcbRegular->IsEnabled() && m_pcbRegular->IsChecked());
        m_pSearchEngine->SetLevenshtein(m_pcbApprox->IsEnabled() && m_pcbApprox->IsChecked());
            // (disabled boxes have to be passed to the engine as sal_False)

        // adjust the Position-Listbox (which is not allowed during Wildcard-search)
        if (&rBox == m_pcbWildCard)
        {
            if (bChecked)
            {
                m_pftPosition->Disable();
                m_plbPosition->Disable();
            }
            else
            {
                m_pftPosition->Enable();
                m_plbPosition->Enable();
            }
        }

        // and the button for similarity-search
        if (&rBox == m_pcbApprox)
        {
            if (bChecked)
                m_ppbApproxSettings->Enable();
            else
                m_ppbApproxSettings->Disable();
        }
    }
    else if (&rBox == m_pHalfFullFormsCJK)
    {
        // forward to the search engine
        m_pSearchEngine->SetIgnoreWidthCJK( !bChecked );
    }
    else if (&rBox == m_pSoundsLikeCJK)
    {
        m_pSoundsLikeCJKSettings->Enable(bChecked);

        // two other buttons which depend on this one
        bool bEnable =  (   m_prbSearchForText->IsChecked()
                            &&  !m_pSoundsLikeCJK->IsChecked()
                            )
                         || !SvtCJKOptions().IsJapaneseFindEnabled();
        m_pcbCase->Enable(bEnable);
        m_pHalfFullFormsCJK->Enable(bEnable);

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
    m_plbField->Clear();

    if (!fmscContext.sFieldDisplayNames.isEmpty())
    {
        // use the display names if supplied
        DBG_ASSERT(comphelper::string::getTokenCount(fmscContext.sFieldDisplayNames, ';') == comphelper::string::getTokenCount(fmscContext.strUsedFields, ';'),
            "FmSearchDialog::InitContext : invalid context description supplied !");
        for (sal_Int32 i=0; i < comphelper::string::getTokenCount(fmscContext.sFieldDisplayNames, ';'); ++i)
            m_plbField->InsertEntry(fmscContext.sFieldDisplayNames.getToken(i, ';'));
    }
    else
    {
        // else use the field names
        for (sal_Int32 i=0; i < comphelper::string::getTokenCount(fmscContext.strUsedFields, ';'); ++i)
            m_plbField->InsertEntry(fmscContext.strUsedFields.getToken(i, ';'));
    }

    if (nContext < (sal_Int32)m_arrContextFields.size() && !m_arrContextFields[nContext].isEmpty())
    {
        m_plbField->SelectEntry(m_arrContextFields[nContext]);
    }
    else
    {
        m_plbField->SelectEntryPos(0);
        if (m_prbSingleField->IsChecked() && (m_plbField->GetEntryCount() > 1))
            m_plbField->GrabFocus();
    }

    m_pSearchEngine->SwitchToContext(fmscContext.xCursor, fmscContext.strUsedFields, fmscContext.arrFields,
        m_prbAllFields->IsChecked() ? -1 : 0);

    m_pftRecord->SetText(OUString::number(fmscContext.xCursor->getRow()));
}

IMPL_LINK_TYPED( FmSearchDialog, OnContextSelection, ListBox&, rBox, void)
{
    InitContext(rBox.GetSelectEntryPos());
}

void FmSearchDialog::EnableSearchUI(bool bEnable)
{
    // when the controls shall be disabled their paint is turned off and then turned on again after a delay
    if (!bEnable)
        EnableControlPaint(false);
    else
    {
        if (m_aDelayedPaint.IsActive())
            m_aDelayedPaint.Stop();
    }
    // (the whole thing goes on below)
    // this small intricateness hopefully leads to no flickering when turning the SearchUI off
    // and on again shortly after (like it's the case during a short search process)

    if ( !bEnable )
    {
        // if one of my children has the focus, remember it
        vcl::Window* pFocusWindow = Application::GetFocusWindow( );
        if ( pFocusWindow && IsChild( pFocusWindow ) )
            m_pPreSearchFocus = pFocusWindow;
        else
            m_pPreSearchFocus = nullptr;
    }

    // the search button has two functions -> adjust its text accordingly
    OUString sButtonText( bEnable ? m_sSearch : m_sCancel );
    m_pbSearchAgain->SetText( sButtonText );

    if (m_pSearchEngine->GetSearchMode() != SM_BRUTE)
    {
        m_prbSearchForText->Enable    (bEnable);
        m_prbSearchForNull->Enable    (bEnable);
        m_prbSearchForNotNull->Enable (bEnable);
        m_plbForm->Enable             (bEnable);
        m_prbAllFields->Enable        (bEnable);
        m_prbSingleField->Enable      (bEnable);
        m_plbField->Enable            (bEnable && m_prbSingleField->IsChecked());
        m_pcbBackwards->Enable        (bEnable);
        m_pcbStartOver->Enable        (bEnable);
        m_pbClose->Enable            (bEnable);
        EnableSearchForDependees    (bEnable);

        if ( !bEnable )
        {   // this means we're preparing for starting a search
            // In this case, EnableSearchForDependees disabled the search button
            // But as we're about to use it for cancelling the search, we really need to enable it, again
            m_pbSearchAgain->Enable();
        }
    }

    if (!bEnable)
        m_aDelayedPaint.Start();
    else
        EnableControlPaint(true);

    if ( bEnable )
    {   // restore focus
        if ( m_pPreSearchFocus )
        {
            m_pPreSearchFocus->GrabFocus();
            if ( WINDOW_EDIT == m_pPreSearchFocus->GetType() )
            {
                Edit* pEdit = static_cast< Edit* >( m_pPreSearchFocus.get() );
                pEdit->SetSelection( Selection( 0, pEdit->GetText().getLength() ) );
            }
        }
        m_pPreSearchFocus = nullptr;
    }

}

void FmSearchDialog::EnableSearchForDependees(bool bEnable)
{
    bool bSearchingForText = m_prbSearchForText->IsChecked();
    m_pbSearchAgain->Enable(bEnable && (!bSearchingForText || (!m_pcmbSearchText->GetText().isEmpty())));

    bEnable = bEnable && bSearchingForText;

    bool bEnableRedundants = !m_pSoundsLikeCJK->IsChecked() || !SvtCJKOptions().IsJapaneseFindEnabled();

    m_pcmbSearchText->Enable          (bEnable);
    m_pftPosition->Enable             (bEnable && !m_pcbWildCard->IsChecked());
    m_pcbWildCard->Enable             (bEnable && !m_pcbRegular->IsChecked() && !m_pcbApprox->IsChecked());
    m_pcbRegular->Enable              (bEnable && !m_pcbWildCard->IsChecked() && !m_pcbApprox->IsChecked());
    m_pcbApprox->Enable               (bEnable && !m_pcbWildCard->IsChecked() && !m_pcbRegular->IsChecked());
    m_ppbApproxSettings->Enable       (bEnable && m_pcbApprox->IsChecked());
    m_pHalfFullFormsCJK->Enable      (bEnable && bEnableRedundants);
    m_pSoundsLikeCJK->Enable         (bEnable);
    m_pSoundsLikeCJKSettings->Enable (bEnable && m_pSoundsLikeCJK->IsChecked());
    m_plbPosition->Enable             (bEnable && !m_pcbWildCard->IsChecked());
    m_pcbUseFormat->Enable            (bEnable);
    m_pcbCase->Enable                 (bEnable && bEnableRedundants);
}

void FmSearchDialog::EnableControlPaint(bool bEnable)
{
    Control* pAffectedControls[] = { m_prbSearchForText, m_pcmbSearchText, m_prbSearchForNull, m_prbSearchForNotNull,
        m_prbSearchForText, m_prbAllFields, m_prbSingleField, m_plbField, m_pftPosition, m_plbPosition,
        m_pcbUseFormat, m_pcbCase, m_pcbBackwards, m_pcbStartOver, m_pcbWildCard, m_pcbRegular, m_pcbApprox, m_ppbApproxSettings,
        m_pbSearchAgain, m_pbClose };

    if (!bEnable)
        for (sal_uInt32 i=0; i<SAL_N_ELEMENTS(pAffectedControls); ++i)
        {
            pAffectedControls[i]->SetUpdateMode(bEnable);
            pAffectedControls[i]->EnablePaint(bEnable);
        }
    else
        for (sal_uInt32 i=0; i<SAL_N_ELEMENTS(pAffectedControls); ++i)
        {
            pAffectedControls[i]->EnablePaint(bEnable);
            pAffectedControls[i]->SetUpdateMode(bEnable);
        }
}

IMPL_LINK_NOARG_TYPED(FmSearchDialog, OnDelayedPaint, Timer *, void)
{
    EnableControlPaint(true);
}

void FmSearchDialog::OnFound(const css::uno::Any& aCursorPos, sal_Int16 nFieldPos)
{
    FmFoundRecordInformation friInfo;
    friInfo.nContext = m_plbForm->GetSelectEntryPos();
    // if I don't do a search in a context, this has an invalid value - but then it doesn't matter anyway
    friInfo.aPosition = aCursorPos;
    if (m_prbAllFields->IsChecked())
        friInfo.nFieldPos = nFieldPos;
    else
        friInfo.nFieldPos = m_plbField->GetSelectEntryPos();
        // this of course implies that I have really searched in the field that is selected in the listbox,
        // which is made sure in RebuildUsedFields

    m_lnkFoundHandler.Call(friInfo);

    m_pcmbSearchText->GrabFocus();
}

IMPL_LINK_TYPED(FmSearchDialog, OnSearchProgress, const FmSearchProgress*, pProgress, void)
{
    SolarMutexGuard aGuard;
        // make this single method thread-safe (it's an overkill to block the whole application for this,
        // but we don't have another safety concept at the moment)

    switch (pProgress->aSearchState)
    {
        case FmSearchProgress::STATE_PROGRESS:
            if (pProgress->bOverflow)
            {
                OUString sHint( CUI_RES( m_pcbBackwards->IsChecked() ? RID_STR_OVERFLOW_BACKWARD : RID_STR_OVERFLOW_FORWARD ) );
                m_pftHint->SetText( sHint );
                m_pftHint->Invalidate();
            }

            m_pftRecord->SetText(OUString::number(1 + pProgress->nCurrentRecord));
            m_pftRecord->Invalidate();
            break;

        case FmSearchProgress::STATE_PROGRESS_COUNTING:
            m_pftHint->SetText(CUI_RESSTR(RID_STR_SEARCH_COUNTING));
            m_pftHint->Invalidate();

            m_pftRecord->SetText(OUString::number(pProgress->nCurrentRecord));
            m_pftRecord->Invalidate();
            break;

        case FmSearchProgress::STATE_SUCCESSFULL:
            OnFound(pProgress->aBookmark, (sal_Int16)pProgress->nFieldIndex);
            EnableSearchUI(true);
            break;

        case FmSearchProgress::STATE_ERROR:
        case FmSearchProgress::STATE_NOTHINGFOUND:
        {
            sal_uInt16 nErrorId = (FmSearchProgress::STATE_ERROR == pProgress->aSearchState)
                ? RID_STR_SEARCH_GENERAL_ERROR
                : RID_STR_SEARCH_NORECORD;
            ScopedVclPtrInstance<MessageDialog>::Create(this, CUI_RES(nErrorId))->Execute();
            SAL_FALLTHROUGH;
        }
        case FmSearchProgress::STATE_CANCELED:
            EnableSearchUI(true);
            if (m_lnkCanceledNotFoundHdl.IsSet())
            {
                FmFoundRecordInformation friInfo;
                friInfo.nContext = m_plbForm->GetSelectEntryPos();
                // if I don't do a search in a context, this has an invalid value - but then it doesn't matter anyway
                friInfo.aPosition = pProgress->aBookmark;
                m_lnkCanceledNotFoundHdl.Call(friInfo);
            }
            break;
    }

    m_pftRecord->SetText(OUString::number(1 + pProgress->nCurrentRecord));
}

void FmSearchDialog::LoadParams()
{
    FmSearchParams aParams(m_pConfig->getParams());

    const OUString* pHistory     =                   aParams.aHistory.getConstArray();
    const OUString* pHistoryEnd  =   pHistory    +   aParams.aHistory.getLength();
    for (; pHistory != pHistoryEnd; ++pHistory)
        m_pcmbSearchText->InsertEntry( *pHistory );

    // I do the settings at my UI-elements and then I simply call the respective change-handler,
    // that way the data is handed on to the SearchEngine and all dependent settings are done

    // current field
    sal_Int32 nInitialField = m_plbField->GetEntryPos( OUString( aParams.sSingleSearchField ) );
    if (nInitialField == LISTBOX_ENTRY_NOTFOUND)
        nInitialField = 0;
    m_plbField->SelectEntryPos(nInitialField);
    LINK(this, FmSearchDialog, OnFieldSelected).Call(*m_plbField);
    // all fields/single field (AFTER selecting the field because OnClickedFieldRadios expects a valid value there)
    if (aParams.bAllFields)
    {
        m_prbSingleField->Check(false);
        m_prbAllFields->Check();
        LINK(this, FmSearchDialog, OnClickedFieldRadios).Call(m_prbAllFields);
        // OnClickedFieldRadios also calls to RebuildUsedFields
    }
    else
    {
        m_prbAllFields->Check(false);
        m_prbSingleField->Check();
        LINK(this, FmSearchDialog, OnClickedFieldRadios).Call(m_prbSingleField);
    }

    m_plbPosition->SelectEntryPos(aParams.nPosition);
    LINK(this, FmSearchDialog, OnPositionSelected).Call(*m_plbPosition);

    // field formatting/case sensitivity/direction
    m_pcbUseFormat->Check(aParams.bUseFormatter);
    m_pcbCase->Check( aParams.isCaseSensitive() );
    m_pcbBackwards->Check(aParams.bBackwards);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*m_pcbUseFormat);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*m_pcbCase);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*m_pcbBackwards);

    m_pHalfFullFormsCJK->Check( !aParams.isIgnoreWidthCJK( ) );  // BEWARE: this checkbox has a inverse semantics!
    m_pSoundsLikeCJK->Check( aParams.bSoundsLikeCJK );
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*m_pHalfFullFormsCJK);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*m_pSoundsLikeCJK);

    m_pcbWildCard->Check(false);
    m_pcbRegular->Check(false);
    m_pcbApprox->Check(false);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*m_pcbWildCard);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*m_pcbRegular);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*m_pcbApprox);

    CheckBox* pToCheck = nullptr;
    if (aParams.bWildcard)
        pToCheck = m_pcbWildCard;
    if (aParams.bRegular)
        pToCheck = m_pcbRegular;
    if (aParams.bApproxSearch)
        pToCheck = m_pcbApprox;
    if (aParams.bSoundsLikeCJK)
        pToCheck = m_pSoundsLikeCJK;
    if (pToCheck)
    {
        pToCheck->Check();
        LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(*pToCheck);
    }

    // set Levenshtein-parameters directly at the SearchEngine
    m_pSearchEngine->SetLevRelaxed(aParams.bLevRelaxed);
    m_pSearchEngine->SetLevOther(aParams.nLevOther);
    m_pSearchEngine->SetLevShorter(aParams.nLevShorter);
    m_pSearchEngine->SetLevLonger(aParams.nLevLonger);

    m_pSearchEngine->SetTransliterationFlags( aParams.getTransliterationFlags( ) );

    m_prbSearchForText->Check(false);
    m_prbSearchForNull->Check(false);
    m_prbSearchForNotNull->Check(false);
    switch (aParams.nSearchForType)
    {
        case 1: m_prbSearchForNull->Check(); break;
        case 2: m_prbSearchForNotNull->Check(); break;
        default: m_prbSearchForText->Check(); break;
    }
    LINK(this, FmSearchDialog, OnClickedFieldRadios).Call(m_prbSearchForText);
}

void FmSearchDialog::SaveParams() const
{
    if (!m_pConfig)
        return;

    FmSearchParams aCurrentSettings;

    aCurrentSettings.aHistory.realloc( m_pcmbSearchText->GetEntryCount() );
    OUString* pHistory = aCurrentSettings.aHistory.getArray();
    for (sal_Int32 i=0; i<m_pcmbSearchText->GetEntryCount(); ++i, ++pHistory)
        *pHistory = m_pcmbSearchText->GetEntry(i);

    aCurrentSettings.sSingleSearchField         = m_plbField->GetSelectEntry();
    aCurrentSettings.bAllFields                 = m_prbAllFields->IsChecked();
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

    if (m_prbSearchForNull->IsChecked())
        aCurrentSettings.nSearchForType = 1;
    else if (m_prbSearchForNotNull->IsChecked())
        aCurrentSettings.nSearchForType = 2;
    else
        aCurrentSettings.nSearchForType = 0;

    m_pConfig->setParams( aCurrentSettings );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
