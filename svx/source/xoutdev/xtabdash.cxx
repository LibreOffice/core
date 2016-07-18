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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <vcl/virdev.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>

#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <memory>

using namespace com::sun::star;

XDashList::XDashList(const OUString& rPath, const OUString& rReferer)
    : XPropertyList(XDASH_LIST, rPath, rReferer)
    , maBitmapSolidLine()
    , maStringSolidLine()
    , maStringNoLine()
{
}

XDashList::~XDashList()
{
}

XDashEntry* XDashList::Replace(XDashEntry* pEntry, long nIndex )
{
    return static_cast<XDashEntry*>( XPropertyList::Replace(pEntry, nIndex) );
}

XDashEntry* XDashList::Remove(long nIndex)
{
    return static_cast<XDashEntry*>( XPropertyList::Remove(nIndex) );
}

XDashEntry* XDashList::GetDash(long nIndex) const
{
    return static_cast<XDashEntry*>( XPropertyList::Get(nIndex) );
}

uno::Reference< container::XNameContainer > XDashList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
}

bool XDashList::Create()
{
    const OUString aStr(SVX_RESSTR(RID_SVXSTR_LINESTYLE));

    Insert(new XDashEntry(XDash(css::drawing::DashStyle_RECT,1, 50,1, 50, 50),aStr + " 1"));
    Insert(new XDashEntry(XDash(css::drawing::DashStyle_RECT,1,500,1,500,500),aStr + " 2"));
    Insert(new XDashEntry(XDash(css::drawing::DashStyle_RECT,2, 50,3,250,120),aStr + " 3"));

    return true;
}

Bitmap XDashList::ImpCreateBitmapForXDash(const XDash* pDash)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
    const sal_uInt32 nFactor(2);
    const Size aSize((rSize.Width() * 5 * 2) / 2, rSize.Height() * nFactor);

    // prepare polygon geometry for line
    basegfx::B2DPolygon aLine;

    aLine.append(basegfx::B2DPoint(0.0, aSize.Height() / 2.0));
    aLine.append(basegfx::B2DPoint(aSize.Width(), aSize.Height() / 2.0));

    // prepare LineAttribute
    const basegfx::BColor aLineColor(rStyleSettings.GetFieldTextColor().getBColor());
    const double fLineWidth(rStyleSettings.GetListBoxPreviewDefaultLineWidth() * (nFactor * 1.1));
    const drawinglayer::attribute::LineAttribute aLineAttribute(
        aLineColor,
        fLineWidth);

    // prepare StrokeAttribute
    ::std::vector< double > aDotDashArray;
    double fFullDotDashLen(0.0);

    if(pDash && (pDash->GetDots() || pDash->GetDashes()))
    {
        const basegfx::B2DHomMatrix aScaleMatrix(OutputDevice::LogicToLogic(MAP_100TH_MM, MAP_PIXEL));
        const basegfx::B2DVector aScaleVector(aScaleMatrix * basegfx::B2DVector(1.0, 0.0));
        const double fScaleValue(aScaleVector.getLength() * (nFactor * (1.4 / 2.0)));
        const double fLineWidthInUnits(fLineWidth / fScaleValue);

        fFullDotDashLen = pDash->CreateDotDashArray(aDotDashArray, fLineWidthInUnits);

        if(!aDotDashArray.empty())
        {
            for(double & a : aDotDashArray)
            {
                a *= fScaleValue;
            }

            fFullDotDashLen *= fScaleValue;
        }
    }

    const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(
        aDotDashArray,
        fFullDotDashLen);

    // create LinePrimitive
    const drawinglayer::primitive2d::Primitive2DReference aLinePrimitive(
        new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
            aLine,
            aLineAttribute,
            aStrokeAttribute));

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
        static const sal_uInt32 nLen(8 * nFactor);
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
        *pVirtualDevice.get(),
        aNewViewInformation2D));

    if(pProcessor2D)
    {
        const drawinglayer::primitive2d::Primitive2DContainer aSequence { aLinePrimitive };

        pProcessor2D->process(aSequence);
        pProcessor2D.reset();
    }

    // get result bitmap and scale
    Bitmap aRetval(pVirtualDevice->GetBitmap(Point(0, 0), pVirtualDevice->GetOutputSizePixel()));

    if(1 != nFactor)
    {
        aRetval.Scale(Size((rSize.Width() * 5) / 2, rSize.Height()));
    }

    return aRetval;
}

Bitmap XDashList::CreateBitmapForUI( long nIndex )
{
    const XDash& rDash = GetDash(nIndex)->GetDash();

    return ImpCreateBitmapForXDash(&rDash);
}

Bitmap const & XDashList::GetBitmapForUISolidLine() const
{
    if(maBitmapSolidLine.IsEmpty())
    {
        const_cast< XDashList* >(this)->maBitmapSolidLine = XDashList::ImpCreateBitmapForXDash(nullptr);
    }

    return maBitmapSolidLine;
}

OUString const & XDashList::GetStringForUiSolidLine() const
{
    if(maStringSolidLine.isEmpty())
    {
        const_cast< XDashList* >(this)->maStringSolidLine = ResId(RID_SVXSTR_SOLID, DIALOG_MGR()).toString();
    }

    return maStringSolidLine;
}

OUString const & XDashList::GetStringForUiNoLine() const
{
    if(maStringNoLine.isEmpty())
    {
        // formally was RID_SVXSTR_INVISIBLE, but to make equal
        // everywhere, use RID_SVXSTR_NONE
        const_cast< XDashList* >(this)->maStringNoLine = ResId(RID_SVXSTR_NONE, DIALOG_MGR()).toString();
    }

    return maStringNoLine;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
