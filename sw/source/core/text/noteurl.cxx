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
            IMapRectangleObject aObj( aRect, rNote.GetURL(), aEmptyStr, aEmptyStr,
                                      rNote.GetTarget(), aEmptyStr, sal_True, sal_False );
            pMap->InsertIMapObject( aObj );
        }
    }
}




