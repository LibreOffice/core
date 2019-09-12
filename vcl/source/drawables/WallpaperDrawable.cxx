/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/drawables/RectangleDrawable.hxx>
#include <vcl/drawables/GradientDrawable.hxx>
#include <vcl/drawables/WallpaperDrawable.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

#include <salgdi.hxx>

#include <cassert>

namespace vcl
{
class MapModeStasher
{
public:
    MapModeStasher(OutputDevice* pRenderContext)
        : mpRenderContext(pRenderContext)
        , mpMetaFile(pRenderContext->GetConnectMetaFile())
        , mbIsMapModeEnabled(pRenderContext->IsMapModeEnabled())
    {
        pRenderContext->EnableMapMode(false);
        pRenderContext->Push(PushFlags::CLIPREGION);
    }

    ~MapModeStasher()
    {
        mpRenderContext->Pop();
        mpRenderContext->EnableMapMode(mbIsMapModeEnabled);
        mpRenderContext->SetConnectMetaFile(mpMetaFile);
    }

private:
    VclPtr<OutputDevice> mpRenderContext;
    GDIMetaFile* mpMetaFile;
    bool mbIsMapModeEnabled;
};

bool WallpaperDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    long nX = maRect.Left();
    long nY = maRect.Top();
    long nWidth = maRect.GetWidth();
    long nHeight = maRect.GetHeight();

    if (mbUseScaffolding && maWallpaper.GetStyle() != WallpaperStyle::NONE)
    {
        tools::Rectangle aRect = pRenderContext->LogicToPixel(maRect);
        aRect.Justify();

        nX = aRect.Left();
        nY = aRect.Top();
        nWidth = aRect.GetWidth();
        nHeight = aRect.GetHeight();

        if (!aRect.IsEmpty())
        {
            return DrawWallpaper(pRenderContext, nX, nY, nWidth, nHeight, maWallpaper);
        }
    }
    else
    {
        return DrawWallpaper(pRenderContext, nX, nY, nWidth, nHeight, maWallpaper);
    }

    return false;
}

bool WallpaperDrawable::DrawWallpaper(OutputDevice* pRenderContext, long nX, long nY, long nWidth,
                                      long nHeight, const Wallpaper& rWallpaper)
{
    if (rWallpaper.IsBitmap())
        return DrawBitmapWallpaper(pRenderContext, nX, nY, nWidth, nHeight, rWallpaper);
    else if (rWallpaper.IsGradient())
        return DrawGradientWallpaper(pRenderContext, nX, nY, nWidth, nHeight, rWallpaper);
    else
        return DrawColorWallpaper(pRenderContext, nX, nY, nWidth, nHeight, rWallpaper);
}

bool WallpaperDrawable::DrawColorWallpaper(OutputDevice* pRenderContext, long nX, long nY,
                                           long nWidth, long nHeight,
                                           const Wallpaper& rWallpaper)
{
    // draw wallpaper without border
    Color aOldLineColor = pRenderContext->GetLineColor();
    Color aOldFillColor = pRenderContext->GetFillColor();
    pRenderContext->SetLineColor();
    pRenderContext->SetFillColor(rWallpaper.GetColor());

    bool bMap = pRenderContext->IsMapModeEnabled();
    pRenderContext->EnableMapMode(false);
    bool bRet = pRenderContext->Draw(
        RectangleDrawable(tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight))));
    pRenderContext->SetLineColor(aOldLineColor);
    pRenderContext->SetFillColor(aOldFillColor);
    pRenderContext->EnableMapMode(bMap);

    return bRet;
}

static bool IsCachedBitmap(Wallpaper const& rWallpaper)
{
    const BitmapEx* pCached = rWallpaper.ImplGetCachedBitmap();
    return pCached;
}

static BitmapEx GetWallpaperBitmap(Wallpaper const& rWallpaper)
{
    const BitmapEx* pCached = rWallpaper.ImplGetCachedBitmap();
    return (IsCachedBitmap(rWallpaper)) ? *pCached : rWallpaper.GetBitmap();
}

static bool UseColorBackground(Wallpaper const& rWallpaper)
{
    BitmapEx aBmpEx = GetWallpaperBitmap(rWallpaper);

    return ((aBmpEx.IsTransparent() && !rWallpaper.IsGradient())
            || ((rWallpaper.GetStyle() != WallpaperStyle::Tile
                 && rWallpaper.GetStyle() != WallpaperStyle::Scale)
                || (!rWallpaper.IsGradient())));
}

static Size CalculateWallpaperSize(OutputDevice* pRenderContext, Wallpaper const& rWallpaper,
                                   long nWidth, long nHeight)
{
    if (rWallpaper.IsRect())
    {
        const tools::Rectangle aBound(pRenderContext->LogicToPixel(rWallpaper.GetRect()));
        return aBound.GetSize();
    }

    return Size(nWidth, nHeight);
}

