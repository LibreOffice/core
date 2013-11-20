/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/filedlghelper.hxx>
#include <svtools/inettbc.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <datastreams.hxx>

namespace {

class DataStreamsDlg : public ModalDialog
{
    DataStreams *mpDataStreams;

    SvtURLBox*      m_pCbUrl;
    PushButton*     m_pBtnBrowse;
    RadioButton*    m_pRBScriptData;
    RadioButton*    m_pRBValuesInLine;
    RadioButton*    m_pRBAddressValue;
    RadioButton*    m_pRBRangeDown;
    RadioButton*    m_pRBNoMove;
    RadioButton*    m_pRBMaxLimit;
    Edit*           m_pEdRange;
    Edit*           m_pEdLimit;
    OKButton*       m_pBtnOk;
    VclFrame*       m_pVclFrameLimit;
    VclFrame*       m_pVclFrameMove;
    VclFrame*       m_pVclFrameRange;

    DECL_LINK(UpdateHdl, void *);
    DECL_LINK(BrowseHdl, void *);

    void UpdateEnable();

public:
    DataStreamsDlg(DataStreams *pDataStreams, Window* pParent);
    ~DataStreamsDlg() {}
    void Start();
};

DataStreamsDlg::DataStreamsDlg(DataStreams *pDataStreams, Window* pParent)
    : ModalDialog(pParent, "DataStreamsDialog", "modules/scalc/ui/datastreams.ui")
      , mpDataStreams(pDataStreams)
{
    get(m_pCbUrl, "url");
    get(m_pBtnBrowse, "browse");
    get(m_pRBScriptData, "scriptdata");
    get(m_pRBValuesInLine, "valuesinline");
    get(m_pRBAddressValue, "addressvalue");
    get(m_pRBRangeDown, "rangedown");
    get(m_pRBNoMove, "nomove");
    get(m_pRBMaxLimit, "maxlimit");
    get(m_pEdRange, "range");
    get(m_pEdLimit, "limit");
    get(m_pBtnOk, "ok");
    get(m_pVclFrameLimit, "framelimit");
    get(m_pVclFrameMove, "framemove");
    get(m_pVclFrameRange, "framerange");

    m_pCbUrl->SetSelectHdl( LINK( this, DataStreamsDlg, UpdateHdl ) );
    m_pRBAddressValue->SetClickHdl( LINK( this, DataStreamsDlg, UpdateHdl ) );
    m_pRBValuesInLine->SetClickHdl( LINK( this, DataStreamsDlg, UpdateHdl ) );
    m_pEdRange->SetModifyHdl( LINK( this, DataStreamsDlg, UpdateHdl ) );
    m_pBtnBrowse->SetClickHdl( LINK( this, DataStreamsDlg, BrowseHdl ) );
    UpdateEnable();
}

void DataStreamsDlg::Start()
{
    sal_Int32 nLimit = 0;
    if (m_pRBMaxLimit->IsChecked())
        nLimit = m_pEdLimit->GetText().toInt32();
    mpDataStreams->Set(
            (m_pRBScriptData->IsChecked() ?
                dynamic_cast<SvStream*>( new SvScriptStream(m_pCbUrl->GetText()) ) :
                dynamic_cast<SvStream*>( new SvFileStream(m_pCbUrl->GetText(), STREAM_READ) )),
            m_pRBValuesInLine->IsChecked(),
            m_pEdRange->GetText(), nLimit, (m_pRBNoMove->IsChecked() ? DataStreams::NO_MOVE :
            m_pRBRangeDown->IsChecked() ? DataStreams::RANGE_DOWN : DataStreams::MOVE_DOWN) );
    mpDataStreams->Start();
}

IMPL_LINK_NOARG(DataStreamsDlg, BrowseHdl)
{
    sfx2::FileDialogHelper aFileDialog(0, 0);
    if ( aFileDialog.Execute() != ERRCODE_NONE )
        return 0;

    m_pCbUrl->SetText( aFileDialog.GetPath() );
    UpdateEnable();
    return 0;
}

IMPL_LINK_NOARG(DataStreamsDlg, UpdateHdl)
{
    UpdateEnable();
    return 0;
}

void DataStreamsDlg::UpdateEnable()
{
    bool bOk = !m_pCbUrl->GetURL().isEmpty();
    if (m_pRBAddressValue->IsChecked())
    {
        m_pVclFrameLimit->Hide();
        m_pVclFrameMove->Hide();
        m_pVclFrameRange->Hide();
    }
    else
    {
        m_pVclFrameLimit->Show(true);
        m_pVclFrameMove->Show();
        m_pVclFrameRange->Show();
        bOk = bOk && !m_pEdRange->GetText().isEmpty();
    }
    m_pBtnOk->Enable(bOk);
}

}

void DataStreams::ShowDialog(Window *pParent)
{
    DataStreamsDlg aDialog(this, pParent);
    if (aDialog.Execute() == RET_OK)
        aDialog.Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
