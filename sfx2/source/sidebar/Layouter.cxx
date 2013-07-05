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
#include "sfx2/sidebar/Layouter.hxx"

#include <vcl/window.hxx>
#include <vcl/fixed.hxx>
#include <vcl/outdev.hxx>

namespace sfx2 { namespace sidebar {

void Layouter::EnlargeControlHorizontally (
    Window& rControl,
    const sal_Int32 nDeltaX)
{
    Size aSize (rControl.GetSizePixel());
    aSize.Width() += nDeltaX;
    rControl.SetSizePixel(aSize);

}




void Layouter::SetWidth (
    Window& rControl,
    const sal_Int32 nWidth)
{
    rControl.SetPosSizePixel(
        0,0,
        nWidth,0,
        WINDOW_POSSIZE_WIDTH);
}




void Layouter::SetRight (
    Window& rControl,
    const sal_Int32 nRight)
{
    rControl.SetPosSizePixel(
        0,0,
        nRight-rControl.GetPosPixel().X(),0,
        WINDOW_POSSIZE_WIDTH);
}




void Layouter::MoveControlHorizontally (
    Window& rControl,
    const sal_Int32 nDeltaX)
{
    Point aPosition (rControl.GetPosPixel());
    aPosition.Move(nDeltaX, 0);
    rControl.SetPosPixel(aPosition);
}




void Layouter::SetHorizontalPosition (
    Window& rControl,
    const sal_Int32 nX)
{
    rControl.SetPosPixel(Point(nX, rControl.GetPosPixel().Y()));
}




void Layouter::PrepareForLayouting (
    Window& rControl)
{
    //    rControl.SetStyle(rControl.GetStyle() | WB_PATHELLIPSIS | WB_INFO);
}




sal_Int32 Layouter::MapX (
    const Window& rControl,
    const sal_Int32 nValue)
{
    return rControl.LogicToPixel(Point(nValue,0), MAP_APPFONT).X();
}




sal_Int32 Layouter::MapWidth (
    const Window& rControl,
    const sal_Int32 nValue)
{
    return rControl.LogicToPixel(Point(nValue,0), MAP_APPFONT).X();
}

} } // end of namespace sfx2::sidebar
