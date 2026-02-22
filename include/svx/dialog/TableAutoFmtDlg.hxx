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

#pragma once

#include <vcl/weld/DialogController.hxx>
#include <vcl/weld/MetricSpinButton.hxx>
#include <vcl/weld/TreeView.hxx>
#include "TableAutoFmtPreview.hxx"

class SVX_DLLPUBLIC SvxTableAutoFmtDlg : public weld::GenericDialogController
{
private:
    // Resource strings
    OUString maStrTitle;
    OUString maStrLabel;
    OUString maStrClose;
    OUString maStrDelMsg;
    OUString maStrDelTitle;
    OUString maStrRename;

    // Core Data
    SvxAutoFormat& mpFormat;
    OUString maFormatName;
    size_t mnIndex;
    bool mbWriter;
    bool mbRTL;

    // Preview
    SvxAutoFmtPreview maWndPreview;

    // UI Controls
    std::unique_ptr<weld::TreeView> mxLbFormat;
    std::unique_ptr<weld::Button> mxBtnCancel;
    std::unique_ptr<weld::Button> mxBtnAdd;
    std::unique_ptr<weld::Button> mxBtnEdit;
    std::unique_ptr<weld::Button> mxBtnRemove;
    std::unique_ptr<weld::Button> mxBtnRename;
    std::unique_ptr<weld::CheckButton> mxBtnNumFormat;
    std::unique_ptr<weld::CheckButton> mxBtnBorder;
    std::unique_ptr<weld::CheckButton> mxBtnFont;
    std::unique_ptr<weld::CheckButton> mxBtnPattern;
    std::unique_ptr<weld::CheckButton> mxBtnAlignment;
    std::unique_ptr<weld::CheckButton> mxBtnAdjust;
    std::unique_ptr<weld::CustomWeld> mxWndPreview;

    DECL_LINK(CheckHdl, weld::Toggleable&, void);
    DECL_LINK(AddHdl, weld::Button&, void);
    DECL_LINK(EditHdl, weld::Button&, void);
    DECL_LINK(RemoveHdl, weld::Button&, void);
    DECL_LINK(RenameHdl, weld::Button&, void);
    DECL_LINK(SelFormatHdl, weld::TreeView&, void);
    DECL_LINK(DblClkHdl, weld::TreeView&, bool);

    OUString GenerateUniqueStyleName();

    void Init();
    void UpdateChecks();
    void PopulateFormatList();
    void UpdateUIState();

public:
    SvxTableAutoFmtDlg(SvxAutoFormat& rFormat, const OUString& sFormatName, weld::Window* pParent,
                       bool bWriter, bool bRTL);

    size_t GetIndex() const { return mnIndex; }
    OUString GetCurrFormatName();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
