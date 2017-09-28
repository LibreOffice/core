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

DataStreamDlg::DataStreamDlg(ScDocShell *pDocShell, vcl::Window* pParent)
    : ModalDialog(pParent, "DataStreamDialog", "modules/scalc/ui/datastreams.ui")
    , mpDocShell(pDocShell)
{
    get(m_pCbUrl, "url");
    get(m_pBtnBrowse, "browse");
    get(m_pRBValuesInLine, "valuesinline");
    get(m_pRBAddressValue, "addressvalue");
    get(m_pCBRefreshOnEmpty, "refresh_ui");
    get(m_pRBDataDown, "datadown");
    get(m_pRBRangeDown, "rangedown");
    get(m_pRBNoMove, "nomove");
    get(m_pRBMaxLimit, "maxlimit");
    get(m_pRBUnlimited, "unlimited");
    get(m_pEdRange, "range");
    get(m_pEdLimit, "limit");
    get(m_pBtnOk, "ok");
    get(m_pVclFrameLimit, "framelimit");
    get(m_pVclFrameMove, "framemove");

    m_pCbUrl->SetSelectHdl( LINK( this, DataStreamDlg, UpdateComboBoxHdl ) );
    m_pRBAddressValue->SetClickHdl( LINK( this, DataStreamDlg, UpdateClickHdl ) );
    m_pRBAddressValue->Enable(false);
    m_pRBNoMove->Hide();
    m_pRBValuesInLine->SetClickHdl( LINK( this, DataStreamDlg, UpdateClickHdl ) );
    m_pEdRange->SetModifyHdl( LINK( this, DataStreamDlg, UpdateHdl ) );
    m_pBtnBrowse->SetClickHdl( LINK( this, DataStreamDlg, BrowseHdl ) );
    UpdateEnable();
}

DataStreamDlg::~DataStreamDlg()
{
    disposeOnce();
}

void DataStreamDlg::dispose()
{
    m_pCbUrl.clear();
    m_pBtnBrowse.clear();
    m_pRBValuesInLine.clear();
    m_pRBAddressValue.clear();
    m_pCBRefreshOnEmpty.clear();
    m_pRBDataDown.clear();
    m_pRBRangeDown.clear();
    m_pRBNoMove.clear();
    m_pRBMaxLimit.clear();
    m_pRBUnlimited.clear();
    m_pEdRange.clear();
    m_pEdLimit.clear();
    m_pBtnOk.clear();
    m_pVclFrameLimit.clear();
    m_pVclFrameMove.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(DataStreamDlg, BrowseHdl, Button*, void)
{
    sfx2::FileDialogHelper aFileDialog(0, FileDialogFlags::NONE, this);
    if ( aFileDialog.Execute() != ERRCODE_NONE )
        return;

    m_pCbUrl->SetText( aFileDialog.GetPath() );
    UpdateEnable();
}

IMPL_LINK_NOARG(DataStreamDlg, UpdateClickHdl, Button*, void)
{
    UpdateEnable();
}
IMPL_LINK_NOARG(DataStreamDlg, UpdateComboBoxHdl, ComboBox&, void)
{
    UpdateEnable();
}
IMPL_LINK_NOARG(DataStreamDlg, UpdateHdl, Edit&, void)
{
    UpdateEnable();
}

void DataStreamDlg::UpdateEnable()
{
    bool bOk = !m_pCbUrl->GetURL().isEmpty();
    if (m_pRBAddressValue->IsChecked())
    {
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

void DataStreamDlg::Init( const DataStream& rStrm )
{
    m_pCbUrl->SetText(rStrm.GetURL());
    ScDocument& rDoc = mpDocShell->GetDocument();

    ScRange aRange = rStrm.GetRange();
    ScRange aTopRange = aRange;
    aTopRange.aEnd.SetRow(aTopRange.aStart.Row());
    OUString aStr = aTopRange.Format(ScRefFlags::RANGE_ABS_3D, &rDoc, rDoc.GetAddressConvention());
    m_pEdRange->SetText(aStr);
    SCROW nRows = aRange.aEnd.Row() - aRange.aStart.Row() + 1;

    if (aRange.aEnd.Row() == MAXROW)
        m_pRBUnlimited->Check();
    else
    {
        m_pRBMaxLimit->Check();
        m_pEdLimit->SetText(OUString::number(nRows));
    }

    DataStream::MoveType eMove = rStrm.GetMove();
    switch (eMove)
    {
        case DataStream::MOVE_DOWN:
            m_pRBDataDown->Check();
        break;
        case DataStream::RANGE_DOWN:
            m_pRBRangeDown->Check();
        break;
        case DataStream::MOVE_UP:
        case DataStream::NO_MOVE:
        default:
            ;
    }

    m_pCBRefreshOnEmpty->Check(rStrm.IsRefreshOnEmptyLine());

    UpdateEnable();
}

void DataStreamDlg::StartStream()
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
    if (m_pRBValuesInLine->IsChecked())
       nSettings |= DataStream::VALUES_IN_LINE;

    DataStream::MoveType eMove =
        m_pRBRangeDown->IsChecked() ? DataStream::RANGE_DOWN : DataStream::MOVE_DOWN;

    DataStream* pStream = DataStream::Set(mpDocShell, rURL, aStartRange, nLimit, eMove, nSettings);
    pStream->SetRefreshOnEmptyLine(m_pCBRefreshOnEmpty->IsChecked());
    DataStream::MakeToolbarVisible();
    pStream->StartImport();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
