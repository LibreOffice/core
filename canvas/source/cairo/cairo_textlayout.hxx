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



#ifndef _CAIROCANVAS_TEXTLAYOUT_HXX
#define _CAIROCANVAS_TEXTLAYOUT_HXX

#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XTextLayout.hpp>

#include <basegfx/vector/b2isize.hxx>

#include <vcl/outdev.hxx>

#include <boost/utility.hpp>

#include "cairo_cairo.hxx"
#include "cairo_canvasfont.hxx"


/* Definition of TextLayout class */

namespace cairocanvas
{
    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XTextLayout,
                                               ::com::sun::star::lang::XServiceInfo > TextLayout_Base;

    class TextLayout : public ::comphelper::OBaseMutex,
                       public TextLayout_Base,
                       private ::boost::noncopyable
    {
    public:
        TextLayout( const ::com::sun::star::rendering::StringContext& aText,
                    sal_Int8                                          nDirection,
                    sal_Int64                                         nRandomSeed,
                    const CanvasFont::Reference&                      rFont,
                    const SurfaceProviderRef&                         rRefDevice );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XTextLayout
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > > SAL_CALL queryTextShapes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealRectangle2D > SAL_CALL queryInkMeasures(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealRectangle2D > SAL_CALL queryMeasures(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL queryLogicalAdvancements(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL applyLogicalAdvancements( const ::com::sun::star::uno::Sequence< double >& aAdvancements ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::geometry::RealRectangle2D SAL_CALL queryTextBounds(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL justify( double nSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL combinedJustify( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XTextLayout > >& aNextLayouts, double nSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::TextHit SAL_CALL getTextHit( const ::com::sun::star::geometry::RealPoint2D& aHitPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::Caret SAL_CALL getCaret( sal_Int32 nInsertionIndex, sal_Bool bExcludeLigatures ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getNextInsertionIndex( sal_Int32 nStartIndex, sal_Int32 nCaretAdvancement, sal_Bool bExcludeLigatures ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > SAL_CALL queryVisualHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > SAL_CALL queryLogicalHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL getBaselineOffset(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int8 SAL_CALL getMainTextDirection(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::StringContext SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        bool draw( ::cairo::Cairo* pCairo );
        bool draw( SurfaceSharedPtr&                               pSurface,
                   OutputDevice&                                   rOutDev,
                   const Point&                                    rOutpos,
                   const ::com::sun::star::rendering::ViewState&   viewState,
                   const ::com::sun::star::rendering::RenderState& renderState ) const;

        void setupTextOffsets( sal_Int32*                                       outputOffsets,
                               const ::com::sun::star::uno::Sequence< double >& inputOffsets,
                               const ::com::sun::star::rendering::ViewState&    viewState,
                               const ::com::sun::star::rendering::RenderState&  renderState     ) const;

    protected:
        ~TextLayout(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        ::com::sun::star::rendering::StringContext maText;
        ::com::sun::star::uno::Sequence< double >  maLogicalAdvancements;
        CanvasFont::Reference                      mpFont;
        SurfaceProviderRef                         mpRefDevice;
        sal_Int8                                   mnTextDirection;

        void useFont( ::cairo::Cairo* pCairo );
        bool isCairoRenderable(SystemFontData aSysFontData) const;
    };

}

#endif /* _CAIROCANVAS_TEXTLAYOUT_HXX */
