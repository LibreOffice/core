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

#undef SC_DLLIMPLEMENTATION

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/inettbc.hxx>
#include <svtools/sfxecode.hxx>

#include <linkarea.hxx>
#include <rangeutl.hxx>
#include <docsh.hxx>
#include <tablink.hxx>

ScLinkedAreaDlg::ScLinkedAreaDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/scalc/ui/externaldata.ui", "ExternalDataDialog")
    , m_pSourceShell(nullptr)
    , m_xCbUrl(new URLBox(m_xBuilder->weld_combo_box("url")))
    , m_xBtnBrowse(m_xBuilder->weld_button("browse"))
    , m_xLbRanges(m_xBuilder->weld_tree_view("ranges"))
    , m_xBtnReload(m_xBuilder->weld_check_button("reload"))
    , m_xNfDelay(m_xBuilder->weld_spin_button("delay"))
    , m_xFtSeconds(m_xBuilder->weld_label("secondsft"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
{
    m_xLbRanges->set_selection_mode(SelectionMode::Multiple);

    m_xCbUrl->connect_entry_activate(LINK(this, ScLinkedAreaDlg, FileHdl));
    m_xBtnBrowse->connect_clicked(LINK( this, ScLinkedAreaDlg, BrowseHdl));
    m_xLbRanges->connect_changed(LINK( this, ScLinkedAreaDlg, RangeHdl));
    m_xLbRanges->set_size_request(m_xLbRanges->get_approximate_digit_width() * 54,
                                  m_xLbRanges->get_height_rows(5));
    m_xBtnReload->connect_clicked(LINK( this, ScLinkedAreaDlg, ReloadHdl));
    UpdateEnable();
}

ScLinkedAreaDlg::~ScLinkedAreaDlg()
{
}

#define FILTERNAME_HTML  "HTML (StarCalc)"
#define FILTERNAME_QUERY "calc_HTML_WebQuery"

IMPL_LINK_NOARG(ScLinkedAreaDlg, BrowseHdl, weld::Button&, void)
{
    m_xDocInserter.reset( new sfx2::DocumentInserter(m_xDialog.get(), ScDocShell::Factory().GetFactoryName()) );
    m_xDocInserter->StartExecuteModal( LINK( this, ScLinkedAreaDlg, DialogClosedHdl ) );
}

IMPL_LINK_NOARG(ScLinkedAreaDlg, FileHdl, weld::ComboBox&, bool)
{
    OUString aEntered = m_xCbUrl->GetURL();
    if (m_pSourceShell)
    {
        SfxMedium* pMed = m_pSourceShell->GetMedium();
        if ( aEntered == pMed->GetName() )
        {
            //  already loaded - nothing to do
            return true;
        }
    }

    OUString aFilter;
    OUString aOptions;
    //  get filter name by looking at the file content (bWithContent = true)
    // Break operation if any error occurred inside.
    if (!ScDocumentLoader::GetFilterName( aEntered, aFilter, aOptions, true, false ))
        return true;

    // #i53241# replace HTML filter with DataQuery filter
    if (aFilter == FILTERNAME_HTML)
        aFilter = FILTERNAME_QUERY;

    LoadDocument( aEntered, aFilter, aOptions );

    UpdateSourceRanges();
    UpdateEnable();

    return true;
}

void ScLinkedAreaDlg::LoadDocument( const OUString& rFile, const OUString& rFilter, const OUString& rOptions )
{
    if (m_pSourceShell)
    {
        //  unload old document
        m_pSourceShell->DoClose();
        m_pSourceShell = nullptr;
        aSourceRef.clear();
    }

    if ( !rFile.isEmpty() )
    {
        weld::WaitObject aWait(m_xDialog.get());

        OUString aNewFilter = rFilter;
        OUString aNewOptions = rOptions;

        SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, rFile );

        ScDocumentLoader aLoader( rFile, aNewFilter, aNewOptions, 0, m_xDialog.get() );    // with interaction
        m_pSourceShell = aLoader.GetDocShell();
        if (m_pSourceShell)
        {
            ErrCode nErr = m_pSourceShell->GetErrorCode();
            if (nErr)
                ErrorHandler::HandleError( nErr );      // including warnings

            aSourceRef = m_pSourceShell;
            aLoader.ReleaseDocRef();    // don't call DoClose in DocLoader dtor
        }
    }
}

void ScLinkedAreaDlg::InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh )
{
    LoadDocument( rFile, rFilter, rOptions );
    if (m_pSourceShell)
    {
        SfxMedium* pMed = m_pSourceShell->GetMedium();
        m_xCbUrl->set_entry_text(pMed->GetName());
    }
    else
        m_xCbUrl->set_entry_text(EMPTY_OUSTRING);

    UpdateSourceRanges();

    if (!rSource.isEmpty())
    {
        sal_Int32 nIdx {0};
        do
        {
            m_xLbRanges->select_text(rSource.getToken(0, ';', nIdx));
        }
        while (nIdx>0);
    }

    bool bDoRefresh = (nRefresh != 0);
    m_xBtnReload->set_active(bDoRefresh);
    if (bDoRefresh)
        m_xNfDelay->set_value(nRefresh);

    UpdateEnable();
}

