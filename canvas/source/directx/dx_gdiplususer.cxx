/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_gdiplususer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:55:12 $
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