static Point CalculateWallpaperPos(OutputDevice* pRenderContext, Wallpaper const& rWallpaper)
{
    if (rWallpaper.IsRect())
    {
        const tools::Rectangle aBound(pRenderContext->LogicToPixel(rWallpaper.GetRect()));
        return aBound.TopLeft();
    }

    return Point(0, 0);
}

static void AdjustWallpaper(OutputDevice* pRenderContext, Wallpaper const& rWallpaper,
                            BitmapEx& rBitmap, Point& rPos, Size const& rSize)
{
    const long nBmpWidth = rBitmap.GetSizePixel().Width();
    const long nBmpHeight = rBitmap.GetSizePixel().Height();

    switch (rWallpaper.GetStyle())
    {
        case WallpaperStyle::Scale:
            if (!IsCachedBitmap(rWallpaper)
                || (IsCachedBitmap(rWallpaper) && rBitmap.GetSizePixel() != rSize))
            {
                if (IsCachedBitmap(rWallpaper))
                    rWallpaper.ImplReleaseCachedBitmap();

                rBitmap = rWallpaper.GetBitmap();
                rBitmap.Scale(rSize);
                rBitmap = BitmapEx(rBitmap.GetBitmap().CreateDisplayBitmap(pRenderContext),
                                   rBitmap.GetMask());
            }
            break;

        case WallpaperStyle::TopLeft:
            break;

        case WallpaperStyle::Top:
            rPos.AdjustX((rSize.Width() - nBmpWidth) >> 1);
            break;

        case WallpaperStyle::TopRight:
            rPos.AdjustX(rSize.Width() - nBmpWidth);
            break;

        case WallpaperStyle::Left:
            rPos.AdjustY((rSize.Height() - nBmpHeight) >> 1);
            break;

        case WallpaperStyle::Center:
            rPos.AdjustX((rSize.Width() - nBmpWidth) >> 1);
            rPos.AdjustY((rSize.Height() - nBmpHeight) >> 1);
            break;

        case WallpaperStyle::Right:
            rPos.AdjustX(rSize.Width() - nBmpWidth);
            rPos.AdjustY((rSize.Height() - nBmpHeight) >> 1);
            break;

        case WallpaperStyle::BottomLeft:
            rPos.AdjustY(rSize.Height() - nBmpHeight);
            break;

        case WallpaperStyle::Bottom:
            rPos.AdjustX((rSize.Width() - nBmpWidth) >> 1);
            rPos.AdjustY(rSize.Height() - nBmpHeight);
            break;

        case WallpaperStyle::BottomRight:
            rPos.AdjustX(rSize.Width() - nBmpWidth);
            rPos.AdjustY(rSize.Height() - nBmpHeight);
            break;

        default:
            break;
    }
}

static bool DrawBitmap(OutputDevice* pRenderContext, Wallpaper const& rWallpaper,
                       BitmapEx const& rBitmap, Point const& rPos, Size const& rSize, long nX,
                       long nY, long nWidth, long nHeight)
{
    const long nBmpWidth = rBitmap.GetSizePixel().Width();
    const long nBmpHeight = rBitmap.GetSizePixel().Height();

    switch (rWallpaper.GetStyle())
    {
        case WallpaperStyle::Scale:
        case WallpaperStyle::TopLeft:
        case WallpaperStyle::Top:
        case WallpaperStyle::TopRight:
        case WallpaperStyle::Left:
        case WallpaperStyle::Center:
        case WallpaperStyle::Right:
        case WallpaperStyle::BottomLeft:
        case WallpaperStyle::Bottom:
        case WallpaperStyle::BottomRight:
            return false;

        default:
        {
            const long nRight = nX + nWidth - 1;
            const long nBottom = nY + nHeight - 1;
            long nFirstX;
            long nFirstY;

            if (rWallpaper.GetStyle() == WallpaperStyle::Tile)
            {
                nFirstX = rPos.X();
                nFirstY = rPos.Y();
            }
            else
            {
                nFirstX = rPos.X() + ((rSize.Width() - nBmpWidth) >> 1);
                nFirstY = rPos.Y() + ((rSize.Height() - nBmpHeight) >> 1);
            }

            const long nOffX = (nFirstX - nX) % nBmpWidth;
            const long nOffY = (nFirstY - nY) % nBmpHeight;
            long nStartX = nX + nOffX;
            long nStartY = nY + nOffY;

            if (nOffX > 0)
                nStartX -= nBmpWidth;

            if (nOffY > 0)
                nStartY -= nBmpHeight;

            for (long nBmpY = nStartY; nBmpY <= nBottom; nBmpY += nBmpHeight)
            {
                for (long nBmpX = nStartX; nBmpX <= nRight; nBmpX += nBmpWidth)
                {
                    pRenderContext->DrawBitmapEx(Point(nBmpX, nBmpY), rBitmap);
                }
            }
            return true;
        }
        break;
    }
}

