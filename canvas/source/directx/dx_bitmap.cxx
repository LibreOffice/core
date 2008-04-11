/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_bitmap.cxx,v $
 * $Revision: 1.3 $
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

#include "dx_bitmap.hxx"
#include "dx_impltools.hxx"
#include "dx_surfacegraphics.hxx"

#include <canvas/debug.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2irange.hxx>

#if defined(DX_DEBUG_IMAGES) && defined(DBG_UTIL)
# include <imdebug.h>
# undef min
# undef max
#endif

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        //////////////////////////////////////////////////////////////////////////////////
        // DXColorBuffer
        //////////////////////////////////////////////////////////////////////////////////

        struct DXColorBuffer : public canvas::IColorBuffer
        {
        public:
            DXColorBuffer( const COMReference<surface_type>& rSurface,
                           const ::basegfx::B2IVector&       rSize ) :
                mpSurface(rSurface),
                maSize(rSize),
                mbAlpha(false)
            {
            }

        // implementation of the 'IColorBuffer' interface
        public:

            virtual sal_uInt8* lock() const;
            virtual void       unlock() const;
            virtual sal_uInt32 getWidth() const;
            virtual sal_uInt32 getHeight() const;
            virtual sal_uInt32 getStride() const;
            virtual Format     getFormat() const;

        private:

            ::basegfx::B2IVector maSize;
#if DIRECTX_VERSION < 0x0900
            mutable DDSURFACEDESC aSurfaceDesc;
#else
            mutable D3DLOCKED_RECT maLockedRect;
#endif
            mutable COMReference<surface_type> mpSurface;
            bool mbAlpha;
        };

        sal_uInt8* DXColorBuffer::lock() const
        {
#if DIRECTX_VERSION < 0x0900
            rtl_fillMemory((void *)&aSurfaceDesc,sizeof(DDSURFACEDESC),0);
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY;
            if(SUCCEEDED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                return static_cast<sal_uInt8 *>(aSurfaceDesc.lpSurface);
#else
            if(SUCCEEDED(mpSurface->LockRect(&maLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                return static_cast<sal_uInt8 *>(maLockedRect.pBits);
#endif
            return NULL;
        }

        void DXColorBuffer::unlock() const
        {
#if DIRECTX_VERSION < 0x0900
            mpSurface->Unlock(NULL);
#else
            mpSurface->UnlockRect();
#endif
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
#if DIRECTX_VERSION < 0x0900
            return aSurfaceDesc.lPitch;
#else
            return maLockedRect.Pitch;
#endif
        }

        canvas::IColorBuffer::Format DXColorBuffer::getFormat() const
        {
            return canvas::IColorBuffer::FMT_X8R8G8B8;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // GDIColorBuffer
        //////////////////////////////////////////////////////////////////////////////////

        struct GDIColorBuffer : public canvas::IColorBuffer
        {
        public:

            GDIColorBuffer( const BitmapSharedPtr&      rSurface,
                            const ::basegfx::B2IVector& rSize ) :
                mpGDIPlusBitmap(rSurface),
                maSize(rSize),
                mbAlpha(true)
            {
            }

        // implementation of the 'IColorBuffer' interface
        public:

            virtual sal_uInt8* lock() const;
            virtual void       unlock() const;
            virtual sal_uInt32 getWidth() const;
            virtual sal_uInt32 getHeight() const;
            virtual sal_uInt32 getStride() const;
            virtual Format     getFormat() const;

        private:

            ::basegfx::B2IVector maSize;
            mutable Gdiplus::BitmapData aBmpData;
            BitmapSharedPtr mpGDIPlusBitmap;
            bool mbAlpha;
        };

        sal_uInt8* GDIColorBuffer::lock() const
        {
            aBmpData.Width = maSize.getX();
            aBmpData.Height = maSize.getY();
            aBmpData.Stride = 4*aBmpData.Width;
            aBmpData.PixelFormat = PixelFormat32bppARGB;
            aBmpData.Scan0 = NULL;
            const Gdiplus::Rect aRect( 0,0,aBmpData.Width,aBmpData.Height );
            if( Gdiplus::Ok != mpGDIPlusBitmap->LockBits( &aRect,
                                                          Gdiplus::ImageLockModeRead,
                                                          PixelFormat32bppARGB,
                                                          &aBmpData ) )
            {
                return NULL;
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
            return canvas::IColorBuffer::FMT_A8R8G8B8;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::DXBitmap
    //////////////////////////////////////////////////////////////////////////////////

    DXBitmap::DXBitmap( const ::basegfx::B2IVector&                     rSize,
                        const canvas::ISurfaceProxyManagerSharedPtr&    rMgr,
                        const IDXRenderModuleSharedPtr&                 rRenderModule,
                        bool                                            bWithAlpha ) :
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        maSize(rSize),
        mpRenderModule(rRenderModule),
        mpSurfaceManager(rMgr),
        mpSurfaceProxy(),
        mpSurface(),
        mpGDIPlusBitmap(),
        mpColorBuffer(),
        mbIsSurfaceDirty(true),
        mbAlpha(bWithAlpha)
    {
        init();
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::init
    //////////////////////////////////////////////////////////////////////////////////

    void DXBitmap::init()
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

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::resize
    //////////////////////////////////////////////////////////////////////////////////

    bool DXBitmap::resize( const ::basegfx::B2IVector& rSize )
    {
        if(maSize != rSize)
        {
            maSize = rSize;
            init();
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::clear
    //////////////////////////////////////////////////////////////////////////////////

    void DXBitmap::clear()
    {
        SurfaceGraphicsSharedPtr aGraphics(getGraphics());
        Gdiplus::Color transColor(255,0,0,0);
        (*aGraphics.get())->SetCompositingMode( Gdiplus::CompositingModeSourceCopy );
        (*aGraphics.get())->Clear( transColor );
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::hasAlpha
    //////////////////////////////////////////////////////////////////////////////////

    bool DXBitmap::hasAlpha() const
    {
        return mbAlpha;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::getGraphics
    //////////////////////////////////////////////////////////////////////////////////

    SurfaceGraphicsSharedPtr DXBitmap::getGraphics() const
    {
        // since clients will most probably draw directly
        // to the GDI+ bitmap, we need to mark it as dirty
        // to ensure that the corrosponding dxsurface will
        // be updated.
        mbIsSurfaceDirty = true;

        if(hasAlpha())
            return SurfaceGraphicsSharedPtr(
                new SurfaceGraphics( mpGDIPlusBitmap ));

        return SurfaceGraphicsSharedPtr(
            new SurfaceGraphics( mpSurface ) );
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::getBitmap
    //////////////////////////////////////////////////////////////////////////////////

    BitmapSharedPtr DXBitmap::getBitmap()
    {
        if(hasAlpha())
            return mpGDIPlusBitmap;

        BitmapSharedPtr pResult;

#if DIRECTX_VERSION < 0x0900
        DDSURFACEDESC aSurfaceDesc;
        rtl_fillMemory(&aSurfaceDesc,sizeof(DDSURFACEDESC),0);
        aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
        const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY;

        // lock the directx surface to receive the pointer to the surface memory.
        if(SUCCEEDED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
        {
            // decide about the format we pass the gdi+, the directx surface is always
            // 32bit, either with or without alpha component.
            Gdiplus::PixelFormat nFormat = hasAlpha() ? PixelFormat32bppARGB : PixelFormat32bppRGB;

            // construct a gdi+ bitmap from the raw pixel data.
            pResult.reset(new Gdiplus::Bitmap( maSize.getX(),maSize.getY(),
                                               aSurfaceDesc.lPitch,
                                               nFormat,
                                               (BYTE *)aSurfaceDesc.lpSurface ));

            // unlock the directx surface
            mpSurface->Unlock(NULL);
        }
#else
        D3DLOCKED_RECT aLockedRect;
        if(SUCCEEDED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
        {
            // decide about the format we pass the gdi+, the directx surface is always
            // 32bit, either with or without alpha component.
            Gdiplus::PixelFormat nFormat = hasAlpha() ? PixelFormat32bppARGB : PixelFormat32bppRGB;

            // construct a gdi+ bitmap from the raw pixel data.
            pResult.reset(new Gdiplus::Bitmap( maSize.getX(),maSize.getY(),
                                                aLockedRect.Pitch,
                                                nFormat,
                                                (BYTE *)aLockedRect.pBits ));

            mpSurface->UnlockRect();
        }
#endif

        return pResult;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::draw
    //////////////////////////////////////////////////////////////////////////////////

    bool DXBitmap::draw( double                           fAlpha,
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

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::draw
    //////////////////////////////////////////////////////////////////////////////////

    bool DXBitmap::draw( double                         fAlpha,
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

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::draw
    //////////////////////////////////////////////////////////////////////////////////

    bool DXBitmap::draw( double                         fAlpha,
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

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::draw
    //////////////////////////////////////////////////////////////////////////////////

    bool DXBitmap::draw( const ::basegfx::B2IRange& rArea )
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

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::imageDebugger
    //////////////////////////////////////////////////////////////////////////////////
#if defined(DX_DEBUG_IMAGES) && defined(DBG_UTIL)
    void DXBitmap::imageDebugger()
    {
        DDSURFACEDESC aSurfaceDesc;
        rtl_fillMemory( &aSurfaceDesc,sizeof(DDSURFACEDESC),0 );
        aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

        if( FAILED(mpSurface->Lock( NULL,
                                    &aSurfaceDesc,
                                    DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY,
                                    NULL)) )
            return;

            imdebug("bgra w=%d h=%d %p", aSurfaceDesc.dwWidth, aSurfaceDesc.dwHeight, aSurfaceDesc.lpSurface);

            mpSurface->Unlock(NULL);
        }
    }
#endif

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::getData
    //////////////////////////////////////////////////////////////////////////////////

    uno::Sequence< sal_Int8 > DXBitmap::getData( rendering::IntegerBitmapLayout&     /*bitmapLayout*/,
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

#if DIRECTX_VERSION < 0x0900
            DDSURFACEDESC aSurfaceDesc;
            rtl_fillMemory(&aSurfaceDesc,sizeof(DDSURFACEDESC),0);
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY;

            // lock the directx surface to receive the pointer to the surface memory.
            if(FAILED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                return uno::Sequence< sal_Int8 >();

            sal_uInt8 *pSrc = (sal_uInt8 *)((((BYTE *)aSurfaceDesc.lpSurface)+(rect.Y1*aSurfaceDesc.lPitch))+rect.X1);
            sal_uInt8 *pDst = (sal_uInt8 *)aRes.getArray();
            sal_uInt32 nSegmentSizeInBytes = nWidth<<4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                rtl_copyMemory(pDst,pSrc,nSegmentSizeInBytes);
                pDst += nSegmentSizeInBytes;
                pSrc += aSurfaceDesc.lPitch;
            }

            mpSurface->Unlock(NULL);
#else
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                return uno::Sequence< sal_Int8 >();

            sal_uInt8 *pSrc = (sal_uInt8 *)((((BYTE *)aLockedRect.pBits)+(rect.Y1*aLockedRect.Pitch))+rect.X1);
            sal_uInt8 *pDst = (sal_uInt8 *)aRes.getArray();
            sal_uInt32 nSegmentSizeInBytes = nWidth<<4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                rtl_copyMemory(pDst,pSrc,nSegmentSizeInBytes);
                pDst += nSegmentSizeInBytes;
                pSrc += aLockedRect.Pitch;
            }

            mpSurface->UnlockRect();
#endif
            return aRes;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::setData
    //////////////////////////////////////////////////////////////////////////////////

    void DXBitmap::setData( const uno::Sequence< sal_Int8 >&        data,
                            const rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                            const geometry::IntegerRectangle2D&     rect )
    {
        if(hasAlpha())
        {
            const Gdiplus::Rect aRect( tools::gdiPlusRectFromIntegerRectangle2D( rect ) );

            Gdiplus::BitmapData aBmpData;
            aBmpData.Width       = rect.X2-rect.X1;
            aBmpData.Height      = rect.Y2-rect.Y1;
            aBmpData.Stride      = 4*aBmpData.Width;
            aBmpData.PixelFormat = PixelFormat32bppARGB;
            aBmpData.Scan0       = (void*)data.getConstArray();

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

#if DIRECTX_VERSION < 0x0900
            DDSURFACEDESC aSurfaceDesc;
            rtl_fillMemory(&aSurfaceDesc,sizeof(DDSURFACEDESC),0);
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY;

            // lock the directx surface to receive the pointer to the surface memory.
            if(FAILED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                throw uno::RuntimeException();

            sal_uInt8 *pSrc = (sal_uInt8 *)data.getConstArray();
            sal_uInt8 *pDst = (sal_uInt8 *)((((BYTE *)aSurfaceDesc.lpSurface)+(rect.Y1*aSurfaceDesc.lPitch))+rect.X1);
            sal_uInt32 nSegmentSizeInBytes = nWidth<<4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                rtl_copyMemory(pDst,pSrc,nSegmentSizeInBytes);
                pSrc += nSegmentSizeInBytes;
                pDst += aSurfaceDesc.lPitch;
            }

            mpSurface->Unlock(NULL);
#else
            // lock the directx surface to receive the pointer to the surface memory.
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                throw uno::RuntimeException();

            sal_uInt8 *pSrc = (sal_uInt8 *)data.getConstArray();
            sal_uInt8 *pDst = (sal_uInt8 *)((((BYTE *)aLockedRect.pBits)+(rect.Y1*aLockedRect.Pitch))+rect.X1);
            sal_uInt32 nSegmentSizeInBytes = nWidth<<4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                rtl_copyMemory(pDst,pSrc,nSegmentSizeInBytes);
                pSrc += nSegmentSizeInBytes;
                pDst += aLockedRect.Pitch;
            }

            mpSurface->UnlockRect();
#endif
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::setPixel
    //////////////////////////////////////////////////////////////////////////////////

    void DXBitmap::setPixel( const uno::Sequence< sal_Int8 >&       color,
                             const rendering::IntegerBitmapLayout&  /*bitmapLayout*/,
                             const geometry::IntegerPoint2D&        pos )
    {
        if(hasAlpha())
        {
            const geometry::IntegerSize2D aSize( maSize.getX(),maSize.getY() );

            CHECK_AND_THROW( pos.X >= 0 && pos.X < aSize.Width,
                            "CanvasHelper::setPixel: X coordinate out of bounds" );
            CHECK_AND_THROW( pos.Y >= 0 && pos.Y < aSize.Height,
                            "CanvasHelper::setPixel: Y coordinate out of bounds" );
            CHECK_AND_THROW( color.getLength() > 3,
                            "CanvasHelper::setPixel: not enough color components" );

            if( Gdiplus::Ok != mpGDIPlusBitmap->SetPixel( pos.X, pos.Y,
                                                Gdiplus::Color( tools::sequenceToArgb( color ))))
            {
                throw uno::RuntimeException();
            }
        }
        else
        {
            CHECK_AND_THROW( pos.X >= 0 && pos.X < maSize.getX(),
                            "CanvasHelper::setPixel: X coordinate out of bounds" );
            CHECK_AND_THROW( pos.Y >= 0 && pos.Y < maSize.getY(),
                            "CanvasHelper::setPixel: Y coordinate out of bounds" );
            CHECK_AND_THROW( color.getLength() > 3,
                            "CanvasHelper::setPixel: not enough color components" );

            Gdiplus::Color aColor(tools::sequenceToArgb(color));

#if DIRECTX_VERSION < 0x0900
            DDSURFACEDESC aSurfaceDesc;
            rtl_fillMemory(&aSurfaceDesc,sizeof(DDSURFACEDESC),0);
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY;

            // lock the directx surface to receive the pointer to the surface memory.
            if(FAILED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = (sal_uInt32 *)((((BYTE *)aSurfaceDesc.lpSurface)+(pos.Y*aSurfaceDesc.lPitch))+pos.X);
            *pDst = aColor.GetValue();
            mpSurface->Unlock(NULL);
#else
            // lock the directx surface to receive the pointer to the surface memory.
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = (sal_uInt32 *)((((BYTE *)aLockedRect.pBits)+(pos.Y*aLockedRect.Pitch))+pos.X);
            *pDst = aColor.GetValue();
            mpSurface->UnlockRect();
#endif
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // DXBitmap::getPixel
    //////////////////////////////////////////////////////////////////////////////////

uno::Sequence< sal_Int8 > DXBitmap::getPixel( rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                                  const geometry::IntegerPoint2D&   pos )
    {
        if(hasAlpha())
        {
            const geometry::IntegerSize2D aSize( maSize.getX(),maSize.getY() );

            CHECK_AND_THROW( pos.X >= 0 && pos.X < aSize.Width,
                            "CanvasHelper::getPixel: X coordinate out of bounds" );
            CHECK_AND_THROW( pos.Y >= 0 && pos.Y < aSize.Height,
                            "CanvasHelper::getPixel: Y coordinate out of bounds" );

            Gdiplus::Color aColor;

            if( Gdiplus::Ok != mpGDIPlusBitmap->GetPixel( pos.X, pos.Y, &aColor ) )
                return uno::Sequence< sal_Int8 >();

            return tools::argbToIntSequence(aColor.GetValue());
        }
        else
        {
            CHECK_AND_THROW( pos.X >= 0 && pos.X < maSize.getX(),
                            "CanvasHelper::getPixel: X coordinate out of bounds" );
            CHECK_AND_THROW( pos.Y >= 0 && pos.Y < maSize.getY(),
                            "CanvasHelper::getPixel: Y coordinate out of bounds" );

#if DIRECTX_VERSION < 0x0900
            DDSURFACEDESC aSurfaceDesc;
            rtl_fillMemory(&aSurfaceDesc,sizeof(DDSURFACEDESC),0);
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY;

            // lock the directx surface to receive the pointer to the surface memory.
            if(FAILED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = (sal_uInt32 *)((((BYTE *)aSurfaceDesc.lpSurface)+(pos.Y*aSurfaceDesc.lPitch))+pos.X);
            Gdiplus::Color aColor(*pDst);
            mpSurface->Unlock(NULL);
#else
            // lock the directx surface to receive the pointer to the surface memory.
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = (sal_uInt32 *)((((BYTE *)aLockedRect.pBits)+(pos.Y*aLockedRect.Pitch))+pos.X);
            Gdiplus::Color aColor(*pDst);
            mpSurface->UnlockRect();
#endif

            return tools::argbToIntSequence(aColor.GetValue());
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // End of file
    //////////////////////////////////////////////////////////////////////////////////
}

