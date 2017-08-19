/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataproviderdlg.hxx>

#include <sfx2/filedlghelper.hxx>
#include <svtools/inettbc.hxx>
#include <vcl/layout.hxx>
#include <address.hxx>
#include <docsh.hxx>
#include <dbdata.hxx>
#include "datamapper.hxx"

namespace sc {

DataProviderDlg::DataProviderDlg(ScDocShell *pDocShell, vcl::Window* pParent)
    : ModalDialog(pParent, "DataProviderDialog", "modules/scalc/ui/dataprovider.ui")
    , mpDocShell(pDocShell)
{
    get(m_pCbUrl, "url");
    get(m_pBtnBrowse, "browse");
    get(m_pBtnOk, "ok");
    get(m_pCBData, "combobox_db");
    get(m_pCBProvider, "combobox_provider");
    get(m_pEdID, "edit_id");

    m_pCbUrl->SetSelectHdl( LINK( this, DataProviderDlg, UpdateComboBoxHdl ) );
    m_pCbUrl->SetModifyHdl(LINK(this, DataProviderDlg, EditHdl));
    m_pBtnBrowse->SetClickHdl( LINK( this, DataProviderDlg, BrowseHdl ) );
    m_pCBData->SetSelectHdl(LINK(this, DataProviderDlg, SelectHdl));
    Init();
    m_pCBData->Resize();
    UpdateEnable();
}

DataProviderDlg::~DataProviderDlg()
{
    disposeOnce();
}

void DataProviderDlg::dispose()
{
    m_pCbUrl.clear();
    m_pBtnBrowse.clear();
    m_pBtnOk.clear();
    m_pCBData.clear();
    m_pCBProvider.clear();
    m_pEdID.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(DataProviderDlg, BrowseHdl, Button*, void)
{
    sfx2::FileDialogHelper aFileDialog(0);
    if ( aFileDialog.Execute() != ERRCODE_NONE )
        return;

    m_pCbUrl->SetText( aFileDialog.GetPath() );
    UpdateEnable();
}

IMPL_LINK_NOARG(DataProviderDlg, UpdateClickHdl, Button*, void)
{
    UpdateEnable();
}

IMPL_LINK_NOARG(DataProviderDlg, UpdateComboBoxHdl, ComboBox&, void)
{
    UpdateEnable();
}

IMPL_LINK_NOARG(DataProviderDlg, EditHdl, Edit&, void)
{
    UpdateEnable();
}

IMPL_LINK_NOARG(DataProviderDlg, SelectHdl, ListBox&, void)
{
    UpdateEnable();
}

void DataProviderDlg::UpdateEnable()
{
    bool bEmptyEntry = m_pCbUrl->GetURL().isEmpty() ||
            m_pCBData->GetSelectEntry().isEmpty() ||
            m_pCBProvider->GetSelectEntry().isEmpty();
    m_pBtnOk->Enable(!bEmptyEntry);
    setOptimalLayoutSize();
}

void DataProviderDlg::Init()
{
    ScDocument& rDoc = mpDocShell->GetDocument();
    ScDBCollection::NamedDBs& rNamedDBs = rDoc.GetDBCollection()->getNamedDBs();
    for (auto& itr : rNamedDBs)
    {
        OUString aName = itr->GetName();
        m_pCBData->InsertEntry(aName);
    }

    std::vector<OUString> aDataProviders = sc::DataProviderFactory::getDataProviders();
    for (auto& itr : aDataProviders)
    {
        m_pCBProvider->InsertEntry(itr);
    }
}

void DataProviderDlg::StartImport()
{
    OUString aURL = m_pCbUrl->GetText();
    if (aURL.isEmpty())
        return;

    OUString maDBDataName = m_pCBData->GetSelectEntry();
    if (maDBDataName.isEmpty())
        return;

    OUString aProvider = m_pCBProvider->GetSelectEntry();
    if (aProvider.isEmpty())
        return;

    ScDocument& rDoc = mpDocShell->GetDocument();
    ScDBData* pDBData = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(maDBDataName));
    if (!pDBData)
        return;

    OUString aID = m_pEdID->GetText();

    ExternalDataSource aDataSource(aURL, aProvider, &mpDocShell->GetDocument());
    aDataSource.setID(aID);
    aDataSource.setDBData(pDBData);
    mpDocShell->GetDocument().GetExternalDataMapper().insertDataSource(aDataSource);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
