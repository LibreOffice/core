/*************************************************************************
 *
 *  $RCSfile: bmcache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-09-24 13:38:09 $
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

#pragma hdrstop

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


