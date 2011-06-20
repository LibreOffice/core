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
#include "precompiled_sw.hxx"



#include "swtypes.hxx"
#include <vcl/outdev.hxx>
#include <svtools/imaprect.hxx>
#include <svtools/imap.hxx>

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
    OSL_ENSURE( pMap, "FillImageMap: No ImageMap, no cookies!" );
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
            IMapRectangleObject aObj( aRect, rNote.GetURL(), aEmptyStr, aEmptyStr,
                                      rNote.GetTarget(), aEmptyStr, sal_True, sal_False );
            pMap->InsertIMapObject( aObj );
        }
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
