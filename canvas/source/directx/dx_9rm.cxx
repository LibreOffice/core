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
#include <sal/log.hxx>

#include <string.h>

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <osl/thread.hxx>
#include <osl/time.h>
#include <tools/diagnose_ex.h>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/window.hxx>

#include <canvas/elapsedtime.hxx>
#include <canvas/canvastools.hxx>
#include <canvas/rendering/icolorbuffer.hxx>
#include <canvas/rendering/irendermodule.hxx>
#include <canvas/rendering/isurface.hxx>

#include "dx_config.hxx"
#include "dx_impltools.hxx"
#include "dx_rendermodule.hxx"

#define MIN_TEXTURE_SIZE (32)
//#define FAKE_MAX_NUMBER_TEXTURES (2)
//#define FAKE_MAX_TEXTURE_SIZE (4096)

#define VERTEX_BUFFER_SIZE (341*3) // 1023, the size of the internal
                                   // vertex buffer (must be divisable
                                   // by 3, as each triangle primitive
                                   // has 3 vertices)


using namespace ::com::sun::star;


// 'dxcanvas' namespace


namespace dxcanvas
{
    namespace
    {

        // monitorSupport


        class monitorSupport
        {
        public:

            monitorSupport() :
                mhLibrary(LoadLibraryW(L"user32.dll")),
                mpMonitorFromWindow(nullptr)
            {
                if(mhLibrary)
                    mpMonitorFromWindow = reinterpret_cast<fMonitorFromWindow>(
                        GetProcAddress(
                            mhLibrary,"MonitorFromWindow"));
            }

            ~monitorSupport()
            {
                if(mhLibrary)
                    FreeLibrary(mhLibrary);
                mhLibrary=nullptr;
            }

            HMONITOR MonitorFromWindow( HWND hwnd )
            {
                // return adapter_default in case something went wrong...
                if(!(mpMonitorFromWindow))
                    return HMONITOR(nullptr);
                // MONITOR_DEFAULTTONEAREST
                const DWORD dwFlags(0x00000002);
                return mpMonitorFromWindow(hwnd,dwFlags);
            }
        private:

            HINSTANCE mhLibrary;
            typedef HMONITOR (WINAPI *fMonitorFromWindow )( HWND hwnd, DWORD dwFlags );
            fMonitorFromWindow mpMonitorFromWindow;
        };

        monitorSupport aMonitorSupport;


        class DXRenderModule;


        // DXSurface


        /** ISurface implementation.

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
            ~DXSurface() override;

            virtual bool selectTexture() override;
            virtual bool isValid() override;
            virtual bool update( const ::basegfx::B2IPoint& rDestPos,
                                const ::basegfx::B2IRange& rSourceRect,
                                ::canvas::IColorBuffer&    rSource ) override;
            virtual ::basegfx::B2IVector getSize();

        private:
            /// Guard local methods against concurrent access to RenderModule
            class ImplRenderModuleGuard
            {
            public:
                /// make noncopyable
                ImplRenderModuleGuard(const ImplRenderModuleGuard&) = delete;
                const ImplRenderModuleGuard& operator=(const ImplRenderModuleGuard&) = delete;

                explicit ImplRenderModuleGuard( DXRenderModule& rRenderModule );
                ~ImplRenderModuleGuard();

            private:
                DXRenderModule& mrRenderModule;
            };

            DXRenderModule&                  mrRenderModule;
            COMReference<IDirect3DTexture9>  mpTexture;

            ::basegfx::B2IVector             maSize;
        };


        // DXRenderModule


        /// Default implementation of IDXRenderModule
        class DXRenderModule : public IDXRenderModule
        {
        public:
            explicit DXRenderModule( const vcl::Window& rWindow );
            ~DXRenderModule() override;

            virtual void lock() const override { maMutex.acquire(); }
            virtual void unlock() const override { maMutex.release(); }

            virtual COMReference<IDirect3DSurface9>
                createSystemMemorySurface( const ::basegfx::B2IVector& rSize ) override;
            virtual void disposing() override;
            virtual HWND getHWND() const override { return mhWnd; }
            virtual void screenShot() override;

            virtual bool flip( const ::basegfx::B2IRectangle& rUpdateArea,
                               const ::basegfx::B2IRectangle& rCurrWindowArea ) override;

            virtual void resize( const ::basegfx::B2IRange& rect ) override;
            virtual ::basegfx::B2IVector getPageSize() override;
            virtual std::shared_ptr<canvas::ISurface> createSurface( const ::basegfx::B2IVector& surfaceSize ) override;
            virtual void beginPrimitive( PrimitiveType eType ) override;
            virtual void endPrimitive() override;
            virtual void pushVertex( const ::canvas::Vertex& vertex ) override;
            virtual bool isError() override;

            COMReference<IDirect3DDevice9> getDevice() { return mpDevice; }

            void flushVertexCache();
            void commitVertexCache();

        private:

            bool create( const vcl::Window& rWindow );
            bool createDevice();
            bool verifyDevice( const UINT nAdapter );
            UINT getAdapterFromWindow();

            /** This object represents the DirectX state machine.  In order
                to serialize access to DirectX's global state, a global
                mutex is required.
            */
            static ::osl::Mutex                         maMutex;

