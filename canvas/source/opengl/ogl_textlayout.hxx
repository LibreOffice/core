/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_TEXTLAYOUT_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_TEXTLAYOUT_HXX

#include <cppuhelper/compbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/rendering/XTextLayout.hpp>

#include <basegfx/vector/b2isize.hxx>

#include <boost/utility.hpp>

#include "ogl_canvasfont.hxx"


/* Definition of TextLayout class */

namespace oglcanvas
{
    typedef ::cppu::WeakComponentImplHelper1< ::com::sun::star::rendering::XTextLayout > TextLayoutBaseT;

    class TextLayout : public ::comphelper::OBaseMutex,
                       public TextLayoutBaseT,
                       private ::boost::noncopyable
    {
    public:
        TextLayout( const ::com::sun::star::rendering::StringContext& aText,
                    sal_Int8                                          nDirection,
                    sal_Int64                                         nRandomSeed,
                    const CanvasFont::ImplRef&                        rFont );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XTextLayout
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > > SAL_CALL queryTextShapes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealRectangle2D > SAL_CALL queryInkMeasures(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealRectangle2D > SAL_CALL queryMeasures(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL queryLogicalAdvancements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL applyLogicalAdvancements( const ::com::sun::star::uno::Sequence< double >& aAdvancements ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::geometry::RealRectangle2D SAL_CALL queryTextBounds(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual double SAL_CALL justify( double nSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual double SAL_CALL combinedJustify( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XTextLayout > >& aNextLayouts, double nSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::rendering::TextHit SAL_CALL getTextHit( const ::com::sun::star::geometry::RealPoint2D& aHitPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::rendering::Caret SAL_CALL getCaret( sal_Int32 nInsertionIndex, sal_Bool bExcludeLigatures ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int32 SAL_CALL getNextInsertionIndex( sal_Int32 nStartIndex, sal_Int32 nCaretAdvancement, sal_Bool bExcludeLigatures ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > SAL_CALL queryVisualHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > SAL_CALL queryLogicalHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual double SAL_CALL getBaselineOffset(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int8 SAL_CALL getMainTextDirection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::rendering::StringContext SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        bool draw( const ::com::sun::star::rendering::ViewState&         rViewState,
                   const ::com::sun::star::rendering::RenderState&       rRenderState,
                   const ::com::sun::star::uno::Reference<
                         ::com::sun::star::rendering::XGraphicDevice >&  xGraphicDevice ) const;

    private:
        ::com::sun::star::rendering::StringContext maText;
        ::com::sun::star::uno::Sequence< double >  maLogicalAdvancements;
        CanvasFont::ImplRef                        mpFont;
        sal_Int8                                   mnTextDirection;
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
