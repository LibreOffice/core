/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textlayout.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _VCLCANVAS_TEXTLAYOUT_HXX
#define _VCLCANVAS_TEXTLAYOUT_HXX

#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/XTextLayout.hpp>

#include <canvas/vclwrapper.hxx>

#include "canvasfont.hxx"
#include "impltools.hxx"

#include <boost/utility.hpp>


/* Definition of TextLayout class */

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XTextLayout,
                                               ::com::sun::star::lang::XServiceInfo > TextLayout_Base;

    class TextLayout : public ::comphelper::OBaseMutex,
                       public TextLayout_Base,
                       private ::boost::noncopyable
    {
    public:
        TextLayout( const ::com::sun::star::rendering::StringContext&   aText,
                    sal_Int8                                            nDirection,
                    sal_Int64                                           nRandomSeed,
                    const CanvasFont::Reference&                        rFont,
                    const DeviceRef&                                    rRefDevice );

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

        bool draw( OutputDevice&                                   rOutDev,
                   const Point&                                    rOutpos,
                   const ::com::sun::star::rendering::ViewState&   viewState,
                   const ::com::sun::star::rendering::RenderState& renderState ) const;

    private:
        void setupTextOffsets( sal_Int32*                                       outputOffsets,
                               const ::com::sun::star::uno::Sequence< double >& inputOffsets,
                               const ::com::sun::star::rendering::ViewState&    viewState,
                               const ::com::sun::star::rendering::RenderState&  renderState     ) const;

        ::com::sun::star::rendering::StringContext  maText;
        ::com::sun::star::uno::Sequence< double >   maLogicalAdvancements;
        CanvasFont::Reference                       mpFont;
        DeviceRef                                   mpRefDevice;
        sal_Int8                                    mnTextDirection;
    };

}

#endif /* _VCLCANVAS_TEXTLAYOUT_HXX */
