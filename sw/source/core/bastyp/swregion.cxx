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




#include <tools/debug.hxx>
#include "swtypes.hxx"
#include "swrect.hxx"
#include "swregion.hxx"


SV_IMPL_VARARR( SwRects, SwRect );

/*************************************************************************
|*
|*  SwRegionRects::SwRegionRects()
|*
|*  Ersterstellung      MA 28. Oct. 92
|*  Letzte Aenderung    MA 01. Feb. 93
|*
|*************************************************************************/

SwRegionRects::SwRegionRects( const SwRect &rStartRect, sal_uInt16 nInit,
                                                        sal_uInt16 nGrow ) :
    SwRects( (sal_uInt8)nInit, (sal_uInt8)nGrow ),
    aOrigin( rStartRect )
{
    Insert( aOrigin, 0 );
}

/*************************************************************************
 *                      inline InsertRect()
 *
 * InsertRect() wird nur von operator-=() gerufen.
 * Wenn bDel == sal_True ist, dann wird das Rect an der Position nPos mit
 * rRect ueberschrieben, ansonsten wird rRect hinten angehaengt.
 *************************************************************************/

inline void SwRegionRects::InsertRect( const SwRect &rRect, const sal_uInt16 nPos,
                                       sal_Bool &rDel )
{
    if( rDel )
    {
        pData = (SwRect*)pData; // looks weird but seems to help gcc ->i78417
        *(pData+nPos) = rRect;
        rDel = sal_False;
    }
    else
        Insert( rRect, Count() );
}

/*************************************************************************
|*
|*  SwRegionRects::operator-=()
|*
|*  Beschreibung        Alle Ueberschneidungen der Rechtecke, die sich
|*      gerade im Array befinden, mit dem uebergebenen Rechteck werden
|*      entfernt.
|*      Dazu muessen die vorhandenen Rechtecke entweder aufgeteilt oder
|*      geloescht werden.
|*  Ersterstellung      MA 28. Oct. 92
|*  Letzte Aenderung    MA 09. Sep. 93
|*
|*************************************************************************/

void SwRegionRects::operator-=( const SwRect &rRect )
{
    sal_uInt16 nMax = Count();
    for ( sal_uInt16 i = 0; i < nMax; ++i )
    {
        if ( rRect.IsOver( *(pData+i) ) )
        {
            SwRect aTmp( *(pData+i) );
            SwRect aInter( aTmp );
            aInter._Intersection( rRect );

            // Das erste Rect, das wir inserten wollen, nimmt die
            // Stelle von i ein. So ersparen wir uns das Delete().
            sal_Bool bDel = sal_True;

            //Jetzt aufteilen das Teil: Es sollen diejenigen Rechtecke
            //zurueckbleiben, die im alten aber nicht im neuen liegen.
            //Sprich alle Rechtecke die im alten aber nicht in der Intersection
            //liegen.
            long nTmp;
            if ( 0 < (nTmp = aInter.Top() - aTmp.Top()) )
            {
                const long nOldVal = aTmp.Height();
                aTmp.Height(nTmp);
                InsertRect( aTmp, i, bDel );
                aTmp.Height( nOldVal );
            }

            aTmp.Top( aInter.Top() + aInter.Height() );
            if ( aTmp.Height() > 0 )
                InsertRect( aTmp, i, bDel );

            aTmp.Top( aInter.Top() );
            aTmp.Bottom( aInter.Bottom() );
            if ( 0 < (nTmp = aInter.Left() - aTmp.Left()) )
            {
                const long nOldVal = aTmp.Width();
                aTmp.Width( nTmp );
                InsertRect( aTmp, i, bDel );
                aTmp.Width( nOldVal );
            }

            aTmp.Left( aInter.Left() + aInter.Width() ); //+1?
            if ( aTmp.Width() > 0 )
                InsertRect( aTmp, i, bDel );

            if( bDel )
            {
                Remove( i );
                --i;              //Damit wir keinen uebergehen.
                --nMax;           //Damit wir keinen zuviel verarbeiten.
            }
        }
    }

}

