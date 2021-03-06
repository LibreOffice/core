/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <vcl/weld.hxx>

class ScDocShell;
class SvtURLBox;
class ScRange;

namespace sc
{
class DataStream;

class DataStreamDlg : public weld::GenericDialogController
{
    ScDocShell* m_pDocShell;

    std::unique_ptr<SvtURLBox> m_xCbUrl;
    std::unique_ptr<weld::Button> m_xBtnBrowse;
    std::unique_ptr<weld::RadioButton> m_xRBValuesInLine;
    std::unique_ptr<weld::RadioButton> m_xRBAddressValue;
    std::unique_ptr<weld::CheckButton> m_xCBRefreshOnEmpty;
    std::unique_ptr<weld::RadioButton> m_xRBDataDown;
    std::unique_ptr<weld::RadioButton> m_xRBRangeDown;
    std::unique_ptr<weld::RadioButton> m_xRBNoMove;
    std::unique_ptr<weld::RadioButton> m_xRBMaxLimit;
    std::unique_ptr<weld::RadioButton> m_xRBUnlimited;
    std::unique_ptr<weld::Entry> m_xEdRange;
    std::unique_ptr<weld::Entry> m_xEdLimit;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Frame> m_xVclFrameLimit;
    std::unique_ptr<weld::Frame> m_xVclFrameMove;

    DECL_LINK(UpdateClickHdl, weld::ToggleButton&, void);
    DECL_LINK(UpdateHdl, weld::Entry&, void);
    DECL_LINK(UpdateComboBoxHdl, weld::ComboBox&, void);
    DECL_LINK(BrowseHdl, weld::Button&, void);

    void UpdateEnable();
    ScRange GetStartRange();

public:
    DataStreamDlg(ScDocShell* pDocShell, weld::Window* pParent);
    virtual ~DataStreamDlg() override;

    void Init(const DataStream& rStrm);

    void StartStream();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
