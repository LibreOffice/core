/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_COMMONSTYLEMANAGER_HXX
#define INCLUDED_SVX_COMMONSTYLEMANAGER_HXX

#include <sfx2/StyleManager.hxx>

#include <vcl/outdev.hxx>
#include <svx/svxdllapi.h>
#include <rsc/rscsfx.hxx>

namespace svx
{

class SVX_DLLPUBLIC CommonStyleManager : public sfx2::StyleManager
{
public:
    CommonStyleManager(SfxObjectShell& rShell)
        : StyleManager(rShell)
    {}

    virtual ~CommonStyleManager()
    {}

    virtual sfx2::StylePreviewRenderer* CreateStylePreviewRenderer(
                                            OutputDevice& rOutputDev, SfxStyleSheetBase* pStyle,
                                            long nMaxHeight = 32) override;
};

} // end namespace svx

#endif // INCLUDED_SVX_COMMONSTYLEMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