/*************************************************************************
 *                      SwRegionRects::Invert()
 *
 * Bezugspunkt ist aOrigin, das Original-SRectangle.
 * Aus Loechern werden Flaechen, aus Flaechen werden Loecher.
 * Ein Hinweis: Wenn keine Rects abgezogen wurden, so ist das enthaltene
 * Rechteck identisch mit aOrigin. Nach Invert() besteht die Region aus
 * einem Null-SRectangle.
 *************************************************************************/

void SwRegionRects::Invert()
{
    // Nicht besonders elegant und schnell, aber wirkungsvoll:
    // Wir legen eine weitere Region an und ziehen alle Flaechen ab,
    // die in uns noch uebrig geblieben sind. Danach werden alle
    // Werte uebertragen.

    // Um unuetze Speicheranforderungen zu vermeiden versuchen wir die
    // iniale Groesse moeglichst brauchbar anzulegen:
    // Anzahl der Rechtecke in der Region * 2 + 2
    // plus zwei um den Sonderfall eines einzelnen Loches (macht vier
    // Rechtecke im inversen Fall) abzudecken.

    SwRegionRects aInvRegion( aOrigin, Count()*2+2 );
    const SwRect *pDat = GetData();
    for( sal_uInt16 i = 0; i < Count(); ++pDat, ++i )
        aInvRegion -= *pDat;

    sal_uInt16 nCpy = Count(), nDel = 0;
    if( aInvRegion.Count() < Count() )
    {
        nDel = Count() - aInvRegion.Count();
        nCpy = aInvRegion.Count();
    }
    // alle vorhandenen ueberschreiben
    memcpy( pData, aInvRegion.GetData(), nCpy * sizeof( SwRect ));

    if( nCpy < aInvRegion.Count() )
        Insert( &aInvRegion, nCpy, nCpy );
    else if( nDel )
        Remove( nCpy, nDel );
}
/*************************************************************************
|*
|*  SwRegionRects::Compress()
|*
|*  Beschreibung        Zusammenfassen von benachbarten Rechtecken.
|*  Ersterstellung      MA 16. Apr. 93
|*  Letzte Aenderung    MA 21. Apr. 93
|*
|*************************************************************************/
inline SwTwips CalcArea( const SwRect &rRect )
{
    return rRect.Width() * rRect.Height();
}


void SwRegionRects::Compress( sal_Bool bFuzzy )
{
    for ( int i = 0; i < Count(); ++i )
    {
        for ( int j = i+1; j < Count(); ++j )
        {
            //Wenn zwei Rechtecke ineinanderliegen, so ist eins davon
            //uberfluessig.
            if ( (*(pData + i)).IsInside( *(pData + j) ) )
            {
                Remove( static_cast<sal_uInt16>(j), 1 );
                --j;
            }
            else if ( (*(pData + j)).IsInside( *(pData + i) ) )
            {
                *(pData + i) = *(pData + j);
                Remove( static_cast<sal_uInt16>(j), 1 );
                i = -1;
                break;
            }
            else
            {
                //Wenn zwei Rechtecke dieselbe Flaeche haben wie deren
                //Union abzueglich deren Intersection, so ist eines
                //davon ueberfluessig.
                //Um moeglichst viel zusammenzufassen und in der Folge
                //moeglichst wenig einzelne Paints zu haben darf die Flaeche
                //der Union ruhig ein bischen groesser sein
                //( 9622 * 141.5 = 1361513 ~= ein virtel Zentimeter ueber die
                //                            Breite einer DINA4 Seite)
                const long nFuzzy = bFuzzy ? 1361513 : 0;
                SwRect aUnion( *(pData + i) );aUnion.Union( *(pData + j) );
                SwRect aInter( *(pData + i) );aInter.Intersection( *(pData + j));
                if ( (::CalcArea( *(pData + i) ) +
                      ::CalcArea( *(pData + j) ) + nFuzzy) >=
                     (::CalcArea( aUnion ) - CalcArea( aInter )) )
                {
                    *(pData + i) = aUnion;
                    Remove( static_cast<sal_uInt16>(j), 1 );
                    i = -1;
                    break;
                }
            }
        }
    }
}

