/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/PanoseLetterForm.hpp>
#include <com/sun/star/rendering/PanoseWeight.hpp>
#include <com/sun/star/rendering/XSimpleCanvas.hpp>
#include <comphelper/servicedecl.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <o3tl/lazy_update.hxx>

#include <canvas/canvastools.hxx>


#include <functional>

#define SERVICE_NAME "com.sun.star.rendering.SimpleCanvas"

using namespace ::com::sun::star;
using namespace canvas;

namespace
{
    uno::Sequence< double > color2Sequence( sal_Int32 nColor )
    {
        // TODO(F3): Color management
        uno::Sequence< double > aRes( 4 );

        aRes[0] = static_cast<sal_uInt8>( (nColor&0xFF000000U) >> 24U ) / 255.0;
        aRes[1] = static_cast<sal_uInt8>( (nColor&0x00FF0000U) >> 16U ) / 255.0;
        aRes[2] = static_cast<sal_uInt8>( (nColor&0x0000FF00U) >>  8U ) / 255.0;
        aRes[3] = static_cast<sal_uInt8>( (nColor&0x000000FFU) )        / 255.0;

        return aRes;
    }

    uno::Reference< rendering::XPolyPolygon2D > rect2Poly( uno::Reference<rendering::XGraphicDevice> const& xDevice,
                                                                  geometry::RealRectangle2D const&                 rRect )
    {
        uno::Sequence< geometry::RealPoint2D > rectSequence( 4 );
        geometry::RealPoint2D* pOutput = rectSequence.getArray();
        pOutput[0] = geometry::RealPoint2D( rRect.X1, rRect.Y1 );
        pOutput[1] = geometry::RealPoint2D( rRect.X2, rRect.Y1 );
        pOutput[2] = geometry::RealPoint2D( rRect.X2, rRect.Y2 );
        pOutput[3] = geometry::RealPoint2D( rRect.X1, rRect.Y2 );

        uno::Sequence< uno::Sequence< geometry::RealPoint2D > > sequenceSequence( 1 );
        sequenceSequence[0] = rectSequence;

        uno::Reference< rendering::XPolyPolygon2D > xRes(
            xDevice->createCompatibleLinePolyPolygon( sequenceSequence ),
            uno::UNO_QUERY );
        if( xRes.is() )
            xRes->setClosed( 0, true );
        return xRes;
    }

    struct SimpleRenderState
    {
        o3tl::LazyUpdate<sal_Int32,
                         uno::Sequence<double>,
                         decltype(&color2Sequence)>                   m_aPenColor;
        o3tl::LazyUpdate<sal_Int32,
                         uno::Sequence<double>,
                         decltype(&color2Sequence)>                   m_aFillColor;
        o3tl::LazyUpdate<geometry::RealRectangle2D,
                         uno::Reference< rendering::XPolyPolygon2D >,
                         std::function<uno::Reference<rendering::XPolyPolygon2D> (geometry::RealRectangle2D)> > m_aRectClip;
        geometry::AffineMatrix2D                                      m_aTransformation;

        explicit SimpleRenderState( uno::Reference<rendering::XGraphicDevice> const& xDevice ) :
            m_aPenColor( &color2Sequence),
            m_aFillColor( &color2Sequence ),
            m_aRectClip( [&xDevice](geometry::RealRectangle2D const& rRect) { return rect2Poly(xDevice, rRect); } ),
            m_aTransformation()
        {
            tools::setIdentityAffineMatrix2D( m_aTransformation );
        }
    };


    typedef ::cppu::WeakComponentImplHelper< css::rendering::XSimpleCanvas,
                                             css::lang::XServiceName >    SimpleCanvasBase;

