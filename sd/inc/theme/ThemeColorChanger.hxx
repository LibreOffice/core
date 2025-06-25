/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sddllapi.h>
#include <svx/theme/IThemeColorChanger.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <svx/svdpage.hxx>
#include <drawdoc.hxx>

namespace sd
{
class SD_DLLPUBLIC ThemeColorChanger final : public svx::IThemeColorChanger
{
private:
    SdrPage* mpMasterPage;
    sd::DrawDocShell* mpDocShell;

public:
    ThemeColorChanger(SdrPage* pMasterPage, sd::DrawDocShell* pDocShell);
    virtual ~ThemeColorChanger() override;

    void doApply(std::shared_ptr<model::ColorSet> const& pColorSet) override;
};

} // end sd namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
