/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#if DIRECTX_VERSION < 0x0900

// Nvidia GeForce Go 6800 crashes with a bluescreen if we take the
// maximum texture size, which would be twice as large. this behaviors
// has only been observed on directx5.
// This value is simply the maximum size for textures we request from
// the system, it has absolutely nothing to do with the size of primitives
// we're able to render, both concepts are totally independent from each other.
#define MAX_TEXTURE_SIZE (2048)
#define MIN_TEXTURE_SIZE (32)
//#define FAKE_MAX_NUMBER_TEXTURES (2)
//#define FAKE_MAX_TEXTURE_SIZE (512)

//////////////////////////////////////////////////////////////////////////////////
// includes
//////////////////////////////////////////////////////////////////////////////////
#include <vcl/syschild.hxx>
#include <vcl/window.hxx>
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/elapsedtime.hxx>
#include <canvas/canvastools.hxx>
#include <canvas/rendering/icolorbuffer.hxx>
#include <canvas/rendering/isurface.hxx>
#include <canvas/rendering/irendermodule.hxx>
#include <tools/diagnose_ex.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <boost/scoped_ptr.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>

#define COMPILE_MULTIMON_STUBS

#include "dx_rendermodule.hxx"
#include "dx_surfacegraphics.hxx"
#include <vcl/sysdata.hxx>

#undef WB_LEFT
#undef WB_RIGHT

#include "dx_impltools.hxx"
#include <malloc.h>

#if defined(DX_DEBUG_IMAGES)
# if OSL_DEBUG_LEVEL > 0
#  include <imdebug.h>
#  undef min
#  undef max
# endif
#endif

#undef COMPILE_MULTIMON_STUBS

#include <stdio.h>

#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////////
// 'dxcanvas' namespace
//////////////////////////////////////////////////////////////////////////////////

namespace dxcanvas
{
    namespace
    {
        bool doBlit( const ::basegfx::B2IPoint& rDestPos,
                     IDirectDrawSurface&        rOutSurface,
                     const ::basegfx::B2IRange& rSourceArea,
                     IDirectDrawSurface&        rSourceSurface,
                     DDBLTFX*                   pBltFx,
                     bool                       bForceSoftware )
        {
            if( !bForceSoftware )
            {
                // blit surface to backbuffer
                RECT aOutRect =
                    {
                        rDestPos.getX(),
                        rDestPos.getY(),
                        rDestPos.getX() + static_cast<sal_Int32>(rSourceArea.getWidth()),
                        rDestPos.getY() + static_cast<sal_Int32>(rSourceArea.getHeight()),
                    };
                RECT aSourceRect =
                    {
                        rSourceArea.getMinX(),
                        rSourceArea.getMinY(),
                        rSourceArea.getMaxX(),
                        rSourceArea.getMaxY()
                    };

                if( SUCCEEDED(rOutSurface.Blt( &aOutRect,
                                               &rSourceSurface,
                                               &aSourceRect,
                                               DDBLT_WAIT,
                                               pBltFx )) )
                {
                    return true;
                }
            }

            // failed, or forced to use SW copy. attempt manual copy.
            bool bResult = false;

            // lock source surface
            DDSURFACEDESC aDescSrc;
            rtl_fillMemory(&aDescSrc,sizeof(DDSURFACEDESC),0);
            aDescSrc.dwSize = sizeof(DDSURFACEDESC);
            const DWORD dwSrcFlags = DDLOCK_NOSYSLOCK|
                DDLOCK_SURFACEMEMORYPTR|
                DDLOCK_WAIT|
                DDLOCK_READONLY;
            if(SUCCEEDED(rSourceSurface.Lock(NULL,
                                             &aDescSrc,
                                             dwSrcFlags,
                                             NULL)))
            {
                // lock destination surface
                DDSURFACEDESC aDescDst;
                rtl_fillMemory(&aDescDst,sizeof(DDSURFACEDESC),0);
                aDescDst.dwSize = sizeof(DDSURFACEDESC);
                const DWORD dwDstFlags = DDLOCK_NOSYSLOCK|
                    DDLOCK_SURFACEMEMORYPTR|
                    DDLOCK_WAIT|
                    DDLOCK_WRITEONLY;
                if(SUCCEEDED(rOutSurface.Lock(NULL,
                                              &aDescDst,
                                              dwDstFlags,
                                              NULL)))
                {
                    sal_uInt32 nSrcFormat;
                    nSrcFormat  = ::canvas::tools::bitcount32(aDescSrc.ddpfPixelFormat.dwRGBAlphaBitMask)<<12;
                    nSrcFormat |= ::canvas::tools::bitcount32(aDescSrc.ddpfPixelFormat.dwRBitMask)<<8;
                    nSrcFormat |= ::canvas::tools::bitcount32(aDescSrc.ddpfPixelFormat.dwGBitMask)<<4;
                    nSrcFormat |= ::canvas::tools::bitcount32(aDescSrc.ddpfPixelFormat.dwBBitMask);

                    sal_uInt32 nDstFormat;
                    nDstFormat  = ::canvas::tools::bitcount32(aDescDst.ddpfPixelFormat.dwRGBAlphaBitMask)<<12;
                    nDstFormat |= ::canvas::tools::bitcount32(aDescDst.ddpfPixelFormat.dwRBitMask)<<8;
                    nDstFormat |= ::canvas::tools::bitcount32(aDescDst.ddpfPixelFormat.dwGBitMask)<<4;
                    nDstFormat |= ::canvas::tools::bitcount32(aDescDst.ddpfPixelFormat.dwBBitMask);

                    // TODO(E1): Use numeric_cast to catch overflow here
                    const sal_uInt32 nWidth( static_cast<sal_uInt32>(
                                                   rSourceArea.getWidth() ) );
                    const sal_uInt32 nHeight( static_cast<sal_uInt32>(
                                                    rSourceArea.getHeight() ) );

                    if((nSrcFormat == 0x8888) && (nDstFormat == 0x0565))
                    {
                        // medium range 8888 to 0565 pixel format conversion.
                        bResult = true;
                        sal_uInt8 *pSrcSurface = (sal_uInt8 *)aDescSrc.lpSurface +
                            rSourceArea.getMinY()*aDescSrc.lPitch +
                            (rSourceArea.getMinX()<<2);
                        sal_uInt8 *pDstSurface = (sal_uInt8 *)aDescDst.lpSurface +
                            rDestPos.getY()*aDescDst.lPitch +
                            (rDestPos.getX()<<1);
                        for(sal_uInt32 y=0; y<nHeight; ++y)
                        {
                            sal_uInt32 *pSrcScanline = (sal_uInt32 *)pSrcSurface;
                            sal_uInt16 *pDstScanline = (sal_uInt16 *)pDstSurface;
                            for(sal_uInt32 x=0; x<nWidth; ++x)
                            {
                                sal_uInt32 srcPixel = *pSrcScanline++;
                                sal_uInt16 dstPixel;
                                dstPixel  = (sal_uInt16)((srcPixel & 0x0000F8) >> 3);
                                dstPixel |= (srcPixel & 0x00FC00) >> 5;
                                dstPixel |= (srcPixel & 0xF80000) >> 8;
                                *pDstScanline++ = dstPixel;
                            }
                            pSrcSurface += aDescSrc.lPitch;
                            pDstSurface += aDescDst.lPitch;
                        }
                    }
                    else if((nSrcFormat == 0x8888) && (nDstFormat == 0x0888))
                    {
                        // medium range 8888 to 0888 pixel format conversion.
                        bResult = true;
                        sal_uInt8 *pSrcSurface = (sal_uInt8 *)aDescSrc.lpSurface +
                            rSourceArea.getMinY()*aDescSrc.lPitch +
                            (rSourceArea.getMinX()<<2);
                        sal_uInt8 *pDstSurface = (sal_uInt8 *)aDescDst.lpSurface +
                            rDestPos.getY()*aDescDst.lPitch +
                            (rDestPos.getX()<<2);
                        for(sal_uInt32 y=0; y<nHeight; ++y)
                        {
                            sal_uInt32 *pSrcScanline = (sal_uInt32 *)pSrcSurface;
                            sal_uInt16 *pDstScanline = (sal_uInt16 *)pDstSurface;
                            for(sal_uInt32 x=0; x<nWidth; ++x)
                            {
                                *pDstScanline++ = (sal_uInt16)*pSrcScanline++;
                            }
                            pSrcSurface += aDescSrc.lPitch;
                            pDstSurface += aDescDst.lPitch;
                        }
                    }
                    else if((nSrcFormat == 0x8888) && (nDstFormat == 0x1555))
                    {
                        // medium range 8888 to 1555 pixel format conversion.
                        bResult = true;
                        sal_uInt8 *pSrcSurface = (sal_uInt8 *)aDescSrc.lpSurface +
                            rSourceArea.getMinY()*aDescSrc.lPitch +
                            (rSourceArea.getMinX()<<2);
                        sal_uInt8 *pDstSurface = (sal_uInt8 *)aDescDst.lpSurface +
                            rDestPos.getY()*aDescDst.lPitch +
                            (rDestPos.getX()<<1);
                        for(sal_uInt32 y=0; y<nHeight; ++y)
                        {
                            sal_uInt32 *pSrcScanline = (sal_uInt32*)pSrcSurface;
                            sal_uInt16 *pDstScanline = (sal_uInt16 *)pDstSurface;
                            for(sal_uInt32 x=0; x<nWidth; ++x)
                            {
                                sal_uInt32 srcPixel = *pSrcScanline++;
                                sal_uInt16 dstPixel;
                                dstPixel  = (sal_uInt16)((srcPixel & 0x000000F8) >> 3);
                                dstPixel |= (srcPixel & 0x0000F800) >> 6;
                                dstPixel |= (srcPixel & 0x00F80000) >> 9;
                                dstPixel |= (srcPixel & 0x80000000) >> 16;
                                *pDstScanline++ = dstPixel;
                            }
                            pSrcSurface += aDescSrc.lPitch;
                            pDstSurface += aDescDst.lPitch;
                        }
                    }

                    // unlock destination surface
                    rOutSurface.Unlock(NULL);
                }

                // unlock source surface
                rSourceSurface.Unlock(NULL);
            }

            return bResult;
        }