            HWND                                        mhWnd;
            COMReference<IDirect3DDevice9>              mpDevice;
            COMReference<IDirect3D9>                    mpDirect3D9;
            COMReference<IDirect3DSwapChain9>           mpSwapChain;
            COMReference<IDirect3DVertexBuffer9>        mpVertexBuffer;
            std::shared_ptr<canvas::ISurface>                 mpTexture;
            VclPtr<SystemChildWindow>                   mpWindow;
            ::basegfx::B2IVector                        maSize;
            typedef std::vector<canvas::Vertex>         vertexCache_t;
            vertexCache_t                               maVertexCache;
            std::size_t                                 mnCount;
            int                                         mnBeginSceneCount;
            bool                                        mbCanUseDynamicTextures;
            bool                                        mbError;
            PrimitiveType                               meType;
            ::basegfx::B2IVector                        maPageSize;
            D3DPRESENT_PARAMETERS                       mad3dpp;

            bool isDisposed() const { return (mhWnd==nullptr); }

            struct dxvertex
            {
                float x,y,z,rhw;
                DWORD diffuse;
                float u,v;
            };

            std::size_t                                 maNumVertices;
            std::size_t                                 maWriteIndex;
            std::size_t                                 maReadIndex;
        };

        ::osl::Mutex DXRenderModule::maMutex;


        // DXSurface::ImplRenderModuleGuard


        DXSurface::ImplRenderModuleGuard::ImplRenderModuleGuard(
            DXRenderModule& rRenderModule ) :
            mrRenderModule( rRenderModule )
        {
            mrRenderModule.lock();
        }

        DXSurface::ImplRenderModuleGuard::~ImplRenderModuleGuard()
        {
            mrRenderModule.unlock();
        }

#ifdef FAKE_MAX_NUMBER_TEXTURES
        static sal_uInt32 gNumSurfaces = 0;
#endif

        // DXSurface::DXSurface


        DXSurface::DXSurface( DXRenderModule&           rRenderModule,
                              const ::basegfx::B2ISize& rSize ) :
            mrRenderModule(rRenderModule),
            mpTexture(nullptr),
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

            COMReference<IDirect3DDevice9> pDevice(rRenderModule.getDevice());

            IDirect3DTexture9 *pTexture(nullptr);
            if(FAILED(pDevice->CreateTexture(
                rSize.getX(),
                rSize.getY(),
                1,0,D3DFMT_A8R8G8B8,
                D3DPOOL_MANAGED,
                &pTexture,nullptr)))
                return;

            mpTexture=COMReference<IDirect3DTexture9>(pTexture);
            maSize = rSize;
        }


        // DXSurface::~DXSurface


        DXSurface::~DXSurface()
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );

#ifdef FAKE_MAX_NUMBER_TEXTURES
            gNumSurfaces--;