IMPL_LINK_NOARG(ScLinkedAreaDlg, RangeHdl, weld::TreeView&, void)
{
    UpdateEnable();
}

IMPL_LINK_NOARG(ScLinkedAreaDlg, ReloadHdl, weld::Button&, void)
{
    UpdateEnable();
}

IMPL_LINK( ScLinkedAreaDlg, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    if ( _pFileDlg->GetError() != ERRCODE_NONE )
        return;

    std::unique_ptr<SfxMedium> pMed = m_xDocInserter->CreateMedium();
    if ( pMed )
    {
        weld::WaitObject aWait(m_xDialog.get());

        // replace HTML filter with DataQuery filter
        const OUString aHTMLFilterName( FILTERNAME_HTML );
        const OUString aWebQFilterName( FILTERNAME_QUERY );

        std::shared_ptr<const SfxFilter> pFilter = pMed->GetFilter();
        if (pFilter && aHTMLFilterName == pFilter->GetFilterName())
        {
            std::shared_ptr<const SfxFilter> pNewFilter =
                ScDocShell::Factory().GetFilterContainer()->GetFilter4FilterName( aWebQFilterName );
            if( pNewFilter )
                pMed->SetFilter( pNewFilter );
        }

        //  ERRCTX_SFX_OPENDOC -> "Error loading document"
        SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

        if (m_pSourceShell)
            m_pSourceShell->DoClose();        // deleted when assigning aSourceRef

        pMed->UseInteractionHandler( true );    // to enable the filter options dialog

        m_pSourceShell = new ScDocShell;
        aSourceRef = m_pSourceShell;
        m_pSourceShell->DoLoad( pMed.get() );

        ErrCode nErr = m_pSourceShell->GetErrorCode();
        if (nErr)
            ErrorHandler::HandleError( nErr );              // including warnings

        if (!m_pSourceShell->GetError())                    // only errors
        {
            m_xCbUrl->set_entry_text(pMed->GetName());
        }
        else
        {
            m_pSourceShell->DoClose();
            m_pSourceShell = nullptr;
            aSourceRef.clear();

            m_xCbUrl->set_entry_text(EMPTY_OUSTRING);
        }
        pMed.release(); // DoLoad takes ownership
    }

    UpdateSourceRanges();
    UpdateEnable();
}

#undef FILTERNAME_HTML
#undef FILTERNAME_QUERY

void ScLinkedAreaDlg::UpdateSourceRanges()
{
    m_xLbRanges->freeze();

    m_xLbRanges->clear();
    if ( m_pSourceShell )
    {
        std::shared_ptr<const SfxFilter> pFilter = m_pSourceShell->GetMedium()->GetFilter();
        if (pFilter && pFilter->GetFilterName() == SC_TEXT_CSV_FILTER_NAME)
        {
            // Insert dummy All range to have something selectable.
            m_xLbRanges->append_text("CSV_all");
        }

        ScAreaNameIterator aIter(&m_pSourceShell->GetDocument());
        ScRange aDummy;
        OUString aName;
        while ( aIter.Next( aName, aDummy ) )
            m_xLbRanges->append_text(aName);
    }

    m_xLbRanges->thaw();

    if (m_xLbRanges->n_children() == 1)
        m_xLbRanges->select(0);
}

void ScLinkedAreaDlg::UpdateEnable()
{
    bool bEnable = ( m_pSourceShell && m_xLbRanges->count_selected_rows() );
    m_xBtnOk->set_sensitive(bEnable);

    bool bReload = m_xBtnReload->get_active();
    m_xNfDelay->set_sensitive(bReload);
    m_xFtSeconds->set_sensitive(bReload);
}

OUString ScLinkedAreaDlg::GetURL()
{
    if (m_pSourceShell)
    {
        SfxMedium* pMed = m_pSourceShell->GetMedium();
        return pMed->GetName();
    }
    return EMPTY_OUSTRING;
}

OUString ScLinkedAreaDlg::GetFilter()
{
    if (m_pSourceShell)
    {
        SfxMedium* pMed = m_pSourceShell->GetMedium();
        return pMed->GetFilter()->GetFilterName();
    }
    return OUString();
}

OUString ScLinkedAreaDlg::GetOptions()
{
    if (m_pSourceShell)
    {
        SfxMedium* pMed = m_pSourceShell->GetMedium();
        return ScDocumentLoader::GetOptions( *pMed );
    }
    return OUString();
}

OUString ScLinkedAreaDlg::GetSource()
{
    OUStringBuffer aBuf;
    std::vector<OUString> aSelection = m_xLbRanges->get_selected_rows_text();
    for (size_t i = 0; i < aSelection.size(); ++i)
    {
        if (i > 0)
            aBuf.append(';');
        aBuf.append(aSelection[i]);
    }
    return aBuf.makeStringAndClear();
}

sal_uLong ScLinkedAreaDlg::GetRefresh()
{
    if (m_xBtnReload->get_active())
        return sal::static_int_cast<sal_uLong>(m_xNfDelay->get_value());
    else
        return 0;   // disabled
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
