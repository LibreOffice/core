/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
