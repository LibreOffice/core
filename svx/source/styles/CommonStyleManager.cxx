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
#include <CommonStylePreviewRenderer.hxx>

namespace svx
{

std::unique_ptr<sfx2::StylePreviewRenderer> CommonStyleManager::CreateStylePreviewRenderer(
                                            OutputDevice& rOutputDev, SfxStyleSheetBase* pStyle,
                                            tools::Long nMaxHeight)
{
    return std::unique_ptr<sfx2::StylePreviewRenderer>(new CommonStylePreviewRenderer(mrShell, rOutputDev, pStyle, nMaxHeight));
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