        void dumpSurface( const COMReference<IDirectDrawSurface> &pSurface, const char *szFilename )
        {
            if(!(pSurface.get()))
                return;

            DDSURFACEDESC aSurfaceDesc;
            rtl_fillMemory( &aSurfaceDesc,sizeof(DDSURFACEDESC),0 );
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

            if( FAILED(pSurface->Lock( NULL,
                                        &aSurfaceDesc,
                                        DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY,
                                        NULL)) )
                return;

            const std::size_t dwBitmapSize(aSurfaceDesc.dwWidth*aSurfaceDesc.dwHeight*4);
            sal_uInt8 *pBuffer = static_cast<sal_uInt8 *>(_alloca(dwBitmapSize));
            if(pBuffer)
            {
                sal_uInt8 *pSource = reinterpret_cast<sal_uInt8 *>(aSurfaceDesc.lpSurface);
                sal_uInt8 *pDest = reinterpret_cast<sal_uInt8 *>(pBuffer);
                const std::size_t dwDestPitch(aSurfaceDesc.dwWidth<<2);
                pDest += aSurfaceDesc.dwHeight*dwDestPitch;
                for(sal_uInt32 y=0; y<aSurfaceDesc.dwHeight; ++y)
                {
                    pDest -= dwDestPitch;
                    rtl_copyMemory( pDest, pSource, dwDestPitch );
                    pSource += aSurfaceDesc.lPitch;
                }

                if(FILE *fp = fopen(szFilename,"wb"))
                {
                    BITMAPINFOHEADER bitmapInfo;

                    bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
                    bitmapInfo.biWidth = aSurfaceDesc.dwWidth;
                    bitmapInfo.biHeight = aSurfaceDesc.dwHeight;
                    bitmapInfo.biPlanes = 1;
                    bitmapInfo.biBitCount = 32;
                    bitmapInfo.biCompression = BI_RGB;
                    bitmapInfo.biSizeImage = 0;
                    bitmapInfo.biXPelsPerMeter = 0;
                    bitmapInfo.biYPelsPerMeter = 0;
                    bitmapInfo.biClrUsed = 0;
                    bitmapInfo.biClrImportant = 0;

                    const std::size_t dwFileSize(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwBitmapSize);

                    BITMAPFILEHEADER header;
                    header.bfType = 'MB';
                    header.bfSize = dwFileSize;
                    header.bfReserved1 = 0;
                    header.bfReserved2 = 0;
                    header.bfOffBits = sizeof(BITMAPFILEHEADER) + bitmapInfo.biSize;

                    fwrite(&header,1,sizeof(BITMAPFILEHEADER),fp);
                    fwrite(&bitmapInfo,1,sizeof(BITMAPINFOHEADER),fp);
                    fwrite(pBuffer,1,dwBitmapSize,fp);

                    fclose(fp);
                }
            }

            pSurface->Unlock(NULL);
        }

        void clearSurface( const COMReference<IDirectDrawSurface>& pSurface )
        {
            if(!(pSurface.is()))
                return;

            DDBLTFX aBltFx;

            rtl_fillMemory( &aBltFx,
                            sizeof(DDBLTFX), 0 );
            aBltFx.dwSize = sizeof(DDBLTFX);
            aBltFx.dwFillColor = 0;

            pSurface->Blt( NULL,
                           NULL,
                           NULL,
                           DDBLT_COLORFILL | DDBLT_WAIT,
                           &aBltFx );
        }

        // Define struct for MonitorEntry
        struct MonitorEntry
        {
             GUID                 mnGUID;
             HMONITOR             mhMonitor;
            MONITORINFO   maMonitorInfo;
        };

        // define type for MonitorList
        typedef ::std::vector< MonitorEntry > MonitorList;

         // Win32 system callback for DirectDrawEnumerateExA call
         BOOL WINAPI EnumerateExA_Callback( GUID FAR* lpGUID,
                                           LPSTR     /*lpDriverDescription*/,
                                           LPSTR     /*lpDriverName*/,
                                           LPVOID    lpContext,
                                           HMONITOR  hMonitor )
        {
            if(lpGUID)
            {
                 MonitorList* pMonitorList = (MonitorList*)lpContext;
                MonitorEntry aEntry;

                 aEntry.mnGUID = *lpGUID;
                 aEntry.mhMonitor = hMonitor;
                aEntry.maMonitorInfo.cbSize = sizeof(MONITORINFO);
                GetMonitorInfo( hMonitor,
                                &aEntry.maMonitorInfo );

                pMonitorList->push_back(aEntry);
            }

            return DDENUMRET_OK;
        }

        void fillMonitorList( MonitorList& rMonitorList )
        {
            // Try to fill MonitorList. If neither lib or call to
            // DirectDrawEnumerateExA does not exist, it's an old
            // DX version (< 5.0), or system does not support
            // multiple monitors.
            HINSTANCE hInstance = LoadLibrary("ddraw.dll");

            if(hInstance)
            {
                LPDIRECTDRAWENUMERATEEX lpDDEnumEx =
                    (LPDIRECTDRAWENUMERATEEX)GetProcAddress(hInstance,"DirectDrawEnumerateExA");

                if(lpDDEnumEx)
                    lpDDEnumEx( (LPDDENUMCALLBACKEXA) EnumerateExA_Callback,
                                &rMonitorList,
                                DDENUM_ATTACHEDSECONDARYDEVICES );

                FreeLibrary(hInstance);
            }
        }

        IDirectDraw2* createDirectDraw( const MonitorList& rMonitorList,
                                        MONITORINFO&       rMonitorInfo,
                                        HWND        renderWindow )
        {
             GUID* gpSelectedDriverGUID = NULL;

            // if we have multiple monitors, choose a gpSelectedDriverGUID from monitor list
            HMONITOR hMonitor = MonitorFromWindow(renderWindow,
                                                  MONITOR_DEFAULTTONEAREST);

            MonitorList::const_iterator       aCurr = rMonitorList.begin();
            const MonitorList::const_iterator aEnd = rMonitorList.end();
            while( !gpSelectedDriverGUID && aCurr != aEnd )
            {
                if(hMonitor == aCurr->mhMonitor)
                {
                    // This is the monitor we are running on
                    gpSelectedDriverGUID = const_cast<GUID*>(&aCurr->mnGUID);
                    rMonitorInfo = aCurr->maMonitorInfo;
                }

                ++aCurr;
            }

            IDirectDraw* pDirectDraw;
            if( FAILED( DirectDrawCreate( gpSelectedDriverGUID, &pDirectDraw, NULL )))
                return NULL;

            IDirectDraw2* pDirectDraw2;
            if( FAILED( pDirectDraw->QueryInterface( IID_IDirectDraw2, (LPVOID*)&pDirectDraw2 )))
                return NULL;

            // queryInterface bumped up the refcount, so release the
            // reference to the original IDirectDraw interface.
            pDirectDraw->Release();

            return pDirectDraw2;
        }

        HRESULT WINAPI EnumTextureFormatsCallback( LPDDSURFACEDESC  pSurfaceDesc,
                                                   LPVOID           pContext        )
        {
            // dirty cast of given context back to result ModeSelectContext
            DDPIXELFORMAT* pResult = (DDPIXELFORMAT*)pContext;

            if( pResult == NULL || pSurfaceDesc == NULL )
                return DDENUMRET_CANCEL;

            VERBOSE_TRACE( "EnumTextureFormatsCallback: advertised texture format has dwRGBBitCount %d, dwRBitMask %x, "
                           "dwGBitMask %x, dwBBitMask %x and dwRGBAlphaBitMask %x. The format uses %s alpha.",
                           pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount,
                           pSurfaceDesc->ddpfPixelFormat.dwRBitMask,
                           pSurfaceDesc->ddpfPixelFormat.dwGBitMask,
                           pSurfaceDesc->ddpfPixelFormat.dwBBitMask,
                           pSurfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask,
                           pSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_ALPHAPREMULT ? "premultiplied" : "non-premultiplied" );

            // Only accept RGB surfaces with alpha channel
            if( (DDPF_ALPHAPIXELS | DDPF_RGB) ==
                (pSurfaceDesc->ddpfPixelFormat.dwFlags & (DDPF_ALPHAPIXELS | DDPF_RGB)) )
            {
                // ignore formats with the DDPF_ALPHAPREMULT flag
                if(!(pSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_ALPHAPREMULT))
                {
                    // take widest alpha channel available
                    if( pSurfaceDesc->ddpfPixelFormat.dwAlphaBitDepth > pResult->dwAlphaBitDepth )
                    {
                        // take new format
                        rtl_copyMemory( pResult, &pSurfaceDesc->ddpfPixelFormat, sizeof(DDPIXELFORMAT) );
                    }
                    else if( pSurfaceDesc->ddpfPixelFormat.dwAlphaBitDepth == pResult->dwAlphaBitDepth )
                    {
                        // tie-breaking: take highest bitcount
                        if( pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount > pResult->dwRGBBitCount )
                        {
                            // take new format
                            rtl_copyMemory( pResult, &pSurfaceDesc->ddpfPixelFormat, sizeof(DDPIXELFORMAT) );
                        }
                    }
                }
            }

            return DDENUMRET_OK;
        }

        class DXRenderModule;

        //////////////////////////////////////////////////////////////////////////////////
        // DXSurface
        //////////////////////////////////////////////////////////////////////////////////

