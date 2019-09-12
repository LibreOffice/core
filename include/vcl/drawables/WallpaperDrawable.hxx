/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLES_WALLPAPERDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLES_WALLPAPERDRAWABLE_HXX

#include <tools/gen.hxx>

#include <vcl/hatch.hxx>
#include <vcl/drawables/Drawable.hxx>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC WallpaperDrawable : public Drawable
{
public:
    WallpaperDrawable(tools::Rectangle aRect, Wallpaper aWallpaper, bool bUseScaffolding = true)
        : Drawable(bUseScaffolding)
        , maRect(aRect)
        , maWallpaper(aWallpaper)
        , mbUseScaffolding(bUseScaffolding)
    {
        mpMetaAction = new MetaWallpaperAction(aRect, aWallpaper);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;
    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    static bool DrawWallpaper(OutputDevice* pRenderContext, long nX, long nY, long nWidth,
                              long nHeight, const Wallpaper& rWallpaper);

    static bool DrawBitmapWallpaper(OutputDevice* pRenderContext, long nX, long nY, long nWidth,
                                    long nHeight, Wallpaper const& rWallpaper);
    static bool DrawGradientWallpaper(OutputDevice* pRenderContext, long nX, long nY, long nWidth,
                                      long nHeight, Wallpaper const& rWallpaper);
    static bool DrawColorWallpaper(OutputDevice* pRenderContext, long nX, long nY, long nWidth,
                                   long nHeight, Wallpaper const& rWallpaper);

    tools::Rectangle maRect;
    Wallpaper maWallpaper;
    bool mbUseScaffolding;
};

} // namespace vcl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
