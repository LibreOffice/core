/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_RENDERSETTINGS_HXX
#define INCLUDED_VCL_RENDERSETTINGS_HXX

#include <vcl/window.hxx>
#include <vcl/outdev.hxx>
#include <vcl/outdevstate.hxx>

namespace vcl
{

class VCL_DLLPUBLIC RenderSettings
{
    OutDevState maOutDevState;
    std::unique_ptr<Wallpaper> mpBackground;

public:
    RenderSettings()
    {}

    virtual ~RenderSettings()
    {}

    inline void SetLineColor(const Color& rColor);
    inline void SetFillColor(const Color& rColor);
    inline void SetBackground(const Wallpaper& rBackground);
    inline void SetFont(const vcl::Font& rNewFont);

    void PushAndApply(vcl::RenderContext& rRenderContext);
    void Apply(vcl::RenderContext& rRenderContext);
};

}

#endif // INCLUDED_VCL_RENDERSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
