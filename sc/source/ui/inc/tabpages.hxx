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

#include <sfx2/tabdlg.hxx>

class ScTabPageProtection : public SfxTabPage
{
    static const sal_uInt16 pProtectionRanges[];
public:
    ScTabPageProtection(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController,
                                              const SfxItemSet* rAttrSet);
    virtual ~ScTabPageProtection() override;

    static  const sal_uInt16* GetRanges () { return pProtectionRanges; }
    virtual bool        FillItemSet     ( SfxItemSet* rCoreAttrs ) override;
    virtual void        Reset           ( const SfxItemSet* ) override;

protected:
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:
                                        // current status:
    bool            bTriEnabled;        //  if before - DontCare
    bool            bDontCare;          //  all in  TriState
    bool            bProtect;           //  secure individual settings for TriState
    bool            bHideForm;
    bool            bHideCell;
    bool            bHidePrint;

    weld::TriStateEnabled aHideCellState;
    weld::TriStateEnabled aProtectState;
    weld::TriStateEnabled aHideFormulaState;
    weld::TriStateEnabled aHidePrintState;

    std::unique_ptr<weld::CheckButton> m_xBtnHideCell;
    std::unique_ptr<weld::CheckButton> m_xBtnProtect;
    std::unique_ptr<weld::CheckButton> m_xBtnHideFormula;
    std::unique_ptr<weld::CheckButton> m_xBtnHidePrint;

    // Handler:
    DECL_LINK(ProtectClickHdl, weld::ToggleButton&, void);
    DECL_LINK(HideCellClickHdl, weld::ToggleButton&, void);
    DECL_LINK(HideFormulaClickHdl, weld::ToggleButton&, void);
    DECL_LINK(HidePrintClickHdl, weld::ToggleButton&, void);
    void ButtonClick(weld::ToggleButton& rBox);
    void UpdateButtons();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
