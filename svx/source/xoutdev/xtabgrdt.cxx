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

#include "svx/XPropertyTable.hxx"

#include <vcl/virdev.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <memory>

using namespace com::sun::star;

XGradientList::XGradientList( const OUString& rPath, const OUString& rReferer )
:   XPropertyList( XGRADIENT_LIST, rPath, rReferer )
{
}

XGradientList::~XGradientList()
{
}

XGradientEntry* XGradientList::Replace(XGradientEntry* pEntry, long nIndex )
{
    return static_cast<XGradientEntry*>( XPropertyList::Replace( pEntry, nIndex ) );
}

XGradientEntry* XGradientList::Remove(long nIndex)
{
    return static_cast<XGradientEntry*>( XPropertyList::Remove( nIndex ) );
}

XGradientEntry* XGradientList::GetGradient(long nIndex) const
{
    return static_cast<XGradientEntry*>( XPropertyList::Get( nIndex ) );
}

uno::Reference< container::XNameContainer > XGradientList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXGradientTable_createInstance( this ),
        uno::UNO_QUERY );
}

bool XGradientList::Create()
{
    rtl::OUStringBuffer aStr(SVX_RESSTR(RID_SVXSTR_GRADIENT));
    aStr.append(" 1");
    sal_Int32 nLen = aStr.getLength() - 1;
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLACK  ),RGB_Color(COL_WHITE  ),css::awt::GradientStyle_LINEAR    ,    0,10,10, 0,100,100),aStr.toString()));
    aStr[nLen] = '2';
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLUE   ),RGB_Color(COL_RED    ),css::awt::GradientStyle_AXIAL     ,  300,20,20,10,100,100),aStr.toString()));
    aStr[nLen] = '3';
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_RED    ),RGB_Color(COL_YELLOW ),css::awt::GradientStyle_RADIAL    ,  600,30,30,20,100,100),aStr.toString()));
    aStr[nLen] = '4';
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_YELLOW ),RGB_Color(COL_GREEN  ),css::awt::GradientStyle_ELLIPTICAL,  900,40,40,30,100,100),aStr.toString()));
    aStr[nLen] = '5';
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_GREEN  ),RGB_Color(COL_MAGENTA),css::awt::GradientStyle_SQUARE    , 1200,50,50,40,100,100),aStr.toString()));
    aStr[nLen] = '6';
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_MAGENTA),RGB_Color(COL_YELLOW ),css::awt::GradientStyle_RECT      , 1900,60,60,50,100,100),aStr.toString()));

    return true;
}

Bitmap XGradientList::CreateBitmapForUI( long nIndex )
{
    Bitmap aRetval;

    OSL_ENSURE(nIndex < Count(), "OOps, access out of range (!)");

    if(nIndex < Count())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();

        // prepare polygon geometry for rectangle
        const basegfx::B2DPolygon aRectangle(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRange(0.0, 0.0, rSize.Width(), rSize.Height())));

        const XGradient& rGradient = GetGradient(nIndex)->GetGradient();
        const sal_uInt16 nStartIntens(rGradient.GetStartIntens());
        basegfx::BColor aStart(rGradient.GetStartColor().getBColor());

        if(nStartIntens != 100)
        {
            const basegfx::BColor aBlack;
            aStart = interpolate(aBlack, aStart, (double)nStartIntens * 0.01);
        }

        const sal_uInt16 nEndIntens(rGradient.GetEndIntens());
        basegfx::BColor aEnd(rGradient.GetEndColor().getBColor());

        if(nEndIntens != 100)
        {
            const basegfx::BColor aBlack;
            aEnd = interpolate(aBlack, aEnd, (double)nEndIntens * 0.01);
        }

        drawinglayer::attribute::GradientStyle aGradientStyle(drawinglayer::attribute::GRADIENTSTYLE_RECT);

        switch(rGradient.GetGradientStyle())
        {
            case css::awt::GradientStyle_LINEAR :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_LINEAR;
                break;
            }
            case css::awt::GradientStyle_AXIAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_AXIAL;
                break;
            }
            case css::awt::GradientStyle_RADIAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_RADIAL;
                break;
            }
            case css::awt::GradientStyle_ELLIPTICAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_ELLIPTICAL;
                break;
            }
            case css::awt::GradientStyle_SQUARE :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_SQUARE;
                break;
            }
            default :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_RECT; // css::awt::GradientStyle_RECT
                break;
            }
        }

        const sal_uInt16 nSteps((rSize.Width() + rSize.Height()) / 3);
        const drawinglayer::attribute::FillGradientAttribute aFillGradient(
            aGradientStyle,
            (double)rGradient.GetBorder() * 0.01,
            (double)rGradient.GetXOffset() * 0.01,
            (double)rGradient.GetYOffset() * 0.01,
            (double)rGradient.GetAngle() * F_PI1800,
            aStart,
            aEnd,
            nSteps);

        const drawinglayer::primitive2d::Primitive2DReference aGradientPrimitive(
            new drawinglayer::primitive2d::PolyPolygonGradientPrimitive2D(
                basegfx::B2DPolyPolygon(aRectangle),
                aFillGradient));

        const basegfx::BColor aBlack(0.0, 0.0, 0.0);
        const drawinglayer::primitive2d::Primitive2DReference aBlackRectanglePrimitive(
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                aRectangle,
                aBlack));

        // prepare VirtualDevice
        ScopedVclPtrInstance< VirtualDevice > pVirtualDevice;
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;

        pVirtualDevice->SetOutputSizePixel(rSize);
        pVirtualDevice->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient
            : DrawModeFlags::Default);

        // create processor and draw primitives
        std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
            *pVirtualDevice.get(),
            aNewViewInformation2D));

        if(pProcessor2D)
        {
            drawinglayer::primitive2d::Primitive2DContainer aSequence(2);

            aSequence[0] = aGradientPrimitive;
            aSequence[1] = aBlackRectanglePrimitive;

            pProcessor2D->process(aSequence);
            pProcessor2D.reset();
        }

        // get result bitmap and scale
        aRetval = pVirtualDevice->GetBitmap(Point(0, 0), pVirtualDevice->GetOutputSizePixel());
    }

    return aRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
