/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterPaneAnimator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:00:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
