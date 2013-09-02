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
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <tools/shl.hxx>
#include <dialmgr.hxx>
#include <sfx2/tabdlg.hxx>
#include <osl/mutex.hxx>
#include <sfx2/app.hxx>
#include <cuires.hrc>
#include <svl/filerec.hxx>
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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::svxform;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;

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
        sal_Int32 nUpper = m_cbApprox.GetPosPixel().Y();
        sal_Int32 nDifference = m_aSoundsLikeCJKSettings.GetPosPixel().Y() - nUpper;

        // hide the options for the japanese search
        Control* pFieldsToMove[] = { &m_flState, &m_ftRecordLabel, &m_ftRecord, &m_ftHint };
        implMoveControls(pFieldsToMove, SAL_N_ELEMENTS(pFieldsToMove), nDifference, &m_flOptions);

        m_aSoundsLikeCJK.Hide();
        m_aSoundsLikeCJKSettings.Hide();
    }

    if (!aCJKOptions.IsCJKFontEnabled())
    {
        m_aHalfFullFormsCJK.Hide();

        // never ignore the width (ignoring is expensive) if the option is not available at all
        m_pSearchEngine->SetIgnoreWidthCJK( sal_False );
    }

    // some initial record texts
    m_ftRecord.SetText( OUString::number(_rxCursor->getRow()) );
    m_pbClose.SetHelpText(String());
}

FmSearchDialog::FmSearchDialog(Window* pParent, const OUString& sInitialText, const ::std::vector< OUString >& _rContexts, sal_Int16 nInitialContext,
    const Link& lnkContextSupplier)
    :ModalDialog(pParent, CUI_RES(RID_SVXDLG_SEARCHFORM))
    ,m_flSearchFor              (this, CUI_RES(FL_SEARCHFOR))
    ,m_rbSearchForText          (this, CUI_RES(RB_SEARCHFORTEXT))
    ,m_rbSearchForNull          (this, CUI_RES(RB_SEARCHFORNULL))
    ,m_rbSearchForNotNull       (this, CUI_RES(RB_SEARCHFORNOTNULL))
    ,m_cmbSearchText            (this, CUI_RES(CMB_SEARCHTEXT))
    ,m_flWhere                  (this, CUI_RES(FL_WHERE))
    ,m_ftForm                   (this, CUI_RES(FT_FORM))
    ,m_lbForm                   (this, CUI_RES(LB_FORM))
    ,m_rbAllFields              (this, CUI_RES(RB_ALLFIELDS))
    ,m_rbSingleField            (this, CUI_RES(RB_SINGLEFIELD))
    ,m_lbField                  (this, CUI_RES(LB_FIELD))
    ,m_flOptions                (this, CUI_RES(FL_OPTIONS))
    ,m_ftPosition               (this, CUI_RES(FT_POSITION))
    ,m_lbPosition               (this, CUI_RES(LB_POSITION))
    ,m_cbUseFormat              (this, CUI_RES(CB_USEFORMATTER))
    ,m_cbCase                   (this, CUI_RES(CB_CASE))
    ,m_cbBackwards              (this, CUI_RES(CB_BACKWARD))
    ,m_cbStartOver              (this, CUI_RES(CB_STARTOVER))
    ,m_cbWildCard               (this, CUI_RES(CB_WILDCARD))
    ,m_cbRegular                (this, CUI_RES(CB_REGULAR))
    ,m_cbApprox                 (this, CUI_RES(CB_APPROX))
    ,m_pbApproxSettings         (this, CUI_RES(PB_APPROXSETTINGS))
    ,m_aHalfFullFormsCJK        (this, CUI_RES(CB_HALFFULLFORMS))
    ,m_aSoundsLikeCJK           (this, CUI_RES(CB_SOUNDSLIKECJK))
    ,m_aSoundsLikeCJKSettings   (this, CUI_RES(PB_SOUNDSLIKESETTINGS))
    ,m_flState                  (this, CUI_RES(FL_STATE))
    ,m_ftRecordLabel            (this, CUI_RES(FT_RECORDLABEL))
    ,m_ftRecord                 (this, CUI_RES(FT_RECORD))
    ,m_ftHint                   (this, CUI_RES(FT_HINT))
    ,m_pbSearchAgain            (this, CUI_RES(PB_SEARCH))
    ,m_pbClose                  (this, CUI_RES(1))
    ,m_pbHelp                   (this, CUI_RES(1))
    ,m_sSearch                  ( m_pbSearchAgain.GetText() )
    ,m_sCancel                  ( Button::GetStandardText( BUTTON_CANCEL ) )
    ,m_pPreSearchFocus( NULL )
    ,m_lnkContextSupplier(lnkContextSupplier)
    ,m_pConfig( NULL )
{
    DBG_ASSERT(m_lnkContextSupplier.IsSet(), "FmSearchDialog::FmSearchDialog : have no ContextSupplier !");

    FmSearchContext fmscInitial;
    fmscInitial.nContext = nInitialContext;
    m_lnkContextSupplier.Call(&fmscInitial);
    DBG_ASSERT(fmscInitial.xCursor.is(), "FmSearchDialog::FmSearchDialog : invalid data supplied by ContextSupplier !");
    DBG_ASSERT(comphelper::string::getTokenCount(fmscInitial.strUsedFields, ';') == (xub_StrLen)fmscInitial.arrFields.size(),
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
        m_lbForm.InsertEntry(*context);
    }
    m_lbForm.SelectEntryPos(nInitialContext);

    m_lbForm.SetSelectHdl(LINK(this, FmSearchDialog, OnContextSelection));

    if (m_arrContextFields.size() == 1)
    {   // remove the context selection listbox and rearrange the controls accordingly
        sal_Int32 nUpper = m_lbForm.GetPosPixel().Y();
        sal_Int32 nDifference = m_rbAllFields.GetPosPixel().Y() - nUpper;

        // move all controls below the affected ones up
        Control* pFieldsToMove[] = { &m_rbAllFields, &m_rbSingleField, &m_lbField, &m_flOptions, &m_ftPosition, &m_lbPosition,
                &m_cbUseFormat, &m_cbCase, &m_cbBackwards, &m_cbStartOver, &m_cbWildCard, &m_cbRegular, &m_cbApprox,
                &m_pbApproxSettings, &m_aHalfFullFormsCJK, &m_aSoundsLikeCJK, &m_aSoundsLikeCJKSettings,
                &m_flState, &m_ftRecordLabel, &m_ftRecord, &m_ftHint };

        implMoveControls(pFieldsToMove, SAL_N_ELEMENTS(pFieldsToMove), nDifference, &m_flWhere);

        Point pt = m_rbAllFields.GetPosPixel();
        pt.X() = m_ftForm.GetPosPixel().X();
        m_rbAllFields.SetPosPixel( pt );
        pt = m_rbSingleField.GetPosPixel();
        pt.X() = m_ftForm.GetPosPixel().X();
        m_rbSingleField.SetPosPixel( pt );

        // hide dispensable controls
        m_ftForm.Hide();
        m_lbForm.Hide();
    }

    m_pSearchEngine = new FmSearchEngine(
        ::comphelper::getProcessComponentContext(), fmscInitial.xCursor, fmscInitial.strUsedFields, fmscInitial.arrFields, SM_ALLOWSCHEDULE );
    initCommon( fmscInitial.xCursor );

    if (fmscInitial.sFieldDisplayNames.Len() != 0)
    {   // use the display names if supplied
        DBG_ASSERT(comphelper::string::getTokenCount(fmscInitial.sFieldDisplayNames, ';') == comphelper::string::getTokenCount(fmscInitial.strUsedFields, ';'),
            "FmSearchDialog::FmSearchDialog : invalid initial context description !");
        Init(fmscInitial.sFieldDisplayNames, sInitialText);
    }
    else
        Init(fmscInitial.strUsedFields, sInitialText);
}

void FmSearchDialog::implMoveControls(
            Control** _ppControls,
            sal_Int32 _nControls,
            sal_Int32 _nUp,
            Control* /*_pToResize*/)
{
    for (sal_Int32 i=0; i<_nControls; ++i)
    {
        Point pt = _ppControls[i]->GetPosPixel();
        pt.Y() -= _nUp;
        _ppControls[i]->SetPosPixel(pt);
    }

    // resize myself
    Size sz = GetSizePixel();
    sz.Height() -= _nUp;
    SetSizePixel(sz);
}

FmSearchDialog::~FmSearchDialog()
{
    if (m_aDelayedPaint.IsActive())
        m_aDelayedPaint.Stop();

    SaveParams();

    if (m_pConfig)
    {
        delete m_pConfig;
        m_pConfig = NULL;
    }

    delete m_pSearchEngine;
}

void FmSearchDialog::Init(const OUString& strVisibleFields, const OUString& sInitialText)
{
    //the initialization of all the Controls
    m_rbSearchForText.SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_rbSearchForNull.SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_rbSearchForNotNull.SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));

    m_rbAllFields.SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));
    m_rbSingleField.SetClickHdl(LINK(this, FmSearchDialog, OnClickedFieldRadios));

    m_pbSearchAgain.SetClickHdl(LINK(this, FmSearchDialog, OnClickedSearchAgain));
    m_pbApproxSettings.SetClickHdl(LINK(this, FmSearchDialog, OnClickedSpecialSettings));
    m_aSoundsLikeCJKSettings.SetClickHdl(LINK(this, FmSearchDialog, OnClickedSpecialSettings));

    m_lbPosition.SetSelectHdl(LINK(this, FmSearchDialog, OnPositionSelected));
    m_lbField.SetSelectHdl(LINK(this, FmSearchDialog, OnFieldSelected));

    m_cmbSearchText.SetModifyHdl(LINK(this, FmSearchDialog, OnSearchTextModified));
    m_cmbSearchText.EnableAutocomplete(sal_False);
    m_cmbSearchText.SetGetFocusHdl(LINK(this, FmSearchDialog, OnFocusGrabbed));

    m_cbUseFormat.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_cbBackwards.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_cbStartOver.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_cbCase.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_cbWildCard.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_cbRegular.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_cbApprox.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_aHalfFullFormsCJK.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));
    m_aSoundsLikeCJK.SetToggleHdl(LINK(this, FmSearchDialog, OnCheckBoxToggled));

    // fill the listboxes
    // method of field comparison
    sal_uInt16 nResIds[] = {
        RID_STR_SEARCH_ANYWHERE,
        RID_STR_SEARCH_BEGINNING,
        RID_STR_SEARCH_END,
        RID_STR_SEARCH_WHOLE
    };
    for ( size_t i=0; i<SAL_N_ELEMENTS(nResIds); ++i )
        m_lbPosition.InsertEntry( String( CUI_RES( nResIds[i] ) ) );
    m_lbPosition.SelectEntryPos(MATCHING_ANYWHERE);

    // the field listbox
    for (sal_Int32 i=0; i < comphelper::string::getTokenCount(strVisibleFields, ';'); ++i)
        m_lbField.InsertEntry(comphelper::string::getToken(strVisibleFields, i, ';'));


    m_pConfig = new FmSearchConfigItem;
    LoadParams();

    m_cmbSearchText.SetText(sInitialText);
    // if the Edit-line has changed the text (e.g. because it contains
    // control characters, as can be the case with memo fields), I use
    // an empty OUString.
    OUString sRealSetText = m_cmbSearchText.GetText();
    if (!sRealSetText.equals(sInitialText))
        m_cmbSearchText.SetText(OUString());
    LINK(this, FmSearchDialog, OnSearchTextModified).Call(&m_cmbSearchText);

    // initial
    m_aDelayedPaint.SetTimeoutHdl(LINK(this, FmSearchDialog, OnDelayedPaint));
    m_aDelayedPaint.SetTimeout(500);
    EnableSearchUI(sal_True);

    if ( m_rbSearchForText.IsChecked() )
        m_cmbSearchText.GrabFocus();

    FreeResource();
}

