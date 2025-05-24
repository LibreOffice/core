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

#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <set>
#include <svl/itempool.hxx>

namespace svx
{
namespace DocumentColorHelper
{
inline Color getColorFromItem(const SvxColorItem* pItem) { return pItem->GetValue(); }

inline Color getColorFromItem(const SvxBrushItem* pItem) { return pItem->GetColor(); }

template <class T>
void queryColors(const sal_uInt16 nAttrib, const SfxItemPool* pPool, std::set<Color>& rOutput)
{
    for (const SfxPoolItem* pItem : pPool->GetItemSurrogates(nAttrib))
    {
        auto pColorItem = static_cast<const T*>(pItem);
        Color aColor(getColorFromItem(pColorItem));
        if (COL_AUTO != aColor)
            rOutput.insert(aColor);
    }
}
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
