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
#include <address.hxx>
#include <docsh.hxx>

namespace sc {

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

    m_pCbUrl->SetSelectHdl( LINK( this, DataStreamDlg, UpdateHdl ) );
    m_pRBAddressValue->SetClickHdl( LINK( this, DataStreamDlg, UpdateHdl ) );
    m_pRBAddressValue->Enable(false);
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
        m_pVclFrameLimit->Disable();
        m_pVclFrameMove->Disable();
        m_pEdRange->Disable();
    }
    else
    {
        m_pVclFrameLimit->Enable();
        m_pVclFrameMove->Enable();
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

ScRange DataStreamDlg::GetStartRange()
{
    OUString aStr = m_pEdRange->GetText();
    ScDocument* pDoc = mpDocShell->GetDocument();
    ScRange aRange;
    sal_uInt16 nRes = aRange.Parse(aStr, pDoc);
    if ((nRes & SCA_VALID) != SCA_VALID || !aRange.IsValid())
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

void DataStreamDlg::Init(
    const OUString& rURL, const ScRange& rRange, const sal_Int32 nLimit,
    DataStream::MoveType eMove, const sal_uInt32 nSettings)
{
    m_pEdLimit->SetText(OUString::number(nLimit));
    m_pCbUrl->SetText(rURL);
    if (nSettings & DataStream::SCRIPT_STREAM)
        m_pRBScriptData->Check();
    if (!(nSettings & DataStream::VALUES_IN_LINE))
        m_pRBAddressValue->Check();

    OUString aStr = rRange.Format(SCA_VALID);
    m_pEdRange->SetText(aStr);

    switch (eMove)
    {
        case DataStream::MOVE_DOWN:
            m_pRBDataDown->Check();
        break;
        case DataStream::NO_MOVE:
            m_pRBNoMove->Check();
        break;
        case DataStream::RANGE_DOWN:
            m_pRBRangeDown->Check();
        break;
        case DataStream::MOVE_UP:
        default:
            ;
    }

    UpdateEnable();
}

void DataStreamDlg::StartStream(DataStream *pStream)
{
    ScRange aStartRange = GetStartRange();
    if (!aStartRange.IsValid())
        // Don't start the stream without a valid range.
        return;

    sal_Int32 nLimit = 0;
    if (m_pRBMaxLimit->IsChecked())
        nLimit = m_pEdLimit->GetText().toInt32();
    OUString rURL = m_pCbUrl->GetText();
    sal_uInt32 nSettings = 0;
    if (m_pRBScriptData->IsChecked())
       nSettings |= DataStream::SCRIPT_STREAM;
    if (m_pRBValuesInLine->IsChecked())
       nSettings |= DataStream::VALUES_IN_LINE;

    DataStream::MoveType eMove =
        m_pRBNoMove->IsChecked() ? DataStream::NO_MOVE : m_pRBRangeDown->IsChecked()
            ? DataStream::RANGE_DOWN : DataStream::MOVE_DOWN;

    if (pStream)
    {
        pStream->Decode(rURL, aStartRange, nLimit, eMove, nSettings);
        return;
    }

    pStream = DataStream::Set(mpDocShell, rURL, aStartRange, nLimit, eMove, nSettings);
    DataStream::MakeToolbarVisible();
    pStream->StartImport();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
