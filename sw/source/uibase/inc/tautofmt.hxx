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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_TAUTOFMT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_TAUTOFMT_HXX

#include <tools/link.hxx>
#include <sfx2/basedlgs.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vcl/weld.hxx>

#include "wrtsh.hxx"
#include "autoformatpreview.hxx"
#include <tblafmt.hxx>

class SwTableAutoFormat;
class SwTableAutoFormatTable;
class SwWrtShell;

class SwAutoFormatDlg final : public SfxDialogController
{
    OUString m_aStrTitle;
    OUString m_aStrLabel;
    OUString m_aStrClose;
    OUString m_aStrDelTitle;
    OUString m_aStrDelMsg;
    OUString m_aStrRenameTitle;
    OUString m_aStrInvalidFormat;

    SwWrtShell* m_pShell;
    sal_uInt8 m_nIndex;
    sal_uInt8 m_nDfltStylePos;
    bool m_bCoreDataChanged : 1;
    bool m_bSetAutoFormat : 1;

    AutoFormatPreview m_aWndPreview;
    std::unique_ptr<SwTableAutoFormatTable> m_xTableTable;

    std::unique_ptr<weld::TreeView> m_xLbFormat;
    std::unique_ptr<weld::CheckButton> m_xBtnNumFormat;
    std::unique_ptr<weld::CheckButton> m_xBtnBorder;
    std::unique_ptr<weld::CheckButton> m_xBtnFont;
    std::unique_ptr<weld::CheckButton> m_xBtnPattern;
    std::unique_ptr<weld::CheckButton> m_xBtnAlignment;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnRemove;
    std::unique_ptr<weld::Button> m_xBtnRename;
    std::unique_ptr<weld::CustomWeld> m_xWndPreview;

    void Init(const SwTableAutoFormat* pSelFormat);
    void UpdateChecks(const SwTableAutoFormat&, bool bEnableBtn);

    DECL_LINK(CheckHdl, weld::Toggleable&, void);
    DECL_LINK(AddHdl, weld::Button&, void);
    DECL_LINK(RemoveHdl, weld::Button&, void);
    DECL_LINK(RenameHdl, weld::Button&, void);
    DECL_LINK(SelFormatHdl, weld::TreeView&, void);

public:
    SwAutoFormatDlg(weld::Window* pParent, SwWrtShell* pShell, bool bSetAutoFormat,
                    const SwTableAutoFormat* pSelFormat);

    std::unique_ptr<SwTableAutoFormat> FillAutoFormatOfIndex() const;

    void Apply();

    virtual ~SwAutoFormatDlg() override;
};

#endif // SW_AUTOFMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
