/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_STYLEMANAGER_HXX
#define INCLUDED_SFX2_STYLEMANAGER_HXX

#include <sfx2/dllapi.h>

#include <svl/style.hxx>
#include <tools/long.hxx>

#include <memory>

class OutputDevice;
class SfxObjectShell;
namespace sfx2 { class StylePreviewRenderer; }

namespace sfx2
{

class SFX2_DLLPUBLIC StyleManager
{
protected:
    SfxObjectShell& mrShell;

public:
    StyleManager(SfxObjectShell& rShell)
        : mrShell(rShell)
    {}

    virtual ~StyleManager()
    {}

    SfxStyleSheetBase* Search(const OUString& rStyleName, SfxStyleFamily eFamily);

    virtual std::unique_ptr<StylePreviewRenderer> CreateStylePreviewRenderer(
                    OutputDevice& rOutputDev, SfxStyleSheetBase* pStyle,
                    tools::Long nMaxHeight) = 0;
};

} // end namespace sfx2

#endif //INCLUDED_SFX2_STYLEMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
