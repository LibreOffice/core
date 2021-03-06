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

class SdPrintOptions : public SfxTabPage
{
    friend class SdModule;

private:
    std::unique_ptr<weld::Frame> m_xFrmContent;
    std::unique_ptr<weld::CheckButton> m_xCbxDraw;
    std::unique_ptr<weld::CheckButton> m_xCbxNotes;
    std::unique_ptr<weld::CheckButton> m_xCbxHandout;
    std::unique_ptr<weld::CheckButton> m_xCbxOutline;
    std::unique_ptr<weld::RadioButton> m_xRbtColor;
    std::unique_ptr<weld::RadioButton> m_xRbtGrayscale;
    std::unique_ptr<weld::RadioButton> m_xRbtBlackWhite;
    std::unique_ptr<weld::CheckButton> m_xCbxPagename;
    std::unique_ptr<weld::CheckButton> m_xCbxDate;
    std::unique_ptr<weld::CheckButton> m_xCbxTime;
    std::unique_ptr<weld::CheckButton> m_xCbxHiddenPages;
    std::unique_ptr<weld::RadioButton> m_xRbtDefault;
    std::unique_ptr<weld::RadioButton> m_xRbtPagesize;
    std::unique_ptr<weld::RadioButton> m_xRbtPagetile;
    std::unique_ptr<weld::RadioButton> m_xRbtBooklet;
    std::unique_ptr<weld::CheckButton> m_xCbxFront;
    std::unique_ptr<weld::CheckButton> m_xCbxBack;
    std::unique_ptr<weld::CheckButton> m_xCbxPaperbin;

    DECL_LINK(ClickCheckboxHdl, weld::ToggleButton&, void);
    DECL_LINK(ClickBookletHdl, weld::ToggleButton&, void);

    void updateControls();

public:
    SdPrintOptions(weld::Container* pPage, weld::DialogController* pController,
                   const SfxItemSet& rInAttrs);
    virtual ~SdPrintOptions() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet*);

    virtual bool FillItemSet(SfxItemSet*) override;
    virtual void Reset(const SfxItemSet*) override;

    void SetDrawMode();
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
