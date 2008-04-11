/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bitmap.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/rendering/bitmap.hxx>
#include <canvas/rendering/isurfaceproxy.hxx>
#include <basegfx/vector/b2isize.hxx>
#include "image.hxx"
#include <algorithm>
#include <iterator>


using namespace ::com::sun::star;

namespace canvas
{
    class ImplBitmap
    {
    public:
        ImplBitmap( const ::basegfx::B2IVector&          rSize,
                    const ISurfaceProxyManagerSharedPtr& rMgr,
                    bool                                 bWithAlpha ) :
            mpImage(),
            mpSurfaceProxy(),
            mbIsSurfaceDirty( true )
        {
            ENSURE_AND_THROW( rMgr,
                              "Bitmap::Bitmap(): Invalid surface proxy manager" );

            Image::Description desc;

            desc.eFormat = bWithAlpha ? Image::FMT_A8R8G8B8 : Image::FMT_R8G8B8;
            desc.nWidth  = rSize.getX();
            desc.nHeight = rSize.getY();
            desc.nStride = 0;
            desc.pBuffer = NULL;

            mpImage.reset( new Image(desc) );

            // clear the surface [fill with opaque black]
            mpImage->clear(0,255,255,255);

            // create a (possibly hardware accelerated) mirror surface.
            mpSurfaceProxy = rMgr->createSurfaceProxy( mpImage );
        }

        bool hasAlpha() const
        {
            if( !mpImage )
                return false;

            return (mpImage->getDescription().eFormat == Image::FMT_A8R8G8B8);
        }

        ::basegfx::B2IVector getSize() const
        {
            return ::basegfx::B2IVector( mpImage->getWidth(),
                                         mpImage->getHeight() );
        }

        ::com::sun::star::uno::Sequence< sal_Int8 > getData( ::com::sun::star::rendering::IntegerBitmapLayout&     /*bitmapLayout*/,
                                                             const ::com::sun::star::geometry::IntegerRectangle2D& rect )
        {
            const IColorBuffer::Format format = mpImage->getFormat();
            const sal_uInt32 dwNumBytesPerPixel = getNumBytes(format);
            const sal_uInt32 dwPitch = mpImage->getWidth()*dwNumBytesPerPixel;

            if(!(dwNumBytesPerPixel))
                return uno::Sequence< sal_Int8 >();

            const sal_uInt32 dwWidth = rect.X2-rect.X1;
            const sal_uInt32 dwHeight = rect.Y2-rect.Y1;

            uno::Sequence< sal_Int8 > aRes(dwWidth*dwHeight*4);
            sal_uInt8 *pDst = reinterpret_cast<sal_uInt8 *>(aRes.getArray());

            const sal_uInt8 *pSrc = mpImage->lock()+(rect.Y1*dwPitch)+(rect.X1*dwNumBytesPerPixel);
            const sal_uInt32 dwSpanSizeInBytes = dwNumBytesPerPixel*dwWidth;
            for(sal_uInt32 y=0; y<dwHeight; ++y)
            {
                rtl_copyMemory(pDst,pSrc,dwSpanSizeInBytes);
                pDst += dwSpanSizeInBytes;
                pSrc += dwPitch;
            }
            mpImage->unlock();

            return aRes;
        }

        void setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&        data,
                      const ::com::sun::star::rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                      const ::com::sun::star::geometry::IntegerRectangle2D&     rect )
        {
            const IColorBuffer::Format format = mpImage->getFormat();
            const sal_uInt32 dwNumBytesPerPixel = getNumBytes(format);
            const sal_uInt32 dwPitch = mpImage->getWidth()*dwNumBytesPerPixel;

            if(!(dwNumBytesPerPixel))
                return;

            const sal_uInt32 dwWidth = rect.X2-rect.X1;
            const sal_uInt32 dwHeight = rect.Y2-rect.Y1;
            const sal_uInt8 *pSrc = reinterpret_cast<const sal_uInt8 *>(data.getConstArray());

            sal_uInt8 *pDst = mpImage->lock()+(rect.Y1*dwPitch)+(rect.X1*dwNumBytesPerPixel);
            const sal_uInt32 dwSpanSizeInBytes = dwNumBytesPerPixel*dwWidth;
            for(sal_uInt32 y=0; y<dwHeight; ++y)
            {
                rtl_copyMemory(pDst,pSrc,dwSpanSizeInBytes);
                pSrc += dwSpanSizeInBytes;
                pDst += dwPitch;
            }
            mpImage->unlock();
        }

