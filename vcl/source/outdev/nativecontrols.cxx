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

#include <cassert>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <sal/log.hxx>

#include <vcl/salnativewidgets.hxx>
#include <vcl/pdfextoutdevdata.hxx>

#include <salgdi.hxx>

ImplControlValue::~ImplControlValue()
{
}

ImplControlValue* ImplControlValue::clone() const
{
    assert( typeid( const ImplControlValue ) == typeid( *this ));
    return new ImplControlValue( *this );
}

ScrollbarValue::~ScrollbarValue()
{
}

ScrollbarValue* ScrollbarValue::clone() const
{
    assert( typeid( const ScrollbarValue ) == typeid( *this ));
    return new ScrollbarValue( *this );
}

SliderValue::~SliderValue()
{
}

SliderValue* SliderValue::clone() const
{
    assert( typeid( const SliderValue ) == typeid( *this ));
    return new SliderValue( *this );
}

int TabPaneValue::m_nOverlap = 0;

TabPaneValue* TabPaneValue::clone() const
{
    assert(typeid(const TabPaneValue) == typeid(*this));
    return new TabPaneValue(*this);
}

TabitemValue::~TabitemValue()
{
}

TabitemValue* TabitemValue::clone() const
{
    assert( typeid( const TabitemValue ) == typeid( *this ));
    return new TabitemValue( *this );
}

SpinbuttonValue::~SpinbuttonValue()
{
}

SpinbuttonValue* SpinbuttonValue::clone() const
{
    assert( typeid( const SpinbuttonValue ) == typeid( *this ));
    return new SpinbuttonValue( *this );
}

ToolbarValue::~ToolbarValue()
{
}

ToolbarValue* ToolbarValue::clone() const
{
    assert( typeid( const ToolbarValue ) == typeid( *this ));
    return new ToolbarValue( *this );
}

MenubarValue::~MenubarValue()
{
}

MenubarValue* MenubarValue::clone() const
{
    assert( typeid( const MenubarValue ) == typeid( *this ));
    return new MenubarValue( *this );
}

MenupopupValue::~MenupopupValue()
{
}

MenupopupValue* MenupopupValue::clone() const
{
    assert( typeid( const MenupopupValue ) == typeid( *this ));
    return new MenupopupValue( *this );
}

PushButtonValue::~PushButtonValue()
{
}

PushButtonValue* PushButtonValue::clone() const
{
    assert( typeid( const PushButtonValue ) == typeid( *this ));
    return new PushButtonValue( *this );
}

// These functions are mainly passthrough functions that allow access to
// the SalFrame behind a Window object for native widget rendering purposes.

bool OutputDevice::IsNativeControlSupported( ControlType nType, ControlPart nPart ) const
{
    if( !CanEnableNativeWidget() )
        return false;

    if ( !mpGraphics && !AcquireGraphics() )
        return false;
    assert(mpGraphics);

    return mpGraphics->IsNativeControlSupported(nType, nPart);
}

bool OutputDevice::HitTestNativeScrollbar(
                              ControlPart nPart,
                              const tools::Rectangle& rControlRegion,
                              const Point& aPos,
                              bool& rIsInside ) const
{
    if( !CanEnableNativeWidget() )
        return false;

    if ( !mpGraphics && !AcquireGraphics() )
        return false;
    assert(mpGraphics);

    Point aWinOffs( mnOutOffX, mnOutOffY );
    tools::Rectangle screenRegion( rControlRegion );
    screenRegion.Move( aWinOffs.X(), aWinOffs.Y());

    return mpGraphics->HitTestNativeScrollbar( nPart, screenRegion, Point( aPos.X() + mnOutOffX, aPos.Y() + mnOutOffY ),
        rIsInside, *this );
}

