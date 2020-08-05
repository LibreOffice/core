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
#include <svx/dialcontrol.hxx>

namespace chart { class TextDirectionListBox; }
namespace weld {
    class CheckButton;
    class CustomWeld;
    class Label;
    class SpinButton;
    class ToggleButton;
}

namespace chart
{

class SchAlignmentTabPage : public SfxTabPage
{
private:
    std::unique_ptr<weld::Label> m_xFtRotate;
    std::unique_ptr<weld::MetricSpinButton> m_xNfRotate;
    std::unique_ptr<weld::CheckButton> m_xCbStacked;
    std::unique_ptr<weld::Label> m_xFtTextDirection;
    std::unique_ptr<weld::Label> m_xFtABCD;
    std::unique_ptr<TextDirectionListBox> m_xLbTextDirection;
    std::unique_ptr<svx::DialControl> m_xCtrlDial;
    std::unique_ptr<weld::CustomWeld> m_xCtrlDialWin;

    DECL_LINK(StackedToggleHdl, weld::ToggleButton&, void);

public:
    SchAlignmentTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs, bool bWithRotation = true);
    virtual ~SchAlignmentTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rInAttrs);
    static std::unique_ptr<SfxTabPage> CreateWithoutRotation(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rInAttrs);
    virtual bool FillItemSet(SfxItemSet* rOutAttrs) override;
    virtual void Reset(const SfxItemSet* rInAttrs) override;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