        /** ISurface implemenation.

            @attention holds the DXRenderModule via non-refcounted
            reference! This is safe with current state of affairs, since
            the canvas::PageManager holds surface and render module via
            shared_ptr (and makes sure all surfaces are deleted before its
            render module member goes out of scope).
        */
        class DXSurface : public canvas::ISurface
        {
        public:
            DXSurface( DXRenderModule&           rRenderModule,
                       const ::basegfx::B2ISize& rSize );
            ~DXSurface();

            virtual bool selectTexture();
            virtual bool isValid();
            virtual bool update( const ::basegfx::B2IPoint& rDestPos,
                                 const ::basegfx::B2IRange& rSourceRect,
                                 ::canvas::IColorBuffer&    rSource );
            virtual ::basegfx::B2IVector getSize();

        private:
            /// Guard local methods against concurrent acces to RenderModule
            class ImplRenderModuleGuard : private ::boost::noncopyable
            {
            public:
                explicit inline ImplRenderModuleGuard( DXRenderModule& rRenderModule );
                inline ~ImplRenderModuleGuard();

            private:
                DXRenderModule& mrRenderModule;
            };

            DXRenderModule&                         mrRenderModule;

            COMReference<IDirectDrawSurface> mpSurface;
            COMReference<IDirect3DTexture2>  mpTexture;

            ::basegfx::B2IVector maSize;
        };

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule
        //////////////////////////////////////////////////////////////////////////////////

        /// Default implementation of IDXRenderModule
        class DXRenderModule : public IDXRenderModule
        {
        public:
            explicit DXRenderModule( const ::Window& rWindow );

            virtual void lock() const { maMutex.acquire(); }
            virtual void unlock() const { maMutex.release(); }

            virtual COMReference<IDirectDrawSurface>
                createSystemMemorySurface( const ::basegfx::B2IVector& rSize );

            virtual bool flip( const ::basegfx::B2IRectangle& rUpdateArea,
                               const ::basegfx::B2IRectangle& rCurrWindowArea );

            virtual void resize( const ::basegfx::B2IRange& rect );
            virtual HWND getHWND() const { return mhWnd; }
            virtual void disposing();
            virtual void screenShot();
            virtual ::basegfx::B2IVector getPageSize();
            virtual ::canvas::ISurfaceSharedPtr createSurface( const ::basegfx::B2IVector& surfaceSize );
            virtual void beginPrimitive( PrimitiveType eType );
            virtual void endPrimitive();
            virtual void pushVertex( const ::canvas::Vertex& vertex );
            virtual bool isError();

            const D3DDEVICEDESC&             getDeviceDesc() const { return maDeviceDesc; }
            const DDPIXELFORMAT&             getTextureFormat() const { return maTextureFormat; }
            COMReference<IDirectDraw2>       getDirectDraw() { return mpDirectDraw; }
            COMReference< IDirect3DDevice2 > getDevice() { return mpDirect3DDevice; }

            void flushVertexCache();

            struct ModeSelectContext
            {
                DDSURFACEDESC selectedDesc;
                ::basegfx::B2ISize   requestedSize;
            };

            /** Query actual size of the device

                This is especially interesting for fullscreen devices
            */
            ::basegfx::B2ISize getFramebufferSize() const;

            /** Query the amount of memory available for new surfaces

                This might differ from getAvailableTextureMem()
                @see getAvailableTextureMem()

                @param bWithAGPMema
                When true, returned value includes non-local,
                i.e. AGP-able memory, too.

                @return the amount of free surface mem
            */
            std::size_t   getAvailableSurfaceMem( bool bWithAGPMem=true ) const;

            /** Query the amount of memory available for new textures

                This might differ from getAvailableSurfaceMem()
                @see getAvailableSurfaceMem()

                @param bWithAGPMema
                When true, returned value includes non-local,
                i.e. AGP-able memory, too.

                @return the amount of free texture mem
            */
            std::size_t     getAvailableTextureMem( bool bWithAGPMem=true ) const;

        private:
            bool queryCaps();
            bool validateCaps();
            bool setup3DDevice();
            unsigned int getDisplayFormat() const;

            void convert2Screen( ::basegfx::B2IPoint& io_rDestPos,
                                 ::basegfx::B2IRange& io_rDestArea );

            void renderInfoText( const ::rtl::OUString& rStr,
                                 const Gdiplus::PointF& rPos ) const;
            void renderFPSCounter() const;
            void renderMemAvailable() const;

            bool create( const ::Window& rWindow );
            bool validateMainSurfaces();

            /** This object represents the DirectX state machine.  In order
                to serialize access to DirectX's global state, a global
                mutex is required.
            */
            static ::osl::Mutex                     maMutex;

            HWND                                    mhWnd;
            ::boost::scoped_ptr<SystemChildWindow>  mpWindow;
            ::basegfx::B2IVector                    maSize;

            ModeSelectContext                       maSelectedFullscreenMode;
            DDPIXELFORMAT                           maTextureFormat;

            MONITORINFO                             maMonitorInfo; // monitor info for mpDirectDraw's monitor
            COMReference<IDirectDraw2>              mpDirectDraw;
            COMReference<IDirectDrawSurface>        mpPrimarySurface;
            COMReference<IDirectDrawSurface>        mpBackBufferSurface;

            COMReference< IDirect3D2 >              mpDirect3D;
            COMReference< IDirect3DDevice2 >        mpDirect3DDevice;

            mutable ::canvas::tools::ElapsedTime    maLastUpdate;   // for the frame counter

            D3DDEVICEDESC                           maDeviceDesc;

            typedef std::vector<canvas::Vertex>     vertexCache_t;
            vertexCache_t                           maVertexCache;
            std::size_t                             mnCount;

            int                                     mnBeginSceneCount;

            const bool                              mbPageFlipping;
            bool                                    mbHasNoTearingBlt;
            bool                                    mbError;
            PrimitiveType                           meType;

            ::canvas::ISurfaceSharedPtr             mpTexture;
            ::basegfx::B2IVector                    maPageSize;
        };

        ::osl::Mutex DXRenderModule::maMutex;

        //////////////////////////////////////////////////////////////////////////////////
        // DXSurface::ImplRenderModuleGuard
        //////////////////////////////////////////////////////////////////////////////////

        inline DXSurface::ImplRenderModuleGuard::ImplRenderModuleGuard(
            DXRenderModule& rRenderModule ) :
            mrRenderModule( rRenderModule )
        {
            mrRenderModule.lock();
        }

        inline DXSurface::ImplRenderModuleGuard::~ImplRenderModuleGuard()
        {
            mrRenderModule.unlock();
        }

#ifdef FAKE_MAX_NUMBER_TEXTURES
        static sal_uInt32 gNumSurfaces = 0;
#endif

        void fillRect( sal_uInt32 *pDest,
                       sal_uInt32 dwWidth,
                       sal_uInt32 dwHeight,
                       sal_uInt32 dwPitch,
                       sal_uInt32 dwColor )
        {
            for(sal_uInt32 i=0; i<dwWidth; ++i)
            {
                pDest[i]=dwColor;
                pDest[((dwHeight-1)*dwPitch)+i]=dwColor;
            }

            for(sal_uInt32 j=0; j<dwHeight; ++j)
            {
                pDest[0]=dwColor;
                pDest[dwWidth-1]=dwColor;
                pDest += dwPitch;
            }
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXSurface::DXSurface
        //////////////////////////////////////////////////////////////////////////////////

        DXSurface::DXSurface( DXRenderModule&           rRenderModule,
                              const ::basegfx::B2ISize& rSize ) :
            mrRenderModule(rRenderModule),
            mpTexture(NULL),
            mpSurface(NULL),
            maSize()
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );

#ifdef FAKE_MAX_NUMBER_TEXTURES
            ++gNumSurfaces;
            if(gNumSurfaces >= FAKE_MAX_NUMBER_TEXTURES)
                return;
#endif

#ifdef FAKE_MAX_TEXTURE_SIZE
            if(rSize.getX() > FAKE_MAX_TEXTURE_SIZE)
                return;
            if(rSize.getY() > FAKE_MAX_TEXTURE_SIZE)
                return;
#endif

            ENSURE_ARG_OR_THROW(rSize.getX() > 0 && rSize.getY() > 0,
                            "DXSurface::DXSurface(): request for zero-sized surface");

            const D3DDEVICEDESC &deviceDesc = rRenderModule.getDeviceDesc();

            DDSURFACEDESC aSurfaceDesc;
            rtl_fillMemory( &aSurfaceDesc,sizeof(DDSURFACEDESC),0 );
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            aSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
            aSurfaceDesc.dwWidth = ::std::min(deviceDesc.dwMaxTextureWidth,::canvas::tools::nextPow2(rSize.getX()));
            aSurfaceDesc.dwHeight = ::std::min(deviceDesc.dwMaxTextureHeight,::canvas::tools::nextPow2(rSize.getY()));
            aSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE |
                                          DDSCAPS_VIDEOMEMORY |
                                          DDSCAPS_LOCALVIDMEM;
            rtl_copyMemory(&aSurfaceDesc.ddpfPixelFormat,&rRenderModule.getTextureFormat(),sizeof(DDPIXELFORMAT));

            IDirectDrawSurface *pSurface;
            COMReference<IDirectDraw2> pDirectDraw(rRenderModule.getDirectDraw());
            HRESULT hr = pDirectDraw->CreateSurface(&aSurfaceDesc, &pSurface, NULL);
            if(FAILED(hr))
            {
                // if the call failed due to 'out of videomemory',
                // retry with request for AGP memory.
                if(DDERR_OUTOFVIDEOMEMORY == hr)
                {
                    aSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE |
                                                  DDSCAPS_VIDEOMEMORY |
                                                  DDSCAPS_NONLOCALVIDMEM;
                    hr = pDirectDraw->CreateSurface(&aSurfaceDesc, &pSurface, NULL);
                }
            }

            if(SUCCEEDED(hr))
            {
                IDirect3DTexture2* pTexture;
                if( FAILED(pSurface->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&pTexture)) )
                {
                    pSurface->Release();
                    return;
                }