sal_Bool FmSearchDialog::Close()
{
    // If the close button is disabled and ESC is pressed in a dialog,
    // then Frame will call Close anyway, which I don't want to happen
    // while I'm in the middle of a search (maybe one that's running
    // in its own thread)
    if (!m_pbClose.IsEnabled())
        return sal_False;
    return ModalDialog::Close();
}

IMPL_LINK(FmSearchDialog, OnClickedFieldRadios, Button*, pButton)
{
    if ((pButton == &m_rbSearchForText) || (pButton == &m_rbSearchForNull) || (pButton == &m_rbSearchForNotNull))
    {
        EnableSearchForDependees(sal_True);
    }
    else
        // en- or disable field list box accordingly
        if (pButton == &m_rbSingleField)
        {
            m_lbField.Enable();
            m_pSearchEngine->RebuildUsedFields(m_lbField.GetSelectEntryPos());
        }
        else
        {
            m_lbField.Disable();
            m_pSearchEngine->RebuildUsedFields(-1);
        }

    return 0;
}

IMPL_LINK_NOARG(FmSearchDialog, OnClickedSearchAgain)
{
    if (m_pbClose.IsEnabled())
    {   // the button has the function 'search'
        OUString strThisRoundText = m_cmbSearchText.GetText();
        // to history
        m_cmbSearchText.RemoveEntry(strThisRoundText);
        m_cmbSearchText.InsertEntry(strThisRoundText, 0);
        // the remove/insert makes sure that a) the OUString does not appear twice and
        // that b) the last searched strings are at the beginning and limit the list length
        while (m_cmbSearchText.GetEntryCount() > MAX_HISTORY_ENTRIES)
            m_cmbSearchText.RemoveEntryAt(m_cmbSearchText.GetEntryCount()-1);

        // take out the 'overflow' hint
        m_ftHint.SetText(OUString());
        m_ftHint.Invalidate();

        if (m_cbStartOver.IsChecked())
        {
            m_cbStartOver.Check(sal_False);
            EnableSearchUI(sal_False);
            if (m_rbSearchForText.IsChecked())
                m_pSearchEngine->StartOver(strThisRoundText);
            else
                m_pSearchEngine->StartOverSpecial(m_rbSearchForNull.IsChecked());
        }
        else
        {
            EnableSearchUI(sal_False);
            if (m_rbSearchForText.IsChecked())
                m_pSearchEngine->SearchNext(strThisRoundText);
            else
                m_pSearchEngine->SearchNextSpecial(m_rbSearchForNull.IsChecked());
        }
    }
    else
    {   // the button has the function 'cancel'
        DBG_ASSERT(m_pSearchEngine->GetSearchMode() != SM_BRUTE, "FmSearchDialog, OnClickedSearchAgain : falscher Modus !");
            // the CancelButton is usually only disabled, when working in a thread or with reschedule
        m_pSearchEngine->CancelSearch();
            // the ProgressHandler is called when it's really finished, here it's only a demand
    }
    return 0;
}

