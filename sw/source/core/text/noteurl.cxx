/*************************************************************************
 *
 *  $RCSfile: noteurl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

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