    class SimpleCanvasImpl : private cppu::BaseMutex,
                             public SimpleCanvasBase
    {
    private:
        bool isStrokingEnabled() const
        {
            return maRenderState.m_aPenColor.getInValue() % 0x100 != 0;
        }

        rendering::RenderState createStrokingRenderState() const
        {
            return rendering::RenderState(maRenderState.m_aTransformation,
                                          *maRenderState.m_aRectClip,
                                          *maRenderState.m_aPenColor,
                                          rendering::CompositeOperation::OVER);
        }

        bool isFillingEnabled() const
        {
            return maRenderState.m_aFillColor.getInValue() % 0x100 != 0;
        }

        rendering::RenderState createFillingRenderState() const
        {
            return rendering::RenderState(maRenderState.m_aTransformation,
                                          *maRenderState.m_aRectClip,
                                          *maRenderState.m_aFillColor,
                                          rendering::CompositeOperation::OVER);
        }

        static uno::Reference<rendering::XCanvas> grabCanvas( uno::Sequence<uno::Any> const& rArgs )
        {
            uno::Reference<rendering::XCanvas> xRet;

            // can't do much without an XCanvas, can't we?
            if( rArgs.getLength() < 1 )
                throw lang::IllegalArgumentException();

            xRet.set( rArgs[0], uno::UNO_QUERY );

            // can't do much without an XCanvas, can't we?
            if( !xRet.is() )
                throw lang::IllegalArgumentException();

            return xRet;
        }

    public:
        SimpleCanvasImpl( const uno::Sequence< uno::Any >&                aArguments,
                          const uno::Reference< uno::XComponentContext >&  ) :
            SimpleCanvasBase( m_aMutex ),
            mxCanvas( grabCanvas(aArguments) ),
            maFont([this](rendering::FontRequest const& rFontRequest) {
                   return mxCanvas->createFont(rFontRequest,
                                               uno::Sequence< beans::PropertyValue >(),
                                               geometry::Matrix2D()); } ),
            maViewState(),
            maRenderState( mxCanvas->getDevice() )
        {
            tools::initViewState(maViewState);
        }


    private:
        // Ifc XServiceName
        virtual OUString SAL_CALL getServiceName(  ) override
        {
            return OUString( SERVICE_NAME );
        }

        // Ifc XSimpleCanvas
        virtual void SAL_CALL selectFont( const OUString& sFontName,
                                          double                 size,
                                          sal_Bool             bold,
                                          sal_Bool             italic ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            maFont->FontDescription.FamilyName = sFontName;
            maFont->CellSize = size;
            maFont->FontDescription.FontDescription.Weight =
                bold ? rendering::PanoseWeight::BOLD : rendering::PanoseWeight::MEDIUM;
            maFont->FontDescription.FontDescription.Letterform =
                italic ? rendering::PanoseLetterForm::OBLIQUE_CONTACT : rendering::PanoseLetterForm::ANYTHING;
        }

        virtual void SAL_CALL setPenColor( ::sal_Int32 nsRgbaColor ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            *(maRenderState.m_aPenColor) = nsRgbaColor;
        }

        virtual void SAL_CALL setFillColor( ::sal_Int32 nsRgbaColor ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            *(maRenderState.m_aFillColor) = nsRgbaColor;
        }

        virtual void SAL_CALL setRectClip( const geometry::RealRectangle2D& aRect ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            *(maRenderState.m_aRectClip) = aRect;
        }

        virtual void SAL_CALL setTransformation( const geometry::AffineMatrix2D& aTransform ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            maRenderState.m_aTransformation = aTransform;
        }

        virtual void SAL_CALL drawPixel( const geometry::RealPoint2D& aPoint ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            mxCanvas->drawPoint(aPoint,
                                maViewState,
                                createFillingRenderState());
        }

        virtual void SAL_CALL drawLine( const geometry::RealPoint2D& aStartPoint,
                                        const geometry::RealPoint2D& aEndPoint ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            mxCanvas->drawLine(aStartPoint,
                               aEndPoint,
                               maViewState,
                               createStrokingRenderState());
        }

        virtual void SAL_CALL drawRect( const geometry::RealRectangle2D& aRect ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            uno::Reference< rendering::XPolyPolygon2D > xPoly(
                rect2Poly( mxCanvas->getDevice(),
                           aRect));

            if( isFillingEnabled() )
                mxCanvas->drawPolyPolygon(xPoly,
                                          maViewState,
                                          createFillingRenderState());
            if( isStrokingEnabled() )
                mxCanvas->drawPolyPolygon(xPoly,
                                          maViewState,
                                          createStrokingRenderState());
        }

        virtual void SAL_CALL drawPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if( isFillingEnabled() )
                mxCanvas->drawPolyPolygon(xPolyPolygon,
                                          maViewState,
                                          createFillingRenderState());
            if( isStrokingEnabled() )
                mxCanvas->drawPolyPolygon(xPolyPolygon,
                                          maViewState,
                                          createStrokingRenderState());
        }

