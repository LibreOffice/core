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

#pragma once

#include <algorithm>
#include <memory>

#include <basegfx/numeric/ftools.hxx>


#define WIN32_LEAN_AND_MEAN
#include <prewin.h>

// Enabling Direct3D Debug Information Further more, with registry key
// \\HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Direct3D\D3D9Debugging\\EnableCreationStack
// set to 1, sets a backtrace each time an object is created to the
// following global variable: LPCWSTR CreationCallStack
#if OSL_DEBUG_LEVEL > 0
# define D3D_DEBUG_INFO
#endif

#include <d3d9.h>

typedef IDirect3DSurface9 surface_type;


#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#include <gdiplus.h>

#undef max
#undef min


namespace dxcanvas
{
    // some shared pointer typedefs to Gdiplus objects
    typedef std::shared_ptr< Gdiplus::Graphics >        GraphicsSharedPtr;
    typedef std::shared_ptr< Gdiplus::GraphicsPath >    GraphicsPathSharedPtr;
    typedef std::shared_ptr< Gdiplus::Bitmap >          BitmapSharedPtr;
    typedef std::shared_ptr< Gdiplus::Font >            FontSharedPtr;
    typedef std::shared_ptr< Gdiplus::TextureBrush >    TextureBrushSharedPtr;

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
            mp( nullptr )
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
            mp( nullptr )
        {
            if( rNew.mp == nullptr )
                return;

            rNew.mp->AddRef(); // do that _before_ assigning the
                               // pointer. Just in case...
            mp = rNew.mp;
        }

        COMReference& operator=( const COMReference& rRHS )
        {
            COMReference aTmp(rRHS);
            std::swap( mp, aTmp.mp );

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

            mp = nullptr;
            return refcount;
        }

        bool        is() const { return mp != nullptr; }
        T*          get() const { return mp; }
        T*          operator->() const { return mp; }
        T&          operator*() const { return *mp; }

    private:
        T*  mp;
    };

}


#include <postwin.h>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
