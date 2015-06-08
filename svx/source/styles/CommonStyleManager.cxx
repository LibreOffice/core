/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/CommonStyleManager.hxx>
#include <svx/CommonStylePreviewRenderer.hxx>

namespace svx
{

sfx2::StylePreviewRenderer* CommonStyleManager::CreateStylePreviewRenderer(
                                            OutputDevice& rOutputDev, OUString const & rName,
                                            SfxStyleFamily eFamily, long nMaxHeight)
{
    SfxStyleSheetBasePool* pPool = mrShell.GetStyleSheetPool();
    if (!pPool)
        return nullptr;

    pPool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL);
    SfxStyleSheetBase* pStyle = nullptr;
    pStyle = pPool->First();

    while (pStyle)
    {
        if (rName == pStyle->GetName())
            return CreateStylePreviewRenderer(rOutputDev, pStyle, nMaxHeight);

        pStyle = pPool->Next();
    }

    return nullptr;
}

sfx2::StylePreviewRenderer* CommonStyleManager::CreateStylePreviewRenderer(
                                            OutputDevice& rOutputDev, SfxStyleSheetBase* pStyle,
                                            long nMaxHeight)
{
    return new CommonStylePreviewRenderer(mrShell, rOutputDev, pStyle, nMaxHeight);
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