static std::unique_ptr< ImplControlValue > TransformControlValue( const ImplControlValue& rVal, const OutputDevice& rDev )
{
    std::unique_ptr< ImplControlValue > aResult;
    switch( rVal.getType() )
    {
    case ControlType::Slider:
        {
            const SliderValue* pSlVal = static_cast<const SliderValue*>(&rVal);
            SliderValue* pNew = new SliderValue( *pSlVal );
            aResult.reset( pNew );
            pNew->maThumbRect = rDev.ImplLogicToDevicePixel( pSlVal->maThumbRect );
        }
        break;
    case ControlType::Scrollbar:
        {
            const ScrollbarValue* pScVal = static_cast<const ScrollbarValue*>(&rVal);
            ScrollbarValue* pNew = new ScrollbarValue( *pScVal );
            aResult.reset( pNew );
            pNew->maThumbRect = rDev.ImplLogicToDevicePixel( pScVal->maThumbRect );
            pNew->maButton1Rect = rDev.ImplLogicToDevicePixel( pScVal->maButton1Rect );
            pNew->maButton2Rect = rDev.ImplLogicToDevicePixel( pScVal->maButton2Rect );
        }
        break;
    case ControlType::SpinButtons:
        {
            const SpinbuttonValue* pSpVal = static_cast<const SpinbuttonValue*>(&rVal);
            SpinbuttonValue* pNew = new SpinbuttonValue( *pSpVal );
            aResult.reset( pNew );
            pNew->maUpperRect = rDev.ImplLogicToDevicePixel( pSpVal->maUpperRect );
            pNew->maLowerRect = rDev.ImplLogicToDevicePixel( pSpVal->maLowerRect );
        }
        break;
    case ControlType::Toolbar:
        {
            const ToolbarValue* pTVal = static_cast<const ToolbarValue*>(&rVal);
            ToolbarValue* pNew = new ToolbarValue( *pTVal );
            aResult.reset( pNew );
            pNew->maGripRect = rDev.ImplLogicToDevicePixel( pTVal->maGripRect );
        }
        break;
    case ControlType::TabPane:
        {
            const TabPaneValue* pTIVal = static_cast<const TabPaneValue*>(&rVal);
            TabPaneValue* pNew = new TabPaneValue(*pTIVal);
            pNew->m_aTabHeaderRect = rDev.ImplLogicToDevicePixel(pTIVal->m_aTabHeaderRect);
            pNew->m_aSelectedTabRect = rDev.ImplLogicToDevicePixel(pTIVal->m_aSelectedTabRect);
            aResult.reset(pNew);
        }
        break;
    case ControlType::TabItem:
        {
            const TabitemValue* pTIVal = static_cast<const TabitemValue*>(&rVal);
            TabitemValue* pNew = new TabitemValue( *pTIVal );
            pNew->maContentRect = rDev.ImplLogicToDevicePixel(pTIVal->maContentRect);
            aResult.reset( pNew );
        }
        break;
    case ControlType::Menubar:
        {
            const MenubarValue* pMVal = static_cast<const MenubarValue*>(&rVal);
            MenubarValue* pNew = new MenubarValue( *pMVal );
            aResult.reset( pNew );
        }
        break;
    case ControlType::Pushbutton:
        {
            const PushButtonValue* pBVal = static_cast<const PushButtonValue*>(&rVal);
            PushButtonValue* pNew = new PushButtonValue( *pBVal );
            aResult.reset( pNew );
        }
        break;
    case ControlType::Generic:
            aResult = std::make_unique<ImplControlValue>( rVal );
            break;
    case ControlType::MenuPopup:
        {
            const MenupopupValue* pMVal = static_cast<const MenupopupValue*>(&rVal);
            MenupopupValue* pNew = new MenupopupValue( *pMVal );
            pNew->maItemRect = rDev.ImplLogicToDevicePixel( pMVal->maItemRect );
            aResult.reset( pNew );
        }
        break;
    default:
        std::abort();
        break;
    }
    return aResult;
}
bool OutputDevice::DrawNativeControl( ControlType nType,
                            ControlPart nPart,
                            const tools::Rectangle& rControlRegion,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            const OUString& aCaption,
                            const Color& rBackgroundColor )
{
    assert(!is_double_buffered_window());

    if( !CanEnableNativeWidget() )
        return false;

    // make sure the current clip region is initialized correctly
    if ( !mpGraphics && !AcquireGraphics() )
        return false;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();
    if ( mbOutputClipped )
        return true;

    if ( mbInitLineColor )
        InitLineColor();
    if ( mbInitFillColor )
        InitFillColor();

    // Convert the coordinates from relative to Window-absolute, so we draw
    // in the correct place in platform code
    std::unique_ptr< ImplControlValue > aScreenCtrlValue( TransformControlValue( aValue, *this ) );
    tools::Rectangle screenRegion( ImplLogicToDevicePixel( rControlRegion ) );

    bool bRet = mpGraphics->DrawNativeControl(nType, nPart, screenRegion, nState, *aScreenCtrlValue, aCaption, *this, rBackgroundColor);

    return bRet;
}

bool OutputDevice::GetNativeControlRegion(  ControlType nType,
                                ControlPart nPart,
                                const tools::Rectangle& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& aValue,
                                tools::Rectangle &rNativeBoundingRegion,
                                tools::Rectangle &rNativeContentRegion ) const
{
    if( !CanEnableNativeWidget() )
        return false;

    if ( !mpGraphics && !AcquireGraphics() )
        return false;
    assert(mpGraphics);

    // Convert the coordinates from relative to Window-absolute, so we draw
    // in the correct place in platform code
    std::unique_ptr< ImplControlValue > aScreenCtrlValue( TransformControlValue( aValue, *this ) );
    tools::Rectangle screenRegion( ImplLogicToDevicePixel( rControlRegion ) );

    bool bRet = mpGraphics->GetNativeControlRegion(nType, nPart, screenRegion, nState, *aScreenCtrlValue,
                                rNativeBoundingRegion,
                                rNativeContentRegion, *this );
    if( bRet )
    {
        // transform back native regions
        rNativeBoundingRegion = ImplDevicePixelToLogic( rNativeBoundingRegion );
        rNativeContentRegion = ImplDevicePixelToLogic( rNativeContentRegion );
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
