/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterSpritePane.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:05:55 $
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

#ifndef SD_PRESENTER_PRESENTER_SPRITE_PANE_HXX
#define SD_PRESENTER_PRESENTER_SPRITE_PANE_HXX

#include "PresenterPaneBase.hxx"
#include "PresenterSprite.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XPaneBorderPainter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <rtl/ref.hxx>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;


namespace sdext { namespace presenter {

/** Use a sprite to display the contents and the border of a pane.  Windows
    are still used to define the locations and sizes of both the border and
    the pane content.  Note that every resize results in a disposed canvas.
    Therefore call getCanvas in every repaint or at least after every resize.
*/
class PresenterSpritePane : public PresenterPaneBase
{
public:
    PresenterSpritePane (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterSpritePane (void);

    virtual void SAL_CALL disposing (void);

    using css::lang::XEventListener::disposing;

    ::boost::shared_ptr<PresenterSprite> GetSprite (void);

    static ::rtl::OUString getImplementationName_static (void);
    static css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static (void);
    static css::uno::Reference<css::uno::XInterface> Create(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        SAL_THROW((css::uno::Exception));

    void ShowTransparentBorder (void);

    // XPane

    css::uno::Reference<css::awt::XWindow> SAL_CALL getWindow (void)
        throw (css::uno::RuntimeException);

    css::uno::Reference<css::rendering::XCanvas> SAL_CALL getCanvas (void)
        throw (css::uno::RuntimeException);


    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException);


private:
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::rendering::XSpriteCanvas> mxParentCanvas;
    ::boost::shared_ptr<PresenterSprite> mpSprite;

    virtual void CreateCanvases (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxParentCanvas);
    void UpdateCanvases (void);
};

} } // end of namespace ::sd::presenter

#endif
