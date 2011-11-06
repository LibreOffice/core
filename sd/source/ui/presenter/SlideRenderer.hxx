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
