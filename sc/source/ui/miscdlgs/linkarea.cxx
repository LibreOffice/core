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

#include <comphelper/string.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/waitobj.hxx>

#include "linkarea.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "rangeutl.hxx"
#include "docsh.hxx"
#include "tablink.hxx"

ScLinkedAreaDlg::ScLinkedAreaDlg(vcl::Window* pParent)
    : ModalDialog(pParent, "ExternalDataDialog", "modules/scalc/ui/externaldata.ui")
    , pSourceShell(nullptr)
    , pDocInserter(nullptr)

{
    get(m_pCbUrl, "url");
    get(m_pLbRanges, "ranges");
    m_pLbRanges->EnableMultiSelection(true);
    m_pLbRanges->SetDropDownLineCount(8);
    get(m_pBtnBrowse, "browse");
    get(m_pBtnReload, "reload");
    get(m_pNfDelay, "delay");
    get(m_pFtSeconds, "secondsft");
    get(m_pBtnOk, "ok");

    m_pCbUrl->SetSelectHdl( LINK( this, ScLinkedAreaDlg, FileHdl ) );
    m_pBtnBrowse->SetClickHdl( LINK( this, ScLinkedAreaDlg, BrowseHdl ) );
    m_pLbRanges->SetSelectHdl( LINK( this, ScLinkedAreaDlg, RangeHdl ) );
    m_pBtnReload->SetClickHdl( LINK( this, ScLinkedAreaDlg, ReloadHdl ) );
    UpdateEnable();
}

ScLinkedAreaDlg::~ScLinkedAreaDlg()
{
    disposeOnce();
}

void ScLinkedAreaDlg::dispose()
{
    // pSourceShell is deleted by aSourceRef
    m_pCbUrl.clear();
    m_pBtnBrowse.clear();
    m_pLbRanges.clear();
    m_pBtnReload.clear();
    m_pNfDelay.clear();
    m_pFtSeconds.clear();
    m_pBtnOk.clear();
    ModalDialog::dispose();
}

#define FILTERNAME_HTML  "HTML (StarCalc)"
#define FILTERNAME_QUERY "calc_HTML_WebQuery"

IMPL_LINK_NOARG(ScLinkedAreaDlg, BrowseHdl, Button*, void)
{
    if ( !pDocInserter )
        pDocInserter = new sfx2::DocumentInserter(
            OUString::createFromAscii( ScDocShell::Factory().GetShortName() ) );
    pDocInserter->StartExecuteModal( LINK( this, ScLinkedAreaDlg, DialogClosedHdl ) );
}

IMPL_LINK_NOARG(ScLinkedAreaDlg, FileHdl, ComboBox&, void)
{
    OUString aEntered = m_pCbUrl->GetURL();
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        if ( aEntered.equals(pMed->GetName()) )
        {
            //  already loaded - nothing to do
            return;
        }
    }

    OUString aFilter;
    OUString aOptions;
    //  get filter name by looking at the file content (bWithContent = true)
    // Break operation if any error occurred inside.
    if (!ScDocumentLoader::GetFilterName( aEntered, aFilter, aOptions, true, true ))
        return;

    // #i53241# replace HTML filter with DataQuery filter
    if (aFilter == FILTERNAME_HTML)
        aFilter = FILTERNAME_QUERY;

    LoadDocument( aEntered, aFilter, aOptions );

    UpdateSourceRanges();
    UpdateEnable();
}

void ScLinkedAreaDlg::LoadDocument( const OUString& rFile, const OUString& rFilter, const OUString& rOptions )
{
    if ( pSourceShell )
    {
        //  unload old document
        pSourceShell->DoClose();
        pSourceShell = nullptr;
        aSourceRef.clear();
    }

    if ( !rFile.isEmpty() )
    {
        WaitObject aWait( this );

        OUString aNewFilter = rFilter;
        OUString aNewOptions = rOptions;

        SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, rFile );

        ScDocumentLoader aLoader( rFile, aNewFilter, aNewOptions, 0, true );    // with interaction
        pSourceShell = aLoader.GetDocShell();
        if ( pSourceShell )
        {
            sal_uLong nErr = pSourceShell->GetErrorCode();
            if (nErr)
                ErrorHandler::HandleError( nErr );      // including warnings

            aSourceRef = pSourceShell;
            aLoader.ReleaseDocRef();    // don't call DoClose in DocLoader dtor
        }
    }
}

void ScLinkedAreaDlg::InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh )
{
    LoadDocument( rFile, rFilter, rOptions );
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        m_pCbUrl->SetText( pMed->GetName() );
    }
    else
        m_pCbUrl->SetText( EMPTY_OUSTRING );

    UpdateSourceRanges();

    sal_Int32 nRangeCount = comphelper::string::getTokenCount(rSource, ';');
    for ( sal_Int32 i=0; i<nRangeCount; i++ )
    {
        OUString aRange = rSource.getToken(i,';');
        m_pLbRanges->SelectEntry( aRange );
    }

    bool bDoRefresh = (nRefresh != 0);
    m_pBtnReload->Check( bDoRefresh );
    if (bDoRefresh)
        m_pNfDelay->SetValue( nRefresh );

    UpdateEnable();
}