IMPL_LINK(FmSearchDialog, OnClickedSpecialSettings, Button*, pButton )
{
    if (&m_pbApproxSettings == pButton)
    {
        AbstractSvxSearchSimilarityDialog* pDlg = NULL;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if ( pFact )
            pDlg = pFact->CreateSvxSearchSimilarityDialog( this, m_pSearchEngine->GetLevRelaxed(), m_pSearchEngine->GetLevOther(),
                        m_pSearchEngine->GetLevShorter(), m_pSearchEngine->GetLevLonger() );
        DBG_ASSERT( pDlg, "FmSearchDialog, OnClickedSpecialSettings: could not load the dialog!" );

        if ( pDlg && pDlg->Execute() == RET_OK )
        {
            m_pSearchEngine->SetLevRelaxed( pDlg->IsRelaxed() );
            m_pSearchEngine->SetLevOther( pDlg->GetOther() );
            m_pSearchEngine->SetLevShorter(pDlg->GetShorter() );
            m_pSearchEngine->SetLevLonger( pDlg->GetLonger() );
        }
        delete pDlg;
    }
    else if (&m_aSoundsLikeCJKSettings == pButton)
    {
        SfxItemSet aSet( SFX_APP()->GetPool() );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            AbstractSvxJSearchOptionsDialog* aDlg = pFact->CreateSvxJSearchOptionsDialog( this, aSet, m_pSearchEngine->GetTransliterationFlags() );
            DBG_ASSERT(aDlg, "Dialogdiet fail!");
            aDlg->Execute();


            sal_Int32 nFlags = aDlg->GetTransliterationFlags();
            m_pSearchEngine->SetTransliterationFlags(nFlags);

            m_cbCase.Check(m_pSearchEngine->GetCaseSensitive());
            OnCheckBoxToggled( &m_cbCase );
            m_aHalfFullFormsCJK.Check( !m_pSearchEngine->GetIgnoreWidthCJK() );
            OnCheckBoxToggled( &m_aHalfFullFormsCJK );
            delete aDlg;
        }
    }

    return 0;
}

IMPL_LINK_NOARG(FmSearchDialog, OnSearchTextModified)
{
    if ((!m_cmbSearchText.GetText().isEmpty()) || !m_rbSearchForText.IsChecked())
        m_pbSearchAgain.Enable();
    else
        m_pbSearchAgain.Disable();

    m_pSearchEngine->InvalidatePreviousLoc();
    return 0;
}

IMPL_LINK(FmSearchDialog, OnFocusGrabbed, ComboBox*, EMPTYARG)
{
    m_cmbSearchText.SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
    return 0;
}

IMPL_LINK(FmSearchDialog, OnPositionSelected, ListBox*, pBox)
{
    (void) pBox; // avoid warning
    DBG_ASSERT(pBox->GetSelectEntryCount() == 1, "FmSearchDialog::OnMethodSelected : unerwartet : nicht genau ein Eintrag selektiert !");

    m_pSearchEngine->SetPosition(m_lbPosition.GetSelectEntryPos());
    return 0;
}

IMPL_LINK(FmSearchDialog, OnFieldSelected, ListBox*, pBox)
{
    (void) pBox; // avoid warning
    DBG_ASSERT(pBox->GetSelectEntryCount() == 1, "FmSearchDialog::OnFieldSelected : unerwartet : nicht genau ein Eintrag selektiert !");

    m_pSearchEngine->RebuildUsedFields(m_rbAllFields.IsChecked() ? -1 : (sal_Int16)m_lbField.GetSelectEntryPos());
        // calls m_pSearchEngine->InvalidatePreviousLoc too

    sal_Int32 nCurrentContext = m_lbForm.GetSelectEntryPos();
    if (nCurrentContext != LISTBOX_ENTRY_NOTFOUND)
        m_arrContextFields[nCurrentContext] = OUString(m_lbField.GetSelectEntry());
    return 0;
}

