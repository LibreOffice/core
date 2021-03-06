/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/tabdlg.hxx>

class ScTpCompatOptions : public SfxTabPage
{
public:
    explicit ScTpCompatOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreAttrs);
    virtual ~ScTpCompatOptions() override;

    virtual bool FillItemSet(SfxItemSet* rCoreAttrs) override;
    virtual void Reset(const SfxItemSet* rCoreAttrs) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet ) override;

private:
    std::unique_ptr<weld::ComboBox> m_xLbKeyBindings;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
