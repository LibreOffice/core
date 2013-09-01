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

#include "swtypes.hxx"
#include <vcl/outdev.hxx>
#include <svtools/imaprect.hxx>
#include <svtools/imap.hxx>

#include "txttypes.hxx"
#include "noteurl.hxx"

// Global variable
SwNoteURL *pNoteURL = NULL;


void SwNoteURL::InsertURLNote( const OUString& rURL, const OUString& rTarget,
    const SwRect& rRect )
{
    MSHORT i;
    MSHORT nCount = aList.size();
    for( i = 0; i < nCount; i++ )
        if( rRect == aList[i].GetRect() )
            break;
    if( i == nCount )
    {
        SwURLNote *pNew = new SwURLNote( rURL, rTarget, rRect );
        aList.push_back( pNew );
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
