/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DashedLine.hxx>

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <memory>

SwDashedLine::SwDashedLine( vcl::Window* pParent, Color& ( *pColorFn )() )
    : Control( pParent, WB_DIALOGCONTROL | WB_HORZ )
    , m_pColorFn( pColorFn )
{
}

SwDashedLine::~SwDashedLine( )
{
}

void SwDashedLine::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));

    // Compute the start and end points
    const tools::Rectangle aRect(tools::Rectangle(Point(0, 0), rRenderContext.PixelToLogic(GetSizePixel())));
    double nHalfWidth = double(aRect.Top() + aRect.Bottom()) / 2.0;

    basegfx::B2DPoint aStart(double(aRect.Left()), nHalfWidth);
    basegfx::B2DPoint aEnd(double(aRect.Right()), nHalfWidth);

    basegfx::B2DPolygon aPolygon;
    aPolygon.append(aStart);
    aPolygon.append(aEnd);

    drawinglayer::primitive2d::Primitive2DContainer aSeq(1);

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    std::vector<double> aStrokePattern;
    basegfx::BColor aColor = m_pColorFn().getBColor();
    if (rSettings.GetHighContrastMode())
    {
        // Only a solid line in high contrast mode
        aColor = rSettings.GetDialogTextColor().getBColor();
    }
    else
    {
        // Get a color for the contrast
        basegfx::BColor aHslLine = basegfx::utils::rgb2hsl(aColor);
        double nLuminance = aHslLine.getZ();
        nLuminance += (1.0 - nLuminance) * 0.75;
        if (aHslLine.getZ() > 0.7)
            nLuminance = aHslLine.getZ() * 0.7;
        aHslLine.setZ(nLuminance);
        const basegfx::BColor aOtherColor = basegfx::utils::hsl2rgb(aHslLine);

        // Compute the plain line
        aSeq[0] = new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aPolygon, aOtherColor);

        // Dashed line in twips
        aStrokePattern.push_back(3);
        aStrokePattern.push_back(3);

        aSeq.resize(2);
    }

    // Compute the dashed line primitive
    aSeq[aSeq.size() - 1] =
            new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D(
                basegfx::B2DPolyPolygon(aPolygon),
                drawinglayer::attribute::LineAttribute(m_pColorFn().getBColor()),
                drawinglayer::attribute::StrokeAttribute(std::move(aStrokePattern)));

    pProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
