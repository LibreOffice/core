/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
