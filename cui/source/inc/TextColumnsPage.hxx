/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <sfx2/tabdlg.hxx>

#include <memory>

/// Tab page for EditEngine columns properties
class SvxTextColumnsPage : public SfxTabPage
{
private:
    static const WhichRangesContainer pRanges;

    std::unique_ptr<weld::SpinButton> m_xColumnsNumber;
    std::unique_ptr<weld::MetricSpinButton> m_xColumnsSpacing;

public:
    SvxTextColumnsPage(weld::Container* pPage, weld::DialogController* pController,
                       const SfxItemSet& rInAttrs);
    virtual ~SvxTextColumnsPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet*);
    static const WhichRangesContainer& GetRanges() { return pRanges; }

    virtual bool FillItemSet(SfxItemSet*) override;
    virtual void Reset(const SfxItemSet*) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
