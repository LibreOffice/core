/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideRenderer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:09:27 $
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

#ifndef SD_PRESENTER_SLIDE_PREVIEW_HXX
#define SD_PRESENTER_SLIDE_PREVIEW_HXX

#include "PreviewRenderer.hxx"
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XSlideRenderer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase2.hxx>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper2 <
        css::drawing::XSlideRenderer,
        css::lang::XInitialization
    > SlideRendererInterfaceBase;
}


/** Render single slides into bitmaps.
*/
class SlideRenderer
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public SlideRendererInterfaceBase
{
public:
    explicit SlideRenderer (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~SlideRenderer (void);
    virtual void SAL_CALL disposing (void);


    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // XSlideRenderer

    virtual css::uno::Reference<css::awt::XBitmap> SAL_CALL createPreview (
        const ::css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::rendering::XBitmap> SAL_CALL createPreviewForCanvas (
        const ::css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor,
        const ::css::uno::Reference<css::rendering::XCanvas>& rxCanvas)
        throw (css::uno::RuntimeException);

    virtual css::awt::Size SAL_CALL calculatePreviewSize (
        double nSlideAspectRatio,
        const css::awt::Size& rMaximumPreviewPixelSize)
        throw (css::uno::RuntimeException);

private:
    PreviewRenderer maPreviewRenderer;

    BitmapEx CreatePreview (
        const ::css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor)
        throw (css::uno::RuntimeException);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif
