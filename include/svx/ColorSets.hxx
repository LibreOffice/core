/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_COLORSETS_HXX
#define INCLUDED_SVX_COLORSETS_HXX

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <svx/svxdllapi.h>
#include <docmodel/theme/ColorSet.hxx>

namespace svx
{

class SVXCORE_DLLPUBLIC ColorSets
{
    std::vector<model::ColorSet> maColorSets;
public:
    ColorSets();
    ~ColorSets();

    void init();
    const std::vector<model::ColorSet>& getColorSets() const
    {
        return maColorSets;
    }

    const model::ColorSet& getColorSet(sal_uInt32 nIndex) const
    {
        return maColorSets[nIndex];
    }

    const model::ColorSet& getColorSet(std::u16string_view rName);

    void insert(model::ColorSet const& rColorSet);
};

} // end of namespace svx

#endif // INCLUDED_SVX_COLORSETS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
