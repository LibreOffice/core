/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/StyleManager.hxx>

namespace sfx2
{

SfxStyleSheetBase* StyleManager::Search(const OUString& rStyleName, SfxStyleFamily eFamily)
{
    SfxStyleSheetBasePool* pPool = mrShell.GetStyleSheetPool();
    if (!pPool)
        return nullptr;

    pPool->SetSearchMask(eFamily);
    SfxStyleSheetBase* pStyle = nullptr;
    pStyle = pPool->First();

    while (pStyle)
    {
        if (rStyleName == pStyle->GetName())
            return pStyle;

        pStyle = pPool->Next();
    }

    return nullptr;
}

} // end namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
