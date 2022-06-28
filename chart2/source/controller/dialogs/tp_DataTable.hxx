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

namespace weld
{
class CheckButton;
}

namespace chart
{
class DataTableTabPage : public SfxTabPage
{
private:
    std::unique_ptr<weld::CheckButton> m_xCbHorizontalBorder;
    std::unique_ptr<weld::CheckButton> m_xCbVerticalBorder;
    std::unique_ptr<weld::CheckButton> m_xCbOutilne;
    std::unique_ptr<weld::CheckButton> m_xCbKeys;

public:
    DataTableTabPage(weld::Container* pPage, weld::DialogController* pController,
                     const SfxItemSet& rInAttrs);
    virtual ~DataTableTabPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rInAttrs);

    virtual bool FillItemSet(SfxItemSet* rOutAttrs) override;
    virtual void Reset(const SfxItemSet* rInAttrs) override;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
