/*************************************************************************
 *
 *  $RCSfile: bmcache.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:43 $
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

#include <limits.h>                         // LONG_MAX
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#include "bmcache.hxx"

// eine Struktur fuer die Cache-Eintraege
typedef struct
{
    const SdPage* pPage;
    Bitmap*       pBitmap;
    long          nZoom;
} BitmapCacheEntry;


/*************************************************************************
|*
|* Destruktor, loescht die gespeicherten Bitmaps
|*
\************************************************************************/

BitmapCache::~BitmapCache()
{
    BitmapCacheEntry* pEntry = NULL;
    while (aEntries.Count() > 0)
    {
        pEntry = (BitmapCacheEntry*)aEntries.Remove((ULONG)0);
        delete pEntry->pBitmap;
        delete pEntry;
    }
}

/*************************************************************************
|*
|* Cache-Eintrag einfuegen
|*
\************************************************************************/

void BitmapCache::Add(const SdPage* pPage, Bitmap* pBitmap, long nZoom)
{
    BitmapCacheEntry* pEntry = NULL;

    ULONG nSizeOfBitmap = pBitmap->GetSizeBytes();

    if (nSizeOfBitmap < nMaxSize)
    {
            while (nCurSize + nSizeOfBitmap > nMaxSize)
            {
                if( aEntries.Count() )
                {
                    pEntry = (BitmapCacheEntry*)aEntries.Remove(aEntries.Count() - 1);
                    if ( pEntry && pEntry->pBitmap != NULL)
                    {
                        nCurSize -= pEntry->pBitmap->GetSizeBytes();
                        delete pEntry->pBitmap;
                    }
                    delete pEntry;
                }
                else
                    break;
            }

        pEntry          = new BitmapCacheEntry;
        pEntry->pPage   = pPage;
        pEntry->pBitmap = pBitmap;
        pEntry->nZoom   = nZoom;

        aEntries.Insert(pEntry, (ULONG)0);
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

const Bitmap* BitmapCache::Get(const SdPage* pPage, long& rZoomPercent,
                               long nZoomTolerancePercent)
{
    BitmapCacheEntry* pEntry  = NULL;
    Bitmap*           pBitmap = NULL;

    // "best fit"-Suche
    if (nZoomTolerancePercent < 0)
    {
        long nTolerance = -nZoomTolerancePercent;
        BitmapCacheEntry* pBest = NULL;
        long              nBest = LONG_MAX;
        long              nTest = 0L;

        for (ULONG nPos = 0; nPos < aEntries.Count(); nPos++)
        {
            pEntry = (BitmapCacheEntry*)aEntries.GetObject(nPos);
            if (pEntry->pPage == pPage)
            {
                nTest = rZoomPercent - pEntry->nZoom;
                if (nTest >= 0 && nTest < nBest && nTest <= nTolerance)
                {
                    pBest = pEntry;
                }
            }
        }
        pEntry = pBest;
    }

    // "first fit"-suche
    else
    {
        for (ULONG nPos = 0; nPos < aEntries.Count(); nPos++)
        {
            pEntry = (BitmapCacheEntry*)aEntries.GetObject(nPos);
            if (pEntry->pPage == pPage &&
                Abs(pEntry->nZoom - rZoomPercent) <= nZoomTolerancePercent)
            {
                break;
            }
            else
            {
                pEntry = NULL;
            }
        }
    }

    // was passendes gefunden?
    if (pEntry)
    {
        pBitmap = pEntry->pBitmap;
        aEntries.Remove(pEntry);
        aEntries.Insert(pEntry, (ULONG)0);
        rZoomPercent = pEntry->nZoom;
    }
    return pBitmap;
}

void BitmapCache::Remove(const SdPage* pPage)
{
    for (ULONG nPos = 0; nPos < aEntries.Count(); )
    {
        BitmapCacheEntry* pCand = (BitmapCacheEntry*)aEntries.GetObject(nPos);

        if(pCand->pPage == pPage)
        {
            pCand = (BitmapCacheEntry*)aEntries.Remove((ULONG)nPos);
            delete pCand->pBitmap;
            delete pCand;
        }
        else
            nPos++;
    }
}


