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

#include <string.h>

#include <com/sun/star/rendering/ColorComponentTag.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2irange.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/rendering/icolorbuffer.hxx>

#include "dx_graphicsprovider.hxx"
#include "dx_impltools.hxx"
#include "dx_surfacebitmap.hxx"
#include "dx_surfacegraphics.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {

        // DXColorBuffer


        struct DXColorBuffer : public canvas::IColorBuffer
        {
        public:
            DXColorBuffer( const COMReference<surface_type>& rSurface,
                           const ::basegfx::B2IVector& rSize )
                : maSize(rSize)
                , maLockedRect{}
                , mpSurface(rSurface)
            {
            }

        // implementation of the 'IColorBuffer' interface
        public:

            virtual sal_uInt8* lock() const override;
            virtual void       unlock() const override;
            virtual sal_uInt32 getWidth() const override;
            virtual sal_uInt32 getHeight() const override;
            virtual sal_uInt32 getStride() const override;
            virtual Format     getFormat() const override;

        private:

            ::basegfx::B2IVector maSize;
            mutable D3DLOCKED_RECT maLockedRect;
            mutable COMReference<surface_type> mpSurface;
        };

        sal_uInt8* DXColorBuffer::lock() const
        {
            if(SUCCEEDED(mpSurface->LockRect(&maLockedRect,nullptr,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                return static_cast<sal_uInt8 *>(maLockedRect.pBits);
            return nullptr;
        }

        void DXColorBuffer::unlock() const
        {
            mpSurface->UnlockRect();
        }

        sal_uInt32 DXColorBuffer::getWidth() const
        {
            return maSize.getX();
        }

        sal_uInt32 DXColorBuffer::getHeight() const
        {
            return maSize.getY();
        }

        sal_uInt32 DXColorBuffer::getStride() const
        {
            return maLockedRect.Pitch;
        }

        canvas::IColorBuffer::Format DXColorBuffer::getFormat() const
        {
            return canvas::IColorBuffer::Format::X8R8G8B8;
        }


        // GDIColorBuffer


        struct GDIColorBuffer : public canvas::IColorBuffer
        {
        public:

            GDIColorBuffer( const BitmapSharedPtr&      rSurface,
                            const ::basegfx::B2IVector& rSize )
                : maSize(rSize)
                , aBmpData{}
                , mpGDIPlusBitmap(rSurface)
            {
            }

        // implementation of the 'IColorBuffer' interface
        public:

            virtual sal_uInt8* lock() const override;
            virtual void       unlock() const override;
            virtual sal_uInt32 getWidth() const override;
            virtual sal_uInt32 getHeight() const override;
            virtual sal_uInt32 getStride() const override;
            virtual Format     getFormat() const override;

        private:

            ::basegfx::B2IVector maSize;
            mutable Gdiplus::BitmapData aBmpData;
            BitmapSharedPtr mpGDIPlusBitmap;
        };

        sal_uInt8* GDIColorBuffer::lock() const
        {
            aBmpData.Width = maSize.getX();
            aBmpData.Height = maSize.getY();
            aBmpData.Stride = 4*aBmpData.Width;
            aBmpData.PixelFormat = PixelFormat32bppARGB;
            aBmpData.Scan0 = nullptr;
            const Gdiplus::Rect aRect( 0,0,aBmpData.Width,aBmpData.Height );
            if( Gdiplus::Ok != mpGDIPlusBitmap->LockBits( &aRect,
                                                          Gdiplus::ImageLockModeRead,
                                                          PixelFormat32bppARGB,
                                                          &aBmpData ) )
            {
                return nullptr;
            }

            return static_cast<sal_uInt8*>(aBmpData.Scan0);
        }

        void GDIColorBuffer::unlock() const
        {
            mpGDIPlusBitmap->UnlockBits( &aBmpData );
        }

        sal_uInt32 GDIColorBuffer::getWidth() const
        {
            return maSize.getX();
        }

        sal_uInt32 GDIColorBuffer::getHeight() const
        {
            return maSize.getY();
        }

        sal_uInt32 GDIColorBuffer::getStride() const
        {
            return aBmpData.Stride;
        }

        canvas::IColorBuffer::Format GDIColorBuffer::getFormat() const
        {
            return canvas::IColorBuffer::Format::A8R8G8B8;
        }
    }


    // DXSurfaceBitmap::DXSurfaceBitmap


    DXSurfaceBitmap::DXSurfaceBitmap( const ::basegfx::B2IVector&                   rSize,
                                      const std::shared_ptr<canvas::ISurfaceProxyManager>&  rMgr,
                                      const IDXRenderModuleSharedPtr&               rRenderModule,
                                      bool                                          bWithAlpha ) :
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        maSize(rSize),
        mpRenderModule(rRenderModule),
        mpSurfaceManager(rMgr),
        mpSurfaceProxy(),
        mpSurface(),
        mpGDIPlusBitmap(),
        mpGraphics(),
        mpColorBuffer(),
        mbIsSurfaceDirty(true),
        mbAlpha(bWithAlpha)
    {
        init();
    }


    // DXSurfaceBitmap::getSize


    ::basegfx::B2IVector DXSurfaceBitmap::getSize() const
    {
        return maSize;
    }


    // DXSurfaceBitmap::init


    void DXSurfaceBitmap::init()
    {
        // create container for pixel data
        if(mbAlpha)
        {
            mpGDIPlusBitmap.reset(
                new Gdiplus::Bitmap(
                    maSize.getX(),
                    maSize.getY(),
                    PixelFormat32bppARGB
                    ));
            mpGraphics.reset( tools::createGraphicsFromBitmap(mpGDIPlusBitmap) );

            // create the colorbuffer object, which is basically a simple
            // wrapper around the directx surface. the colorbuffer is the
            // interface which is used by the surfaceproxy to support any
            // kind of underlying structure for the pixel data container.
            mpColorBuffer.reset(new GDIColorBuffer(mpGDIPlusBitmap,maSize));
        }
        else
        {
            mpSurface = mpRenderModule->createSystemMemorySurface(maSize);

            // create the colorbuffer object, which is basically a simple
            // wrapper around the directx surface. the colorbuffer is the
            // interface which is used by the surfaceproxy to support any
            // kind of underlying structure for the pixel data container.
            mpColorBuffer.reset(new DXColorBuffer(mpSurface,maSize));
        }

        // create a (possibly hardware accelerated) mirror surface.
        mpSurfaceProxy = mpSurfaceManager->createSurfaceProxy(mpColorBuffer);
    }


    // DXSurfaceBitmap::resize


    bool DXSurfaceBitmap::resize( const ::basegfx::B2IVector& rSize )
    {
        if(maSize != rSize)
        {
            maSize = rSize;
            init();
        }

        return true;
    }


    // DXSurfaceBitmap::clear


    void DXSurfaceBitmap::clear()
    {
        GraphicsSharedPtr pGraphics(getGraphics());
        Gdiplus::Color transColor(255,0,0,0);
        pGraphics->SetCompositingMode( Gdiplus::CompositingModeSourceCopy );
        pGraphics->Clear( transColor );
    }


    // DXSurfaceBitmap::hasAlpha


    bool DXSurfaceBitmap::hasAlpha() const
    {
        return mbAlpha;
    }


    // DXSurfaceBitmap::getGraphics


    GraphicsSharedPtr DXSurfaceBitmap::getGraphics()
    {
        // since clients will most probably draw directly
        // to the GDI+ bitmap, we need to mark it as dirty
        // to ensure that the corresponding dxsurface will
        // be updated.
        mbIsSurfaceDirty = true;

        if(hasAlpha())
            return mpGraphics;
        else
            return createSurfaceGraphics(mpSurface);
    }


    // DXSurfaceBitmap::getBitmap


    BitmapSharedPtr DXSurfaceBitmap::getBitmap() const
    {
        if(hasAlpha())
            return mpGDIPlusBitmap;

        BitmapSharedPtr pResult;

        D3DLOCKED_RECT aLockedRect;
        if(SUCCEEDED(mpSurface->LockRect(&aLockedRect,nullptr,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
        {
            // decide about the format we pass the gdi+, the directx surface is always
            // 32bit, either with or without alpha component.
            Gdiplus::PixelFormat nFormat = hasAlpha() ? PixelFormat32bppARGB : PixelFormat32bppRGB;

            // construct a gdi+ bitmap from the raw pixel data.
            pResult.reset(new Gdiplus::Bitmap( maSize.getX(),maSize.getY(),
                                                aLockedRect.Pitch,
                                                nFormat,
                                                static_cast<BYTE *>(aLockedRect.pBits) ));

            mpSurface->UnlockRect();
        }

        return pResult;
    }


    // DXSurfaceBitmap::draw


    bool DXSurfaceBitmap::draw( double                           fAlpha,
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


    // DXSurfaceBitmap::draw


    bool DXSurfaceBitmap::draw( double                         fAlpha,
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


    // DXSurfaceBitmap::draw


    bool DXSurfaceBitmap::draw( double                         fAlpha,
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


    // DXSurfaceBitmap::draw


    bool DXSurfaceBitmap::draw( const ::basegfx::B2IRange& rArea )
    {
        if( mbIsSurfaceDirty )
        {
            mpSurfaceProxy->setColorBufferDirty();
            mbIsSurfaceDirty = false;
        }

        const double                  fAlpha(1.0);
        const ::basegfx::B2DHomMatrix aTransform;
        const ::basegfx::B2DRange     aIEEEArea( rArea );
        return mpSurfaceProxy->draw(fAlpha,
                                    ::basegfx::B2DPoint(),
                                    aIEEEArea,
                                    aTransform);
    }


    // DXSurfaceBitmap::getData


    uno::Sequence< sal_Int8 > DXSurfaceBitmap::getData( rendering::IntegerBitmapLayout& rBitmapLayout,
                                                        const geometry::IntegerRectangle2D& rect )
    {
        if(hasAlpha())
        {
            uno::Sequence< sal_Int8 > aRes( (rect.X2-rect.X1)*(rect.Y2-rect.Y1)*4 ); // TODO(F1): Be format-agnostic here

            const Gdiplus::Rect aRect( tools::gdiPlusRectFromIntegerRectangle2D( rect ) );

            Gdiplus::BitmapData aBmpData;
            aBmpData.Width       = rect.X2-rect.X1;
            aBmpData.Height      = rect.Y2-rect.Y1;
            aBmpData.Stride      = 4*aBmpData.Width;
            aBmpData.PixelFormat = PixelFormat32bppARGB;
            aBmpData.Scan0       = aRes.getArray();

            // TODO(F1): Support more pixel formats natively

            // read data from bitmap
            if( Gdiplus::Ok != mpGDIPlusBitmap->LockBits( &aRect,
                                                Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf,
                                                PixelFormat32bppARGB, // TODO(F1): Adapt to
                                                                           // Graphics native
                                                                        // format/change
                                                                        // getMemoryLayout
                                                &aBmpData ) )
            {
                // failed to lock, bail out
                return uno::Sequence< sal_Int8 >();
            }

            mpGDIPlusBitmap->UnlockBits( &aBmpData );

            return aRes;
        }
        else
        {
            sal_uInt32 nWidth = rect.X2-rect.X1;
            sal_uInt32 nHeight = rect.Y2-rect.Y1;

            uno::Sequence< sal_Int8 > aRes(nWidth*nHeight*4);

            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,nullptr,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                return uno::Sequence< sal_Int8 >();
            D3DSURFACE_DESC aDesc;
            if(FAILED(mpSurface->GetDesc(&aDesc)))
                return uno::Sequence< sal_Int8 >();

            assert(aDesc.Format == D3DFMT_A8R8G8B8 || aDesc.Format == D3DFMT_X8R8G8B8);

            sal_uInt8 *pSrc = (static_cast<BYTE *>(aLockedRect.pBits)+(rect.Y1*aLockedRect.Pitch))+rect.X1;
            sal_uInt8 *pDst = reinterpret_cast<sal_uInt8 *>(aRes.getArray());
            sal_uInt32 nSegmentSizeInBytes = nWidth*4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                memcpy(pDst,pSrc,nSegmentSizeInBytes);
                pDst += nSegmentSizeInBytes;
                pSrc += aLockedRect.Pitch;
            }

            if(rBitmapLayout.ColorSpace->getComponentTags().getArray()[0] == rendering::ColorComponentTag::RGB_RED &&
               rBitmapLayout.ColorSpace->getComponentTags().getArray()[2] == rendering::ColorComponentTag::RGB_BLUE)
            {
                pDst = reinterpret_cast<sal_uInt8 *>(aRes.getArray());
                for(sal_uInt32 y=0; y<nHeight; ++y)
                {
                    sal_uInt8* pPixel = pDst;
                    for(sal_uInt32 n = 0; n<nWidth; n++)
                    {
                        sal_uInt8 nB = pPixel[0];
                        pPixel[0] = pPixel[2];
                        pPixel[2] = nB;
                        pPixel += 4;
                    }
                    pDst += nSegmentSizeInBytes;
                }
            }

            mpSurface->UnlockRect();
            return aRes;
        }
    }


    // DXSurfaceBitmap::setData


    void DXSurfaceBitmap::setData( const uno::Sequence< sal_Int8 >&      data,
                                   const rendering::IntegerBitmapLayout& /*bitmapLayout*/,
                                   const geometry::IntegerRectangle2D&   rect )
    {
        if(hasAlpha())
        {
            const Gdiplus::Rect aRect( tools::gdiPlusRectFromIntegerRectangle2D( rect ) );

            Gdiplus::BitmapData aBmpData;
            aBmpData.Width       = rect.X2-rect.X1;
            aBmpData.Height      = rect.Y2-rect.Y1;
            aBmpData.Stride      = 4*aBmpData.Width;
            aBmpData.PixelFormat = PixelFormat32bppARGB;
            aBmpData.Scan0       = const_cast<sal_Int8 *>(data.getConstArray());

            // TODO(F1): Support more pixel formats natively

            if( Gdiplus::Ok != mpGDIPlusBitmap->LockBits( &aRect,
                                                Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeUserInputBuf,
                                                PixelFormat32bppARGB, // TODO: Adapt to
                                                                           // Graphics native
                                                                          // format/change
                                                                          // getMemoryLayout
                                                &aBmpData ) )
            {
                throw uno::RuntimeException();
            }

            // commit data to bitmap
            mpGDIPlusBitmap->UnlockBits( &aBmpData );
        }
        else
        {
            sal_uInt32 nWidth = rect.X2-rect.X1;
            sal_uInt32 nHeight = rect.Y2-rect.Y1;

            // lock the directx surface to receive the pointer to the surface memory.
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,nullptr,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                throw uno::RuntimeException();

            sal_uInt8 const *pSrc = reinterpret_cast<sal_uInt8 const *>(data.getConstArray());
            sal_uInt8 *pDst = (static_cast<BYTE *>(aLockedRect.pBits)+(rect.Y1*aLockedRect.Pitch))+rect.X1;
            sal_uInt32 nSegmentSizeInBytes = nWidth<<4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                memcpy(pDst,pSrc,nSegmentSizeInBytes);
                pSrc += nSegmentSizeInBytes;
                pDst += aLockedRect.Pitch;
            }

            mpSurface->UnlockRect();
        }

        mbIsSurfaceDirty = true;
    }


    // DXSurfaceBitmap::setPixel


    void DXSurfaceBitmap::setPixel( const uno::Sequence< sal_Int8 >&      color,
                                    const rendering::IntegerBitmapLayout& /*bitmapLayout*/,
                                    const geometry::IntegerPoint2D&       pos )
    {
        if(hasAlpha())
        {
            const geometry::IntegerSize2D aSize( maSize.getX(),maSize.getY() );

            ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aSize.Width,
                            "CanvasHelper::setPixel: X coordinate out of bounds" );
            ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aSize.Height,
                            "CanvasHelper::setPixel: Y coordinate out of bounds" );
            ENSURE_ARG_OR_THROW( color.getLength() > 3,
                            "CanvasHelper::setPixel: not enough color components" );

            if( Gdiplus::Ok != mpGDIPlusBitmap->SetPixel( pos.X, pos.Y,
                                                Gdiplus::Color( tools::sequenceToArgb( color ))))
            {
                throw uno::RuntimeException();
            }
        }
        else
        {
            ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < maSize.getX(),
                            "CanvasHelper::setPixel: X coordinate out of bounds" );
            ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < maSize.getY(),
                            "CanvasHelper::setPixel: Y coordinate out of bounds" );
            ENSURE_ARG_OR_THROW( color.getLength() > 3,
                            "CanvasHelper::setPixel: not enough color components" );

            Gdiplus::Color aColor(tools::sequenceToArgb(color));

            // lock the directx surface to receive the pointer to the surface memory.
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,nullptr,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = reinterpret_cast<sal_uInt32 *>((static_cast<BYTE *>(aLockedRect.pBits)+(pos.Y*aLockedRect.Pitch))+pos.X);
            *pDst = aColor.GetValue();
            mpSurface->UnlockRect();
        }

        mbIsSurfaceDirty = true;
    }


    // DXSurfaceBitmap::getPixel


    uno::Sequence< sal_Int8 > DXSurfaceBitmap::getPixel( rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                                         const geometry::IntegerPoint2D&   pos )
    {
        if(hasAlpha())
        {
            const geometry::IntegerSize2D aSize( maSize.getX(),maSize.getY() );

            ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aSize.Width,
                            "CanvasHelper::getPixel: X coordinate out of bounds" );
            ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aSize.Height,
                            "CanvasHelper::getPixel: Y coordinate out of bounds" );

            Gdiplus::Color aColor;

            if( Gdiplus::Ok != mpGDIPlusBitmap->GetPixel( pos.X, pos.Y, &aColor ) )
                return uno::Sequence< sal_Int8 >();

            return tools::argbToIntSequence(aColor.GetValue());
        }
        else
        {
            ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < maSize.getX(),
                            "CanvasHelper::getPixel: X coordinate out of bounds" );
            ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < maSize.getY(),
                            "CanvasHelper::getPixel: Y coordinate out of bounds" );

            // lock the directx surface to receive the pointer to the surface memory.
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,nullptr,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = reinterpret_cast<sal_uInt32 *>((static_cast<BYTE *>(aLockedRect.pBits)+(pos.Y*aLockedRect.Pitch))+pos.X);
            Gdiplus::Color aColor(*pDst);
            mpSurface->UnlockRect();

            return tools::argbToIntSequence(aColor.GetValue());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
