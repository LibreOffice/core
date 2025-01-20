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
#include <docmodel/theme/ColorSet.hxx>

namespace svx
{

class SVXCORE_DLLPUBLIC ColorSets
{
private:
    std::vector<model::ColorSet> maColorSets;
    ColorSets();
    void init();
public:
    enum class IdenticalNameAction { Overwrite, AutoRename };
    static ColorSets& get();

    const std::vector<model::ColorSet>& getColorSetVector() const
    {
        return maColorSets;
    }

    const model::ColorSet& getColorSet(sal_uInt32 nIndex) const
    {
        return maColorSets[nIndex];
    }

    model::ColorSet const* getColorSet(std::u16string_view rName) const;

    void insert(model::ColorSet const& rColorSet, IdenticalNameAction eAction);
};

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
