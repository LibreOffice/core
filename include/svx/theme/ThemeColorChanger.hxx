/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/svxdllapi.h>
#include <svx/theme/IThemeColorChanger.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>

namespace svx
{
namespace theme
{
SVXCORE_DLLPUBLIC void updateSdrObject(model::ColorSet const& rColorSet, SdrObject* pObject);
}

class SVXCORE_DLLPUBLIC ThemeColorChanger : public IThemeColorChanger
{
private:
    SdrPage* mpPage;

public:
    ThemeColorChanger(SdrPage* pPage);
    virtual ~ThemeColorChanger() override;

    void apply(std::shared_ptr<model::ColorSet> const& pColorSet) override;
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
