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
#include "precompiled_sd.hxx"


#include <limits.h>     // LONG_MAX
#include <tools/solar.h>
#include <svtools/grfmgr.hxx>
#include "bmcache.hxx"

// eine Struktur fuer die Cache-Eintraege
struct BitmapCacheEntry
{
    const SdPage*   pPage;
    GraphicObject*  pGraphicObject;
    sal_uInt32      nSizeBytes;
    long            nZoom;
};

/*************************************************************************
|*
|* Destruktor, loescht die gespeicherten Bitmaps
|*
\************************************************************************/

BitmapCache::~BitmapCache()
{
    for( void* pEntry = aEntries.First();  pEntry; pEntry = aEntries.Next() )
    {
        delete static_cast< BitmapCacheEntry* >( pEntry )->pGraphicObject;
        delete static_cast< BitmapCacheEntry* >( pEntry );
    }
}

void BitmapCache::Remove( const SdPage* pPage )
{
    for( sal_uLong nPos = 0; nPos < aEntries.Count();  )
    {
        BitmapCacheEntry* pCand = (BitmapCacheEntry*) aEntries.GetObject( nPos );

        if( pCand->pPage == pPage )
        {
            pCand = (BitmapCacheEntry*) aEntries.Remove((sal_uLong)nPos);
            delete pCand->pGraphicObject;
            delete pCand;
        }
        else
            nPos++;
    }
}