IMPL_LINK(FmSearchDialog, OnCheckBoxToggled, CheckBox*, pBox)
{
    sal_Bool bChecked = pBox->IsChecked();

    // formatter or case -> pass on to the engine
    if (pBox == &m_cbUseFormat)
        m_pSearchEngine->SetFormatterUsing(bChecked);
    else if (pBox == &m_cbCase)
        m_pSearchEngine->SetCaseSensitive(bChecked);
    // direction -> pass on and reset the checkbox-text for StartOver
    else if (pBox == &m_cbBackwards)
    {
        m_cbStartOver.SetText( OUString( CUI_RES( bChecked ? RID_STR_FROM_BOTTOM : RID_STR_FROM_TOP ) ) );
        m_pSearchEngine->SetDirection(!bChecked);
    }
    // similarity-search or regular expression
    else if ((pBox == &m_cbApprox) || (pBox == &m_cbRegular) || (pBox == &m_cbWildCard))
    {
        CheckBox* pBoxes[] = { &m_cbWildCard, &m_cbRegular, &m_cbApprox };
        for (sal_uInt32 i=0; i< SAL_N_ELEMENTS(pBoxes); ++i)
        {
            if (pBoxes[i] != pBox)
            {
                if (bChecked)
                    pBoxes[i]->Disable();
                else
                    pBoxes[i]->Enable();
            }
        }

        // pass on to the engine
        m_pSearchEngine->SetWildcard(m_cbWildCard.IsEnabled() ? m_cbWildCard.IsChecked() : sal_False);
        m_pSearchEngine->SetRegular(m_cbRegular.IsEnabled() ? m_cbRegular.IsChecked() : sal_False);
        m_pSearchEngine->SetLevenshtein(m_cbApprox.IsEnabled() ? m_cbApprox.IsChecked() : sal_False);
            // (disabled boxes have to be passed to the engine as sal_False)

        // adjust the Position-Listbox (which is not allowed during Wildcard-search)
        if (pBox == &m_cbWildCard)
        {
            if (bChecked)
            {
                m_ftPosition.Disable();
                m_lbPosition.Disable();
            }
            else
            {
                m_ftPosition.Enable();
                m_lbPosition.Enable();
            }
        }

        // and the button for similarity-search
        if (pBox == &m_cbApprox)
        {
            if (bChecked)
                m_pbApproxSettings.Enable();
            else
                m_pbApproxSettings.Disable();
        }
    }
    else if (pBox == &m_aHalfFullFormsCJK)
    {
        // forward to the search engine
        m_pSearchEngine->SetIgnoreWidthCJK( !bChecked );
    }
    else if (pBox == &m_aSoundsLikeCJK)
    {
        m_aSoundsLikeCJKSettings.Enable(bChecked);

        // two other buttons which depend on this one
        sal_Bool bEnable =  (   m_rbSearchForText.IsChecked()
                            &&  !m_aSoundsLikeCJK.IsChecked()
                            )
                         || !SvtCJKOptions().IsJapaneseFindEnabled();
        m_cbCase.Enable(bEnable);
        m_aHalfFullFormsCJK.Enable(bEnable);

        // forward to the search engine
        m_pSearchEngine->SetTransliteration( bChecked );
    }

    return 0;
}

void FmSearchDialog::InitContext(sal_Int16 nContext)
{
    FmSearchContext fmscContext;
    fmscContext.nContext = nContext;

#ifdef DBG_UTIL
    sal_uInt32 nResult =
#endif
    m_lnkContextSupplier.Call(&fmscContext);
    DBG_ASSERT(nResult > 0, "FmSearchDialog::InitContext : ContextSupplier didn't give me any controls !");

    // put the field names into the respective listbox
    m_lbField.Clear();

    if (fmscContext.sFieldDisplayNames.Len() != 0)
    {
        // use the display names if supplied
        DBG_ASSERT(comphelper::string::getTokenCount(fmscContext.sFieldDisplayNames, ';') == comphelper::string::getTokenCount(fmscContext.strUsedFields, ';'),
            "FmSearchDialog::InitContext : invalid context description supplied !");
        for (sal_Int32 i=0; i < comphelper::string::getTokenCount(fmscContext.sFieldDisplayNames, ';'); ++i)
            m_lbField.InsertEntry(comphelper::string::getToken(fmscContext.sFieldDisplayNames, i, ';'));
    }
    else
    {
        // else use the field names
        for (sal_Int32 i=0; i < comphelper::string::getTokenCount(fmscContext.strUsedFields, ';'); ++i)
            m_lbField.InsertEntry(comphelper::string::getToken(fmscContext.strUsedFields, i, ';'));
    }

    if (nContext < (sal_Int32)m_arrContextFields.size() && m_arrContextFields[nContext].Len())
    {
        m_lbField.SelectEntry(m_arrContextFields[nContext]);
    }
    else
    {
        m_lbField.SelectEntryPos(0);
        if (m_rbSingleField.IsChecked() && (m_lbField.GetEntryCount() > 1))
            m_lbField.GrabFocus();
    }

    m_pSearchEngine->SwitchToContext(fmscContext.xCursor, fmscContext.strUsedFields, fmscContext.arrFields,
        m_rbAllFields.IsChecked() ? -1 : 0);

    m_ftRecord.SetText(OUString::number(fmscContext.xCursor->getRow()));
}

