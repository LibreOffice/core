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


#ifdef WNT
# if defined _MSC_VER
# pragma warning(push,1)
# endif

# include <windows.h>

# if defined _MSC_VER
# pragma warning(pop)
# endif
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

            if ( 0 == SetThreadPriority( aCurrThread, mpImpl->mnOldPriority + nDelta ) )
            {
                OSL_FAIL( "PriorityBooster::PriorityBooster(): Was not able to modify thread priority" );
            }
#else
            (void)nDelta;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