bool WallpaperDrawable::DrawBitmapWallpaper(OutputDevice* pRenderContext, long nX, long nY,
                                            long nWidth, long nHeight,
                                            const Wallpaper& rWallpaper)
{
    BitmapEx aBmpEx = GetWallpaperBitmap(rWallpaper);

    const long nBmpWidth = aBmpEx.GetSizePixel().Width();
    const long nBmpHeight = aBmpEx.GetSizePixel().Height();

    // draw background
    if (aBmpEx.IsTransparent())
    {
        if (!rWallpaper.IsGradient())
        {
            if (!IsCachedBitmap(rWallpaper) && !rWallpaper.GetColor().GetTransparency())
            {
                ScopedVclPtrInstance<VirtualDevice> aVDev(*pRenderContext);
                aVDev->SetBackground(rWallpaper.GetColor());
                aVDev->SetOutputSizePixel(Size(nBmpWidth, nBmpHeight));
                aVDev->DrawBitmapEx(Point(), aBmpEx);
                aBmpEx = aVDev->GetBitmapEx(Point(), aVDev->GetOutputSizePixel());
            }
        }
    }

    bool bIsColorBackgroundDrawn = false;

    // background of bitmap?
    if (rWallpaper.IsGradient())
    {
        DrawGradientWallpaper(pRenderContext, nX, nY, nWidth, nHeight, rWallpaper);
    }
    else if (UseColorBackground(rWallpaper) && aBmpEx.IsTransparent())
    {
        DrawColorWallpaper(pRenderContext, nX, nY, nWidth, nHeight, rWallpaper);
        bIsColorBackgroundDrawn = true;
    }

    Point aPos = CalculateWallpaperPos(pRenderContext, rWallpaper);
    Size aSize = CalculateWallpaperSize(pRenderContext, rWallpaper, nWidth, nHeight);

    MapModeStasher aStash(pRenderContext);
    pRenderContext->IntersectClipRegion(tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight)));
    AdjustWallpaper(pRenderContext, rWallpaper, aBmpEx, aPos, aSize);

    if (!DrawBitmap(pRenderContext, rWallpaper, aBmpEx, aPos, aSize, nX, nY, nWidth, nHeight))
    {
        // optimized for non-transparent bitmaps
        if (!bIsColorBackgroundDrawn)
        {
            const Size aBmpSize(aBmpEx.GetSizePixel());
            const tools::Rectangle aOutRect(Point(0, 0), pRenderContext->GetOutputSizePixel());
            const tools::Rectangle aColRect(Point(nX, nY), Size(nWidth, nHeight));

            tools::Rectangle aWorkRect(0, 0, aOutRect.Right(), aPos.Y() - 1);
            aWorkRect.Justify();
            aWorkRect.Intersection(aColRect);
            if (!aWorkRect.IsEmpty())
            {
                DrawColorWallpaper(pRenderContext, aWorkRect.Left(), aWorkRect.Top(),
                                   aWorkRect.GetWidth(), aWorkRect.GetHeight(), rWallpaper);
            }

            aWorkRect
                = tools::Rectangle(0, aPos.Y(), aPos.X() - 1, aPos.Y() + aBmpSize.Height() - 1);
            aWorkRect.Justify();
            aWorkRect.Intersection(aColRect);
            if (!aWorkRect.IsEmpty())
            {
                DrawColorWallpaper(pRenderContext, aWorkRect.Left(), aWorkRect.Top(),
                                   aWorkRect.GetWidth(), aWorkRect.GetHeight(), rWallpaper);
            }

            aWorkRect = tools::Rectangle(aPos.X() + aBmpSize.Width(), aPos.Y(), aOutRect.Right(),
                                         aPos.Y() + aBmpSize.Height() - 1);
            aWorkRect.Justify();
            aWorkRect.Intersection(aColRect);
            if (!aWorkRect.IsEmpty())
            {
                DrawColorWallpaper(pRenderContext, aWorkRect.Left(), aWorkRect.Top(),
                                   aWorkRect.GetWidth(), aWorkRect.GetHeight(), rWallpaper);
            }

            aWorkRect = tools::Rectangle(0, aPos.Y() + aBmpSize.Height(), aOutRect.Right(),
                                         aOutRect.Bottom());
            aWorkRect.Justify();
            aWorkRect.Intersection(aColRect);
            if (!aWorkRect.IsEmpty())
            {
                DrawColorWallpaper(pRenderContext, aWorkRect.Left(), aWorkRect.Top(),
                                   aWorkRect.GetWidth(), aWorkRect.GetHeight(), rWallpaper);
            }
        }

        pRenderContext->DrawBitmapEx(aPos, aBmpEx);
    }

    rWallpaper.ImplSetCachedBitmap(aBmpEx);

    return true;
}

bool WallpaperDrawable::DrawGradientWallpaper(OutputDevice* pRenderContext, long nX, long nY,
                                              long nWidth, long nHeight,
                                              const Wallpaper& rWallpaper)
{
    MapModeStasher aStash(pRenderContext);
    pRenderContext->IntersectClipRegion(tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight)));
    return pRenderContext->Draw(GradientDrawable(
        tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight)), rWallpaper.GetGradient()));
}

bool WallpaperDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary() || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