        virtual void SAL_CALL drawText( const rendering::StringContext& aText,
                                        const geometry::RealPoint2D&    aOutPos,
                                        ::sal_Int8                      nTextDirection ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            const basegfx::B2DHomMatrix offsetTransform(basegfx::utils::createTranslateB2DHomMatrix(aOutPos.X,aOutPos.Y));
            rendering::RenderState aRenderState( createStrokingRenderState() );
            tools::appendToRenderState(aRenderState, offsetTransform);

            mxCanvas->drawText(aText,
                               maFont.getOutValue(),
                               maViewState,
                               aRenderState,
                               nTextDirection);
        }

        virtual void SAL_CALL drawBitmap( const uno::Reference< rendering::XBitmap >& xBitmap,
                                          const geometry::RealPoint2D&                aLeftTop ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            const basegfx::B2DHomMatrix offsetTransform(basegfx::utils::createTranslateB2DHomMatrix(aLeftTop.X,aLeftTop.Y));
            rendering::RenderState aRenderState( createStrokingRenderState() );
            tools::appendToRenderState(aRenderState, offsetTransform);

            mxCanvas->drawBitmap(xBitmap,maViewState,aRenderState);
        }

        virtual uno::Reference< rendering::XGraphicDevice > SAL_CALL getDevice(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return mxCanvas->getDevice();
        }

        virtual uno::Reference< rendering::XCanvas > SAL_CALL getCanvas(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return mxCanvas;
        }

        virtual rendering::FontMetrics SAL_CALL getFontMetrics(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return maFont.getOutValue()->getFontMetrics();
        }

        virtual uno::Reference< rendering::XCanvasFont > SAL_CALL getCurrentFont(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return maFont.getOutValue();
        }

        virtual ::sal_Int32 SAL_CALL getCurrentPenColor(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return maRenderState.m_aPenColor.getInValue();
        }

        virtual ::sal_Int32 SAL_CALL getCurrentFillColor(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return maRenderState.m_aFillColor.getInValue();
        }

        virtual geometry::RealRectangle2D SAL_CALL getCurrentClipRect(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return maRenderState.m_aRectClip.getInValue();
        }

        virtual geometry::AffineMatrix2D SAL_CALL getCurrentTransformation(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return maRenderState.m_aTransformation;
        }

        virtual rendering::ViewState SAL_CALL getCurrentViewState(  ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            return maViewState;
        }

        virtual rendering::RenderState SAL_CALL getCurrentRenderState( sal_Bool bUseFillColor ) override
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if( bUseFillColor )
                return createFillingRenderState();
            else
                return createStrokingRenderState();
        }


        typedef o3tl::LazyUpdate<
            rendering::FontRequest,
            uno::Reference< rendering::XCanvasFont >,
            std::function<uno::Reference<rendering::XCanvasFont> (rendering::FontRequest)> > SimpleFont;

        uno::Reference<rendering::XCanvas> mxCanvas;
        SimpleFont                         maFont;
        rendering::ViewState               maViewState;
        SimpleRenderState                  maRenderState;
    };

    namespace sdecl = comphelper::service_decl;
    const sdecl::ServiceDecl simpleCanvasDecl(
        sdecl::class_<SimpleCanvasImpl, sdecl::with_args<true> >(),
        "com.sun.star.comp.rendering.SimpleCanvas",
        SERVICE_NAME );
}

// The C shared lib entry points
extern "C" SAL_DLLPUBLIC_EXPORT void* simplecanvas_component_getFactory( sal_Char const* pImplName,
                                         void*, void* )
{
    return sdecl::component_getFactoryHelper( pImplName, {&simpleCanvasDecl} );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
