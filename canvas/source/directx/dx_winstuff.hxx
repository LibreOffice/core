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

#ifndef _DXCANVAS_WINSTUFF_HXX
#define _DXCANVAS_WINSTUFF_HXX

#include <algorithm>

#include <boost/shared_ptr.hpp>

#include <basegfx/numeric/ftools.hxx>

#ifdef _WINDOWS_
#error someone else included <windows.h>
#endif

// Enabling Direct3D Debug Information Further more, with registry key
// \\HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Direct3D\D3D9Debugging\\EnableCreationStack
// set to 1, sets a backtrace each time an object is created to the
// following global variable: LPCWSTR CreationCallStack
#if OSL_DEBUG_LEVEL > 0
# define D3D_DEBUG_INFO
#endif

#ifndef DIRECTX_VERSION
#error please define for which directx version we should compile
#endif

#if defined _MSC_VER
#pragma warning(push,1)
#endif


#define ULONG win32ULONG
#define GradientStyle_RECT win32GradientStyle_RECT
#define Polygon win32Polygon
#define PolyPolygon win32PolyPolygon
#undef WB_LEFT
#undef WB_RIGHT

#define WIN32_LEAN_AND_MEAN
#include <windows.h> // TODO(Q1): extract minimal set of required headers for gdiplus

#if DIRECTX_VERSION < 0x0900

    #include <multimon.h>

    // Be compatible with directdraw 3.0. Lets see how far this takes us
    #define DIRECTDRAW_VERSION 0x0300
    #include <ddraw.h>

    // Be compatible with direct3d 5.0. Lets see how far this takes us
    #define DIRECT3D_VERSION 0x0500
    #define D3D_OVERLOADS
    #include <d3d.h>

    typedef IDirectDrawSurface surface_type;

#elif WIN8_SDK == 1 //Windows 8 SDK

    #include <d3d9.h>

    typedef IDirect3DSurface9 surface_type;

#else

    #include <dxsdkver.h>
    #include <d3d9.h>
    #include <d3dx9.h>
    #if _DXSDK_BUILD_MAJOR < 1734 /* Earlier than the August 2009 DXSDK */
        #include <dxerr9.h>
    #else
        #include <dxerr.h>
    #endif

    typedef IDirect3DSurface9 surface_type;

#endif

#undef DrawText

#ifdef __MINGW32__
using ::std::max;
using ::std::min;
#endif

#include <gdiplus.h>

#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

namespace dxcanvas
{
    // some shared pointer typedefs to Gdiplus objects
    typedef ::boost::shared_ptr< Gdiplus::Graphics >        GraphicsSharedPtr;
    typedef ::boost::shared_ptr< Gdiplus::GraphicsPath >    GraphicsPathSharedPtr;
    typedef ::boost::shared_ptr< Gdiplus::Bitmap >          BitmapSharedPtr;
    typedef ::boost::shared_ptr< Gdiplus::CachedBitmap >    CachedBitmapSharedPtr;
    typedef ::boost::shared_ptr< Gdiplus::Font >            FontSharedPtr;
    typedef ::boost::shared_ptr< Gdiplus::Brush >           BrushSharedPtr;
    typedef ::boost::shared_ptr< Gdiplus::TextureBrush >    TextureBrushSharedPtr;

    /** COM object RAII wrapper

        This template wraps a Windows COM object, transparently
        handling lifetime issues the C++ way (i.e. releasing the
        reference when the object is destroyed)
     */
    template< typename T > class COMReference
    {
    public:
        typedef T Wrappee;

        COMReference() :
            mp( NULL )
        {
        }

        /** Create from raw pointer

            @attention This constructor assumes the interface is
            already acquired (unless p is NULL), no additional AddRef
            is called here.

            This caters e.g. for all DirectX factory methods, which
            return the created interfaces pre-acquired, into a raw
            pointer. Simply pass the pointer to this class, but don't
            call Release manually on it!

            @example IDirectDrawSurface* pSurface;
            pDD->CreateSurface(&aSurfaceDesc, &pSurface, NULL);
            mpSurface = COMReference< IDirectDrawSurface >(pSurface);

         */
        explicit COMReference( T* p ) :
            mp( p )
        {
        }

        COMReference( const COMReference& rNew ) :
            mp( NULL )
        {
            if( rNew.mp == NULL )
                return;

            rNew.mp->AddRef(); // do that _before_ assigning the
                               // pointer. Just in case...
            mp = rNew.mp;
        }

        COMReference& operator=( const COMReference& rRHS )
        {
            COMReference aTmp(rRHS);
            ::std::swap( mp, aTmp.mp );

            return *this;
        }

        ~COMReference()
        {
            reset();
        }

        int reset()
        {
            int refcount = 0;
            if( mp )
                refcount = mp->Release();

            mp = NULL;
            return refcount;
        }

        bool        is() const { return mp != NULL; }
        T*          get() const { return mp; }
        T*          operator->() const { return mp; }
        T&          operator*() const { return *mp; }

    private:
        T*  mp;
    };

    // get_pointer() enables boost::mem_fn to recognize COMReference
    template<class T> inline T * get_pointer(COMReference<T> const& p)
    {
        return p.get();
    }
}

#if defined _MSC_VER
#pragma warning(pop)
#endif

#undef DELETE
#undef PolyPolygon

#endif /* _DXCANVAS_WINSTUFF_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
