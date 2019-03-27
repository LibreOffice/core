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

#include <osl/mutex.hxx>

#include "dx_gdiplususer.hxx"
#include "dx_winstuff.hxx"


namespace dxcanvas
{
    namespace
    {
        ::osl::Mutex*   p_gdiPlusUsageCountMutex( osl::Mutex::getGlobalMutex() );
        int             n_gdiPlusUsageCount( 0 );

        ULONG_PTR       a_GdiPlusToken;     // GDI+ handle. Owned by this object
    }

    GDIPlusUser::GDIPlusUserSharedPtr GDIPlusUser::createInstance()
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
                                     nullptr );
        }

        ++n_gdiPlusUsageCount;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
