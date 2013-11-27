/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <datastreamdlg.hxx>

#include <sfx2/filedlghelper.hxx>
#include <svtools/inettbc.hxx>
#include <vcl/layout.hxx>
#include <datastream.hxx>

DataStreamDlg::DataStreamDlg(ScDocShell *pDocShell, Window* pParent)
    : ModalDialog(pParent, "DataStreamDialog", "modules/scalc/ui/datastreams.ui")
    , mpDocShell(pDocShell)
{
    get(m_pCbUrl, "url");
    get(m_pBtnBrowse, "browse");
    get(m_pRBScriptData, "scriptdata");
    get(m_pRBValuesInLine, "valuesinline");
    get(m_pRBAddressValue, "addressvalue");
    get(m_pRBDataDown, "datadown");
    get(m_pRBRangeDown, "rangedown");
    get(m_pRBNoMove, "nomove");
    get(m_pRBMaxLimit, "maxlimit");
    get(m_pEdRange, "range");
    get(m_pEdLimit, "limit");
    get(m_pBtnOk, "ok");
    get(m_pVclFrameLimit, "framelimit");
    get(m_pVclFrameMove, "framemove");
    get(m_pVclFrameRange, "framerange");

    m_pCbUrl->SetSelectHdl( LINK( this, DataStreamDlg, UpdateHdl ) );
    m_pRBAddressValue->SetClickHdl( LINK( this, DataStreamDlg, UpdateHdl ) );
    m_pRBValuesInLine->SetClickHdl( LINK( this, DataStreamDlg, UpdateHdl ) );
    m_pEdRange->SetModifyHdl( LINK( this, DataStreamDlg, UpdateHdl ) );
    m_pBtnBrowse->SetClickHdl( LINK( this, DataStreamDlg, BrowseHdl ) );
    UpdateEnable();
}

IMPL_LINK_NOARG(DataStreamDlg, BrowseHdl)
{
    sfx2::FileDialogHelper aFileDialog(0, 0);
    if ( aFileDialog.Execute() != ERRCODE_NONE )
        return 0;

    m_pCbUrl->SetText( aFileDialog.GetPath() );
    UpdateEnable();
    return 0;
}

IMPL_LINK_NOARG(DataStreamDlg, UpdateHdl)
{
    UpdateEnable();
    return 0;
}

void DataStreamDlg::UpdateEnable()
{
    bool bOk = !m_pCbUrl->GetURL().isEmpty();
    if (m_pRBAddressValue->IsChecked())
    {
        m_pRBNoMove->Check();
        m_pVclFrameLimit->Hide();
        m_pVclFrameMove->Hide();
        m_pVclFrameRange->Hide();
        m_pEdRange->SetText("");
    }
    else
    {
        m_pVclFrameLimit->Show();
        m_pVclFrameMove->Show();
        m_pVclFrameRange->Show();
        bOk = bOk && !m_pEdRange->GetText().isEmpty();
    }
    m_pBtnOk->Enable(bOk);
    setOptimalLayoutSize();
}

void DataStreamDlg::Init(const OUString& rURL, const OUString& rRange, const sal_Int32 nLimit,
        const OUString& rMove, const sal_uInt32 nSettings)
{
    m_pEdLimit->SetText(OUString::number(nLimit));
    m_pCbUrl->SetText(rURL);
    if (nSettings & DataStream::SCRIPT_STREAM)
        m_pRBScriptData->Check();
    if (!(nSettings & DataStream::VALUES_IN_LINE))
        m_pRBAddressValue->Check();
    m_pEdRange->SetText(rRange);
    if (rMove == "NO_MOVE")
        m_pRBNoMove->Check();
    else if (rMove == "RANGE_DOWN")
        m_pRBRangeDown->Check();
    else if (rMove == "MOVE_DOWN")
        m_pRBDataDown->Check();
    UpdateEnable();
}

void DataStreamDlg::StartStream(DataStream *pStream)
{
    sal_Int32 nLimit = 0;
    if (m_pRBMaxLimit->IsChecked())
        nLimit = m_pEdLimit->GetText().toInt32();
    OUString rURL = m_pCbUrl->GetText();
    sal_uInt32 nSettings = 0;
    if (m_pRBScriptData->IsChecked())
       nSettings |= DataStream::SCRIPT_STREAM;
    if (m_pRBValuesInLine->IsChecked())
       nSettings |= DataStream::VALUES_IN_LINE;
    if (pStream)
    {
        pStream->Decode(rURL, m_pEdRange->GetText(), nLimit,
                m_pRBNoMove->IsChecked() ? OUString("NO_MOVE") : m_pRBRangeDown->IsChecked()
                    ? OUString("RANGE_DOWN") : OUString("MOVE_DOWN"),
                nSettings);
        return;
    }
    pStream = DataStream::Set( mpDocShell,
            rURL,
            m_pEdRange->GetText(),
            nLimit,
            m_pRBNoMove->IsChecked() ? OUString("NO_MOVE") : m_pRBRangeDown->IsChecked()
                ? OUString("RANGE_DOWN") : OUString("MOVE_DOWN")
            , nSettings
            );
    DataStream::MakeToolbarVisible();
    pStream->StartImport();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
