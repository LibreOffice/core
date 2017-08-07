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
    get(m_pRBAddressValue, "addressvalue");
    get(m_pCBRefreshOnEmpty, "refresh_ui");
    //get(m_pRBRangeDown, "rangedown");
    //get(m_pRBNoMove, "nomove");
    get(m_pRBMaxLimit, "maxlimit");
    get(m_pRBUnlimited, "unlimited");
    get(m_pEdRange, "range");
    get(m_pEdLimit, "limit");
    get(m_pBtnOk, "ok");
    get(m_pVclFrameLimit, "framelimit");
    //get(m_pVclFrameMove, "framemove");

    m_pCbUrl->SetSelectHdl( LINK( this, DataProviderDlg, UpdateComboBoxHdl ) );
    m_pRBAddressValue->SetClickHdl( LINK( this, DataProviderDlg, UpdateClickHdl ) );
    m_pRBAddressValue->Enable(false);
    //m_pRBScriptData->Enable(false);
    //m_pRBDirectData->Hide();
    //m_pRBScriptData->Hide();
    //m_pRBNoMove->Hide();
    //m_pRBValuesInLine->SetClickHdl( LINK( this, DataProviderDlg, UpdateClickHdl ) );
    m_pEdRange->SetModifyHdl( LINK( this, DataProviderDlg, UpdateHdl ) );
    m_pBtnBrowse->SetClickHdl( LINK( this, DataProviderDlg, BrowseHdl ) );
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
    //m_pRBDirectData.clear();
    //m_pRBScriptData.clear();
    //m_pRBValuesInLine.clear();
    m_pRBAddressValue.clear();
    m_pCBRefreshOnEmpty.clear();
    //m_pRBDataDown.clear();
    //m_pRBRangeDown.clear();
    //m_pRBNoMove.clear();
    m_pRBMaxLimit.clear();
    m_pRBUnlimited.clear();
    m_pEdRange.clear();
    m_pEdLimit.clear();
    m_pBtnOk.clear();
    m_pVclFrameLimit.clear();
    //m_pVclFrameMove.clear();
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
IMPL_LINK_NOARG(DataProviderDlg, UpdateHdl, Edit&, void)
{
    UpdateEnable();
}

void DataProviderDlg::UpdateEnable()
{
    bool bOk = !m_pCbUrl->GetURL().isEmpty();
    if (m_pRBAddressValue->IsChecked())
    {
        m_pVclFrameLimit->Disable();
        //m_pVclFrameMove->Disable();
        m_pEdRange->Disable();
    }
    else
    {
        m_pVclFrameLimit->Enable();
        //m_pVclFrameMove->Enable();
        m_pEdRange->Enable();
        if (bOk)
        {
            // Check the given range to make sure it's valid.
            ScRange aTest = GetStartRange();
            if (!aTest.IsValid())
                bOk = false;
        }
    }
    m_pBtnOk->Enable(bOk);
    setOptimalLayoutSize();
}

ScRange DataProviderDlg::GetStartRange()
{
    OUString aStr = m_pEdRange->GetText();
    ScDocument& rDoc = mpDocShell->GetDocument();
    ScRange aRange;
    ScRefFlags nRes = aRange.Parse(aStr, &rDoc, rDoc.GetAddressConvention());
    if ( ((nRes & ScRefFlags::VALID) == ScRefFlags::ZERO) || !aRange.IsValid())
    {
        // Invalid range.
        aRange.SetInvalid();
        return aRange;
    }

    // Make sure it's only one row tall.
    if (aRange.aStart.Row() != aRange.aEnd.Row())
        aRange.SetInvalid();

    return aRange;
}

void DataProviderDlg::Init()
{
    // TODO : Get the user specified Url and Range
    (void)this;
}

void DataProviderDlg::StartImport()
{
    ScRange aRange = GetStartRange();
    if (!aRange.IsValid())
        // Don't start the stream without a valid range.
        return;

    OUString aURL;
    // TODO : replace those strings with something that is obtained from user
    ExternalDataSource aDataSource(aURL, "org.libreoffice.calc.csv");
    //aDataSource.setDBData(pDBData);
    mpDocShell->GetDocument().GetExternalDataMapper().insertDataSource(aDataSource);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
