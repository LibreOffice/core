/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_winstuff.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 18:00:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


#define BOOL win32BOOL
#define INT32 win32INT32
#define UINT32 win32UINT32
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

#else

    #include <d3d9.h>
    #include <d3dx9.h>
    #include <dxerr9.h>

    typedef IDirect3DSurface9 surface_type;

#endif

#undef DrawText

// Needed for #?$&/@ gdiplus header
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define __WORKAROUND_MAX_DEFINED__
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define __WORKAROUND_MIN_DEFINED__
#endif

#include <gdiplus.h>

#ifdef __WORKAROUND_MAX_DEFINED__
#undef max
#endif

#ifdef __WORKAROUND_MIN_DEFINED__
#undef min
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
#undef BOOL
#undef INT32
#undef UINT32
#undef PolyPolygon

#endif /* _DXCANVAS_WINSTUFF_HXX */
