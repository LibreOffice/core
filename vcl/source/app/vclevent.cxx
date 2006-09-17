/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclevent.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:51:14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vclevent.hxx>

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
