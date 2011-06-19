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

#include <osl/mutex.hxx>

#include "dx_winstuff.hxx"
#include "dx_gdiplususer.hxx"


namespace dxcanvas
{
    namespace
    {
        ::osl::Mutex*   p_gdiPlusUsageCountMutex( osl::Mutex::getGlobalMutex() );
        int             n_gdiPlusUsageCount( 0 );

        ULONG_PTR       a_GdiPlusToken;     // GDI+ handle. Owned by this object
    }

    GDIPlusUserSharedPtr GDIPlusUser::createInstance()
    {
        return GDIPlusUserSharedPtr( new GDIPlusUser() );
    }

    GDIPlusUser::~GDIPlusUser()
    {
        ::osl::MutexGuard aGuard( *p_gdiPlusUsageCountMutex );

        --n_gdiPlusUsageCount;

        if( n_gdiPlusUsageCount == 0 )
            Gdiplus::GdiplusShutdown( a_GdiPlusToken );
    }

    GDIPlusUser::GDIPlusUser()
    {
        ::osl::MutexGuard aGuard( *p_gdiPlusUsageCountMutex );

        if( n_gdiPlusUsageCount == 0 )
        {
            // Setup GDI+

            // No extras here, simply taking GdiplusStartupInput's
            // default constructor
            Gdiplus::GdiplusStartupInput gdiPlusStartupInput;

            Gdiplus::GdiplusStartup( &a_GdiPlusToken,
                                     &gdiPlusStartupInput,
                                     NULL );
        }

        ++n_gdiPlusUsageCount;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