IMPL_LINK( FmSearchDialog, OnContextSelection, ListBox*, pBox)
{
    InitContext(pBox->GetSelectEntryPos());
    return 0L;
}

void FmSearchDialog::EnableSearchUI(sal_Bool bEnable)
{
    // when the controls shall be disabled their paint is turned off and then turned on again after a delay
    if (!bEnable)
        EnableControlPaint(sal_False);
    else
    {
        if (m_aDelayedPaint.IsActive())
            m_aDelayedPaint.Stop();
    }
    // (the whole thing goes on below)
    // this small intricateness hopfully leads to no flickering when turning the SearchUI off
    // and on again shortly after (like it's the case during a short search process)

    if ( !bEnable )
    {
        // if one of my children has the focus, remember it
        Window* pFocusWindow = Application::GetFocusWindow( );
        if ( pFocusWindow && IsChild( pFocusWindow ) )
            m_pPreSearchFocus = pFocusWindow;
        else
            m_pPreSearchFocus = NULL;
    }

    // the search button has two functions -> adjust its text accordingly
    OUString sButtonText( bEnable ? m_sSearch : m_sCancel );
    m_pbSearchAgain.SetText( sButtonText );

    if (m_pSearchEngine->GetSearchMode() != SM_BRUTE)
    {
        m_flSearchFor.Enable        (bEnable);
        m_rbSearchForText.Enable    (bEnable);
        m_rbSearchForNull.Enable    (bEnable);
        m_rbSearchForNotNull.Enable (bEnable);
        m_flWhere.Enable            (bEnable);
        m_ftForm.Enable             (bEnable);
        m_lbForm.Enable             (bEnable);
        m_rbAllFields.Enable        (bEnable);
        m_rbSingleField.Enable      (bEnable);
        m_lbField.Enable            (bEnable && m_rbSingleField.IsChecked());
        m_flOptions.Enable          (bEnable);
        m_cbBackwards.Enable        (bEnable);
        m_cbStartOver.Enable        (bEnable);
        m_pbClose.Enable            (bEnable);
        EnableSearchForDependees    (bEnable);

        if ( !bEnable )
        {   // this means we're preparing for starting a search
            // In this case, EnableSearchForDependees disabled the search button
            // But as we're about to use it for cancelling the search, we really need to enable it, again
            m_pbSearchAgain.Enable( sal_True );
        }
    }

    if (!bEnable)
        m_aDelayedPaint.Start();
    else
        EnableControlPaint(sal_True);

    if ( bEnable )
    {   // restore focus
        if ( m_pPreSearchFocus )
        {
            m_pPreSearchFocus->GrabFocus();
            if ( WINDOW_EDIT == m_pPreSearchFocus->GetType() )
            {
                Edit* pEdit = static_cast< Edit* >( m_pPreSearchFocus );
                pEdit->SetSelection( Selection( 0, pEdit->GetText().getLength() ) );
            }
        }
        m_pPreSearchFocus = NULL;
    }

}

void FmSearchDialog::EnableSearchForDependees(sal_Bool bEnable)
{
    sal_Bool bSearchingForText = m_rbSearchForText.IsChecked();
    m_pbSearchAgain.Enable(bEnable && (!bSearchingForText || (!m_cmbSearchText.GetText().isEmpty())));

    bEnable = bEnable && bSearchingForText;

    sal_Bool bEnableRedundants = !m_aSoundsLikeCJK.IsChecked() || !SvtCJKOptions().IsJapaneseFindEnabled();

    m_cmbSearchText.Enable          (bEnable);
    m_ftPosition.Enable             (bEnable && !m_cbWildCard.IsChecked());
    m_cbWildCard.Enable             (bEnable && !m_cbRegular.IsChecked() && !m_cbApprox.IsChecked());
    m_cbRegular.Enable              (bEnable && !m_cbWildCard.IsChecked() && !m_cbApprox.IsChecked());
    m_cbApprox.Enable               (bEnable && !m_cbWildCard.IsChecked() && !m_cbRegular.IsChecked());
    m_pbApproxSettings.Enable       (bEnable && m_cbApprox.IsChecked());
    m_aHalfFullFormsCJK.Enable      (bEnable && bEnableRedundants);
    m_aSoundsLikeCJK.Enable         (bEnable);
    m_aSoundsLikeCJKSettings.Enable (bEnable && m_aSoundsLikeCJK.IsChecked());
    m_lbPosition.Enable             (bEnable && !m_cbWildCard.IsChecked());
    m_cbUseFormat.Enable            (bEnable);
    m_cbCase.Enable                 (bEnable && bEnableRedundants);
}

