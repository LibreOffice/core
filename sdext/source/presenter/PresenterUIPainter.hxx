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



#ifndef SDEXT_PRESENTER_PRESENTER_UI_PAINTER_HXX
#define SDEXT_PRESENTER_PRESENTER_UI_PAINTER_HXX

#include "PresenterTheme.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {


/** Functions for painting UI elements.
*/
class PresenterUIPainter
    : ::boost::noncopyable
{
public:
    PresenterUIPainter (void);
    ~PresenterUIPainter (void);

    static void PaintHorizontalBitmapComposite (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rRepaintBox,
        const css::awt::Rectangle& rBoundingBox,
        const css::uno::Reference<css::rendering::XBitmap>& rxLeftBitmap,
        const css::uno::Reference<css::rendering::XBitmap>& rxRepeatableCenterBitmap,
        const css::uno::Reference<css::rendering::XBitmap>& rxRightBitmap);

    static void PaintVerticalBitmapComposite (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rRepaintBox,
        const css::awt::Rectangle& rBoundingBox,
        const css::uno::Reference<css::rendering::XBitmap>& rxTopBitmap,
        const css::uno::Reference<css::rendering::XBitmap>& rxRepeatableCenterBitmap,
        const css::uno::Reference<css::rendering::XBitmap>& rxBottomBitmap);
};

} }

#endif