IMPL_LINK_NOARG(ScLinkedAreaDlg, RangeHdl, ListBox&, void)
{
    UpdateEnable();
}

IMPL_LINK_NOARG(ScLinkedAreaDlg, ReloadHdl, Button*, void)
{
    UpdateEnable();
}

IMPL_LINK( ScLinkedAreaDlg, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    if ( _pFileDlg->GetError() != ERRCODE_NONE )
        return;

    SfxMedium* pMed = pDocInserter->CreateMedium();
    if ( pMed )
    {
        WaitObject aWait( this );

        // replace HTML filter with DataQuery filter
        const OUString aHTMLFilterName( FILTERNAME_HTML );
        const OUString aWebQFilterName( FILTERNAME_QUERY );

        std::shared_ptr<const SfxFilter> pFilter = pMed->GetFilter();
        if (pFilter && aHTMLFilterName.equals(pFilter->GetFilterName()))
        {
            std::shared_ptr<const SfxFilter> pNewFilter =
                ScDocShell::Factory().GetFilterContainer()->GetFilter4FilterName( aWebQFilterName );
            if( pNewFilter )
                pMed->SetFilter( pNewFilter );
        }

        //  ERRCTX_SFX_OPENDOC -> "Error loading document"
        SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

        if (pSourceShell)
            pSourceShell->DoClose();        // deleted when assigning aSourceRef

        pMed->UseInteractionHandler( true );    // to enable the filter options dialog

        pSourceShell = new ScDocShell;
        aSourceRef = pSourceShell;
        pSourceShell->DoLoad( pMed );

        sal_uLong nErr = pSourceShell->GetErrorCode();
        if (nErr)
            ErrorHandler::HandleError( nErr );              // including warnings

        if ( !pSourceShell->GetError() )                    // only errors
        {
            m_pCbUrl->SetText( pMed->GetName() );
        }
        else
        {
            pSourceShell->DoClose();
            pSourceShell = nullptr;
            aSourceRef.clear();

            m_pCbUrl->SetText( EMPTY_OUSTRING );
        }
    }

    UpdateSourceRanges();
    UpdateEnable();
}

#undef FILTERNAME_HTML
#undef FILTERNAME_QUERY

void ScLinkedAreaDlg::UpdateSourceRanges()
{
    m_pLbRanges->SetUpdateMode(false);

    m_pLbRanges->Clear();
    if ( pSourceShell )
    {
        ScAreaNameIterator aIter( &pSourceShell->GetDocument() );
        ScRange aDummy;
        OUString aName;
        while ( aIter.Next( aName, aDummy ) )
            m_pLbRanges->InsertEntry( aName );
    }

    m_pLbRanges->SetUpdateMode(true);

    if ( m_pLbRanges->GetEntryCount() == 1 )
        m_pLbRanges->SelectEntryPos(0);
}

void ScLinkedAreaDlg::UpdateEnable()
{
    bool bEnable = ( pSourceShell && m_pLbRanges->GetSelectEntryCount() );
    m_pBtnOk->Enable( bEnable );

    bool bReload = m_pBtnReload->IsChecked();
    m_pNfDelay->Enable( bReload );
    m_pFtSeconds->Enable( bReload );
}

OUString ScLinkedAreaDlg::GetURL()
{
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        return pMed->GetName();
    }
    return EMPTY_OUSTRING;
}

OUString ScLinkedAreaDlg::GetFilter()
{
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        return pMed->GetFilter()->GetFilterName();
    }
    return OUString();
}

OUString ScLinkedAreaDlg::GetOptions()
{
    if (pSourceShell)
    {
        SfxMedium* pMed = pSourceShell->GetMedium();
        return ScDocumentLoader::GetOptions( *pMed );
    }
    return OUString();
}

OUString ScLinkedAreaDlg::GetSource()
{
    OUStringBuffer aBuf;
    const sal_Int32 nCount = m_pLbRanges->GetSelectEntryCount();
    for (sal_Int32 i=0; i<nCount; ++i)
    {
        if (i > 0)
            aBuf.append(';');
        aBuf.append(m_pLbRanges->GetSelectEntry(i));
    }
    return aBuf.makeStringAndClear();
}

sal_uLong ScLinkedAreaDlg::GetRefresh()
{
    if ( m_pBtnReload->IsChecked() )
        return sal::static_int_cast<sal_uLong>( m_pNfDelay->GetValue() );
    else
        return 0;   // disabled
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
