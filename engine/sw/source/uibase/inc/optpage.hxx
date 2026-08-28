/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sfx2/tabdlg.hxx>

#include <vcl/weld.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/colorbox.hxx>
#include <svx/fntctrl.hxx>
#include "fontcfg.hxx"
class SfxPrinter;
class SwStdFontConfig;
class SwWrtShell;
class FontList;

// TabPage printer settings additions
class SwAddPrinterTabPage final : public SfxTabPage
{
    OUString m_sNone;
    bool m_bAttrModified;
    bool m_bPreview;
    bool m_bHTMLMode;

    std::unique_ptr<weld::CheckButton>  m_xGrfCB;
    std::unique_ptr<weld::Widget>  m_xGrfImg;
    std::unique_ptr<weld::CheckButton>  m_xCtrlFieldCB;
    std::unique_ptr<weld::Widget>  m_xCtrlFieldImg;
    std::unique_ptr<weld::CheckButton>  m_xBackgroundCB;
    std::unique_ptr<weld::Widget>  m_xBackgroundImg;
    std::unique_ptr<weld::CheckButton>  m_xBlackFontCB;
    std::unique_ptr<weld::Widget>  m_xBlackFontImg;
    std::unique_ptr<weld::CheckButton>  m_xPrintHiddenTextCB;
    std::unique_ptr<weld::Widget>  m_xPrintHiddenTextImg;
    std::unique_ptr<weld::CheckButton>  m_xPrintTextPlaceholderCB;
    std::unique_ptr<weld::Widget>  m_xPrintTextPlaceholderImg;

    std::unique_ptr<weld::Widget>       m_xPagesFrame;
    std::unique_ptr<weld::CheckButton>  m_xLeftPageCB;
    std::unique_ptr<weld::Widget>  m_xLeftPageImg;
    std::unique_ptr<weld::CheckButton>  m_xRightPageCB;
    std::unique_ptr<weld::Widget>  m_xRightPageImg;
    std::unique_ptr<weld::CheckButton>  m_xProspectCB;
    std::unique_ptr<weld::Widget>  m_xProspectImg;
    std::unique_ptr<weld::CheckButton>  m_xProspectCB_RTL;
    std::unique_ptr<weld::Widget>  m_xProspectImg_RTL;

    std::unique_ptr<weld::Widget>       m_xCommentsFrame;
    std::unique_ptr<weld::RadioButton>  m_xNoRB;
    std::unique_ptr<weld::RadioButton>  m_xOnlyRB;
    std::unique_ptr<weld::RadioButton>  m_xEndRB;
    std::unique_ptr<weld::RadioButton>  m_xEndPageRB;
    std::unique_ptr<weld::RadioButton>  m_xInMarginsRB;
    std::unique_ptr<weld::Widget>  m_xMarginsImg;

    std::unique_ptr<weld::CheckButton>  m_xPrintEmptyPagesCB;
    std::unique_ptr<weld::Widget>  m_xPrintEmptyPagesImg;
    std::unique_ptr<weld::CheckButton>  m_xPaperFromSetupCB;
    std::unique_ptr<weld::Widget>  m_xPaperFromSetupImg;
    std::unique_ptr<weld::ComboBox> m_xFaxLB;
    std::unique_ptr<weld::Widget> m_xFaxImg;

    DECL_LINK(AutoClickHdl, weld::Toggleable&, void);
    DECL_LINK(SelectHdl, weld::ComboBox&, void);

public:
    SwAddPrinterTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwAddPrinterTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
    void SetFax(const std::vector<OUString>& );
    void SetPreview(bool bPrev);
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
