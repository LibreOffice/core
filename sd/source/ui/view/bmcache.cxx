/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bmcache.cxx,v $
 * $Revision: 1.5 $
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
#include <goodies/grfmgr.hxx>
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

/*************************************************************************
|*
|* Cache-Eintrag einfuegen
|*
\************************************************************************/

void BitmapCache::Add(const SdPage* pPage, const Bitmap& rBmp, long nZoom)
{
    BitmapCacheEntry*   pEntry = NULL;
    ULONG               nSizeOfBitmap = rBmp.GetSizeBytes();

    if( nSizeOfBitmap < nMaxSize )
    {
        while (nCurSize + nSizeOfBitmap > nMaxSize)
        {
            if( aEntries.Count() )
            {
                pEntry = (BitmapCacheEntry*) aEntries.Remove(aEntries.Count() - 1);

                if( pEntry && pEntry->pGraphicObject )
                {
                    nCurSize -= pEntry->nSizeBytes;
                    delete pEntry->pGraphicObject;
                }

                delete pEntry;
            }
            else
                break;
        }

        pEntry = new BitmapCacheEntry;
        pEntry->pPage = pPage;
        pEntry->pGraphicObject = new GraphicObject( rBmp );
        pEntry->nSizeBytes = nSizeOfBitmap;
        pEntry->nZoom = nZoom;

        aEntries.Insert( pEntry, (ULONG) 0 );
        nCurSize += nSizeOfBitmap;
    }
}

/*************************************************************************
|*
|* Cache-Eintrag suchen, Bitmap-Zeiger zurueckgeben (NULL, wenn Bitmap nicht
|* im Cache)
|* - auf rZoomPercent wird der Zoomfaktor der Bitmap geschrieben
|* - abs(nZoomTolerance) gibt die maximal erlaubte Abweichung des
|*   Zoomfaktors an; ist die Toleranz negativ, so sucht der Cache den
|*   "best fit", sonst den "first fit",
|* - es werden nur Bitmaps mit Zoomfaktoren kleiner oder gleich rZoomPercent
|*   beruecksichtigt, um ein Verkleinern der Bitmap beim Zeichnen zu vermeiden
|*
\************************************************************************/

const GraphicObject* BitmapCache::Get( const SdPage* pPage, long& rZoomPercent, long nZoomTolerancePercent)
{
    BitmapCacheEntry* pEntry  = NULL;
    GraphicObject*    pGraphicObject = NULL;

    if( nZoomTolerancePercent < 0 )
    {
        // "best fit"-Suche
        long nTolerance = -nZoomTolerancePercent;
        BitmapCacheEntry* pBest = NULL;
        long              nBest = LONG_MAX;
        long              nTest = 0L;

        for( ULONG nPos = 0; nPos < aEntries.Count(); nPos++ )
        {
            pEntry = (BitmapCacheEntry*) aEntries.GetObject( nPos );

            if( pEntry->pPage == pPage )
            {
                nTest = rZoomPercent - pEntry->nZoom;

                if( nTest >= 0 && nTest < nBest && nTest <= nTolerance )
                    pBest = pEntry;
            }
        }

        pEntry = pBest;
    }
    else
    {
        // "first fit"-suche
        for( ULONG nPos = 0; nPos < aEntries.Count(); nPos++ )
        {
            pEntry = (BitmapCacheEntry*)aEntries.GetObject( nPos );

            if (pEntry->pPage == pPage && Abs( pEntry->nZoom - rZoomPercent ) <= nZoomTolerancePercent )
                break;
            else
                pEntry = NULL;
        }
    }

    // was passendes gefunden?
    if( pEntry )
    {
        pGraphicObject = pEntry->pGraphicObject;
        aEntries.Remove( pEntry );
        aEntries.Insert( pEntry, (ULONG) 0 );
        rZoomPercent = pEntry->nZoom;
    }

    return pGraphicObject;
}

void BitmapCache::Remove( const SdPage* pPage )
{
    for( ULONG nPos = 0; nPos < aEntries.Count();  )
    {
        BitmapCacheEntry* pCand = (BitmapCacheEntry*) aEntries.GetObject( nPos );

        if( pCand->pPage == pPage )
        {
            pCand = (BitmapCacheEntry*) aEntries.Remove((ULONG)nPos);
            delete pCand->pGraphicObject;
            delete pCand;
        }
        else
            nPos++;
    }
}


