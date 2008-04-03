/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterPaneBorderPainter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:01:58 $
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

#ifndef SDEXT_PRESENTER_PRESENTER_PANE_BORDER_PAINTER_HXX
#define SDEXT_PRESENTER_PRESENTER_PANE_BORDER_PAINTER_HXX

#include <vcl/bitmap.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/framework/XPaneBorderPainter.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace tools {
    class ConfigurationAccess;
} }

namespace sdext { namespace presenter {

class PresenterPane;
class PresenterTheme;

namespace {
    typedef ::cppu::WeakComponentImplHelper1<
        css::drawing::framework::XPaneBorderPainter
    > PresenterPaneBorderPainterInterfaceBase;
}

/** This class is responsible for painting window borders of PresenterPane
    objects.
*/
class PresenterPaneBorderPainter
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public PresenterPaneBorderPainterInterfaceBase
{
public:
    PresenterPaneBorderPainter (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterPaneBorderPainter (void);

    /** Transform the bounding box of the window content to the outer
        bounding box of the border that is painted around it.
        @param rsPaneURL
            Specifies the pane style that is used to determine the border sizes.
        @param rInnerBox
            The rectangle of the inner window content.
    */
    css::awt::Rectangle AddBorder (
        const ::rtl::OUString& rsPaneURL,
        const css::awt::Rectangle& rInnerBox,
        const css::drawing::framework::BorderType eBorderType) const;

    /** Transorm the outer bounding box of a window to the bounding box of
        the inner content area.
        @param rsPaneURL
            Specifies the pane style that is used to determine the border sizes.
        @param rOuterBox
            The bounding box of the rectangle around the window.
        @param bIsTitleVisible
            This flag controls whether the upper part of the frame is
            supposed to contain the window title.
    */
    css::awt::Rectangle RemoveBorder (
        const ::rtl::OUString& rsPaneURL,
        const css::awt::Rectangle& rOuterBox,
        const css::drawing::framework::BorderType eBorderType) const;

    bool HasTheme (void) const;

    void SetTheme (const ::boost::shared_ptr<PresenterTheme>& rpTheme);

    class Renderer;

    // XPaneBorderPainter

    virtual css::awt::Rectangle SAL_CALL addBorder (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::awt::Rectangle& rRectangle,
        css::drawing::framework::BorderType eBorderType)
        throw(css::uno::RuntimeException);

    virtual css::awt::Rectangle SAL_CALL removeBorder (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::awt::Rectangle& rRectangle,
        css::drawing::framework::BorderType eBorderType)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL paintBorder(
        const ::rtl::OUString& sPaneBorderStyleName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >& xCanvas,
        const ::com::sun::star::awt::Rectangle& aOuterBorderRectangle,
        const ::com::sun::star::awt::Rectangle& aRepaintArea,
        const ::rtl::OUString& sTitle )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL paintBorderWithCallout(
        const ::rtl::OUString& sPaneBorderStyleName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >& xCanvas,
        const ::com::sun::star::awt::Rectangle& aOuterBorderRectangle,
        const ::com::sun::star::awt::Rectangle& aRepaintArea,
        const ::rtl::OUString& sTitle,
        const ::com::sun::star::awt::Point& aCalloutAnchor )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getCalloutOffset(
        const ::rtl::OUString& sPaneBorderStyleName )
        throw (::com::sun::star::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    ::boost::shared_ptr<PresenterTheme> mpTheme;
    ::boost::scoped_ptr<Renderer> mpRenderer;

    /** When the theme for the border has not yet been loaded then try again
        when this method is called.
        @return
            Returns <TRUE/> only one time when the theme is loaded and/or the
            renderer is initialized.
    */
    bool ProvideTheme (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);
    bool ProvideTheme (void);

    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif
