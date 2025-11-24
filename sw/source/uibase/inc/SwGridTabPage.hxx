/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/tabdlg.hxx>
#include <svx/optgrid.hxx>

/// Grid tab page class that handles Writer-only grid settings (e.g. baseline grid options).
class SwGridTabPage final : public SvxGridTabPage
{
public:
    SwGridTabPage(weld::Container* pPage, weld::DialogController* pController,
                  const SfxItemSet& rItemSet);

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* pItemSet);

    virtual bool FillItemSet(SfxItemSet* pItemSet) override;
    virtual void Reset(const SfxItemSet* pItemSet) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
