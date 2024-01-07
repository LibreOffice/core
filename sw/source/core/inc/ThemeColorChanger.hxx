/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#pragma once

#include <swdllapi.h>
#include <docsh.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <svx/theme/IThemeColorChanger.hxx>

namespace sw
{
class SW_DLLPUBLIC ThemeColorChanger : public svx::IThemeColorChanger
{
private:
    SwDocShell* mpDocSh;

public:
    ThemeColorChanger(SwDocShell* pDocSh);
    virtual ~ThemeColorChanger() override;

    void doApply(std::shared_ptr<model::ColorSet> const& pColorSet) override;
};

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
