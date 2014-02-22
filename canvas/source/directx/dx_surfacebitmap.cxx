/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <string.h>

#include "dx_surfacebitmap.hxx"
#include "dx_impltools.hxx"
#include "dx_surfacegraphics.hxx"
#include "dx_graphicsprovider.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2irange.hxx>

#if defined(DX_DEBUG_IMAGES)
# if OSL_DEBUG_LEVEL > 0
#  include <imdebug.h>
#  undef min
#  undef max
# endif
#endif

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        
        
        

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
            memset((void *)&aSurfaceDesc, 0, sizeof(DDSURFACEDESC));
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

    
    
    

    DXSurfaceBitmap::DXSurfaceBitmap( const ::basegfx::B2IVector&                   rSize,
                                      const canvas::ISurfaceProxyManagerSharedPtr&  rMgr,
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

    
    
    

    ::basegfx::B2IVector DXSurfaceBitmap::getSize() const
    {
        return maSize;
    }

    
    
    

    void DXSurfaceBitmap::init()
    {
        
        if(mbAlpha)
        {
            mpGDIPlusBitmap.reset(
                new Gdiplus::Bitmap(
                    maSize.getX(),
                    maSize.getY(),
                    PixelFormat32bppARGB
                    ));
            mpGraphics.reset( tools::createGraphicsFromBitmap(mpGDIPlusBitmap) );

            
            
            
            
            mpColorBuffer.reset(new GDIColorBuffer(mpGDIPlusBitmap,maSize));
        }
        else
        {
            mpSurface = mpRenderModule->createSystemMemorySurface(maSize);

            
            
            
            
            mpColorBuffer.reset(new DXColorBuffer(mpSurface,maSize));
        }

        
        mpSurfaceProxy = mpSurfaceManager->createSurfaceProxy(mpColorBuffer);
    }

    
    
    

    bool DXSurfaceBitmap::resize( const ::basegfx::B2IVector& rSize )
    {
        if(maSize != rSize)
        {
            maSize = rSize;
            init();
        }

        return true;
    }

    
    
    

    void DXSurfaceBitmap::clear()
    {
        GraphicsSharedPtr pGraphics(getGraphics());
        Gdiplus::Color transColor(255,0,0,0);
        pGraphics->SetCompositingMode( Gdiplus::CompositingModeSourceCopy );
        pGraphics->Clear( transColor );
    }

    
    
    

    bool DXSurfaceBitmap::hasAlpha() const
    {
        return mbAlpha;
    }

    
    
    

    GraphicsSharedPtr DXSurfaceBitmap::getGraphics()
    {
        
        
        
        
        mbIsSurfaceDirty = true;

        if(hasAlpha())
            return mpGraphics;
        else
            return createSurfaceGraphics(mpSurface);
    }

    
    
    

    BitmapSharedPtr DXSurfaceBitmap::getBitmap() const
    {
        if(hasAlpha())
            return mpGDIPlusBitmap;

        BitmapSharedPtr pResult;

#if DIRECTX_VERSION < 0x0900
        DDSURFACEDESC aSurfaceDesc;
        memset(&aSurfaceDesc, 0, sizeof(DDSURFACEDESC));
        aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
        const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY;

        
        if(SUCCEEDED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
        {
            
            
            Gdiplus::PixelFormat nFormat = hasAlpha() ? PixelFormat32bppARGB : PixelFormat32bppRGB;

            
            pResult.reset(new Gdiplus::Bitmap( maSize.getX(),maSize.getY(),
                                               aSurfaceDesc.lPitch,
                                               nFormat,
                                               (BYTE *)aSurfaceDesc.lpSurface ));

            
            mpSurface->Unlock(NULL);
        }
#else
        D3DLOCKED_RECT aLockedRect;
        if(SUCCEEDED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
        {
            
            
            Gdiplus::PixelFormat nFormat = hasAlpha() ? PixelFormat32bppARGB : PixelFormat32bppRGB;

            
            pResult.reset(new Gdiplus::Bitmap( maSize.getX(),maSize.getY(),
                                                aLockedRect.Pitch,
                                                nFormat,
                                                (BYTE *)aLockedRect.pBits ));

            mpSurface->UnlockRect();
        }
#endif

        return pResult;
    }

    
    
    

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

    
    
    
#if defined(DX_DEBUG_IMAGES)
# if OSL_DEBUG_LEVEL > 0
    void DXSurfaceBitmap::imageDebugger()
    {
#if DIRECTX_VERSION < 0x0900
        DDSURFACEDESC aSurfaceDesc;
        memset( &aSurfaceDesc, 0, sizeof(DDSURFACEDESC) );
        aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

        if( FAILED(mpSurface->Lock( NULL,
                                    &aSurfaceDesc,
                                    DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY,
                                    NULL)) )
            return;

        imdebug("bgra w=%d h=%d %p", aSurfaceDesc.dwWidth, aSurfaceDesc.dwHeight, aSurfaceDesc.lpSurface);

        mpSurface->Unlock(NULL);
#else
        D3DLOCKED_RECT aLockedRect;
        if( FAILED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)) )
            return;

        imdebug("bgra w=%d h=%d %p", maSize.getX(),
                maSize.getY(), aLockedRect.pBits);
        mpSurface->UnlockRect();
#endif
    }
# endif
#endif

    
    
    

    uno::Sequence< sal_Int8 > DXSurfaceBitmap::getData( rendering::IntegerBitmapLayout&     /*bitmapLayout*/,
                                                        const geometry::IntegerRectangle2D& rect )
    {
        if(hasAlpha())
        {
            uno::Sequence< sal_Int8 > aRes( (rect.X2-rect.X1)*(rect.Y2-rect.Y1)*4 ); 

            const Gdiplus::Rect aRect( tools::gdiPlusRectFromIntegerRectangle2D( rect ) );

            Gdiplus::BitmapData aBmpData;
            aBmpData.Width       = rect.X2-rect.X1;
            aBmpData.Height      = rect.Y2-rect.Y1;
            aBmpData.Stride      = 4*aBmpData.Width;
            aBmpData.PixelFormat = PixelFormat32bppARGB;
            aBmpData.Scan0       = aRes.getArray();

            

            
            if( Gdiplus::Ok != mpGDIPlusBitmap->LockBits( &aRect,
                                                Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf,
                                                PixelFormat32bppARGB, 
                                                                           
                                                                        
                                                                        
                                                &aBmpData ) )
            {
                
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
            memset(&aSurfaceDesc,0, sizeof(DDSURFACEDESC));
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY;

            
            if(FAILED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                return uno::Sequence< sal_Int8 >();

            sal_uInt8 *pSrc = (sal_uInt8 *)((((BYTE *)aSurfaceDesc.lpSurface)+(rect.Y1*aSurfaceDesc.lPitch))+rect.X1);
            sal_uInt8 *pDst = (sal_uInt8 *)aRes.getArray();
            sal_uInt32 nSegmentSizeInBytes = nWidth<<4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                memcpy(pDst,pSrc,nSegmentSizeInBytes);
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
                memcpy(pDst,pSrc,nSegmentSizeInBytes);
                pDst += nSegmentSizeInBytes;
                pSrc += aLockedRect.Pitch;
            }

            mpSurface->UnlockRect();
#endif
            return aRes;
        }
    }

    
    
    

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
            aBmpData.Scan0       = (void*)data.getConstArray();

            

            if( Gdiplus::Ok != mpGDIPlusBitmap->LockBits( &aRect,
                                                Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeUserInputBuf,
                                                PixelFormat32bppARGB, 
                                                                           
                                                                          
                                                                          
                                                &aBmpData ) )
            {
                throw uno::RuntimeException();
            }

            
            mpGDIPlusBitmap->UnlockBits( &aBmpData );
        }
        else
        {
            sal_uInt32 nWidth = rect.X2-rect.X1;
            sal_uInt32 nHeight = rect.Y2-rect.Y1;

#if DIRECTX_VERSION < 0x0900
            DDSURFACEDESC aSurfaceDesc;
            memset(&aSurfaceDesc, 0, sizeof(DDSURFACEDESC));
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY;

            
            if(FAILED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                throw uno::RuntimeException();

            sal_uInt8 *pSrc = (sal_uInt8 *)data.getConstArray();
            sal_uInt8 *pDst = (sal_uInt8 *)((((BYTE *)aSurfaceDesc.lpSurface)+(rect.Y1*aSurfaceDesc.lPitch))+rect.X1);
            sal_uInt32 nSegmentSizeInBytes = nWidth<<4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                memcpy(pDst,pSrc,nSegmentSizeInBytes);
                pSrc += nSegmentSizeInBytes;
                pDst += aSurfaceDesc.lPitch;
            }

            mpSurface->Unlock(NULL);
#else
            
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                throw uno::RuntimeException();

            sal_uInt8 *pSrc = (sal_uInt8 *)data.getConstArray();
            sal_uInt8 *pDst = (sal_uInt8 *)((((BYTE *)aLockedRect.pBits)+(rect.Y1*aLockedRect.Pitch))+rect.X1);
            sal_uInt32 nSegmentSizeInBytes = nWidth<<4;
            for(sal_uInt32 y=0; y<nHeight; ++y)
            {
                memcpy(pDst,pSrc,nSegmentSizeInBytes);
                pSrc += nSegmentSizeInBytes;
                pDst += aLockedRect.Pitch;
            }

            mpSurface->UnlockRect();
#endif
        }

        mbIsSurfaceDirty = true;
    }

    
    
    

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

