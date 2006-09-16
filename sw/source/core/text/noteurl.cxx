/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: noteurl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 21:36:46 $
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
#include "precompiled_sw.hxx"



#include "swtypes.hxx"

#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _GOODIES_IMAPRECT_HXX
#include <svtools/imaprect.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif

#include "txttypes.hxx"
#include "noteurl.hxx"

// globale Variable, wird in noteurl.Hxx bekanntgegeben
SwNoteURL *pNoteURL = NULL;

SV_IMPL_PTRARR( SwURLNoteList, SwURLNotePtr )


void SwNoteURL::InsertURLNote( const XubString& rURL, const XubString& rTarget,
    const SwRect& rRect )
{
    MSHORT i;
    MSHORT nCount = aList.Count();
    for( i = 0; i < nCount; i++ )
        if( rRect == aList.GetObject(i)->GetRect() )
            break;
    if( i == nCount )
    {
        SwURLNote *pNew = new SwURLNote( rURL, rTarget, rRect );
        aList.Insert( pNew, nCount );
    }
}


void SwNoteURL::FillImageMap( ImageMap *pMap, const Point &rPos,
    const MapMode& rMap )
{
    ASSERT( pMap, "FillImageMap: No ImageMap, no cookies!" );
    MSHORT nCount = Count();
    if( nCount )
    {
        MapMode aMap( MAP_100TH_MM );
        for( MSHORT i = 0; i < nCount; ++i )
        {
            const SwURLNote &rNote = GetURLNote( i );
            SwRect aSwRect( rNote.GetRect() );
            aSwRect -= rPos;
            Rectangle aRect( OutputDevice::LogicToLogic( aSwRect.SVRect(),
                                                         rMap, aMap ) );
            IMapRectangleObject aObj( aRect, rNote.GetURL(), aEmptyStr,
                                      rNote.GetTarget(), sal_True, sal_False );
            pMap->InsertIMapObject( aObj );
        }
    }
}




