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
#include <address.hxx>
#include <docsh.hxx>
#include <datastream.hxx>

namespace sc {

DataStreamDlg::DataStreamDlg(ScDocShell *pDocShell, weld::Window* pParent)
    : GenericDialogController(pParent, "modules/scalc/ui/datastreams.ui", "DataStreamDialog")
    , m_pDocShell(pDocShell)
    , m_xCbUrl(new URLBox(m_xBuilder->weld_combo_box("url")))
    , m_xBtnBrowse(m_xBuilder->weld_button("browse"))
    , m_xRBValuesInLine(m_xBuilder->weld_radio_button("valuesinline"))
    , m_xRBAddressValue(m_xBuilder->weld_radio_button("addressvalue"))
    , m_xCBRefreshOnEmpty(m_xBuilder->weld_check_button("refresh_ui"))
    , m_xRBDataDown(m_xBuilder->weld_radio_button("datadown"))
    , m_xRBRangeDown(m_xBuilder->weld_radio_button("rangedown"))
    , m_xRBNoMove(m_xBuilder->weld_radio_button("nomove"))
    , m_xRBMaxLimit(m_xBuilder->weld_radio_button("maxlimit"))
    , m_xRBUnlimited(m_xBuilder->weld_radio_button("unlimited"))
    , m_xEdRange(m_xBuilder->weld_entry("range"))
    , m_xEdLimit(m_xBuilder->weld_entry("limit"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xVclFrameLimit(m_xBuilder->weld_frame("framelimit"))
    , m_xVclFrameMove(m_xBuilder->weld_frame("framemove"))
{
    m_xCbUrl->connect_changed( LINK( this, DataStreamDlg, UpdateComboBoxHdl ) );
    m_xRBAddressValue->connect_toggled( LINK( this, DataStreamDlg, UpdateClickHdl ) );
    m_xRBAddressValue->set_sensitive(false);
    m_xRBNoMove->hide();
    m_xRBValuesInLine->connect_toggled( LINK( this, DataStreamDlg, UpdateClickHdl ) );
    m_xEdRange->connect_changed( LINK( this, DataStreamDlg, UpdateHdl ) );
    m_xBtnBrowse->connect_clicked( LINK( this, DataStreamDlg, BrowseHdl ) );
    UpdateEnable();
}

DataStreamDlg::~DataStreamDlg()
{
}

IMPL_LINK_NOARG(DataStreamDlg, BrowseHdl, weld::Button&, void)
{
    sfx2::FileDialogHelper aFileDialog(0, FileDialogFlags::NONE, m_xDialog.get());
    if ( aFileDialog.Execute() != ERRCODE_NONE )
        return;

    m_xCbUrl->set_entry_text(aFileDialog.GetPath());
    UpdateEnable();
}

IMPL_LINK_NOARG(DataStreamDlg, UpdateClickHdl, weld::ToggleButton&, void)
{
    UpdateEnable();
}

IMPL_LINK_NOARG(DataStreamDlg, UpdateComboBoxHdl, weld::ComboBox&, void)
{
    UpdateEnable();
}

IMPL_LINK_NOARG(DataStreamDlg, UpdateHdl, weld::Entry&, void)
{
    UpdateEnable();
}

void DataStreamDlg::UpdateEnable()
{
    bool bOk = !m_xCbUrl->GetURL().isEmpty();
    if (m_xRBAddressValue->get_active())
    {
        m_xVclFrameLimit->set_sensitive(false);
        m_xVclFrameMove->set_sensitive(false);
        m_xEdRange->set_sensitive(false);
    }
    else
    {
        m_xVclFrameLimit->set_sensitive(true);
        m_xVclFrameMove->set_sensitive(true);
        m_xEdRange->set_sensitive(true);
        if (bOk)
        {
            // Check the given range to make sure it's valid.
            ScRange aTest = GetStartRange();
            if (!aTest.IsValid())
                bOk = false;
        }
    }
    m_xBtnOk->set_sensitive(bOk);
//    setOptimalLayoutSize();
}

ScRange DataStreamDlg::GetStartRange()
{
    OUString aStr = m_xEdRange->get_text();
    ScDocument& rDoc = m_pDocShell->GetDocument();
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
    m_xCbUrl->set_entry_text(rStrm.GetURL());
    ScDocument& rDoc = m_pDocShell->GetDocument();

    ScRange aRange = rStrm.GetRange();
    ScRange aTopRange = aRange;
    aTopRange.aEnd.SetRow(aTopRange.aStart.Row());
    OUString aStr = aTopRange.Format(ScRefFlags::RANGE_ABS_3D, &rDoc, rDoc.GetAddressConvention());
    m_xEdRange->set_text(aStr);
    SCROW nRows = aRange.aEnd.Row() - aRange.aStart.Row() + 1;

    if (aRange.aEnd.Row() == MAXROW)
        m_xRBUnlimited->set_active(true);
    else
    {
        m_xRBMaxLimit->set_active(true);
        m_xEdLimit->set_text(OUString::number(nRows));
    }

    DataStream::MoveType eMove = rStrm.GetMove();
    switch (eMove)
    {
        case DataStream::MOVE_DOWN:
            m_xRBDataDown->set_active(true);
        break;
        case DataStream::RANGE_DOWN:
            m_xRBRangeDown->set_active(true);
        break;
        case DataStream::MOVE_UP:
        case DataStream::NO_MOVE:
        default:
            ;
    }

    m_xCBRefreshOnEmpty->set_active(rStrm.IsRefreshOnEmptyLine());

    UpdateEnable();
}

void DataStreamDlg::StartStream()
{
    ScRange aStartRange = GetStartRange();
    if (!aStartRange.IsValid())
        // Don't start the stream without a valid range.
        return;

    sal_Int32 nLimit = 0;
    if (m_xRBMaxLimit->get_active())
        nLimit = m_xEdLimit->get_text().toInt32();
    OUString rURL = m_xCbUrl->get_active_text();
    sal_uInt32 nSettings = 0;
    if (m_xRBValuesInLine->get_active())
       nSettings |= DataStream::VALUES_IN_LINE;

    DataStream::MoveType eMove =
        m_xRBRangeDown->get_active() ? DataStream::RANGE_DOWN : DataStream::MOVE_DOWN;

    DataStream* pStream = DataStream::Set(m_pDocShell, rURL, aStartRange, nLimit, eMove, nSettings);
    pStream->SetRefreshOnEmptyLine(m_xCBRefreshOnEmpty->get_active());
    DataStream::MakeToolbarVisible();
    pStream->StartImport();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