        void setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&       color,
                       const ::com::sun::star::rendering::IntegerBitmapLayout&  /*bitmapLayout*/,
                       const ::com::sun::star::geometry::IntegerPoint2D&        pos )
        {
            struct ARGBColor
            {
                sal_uInt8 a;
                sal_uInt8 r;
                sal_uInt8 g;
                sal_uInt8 b;
            };

            union ARGB
            {
                ARGBColor  Color;
                sal_uInt32 color;

                inline ARGB( sal_uInt32 _color ) : color(_color) {}
            };

            ARGB argb(0xFFFFFFFF);

            if(color.getLength() > 2)
            {
                argb.Color.r = static_cast<sal_uInt8>(color[0]);
                argb.Color.g = static_cast<sal_uInt8>(color[1]);
                argb.Color.b = static_cast<sal_uInt8>(color[2]);
                if(color.getLength() > 3)
                    argb.Color.a = static_cast<sal_uInt8>(255.0f*color[3]);
            }

            const IColorBuffer::Format format = mpImage->getFormat();
            const sal_uInt32 dwNumBytesPerPixel = getNumBytes(format);
            const sal_uInt32 dwPitch = mpImage->getWidth()*dwNumBytesPerPixel;

            if(!(dwNumBytesPerPixel))
                return;

            sal_uInt8 *pDst = mpImage->lock()+(pos.Y*dwPitch)+(pos.X*dwNumBytesPerPixel);

            switch(format)
            {
                case IColorBuffer::FMT_R8G8B8:
                    pDst[0] = argb.Color.r;
                    pDst[1] = argb.Color.g;
                    pDst[2] = argb.Color.b;
                    break;
                case IColorBuffer::FMT_A8R8G8B8:
                case IColorBuffer::FMT_X8R8G8B8:
                    pDst[0] = argb.Color.a;
                    pDst[1] = argb.Color.r;
                    pDst[2] = argb.Color.g;
                    pDst[3] = argb.Color.b;
                    break;
                default:
                    break;
            }

            mpImage->unlock();
        }

        uno::Sequence< sal_Int8 > getPixel( ::com::sun::star::rendering::IntegerBitmapLayout& /*bitmapLayout*/,
                                            const ::com::sun::star::geometry::IntegerPoint2D& pos )
        {
            const IColorBuffer::Format format = mpImage->getFormat();
            const sal_uInt32 dwNumBytesPerPixel = getNumBytes(format);
            const sal_uInt32 dwPitch = mpImage->getWidth()*dwNumBytesPerPixel;

            if(!(dwNumBytesPerPixel))
                return uno::Sequence< sal_Int8 >();

            uno::Sequence< sal_Int8 > aRet(dwNumBytesPerPixel);
            const sal_uInt8 *pSrc = mpImage->lock()+(pos.Y*dwPitch)+(pos.X*dwNumBytesPerPixel);

            switch(format)
            {
                case IColorBuffer::FMT_R8G8B8:
                    aRet[0] = pSrc[0];
                    aRet[1] = pSrc[1];
                    aRet[2] = pSrc[2];
                    break;
                case IColorBuffer::FMT_A8R8G8B8:
                case IColorBuffer::FMT_X8R8G8B8:
                    aRet[0] = pSrc[1];
                    aRet[1] = pSrc[2];
                    aRet[2] = pSrc[3];
                    aRet[3] = pSrc[0];
                    break;
                default:
                    break;
            }

            mpImage->unlock();

            return aRet;
        }

        // the IColorBuffer interface
        // ==========================

        bool draw( double                         fAlpha,
                   const ::basegfx::B2DPoint&     rPos,
                   const ::basegfx::B2DHomMatrix& rTransform )
        {
            if( mbIsSurfaceDirty )
            {
                mpSurfaceProxy->setColorBufferDirty();
                mbIsSurfaceDirty = false;
            }

            return mpSurfaceProxy->draw( fAlpha, rPos, rTransform );
        }

        bool draw( double                         fAlpha,
                   const ::basegfx::B2DPoint&     rPos,
                   const ::basegfx::B2DRange&     rArea,
                   const ::basegfx::B2DHomMatrix& rTransform )
        {
            if( mbIsSurfaceDirty )
            {
                mpSurfaceProxy->setColorBufferDirty();
                mbIsSurfaceDirty = false;
            }

            return mpSurfaceProxy->draw( fAlpha, rPos, rArea, rTransform );
        }


        bool draw( double                           fAlpha,
                   const ::basegfx::B2DPoint&       rPos,
                   const ::basegfx::B2DPolyPolygon& rClipPoly,
                   const ::basegfx::B2DHomMatrix&   rTransform )
        {
            if( mbIsSurfaceDirty )
            {
                mpSurfaceProxy->setColorBufferDirty();
                mbIsSurfaceDirty = false;
            }

            return mpSurfaceProxy->draw( fAlpha, rPos, rClipPoly, rTransform );
        }

        void clear( const uno::Sequence< double >& color )
        {
            if(color.getLength() > 2)
            {
                mbIsSurfaceDirty = true;

                if(color.getLength() > 3)
                {
                    mpImage->clear( static_cast<sal_uInt8>(255.0f*color[0]),
                                    static_cast<sal_uInt8>(255.0f*color[1]),
                                    static_cast<sal_uInt8>(255.0f*color[2]),
                                    static_cast<sal_uInt8>(255.0f*color[3]) );
                }
                else
                {
                    mpImage->clear( static_cast<sal_uInt8>(255.0f*color[0]),
                                    static_cast<sal_uInt8>(255.0f*color[1]),
                                    static_cast<sal_uInt8>(255.0f*color[2]),
                                    255 );
                }
            }
        }

        void fillB2DPolyPolygon( const ::basegfx::B2DPolyPolygon&   rPolyPolygon,
                                 const rendering::ViewState&        viewState,
                                 const rendering::RenderState&      renderState )
        {
            mbIsSurfaceDirty = true;

            mpImage->fillB2DPolyPolygon( rPolyPolygon,
                                         viewState,
                                         renderState );
        }


        // the XCanvas-render interface
        // ============================

        void drawPoint( const geometry::RealPoint2D&    aPoint,
                        const rendering::ViewState&     viewState,
                        const rendering::RenderState&   renderState )
        {
            mbIsSurfaceDirty = true;

            mpImage->drawPoint( aPoint, viewState, renderState );
        }

        void drawLine( const geometry::RealPoint2D&     aStartPoint,
                       const geometry::RealPoint2D&     aEndPoint,
                       const rendering::ViewState&      viewState,
                       const rendering::RenderState&    renderState )
        {
            mbIsSurfaceDirty = true;

            mpImage->drawLine( aStartPoint, aEndPoint, viewState, renderState );
        }

        void drawBezier( const geometry::RealBezierSegment2D&   aBezierSegment,
                         const geometry::RealPoint2D&           aEndPoint,
                         const rendering::ViewState&            viewState,
                         const rendering::RenderState&          renderState )
        {
            mbIsSurfaceDirty = true;

            mpImage->drawBezier( aBezierSegment, aEndPoint, viewState, renderState );
        }

        ICachedPrimitiveSharedPtr drawPolyPolygon(
            const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
            const rendering::ViewState&                         viewState,
            const rendering::RenderState&                       renderState )
        {
            mbIsSurfaceDirty = true;

            return setupCachedPrimitive(
                mpImage->drawPolyPolygon( xPolyPolygon, viewState, renderState ) );
        }

        ICachedPrimitiveSharedPtr strokePolyPolygon(
            const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
            const rendering::ViewState&                         viewState,
            const rendering::RenderState&                       renderState,
            const rendering::StrokeAttributes&                  strokeAttributes )
        {
            mbIsSurfaceDirty = true;

            return setupCachedPrimitive(
                mpImage->strokePolyPolygon( xPolyPolygon,
                                            viewState,
                                            renderState,
                                            strokeAttributes ) );
        }

        ICachedPrimitiveSharedPtr strokeTexturedPolyPolygon(
            const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
            const rendering::ViewState&                          viewState,
            const rendering::RenderState&                        renderState,
            const uno::Sequence< rendering::Texture >&           textures,
            const ::std::vector< ::boost::shared_ptr<Bitmap> >&  textureAnnotations,
            const rendering::StrokeAttributes&                   strokeAttributes )
        {
            mbIsSurfaceDirty = true;

            ::std::vector< ImageSharedPtr > aTextureAnnotations;
            convertTextureAnnotations( aTextureAnnotations,
                                       textureAnnotations );

            return setupCachedPrimitive(
                mpImage->strokeTexturedPolyPolygon( xPolyPolygon,
                                                    viewState,
                                                    renderState,
                                                    textures,
                                                    aTextureAnnotations,
                                                    strokeAttributes ) );
        }

        ICachedPrimitiveSharedPtr strokeTextureMappedPolyPolygon(
            const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
            const rendering::ViewState&                          viewState,
            const rendering::RenderState&                        renderState,
            const uno::Sequence< rendering::Texture >&           textures,
            const ::std::vector< ::boost::shared_ptr<Bitmap> >&  textureAnnotations,
            const uno::Reference< geometry::XMapping2D >&        xMapping,
            const rendering::StrokeAttributes&                   strokeAttributes )
        {
            mbIsSurfaceDirty = true;

            ::std::vector< ImageSharedPtr > aTextureAnnotations;
            convertTextureAnnotations( aTextureAnnotations,
                                       textureAnnotations );

            return setupCachedPrimitive(
                mpImage->strokeTextureMappedPolyPolygon( xPolyPolygon,
                                                         viewState,
                                                         renderState,
                                                         textures,
                                                         aTextureAnnotations,
                                                         xMapping,
                                                         strokeAttributes ) );
        }


        ICachedPrimitiveSharedPtr fillPolyPolygon(
            const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
            const rendering::ViewState&                         viewState,
            const rendering::RenderState&                       renderState )
        {
            mbIsSurfaceDirty = true;

            return setupCachedPrimitive(
                mpImage->fillPolyPolygon( xPolyPolygon,
                                          viewState,
                                          renderState ) );
        }

        ICachedPrimitiveSharedPtr fillTexturedPolyPolygon(
            const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
            const rendering::ViewState&                          viewState,
            const rendering::RenderState&                        renderState,
            const uno::Sequence< rendering::Texture >&           textures,
            const ::std::vector< ::boost::shared_ptr<Bitmap> >&  textureAnnotations )
        {
            mbIsSurfaceDirty = true;

            ::std::vector< ImageSharedPtr > aTextureAnnotations;
            convertTextureAnnotations( aTextureAnnotations,
                                       textureAnnotations );

            return setupCachedPrimitive(
                mpImage->fillTexturedPolyPolygon( xPolyPolygon,
                                                  viewState,
                                                  renderState,
                                                  textures,
                                                  aTextureAnnotations ) );
        }


        ICachedPrimitiveSharedPtr fillTextureMappedPolyPolygon(
            const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
            const rendering::ViewState&                          viewState,
            const rendering::RenderState&                        renderState,
            const uno::Sequence< rendering::Texture >&           textures,
            const ::std::vector< ::boost::shared_ptr<Bitmap> >&  textureAnnotations,
            const uno::Reference< geometry::XMapping2D >&        xMapping )
        {
            mbIsSurfaceDirty = true;

            ::std::vector< ImageSharedPtr > aTextureAnnotations;
            convertTextureAnnotations( aTextureAnnotations,
                                       textureAnnotations );

            return setupCachedPrimitive(
                mpImage->fillTextureMappedPolyPolygon( xPolyPolygon,
                                                       viewState,
                                                       renderState,
                                                       textures,
                                                       aTextureAnnotations,
                                                       xMapping ) );
        }

        ICachedPrimitiveSharedPtr drawBitmap(
            const uno::Reference< rendering::XBitmap >&   xBitmap,
            const rendering::ViewState&                   viewState,
            const rendering::RenderState&                 renderState )
        {
            mbIsSurfaceDirty = true;

            return setupCachedPrimitive(
                mpImage->drawBitmap( xBitmap,
                                     viewState,
                                     renderState ) );
        }

        ICachedPrimitiveSharedPtr drawBitmap(
            const ::boost::shared_ptr<Bitmap>&  rImage,
            const rendering::ViewState&         viewState,
            const rendering::RenderState&       renderState )
        {
            mbIsSurfaceDirty = true;

            return setupCachedPrimitive(
                mpImage->drawBitmap( rImage->mpImpl->mpImage,
                                     viewState,
                                     renderState ) );
        }

        ICachedPrimitiveSharedPtr drawBitmapModulated(
            const uno::Reference< rendering::XBitmap >&     xBitmap,
            const rendering::ViewState&                     viewState,
            const rendering::RenderState&                   renderState )
        {
            mbIsSurfaceDirty = true;

            return setupCachedPrimitive(
                mpImage->drawBitmapModulated( xBitmap,
                                              viewState,
                                              renderState ) );
        }


        ICachedPrimitiveSharedPtr drawBitmapModulated(
            const ::boost::shared_ptr<Bitmap>&   rImage,
            const rendering::ViewState&          viewState,
            const rendering::RenderState&        renderState )
        {
            mbIsSurfaceDirty = true;

            return setupCachedPrimitive(
                mpImage->drawBitmapModulated( rImage->mpImpl->mpImage,
                                              viewState,
                                              renderState ) );
        }

    private:
        ICachedPrimitiveSharedPtr setupCachedPrimitive(
            const ImageCachedPrimitiveSharedPtr& rCachedPrimitive ) const
        {
            if( rCachedPrimitive )
                rCachedPrimitive->setImage( mpImage );

            return rCachedPrimitive;
        }

        void convertTextureAnnotations( ::std::vector< ImageSharedPtr >&         o_rTextureAnnotations,
                                        const ::std::vector< BitmapSharedPtr >&  textureAnnotations )
        {
            ::std::vector< BitmapSharedPtr >::const_iterator       aCurr( textureAnnotations.begin() );
            const ::std::vector< BitmapSharedPtr >::const_iterator aEnd( textureAnnotations.end() );
            while( aCurr != aEnd )
            {
                if( aCurr->get() != NULL )
                    o_rTextureAnnotations.push_back( (*aCurr)->mpImpl->mpImage );
                else
                    o_rTextureAnnotations.push_back( ImageSharedPtr() );

                ++aCurr;
            }
        }

        sal_uInt32 getNumBytes( IColorBuffer::Format format )
        {
            switch(format)
            {
                case IColorBuffer::FMT_R8G8B8:
                    return 3;
                case IColorBuffer::FMT_A8R8G8B8:
                case IColorBuffer::FMT_X8R8G8B8:
                    return 4;
                default:
                    return 0;
            }
        }

        ImageSharedPtr          mpImage;
        ISurfaceProxySharedPtr  mpSurfaceProxy;
        bool                    mbIsSurfaceDirty;
    };


    /////////////////////////////////////////////////////////////////
    // Actual Bitmap pimpl forwarding functions
    /////////////////////////////////////////////////////////////////

    Bitmap::Bitmap( const ::basegfx::B2IVector&          rSize,
                    const ISurfaceProxyManagerSharedPtr& rMgr,
                    bool                                 bWithAlpha ) :
        mpImpl( new ImplBitmap( rSize,
                                rMgr,
                                bWithAlpha ) )
    {
    }

    Bitmap::~Bitmap()
    {
        // outline destructor is _necessary_, because ImplBitmap is an
        // incomplete type outside this file.
    }

    // forward all methods to ImplBitmap
    // ==================================================

    bool Bitmap::hasAlpha() const
    {
        return mpImpl->hasAlpha();
    }

    ::basegfx::B2IVector Bitmap::getSize() const
    {
        return mpImpl->getSize();
    }

    ::com::sun::star::uno::Sequence< sal_Int8 > Bitmap::getData(
        ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
        const ::com::sun::star::geometry::IntegerRectangle2D& rect )
    {
        return mpImpl->getData(bitmapLayout,rect);
    }

    void Bitmap::setData(
        const ::com::sun::star::uno::Sequence< sal_Int8 >& data,
        const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
        const ::com::sun::star::geometry::IntegerRectangle2D& rect )
    {
        mpImpl->setData(data,bitmapLayout,rect);
    }

    void Bitmap::setPixel(
        const ::com::sun::star::uno::Sequence< sal_Int8 >&      color,
        const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
        const ::com::sun::star::geometry::IntegerPoint2D&       pos )
    {
        mpImpl->setPixel(color,bitmapLayout,pos);
    }

    uno::Sequence< sal_Int8 > Bitmap::getPixel(
        ::com::sun::star::rendering::IntegerBitmapLayout&       bitmapLayout,
        const ::com::sun::star::geometry::IntegerPoint2D&       pos )
    {
        return mpImpl->getPixel(bitmapLayout,pos);
    }

    bool Bitmap::draw( double                         fAlpha,
                       const ::basegfx::B2DPoint&     rPos,
                       const ::basegfx::B2DHomMatrix& rTransform )
    {
        return mpImpl->draw( fAlpha, rPos, rTransform );
    }

    bool Bitmap::draw( double                         fAlpha,
                       const ::basegfx::B2DPoint&     rPos,
                       const ::basegfx::B2DRange&     rArea,
                       const ::basegfx::B2DHomMatrix& rTransform )
    {
        return mpImpl->draw( fAlpha, rPos, rArea, rTransform );
    }


    bool Bitmap::draw( double                           fAlpha,
                       const ::basegfx::B2DPoint&       rPos,
                       const ::basegfx::B2DPolyPolygon& rClipPoly,
                       const ::basegfx::B2DHomMatrix&   rTransform )
    {
        return mpImpl->draw( fAlpha, rPos, rClipPoly, rTransform );
    }

    void Bitmap::clear( const uno::Sequence< double >& color )
    {
        mpImpl->clear( color );
    }

    void Bitmap::fillB2DPolyPolygon(
            const ::basegfx::B2DPolyPolygon&    rPolyPolygon,
            const rendering::ViewState&         viewState,
            const rendering::RenderState&       renderState )
    {
        mpImpl->fillB2DPolyPolygon( rPolyPolygon, viewState, renderState );
    }

    void Bitmap::drawPoint( const geometry::RealPoint2D&    aPoint,
                            const rendering::ViewState&     viewState,
                            const rendering::RenderState&   renderState )
    {
        return mpImpl->drawPoint( aPoint, viewState, renderState );
    }

    void Bitmap::drawLine( const geometry::RealPoint2D&     aStartPoint,
                           const geometry::RealPoint2D&     aEndPoint,
                           const rendering::ViewState&      viewState,
                           const rendering::RenderState&    renderState )
    {
        return mpImpl->drawLine( aStartPoint, aEndPoint, viewState, renderState );
    }

    void Bitmap::drawBezier( const geometry::RealBezierSegment2D&   aBezierSegment,
                             const geometry::RealPoint2D&           aEndPoint,
                             const rendering::ViewState&            viewState,
                             const rendering::RenderState&          renderState )
    {
        return mpImpl->drawBezier( aBezierSegment, aEndPoint, viewState, renderState );
    }

    ICachedPrimitiveSharedPtr Bitmap::drawPolyPolygon(
        const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
        const rendering::ViewState&                         viewState,
        const rendering::RenderState&                       renderState )
    {
        return mpImpl->drawPolyPolygon( xPolyPolygon, viewState, renderState );
    }

    ICachedPrimitiveSharedPtr Bitmap::strokePolyPolygon(
        const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
        const rendering::ViewState&                         viewState,
        const rendering::RenderState&                       renderState,
        const rendering::StrokeAttributes&                  strokeAttributes )
    {
        return mpImpl->strokePolyPolygon( xPolyPolygon, viewState, renderState, strokeAttributes );
    }

    ICachedPrimitiveSharedPtr Bitmap::strokeTexturedPolyPolygon(
        const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
        const rendering::ViewState&                          viewState,
        const rendering::RenderState&                        renderState,
        const uno::Sequence< rendering::Texture >&           textures,
        const ::std::vector< ::boost::shared_ptr<Bitmap> >&  textureAnnotations,
        const rendering::StrokeAttributes&                   strokeAttributes )
    {
        return mpImpl->strokeTexturedPolyPolygon( xPolyPolygon,
                                                  viewState,
                                                  renderState,
                                                  textures,
                                                  textureAnnotations,
                                                  strokeAttributes );
    }

    ICachedPrimitiveSharedPtr Bitmap::strokeTextureMappedPolyPolygon(
        const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
        const rendering::ViewState&                          viewState,
        const rendering::RenderState&                        renderState,
        const uno::Sequence< rendering::Texture >&           textures,
        const ::std::vector< ::boost::shared_ptr<Bitmap> >&  textureAnnotations,
        const uno::Reference< geometry::XMapping2D >&        xMapping,
        const rendering::StrokeAttributes&                   strokeAttributes )
    {
        return mpImpl->strokeTextureMappedPolyPolygon( xPolyPolygon,
                                                       viewState,
                                                       renderState,
                                                       textures,
                                                       textureAnnotations,
                                                       xMapping,
                                                       strokeAttributes );
    }


    ICachedPrimitiveSharedPtr Bitmap::fillPolyPolygon(
        const uno::Reference< rendering::XPolyPolygon2D >&  xPolyPolygon,
        const rendering::ViewState&                         viewState,
        const rendering::RenderState&                       renderState )
    {
        return mpImpl->fillPolyPolygon( xPolyPolygon,
                                        viewState,
                                        renderState );
    }

    ICachedPrimitiveSharedPtr Bitmap::fillTexturedPolyPolygon(
        const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
        const rendering::ViewState&                          viewState,
        const rendering::RenderState&                        renderState,
        const uno::Sequence< rendering::Texture >&           textures,
        const ::std::vector< ::boost::shared_ptr<Bitmap> >&  textureAnnotations )
    {
        return mpImpl->fillTexturedPolyPolygon( xPolyPolygon,
                                                viewState,
                                                renderState,
                                                textures,
                                                textureAnnotations );
    }

    ICachedPrimitiveSharedPtr Bitmap::fillTextureMappedPolyPolygon(
        const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
        const rendering::ViewState&                          viewState,
        const rendering::RenderState&                        renderState,
        const uno::Sequence< rendering::Texture >&           textures,
        const ::std::vector< ::boost::shared_ptr<Bitmap> >&  textureAnnotations,
        const uno::Reference< geometry::XMapping2D >&        xMapping )
    {
        return mpImpl->fillTextureMappedPolyPolygon( xPolyPolygon,
                                                     viewState,
                                                     renderState,
                                                     textures,
                                                     textureAnnotations,
                                                     xMapping );
    }

    ICachedPrimitiveSharedPtr Bitmap::drawBitmap(
        const uno::Reference< rendering::XBitmap >&   xBitmap,
        const rendering::ViewState&                   viewState,
        const rendering::RenderState&                 renderState )
    {
        return mpImpl->drawBitmap( xBitmap,
                                   viewState,
                                   renderState );
    }

    ICachedPrimitiveSharedPtr Bitmap::drawBitmap(
        const ::boost::shared_ptr<Bitmap>&  rImage,
        const rendering::ViewState&         viewState,
        const rendering::RenderState&       renderState )
    {
        return mpImpl->drawBitmap( rImage,
                                   viewState,
                                   renderState );
    }

    ICachedPrimitiveSharedPtr Bitmap::drawBitmapModulated(
        const uno::Reference< rendering::XBitmap >&     xBitmap,
        const rendering::ViewState&                     viewState,
        const rendering::RenderState&                   renderState )
    {
        return mpImpl->drawBitmapModulated( xBitmap,
                                            viewState,
                                            renderState );
    }

    ICachedPrimitiveSharedPtr Bitmap::drawBitmapModulated(
        const ::boost::shared_ptr<Bitmap>&   rImage,
        const rendering::ViewState&          viewState,
        const rendering::RenderState&        renderState )
    {
        return mpImpl->drawBitmapModulated( rImage,
                                            viewState,
                                            renderState );
    }
}
