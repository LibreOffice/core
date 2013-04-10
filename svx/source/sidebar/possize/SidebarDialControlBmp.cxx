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

#include "SidebarDialControlBmp.hxx"

#include <sfx2/sidebar/Theme.hxx>

#include <vcl/svapp.hxx>

#include "PosSizePropertyPanel.hrc"
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/svdstr.hrc>

namespace svx { namespace sidebar {

SidebarDialControlBmp::SidebarDialControlBmp (Window& rParent)
    : DialControlBmp(rParent)
{
}




SidebarDialControlBmp::~SidebarDialControlBmp (void)
{
}




void SidebarDialControlBmp::DrawElements (
    const String& /*rText*/,
    sal_Int32 nAngle)
{
    if (Application::GetSettings().GetLayoutRTL())
        nAngle = 18000 - nAngle;
    double fAngle = nAngle * F_PI180 / 100.0;
    double fSin = sin( fAngle );
    double fCos = cos( fAngle );
    DrawText( maRect, String(), mbEnabled ? 0 : TEXT_DRAW_DISABLE );
    const sal_Int32 nDx (fCos * (maRect.GetWidth()-4) / 2);
    const sal_Int32 nDy (-fSin * (maRect.GetHeight()-4) / 2);
    Point pt1( maRect.Center() );
    Point pt2( pt1.X() + nDx, pt1.Y() + nDy);
    if ( ! sfx2::sidebar::Theme::IsHighContrastMode())
        SetLineColor( Color( 60, 93, 138 ) );
    else
        SetLineColor(COL_BLACK);//Application::GetSettings().GetStyleSettings().GetFieldTextColor()
    DrawLine( pt1, pt2 );
}




void SidebarDialControlBmp::DrawBackground()
{
    SetLineColor();
    SetFillColor(sfx2::sidebar::Theme::GetColor(sfx2::sidebar::Theme::Paint_PanelBackground));
    DrawRect(maRect);

    const BitmapEx aBitmapEx(
        sfx2::sidebar::Theme::IsHighContrastMode()
            ? SVX_RES(IMG_DIACONTROL_H)
            : SVX_RES(IMG_DIACONTROL_NORMAL));
    // Size aImageSize(aBitmapEx.GetSizePixel());
    //    aImageSize.Width() -= 1;
    //    aImageSize.Height() -= 1;
    SetAntialiasing(ANTIALIASING_ENABLE_B2DDRAW |  ANTIALIASING_PIXELSNAPHAIRLINE);
    DrawBitmapEx(maRect.TopLeft(), /*aImageSize,*/ aBitmapEx);
}


} } // end of namespace svx::sidebar

// eof
