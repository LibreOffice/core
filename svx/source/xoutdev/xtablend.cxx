/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/XPropertyTable.hxx>
#include <vcl/virdev.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <svx/xtable.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <memory>

using namespace com::sun::star;

XLineEndList::XLineEndList( const OUString& rPath, const OUString& rReferer )
    : XPropertyList( XPropertyListType::LineEnd, rPath, rReferer )
{
}

XLineEndList::~XLineEndList()
{
}

XLineEndEntry* XLineEndList::GetLineEnd(long nIndex) const
{
    return static_cast<XLineEndEntry*>( XPropertyList::Get(nIndex) );
}

uno::Reference< container::XNameContainer > XLineEndList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
}

bool XLineEndList::Create()
{
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
    aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
    aTriangle.setClosed(true);
    Insert( std::make_unique<XLineEndEntry>( basegfx::B2DPolyPolygon(aTriangle), SvxResId( RID_SVXSTR_ARROW ) ) );

    basegfx::B2DPolygon aSquare;
    aSquare.append(basegfx::B2DPoint(0.0, 0.0));
    aSquare.append(basegfx::B2DPoint(10.0, 0.0));
    aSquare.append(basegfx::B2DPoint(10.0, 10.0));
    aSquare.append(basegfx::B2DPoint(0.0, 10.0));
    aSquare.setClosed(true);
    Insert( std::make_unique<XLineEndEntry>( basegfx::B2DPolyPolygon(aSquare), SvxResId( RID_SVXSTR_SQUARE ) ) );

    basegfx::B2DPolygon aCircle(basegfx::utils::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 100.0));
    Insert( std::make_unique<XLineEndEntry>( basegfx::B2DPolyPolygon(aCircle), SvxResId( RID_SVXSTR_CIRCLE ) ) );

    return true;
}

BitmapEx XLineEndList::CreateBitmapForUI( long nIndex )
{
    BitmapEx aRetval;
    OSL_ENSURE(nIndex < Count(), "OOps, access out of range (!)");

    if(nIndex < Count())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();

        const Size aSize(rSize.Width() * 2, rSize.Height());

        // prepare line geometry
        basegfx::B2DPolygon aLine;
        const double fBorderDistance(aSize.Height() * 0.1);

        aLine.append(basegfx::B2DPoint(fBorderDistance, aSize.Height() / 2));
        aLine.append(basegfx::B2DPoint(aSize.Width() - fBorderDistance, aSize.Height() / 2));

        // prepare LineAttribute
        const basegfx::BColor aLineColor(rStyleSettings.GetFieldTextColor().getBColor());
        const double fLineWidth(StyleSettings::GetListBoxPreviewDefaultLineWidth() * 1.1);
        const drawinglayer::attribute::LineAttribute aLineAttribute(
            aLineColor,
            fLineWidth);

        const basegfx::B2DPolyPolygon aLineEnd(GetLineEnd(nIndex)->GetLineEnd());
        const double fArrowHeight(aSize.Height() - (2.0 * fBorderDistance));
        const drawinglayer::attribute::LineStartEndAttribute aLineStartEndAttribute(
            fArrowHeight,
            aLineEnd,
            false);

        // prepare line primitive
        const drawinglayer::primitive2d::Primitive2DReference aLineStartEndPrimitive(
            new drawinglayer::primitive2d::PolygonStrokeArrowPrimitive2D(
                aLine,
                aLineAttribute,
                aLineStartEndAttribute,
                aLineStartEndAttribute));

        // prepare VirtualDevice
        ScopedVclPtrInstance< VirtualDevice > pVirtualDevice;
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;

        pVirtualDevice->SetOutputSizePixel(aSize);
        pVirtualDevice->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient
            : DrawModeFlags::Default);

        if(rStyleSettings.GetPreviewUsesCheckeredBackground())
        {
            const Point aNull(0, 0);
            static const sal_uInt32 nLen(8);
            static const Color aW(COL_WHITE);
            static const Color aG(0xef, 0xef, 0xef);
            pVirtualDevice->DrawCheckered(aNull, aSize, nLen, aW, aG);
        }
        else
        {
            pVirtualDevice->SetBackground(rStyleSettings.GetFieldColor());
            pVirtualDevice->Erase();
        }

        // create processor and draw primitives
        std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
            *pVirtualDevice,
            aNewViewInformation2D));

        if(pProcessor2D)
        {
            const drawinglayer::primitive2d::Primitive2DContainer aSequence { aLineStartEndPrimitive };

            pProcessor2D->process(aSequence);
            pProcessor2D.reset();
        }

        // get result bitmap and scale
        aRetval = pVirtualDevice->GetBitmapEx(Point(0, 0), pVirtualDevice->GetOutputSizePixel());
    }

    return aRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