                maSize.setX(aSurfaceDesc.dwWidth);
                maSize.setY(aSurfaceDesc.dwHeight);

                mpSurface=COMReference<IDirectDrawSurface>(pSurface);
                mpTexture=COMReference<IDirect3DTexture2>(pTexture);

                // #122683# Clear texture, to avoid ugly artifacts at the
                // border to invisible sprite areas (note that the textures
                // are usually only partly utilized).
                clearSurface( mpSurface );
            }
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXSurface::~DXSurface
        //////////////////////////////////////////////////////////////////////////////////

        DXSurface::~DXSurface()
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );

#ifdef FAKE_MAX_NUMBER_TEXTURES
            gNumSurfaces--;
#endif
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXSurface::selectTexture
        //////////////////////////////////////////////////////////////////////////////////

        bool DXSurface::selectTexture()
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );

            mrRenderModule.flushVertexCache();

            D3DTEXTUREHANDLE aTextureHandle;
            if(FAILED(mpTexture->GetHandle(
                          mrRenderModule.getDevice().get(),
                          &aTextureHandle)))
            {
                return false;
            }

            // select texture for next primitive
            if(FAILED(mrRenderModule.getDevice()->SetRenderState(
                          D3DRENDERSTATE_TEXTUREHANDLE,aTextureHandle)))
            {
                return false;
            }

#if defined(DX_DEBUG_IMAGES)
# if OSL_DEBUG_LEVEL > 0
            if( mpSurface.is() )
            {
                DDSURFACEDESC aSurfaceDesc;
                rtl_fillMemory( &aSurfaceDesc,sizeof(DDSURFACEDESC),0 );
                aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

                if( SUCCEEDED(mpSurface->Lock( NULL,
                                               &aSurfaceDesc,
                                               DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_READONLY,
                                               NULL)) )
                {
                    imdebug( "rgba w=%d h=%d %p",
                             aSurfaceDesc.dwWidth,
                             aSurfaceDesc.dwHeight,
                             aSurfaceDesc.lpSurface );

                    mpSurface->Unlock(NULL);
                }
            }