void FmSearchDialog::EnableControlPaint(sal_Bool bEnable)
{
    Control* pAffectedControls[] = { &m_flSearchFor, &m_rbSearchForText, &m_cmbSearchText, &m_rbSearchForNull, &m_rbSearchForNotNull,
        &m_rbSearchForText, &m_flWhere, &m_rbAllFields, &m_rbSingleField, &m_lbField, &m_flOptions, &m_ftPosition, &m_lbPosition,
        &m_cbUseFormat, &m_cbCase, &m_cbBackwards, &m_cbStartOver, &m_cbWildCard, &m_cbRegular, &m_cbApprox, &m_pbApproxSettings,
        &m_pbSearchAgain, &m_pbClose };

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

IMPL_LINK_NOARG(FmSearchDialog, OnDelayedPaint)
{
    EnableControlPaint(sal_True);
    return 0L;
}

void FmSearchDialog::OnFound(const ::com::sun::star::uno::Any& aCursorPos, sal_Int16 nFieldPos)
{
    FmFoundRecordInformation friInfo;
    friInfo.nContext = m_lbForm.GetSelectEntryPos();
    // if I don't do a search in a context, this has an invalid value - but then it doesn't matter anyway
    friInfo.aPosition = aCursorPos;
    if (m_rbAllFields.IsChecked())
        friInfo.nFieldPos = nFieldPos;
    else
        friInfo.nFieldPos = m_lbField.GetSelectEntryPos();
        // this of course implies that I have really searched in the field that is selected in the listbox,
        // which is made sure in RebuildUsedFields

    m_lnkFoundHandler.Call(&friInfo);

    m_cmbSearchText.GrabFocus();
}

IMPL_LINK(FmSearchDialog, OnSearchProgress, FmSearchProgress*, pProgress)
{
    SolarMutexGuard aGuard;
        // make this single method thread-safe (it's an overkill to block the whole application for this,
        // but we don't have another safety concept at the moment)

    switch (pProgress->aSearchState)
    {
        case FmSearchProgress::STATE_PROGRESS:
            if (pProgress->bOverflow)
            {
                OUString sHint( CUI_RES( m_cbBackwards.IsChecked() ? RID_STR_OVERFLOW_BACKWARD : RID_STR_OVERFLOW_FORWARD ) );
                m_ftHint.SetText( sHint );
                m_ftHint.Invalidate();
            }

            m_ftRecord.SetText(OUString::number(1 + pProgress->nCurrentRecord));
            m_ftRecord.Invalidate();
            break;

        case FmSearchProgress::STATE_PROGRESS_COUNTING:
            m_ftHint.SetText(CUI_RESSTR(RID_STR_SEARCH_COUNTING));
            m_ftHint.Invalidate();

            m_ftRecord.SetText(OUString::number(pProgress->nCurrentRecord));
            m_ftRecord.Invalidate();
            break;

        case FmSearchProgress::STATE_SUCCESSFULL:
            OnFound(pProgress->aBookmark, (sal_Int16)pProgress->nFieldIndex);
            EnableSearchUI(sal_True);
            break;

        case FmSearchProgress::STATE_ERROR:
        case FmSearchProgress::STATE_NOTHINGFOUND:
        {
            sal_uInt16 nErrorId = (FmSearchProgress::STATE_ERROR == pProgress->aSearchState)
                ? RID_SVXERR_SEARCH_GENERAL_ERROR
                : RID_SVXERR_SEARCH_NORECORD;
            ErrorBox(this, CUI_RES(nErrorId)).Execute();
        }
            // NO break !
        case FmSearchProgress::STATE_CANCELED:
            EnableSearchUI(sal_True);
            if (m_lnkCanceledNotFoundHdl.IsSet())
            {
                FmFoundRecordInformation friInfo;
                friInfo.nContext = m_lbForm.GetSelectEntryPos();
                // if I don't do a search in a context, this has an invalid value - but then it doesn't matter anyway
                friInfo.aPosition = pProgress->aBookmark;
                m_lnkCanceledNotFoundHdl.Call(&friInfo);
            }
            break;
    }

    m_ftRecord.SetText(OUString::number(1 + pProgress->nCurrentRecord));

    return 0L;
}

void FmSearchDialog::LoadParams()
{
    FmSearchParams aParams(m_pConfig->getParams());

    const OUString* pHistory     =                   aParams.aHistory.getConstArray();
    const OUString* pHistoryEnd  =   pHistory    +   aParams.aHistory.getLength();
    for (; pHistory != pHistoryEnd; ++pHistory)
        m_cmbSearchText.InsertEntry( *pHistory );

    // I do the settings at my UI-elements and then I simply call the respective change-handler,
    // that way the data is handed on to the SearchEngine and all dependent settings are done

    // current field
    sal_uInt16 nInitialField = m_lbField.GetEntryPos( String( aParams.sSingleSearchField ) );
    if (nInitialField == COMBOBOX_ENTRY_NOTFOUND)
        nInitialField = 0;
    m_lbField.SelectEntryPos(nInitialField);
    LINK(this, FmSearchDialog, OnFieldSelected).Call(&m_lbField);
    // all fields/single field (AFTER selcting the field because OnClickedFieldRadios expects a valid value there)
    if (aParams.bAllFields)
    {
        m_rbSingleField.Check(sal_False);
        m_rbAllFields.Check(sal_True);
        LINK(this, FmSearchDialog, OnClickedFieldRadios).Call(&m_rbAllFields);
        // OnClickedFieldRadios also calls to RebuildUsedFields
    }
    else
    {
        m_rbAllFields.Check(sal_False);
        m_rbSingleField.Check(sal_True);
        LINK(this, FmSearchDialog, OnClickedFieldRadios).Call(&m_rbSingleField);
    }

    m_lbPosition.SelectEntryPos(aParams.nPosition);
    LINK(this, FmSearchDialog, OnPositionSelected).Call(&m_lbPosition);

    // field formatting/case sensitivity/direction
    m_cbUseFormat.Check(aParams.bUseFormatter);
    m_cbCase.Check( aParams.isCaseSensitive() );
    m_cbBackwards.Check(aParams.bBackwards);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(&m_cbUseFormat);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(&m_cbCase);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(&m_cbBackwards);

    m_aHalfFullFormsCJK.Check( !aParams.isIgnoreWidthCJK( ) );  // BEWARE: this checkbox has a inverse semantics!
    m_aSoundsLikeCJK.Check( aParams.bSoundsLikeCJK );
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(&m_aHalfFullFormsCJK);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(&m_aSoundsLikeCJK);

    m_cbWildCard.Check(sal_False);
    m_cbRegular.Check(sal_False);
    m_cbApprox.Check(sal_False);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(&m_cbWildCard);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(&m_cbRegular);
    LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(&m_cbApprox);

    CheckBox* pToCheck = NULL;
    if (aParams.bWildcard)
        pToCheck = &m_cbWildCard;
    if (aParams.bRegular)
        pToCheck = &m_cbRegular;
    if (aParams.bApproxSearch)
        pToCheck = &m_cbApprox;
    if (aParams.bSoundsLikeCJK)
        pToCheck = &m_aSoundsLikeCJK;
    if (pToCheck)
    {
        pToCheck->Check(sal_True);
        LINK(this, FmSearchDialog, OnCheckBoxToggled).Call(pToCheck);
    }

    // set Levenshtein-parameters directly at the SearchEngine
    m_pSearchEngine->SetLevRelaxed(aParams.bLevRelaxed);
    m_pSearchEngine->SetLevOther(aParams.nLevOther);
    m_pSearchEngine->SetLevShorter(aParams.nLevShorter);
    m_pSearchEngine->SetLevLonger(aParams.nLevLonger);

    m_pSearchEngine->SetTransliterationFlags( aParams.getTransliterationFlags( ) );

    m_rbSearchForText.Check(sal_False);
    m_rbSearchForNull.Check(sal_False);
    m_rbSearchForNotNull.Check(sal_False);
    switch (aParams.nSearchForType)
    {
        case 1: m_rbSearchForNull.Check(sal_True); break;
        case 2: m_rbSearchForNotNull.Check(sal_True); break;
        default: m_rbSearchForText.Check(sal_True); break;
    }
    LINK(this, FmSearchDialog, OnClickedFieldRadios).Call(&m_rbSearchForText);
}

void FmSearchDialog::SaveParams() const
{
    if (!m_pConfig)
        return;

    FmSearchParams aCurrentSettings;

    aCurrentSettings.aHistory.realloc( m_cmbSearchText.GetEntryCount() );
    OUString* pHistory = aCurrentSettings.aHistory.getArray();
    for (sal_uInt16 i=0; i<m_cmbSearchText.GetEntryCount(); ++i, ++pHistory)
        *pHistory = m_cmbSearchText.GetEntry(i);

    aCurrentSettings.sSingleSearchField         = m_lbField.GetSelectEntry();
    aCurrentSettings.bAllFields                 = m_rbAllFields.IsChecked();
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

    if (m_rbSearchForNull.IsChecked())
        aCurrentSettings.nSearchForType = 1;
    else if (m_rbSearchForNotNull.IsChecked())
        aCurrentSettings.nSearchForType = 2;
    else
        aCurrentSettings.nSearchForType = 0;

    m_pConfig->setParams( aCurrentSettings );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