#if DIRECTX_VERSION < 0x0900
            DDSURFACEDESC aSurfaceDesc;
            memset(&aSurfaceDesc, 0, sizeof(DDSURFACEDESC));
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY;

            
            if(FAILED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = (sal_uInt32 *)((((BYTE *)aSurfaceDesc.lpSurface)+(pos.Y*aSurfaceDesc.lPitch))+pos.X);
            *pDst = aColor.GetValue();
            mpSurface->Unlock(NULL);
#else
            
            D3DLOCKED_RECT aLockedRect;
            if(FAILED(mpSurface->LockRect(&aLockedRect,NULL,D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = (sal_uInt32 *)((((BYTE *)aLockedRect.pBits)+(pos.Y*aLockedRect.Pitch))+pos.X);
            *pDst = aColor.GetValue();
            mpSurface->UnlockRect();
#endif
        }

        mbIsSurfaceDirty = true;
    }

    
    
    

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

#if DIRECTX_VERSION < 0x0900
            DDSURFACEDESC aSurfaceDesc;
            memset(&aSurfaceDesc, 0, sizeof(DDSURFACEDESC));
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwFlags = DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY;

            
            if(FAILED(mpSurface->Lock(NULL,&aSurfaceDesc,dwFlags,NULL)))
                throw uno::RuntimeException();

            sal_uInt32 *pDst = (sal_uInt32 *)((((BYTE *)aSurfaceDesc.lpSurface)+(pos.Y*aSurfaceDesc.lPitch))+pos.X);
            Gdiplus::Color aColor(*pDst);
            mpSurface->Unlock(NULL);
#else
            
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
