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



#ifndef SDEXT_PRESENTER_PRESENTER_PANE_ANIMATOR_HXXs
#define SDEXT_PRESENTER_PRESENTER_PANE_ANIMATOR_HXX

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XSprite.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ref.hxx>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterController;
class PresenterPaneContainer;
class PresenterWindowManager;

/** Base class for different types of pane animations.  Each of these
    animations either shows or hides a single pane.
*/
class PresenterPaneAnimator
    : private ::boost::noncopyable
{
public:
    virtual void ShowPane (void) = 0;
    virtual void HidePane (void) = 0;

protected:
    virtual ~PresenterPaneAnimator (void) {};
};


typedef ::std::vector< ::boost::function<void()> > EndActions;

::boost::shared_ptr<PresenterPaneAnimator> CreateUnfoldInCenterAnimator (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const bool bAnimate,
        const EndActions& rShowEndActions,
        const EndActions& rEndEndActions);

::boost::shared_ptr<PresenterPaneAnimator> CreateMoveInFromBottomAnimator (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const bool bAnimate,
        const EndActions& rShowEndActions,
        const EndActions& rEndEndActions);

::boost::shared_ptr<PresenterPaneAnimator> CreateTransparentOverlay (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const bool bAnimate,
        const EndActions& rShowEndActions,
        const EndActions& rEndEndActions);

} }

#endif
