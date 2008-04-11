/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclevent.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_vcl.hxx"

#include <vcl/vclevent.hxx>

TYPEINIT0(VclSimpleEvent);
TYPEINIT1(VclWindowEvent, VclSimpleEvent);
TYPEINIT1(VclMenuEvent, VclSimpleEvent);

void VclEventListeners::Call( VclSimpleEvent* pEvent ) const
{
    // Copy the list, because this can be destroyed when calling a Link...
    std::list<Link> aCopy( *this );
    std::list<Link>::iterator aIter( aCopy.begin() );
    while ( aIter != aCopy.end() )
    {
        (*aIter).Call( pEvent );
        aIter++;
    }
}

BOOL VclEventListeners::Process( VclSimpleEvent* pEvent ) const
{
    BOOL bProcessed = FALSE;
    // Copy the list, because this can be destroyed when calling a Link...
    std::list<Link> aCopy( *this );
    std::list<Link>::iterator aIter( aCopy.begin() );
    while ( aIter != aCopy.end() )
    {
        if( (*aIter).Call( pEvent ) != 0 )
        {
            bProcessed = TRUE;
            break;
        }
        aIter++;
    }
    return bProcessed;
}
