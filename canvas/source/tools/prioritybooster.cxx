/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prioritybooster.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:18:49 $
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


#ifdef WNT
#include <windows.h>
#endif

#include "osl/diagnose.h"
#include "canvas/prioritybooster.hxx"


namespace canvas
{
    namespace tools
    {
        struct PriorityBooster_Impl
        {
            int mnOldPriority;
        };

        PriorityBooster::PriorityBooster( sal_Int32 nDelta ) :
            mpImpl( new PriorityBooster_Impl )
        {
#ifdef WNT
            HANDLE aCurrThread = GetCurrentThread();
            mpImpl->mnOldPriority = GetThreadPriority( aCurrThread );

            const bool bSuccess( 0 != SetThreadPriority( aCurrThread, mpImpl->mnOldPriority + nDelta ) );

            OSL_ENSURE( bSuccess,
                        "PriorityBooster::PriorityBooster(): Was not able to modify thread priority" );
#else
            nDelta = 0; // #i55991# placate gcc warning
#endif
        }

        PriorityBooster::~PriorityBooster()
        {
#ifdef WNT
            SetThreadPriority( GetCurrentThread(),
                               mpImpl->mnOldPriority );
#endif
        }
    } // namespace tools

} // namespace canvas