#endif
        }


        // DXSurface::selectTexture


        bool DXSurface::selectTexture()
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );
            mrRenderModule.flushVertexCache();
            COMReference<IDirect3DDevice9> pDevice(mrRenderModule.getDevice());

            if( FAILED(pDevice->SetTexture(0,mpTexture.get())) )
                return false;

            return true;
        }


        // DXSurface::isValid


        bool DXSurface::isValid()
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );

            if(!(mpTexture.is()))
                return false;
            return true;
        }


        // DXSurface::update


        bool DXSurface::update( const ::basegfx::B2IPoint& rDestPos,
                                const ::basegfx::B2IRange& rSourceRect,
                                ::canvas::IColorBuffer&    rSource )
        {
            ImplRenderModuleGuard aGuard( mrRenderModule );

            // can't update if surface is not valid, that means
            // either not existent nor restored...
            if(!(isValid()))
                return false;

            D3DLOCKED_RECT aLockedRect;
            RECT rect;
            rect.left = std::max(sal_Int32(0),rDestPos.getX());
            rect.top =  std::max(sal_Int32(0),rDestPos.getY());
            // to avoid interpolation artifacts from other textures,
            // the surface manager allocates one pixel gap between
            // them. Clear that to transparent.
            rect.right = std::min(maSize.getX(),
                                  rect.left + sal_Int32(rSourceRect.getWidth()+1));
            rect.bottom = std::min(maSize.getY(),
                                   rect.top + sal_Int32(rSourceRect.getHeight()+1));
            const bool bClearRightColumn( rect.right < maSize.getX() );
            const bool bClearBottomRow( rect.bottom < maSize.getY() );

            if(SUCCEEDED(mpTexture->LockRect(0,&aLockedRect,&rect,D3DLOCK_NOSYSLOCK)))
            {
                if(sal_uInt8* pImage = rSource.lock())
                {
                    switch( rSource.getFormat() )
                    {
                        case ::canvas::IColorBuffer::Format::A8R8G8B8:
                        {
                            const std::size_t nSourceBytesPerPixel(4);
                            const std::size_t nSourcePitchInBytes(rSource.getStride());
                            pImage += rSourceRect.getMinY()*nSourcePitchInBytes;
                            pImage += rSourceRect.getMinX()*nSourceBytesPerPixel;

                            // calculate the destination memory address
                            sal_uInt8 *pDst = static_cast<sal_uInt8*>(aLockedRect.pBits);

                            const sal_uInt32 nNumBytesToCopy(
                                static_cast<sal_uInt32>(
                                    rSourceRect.getWidth())*
                                nSourceBytesPerPixel);
                            const sal_uInt64 nNumLines(rSourceRect.getHeight());

                            for(sal_uInt64 i=0; i<nNumLines; ++i)
                            {
                                memcpy(pDst,pImage,nNumBytesToCopy);

                                if( bClearRightColumn )
                                {
                                    // to avoid interpolation artifacts
                                    // from other textures, the surface
                                    // manager allocates one pixel gap
                                    // between them. Clear that to
                                    // transparent.
                                    pDst[nNumBytesToCopy] =
                                        pDst[nNumBytesToCopy+1] =
                                        pDst[nNumBytesToCopy+2] =
                                        pDst[nNumBytesToCopy+3] = 0x00;
                                }
                                pDst += aLockedRect.Pitch;
                                pImage += nSourcePitchInBytes;
                            }

                            if( bClearBottomRow )
                                memset(pDst, 0, nNumBytesToCopy+4);
                        }
                        break;

                        case ::canvas::IColorBuffer::Format::X8R8G8B8:
                        {
                            const std::size_t nSourceBytesPerPixel(4);
                            const std::size_t nSourcePitchInBytes(rSource.getStride());
                            pImage += rSourceRect.getMinY()*nSourcePitchInBytes;
                            pImage += rSourceRect.getMinX()*nSourceBytesPerPixel;

                            // calculate the destination memory address
                            sal_uInt8 *pDst = static_cast<sal_uInt8*>(aLockedRect.pBits);

                            const sal_Int32 nNumLines(
                                sal::static_int_cast<sal_Int32>(rSourceRect.getHeight()));
                            const sal_Int32 nNumColumns(
                                sal::static_int_cast<sal_Int32>(rSourceRect.getWidth()));
                            for(sal_Int32 i=0; i<nNumLines; ++i)
                            {
                                sal_uInt32 *pSrc32 = reinterpret_cast<sal_uInt32 *>(pImage);
                                sal_uInt32 *pDst32 = reinterpret_cast<sal_uInt32 *>(pDst);
                                for(sal_Int32 j=0; j<nNumColumns; ++j)
                                    pDst32[j] = 0xFF000000 | pSrc32[j];

                                if( bClearRightColumn )
                                    pDst32[nNumColumns] = 0xFF000000;

                                pDst += aLockedRect.Pitch;
                                pImage += nSourcePitchInBytes;
                            }

                            if( bClearBottomRow )
                                memset(pDst, 0, 4*(nNumColumns+1));
                        }
                        break;

                        default:
                            ENSURE_OR_RETURN_FALSE(false,
                                            "DXSurface::update(): Unknown/unimplemented buffer format" );
                            break;
                    }

                    rSource.unlock();
                }

                return SUCCEEDED(mpTexture->UnlockRect(0));
            }

            return true;
        }


        // DXSurface::getSize


        ::basegfx::B2IVector DXSurface::getSize()
        {
            return maSize;
        }

        // DXRenderModule::DXRenderModule


        DXRenderModule::DXRenderModule( const vcl::Window& rWindow ) :
            mhWnd(nullptr),
            mpDevice(),
            mpDirect3D9(),
            mpSwapChain(),
            mpVertexBuffer(),
            mpTexture(),
            maSize(),
            maVertexCache(),
            mnCount(0),
            mnBeginSceneCount(0),
            mbCanUseDynamicTextures(false),
            mbError( false ),
            meType( PrimitiveType::Unknown ),
            maPageSize(),
            mad3dpp(),
            maNumVertices( VERTEX_BUFFER_SIZE ),
            maWriteIndex(0),
            maReadIndex(0)
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if(!(create(rWindow)))
            {
                throw lang::NoSupportException( "Could not create DirectX device!" );
            }

            // allocate a single texture surface which can be used later.
            // we also use this to calibrate the page size.
            ::basegfx::B2IVector aPageSize(maPageSize);
            while(true)
            {
                mpTexture = std::shared_ptr<canvas::ISurface>(
                    new DXSurface(*this,aPageSize));
                if(mpTexture->isValid())
                    break;

                aPageSize.setX(aPageSize.getX()>>1);
                aPageSize.setY(aPageSize.getY()>>1);
                if((aPageSize.getX() < MIN_TEXTURE_SIZE) ||
                   (aPageSize.getY() < MIN_TEXTURE_SIZE))
                {
                    throw lang::NoSupportException(
                        "Could not create DirectX device - insufficient texture space!" );
                }
            }
            maPageSize=aPageSize;

            IDirect3DVertexBuffer9 *pVB(nullptr);
            if( FAILED(mpDevice->CreateVertexBuffer(sizeof(dxvertex)*maNumVertices,
                                                    D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,
                                                    D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1,
                                                    D3DPOOL_DEFAULT,
                                                    &pVB,
                                                    nullptr)) )
            {
                throw lang::NoSupportException(
                    "Could not create DirectX device - out of memory!" );
            }

            mpVertexBuffer=COMReference<IDirect3DVertexBuffer9>(pVB);
        }


        // DXRenderModule::~DXRenderModule


        DXRenderModule::~DXRenderModule()
        {
            disposing();
        }


        // DXRenderModule::disposing


        void DXRenderModule::disposing()
        {
            if(!(mhWnd))
                return;

            mpTexture.reset();
            mpWindow.disposeAndClear();
            mhWnd=nullptr;

            // refrain from releasing the DX9 objects. We're the only
            // ones holding references to them, and it might be
            // dangerous to destroy the DX9 device, before all other
            // objects are dead.
        }


        // DXRenderModule::create


        bool DXRenderModule::create( const vcl::Window& rWindow )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            // TODO(F2): since we would like to share precious hardware
            // resources, the direct3d9 object should be global. each new
            // request for a canvas should only create a new swapchain.
            mpDirect3D9 = COMReference<IDirect3D9>(
                Direct3DCreate9(D3D_SDK_VERSION));
            if(!mpDirect3D9.is())
                return false;

            maVertexCache.reserve( 1024 );

            mpWindow.disposeAndClear();
            mpWindow.reset( VclPtr<SystemChildWindow>::Create(
                              const_cast<vcl::Window *>(&rWindow), 0) );

            // system child window must not receive mouse events
            mpWindow->SetMouseTransparent( TRUE );

            // parent should receive paint messages as well
            mpWindow->SetParentClipMode(ParentClipMode::NoClip);

            // the system child window must not clear its background
            mpWindow->EnableEraseBackground( false );

            mpWindow->SetControlForeground();
            mpWindow->SetControlBackground();

            const SystemEnvData *pData = mpWindow->GetSystemData();
            const HWND hwnd(reinterpret_cast<HWND>(pData->hWnd));
            mhWnd = hwnd;

            ENSURE_OR_THROW( IsWindow( reinterpret_cast<HWND>(mhWnd) ),
                            "DXRenderModule::create() No valid HWND given." );

            // retrieve position and size of the parent window
            const ::Size &rSizePixel(rWindow.GetSizePixel());

            // remember the size of the parent window, since we
            // need to use this for our child window.
            maSize.setX(static_cast<sal_Int32>(rSizePixel.Width()));
            maSize.setY(static_cast<sal_Int32>(rSizePixel.Height()));

            // let the child window cover the same size as the parent window.
            mpWindow->setPosSizePixel(0,0,maSize.getX(),maSize.getY());

            // create a device from the direct3d9 object.
            if(!(createDevice()))
            {
                mpWindow.disposeAndClear();
                return false;
            }

            mpWindow->Show();

            return true;
        }


        // DXRenderModule::verifyDevice


        bool DXRenderModule::verifyDevice( const UINT nAdapter )
        {
            ENSURE_OR_THROW( mpDirect3D9.is(),
                              "DXRenderModule::verifyDevice() No valid device." );

            // ask direct3d9 about the capabilities of hardware devices on a specific adapter.
            // here we decide if the underlying hardware of the machine 'is good enough'.
            // since we only need a tiny little fraction of what could be used, this
            // is basically a no-op.
            D3DCAPS9 aCaps;
            if(FAILED(mpDirect3D9->GetDeviceCaps(nAdapter,D3DDEVTYPE_HAL,&aCaps)))
                return false;
            if(!(aCaps.MaxTextureWidth))
                return false;
            if(!(aCaps.MaxTextureHeight))
                return false;
            maPageSize = ::basegfx::B2IVector(aCaps.MaxTextureWidth,aCaps.MaxTextureHeight);

            // check device against white & blacklist entries
            D3DADAPTER_IDENTIFIER9 aIdent;
            if(FAILED(mpDirect3D9->GetAdapterIdentifier(nAdapter,0,&aIdent)))
                return false;

            DXCanvasItem aConfigItem;
            DXCanvasItem::DeviceInfo aInfo;
            aInfo.nVendorId = aIdent.VendorId;
            aInfo.nDeviceId = aIdent.DeviceId;
            aInfo.nDeviceSubSysId = aIdent.SubSysId;
            aInfo.nDeviceRevision = aIdent.Revision;

            aInfo.nDriverId = HIWORD(aIdent.DriverVersion.HighPart);
            aInfo.nDriverVersion = LOWORD(aIdent.DriverVersion.HighPart);
            aInfo.nDriverSubVersion = HIWORD(aIdent.DriverVersion.LowPart);
            aInfo.nDriverBuildId = LOWORD(aIdent.DriverVersion.LowPart);

            if( !aConfigItem.isDeviceUsable(aInfo) )
                return false;

            if( aConfigItem.isBlacklistCurrentDevice() )
            {
                aConfigItem.blacklistDevice(aInfo);
                return false;
            }

            aConfigItem.adaptMaxTextureSize(maPageSize);

            mbCanUseDynamicTextures = (aCaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) != 0;

            return true;
        }


        // DXRenderModule::createDevice


        bool DXRenderModule::createDevice()
        {
            // we expect that the caller provides us with a valid HWND
            ENSURE_OR_THROW( IsWindow(mhWnd),
                              "DXRenderModule::createDevice() No valid HWND given." );

            // we expect that the caller already created the direct3d9 object.
            ENSURE_OR_THROW( mpDirect3D9.is(),
                              "DXRenderModule::createDevice() no direct3d?." );

            // find the adapter identifier from the window.
            const UINT aAdapter(getAdapterFromWindow());
            if(aAdapter == static_cast<UINT>(-1))
                return false;

            // verify that device possibly works
            if( !verifyDevice(aAdapter) )
                return false;

            // query the display mode from the selected adapter.
            // we'll later request the backbuffer format to be same
            // same as the display format.
            D3DDISPLAYMODE d3ddm;
            mpDirect3D9->GetAdapterDisplayMode(aAdapter,&d3ddm);

            // we need to use D3DSWAPEFFECT_COPY here since the canvas-api has
            // basically nothing to do with efficient resource handling. it tries
            // to avoid drawing whenever possible, which is simply not the most
            // efficient way we could leverage the hardware in this case. it would
            // be far better to redraw the backbuffer each time we would like to
            // display the content of the backbuffer, but we need to face reality
            // here and follow how the canvas was designed.

            // Strictly speaking, we don't need a full screen worth of
            // backbuffer here. We could also scale dynamically with
            // the current window size, but this will make it
            // necessary to temporarily have two buffers while copying
            // from the old to the new one. What's more, at the time
            // we need a larger buffer, DX might not have sufficient
            // resources available, and we're then left with too small
            // a back buffer, and no way of falling back to a
            // different canvas implementation.
            ZeroMemory( &mad3dpp, sizeof(mad3dpp) );
            mad3dpp.BackBufferWidth = std::max(maSize.getX(),
                                               sal_Int32(d3ddm.Width));
            mad3dpp.BackBufferHeight = std::max(maSize.getY(),
                                                sal_Int32(d3ddm.Height));
            mad3dpp.BackBufferCount = 1;
            mad3dpp.Windowed = TRUE;
            mad3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
            mad3dpp.BackBufferFormat = d3ddm.Format;
            mad3dpp.EnableAutoDepthStencil = FALSE;
            mad3dpp.hDeviceWindow = mhWnd;
            mad3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

            // now create the device, first try hardware vertex processing,
            // then software vertex processing. if both queries fail, we give up
            // and indicate failure.
            IDirect3DDevice9 *pDevice(nullptr);
            if(FAILED(mpDirect3D9->CreateDevice(aAdapter,
                                                D3DDEVTYPE_HAL,
                                                mhWnd,
                                                D3DCREATE_HARDWARE_VERTEXPROCESSING|
                                                D3DCREATE_MULTITHREADED|D3DCREATE_FPU_PRESERVE,
                                                &mad3dpp,
                                                &pDevice)))
                if(FAILED(mpDirect3D9->CreateDevice(aAdapter,
                                                    D3DDEVTYPE_HAL,
                                                    mhWnd,
                                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING|
                                                    D3DCREATE_MULTITHREADED|D3DCREATE_FPU_PRESERVE,
                                                    &mad3dpp,
                                                    &pDevice)))
                    return false;

            // got it, store it in a safe place...
            mpDevice=COMReference<IDirect3DDevice9>(pDevice);

            // After CreateDevice, the first swap chain already exists, so just get it...
            IDirect3DSwapChain9 *pSwapChain(nullptr);
            pDevice->GetSwapChain(0,&pSwapChain);
            mpSwapChain=COMReference<IDirect3DSwapChain9>(pSwapChain);
            if( !mpSwapChain.is() )
                return false;

            // clear the render target [which is the backbuffer in this case].
            // we are forced to do this once, and furthermore right now.
            // please note that this is only possible since we created the
            // backbuffer with copy semantics [the content is preserved after
            // calls to Present()], which is an unnecessarily expensive operation.
            LPDIRECT3DSURFACE9 pBackBuffer = nullptr;
            mpSwapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer);
            mpDevice->SetRenderTarget( 0, pBackBuffer );
            mpDevice->Clear(0,nullptr,D3DCLEAR_TARGET,0,1.0f,0);
            pBackBuffer->Release();

            return true;
        }


        // DXRenderModule::createSystemMemorySurface


        COMReference<IDirect3DSurface9> DXRenderModule::createSystemMemorySurface( const ::basegfx::B2IVector& rSize )
        {
            if(isDisposed())
                return COMReference<IDirect3DSurface9>(nullptr);

            // please note that D3DFMT_X8R8G8B8 is the only format we're
            // able to choose here, since GetDC() doesn't support any
            // other 32bit-format.
            IDirect3DSurface9 *pSurface(nullptr);
            if( FAILED(mpDevice->CreateOffscreenPlainSurface(
                           rSize.getX(),
                           rSize.getY(),
                           D3DFMT_X8R8G8B8,
                           D3DPOOL_SYSTEMMEM,
                           &pSurface,
                           nullptr)) )
            {
                throw lang::NoSupportException(
                    "Could not create offscreen surface - out of mem!" );
            }

            return COMReference<IDirect3DSurface9>(pSurface);
        }


        // DXRenderModule::flip


        bool DXRenderModule::flip( const ::basegfx::B2IRectangle& rUpdateArea,
                                   const ::basegfx::B2IRectangle& /*rCurrWindowArea*/ )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if(isDisposed() || !mpSwapChain.is())
                return false;

            flushVertexCache();

            // TODO(P2): Might be faster to actually pass update area here
            RECT aRect =
                {
                    rUpdateArea.getMinX(),
                    rUpdateArea.getMinY(),
                    rUpdateArea.getMaxX(),
                    rUpdateArea.getMaxY()
                };
            HRESULT hr(mpSwapChain->Present(&aRect,&aRect,nullptr,nullptr,0));
            if(FAILED(hr))
            {
                if(hr != D3DERR_DEVICELOST)
                    return false;

                // interestingly enough, sometimes the Reset() below
                // *still* causes DeviceLost errors. So, cycle until
                // DX was kind enough to really reset the device...
                do
                {
                    mpVertexBuffer.reset();
                    hr = mpDevice->Reset(&mad3dpp);
                    if(SUCCEEDED(hr))
                    {
                        IDirect3DVertexBuffer9 *pVB(nullptr);
                        if( FAILED(mpDevice->CreateVertexBuffer(sizeof(dxvertex)*maNumVertices,
                                                                D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,
                                                                D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1,
                                                                D3DPOOL_DEFAULT,
                                                                &pVB,
                                                                nullptr)) )
                        {
                            throw lang::NoSupportException(
                                "Could not create DirectX device - out of memory!" );
                        }
                        mpVertexBuffer=COMReference<IDirect3DVertexBuffer9>(pVB);

                        // retry after the restore
                        if(SUCCEEDED(mpSwapChain->Present(&aRect,&aRect,nullptr,nullptr,0)))
                            return true;
                    }

                    osl::Thread::wait(std::chrono::seconds(1));
                }
                while(hr == D3DERR_DEVICELOST);

                return false;
            }

            return true;
        }


        // DXRenderModule::screenShot


        void DXRenderModule::screenShot()
        {
        }


        // DXRenderModule::resize


        void DXRenderModule::resize( const ::basegfx::B2IRange& rect )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if(isDisposed())
                return;

            // don't do anything if the size didn't change.
            if(maSize.getX() == static_cast<sal_Int32>(rect.getWidth()) &&
               maSize.getY() == static_cast<sal_Int32>(rect.getHeight()))
               return;

            // TODO(Q2): use numeric cast to prevent overflow
            maSize.setX(static_cast<sal_Int32>(rect.getWidth()));
            maSize.setY(static_cast<sal_Int32>(rect.getHeight()));

            mpWindow->setPosSizePixel(0,0,maSize.getX(),maSize.getY());

            // resize back buffer, if necessary


            // don't attempt to create anything if the
            // requested size is NULL.
            if(!(maSize.getX()))
                return;
            if(!(maSize.getY()))
                return;

            // backbuffer too small (might happen, if window is
            // maximized across multiple monitors)
            if( sal_Int32(mad3dpp.BackBufferWidth) < maSize.getX() ||
                sal_Int32(mad3dpp.BackBufferHeight) < maSize.getY() )
            {
                mad3dpp.BackBufferWidth = maSize.getX();
                mad3dpp.BackBufferHeight = maSize.getY();

                // clear before, save resources
                mpSwapChain.reset();

                IDirect3DSwapChain9 *pSwapChain(nullptr);
                if(FAILED(mpDevice->CreateAdditionalSwapChain(&mad3dpp,&pSwapChain)))
                    return;
                mpSwapChain=COMReference<IDirect3DSwapChain9>(pSwapChain);

                // clear the render target [which is the backbuffer in this case].
                // we are forced to do this once, and furthermore right now.
                // please note that this is only possible since we created the
                // backbuffer with copy semantics [the content is preserved after
                // calls to Present()], which is an unnecessarily expensive operation.
                LPDIRECT3DSURFACE9 pBackBuffer = nullptr;
                mpSwapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer);
                mpDevice->SetRenderTarget( 0, pBackBuffer );
                mpDevice->Clear(0,nullptr,D3DCLEAR_TARGET,0,1.0f,0);
                pBackBuffer->Release();
            }
        }


        // DXRenderModule::getPageSize


        ::basegfx::B2IVector DXRenderModule::getPageSize()
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );
            return maPageSize;
        }


        // DXRenderModule::createSurface


        std::shared_ptr<canvas::ISurface> DXRenderModule::createSurface( const ::basegfx::B2IVector& surfaceSize )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if(isDisposed())
                return std::shared_ptr<canvas::ISurface>();

            const ::basegfx::B2IVector& rPageSize( getPageSize() );
            ::basegfx::B2ISize aSize(surfaceSize);
            if(!(aSize.getX()))
                aSize.setX(rPageSize.getX());
            if(!(aSize.getY()))
                aSize.setY(rPageSize.getY());

            if(mpTexture.use_count() == 1)
                return mpTexture;

            return std::shared_ptr<canvas::ISurface>( new DXSurface(*this,aSize) );
        }


        // DXRenderModule::beginPrimitive


        void DXRenderModule::beginPrimitive( PrimitiveType eType )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if(isDisposed())
                return;

            ENSURE_OR_THROW( !mnBeginSceneCount,
                              "DXRenderModule::beginPrimitive(): nested call" );

            ++mnBeginSceneCount;
            meType=eType;
            mnCount=0;
        }


        // DXRenderModule::endPrimitive


        void DXRenderModule::endPrimitive()
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if(isDisposed())
                return;

            --mnBeginSceneCount;
            meType = PrimitiveType::Unknown;
            mnCount = 0;
        }


        // DXRenderModule::pushVertex


        void DXRenderModule::pushVertex( const ::canvas::Vertex& vertex )
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            if(isDisposed())
                return;

            switch(meType)
            {
                case PrimitiveType::Triangle:
                {
                    maVertexCache.push_back(vertex);
                    ++mnCount;
                    mnCount &= 3;
                    break;
                }

                case PrimitiveType::Quad:
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
                    SAL_WARN("canvas.directx", "DXRenderModule::pushVertex(): unexpected primitive type");
                    break;
            }
        }


        // DXRenderModule::isError


        bool DXRenderModule::isError()
        {
            // TODO(P2): get rid of those fine-grained locking
            ::osl::MutexGuard aGuard( maMutex );

            return mbError;
        }


        // DXRenderModule::getAdapterFromWindow


        UINT DXRenderModule::getAdapterFromWindow()
        {
            HMONITOR hMonitor(aMonitorSupport.MonitorFromWindow(mhWnd));
            UINT aAdapterCount(mpDirect3D9->GetAdapterCount());
            for(UINT i=0; i<aAdapterCount; ++i)
                if(hMonitor == mpDirect3D9->GetAdapterMonitor(i))
                    return i;
            return static_cast<UINT>(-1);
        }


        // DXRenderModule::commitVertexCache


        void DXRenderModule::commitVertexCache()
        {
            if(maReadIndex != maWriteIndex)
            {
                const std::size_t nVertexStride = sizeof(dxvertex);
                const unsigned int nNumVertices = maWriteIndex-maReadIndex;
                const unsigned int nNumPrimitives = nNumVertices / 3;

                if(FAILED(mpDevice->SetStreamSource(0,mpVertexBuffer.get(),0,nVertexStride)))
                    return;

                if(FAILED(mpDevice->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)))
                    return;

                if(FAILED(mpDevice->BeginScene()))
                    return;

                mbError |= FAILED(mpDevice->DrawPrimitive(D3DPT_TRIANGLELIST,maReadIndex,nNumPrimitives));
                mbError |= FAILED(mpDevice->EndScene());

                maReadIndex += nNumVertices;
            }
        }


        // DXRenderModule::flushVertexCache


        void DXRenderModule::flushVertexCache()
        {
            if(maVertexCache.empty())
                return;

            mbError=true;

            if( FAILED(mpDevice->SetRenderState(D3DRS_LIGHTING,FALSE)))
                return;

            // enable texture alpha blending
            if( FAILED(mpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE)))
                return;

            mpDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
            mpDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
            mpDevice->SetSamplerState(0,D3DSAMP_ADDRESSU ,D3DTADDRESS_CLAMP );
            mpDevice->SetSamplerState(0,D3DSAMP_ADDRESSV ,D3DTADDRESS_CLAMP );

            // configure the fixed-function pipeline.
            // the only 'feature' we need here is to modulate the alpha-channels
            // from the texture and the interpolated diffuse color. the result
            // will then be blended with the backbuffer.
            // fragment color = texture color * diffuse.alpha.
            mpDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
            mpDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
            mpDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);

            // normal combination of object...
            if( FAILED(mpDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA)) )
                return;

            // ..and background color
            if( FAILED(mpDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA)) )
                return;

            // disable backface culling; this enables us to mirror sprites
            // by simply reverting the triangles, which, with enabled
            // culling, would be invisible otherwise
            if( FAILED(mpDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE)) )
                return;

            mbError=false;

            std::size_t nSize(maVertexCache.size());
            const std::size_t nVertexStride = sizeof(dxvertex);

            const ::basegfx::B2IVector aPageSize(getPageSize());
            const float nHalfPixelSizeX(0.5f/aPageSize.getX());
            const float nHalfPixelSizeY(0.5f/aPageSize.getY());
            vertexCache_t::const_iterator it(maVertexCache.begin());

            while( nSize )
            {
                DWORD dwLockFlags(D3DLOCK_NOOVERWRITE);

                // Check to see if there's space for the current set of
                // vertices in the buffer.
                if( maNumVertices - maWriteIndex < nSize )
                {
                    commitVertexCache();
                    dwLockFlags = D3DLOCK_DISCARD;
                    maWriteIndex = 0;
                    maReadIndex = 0;
                }

                dxvertex *vertices(nullptr);
                const std::size_t nNumVertices(
                    std::min(maNumVertices - maWriteIndex,
                             nSize));
                if(FAILED(mpVertexBuffer->Lock(maWriteIndex*nVertexStride,
                                               nNumVertices*nVertexStride,
                                               reinterpret_cast<void **>(&vertices),
                                               dwLockFlags)))
                    return;

                std::size_t nIndex(0);
                while( nIndex < nNumVertices )
                {
                    dxvertex &dest = vertices[nIndex++];
                    dest.x=it->x;
                    dest.y=it->y;
                    dest.z=it->z;
                    dest.rhw=1;
                    const sal_uInt32 alpha(static_cast<sal_uInt32>(it->a*255.0f));
                    dest.diffuse=D3DCOLOR_ARGB(alpha,255,255,255);
                    dest.u=static_cast<float>(it->u + nHalfPixelSizeX);
                    dest.v=static_cast<float>(it->v + nHalfPixelSizeY);
                    ++it;
                }

                mpVertexBuffer->Unlock();

                // Advance to the next position in the vertex buffer.
                maWriteIndex += nNumVertices;
                nSize -= nNumVertices;

                commitVertexCache();
            }

            maVertexCache.clear();
        }
    }


    // createRenderModule


    IDXRenderModuleSharedPtr createRenderModule( const vcl::Window& rParent )
    {
        return IDXRenderModuleSharedPtr( new DXRenderModule(rParent) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