# endif
#endif

            return true;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXSurface::isValid
        //////////////////////////////////////////////////////////////////////////////////

        bool DXSurface::isValid()
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );

            if(!(mpSurface.is()))
                return false;

            if(mpSurface->IsLost() == DDERR_SURFACELOST)
            {
                mpSurface->Restore();
                return false;
            }

            return true;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXSurface::update
        //////////////////////////////////////////////////////////////////////////////////

        bool DXSurface::update( const ::basegfx::B2IPoint& rDestPos,
                                const ::basegfx::B2IRange& rSourceRect,
                                ::canvas::IColorBuffer&    rSource )
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );

            // can't update if surface is not valid, that means
            // either not existent nor restored...
            if(!(isValid()))
                return false;

            DDSURFACEDESC aSurfaceDesc;
            rtl_fillMemory( &aSurfaceDesc,sizeof(DDSURFACEDESC),0 );
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

            // TODO(P2): only lock the region we want to update
            if( FAILED(mpSurface->Lock( NULL,
                                        &aSurfaceDesc,
                                        DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY,
                                        NULL)) )
                return false;

            if(sal_uInt8* pImage = rSource.lock())
            {
                switch( rSource.getFormat() )
                {
                    case ::canvas::IColorBuffer::FMT_A8R8G8B8:
                    {
                        const std::size_t nSourceBytesPerPixel(4);
                        const std::size_t nSourcePitchInBytes(rSource.getStride());
                        pImage += rSourceRect.getMinY()*nSourcePitchInBytes;
                        pImage += rSourceRect.getMinX()*nSourceBytesPerPixel;

                        // calculate the destination memory address
                        sal_uInt8 *pDst = ((sal_uInt8*)aSurfaceDesc.lpSurface+
                                           (rDestPos.getY()*aSurfaceDesc.lPitch) +
                                           (4*rDestPos.getX()));

                        const sal_uInt32 nNumBytesToCopy(
                            static_cast<sal_uInt32>(
                                rSourceRect.getWidth())*
                            nSourceBytesPerPixel);
                        const sal_uInt64 nNumLines(rSourceRect.getHeight());

                        for(sal_uInt32 i=0; i<nNumLines; ++i)
                        {
                            rtl_copyMemory(pDst,pImage,nNumBytesToCopy);

                            pDst += aSurfaceDesc.lPitch;
                            pImage += nSourcePitchInBytes;
                        }
                    }
                    break;

                    case ::canvas::IColorBuffer::FMT_R8G8B8:
                    {
                        const std::size_t nSourceBytesPerPixel(3);
                        const std::size_t nSourcePitchInBytes(rSource.getStride());
                        pImage += rSourceRect.getMinY()*nSourcePitchInBytes;
                        pImage += rSourceRect.getMinX()*nSourceBytesPerPixel;

                        // calculate the destination memory address
                        sal_uInt8 *pDst = ((sal_uInt8*)aSurfaceDesc.lpSurface+
                                           (rDestPos.getY()*aSurfaceDesc.lPitch) +
                                           (4*rDestPos.getX()));

                        const sal_uInt64 nNumColumns(rSourceRect.getWidth());
                        const sal_uInt64 nNumLines(rSourceRect.getHeight());
                        for(sal_uInt32 i=0; i<nNumLines; ++i)
                        {
                            sal_uInt32 *pDstScanline = reinterpret_cast<sal_uInt32 *>(pDst);
                            sal_uInt8 *pSrcScanline = reinterpret_cast<sal_uInt8 *>(pImage);
                            for(sal_uInt32 x=0; x<nNumColumns; ++x)
                            {
                                sal_uInt32 color(0xFF000000);
                                color |= pSrcScanline[2]<<16;
                                color |= pSrcScanline[1]<<8;
                                color |= pSrcScanline[0];
                                pSrcScanline += 3;
                                *pDstScanline++ = color;
                            }

                            pDst += aSurfaceDesc.lPitch;
                            pImage += nSourcePitchInBytes;
                        }
                    }
                    break;

                    case ::canvas::IColorBuffer::FMT_X8R8G8B8:
                    {
                        const std::size_t nSourceBytesPerPixel(4);
                        const std::size_t nSourcePitchInBytes(rSource.getStride());
                        pImage += rSourceRect.getMinY()*nSourcePitchInBytes;
                        pImage += rSourceRect.getMinX()*nSourceBytesPerPixel;

                        // calculate the destination memory address
                        sal_uInt8 *pDst = ((sal_uInt8*)aSurfaceDesc.lpSurface+
                                           (rDestPos.getY()*aSurfaceDesc.lPitch) +
                                           (4*rDestPos.getX()));

                        const sal_uInt64 nNumLines(rSourceRect.getHeight());

                        for(sal_uInt32 i=0; i<nNumLines; ++i)
                        {
                            sal_uInt32 *pSrc32 = reinterpret_cast<sal_uInt32 *>(pImage);
                            sal_uInt32 *pDst32 = reinterpret_cast<sal_uInt32 *>(pDst);
                            for(sal_uInt32 j=0; j<rSourceRect.getWidth(); ++j)
                                pDst32[j] = 0xFF000000 | pSrc32[j];

                            pDst += aSurfaceDesc.lPitch;
                            pImage += nSourcePitchInBytes;
                        }
                    }
                    break;

                    default:
                        ENSURE_OR_RETURN_FALSE(false,
                                          "DXSurface::update(): Unknown/unimplemented buffer format" );
                        break;
                }

                rSource.unlock();
            }

            return SUCCEEDED(mpSurface->Unlock(NULL));
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXSurface::getSize
        //////////////////////////////////////////////////////////////////////////////////

        ::basegfx::B2IVector DXSurface::getSize()
        {
            return maSize;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::DXRenderModule
        //////////////////////////////////////////////////////////////////////////////////

        DXRenderModule::DXRenderModule( const ::Window& rWindow ) :
            mhWnd(0),
            mpWindow(),
            maSize(),
            maSelectedFullscreenMode(),
            maTextureFormat(),
            maMonitorInfo(),
            mpDirectDraw(),
            mpPrimarySurface(),
            mpBackBufferSurface(),
            mpDirect3D(),
            mpDirect3DDevice(),
            maLastUpdate(),
            maDeviceDesc(),
            maVertexCache(),
            mnCount(0),
            mnBeginSceneCount(0),
            mbPageFlipping( false ),
            mbHasNoTearingBlt( false ),
            mbError( false ),
            meType( PRIMITIVE_TYPE_UNKNOWN ),
            mpTexture(),
            maPageSize()
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if(!(create(rWindow)))
            {
                throw lang::NoSupportException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                         "Could not create DirectX device!") ),NULL);
            }

            // allocate a single texture surface which can be used later.
            // we also use this to calibrate the page size.
            ::basegfx::B2IVector aPageSize(
                ::std::min(
                    static_cast<sal_uInt32>(maDeviceDesc.dwMaxTextureWidth),
                    static_cast<sal_uInt32>(MAX_TEXTURE_SIZE)),
                ::std::min(
                    static_cast<sal_uInt32>(maDeviceDesc.dwMaxTextureHeight),
                    static_cast<sal_uInt32>(MAX_TEXTURE_SIZE)));
            while(true)
            {
                mpTexture = ::canvas::ISurfaceSharedPtr(
                    new DXSurface(*this,aPageSize));
                if(mpTexture->isValid())
                    break;

                aPageSize.setX(aPageSize.getX()>>1);
                aPageSize.setY(aPageSize.getY()>>1);
                if((aPageSize.getX() < MIN_TEXTURE_SIZE) ||
                   (aPageSize.getY() < MIN_TEXTURE_SIZE))
                {
                    throw lang::NoSupportException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            "Could not create DirectX device!") ),NULL);
                }
            }
            maPageSize=aPageSize;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::create
        //////////////////////////////////////////////////////////////////////////////////

        bool DXRenderModule::create( const ::Window& rWindow )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            maVertexCache.reserve(1024);

            mpWindow.reset(
                new SystemChildWindow(
                const_cast<Window *>(&rWindow), 0) );

            // system child window must not receive mouse events
            mpWindow->SetMouseTransparent( TRUE );

            // parent should receive paint messages as well
            // [PARENTCLIPMODE_NOCLIP], the argument is here
            // passed as plain numeric value since the stupid
            // define utilizes a USHORT cast.
            mpWindow->SetParentClipMode(0x0002);

            // the system child window must not clear its background
            mpWindow->EnableEraseBackground( FALSE );

            mpWindow->SetControlForeground();
            mpWindow->SetControlBackground();
            mpWindow->EnablePaint(FALSE);

            const SystemEnvData *pData = mpWindow->GetSystemData();
            const HWND hwnd(reinterpret_cast<HWND>(pData->hWnd));
            mhWnd = const_cast<HWND>(hwnd);

            ENSURE_OR_THROW( IsWindow( reinterpret_cast<HWND>(mhWnd) ),
                              "DXRenderModuleDXRenderModuleWin32() No valid HWND given." );

            // retrieve position and size of the parent window
            const ::Size &rSizePixel(rWindow.GetSizePixel());

            // remember the size of the parent window, since we
            // need to use this for our child window.
            maSize.setX(static_cast<sal_Int32>(rSizePixel.Width()));
            maSize.setY(static_cast<sal_Int32>(rSizePixel.Height()));

            // let the child window cover the same size as the parent window.
            mpWindow->SetPosSizePixel(0,0,maSize.getX(),maSize.getY());

            MonitorList aMonitorList;
            fillMonitorList( aMonitorList );

             mpDirectDraw = COMReference<IDirectDraw2>(
                createDirectDraw(aMonitorList, maMonitorInfo, mhWnd));

            if(!mpDirectDraw.is())
                return false;

            if( !queryCaps() )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): GetCaps failed" );
                mpDirectDraw.reset();
                return false;
            }

            if( !validateCaps() )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): Insufficient DirectX capabilities, failed" );
                mpDirectDraw.reset();
                return false;
            }

            if( FAILED( mpDirectDraw->SetCooperativeLevel( mhWnd,
                                                           DDSCL_NORMAL|DDSCL_MULTITHREADED|DDSCL_FPUPRESERVE ) ) )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): SetCooperativeLevel failed" );
                mpDirectDraw.reset();
                return false;
            }

            // setup query struct
            rtl_fillMemory( &maSelectedFullscreenMode.selectedDesc,
                            sizeof(DDSURFACEDESC), 0 );
            maSelectedFullscreenMode.selectedDesc.dwSize = sizeof(DDSURFACEDESC);

            // read current display mode, e.g. for screen dimension
            if( FAILED( mpDirectDraw->GetDisplayMode( &maSelectedFullscreenMode.selectedDesc )) )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): GetDisplayMode failed" );
                mpDirectDraw.reset();
                return false;
            }

            // check for supported primary surface formats...
            unsigned int nDisplayFormat = getDisplayFormat() & 0x00000FFF;
            if(nDisplayFormat != 0x888 && nDisplayFormat != 0x565)
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): Unsupported DisplayFormat" );
                mpDirectDraw.reset();
                return false;
            }

            // create primary surface reference
            DDSURFACEDESC       aSurfaceDesc;
            IDirectDrawSurface* pPrimarySurface;

            rtl_fillMemory( &aSurfaceDesc,
                            sizeof(DDSURFACEDESC), 0 );
            aSurfaceDesc.dwSize = sizeof(aSurfaceDesc);
            aSurfaceDesc.dwFlags = DDSD_CAPS;
            aSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

            if( FAILED(mpDirectDraw->CreateSurface(&aSurfaceDesc, &pPrimarySurface, NULL)) )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): CreateSurface failed" );
                mpDirectDraw.reset();
                return false;
            }

            mpPrimarySurface = COMReference< IDirectDrawSurface >(pPrimarySurface);

            // create a Clipper and associate it with the primary surface
            // and the render window
            LPDIRECTDRAWCLIPPER pClipper;
            if( FAILED(mpDirectDraw->CreateClipper( 0, &pClipper, NULL )) )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): CreateClipper failed" );
                mpPrimarySurface.reset();
                mpDirectDraw.reset();
                return false;
            }
            if( FAILED(pClipper->SetHWnd(0, mhWnd)) )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): Clipper->SetHWnd failed" );
                pClipper->Release();
                mpPrimarySurface.reset();
                mpDirectDraw.reset();
                return false;
            }
            if( FAILED(mpPrimarySurface->SetClipper( pClipper )) )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): SetClipper failed" );
                pClipper->Release();
                mpPrimarySurface.reset();
                mpDirectDraw.reset();
                return false;
            }

            // clipper is now owned by mpPrimarySurface, release our reference
            pClipper->Release();

            // TODO(F3): Check whether palette needs any setup here

            // get us a backbuffer for simulated flipping
            IDirectDrawSurface* pSurface;

            // Strictly speaking, we don't need a full screen worth of
            // backbuffer here. We could also scale dynamically with
            // the current window size, but this will make it
            // necessary to temporarily have two buffers while copying
            // from the old to the new one. What's more, at the time
            // we need a larger buffer, DX might not have sufficient
            // resources available, and we're then left with too small
            // a back buffer, and no way of falling back to a
            // different canvas implementation.
            const ::basegfx::B2ISize aSize( getFramebufferSize() );

            rtl_fillMemory( &aSurfaceDesc,
                            sizeof(DDSURFACEDESC), 0 );
            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            aSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
            aSurfaceDesc.dwHeight= aSize.getY();
            aSurfaceDesc.dwWidth = aSize.getX();

            aSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

            HRESULT nRes = mpDirectDraw->CreateSurface(&aSurfaceDesc, &pSurface, NULL);

            if( FAILED( nRes ) )
            {
                if( nRes == DDERR_OUTOFVIDEOMEMORY )
                {
                    // local vid mem failed. Maybe AGP mem works?
                    aSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM;
                    if( FAILED(mpDirectDraw->CreateSurface(&aSurfaceDesc, &pSurface, NULL)) )
                    {
                        // no chance, go defunct, and exit
                        VERBOSE_TRACE( "Device::Device(): CreateSurface for backbuffer failed" );
                        mpPrimarySurface.reset();
                        mpDirectDraw.reset();
                        return false;
                    }

                    VERBOSE_TRACE( "Device::Device(): CreateSurface for backbuffer reverted to non-local video mem" );
                }
                else
                {
                    // no chance, go defunct, and exit
                    VERBOSE_TRACE( "Device::Device(): CreateSurface for backbuffer failed" );
                    mpPrimarySurface.reset();
                    mpDirectDraw.reset();
                    return false;
                }
            }

            VERBOSE_TRACE( "Device::Device(): created backbuffer of size %d times %d pixel",
                           aSurfaceDesc.dwWidth,
                           aSurfaceDesc.dwHeight );

            mpBackBufferSurface = COMReference< IDirectDrawSurface >(pSurface);
            clearSurface(mpBackBufferSurface);

            if( !setup3DDevice() )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::Device(): setup3DDevice failed" );
                mpBackBufferSurface.reset();
                mpPrimarySurface.reset();
                mpDirectDraw.reset();
                return false;
            }

            mpWindow->Show();

            return true;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::getSize
        //////////////////////////////////////////////////////////////////////////////////

        ::basegfx::B2ISize DXRenderModule::getFramebufferSize() const
        {
            return mpDirectDraw.is() ?
                ::basegfx::B2ISize( maSelectedFullscreenMode.selectedDesc.dwWidth,
                                    maSelectedFullscreenMode.selectedDesc.dwHeight ) :
                ::basegfx::B2ISize();
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::setup3DDevice
        //////////////////////////////////////////////////////////////////////////////////

        bool DXRenderModule::setup3DDevice()
        {
            // create and setup 3D device
            // ==========================
            LPDIRECT3D2 pDirect3D;
            if( FAILED( mpDirectDraw->QueryInterface( IID_IDirect3D2, (LPVOID*)&pDirect3D ) ) )
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::setup3DDevice(): QueryInterface() for Direct3D failed" );
                return false;
            }

            mpDirect3D = COMReference< IDirect3D2 >(pDirect3D);

            LPDIRECT3DDEVICE2 pDirect3DDevice;
            // try HW-accelerated device first
            if( FAILED(mpDirect3D->CreateDevice( IID_IDirect3DHALDevice,
                                                 mpBackBufferSurface.get(),
                                                 &pDirect3DDevice )) )
            {
                // no HW 3D support - go defunct, and exit
                VERBOSE_TRACE( "Device::setup3DDevice(): CreateDevice() for HW Direct3D rendering failed" );
                mpDirect3D.reset();
                return false;
            }

            D3DDEVICEDESC aHELDeviceDesc;
            rtl_fillMemory(&maDeviceDesc,sizeof(maDeviceDesc),0);
            rtl_fillMemory(&aHELDeviceDesc,sizeof(aHELDeviceDesc),0);
            maDeviceDesc.dwSize = sizeof(maDeviceDesc);
            aHELDeviceDesc.dwSize = sizeof(aHELDeviceDesc);
            if(FAILED(pDirect3DDevice->GetCaps(&maDeviceDesc,&aHELDeviceDesc)))
            {
                // go defunct, and exit
                VERBOSE_TRACE( "Device::setup3DDevice(): GetCaps() for Direct3DDevice failed" );
                mpDirect3D.reset();
                return false;
            }

            mpDirect3DDevice = COMReference< IDirect3DDevice2 >(pDirect3DDevice);

            // select appropriate texture format (_need_ alpha channel here)
            rtl_fillMemory( &maTextureFormat,
                            sizeof(DDPIXELFORMAT), 0 );
            maTextureFormat.dwSize = sizeof(DDPIXELFORMAT);
            if( SUCCEEDED(mpDirect3DDevice->EnumTextureFormats( EnumTextureFormatsCallback, &maTextureFormat )) )
            {
                bool bSupportedFormat = true;
                if((maTextureFormat.dwFlags & (DDPF_ALPHAPIXELS | DDPF_RGB)) != (DDPF_ALPHAPIXELS | DDPF_RGB))
                    bSupportedFormat = false;
                else if(maTextureFormat.dwRGBAlphaBitMask != 0xFF000000)
                    bSupportedFormat = false;
                else if(maTextureFormat.dwRBitMask != 0x00FF0000)
                    bSupportedFormat = false;
                else if(maTextureFormat.dwGBitMask != 0x0000FF00)
                    bSupportedFormat = false;
                else if(maTextureFormat.dwBBitMask != 0x000000FF)
                    bSupportedFormat = false;

                if(bSupportedFormat)
                {
                    VERBOSE_TRACE( "Device::setup3DDevice(): chose texture format dwRGBBitCount %d, dwRBitMask %x, "
                                   "dwGBitMask %x, dwBBitMask %x and dwRGBAlphaBitMask %x. The texture uses %s alpha.",
                                   maTextureFormat.dwRGBBitCount,
                                   maTextureFormat.dwRBitMask,
                                   maTextureFormat.dwGBitMask,
                                   maTextureFormat.dwBBitMask,
                                   maTextureFormat.dwRGBAlphaBitMask,
                                   maTextureFormat.dwFlags & DDPF_ALPHAPREMULT ? "premultiplied" : "non-premultiplied" );

                    // setup the device (with as much as we can possibly do here)
                    // ==========================================================

                    LPDIRECT3DVIEWPORT2 pViewport;

                    if( SUCCEEDED(mpDirect3D->CreateViewport( &pViewport, NULL )) )
                    {
                        if( SUCCEEDED(mpDirect3DDevice->AddViewport( pViewport )) )
                        {
                            // setup viewport (to whole backbuffer)
                            D3DVIEWPORT2 aViewport;

                            aViewport.dwSize = sizeof(D3DVIEWPORT2);
                            aViewport.dwX = 0;
                            aViewport.dwY = 0;
                            aViewport.dwWidth = maSelectedFullscreenMode.selectedDesc.dwWidth;
                            aViewport.dwHeight = maSelectedFullscreenMode.selectedDesc.dwHeight;
                            aViewport.dvClipX = -1.0;
                            aViewport.dvClipY =  -1.0;
                            aViewport.dvClipWidth  = 2.0;
                            aViewport.dvClipHeight = 2.0;
                            aViewport.dvMinZ = 0.0;
                            aViewport.dvMaxZ = 1.0;

                            if( SUCCEEDED(pViewport->SetViewport2( &aViewport )) )
                            {
                                if( SUCCEEDED(mpDirect3DDevice->SetCurrentViewport( pViewport )) )
                                {
                                    // Viewport was handed over to 3DDevice, thus we can release now
                                    pViewport->Release();

                                    // currently, no need for any
                                    // matrix or light source
                                    // setup, since we only render
                                    // transformed&lighted
                                    // vertices

                                    // done; successfully
                                    return true;
                                }
                                else
                                {
                                    VERBOSE_TRACE( "Device::setup3DDevice(): SetCurrentViewport failed" );
                                }
                            }
                            else
                            {
                                VERBOSE_TRACE( "Device::setup3DDevice(): SetViewport2 failed" );
                            }
                        }
                        else
                        {
                            VERBOSE_TRACE( "Device::setup3DDevice(): AddViewport failed" );
                        }

                        pViewport->Release();
                    }
                    else
                    {
                        VERBOSE_TRACE( "Device::setup3DDevice(): CreateViewport failed" );
                    }
                }
                else
                {
                    VERBOSE_TRACE( "Device::setup3DDevice(): No supported pixelformat" );
                }
            }
            else
            {
                VERBOSE_TRACE( "Device::setup3DDevice(): EnumTextureFormats failed" );
            }

            // go defunct, and exit
            mpDirect3DDevice.reset();
            mpDirect3D.reset();

            return false;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::queryCaps
        //////////////////////////////////////////////////////////////////////////////////

        bool DXRenderModule::queryCaps()
        {
            DDCAPS  aHWCaps;
            DDCAPS  aHELCaps;

            rtl_fillMemory( &aHWCaps,
                            sizeof(aHWCaps), 0 );
            rtl_fillMemory( &aHELCaps,
                            sizeof(aHELCaps), 0 );
            aHWCaps.dwSize = sizeof( aHWCaps );
            aHELCaps.dwSize = sizeof( aHELCaps );

            if( FAILED( mpDirectDraw->GetCaps( &aHWCaps,
                                            &aHELCaps ) ) )
            {
                return false;
            }

            mbHasNoTearingBlt = aHWCaps.dwFXCaps & DDBLTFX_NOTEARING;

            VERBOSE_TRACE( "dxcanvas initialization: %d bytes VRAM free for surfaces (%d with AGP mem), "
                           "%d bytes VRAM free for textures (%d with AGP mem)",
                           getAvailableSurfaceMem( false ),
                           getAvailableSurfaceMem( true ),
                           getAvailableTextureMem( false ),
                           getAvailableTextureMem( true ) );

            return true;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::validateCaps
        //////////////////////////////////////////////////////////////////////////////////

        bool DXRenderModule::validateCaps()
        {
            // TODO(E3): Validate HW capabilities. Depending on primary
            // surface size, reject HW e.g. on the grounds of insufficient
            // VRAM.

            // setup query struct
            DDSURFACEDESC desc;
            rtl_fillMemory(&desc,sizeof(DDSURFACEDESC),0);
            desc.dwSize = sizeof(DDSURFACEDESC);

            // read current display mode, e.g. for screen dimension
            if(FAILED( mpDirectDraw->GetDisplayMode(&desc)))
                return false;

            // simple heuristic: we need at least 3 times the desktop
            // resolution based on ARGB color values...
            std::size_t nMinimumVRAMSize = ((desc.dwWidth*desc.dwHeight)<<2)*3;
            if(getAvailableSurfaceMem() < nMinimumVRAMSize)
                return false;

            return true;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::getDisplayFormat
        //////////////////////////////////////////////////////////////////////////////////

        unsigned int DXRenderModule::getDisplayFormat() const
        {
            unsigned int nFormat;
            nFormat  = ::canvas::tools::bitcount32(maSelectedFullscreenMode.selectedDesc.ddpfPixelFormat.dwRGBAlphaBitMask)<<12;
            nFormat |= ::canvas::tools::bitcount32(maSelectedFullscreenMode.selectedDesc.ddpfPixelFormat.dwRBitMask)<<8;
            nFormat |= ::canvas::tools::bitcount32(maSelectedFullscreenMode.selectedDesc.ddpfPixelFormat.dwGBitMask)<<4;
            nFormat |= ::canvas::tools::bitcount32(maSelectedFullscreenMode.selectedDesc.ddpfPixelFormat.dwBBitMask);
            return nFormat;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::getAvailableSurfaceMem
        //////////////////////////////////////////////////////////////////////////////////

        std::size_t DXRenderModule::getAvailableSurfaceMem( bool bWithAGPMem ) const
        {
            if( !mpDirectDraw.is() )
                return 0;

            std::size_t nRes( 0 );

            DDSCAPS aSurfaceCaps;
            DWORD   nTotal, nFree;

            // real VRAM (const_cast, since GetAvailableVidMem is non-const)
            aSurfaceCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
            if( FAILED(const_cast<IDirectDraw2&>(*mpDirectDraw).GetAvailableVidMem( &aSurfaceCaps, &nTotal, &nFree )) )
                return 0;

            nRes += nFree;

            if( bWithAGPMem )
            {
                // AGP RAM (const_cast, since GetAvailableVidMem is non-const)
                aSurfaceCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM;
                if( FAILED(const_cast<IDirectDraw2&>(*mpDirectDraw).GetAvailableVidMem( &aSurfaceCaps, &nTotal, &nFree )) )
                    return 0;

                nRes += nFree;
            }

            return nRes;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::getAvailableTextureMem
        //////////////////////////////////////////////////////////////////////////////////

        std::size_t DXRenderModule::getAvailableTextureMem( bool bWithAGPMem ) const
        {
            if( !mpDirectDraw.is() )
                return 0;

            std::size_t nRes( 0 );

            DDSCAPS aSurfaceCaps;
            DWORD   nTotal, nFree;

            // TODO(F1): Check if flags are applicable

            // real VRAM (const_cast, since GetAvailableVidMem is non-const)
            aSurfaceCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
            if( FAILED(const_cast<IDirectDraw2&>(*mpDirectDraw).GetAvailableVidMem( &aSurfaceCaps, &nTotal, &nFree )) )
                return 0;

            nRes += nFree;

            if( bWithAGPMem )
            {
                // AGP RAM (const_cast, since GetAvailableVidMem is non-const)
                aSurfaceCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM;
                if( FAILED(const_cast<IDirectDraw2&>(*mpDirectDraw).GetAvailableVidMem( &aSurfaceCaps, &nTotal, &nFree )) )
                    return 0;

                nRes += nFree;
            }

            // TODO(F1): Add pool mem

            return nRes;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::convert2Screen
        //////////////////////////////////////////////////////////////////////////////////

        void DXRenderModule::convert2Screen( ::basegfx::B2IPoint& io_rDestPos,
                                             ::basegfx::B2IRange& io_rDestArea )
        {
            POINT aPoint = { 0, 0 };
            ClientToScreen( mhWnd, &aPoint );

            // i52230 make sure given screen coordinate is relative to
            // this monitor's area (the device rendering is always
            // contained to a single monitor)
            aPoint.x -= maMonitorInfo.rcMonitor.left;
            aPoint.y -= maMonitorInfo.rcMonitor.top;

            io_rDestPos.setX( io_rDestPos.getX() + aPoint.x );
            io_rDestPos.setY( io_rDestPos.getY() + aPoint.y );

            const ::basegfx::B2ISize& rSize( getFramebufferSize() );

            // calc output bounds (clip against framebuffer bounds)
            io_rDestArea = ::basegfx::B2IRange(
                ::std::max( sal_Int32(0),
                            ::std::min( sal_Int32(rSize.getX()),
                                        sal_Int32(io_rDestArea.getMinX() + aPoint.x) ) ),
                ::std::max( sal_Int32(0),
                            ::std::min( sal_Int32(rSize.getY()),
                                        sal_Int32(io_rDestArea.getMinY() + aPoint.y) ) ),
                ::std::max( sal_Int32(0),
                            ::std::min( sal_Int32(rSize.getX()),
                                        sal_Int32(io_rDestArea.getMaxX() + aPoint.x) ) ),
                ::std::max( sal_Int32(0),
                            ::std::min( sal_Int32(rSize.getY()),
                                        sal_Int32(io_rDestArea.getMaxY() + aPoint.y) ) ) );
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::createSystemMemorySurface
        //////////////////////////////////////////////////////////////////////////////////

        COMReference<IDirectDrawSurface> DXRenderModule::createSystemMemorySurface( const ::basegfx::B2IVector& rSize )
        {
            DDSURFACEDESC       aSurfaceDesc;
            IDirectDrawSurface* pSurface;

            aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
            aSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;;
            aSurfaceDesc.dwWidth = rSize.getX();
            aSurfaceDesc.dwHeight= rSize.getY();

            rtl_copyMemory( &aSurfaceDesc.ddpfPixelFormat, &maTextureFormat, sizeof(DDPIXELFORMAT) );

            aSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

            HRESULT nRes = mpDirectDraw->CreateSurface(&aSurfaceDesc, &pSurface, NULL);
            if(FAILED(nRes))
                return COMReference<IDirectDrawSurface>(NULL);

            return COMReference<IDirectDrawSurface>(pSurface);
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::flip
        //////////////////////////////////////////////////////////////////////////////////

        bool DXRenderModule::flip( const ::basegfx::B2IRectangle& rUpdateArea,
                                   const ::basegfx::B2IRectangle& rCurrWindowArea )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            // see if the main surfaces got lost. if so, try to
            // restore them. bail out if this operation fails.
            if(!(validateMainSurfaces()))
                return false;

            flushVertexCache();

            ENSURE_OR_THROW( !mnBeginSceneCount,
                              "Device::flip(): within 3D scene" );

            // TODO(E3): handle DX errors more thoroughly. For fullscreen
            // exclusive mode, actually even our primary surface can get
            // lost and needs restore!

            if( mpDirectDraw.is() &&
                mpPrimarySurface.is() &&
                mpBackBufferSurface.is() )
            {
                // ignore area and offset for page flipping device
                if( mbPageFlipping )
                {
#if defined(VERBOSE) && defined(DBG_UTIL)
                    renderFPSCounter();
                    renderMemAvailable();
#endif
                    VERBOSE_TRACE( "Device::flip(): Using true page flipping" );

                    // use true page flipping. Hopefully, the 3D hardware
                    // is flushed on this flip call (rumours have it that
                    // way), otherwise, perform the Lock hack as for the
                    // Blt below.
                    if( SUCCEEDED(mpPrimarySurface->Flip( NULL, DDFLIP_WAIT )) )
                        return true;
                }
                else
                {
                    VERBOSE_TRACE( "Device::flip(): Using blt for page flipping" );

                    // determine actual window position
                    ::basegfx::B2IPoint aDestPoint( rUpdateArea.getMinimum() );
                    ::basegfx::B2IRange aSourceArea( rUpdateArea );
                    ::basegfx::B2IRange aDestArea( 0,0,
                                                   static_cast<sal_Int32>(rCurrWindowArea.getWidth()),
                                                   static_cast<sal_Int32>(rCurrWindowArea.getHeight()) );
                    convert2Screen( aDestPoint, aDestArea );

                    // perform clipping
                    if( !::canvas::tools::clipBlit( aSourceArea,
                                                    aDestPoint,
                                                    rUpdateArea,
                                                    aDestArea ) )
                        return true; // fully clipped, but still, in a way,
                                     // successful.

                    // TODO(P1): Rumours have it that the 3D hardware
                    // _might_ still be rendering with flaky drivers,
                    // which don't flush properly on Blt(). It was said,
                    // that 'usually', it works to lock the 3D render
                    // target (the backbuffer in this case). OTOH, I've
                    // found that this tends to degrade performance
                    // significantly on complying cards...

                    // TODO(P1): Up until rev. 1.3, this method contained
                    // code to make sure the blit will start _immediately_
                    // after the Blt call. If this is not warranted, wait
                    // for the next vsync. As this case was found to be
                    // extremely seldom, kicked out (what's more, there's
                    // simply no guarantee that the blitter will be
                    // available at any point in the code - Windows still
                    // is a preemptive multi-processing environment. And
                    // _if_ we're competing with someone over the blitter,
                    // we will do so the next VBLANK interval, and the
                    // following...)

                    // screen update seems to be smoother when waiting
                    // for vblank in every case - even when blitter
                    // supports the DDBLTFX_NOTEARING flag.
                    if( FAILED(mpDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,
                                                                  NULL)) )
                        return false;

                    DDBLTFX  aBltFx;
                    DDBLTFX* pBltFX = NULL;
                    if( mbHasNoTearingBlt )
                    {
                        // Blt can internally schedule for no-tearing
                        // ===========================================

                        rtl_fillMemory( &aBltFx,
                                        sizeof(aBltFx), 0 );
                        aBltFx.dwSize = sizeof(aBltFx);
                        aBltFx.dwDDFX = DDBLTFX_NOTEARING;

                        pBltFX = &aBltFx;
                    }

                    if( doBlit( aDestPoint,
                                *mpPrimarySurface,
                                aSourceArea,
                                *mpBackBufferSurface,
                                pBltFX,false ) )
                    {
#if defined(VERBOSE) && defined(DBG_UTIL)
                        renderFPSCounter();
                        renderMemAvailable();
#endif
                        return true;
                    }
                }
            }
            return false;
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::disposing
        //////////////////////////////////////////////////////////////////////////////////

        void DXRenderModule::disposing()
        {
            if(!(mhWnd))
                return;

            mpTexture.reset();
            mpWindow.reset();
            mhWnd=NULL;

            // refrain from releasing the DX5 objects - deleting the
            // DX5 device seems to kill the whole engine, including
            // all objects we might still hold references to
            // (surfaces, e.g.)
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::screenshot
        //////////////////////////////////////////////////////////////////////////////////

        void DXRenderModule::screenShot()
        {
            if(!(mpBackBufferSurface.get()))
                return;
            char filename[256];
            static sal_uInt32 counter = 0;
            sprintf(filename,"c:\\shot%d.bmp",counter++);
            dumpSurface(mpBackBufferSurface,filename);
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::validateMainSurfaces
        //////////////////////////////////////////////////////////////////////////////////

        bool DXRenderModule::validateMainSurfaces()
        {
            if(mpPrimarySurface.get()) {
                if(mpPrimarySurface->IsLost() == DDERR_SURFACELOST) {
                    if(FAILED(mpPrimarySurface->Restore()))
                        return false;
                }
            }

            if(mpBackBufferSurface.get()) {
                if(mpBackBufferSurface->IsLost() == DDERR_SURFACELOST)
                {
                    // TODO(F1): simply restoring the backbuffer does not
                    // work as expected, we need to re-create everything
                    // from scratch. find out why...
                    //if(SUCCEEDED(mpBackBufferSurface->Restore()))
                    //  return setup3DDevice();

                    mpBackBufferSurface.reset();

                    // get us a backbuffer for simulated flipping
                    IDirectDrawSurface* pSurface;

                    // TODO(P2): Strictly speaking, we don't need a full screen worth of
                    // backbuffer here. We could also scale dynamically with the current
                    // window size, but this will make it necessary to temporarily have two
                    // buffers while copying from the old to the new one. YMMV.
                    const ::basegfx::B2ISize aSize( getFramebufferSize() );

                    DDSURFACEDESC aSurfaceDesc;
                    rtl_fillMemory( &aSurfaceDesc, sizeof(DDSURFACEDESC), 0 );
                    aSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
                    aSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
                    aSurfaceDesc.dwHeight= aSize.getY();
                    aSurfaceDesc.dwWidth = aSize.getX();

                    aSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

                    HRESULT nRes = mpDirectDraw->CreateSurface(&aSurfaceDesc, &pSurface, NULL);

                    if( FAILED( nRes ) )
                    {
                        if( nRes == DDERR_OUTOFVIDEOMEMORY )
                        {
                            // local vid mem failed. Maybe AGP mem works?
                            aSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM;
                            if( FAILED(mpDirectDraw->CreateSurface(&aSurfaceDesc, &pSurface, NULL)) )
                            {
                                // no chance
                                return false;
                            }

                            VERBOSE_TRACE( "Device::Device(): CreateSurface for backbuffer reverted to non-local video mem" );
                        }
                        else
                        {
                            // no chance
                            VERBOSE_TRACE( "Device::Device(): CreateSurface for backbuffer failed" );
                            return false;
                        }
                    }

                    VERBOSE_TRACE( "Device::Device(): created backbuffer of size %d times %d pixel",
                                   aSurfaceDesc.dwWidth,
                                   aSurfaceDesc.dwHeight );

                    mpBackBufferSurface = COMReference< IDirectDrawSurface >(pSurface);

                    return setup3DDevice();
                }
            }

            return true;
        }

        void DXRenderModule::renderInfoText( const ::rtl::OUString& rStr,
                                             const Gdiplus::PointF& rPos ) const
        {
            ENSURE_OR_THROW( !mnBeginSceneCount,
                              "Device::renderInfoText(): within 3D scene" );

            // render text directly to primary surface
            GraphicsSharedPtr pGraphics;

            if( mbPageFlipping )
            {
                // render on top of backbuffer. We have
                // page flipping, anyway, thus this will
                // cost us nothing.
                pGraphics = createSurfaceGraphics( mpBackBufferSurface );
            }
            else
            {
                // render FPS directly to front buffer.
                // That saves us another explicit blit,
                // and for me, the FPS counter can blink,
                // if it likes to...
                pGraphics = createSurfaceGraphics( mpPrimarySurface );
            }

            if( !mbPageFlipping )
            {
                // clear background. We might be doing optimized redraws,
                // and the background under the FPS count will then not be
                // cleared.
                Gdiplus::SolidBrush aBrush(
                    Gdiplus::Color( 255, 255, 255 ) );

                pGraphics->FillRectangle( &aBrush,
                                          rPos.X, rPos.Y, 80.0, 20.0 );
            }

            Gdiplus::SolidBrush aBrush(
                Gdiplus::Color( 255, 0, 255 ) );
            Gdiplus::Font aFont( NULL,
                                 16,
                                 Gdiplus::FontStyleRegular,
                                 Gdiplus::UnitWorld,
                                 NULL );
            pGraphics->DrawString( reinterpret_cast<LPCWSTR>(rStr.getStr()),
                                   rStr.getLength(),
                                   &aFont,
                                   rPos,
                                   &aBrush );
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::renderMemAvailable
        //////////////////////////////////////////////////////////////////////////////////

        void DXRenderModule::renderMemAvailable() const
        {
            ENSURE_OR_THROW( !mnBeginSceneCount,
                              "DXRenderModule::renderMemAvailable(): within 3D scene" );

            const double nSurfaceMem( getAvailableSurfaceMem()/1024 );

            ::rtl::OUString text( ::rtl::math::doubleToUString( nSurfaceMem,
                                                                rtl_math_StringFormat_F,
                                                                2,'.',NULL,' ') );

            // pad with leading space
            while( text.getLength() < 6 )
                text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (" ")) + text;

            text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("S: ")) + text;

            renderInfoText( text,
                            Gdiplus::PointF( 0.0, 20) );


            const double nTexMem( getAvailableTextureMem()/1024 );

            text = ::rtl::math::doubleToUString( nTexMem,
                                                rtl_math_StringFormat_F,
                                                2,'.',NULL,' ');
            // pad with leading space
            while( text.getLength() < 6 )
                text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (" ")) + text;

            text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("T: ")) + text;

            renderInfoText( text,
                            Gdiplus::PointF( 0.0, 40) );

            VERBOSE_TRACE( "dxcanvas: %f free surface mem, %f free texture mem",
                           nSurfaceMem, nTexMem );
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::renderFPSCounter
        //////////////////////////////////////////////////////////////////////////////////

        void DXRenderModule::renderFPSCounter() const
        {
            ENSURE_OR_THROW( !mnBeginSceneCount,
                              "DXRenderModule::ren    derFPSCounter(): within 3D scene" );

            const double denominator( maLastUpdate.getElapsedTime() );
            maLastUpdate.reset();

            ::rtl::OUString text( ::rtl::math::doubleToUString( denominator == 0.0 ? 100.0 : 1.0/denominator,
                                                                rtl_math_StringFormat_F,
                                                                2,'.',NULL,' ') );

            // pad with leading space
            while( text.getLength() < 6 )
                text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (" ")) + text;

            text += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (" fps"));

            renderInfoText( text,
                            Gdiplus::PointF() );

            VERBOSE_TRACE( "dxcanvas: %f FPS",
                           denominator == 0.0 ? 100.0 : 1.0/denominator );
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::resize
        //////////////////////////////////////////////////////////////////////////////////

        void DXRenderModule::resize( const ::basegfx::B2IRange& rect )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if( mhWnd==0 )
                return;

            // don't do anything if the size didn't change.
            if(maSize.getX() == static_cast<sal_Int32>(rect.getWidth()) &&
               maSize.getY() == static_cast<sal_Int32>(rect.getHeight()))
               return;

            // TODO(Q2): use numeric cast to prevent overflow
            maSize.setX(static_cast<sal_Int32>(rect.getWidth()));
            maSize.setY(static_cast<sal_Int32>(rect.getHeight()));

            mpWindow->SetPosSizePixel(0,0,maSize.getX(),maSize.getY());
        }

        //////////////////////////////////////////////////////////////////////////////////
        // DXRenderModule::getPageSize
        //////////////////////////////////////////////////////////////////////////////////

        ::basegfx::B2IVector DXRenderModule::getPageSize()
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );
            return maPageSize;
        }

        ::canvas::ISurfaceSharedPtr DXRenderModule::createSurface( const ::basegfx::B2IVector& surfaceSize )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            const ::basegfx::B2IVector& rPageSize( getPageSize() );
            ::basegfx::B2ISize aSize(surfaceSize);
            if(!(aSize.getX()))
                aSize.setX(rPageSize.getX());
            if(!(aSize.getY()))
                aSize.setY(rPageSize.getY());

            if(mpTexture.use_count() == 1)
                return mpTexture;

            return ::canvas::ISurfaceSharedPtr(
                new DXSurface(*this,
                              aSize) );
        }

        void DXRenderModule::beginPrimitive( PrimitiveType eType )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            ENSURE_OR_THROW( !mnBeginSceneCount,
                              "DXRenderModule::beginPrimitive(): nested call" );

            ++mnBeginSceneCount;
            meType=eType;
            mnCount=0;
        }

        void DXRenderModule::endPrimitive()
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            --mnBeginSceneCount;
            meType=PRIMITIVE_TYPE_UNKNOWN;
            mnCount=0;
        }

        void DXRenderModule::pushVertex( const ::canvas::Vertex& vertex )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            switch(meType)
            {
                case PRIMITIVE_TYPE_TRIANGLE:
                {
                    maVertexCache.push_back(vertex);
                    ++mnCount;
                    mnCount &= 3;
                    break;
                }

                case PRIMITIVE_TYPE_QUAD:
                {
                    if(mnCount == 3)
                    {
                        const std::size_t size(maVertexCache.size());
                        ::canvas::Vertex v0(maVertexCache[size-1]);
                        ::canvas::Vertex v2(maVertexCache[size-3]);
                        maVertexCache.push_back(v0);
                        maVertexCache.push_back(vertex);
                        maVertexCache.push_back(v2);
                        mnCount=0;
                    }
                    else
                    {
                        maVertexCache.push_back(vertex);
                        ++mnCount;
                    }
                    break;
                }

                default:
                    OSL_FAIL( "DXRenderModule::pushVertex(): unexpected primitive types" );
                    break;
            }
        }

        bool DXRenderModule::isError()
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            return mbError;
        }

        void DXRenderModule::flushVertexCache()
        {
            if(!(maVertexCache.size()))
                return;

            mbError=true;

            if( FAILED(mpDirect3DDevice->BeginScene()) )
                return;

            // enable texture alpha blending
            if( FAILED(mpDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE)))
                return;

            // enable texture alpha modulation, for honoring fAlpha
            if( FAILED(mpDirect3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,
                                                        D3DTBLEND_MODULATEALPHA)) )
                return;

            // enable texture magnification filtering (don't care if this
            // fails, it's just visually more pleasant)
            mpDirect3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG,
                                             D3DFILTER_LINEAR);

            // enable texture minification filtering (don't care if this
            // fails, it's just visually more pleasant)
            mpDirect3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMIN,
                                             D3DFILTER_LINEAR);

            // enable subpixel texture output (don't care if this
            // fails, it's just visually more pleasant)
             mpDirect3DDevice->SetRenderState(D3DRENDERSTATE_SUBPIXEL,
                                             TRUE);

            // normal combination of object...
            if( FAILED(mpDirect3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
                                                        D3DBLEND_SRCALPHA)) )
                return;

            // ..and background color
            if( FAILED(mpDirect3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
                                                        D3DBLEND_INVSRCALPHA)) )
                return;

            // disable backface culling; this enables us to mirror sprites
            // by simply reverting the triangles, which, with enabled
            // culling, would be invisible otherwise
            if( FAILED(mpDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,
                                                        D3DCULL_NONE)) )
                return;

            mbError=false;

             const float nHalfPixelSizeX(0.5f/maPageSize.getX());
             const float nHalfPixelSizeY(0.5f/maPageSize.getY());
            sal_uInt32 nIndex(0);
            const std::size_t size(maVertexCache.size());
            D3DTLVERTEX *vertices = static_cast<D3DTLVERTEX *>(_alloca(sizeof(D3DTLVERTEX)*size));
            vertexCache_t::const_iterator it(maVertexCache.begin());
            while(it != maVertexCache.end())
            {
                vertices[nIndex++] = D3DTLVERTEX(
                    D3DVECTOR(static_cast<D3DVALUE>(it->x),
                              static_cast<D3DVALUE>(it->y),
                              static_cast<D3DVALUE>(it->z)),
                    1,
                    D3DRGBA(1,1,1,it->a),
                    D3DRGBA(0,0,0,0),
                    static_cast<float>(it->u + nHalfPixelSizeX),
                    static_cast<float>(it->v + nHalfPixelSizeY));
                ++it;
            }

            maVertexCache.clear();

            mbError |= FAILED(mpDirect3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
                                                              D3DVT_TLVERTEX,
                                                              (LPVOID)vertices,
                                                              size,
                                                              0));

            mbError |= FAILED(mpDirect3DDevice->EndScene());
        }
    }

    IDXRenderModuleSharedPtr createRenderModule( const ::Window& rParent )
    {
        return IDXRenderModuleSharedPtr( new DXRenderModule(rParent) );
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
