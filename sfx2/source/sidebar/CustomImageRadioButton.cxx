/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "precompiled_sfx2.hxx"

#include "CustomImageRadioButton.hxx"

#include "DrawHelper.hxx"
#include "Paint.hxx"
#include "Tools.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sfx2 { namespace sidebar {


CustomImageRadioButton::CustomImageRadioButton (
    Window* pParentWindow,
    const ResId& rResId )
    : ImageRadioButton( pParentWindow, rResId )
{
    SetStyle( GetStyle() | WB_NOPOINTERFOCUS );
}


CustomImageRadioButton::~CustomImageRadioButton (void)
{
}


void CustomImageRadioButton::Paint (const Rectangle& rUpdateArea)
{
    Rectangle aPaintRect( Rectangle(Point(0,0), GetSizePixel() ) );
    SetMouseRect( aPaintRect );
    SetStateRect( aPaintRect );

    const Theme::ThemeItem eBackground =
        IsMouseOver()
          ? Theme::Paint_TabItemBackgroundHighlight
          : Theme::Paint_PanelBackground;
    DrawHelper::DrawRoundedRectangle(
        *this,
        aPaintRect,
        Theme::GetInteger(Theme::Int_ButtonCornerRadius),
        IsChecked() || IsMouseOver() ? Theme::GetColor(Theme::Color_TabItemBorder) : Color(0xffffffff),
        Theme::GetPaint( eBackground ) );

    const Image aIcon (GetModeRadioImage(Theme::IsHighContrastMode()
            ? BMP_COLOR_HIGHCONTRAST
            : BMP_COLOR_NORMAL));
    const Size aIconSize (aIcon.GetSizePixel());
    const Point aIconLocation(
        (GetSizePixel().Width() - aIconSize.Width())/2,
        (GetSizePixel().Height() - aIconSize.Height())/2 );
    DrawImage(
        aIconLocation,
        aIcon,
        IsEnabled() ? 0 : IMAGE_DRAW_DISABLE );
}


} } // end of namespace sfx2::sidebar
