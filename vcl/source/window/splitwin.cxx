/*************************************************************************
 *
 *  $RCSfile: splitwin.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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

#define _SV_SPLITWIN_CXX

#include <string.h>

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RCID_H
#include <rcid.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_WALL_HXX
#include <wall.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_SYMBOL_HXX
#include <symbol.hxx>
#endif
#ifndef _SV_SVIDS_HRC
#include <svids.hrc>
#endif
#ifndef _SV_IMAGE_HXX
#include <image.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#define private public
#ifndef _SV_SPLITWIN_HXX
#include <splitwin.hxx>
#endif
#undef private

#pragma hdrstop

// =======================================================================

// Achtung: Darf keine Objekte enthalten, da mit memmove/memcpy gearbeitet wird
struct ImplSplitItem
{
    long                mnSize;
    long                mnPixSize;
    long                mnLeft;
    long                mnTop;
    long                mnWidth;
    long                mnHeight;
    long                mnSplitPos;
    long                mnSplitSize;
    long                mnOldSplitPos;
    long                mnOldSplitSize;
    long                mnOldWidth;
    long                mnOldHeight;
    ImplSplitSet*       mpSet;
    Window*             mpWindow;
    Window*             mpOrgParent;
    USHORT              mnId;
    SplitWindowItemBits mnBits;
    BOOL                mbFixed;
    BOOL                mbSubSize;
};

struct ImplSplitSet
{
    ImplSplitItem*      mpItems;
    Wallpaper*          mpWallpaper;
    Bitmap*             mpBitmap;
    long                mnLastSize;
    long                mnSplitSize;
    USHORT              mnItems;
    USHORT              mnId;
    BOOL                mbCalcPix;
};

#define SPLITWIN_SPLITSIZE              6
#define SPLITWIN_SPLITSIZEEX            6
#define SPLITWIN_SPLITSIZEAUTOHIDE      40
#define SPLITWIN_SPLITSIZEFADE          40

#define SPLIT_HORZ              ((USHORT)0x0001)
#define SPLIT_VERT              ((USHORT)0x0002)
#define SPLIT_WINDOW            ((USHORT)0x0004)
#define SPLIT_NOSPLIT           ((USHORT)0x8000)

// -----------------------------------------------------------------------

DECLARE_LIST( ImplSplitList, SplitWindow* );

// =======================================================================

static void ImplCalcBorder( WindowAlign eAlign, BOOL bNoAlign,
                            long& rLeft, long& rTop,
                            long& rRight, long& rBottom )
{
    if ( bNoAlign )
    {
        rLeft   = 2;
        rTop    = 2;
        rRight  = 2;
        rBottom = 2;
    }
    else
    {
        if ( eAlign == WINDOWALIGN_TOP )
        {
            rLeft   = 0;
            rTop    = 2;
            rRight  = 0;
            rBottom = 0;
        }
        else if ( eAlign == WINDOWALIGN_LEFT )
        {
            rLeft   = 2;
            rTop    = 2;
            rRight  = 0;
            rBottom = 2;
        }
        else if ( eAlign == WINDOWALIGN_BOTTOM )
        {
            rLeft   = 0;
            rTop    = 0;
            rRight  = 0;
            rBottom = 2;
        }
        else
        {
            rLeft   = 0;
            rTop    = 2;
            rRight  = 2;
            rBottom = 2;
        }
    }
}

// -----------------------------------------------------------------------

static void ImplDrawBorder( SplitWindow* pWin )
{
    const StyleSettings&    rStyleSettings = pWin->GetSettings().GetStyleSettings();
    long                    nDX = pWin->mnDX;
    long                    nDY = pWin->mnDY;

    if ( pWin->mbNoAlign )
    {
        DecorationView  aDecoView( pWin );
        Point           aTmpPoint;
        Rectangle       aRect( aTmpPoint, Size( nDX, nDY ) );
        aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
    }
    else
    {
        if ( pWin->meAlign == WINDOWALIGN_BOTTOM )
        {
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
        }
        else
        {
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( 0, 0 ), Point( nDX-1, 0 ) );
            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( 0, 1 ), Point( nDX-1, 1 ) );
            if ( (pWin->meAlign == WINDOWALIGN_LEFT) || (pWin->meAlign == WINDOWALIGN_RIGHT) )
            {
                if ( pWin->meAlign == WINDOWALIGN_LEFT )
                {
                    pWin->SetLineColor( rStyleSettings.GetShadowColor() );
                    pWin->DrawLine( Point( 0, 0 ), Point( 0, nDY-1 ) );
                    pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
                    pWin->SetLineColor( rStyleSettings.GetLightColor() );
                    pWin->DrawLine( Point( 1, 1 ), Point( 1, nDY-3 ) );
                    pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
                }
                else
                {
                    pWin->SetLineColor( rStyleSettings.GetShadowColor() );
                    pWin->DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-3 ) );
                    pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-2, nDY-2 ) );
                    pWin->SetLineColor( rStyleSettings.GetLightColor() );
                    pWin->DrawLine( Point( nDX-1, 0 ), Point( nDX-1, nDY-1 ) );
                    pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

static ImplSplitSet* ImplFindSet( ImplSplitSet* pSet, USHORT nId )
{
    if ( pSet->mnId == nId )
        return pSet;

    USHORT          i;
    USHORT          nItems = pSet->mnItems;
    ImplSplitItem*  pItems = pSet->mpItems;

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mnId == nId )
            return pItems[i].mpSet;
    }

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpSet )
        {
            ImplSplitSet* pFindSet = ImplFindSet( pItems[i].mpSet, nId );
            if ( pFindSet )
                return pFindSet;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------

static ImplSplitSet* ImplFindItem( ImplSplitSet* pSet, USHORT nId, USHORT& rPos )
{
    USHORT          i;
    USHORT          nItems = pSet->mnItems;
    ImplSplitItem*  pItems = pSet->mpItems;

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mnId == nId )
        {
            rPos = i;
            return pSet;
        }
    }

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpSet )
        {
            ImplSplitSet* pFindSet = ImplFindItem( pItems[i].mpSet, nId, rPos );
            if ( pFindSet )
                return pFindSet;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------

static USHORT ImplFindItem( ImplSplitSet* pSet, Window* pWindow )
{
    USHORT          i;
    USHORT          nItems = pSet->mnItems;
    ImplSplitItem*  pItems = pSet->mpItems;

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpWindow == pWindow )
            return pItems[i].mnId;
        else
        {
            if ( pItems[i].mpSet )
            {
                USHORT nId = ImplFindItem( pItems[i].mpSet, pWindow );
                if ( nId )
                    return nId;
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

static USHORT ImplFindItem( ImplSplitSet* pSet, const Point& rPos,
                            BOOL bRows, BOOL bDown = TRUE )
{
    USHORT          i;
    USHORT          nItems = pSet->mnItems;
    ImplSplitItem*  pItems = pSet->mpItems;

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mnWidth && pItems[i].mnHeight )
        {
            // Wegen ICC auftrennen
            Point       aPoint( pItems[i].mnLeft, pItems[i].mnTop );
            Size        aSize( pItems[i].mnWidth, pItems[i].mnHeight );
            Rectangle   aRect( aPoint, aSize );
            if ( bRows )
            {
                if ( bDown )
                    aRect.Bottom() += pSet->mnSplitSize;
                else
                    aRect.Top() -= pSet->mnSplitSize;
            }
            else
            {
                if ( bDown )
                    aRect.Right() += pSet->mnSplitSize;
                else
                    aRect.Left() -= pSet->mnSplitSize;
            }

            if ( aRect.IsInside( rPos ) )
            {
                if ( pItems[i].mpSet && pItems[i].mpSet->mpItems )
                {
                    return ImplFindItem( pItems[i].mpSet, rPos,
                                        ((pItems[i].mnBits & SWIB_COLSET) == 0) );
                }
                else
                    return pItems[i].mnId;
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

static void ImplDeleteSet( ImplSplitSet* pSet )
{
    USHORT          i;
    USHORT          nItems = pSet->mnItems;
    ImplSplitItem*  pItems = pSet->mpItems;

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpSet )
            ImplDeleteSet( pItems[i].mpSet );
    }

    if ( pSet->mpWallpaper )
        delete pSet->mpWallpaper;

    if ( pSet->mpBitmap )
        delete pSet->mpBitmap;

    delete pItems;
    delete pSet;
}

// -----------------------------------------------------------------------

static void ImplSetSplitSize( ImplSplitSet* pSet, long nNewSize )
{
    pSet->mnSplitSize = nNewSize;
    for ( USHORT i = 0; i < pSet->mnItems; i++ )
    {
        if ( pSet->mpItems[i].mpSet )
            ImplSetSplitSize( pSet->mpItems[i].mpSet, nNewSize );
    }
}

// -----------------------------------------------------------------------

static void ImplCalcSet( ImplSplitSet* pSet,
                         long nSetLeft, long nSetTop,
                         long nSetWidth, long nSetHeight,
                         BOOL bRows, BOOL bDown = TRUE )
{
    if ( !pSet->mpItems )
        return;

    USHORT              i;
    USHORT              j;
    USHORT              nMins;
    USHORT              nCalcItems;
    USHORT              nItems = pSet->mnItems;
    USHORT              nVisItems;
    USHORT              nAbsItems;
    long                nCalcSize;
    long                nSizeDelta;
    long                nCurSize;
    long                nSizeWinSize;
    long                nNewSizeWinSize;
    long                nTemp;
    long                nTempErr;
    long                nErrorSum;
    long                nCurSizeDelta;
    long                nPos;
    long                nMaxPos;
    long*               pSize;
    ImplSplitItem*      pItems = pSet->mpItems;
    BOOL                bEmpty;

    // Anzahl sichtbarer Items ermitteln
    nVisItems = 0;
    for ( i = 0; i < nItems; i++ )
    {
        if ( !(pItems[i].mnBits & SWIB_INVISIBLE) )
            nVisItems++;
    }

    // Groessen berechnen
    if ( bRows )
        nCalcSize = nSetHeight;
    else
        nCalcSize = nSetWidth;
    nCalcSize -= (nVisItems-1)*pSet->mnSplitSize;
    nCurSize   = 0;
    if ( pSet->mbCalcPix || (pSet->mnLastSize != nCalcSize) )
    {
        long nPercentFactor = 10;
        long nRelCount      = 0;
        long nPercent       = 0;
        long nRelPercent    = 0;
        long nAbsSize       = 0;
        for ( i = 0; i < nItems; i++ )
        {
            if ( !(pItems[i].mnBits & SWIB_INVISIBLE) )
            {
                if ( pItems[i].mnBits & SWIB_RELATIVESIZE )
                    nRelCount += pItems[i].mnSize;
                else if ( pItems[i].mnBits & SWIB_PERCENTSIZE )
                    nPercent += pItems[i].mnSize;
                else
                    nAbsSize += pItems[i].mnSize;
            }
        }
        // Relative-Werte auf prozentual mappen (Percent bei uns 10tel Prozent)
        nPercent *= nPercentFactor;
        if ( nRelCount )
        {
            long nRelPercentBase = 1000;
            while ( (nRelCount > nRelPercentBase) && (nPercentFactor < 100000) )
            {
                nRelPercentBase *= 10;
                nPercentFactor *= 10;
            }
            if ( nPercent < nRelPercentBase )
            {
                nRelPercent = (nRelPercentBase-nPercent)/nRelCount;
                nPercent += nRelPercent*nRelCount;
            }
            else
                nRelPercent = 0;
        }
        if ( !nPercent )
            nPercent = 1;
        nSizeDelta = nCalcSize-nAbsSize;
        for ( i = 0; i < nItems; i++ )
        {
            if ( pItems[i].mnBits & SWIB_INVISIBLE )
                pItems[i].mnPixSize = 0;
            else if ( pItems[i].mnBits & SWIB_RELATIVESIZE )
            {
                if ( nSizeDelta <= 0 )
                    pItems[i].mnPixSize = 0;
                else
                    pItems[i].mnPixSize = (nSizeDelta*pItems[i].mnSize*nRelPercent)/nPercent;
            }
            else if ( pItems[i].mnBits & SWIB_PERCENTSIZE )
            {
                if ( nSizeDelta <= 0 )
                    pItems[i].mnPixSize = 0;
                else
                    pItems[i].mnPixSize = (nSizeDelta*pItems[i].mnSize*nPercentFactor)/nPercent;
            }
            else
                pItems[i].mnPixSize = pItems[i].mnSize;
            nCurSize += pItems[i].mnPixSize;
        }

        pSet->mbCalcPix  = FALSE;
        pSet->mnLastSize = nCalcSize;

        // Fenster einpassen
        nSizeDelta  = nCalcSize-nCurSize;
        if ( nSizeDelta )
        {
            nAbsItems       = 0;
            nSizeWinSize    = 0;
            nNewSizeWinSize = 0;

            // Zuerst die absoluten Items relativ resizen
            for ( i = 0; i < nItems; i++ )
            {
                if ( !(pItems[i].mnBits & SWIB_INVISIBLE) )
                {
                    if ( !(pItems[i].mnBits & (SWIB_RELATIVESIZE | SWIB_PERCENTSIZE)) )
                    {
                        nAbsItems++;
                        nSizeWinSize += pItems[i].mnPixSize;
                    }
                }
            }
            // Rundungsfehler werden hier nicht ausgelichen
            if ( (nAbsItems < (USHORT)(Abs( nSizeDelta ))) && nSizeWinSize )
            {
                for ( i = 0; i < nItems; i++ )
                {
                    if ( !(pItems[i].mnBits & SWIB_INVISIBLE) )
                    {
                        if ( !(pItems[i].mnBits & (SWIB_RELATIVESIZE | SWIB_PERCENTSIZE)) )
                        {
                            pItems[i].mnPixSize += (nSizeDelta*pItems[i].mnPixSize)/nSizeWinSize;
                            nNewSizeWinSize += pItems[i].mnPixSize;
                        }
                    }
                }
                nSizeDelta -= nNewSizeWinSize-nSizeWinSize;
            }

            // Jetzt die Rundunsfehler ausgleichen
            j           = 0;
            nMins       = 0;
            while ( nSizeDelta && (nItems != nMins) )
            {
                // Feststellen, welche Items berechnet werden duerfen
                nCalcItems = 0;
                while ( !nCalcItems )
                {
                    for ( i = 0; i < nItems; i++ )
                    {
                        pItems[i].mbSubSize = FALSE;

                        if ( j >= 2 )
                            pItems[i].mbSubSize = TRUE;
                        else
                        {
                            if ( !(pItems[i].mnBits & SWIB_INVISIBLE) )
                            {
                                if ( (nSizeDelta > 0) || pItems[i].mnPixSize )
                                {
                                    if ( j >= 1 )
                                        pItems[i].mbSubSize = TRUE;
                                    else
                                    {
                                        if ( (j == 0) && (pItems[i].mnBits & (SWIB_RELATIVESIZE | SWIB_PERCENTSIZE)) )
                                            pItems[i].mbSubSize = TRUE;
                                    }
                                }
                            }
                        }

                        if ( pItems[i].mbSubSize )
                            nCalcItems++;
                    }

                    j++;
                }

                // Groessen von den einzelnen Items abziehen
                nErrorSum       = nSizeDelta % nCalcItems;
                nCurSizeDelta   = nSizeDelta / nCalcItems;
                nMins           = 0;
                for ( i = 0; i < nItems; i++ )
                {
                    if ( pItems[i].mnBits & SWIB_INVISIBLE )
                        nMins++;
                    else if ( pItems[i].mbSubSize )
                    {
                        pSize = &(pItems[i].mnPixSize);

                        if ( nErrorSum )
                        {
                            if ( nErrorSum < 0 )
                                nTempErr = -1;
                            else
                                nTempErr = 1;
                        }
                        else
                            nTempErr = 0;

                        if ( (*pSize+nCurSizeDelta+nTempErr) <= 0 )
                        {
                            nTemp = *pSize;
                            if ( nTemp )
                            {
                                *pSize -= nTemp;
                                nSizeDelta += nTemp;
                            }
                            nMins++;
                        }
                        else
                        {
                            *pSize += nCurSizeDelta;
                            nSizeDelta -= nCurSizeDelta;
                            if ( nTempErr && (*pSize || (nTempErr > 0)) )
                            {
                                *pSize += nTempErr;
                                nSizeDelta -= nTempErr;
                                nErrorSum -= nTempErr;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        for ( i = 0; i < nItems; i++ )
        {
            if ( !(pItems[i].mnBits & SWIB_INVISIBLE) )
                nCurSize += pItems[i].mnPixSize;
        }
    }

    // Maximale Groesse berechnen
    if ( bRows )
    {
        nPos = nSetTop;
        if ( !bDown )
            nMaxPos = nSetTop-nSetHeight;
        else
            nMaxPos = nSetTop+nSetHeight;
    }
    else
    {
        nPos = nSetLeft;
        if ( !bDown )
            nMaxPos = nSetLeft-nSetWidth;
        else
            nMaxPos = nSetLeft+nSetWidth;
    }

    // Fenster anordnen und Werte anpassen
    for ( i = 0; i < nItems; i++ )
    {
        pItems[i].mnOldSplitPos    = pItems[i].mnSplitPos;
        pItems[i].mnOldSplitSize   = pItems[i].mnSplitSize;
        pItems[i].mnOldWidth       = pItems[i].mnWidth;
        pItems[i].mnOldHeight      = pItems[i].mnHeight;

        if ( pItems[i].mnBits & SWIB_INVISIBLE )
            bEmpty = TRUE;
        else
        {
            bEmpty = FALSE;
            if ( bDown )
            {
                if ( nPos+pItems[i].mnPixSize > nMaxPos )
                    bEmpty = TRUE;
            }
            else
            {
                nPos -= pItems[i].mnPixSize;
                if ( nPos < nMaxPos )
                    bEmpty = TRUE;
            }
        }

        if ( bEmpty )
        {
            pItems[i].mnWidth     = 0;
            pItems[i].mnHeight    = 0;
            pItems[i].mnSplitSize = 0;
        }
        else
        {
            if ( bRows )
            {
                pItems[i].mnLeft   = nSetLeft;
                pItems[i].mnTop    = nPos;
                pItems[i].mnWidth  = nSetWidth;
                pItems[i].mnHeight = pItems[i].mnPixSize;
            }
            else
            {
                pItems[i].mnLeft   = nPos;
                pItems[i].mnTop    = nSetTop;
                pItems[i].mnWidth  = pItems[i].mnPixSize;
                pItems[i].mnHeight = nSetHeight;
            }

            if ( i > nItems-1 )
                pItems[i].mnSplitSize = 0;
            else
            {
                pItems[i].mnSplitSize = pSet->mnSplitSize;
                if ( bDown )
                {
                    pItems[i].mnSplitPos  = nPos+pItems[i].mnPixSize;
                    if ( pItems[i].mnSplitPos+pItems[i].mnSplitSize > nMaxPos )
                        pItems[i].mnSplitSize = nMaxPos-pItems[i].mnSplitPos;
                }
                else
                {
                    pItems[i].mnSplitPos = nPos-pSet->mnSplitSize;
                    if ( pItems[i].mnSplitPos < nMaxPos )
                        pItems[i].mnSplitSize = pItems[i].mnSplitPos+pSet->mnSplitSize-nMaxPos;
                }
            }
        }

        if ( !(pItems[i].mnBits & SWIB_INVISIBLE) )
        {
            if ( !bDown )
                nPos -= pSet->mnSplitSize;
            else
                nPos += pItems[i].mnPixSize+pSet->mnSplitSize;
        }
    }

    // Sub-Set's berechnen
    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpSet && pItems[i].mnWidth && pItems[i].mnHeight )
        {
            ImplCalcSet( pItems[i].mpSet,
                         pItems[i].mnLeft, pItems[i].mnTop,
                         pItems[i].mnWidth, pItems[i].mnHeight,
                         ((pItems[i].mnBits & SWIB_COLSET) == 0) );
        }
    }

    // Fixed setzen
    for ( i = 0; i < nItems; i++ )
    {
        pItems[i].mbFixed = FALSE;
        if ( pItems[i].mnBits & SWIB_FIXED )
            pItems[i].mbFixed = TRUE;
        else
        {
            // Wenn Child-Set vorhanden, ist dieses Item auch Fixed, wenn
            // ein Child fixed ist
            if ( pItems[i].mpSet )
            {
                for ( j = 0; j < pItems[i].mpSet->mnItems; j++ )
                {
                    if ( pItems[i].mpSet->mpItems[j].mbFixed )
                    {
                        pItems[i].mbFixed = TRUE;
                        break;
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

static void ImplCalcSet2( SplitWindow* pWindow, ImplSplitSet* pSet, BOOL bHide,
                          BOOL bRows, BOOL bDown = TRUE )
{
    USHORT          i;
    USHORT          nItems = pSet->mnItems;
    ImplSplitItem*  pItems = pSet->mpItems;

    if ( pWindow->IsReallyVisible() && pWindow->IsUpdateMode() && pWindow->mbInvalidate )
    {
        for ( i = 0; i < nItems; i++ )
        {
            if ( pItems[i].mnSplitSize )
            {
                // Evt. alles invalidieren oder nur einen kleinen Teil
                if ( (pItems[i].mnOldSplitPos  != pItems[i].mnSplitPos)  ||
                     (pItems[i].mnOldSplitSize != pItems[i].mnSplitSize) ||
                     (pItems[i].mnOldWidth     != pItems[i].mnWidth)     ||
                     (pItems[i].mnOldHeight    != pItems[i].mnHeight) )
                {
                    Rectangle aRect;

                    // Old Rect invalidieren
                    if ( bRows )
                    {
                        aRect.Left()    = pItems[i].mnLeft;
                        aRect.Right()   = pItems[i].mnLeft+pItems[i].mnOldWidth-1;
                        aRect.Top()     = pItems[i].mnOldSplitPos;
                        aRect.Bottom()  = aRect.Top() + pItems[i].mnOldSplitSize;
                    }
                    else
                    {
                        aRect.Top()     = pItems[i].mnTop;
                        aRect.Bottom()  = pItems[i].mnTop+pItems[i].mnOldHeight-1;
                        aRect.Left()    = pItems[i].mnOldSplitPos;
                        aRect.Right()   = aRect.Left() + pItems[i].mnOldSplitSize;
                    }
                    pWindow->Invalidate( aRect );
                    // New Rect invalidieren
                    if ( bRows )
                    {
                        aRect.Left()    = pItems[i].mnLeft;
                        aRect.Right()   = pItems[i].mnLeft+pItems[i].mnWidth-1;
                        aRect.Top()     = pItems[i].mnSplitPos;
                        aRect.Bottom()  = aRect.Top() + pItems[i].mnSplitSize;
                    }
                    else
                    {
                        aRect.Top()     = pItems[i].mnTop;
                        aRect.Bottom()  = pItems[i].mnTop+pItems[i].mnHeight-1;
                        aRect.Left()    = pItems[i].mnSplitPos;
                        aRect.Right()   = aRect.Left() + pItems[i].mnSplitSize;
                    }
                    pWindow->Invalidate( aRect );

                    // Leere Sets komplett invalidieren, da diese Flaechen
                    // nicht von Fenstern ueberladen werden
                    if ( pItems[i].mpSet && !pItems[i].mpSet->mpItems )
                    {
                        aRect.Left()    = pItems[i].mnLeft;
                        aRect.Top()     = pItems[i].mnTop;
                        aRect.Right()   = pItems[i].mnLeft+pItems[i].mnWidth-1;
                        aRect.Bottom()  = pItems[i].mnTop+pItems[i].mnHeight-1;
                        pWindow->Invalidate( aRect );
                    }
                }
            }
        }
    }

    // Fenster positionieren
    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpSet )
        {
            BOOL bTempHide = bHide;
            if ( !pItems[i].mnWidth || !pItems[i].mnHeight )
                bTempHide = TRUE;
            ImplCalcSet2( pWindow, pItems[i].mpSet, bTempHide,
                          ((pItems[i].mnBits & SWIB_COLSET) == 0) );
        }
        else
        {
            if ( pItems[i].mnWidth && pItems[i].mnHeight && !bHide )
            {
                Point aPos( pItems[i].mnLeft, pItems[i].mnTop );
                Size  aSize( pItems[i].mnWidth, pItems[i].mnHeight );
                pItems[i].mpWindow->SetPosSizePixel( aPos, aSize );
            }
            else
                pItems[i].mpWindow->Hide();
        }
    }

    // Fenster anzeigen und Flag zuruecksetzen
    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpWindow && pItems[i].mnWidth && pItems[i].mnHeight && !bHide )
            pItems[i].mpWindow->Show();
    }
}

// -----------------------------------------------------------------------

static void ImplCalcLogSize( ImplSplitItem* pItems, USHORT nItems )
{
    // Original-Groessen updaten
    USHORT  i;
    long    nRelSize = 0;
    long    nPerSize = 0;
    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mnBits & SWIB_RELATIVESIZE )
            nRelSize += pItems[i].mnPixSize;
        else if ( pItems[i].mnBits & SWIB_PERCENTSIZE )
            nPerSize += pItems[i].mnPixSize;
    }
    nPerSize += nRelSize;
    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mnBits & SWIB_RELATIVESIZE )
        {
            if ( nRelSize )
                pItems[i].mnSize = (pItems[i].mnPixSize+(nRelSize/2))/nRelSize;
            else
                pItems[i].mnSize = 1;
        }
        else if ( pItems[i].mnBits & SWIB_PERCENTSIZE )
        {
            if ( nPerSize )
                pItems[i].mnSize = (pItems[i].mnPixSize*100)/nPerSize;
            else
                pItems[i].mnSize = 1;
        }
        else
            pItems[i].mnSize = pItems[i].mnPixSize;
    }
}

// -----------------------------------------------------------------------

static void ImplDrawBack( SplitWindow* pWindow, const Rectangle& rRect,
                          const Wallpaper* pWall, const Bitmap* pBitmap )
{
    if ( pBitmap )
    {
        Point   aPos = rRect.TopLeft();
        Size    aBmpSize = pBitmap->GetSizePixel();
        pWindow->Push( PUSH_CLIPREGION );
        pWindow->IntersectClipRegion( rRect );
        do
        {
            aPos.X() = rRect.Left();
            do
            {
                pWindow->DrawBitmap( aPos, *pBitmap );
                aPos.X() += aBmpSize.Width();
            }
            while ( aPos.X() < rRect.Right() );
            aPos.Y() += aBmpSize.Height();
        }
        while ( aPos.Y() < rRect.Bottom() );
        pWindow->Pop();
    }
    else
        pWindow->DrawWallpaper( rRect, *pWall );
}

// -----------------------------------------------------------------------

static void ImplDrawBack( SplitWindow* pWindow, ImplSplitSet* pSet )
{
    USHORT          i;
    USHORT          nItems = pSet->mnItems;
    ImplSplitItem*  pItems = pSet->mpItems;

    // Beim Mainset auch den Hintergrund zeichnen
    if ( pSet->mnId == 0 )
    {
        if ( pSet->mpBitmap )
        {
            Rectangle aRect( pWindow->mnLeftBorder,
                             pWindow->mnTopBorder,
                             pWindow->mnDX-pWindow->mnRightBorder-1,
                             pWindow->mnDY-pWindow->mnBottomBorder-1 );
            ImplDrawBack( pWindow, aRect, pSet->mpWallpaper, pSet->mpBitmap );
        }
    }

    for ( i = 0; i < nItems; i++ )
    {
        pSet = pItems[i].mpSet;
        if ( pSet )
        {
            if ( pSet->mpBitmap || pSet->mpWallpaper )
            {
                // Wegen ICC auftrennen
                Point       aPoint( pItems[i].mnLeft, pItems[i].mnTop );
                Size        aSize( pItems[i].mnWidth, pItems[i].mnHeight );
                Rectangle   aRect( aPoint, aSize );
                ImplDrawBack( pWindow, aRect, pSet->mpWallpaper, pSet->mpBitmap );
            }
        }
    }

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpSet )
            ImplDrawBack( pWindow, pItems[i].mpSet );
    }
}

// -----------------------------------------------------------------------

static void ImplDrawSplit( SplitWindow* pWindow, ImplSplitSet* pSet,
                           BOOL bRows, BOOL bDown = TRUE )
{
    if ( !pSet->mpItems )
        return;

    USHORT                  i;
    USHORT                  nItems = pSet->mnItems;
    long                    nPos;
    long                    nTop;
    long                    nBottom;
    ImplSplitItem*          pItems = pSet->mpItems;
    const StyleSettings&    rStyleSettings = pWindow->GetSettings().GetStyleSettings();

    BOOL bFlat = (pWindow->GetStyle() & WB_FLATSPLITDRAW) == WB_FLATSPLITDRAW;

    for ( i = 0; i < nItems-1; i++ )
    {
        if ( pItems[i].mnSplitSize )
        {
            nPos = pItems[i].mnSplitPos;

            long nItemSplitSize = pItems[i].mnSplitSize;
            long nSplitSize = pSet->mnSplitSize;
            if ( bRows )
            {
                nTop    = pItems[i].mnLeft;
                nBottom = pItems[i].mnLeft+pItems[i].mnWidth-1;

                if ( bFlat ) nPos--;

                if ( bDown || (nItemSplitSize >= nSplitSize) )
                {
                    pWindow->SetLineColor( rStyleSettings.GetLightColor() );
                    pWindow->DrawLine( Point( nTop, nPos+1 ), Point( nBottom, nPos+1 ) );
                }
                nPos += nSplitSize-2;
                if ( bFlat ) nPos+=2;
                if ( (!bDown && (nItemSplitSize >= 2)) ||
                     (bDown  && (nItemSplitSize >= nSplitSize-1)) )
                {
                    pWindow->SetLineColor( rStyleSettings.GetShadowColor() );
                    pWindow->DrawLine( Point( nTop, nPos ), Point( nBottom, nPos ) );
                }
                if ( !bFlat )
                {
                    nPos++;
                    if ( !bDown || (nItemSplitSize >= nSplitSize) )
                    {
                        pWindow->SetLineColor( rStyleSettings.GetDarkShadowColor() );
                        pWindow->DrawLine( Point( nTop, nPos ), Point( nBottom, nPos ) );
                    }
                }
            }
            else
            {
                nTop    = pItems[i].mnTop;
                nBottom = pItems[i].mnTop+pSet->mpItems[i].mnHeight-1;

                if ( bFlat ) nPos--;
                if ( bDown || (nItemSplitSize >= nSplitSize) )
                {
                    pWindow->SetLineColor( rStyleSettings.GetLightColor() );
                    pWindow->DrawLine( Point( nPos+1, nTop ), Point( nPos+1, nBottom ) );
                }
                nPos += pSet->mnSplitSize-2;
                if ( bFlat ) nPos+=2;
                if ( (!bDown && (nItemSplitSize >= 2)) ||
                     (bDown  && (nItemSplitSize >= nSplitSize-1)) )
                {
                    pWindow->SetLineColor( rStyleSettings.GetShadowColor() );
                    pWindow->DrawLine( Point( nPos, nTop ), Point( nPos, nBottom ) );
                }
                if( !bFlat )
                {
                    nPos++;
                    if ( !bDown || (nItemSplitSize >= nSplitSize) )
                    {
                        pWindow->SetLineColor( rStyleSettings.GetDarkShadowColor() );
                        pWindow->DrawLine( Point( nPos, nTop ), Point( nPos, nBottom ) );
                    }
                }
            }
        }
    }

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpSet && pItems[i].mnWidth && pItems[i].mnHeight )
            ImplDrawSplit( pWindow, pItems[i].mpSet, ((pItems[i].mnBits & SWIB_COLSET) == 0) );
    }
}

// -----------------------------------------------------------------------

static USHORT ImplTestSplit( ImplSplitSet* pSet, const Point& rPos,
                             long& rMouseOff, ImplSplitSet** ppFoundSet, USHORT& rFoundPos,
                             BOOL bRows, BOOL bDown = TRUE )
{
    if ( !pSet->mpItems )
        return 0;

    USHORT          i;
    USHORT          nSplitTest;
    USHORT          nItems = pSet->mnItems;
    long            nMPos1;
    long            nMPos2;
    long            nPos;
    long            nTop;
    long            nBottom;
    ImplSplitItem*   pItems = pSet->mpItems;

    if ( bRows )
    {
        nMPos1 = rPos.X();
        nMPos2 = rPos.Y();
    }
    else
    {
        nMPos1 = rPos.Y();
        nMPos2 = rPos.X();
    }

    for ( i = 0; i < nItems-1; i++ )
    {
        if ( pItems[i].mnSplitSize )
        {
            if ( bRows )
            {
                nTop    = pItems[i].mnLeft;
                nBottom = pItems[i].mnLeft+pItems[i].mnWidth-1;
            }
            else
            {
                nTop    = pItems[i].mnTop;
                nBottom = pItems[i].mnTop+pItems[i].mnHeight-1;
            }
            nPos = pItems[i].mnSplitPos;

            if ( (nMPos1 >= nTop) && (nMPos1 <= nBottom) &&
                 (nMPos2 >= nPos) && (nMPos2 <= nPos+pItems[i].mnSplitSize) )
            {
                if ( !pItems[i].mbFixed && !pItems[i+1].mbFixed )
                {
                    rMouseOff = nMPos2-nPos;
                    *ppFoundSet = pSet;
                    rFoundPos = i;
                    if ( bRows )
                        return SPLIT_VERT;
                    else
                        return SPLIT_HORZ;
                }
                else
                    return SPLIT_NOSPLIT;
            }
        }
    }

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpSet )
        {
            nSplitTest = ImplTestSplit( pItems[i].mpSet, rPos,
                                       rMouseOff, ppFoundSet, rFoundPos,
                                       ((pItems[i].mnBits & SWIB_COLSET) == 0) );
            if ( nSplitTest )
                return nSplitTest;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

static USHORT ImplTestSplit( SplitWindow* pWindow, const Point& rPos,
                             long& rMouseOff, ImplSplitSet** ppFoundSet, USHORT& rFoundPos )
{
    // Resizeable SplitWindow muss anders behandelt werden
    if ( pWindow->mnWinStyle & WB_SIZEABLE )
    {
        long    nTPos;
        long    nPos;
        long    nBorder;

        if ( pWindow->mbHorz )
        {
            if ( pWindow->mbBottomRight )
            {
                nBorder = pWindow->mnBottomBorder;
                nPos = 0;
            }
            else
            {
                nBorder = pWindow->mnTopBorder;
                nPos = pWindow->mnDY-nBorder;
            }
            nTPos = rPos.Y();
        }
        else
        {
            if ( pWindow->mbBottomRight )
            {
                nBorder = pWindow->mnRightBorder;
                nPos = 0;
            }
            else
            {
                nBorder = pWindow->mnLeftBorder;
                nPos = pWindow->mnDX-nBorder;
            }
            nTPos = rPos.X();
        }
        long nSplitSize = pWindow->mpMainSet->mnSplitSize-2;
        if ( pWindow->mbAutoHide || pWindow->mbFadeOut )
            nSplitSize += SPLITWIN_SPLITSIZEEX;
        if ( !pWindow->mbBottomRight )
            nPos -= nSplitSize;
        if ( (nTPos >= nPos) && (nTPos <= nPos+nSplitSize+nBorder) )
        {
            rMouseOff = nTPos-nPos;
            *ppFoundSet = pWindow->mpMainSet;
            if ( pWindow->mpMainSet->mpItems )
                rFoundPos = pWindow->mpMainSet->mnItems-1;
            else
                rFoundPos = 0;
            if ( pWindow->mbHorz )
                return SPLIT_VERT | SPLIT_WINDOW;
            else
                return SPLIT_HORZ | SPLIT_WINDOW;
        }
    }

    return ImplTestSplit( pWindow->mpMainSet, rPos, rMouseOff, ppFoundSet, rFoundPos,
                         pWindow->mbHorz, !pWindow->mbBottomRight );
}

// -----------------------------------------------------------------------

static void ImplDrawSplitTracking( SplitWindow* pThis, const Point& rPos )
{
    Rectangle aRect;

    if ( pThis->mnSplitTest & SPLIT_HORZ )
    {
        aRect.Top()    = pThis->maDragRect.Top();
        aRect.Bottom() = pThis->maDragRect.Bottom();
        aRect.Left()   = rPos.X();
        aRect.Right()  = aRect.Left()+pThis->mpSplitSet->mnSplitSize-1;
        if ( !(pThis->mnWinStyle & WB_NOSPLITDRAW) )
            aRect.Right()--;
        if ( (pThis->mnSplitTest & SPLIT_WINDOW) &&
             (pThis->mbAutoHide || pThis->mbFadeOut) )
        {
            aRect.Left()  += SPLITWIN_SPLITSIZEEX;
            aRect.Right() += SPLITWIN_SPLITSIZEEX;
        }
    }
    else
    {
        aRect.Left()    = pThis->maDragRect.Left();
        aRect.Right()   = pThis->maDragRect.Right();
        aRect.Top()     = rPos.Y();
        aRect.Bottom()  = aRect.Top()+pThis->mpSplitSet->mnSplitSize-1;
        if ( !(pThis->mnWinStyle & WB_NOSPLITDRAW) )
            aRect.Bottom()--;
        if ( (pThis->mnSplitTest & SPLIT_WINDOW) &&
             (pThis->mbAutoHide || pThis->mbFadeOut) )
        {
            aRect.Top()    += SPLITWIN_SPLITSIZEEX;
            aRect.Bottom() += SPLITWIN_SPLITSIZEEX;
        }
    }
    pThis->ShowTracking( aRect, SHOWTRACK_SPLIT );
}

// -----------------------------------------------------------------------

void SplitWindow::ImplInit( Window* pParent, WinBits nStyle )
{
    ImplSplitSet* pNewSet   = new ImplSplitSet;
    pNewSet->mpItems        = NULL;
    pNewSet->mpWallpaper    = NULL;
    pNewSet->mpBitmap       = NULL;
    pNewSet->mnLastSize     = 0;
    pNewSet->mnItems        = 0;
    pNewSet->mnId           = 0;
    pNewSet->mnSplitSize    = SPLITWIN_SPLITSIZE;
    pNewSet->mbCalcPix      = TRUE;

    mpMainSet               = pNewSet;
    mpBaseSet               = pNewSet;
    mpSplitSet              = NULL;
    mpLastSizes             = NULL;
    mnDX                    = 0;
    mnDY                    = 0;
    mnLeftBorder            = 0;
    mnTopBorder             = 0;
    mnRightBorder           = 0;
    mnBottomBorder          = 0;
    mnMaxSize               = 0;
    mnMouseOff              = 0;
    meAlign                 = WINDOWALIGN_TOP;
    mnWinStyle              = nStyle;
    mnSplitTest             = 0;
    mnSplitPos              = 0;
    mnMouseModifier         = 0;
    mnMStartPos             = 0;
    mnMSplitPos             = 0;
    mbDragFull              = FALSE;
    mbHorz                  = TRUE;
    mbBottomRight           = FALSE;
    mbCalc                  = FALSE;
    mbRecalc                = TRUE;
    mbInvalidate            = TRUE;
    mbAutoHide              = FALSE;
    mbFadeIn                = FALSE;
    mbFadeOut               = FALSE;
    mbAutoHideIn            = FALSE;
    mbAutoHideDown          = FALSE;
    mbFadeInDown            = FALSE;
    mbFadeOutDown           = FALSE;
    mbAutoHidePressed       = FALSE;
    mbFadeInPressed         = FALSE;
    mbFadeOutPressed        = FALSE;
    mbFadeNoButtonMode      = FALSE;
    mbNoAlign               = FALSE;

    if ( nStyle & WB_NOSPLITDRAW )
    {
        pNewSet->mnSplitSize -= 2;
        mbInvalidate = FALSE;
    }

    if ( nStyle & WB_BORDER )
    {
        ImplCalcBorder( meAlign, mbNoAlign, mnLeftBorder, mnTopBorder,
                        mnRightBorder, mnBottomBorder );
    }
    else
    {
        mnLeftBorder   = 0;
        mnTopBorder    = 0;
        mnRightBorder  = 0;
        mnBottomBorder = 0;
    }

    DockingWindow::ImplInit( pParent, (nStyle | WB_CLIPCHILDREN) & ~(WB_BORDER | WB_SIZEABLE) );

    ImplInitSettings();
}

// -----------------------------------------------------------------------

void SplitWindow::ImplInitSettings()
{
    // Wenn fuer das MainSet eine Bitmap gesetzt wird, dann
    // brauchen wir nicht mehr den Hintergrund loeschen
    // Wenn MainSet Wallpaper hat, dann ist das der Hintergrund, ansonsten
    // sind es die Standard-Farben
    if ( mpMainSet->mpBitmap )
        SetBackground();
    else if ( mpMainSet->mpWallpaper )
        SetBackground( *mpMainSet->mpWallpaper );
    else
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else if ( Window::GetStyle() & WB_3DLOOK )
            aColor = rStyleSettings.GetFaceColor();
        else
            aColor = rStyleSettings.GetWindowColor();
        SetBackground( aColor );
    }
}

// =======================================================================

SplitWindow::SplitWindow( Window* pParent, WinBits nStyle ) :
    DockingWindow( WINDOW_SPLITWINDOW )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

SplitWindow::SplitWindow( Window* pParent, const ResId& rResId ) :
    DockingWindow( WINDOW_SPLITWINDOW )
{
    rResId.SetRT( RSC_SPLITWINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

SplitWindow::~SplitWindow()
{
    // Sets loeschen
    ImplDeleteSet( mpMainSet );
}

// -----------------------------------------------------------------------

void SplitWindow::ImplSetWindowSize( long nDelta )
{
    if ( !nDelta )
        return;

    Size aSize = GetSizePixel();
    if ( meAlign == WINDOWALIGN_TOP )
    {
        aSize.Height() += nDelta;
        SetSizePixel( aSize );
    }
    else if ( meAlign == WINDOWALIGN_BOTTOM )
    {
        Point aPos = GetPosPixel();
        aPos.Y() -= nDelta;
        aSize.Height() += nDelta;
        SetPosSizePixel( aPos, aSize );
    }
    else if ( meAlign == WINDOWALIGN_LEFT )
    {
        aSize.Width() += nDelta;
        SetSizePixel( aSize );
    }
    else // meAlign == WINDOWALIGN_RIGHT
    {
        Point aPos = GetPosPixel();
        aPos.X() -= nDelta;
        aSize.Width() += nDelta;
        SetPosSizePixel( aPos, aSize );
    }

    SplitResize();
}

// -----------------------------------------------------------------------

void SplitWindow::ImplCalcLayout()
{
    if ( !mbCalc || !mbRecalc || !mpMainSet->mpItems )
        return;

    long nSplitSize = mpMainSet->mnSplitSize-2;
    if ( mbAutoHide || mbFadeOut )
        nSplitSize += SPLITWIN_SPLITSIZEEX;

    // Wenn Fenster sizeable ist, wird die groesse automatisch nach
    // dem MainSet festgelegt, wenn kein relatives Fenster enthalten
    // ist
    if ( mnWinStyle & WB_SIZEABLE )
    {
        long    nCurSize;
        long    nCalcSize = 0;
        USHORT  i;

        for ( i = 0; i < mpMainSet->mnItems; i++ )
        {
            if ( mpMainSet->mpItems[i].mnBits & (SWIB_RELATIVESIZE | SWIB_PERCENTSIZE) )
                break;
            else
                nCalcSize += mpMainSet->mpItems[i].mnSize;
        }

        if ( i == mpMainSet->mnItems )
        {
            if ( mbHorz )
                nCurSize = mnDY-mnTopBorder-mnBottomBorder;
            else
                nCurSize = mnDX-mnLeftBorder-mnRightBorder;
            nCurSize -= nSplitSize;
            nCurSize -= (mpMainSet->mnItems-1)*mpMainSet->mnSplitSize;

            mbRecalc = FALSE;
            ImplSetWindowSize( nCalcSize-nCurSize );
            mbRecalc = TRUE;
        }
    }

    if ( (mnDX <= 0) || (mnDY <= 0) )
        return;

    // Groessen/Position vorberechnen
    long    nL;
    long    nT;
    long    nW;
    long    nH;

    if ( mbHorz )
    {
        if ( mbBottomRight )
            nT = mnDY-mnBottomBorder;
        else
            nT = mnTopBorder;
        nL = mnLeftBorder;
    }
    else
    {
        if ( mbBottomRight )
            nL = mnDX-mnRightBorder;
        else
            nL = mnLeftBorder;
        nT = mnTopBorder;
    }
    nW = mnDX-mnLeftBorder-mnRightBorder;
    nH = mnDY-mnTopBorder-mnBottomBorder;
    if ( mnWinStyle & WB_SIZEABLE )
    {
        if ( mbHorz )
            nH -= nSplitSize;
        else
            nW -= nSplitSize;
    }

    // Sets rekursiv berechnen
    ImplCalcSet( mpMainSet, nL, nT, nW, nH, mbHorz, !mbBottomRight );
    ImplCalcSet2( this, mpMainSet, FALSE, mbHorz, !mbBottomRight );
    mbCalc = FALSE;
}

// -----------------------------------------------------------------------

void SplitWindow::ImplUpdate()
{
    mbCalc = TRUE;

    if ( IsReallyShown() && IsUpdateMode() && mbRecalc )
    {
        if ( mpMainSet->mpItems )
            ImplCalcLayout();
        else
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplUpdateSet( ImplSplitSet* pSet )
{
    if ( IsReallyShown() && IsUpdateMode() && mbRecalc )
    {
        // Wenn wir noch berechnen muessen, dann alles invalidieren.
        if ( mbCalc )
        {
            // Wenn nicht NOSPLITDRAW gesetzt ist, koennen wir uns das
            // invalidieren sparen, da bei ImplCalcSet2() die freien flaechen
            // sowieso invalidiert werden
            if ( !mpMainSet->mpItems || (mnWinStyle & WB_NOSPLITDRAW) )
                pSet = mpMainSet;
            else
                return;
        }

        Rectangle aRect;
        if ( pSet == mpMainSet )
        {
            aRect.Left()    = mnLeftBorder;
            aRect.Top()     = mnTopBorder;
            aRect.Right()   = mnDX-mnRightBorder-1;
            aRect.Bottom()  = mnDY-mnBottomBorder-1;
        }
        else
        {
            ImplSplitItem*  pItem;
            USHORT          nPos;

            pSet = ImplFindItem( mpMainSet, pSet->mnId, nPos );
            pItem = &(pSet->mpItems[nPos]);
            aRect.Left()    = pItem->mnLeft;
            aRect.Top()     = pItem->mnTop;
            aRect.Right()   = aRect.Left()+pItem->mnWidth;
            aRect.Bottom()  = aRect.Top()+pItem->mnHeight;
        }
        Invalidate( aRect );
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplSplitMousePos( Point& rMousePos )
{
    if ( mnSplitTest & SPLIT_HORZ )
    {
        rMousePos.X() -= mnMouseOff;
        if ( rMousePos.X() < maDragRect.Left() )
            rMousePos.X() = maDragRect.Left();
        else if ( rMousePos.X()+mpSplitSet->mnSplitSize+1 > maDragRect.Right() )
            rMousePos.X() = maDragRect.Right()-mpSplitSet->mnSplitSize+1;
        // Wegen FullDrag in Screen-Koordinaaten merken
        mnMSplitPos = OutputToScreenPixel( rMousePos ).X();
    }
    else
    {
        rMousePos.Y() -= mnMouseOff;
        if ( rMousePos.Y() < maDragRect.Top() )
            rMousePos.Y() = maDragRect.Top();
        else if ( rMousePos.Y()+mpSplitSet->mnSplitSize+1 > maDragRect.Bottom() )
            rMousePos.Y() = maDragRect.Bottom()-mpSplitSet->mnSplitSize+1;
        mnMSplitPos = OutputToScreenPixel( rMousePos ).Y();
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplGetButtonRect( Rectangle& rRect, long nEx, BOOL bTest ) const
{
    long nSplitSize = mpMainSet->mnSplitSize-2;
    if ( mbAutoHide || mbFadeOut || mbFadeIn )
        nSplitSize += SPLITWIN_SPLITSIZEEX;

/* Wir wollen doch erstmal nicht zentrieren
    long nButtonSize = 0;
    if ( mbFadeIn )
        nButtonSize += SPLITWIN_SPLITSIZEFADE+1;
    if ( mbFadeOut )
        nButtonSize += SPLITWIN_SPLITSIZEFADE+1;
    if ( mbAutoHide )
        nButtonSize += SPLITWIN_SPLITSIZEAUTOHIDE+1;
    long nCenterEx = 0;
    if ( mbHorz )
        nCenterEx += ((mnDX-mnLeftBorder-mnRightBorder)-nButtonSize)/2;
    else
        nCenterEx += ((mnDY-mnTopBorder-mnBottomBorder)-nButtonSize)/2;
    if ( nCenterEx > 0 )
        nEx += nCenterEx;
*/

    if ( meAlign == WINDOWALIGN_TOP )
    {
        rRect.Left()    = mnLeftBorder+nEx;
        rRect.Top()     = mnDY-mnBottomBorder-nSplitSize;
        rRect.Right()   = rRect.Left()+SPLITWIN_SPLITSIZEAUTOHIDE;
        rRect.Bottom()  = mnDY-mnBottomBorder-1;
        if ( bTest )
        {
            rRect.Top()     -= mnTopBorder;
            rRect.Bottom()  += mnBottomBorder;
        }
    }
    else if ( meAlign == WINDOWALIGN_BOTTOM )
    {
        rRect.Left()    = mnLeftBorder+nEx;
        rRect.Top()     = mnTopBorder;
        rRect.Right()   = rRect.Left()+SPLITWIN_SPLITSIZEAUTOHIDE;
        rRect.Bottom()  = mnTopBorder+nSplitSize-1;
        if ( bTest )
        {
            rRect.Top()     -= mnTopBorder;
            rRect.Bottom()  += mnBottomBorder;
        }
    }
    else if ( meAlign == WINDOWALIGN_LEFT )
    {
        rRect.Left()    = mnDX-mnRightBorder-nSplitSize;
        rRect.Top()     = mnTopBorder+nEx;
        rRect.Right()   = mnDX-mnRightBorder-1;
        rRect.Bottom()  = rRect.Top()+SPLITWIN_SPLITSIZEAUTOHIDE;
        if ( bTest )
        {
            rRect.Left()    -= mnLeftBorder;
            rRect.Right()   += mnRightBorder;
        }
    }
    else if ( meAlign == WINDOWALIGN_RIGHT )
    {
        rRect.Left()    = mnLeftBorder;
        rRect.Top()     = mnTopBorder+nEx;
        rRect.Right()   = mnLeftBorder+nSplitSize-1;
        rRect.Bottom()  = rRect.Top()+SPLITWIN_SPLITSIZEAUTOHIDE;
        if ( bTest )
        {
            rRect.Left()    -= mnLeftBorder;
            rRect.Right()   += mnRightBorder;
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplGetAutoHideRect( Rectangle& rRect, BOOL bTest ) const
{
    Rectangle aRect;

    if ( mbAutoHide )
    {
        long nEx = 0;
        if ( mbFadeIn || mbFadeOut )
            nEx = SPLITWIN_SPLITSIZEFADE+1;
        ImplGetButtonRect( aRect, nEx, bTest && mbFadeIn );
    }

    rRect = aRect;
}

// -----------------------------------------------------------------------

void SplitWindow::ImplGetFadeInRect( Rectangle& rRect, BOOL bTest ) const
{
    Rectangle aRect;

    if ( mbFadeIn )
        ImplGetButtonRect( aRect, 0, bTest );

    rRect = aRect;
}

// -----------------------------------------------------------------------

void SplitWindow::ImplGetFadeOutRect( Rectangle& rRect, BOOL ) const
{
    Rectangle aRect;

    if ( mbFadeOut )
        ImplGetButtonRect( aRect, 0, FALSE );

    rRect = aRect;
}

// -----------------------------------------------------------------------

void SplitWindow::ImplDrawButtonRect( const Rectangle& rRect, long nSize )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( mbHorz )
    {
        long nLeft = rRect.Left();
        long nRight = rRect.Right();
        long nCenter = rRect.Center().Y();
        long nEx1 = nLeft+((rRect.GetWidth()-nSize)/2)-2;
        long nEx2 = nEx1+nSize+3;
        SetLineColor( rStyleSettings.GetLightColor() );
        DrawLine( Point( rRect.Left(), rRect.Top() ), Point( rRect.Left(), rRect.Bottom() ) );
        DrawLine( Point( rRect.Left(), rRect.Top() ), Point( rRect.Right(), rRect.Top() ) );
        SetLineColor( rStyleSettings.GetShadowColor() );
        DrawLine( Point( rRect.Right(), rRect.Top() ), Point( rRect.Right(), rRect.Bottom() ) );
        DrawLine( Point( rRect.Left(), rRect.Bottom() ), Point( rRect.Right(), rRect.Bottom() ) );
        long i = nLeft+2;
        while ( i < nRight-3 )
        {
            if ( (i < nEx1) || (i > nEx2 ) )
            {
                DrawPixel( Point( i, nCenter-2 ), rStyleSettings.GetLightColor() );
                DrawPixel( Point( i+1, nCenter-2+1 ), rStyleSettings.GetShadowColor() );
            }
            i++;
            if ( (i < nEx1) || (i > nEx2 ) && (i < nRight-3) )
            {
                DrawPixel( Point( i, nCenter+2 ), rStyleSettings.GetLightColor() );
                DrawPixel( Point( i+1, nCenter+2+1 ), rStyleSettings.GetShadowColor() );
            }
            i += 2;
        }
    }
    else
    {
        long nTop = rRect.Top();
        long nBottom = rRect.Bottom();
        long nCenter = rRect.Center().X();
        long nEx1 = nTop+((rRect.GetHeight()-nSize)/2)-2;
        long nEx2 = nEx1+nSize+3;
        SetLineColor( rStyleSettings.GetLightColor() );
        DrawLine( Point( rRect.Left(), rRect.Top() ), Point( rRect.Right(), rRect.Top() ) );
        DrawLine( Point( rRect.Left(), rRect.Top() ), Point( rRect.Left(), rRect.Bottom() ) );
        SetLineColor( rStyleSettings.GetShadowColor() );
        DrawLine( Point( rRect.Right(), rRect.Top() ), Point( rRect.Right(), rRect.Bottom() ) );
        DrawLine( Point( rRect.Left(), rRect.Bottom() ), Point( rRect.Right(), rRect.Bottom() ) );
        long i = nTop+2;
        while ( i < nBottom-3 )
        {
            if ( (i < nEx1) || (i > nEx2 ) )
            {
                DrawPixel( Point( nCenter-2, i ), rStyleSettings.GetLightColor() );
                DrawPixel( Point( nCenter-2+1, i+1 ), rStyleSettings.GetShadowColor() );
            }
            i++;
            if ( (i < nEx1) || (i > nEx2 ) && (i < nBottom-3) )
            {
                DrawPixel( Point( nCenter+2, i ), rStyleSettings.GetLightColor() );
                DrawPixel( Point( nCenter+2+1, i+1 ), rStyleSettings.GetShadowColor() );
            }
            i += 2;
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplDrawAutoHide( BOOL bInPaint )
{
    if ( mbAutoHide )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Rectangle aTempRect;
        ImplGetAutoHideRect( aTempRect );

        if ( !bInPaint )
            Erase( aTempRect );

        // ImageListe laden, wenn noch nicht vorhanden
        ImplSVData* pSVData = ImplGetSVData();
        ImageList*  pImageList;
        if ( mbHorz )
        {
            if ( !pSVData->maCtrlData.mpSplitHPinImgList )
            {
                Bitmap aBmp( ResId( SV_RESID_BITMAP_SPLITHPIN, ImplGetResMgr() ) );
                pSVData->maCtrlData.mpSplitHPinImgList = new ImageList( aBmp, Color( 0x00, 0x00, 0xFF ), 4 );
            }
            pImageList = pSVData->maCtrlData.mpSplitHPinImgList;
        }
        else
        {
            if ( !pSVData->maCtrlData.mpSplitVPinImgList )
            {
                Bitmap aBmp( ResId( SV_RESID_BITMAP_SPLITVPIN, ImplGetResMgr() ) );
                pSVData->maCtrlData.mpSplitVPinImgList = new ImageList( aBmp, Color( 0x00, 0x00, 0xFF ), 4 );
            }
            pImageList = pSVData->maCtrlData.mpSplitVPinImgList;
        }

        // Image ermitteln und zurueckgeben
        USHORT nId;
        if ( mbAutoHidePressed )
        {
            if ( mbAutoHideIn )
                nId = 3;
            else
                nId = 4;
        }
        else
        {
            if ( mbAutoHideIn )
                nId = 1;
            else
                nId = 2;
        }

        Image   aImage = pImageList->GetImage( nId );
        Size    aImageSize = aImage.GetSizePixel();
        Point   aPos( aTempRect.Left()+((aTempRect.GetWidth()-aImageSize.Width())/2),
                      aTempRect.Top()+((aTempRect.GetHeight()-aImageSize.Height())/2) );
        long    nSize;
        if ( mbHorz )
            nSize = aImageSize.Width();
        else
            nSize = aImageSize.Height();
        ImplDrawButtonRect( aTempRect, nSize );
        DrawImage( aPos, aImage );
    }
}

// -----------------------------------------------------------------------

static void ImplGetSplitArrowImage( BOOL bHorz, BOOL bLeft, BOOL bPressed,
                                    Image& rImage )
{
    // ImageListe laden, wenn noch nicht vorhanden
    ImplSVData* pSVData = ImplGetSVData();
    ImageList*  pImageList;
    if ( bHorz )
    {
        if ( !pSVData->maCtrlData.mpSplitHArwImgList )
        {
            Bitmap aBmp( ResId( SV_RESID_BITMAP_SPLITHARW, ImplGetResMgr() ) );
            pSVData->maCtrlData.mpSplitHArwImgList = new ImageList( aBmp, Color( 0x00, 0x00, 0xFF ), 4 );
        }
        pImageList = pSVData->maCtrlData.mpSplitHArwImgList;
    }
    else
    {
        if ( !pSVData->maCtrlData.mpSplitVArwImgList )
        {
            Bitmap aBmp( ResId( SV_RESID_BITMAP_SPLITVARW, ImplGetResMgr() ) );
            pSVData->maCtrlData.mpSplitVArwImgList = new ImageList( aBmp, Color( 0x00, 0x00, 0xFF ), 4 );
        }
        pImageList = pSVData->maCtrlData.mpSplitVArwImgList;
    }

    // Image ermitteln und zurueckgeben
    USHORT nId = 1;
    if ( !bLeft )
        nId += 2;
    if ( bPressed )
        nId++;

    rImage = pImageList->GetImage( nId );
}

// -----------------------------------------------------------------------

void SplitWindow::ImplDrawFadeIn( BOOL bInPaint )
{
    if ( mbFadeIn )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Rectangle       aTempRect;
        Image           aImage;
        ImplGetFadeInRect( aTempRect );

        BOOL bLeft;
        if ( meAlign == WINDOWALIGN_TOP )
            bLeft   = FALSE;
        else if ( meAlign == WINDOWALIGN_BOTTOM )
            bLeft   = TRUE;
        else if ( meAlign == WINDOWALIGN_LEFT )
            bLeft   = FALSE;
        else if ( meAlign == WINDOWALIGN_RIGHT )
            bLeft   = TRUE;
        ImplGetSplitArrowImage( mbHorz, bLeft, mbFadeInPressed, aImage );

        if ( !bInPaint )
            Erase( aTempRect );

        Size    aImageSize = aImage.GetSizePixel();
        Point   aPos( aTempRect.Left()+((aTempRect.GetWidth()-aImageSize.Width())/2),
                      aTempRect.Top()+((aTempRect.GetHeight()-aImageSize.Height())/2) );
        long    nSize;
        if ( mbHorz )
            nSize = aImageSize.Width();
        else
            nSize = aImageSize.Height();
        ImplDrawButtonRect( aTempRect, nSize );
        DrawImage( aPos, aImage );
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplDrawFadeOut( BOOL bInPaint )
{
    if ( mbFadeOut )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Rectangle       aTempRect;
        Image           aImage;
        ImplGetFadeOutRect( aTempRect );

        BOOL bLeft;
        if ( meAlign == WINDOWALIGN_TOP )
            bLeft   = TRUE;
        else if ( meAlign == WINDOWALIGN_BOTTOM )
            bLeft   = FALSE;
        else if ( meAlign == WINDOWALIGN_LEFT )
            bLeft   = TRUE;
        else if ( meAlign == WINDOWALIGN_RIGHT )
            bLeft   = FALSE;
        ImplGetSplitArrowImage( mbHorz, bLeft, mbFadeOutPressed, aImage );

        if ( !bInPaint )
            Erase( aTempRect );

        Size    aImageSize = aImage.GetSizePixel();
        Point   aPos( aTempRect.Left()+((aTempRect.GetWidth()-aImageSize.Width())/2),
                      aTempRect.Top()+((aTempRect.GetHeight()-aImageSize.Height())/2) );
        long    nSize;
        if ( mbHorz )
            nSize = aImageSize.Width();
        else
            nSize = aImageSize.Height();
        ImplDrawButtonRect( aTempRect, nSize );
        DrawImage( aPos, aImage );
    }
}

// -----------------------------------------------------------------------

void SplitWindow::StartSplit()
{
    maStartSplitHdl.Call( this );
}

// -----------------------------------------------------------------------

void SplitWindow::Split()
{
    maSplitHdl.Call( this );
}

// -----------------------------------------------------------------------

void SplitWindow::SplitResize()
{
    maSplitResizeHdl.Call( this );
}

// -----------------------------------------------------------------------

void SplitWindow::AutoHide()
{
    maAutoHideHdl.Call( this );
}

// -----------------------------------------------------------------------

void SplitWindow::FadeIn()
{
    maFadeInHdl.Call( this );
}

// -----------------------------------------------------------------------

void SplitWindow::FadeOut()
{
    maFadeOutHdl.Call( this );
}

// -----------------------------------------------------------------------

void SplitWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() || rMEvt.IsMod2() )
    {
        DockingWindow::MouseButtonDown( rMEvt );
        return;
    }

    Point           aMousePosPixel = rMEvt.GetPosPixel();
    Rectangle       aTestRect;

    mbFadeNoButtonMode = FALSE;
    ImplGetAutoHideRect( aTestRect, TRUE );
    if ( aTestRect.IsInside( aMousePosPixel ) )
    {
        mbAutoHideDown = TRUE;
        mbAutoHidePressed = TRUE;
        ImplDrawAutoHide( FALSE );
    }
    else
    {
        ImplGetFadeOutRect( aTestRect, TRUE );
        if ( aTestRect.IsInside( aMousePosPixel ) )
        {
            mbFadeOutDown = TRUE;
            mbFadeOutPressed = TRUE;
            ImplDrawFadeOut( FALSE );
        }
        else
        {
            ImplGetFadeInRect( aTestRect, TRUE );
            if ( aTestRect.IsInside( aMousePosPixel ) )
            {
                mbFadeInDown = TRUE;
                mbFadeInPressed = TRUE;
                ImplDrawFadeIn( FALSE );
            }
            else if ( !aTestRect.IsEmpty() && !(mnWinStyle & WB_SIZEABLE) )
            {
                mbFadeNoButtonMode = TRUE;
                FadeIn();
                return;
            }
        }
    }

    if ( mbAutoHideDown || mbFadeInDown || mbFadeOutDown )
        StartTracking();
    else
    {
        mnSplitTest = ImplTestSplit( this, aMousePosPixel, mnMouseOff, &mpSplitSet, mnSplitPos );
        if ( mnSplitTest && !(mnSplitTest & SPLIT_NOSPLIT) )
        {
            ImplSplitItem*  pSplitItem;
            long            nCurMaxSize;
            USHORT          nTemp;
            BOOL            bDown;
            BOOL            bPropSmaller;

            mnMouseModifier = rMEvt.GetModifier();
            if ( !(mnMouseModifier & KEY_SHIFT) || (mnSplitPos+1 >= mpSplitSet->mnItems) )
                bPropSmaller = FALSE;
            else
                bPropSmaller = TRUE;

            // Hier kann noch die maximale Groesse gesetzt werden
            StartSplit();

            if ( mnMaxSize )
                nCurMaxSize = mnMaxSize;
            else
            {
                Size aSize = GetParent()->GetOutputSizePixel();
                if ( mbHorz )
                    nCurMaxSize = aSize.Height();
                else
                    nCurMaxSize = aSize.Width();
            }

            if ( mpSplitSet->mpItems )
            {
                bDown = TRUE;
                if ( (mpSplitSet == mpMainSet) && mbBottomRight )
                    bDown = FALSE;

                pSplitItem          = &(mpSplitSet->mpItems[mnSplitPos]);
                maDragRect.Left()   = pSplitItem->mnLeft;
                maDragRect.Top()    = pSplitItem->mnTop;
                maDragRect.Right()  = pSplitItem->mnLeft+pSplitItem->mnWidth-1;
                maDragRect.Bottom() = pSplitItem->mnTop+pSplitItem->mnHeight-1;

                if ( mnSplitTest & SPLIT_HORZ )
                {
                    if ( bDown )
                        maDragRect.Right() += mpSplitSet->mnSplitSize;
                    else
                        maDragRect.Left() -= mpSplitSet->mnSplitSize;
                }
                else
                {
                    if ( bDown )
                        maDragRect.Bottom() += mpSplitSet->mnSplitSize;
                    else
                        maDragRect.Top() -= mpSplitSet->mnSplitSize;
                }

                if ( mnSplitPos )
                {
                    nTemp = mnSplitPos;
                    while ( nTemp )
                    {
                        pSplitItem = &(mpSplitSet->mpItems[nTemp-1]);
                        if ( pSplitItem->mbFixed )
                            break;
                        else
                        {
                            if ( mnSplitTest & SPLIT_HORZ )
                            {
                                if ( bDown )
                                    maDragRect.Left() -= pSplitItem->mnPixSize;
                                else
                                    maDragRect.Right() += pSplitItem->mnPixSize;
                            }
                            else
                            {
                                if ( bDown )
                                    maDragRect.Top() -= pSplitItem->mnPixSize;
                                else
                                    maDragRect.Bottom() += pSplitItem->mnPixSize;
                            }
                        }
                        nTemp--;
                    }
                }

                if ( (mpSplitSet == mpMainSet) && (mnWinStyle & WB_SIZEABLE) && !bPropSmaller )
                {
                    if ( bDown )
                    {
                        if ( mbHorz )
                            maDragRect.Bottom() += nCurMaxSize-mnDY-mnTopBorder;
                        else
                            maDragRect.Right() += nCurMaxSize-mnDX-mnLeftBorder;
                    }
                    else
                    {
                        if ( mbHorz )
                            maDragRect.Top() -= nCurMaxSize-mnDY-mnBottomBorder;
                        else
                            maDragRect.Left() -= nCurMaxSize-mnDX-mnRightBorder;
                    }
                }
                else
                {
                    nTemp = mnSplitPos+1;
                    while ( nTemp < mpSplitSet->mnItems )
                    {
                        pSplitItem = &(mpSplitSet->mpItems[nTemp]);
                        if ( pSplitItem->mbFixed )
                            break;
                        else
                        {
                            if ( mnSplitTest & SPLIT_HORZ )
                            {
                                if ( bDown )
                                    maDragRect.Right() += pSplitItem->mnPixSize;
                                else
                                    maDragRect.Left() -= pSplitItem->mnPixSize;
                            }
                            else
                            {
                                if ( bDown )
                                    maDragRect.Bottom() += pSplitItem->mnPixSize;
                                else
                                    maDragRect.Top() -= pSplitItem->mnPixSize;
                            }
                        }
                        nTemp++;
                    }
                }
            }
            else
            {
                maDragRect.Left()   = mnLeftBorder;
                maDragRect.Top()    = mnTopBorder;
                maDragRect.Right()  = mnDX-mnRightBorder-1;
                maDragRect.Bottom() = mnDY-mnBottomBorder-1;
                if ( mbHorz )
                {
                    if ( mbBottomRight )
                        maDragRect.Top() -= nCurMaxSize-mnDY-mnBottomBorder;
                    else
                        maDragRect.Bottom() += nCurMaxSize-mnDY-mnTopBorder;
                }
                else
                {
                    if ( mbBottomRight )
                        maDragRect.Left() -= nCurMaxSize-mnDX-mnRightBorder;
                    else
                        maDragRect.Right() += nCurMaxSize-mnDX-mnLeftBorder;
                }
            }

            StartTracking();

            mbDragFull = (GetSettings().GetStyleSettings().GetDragFullOptions() & DRAGFULL_OPTION_SPLIT) != 0;
            ImplSplitMousePos( aMousePosPixel );
            if ( !mbDragFull )
                ImplDrawSplitTracking( this, aMousePosPixel );
            else
            {
                ImplSplitItem*  pItems = mpSplitSet->mpItems;
                USHORT          nItems = mpSplitSet->mnItems;
                mpLastSizes = new long[nItems*2];
                for ( USHORT i = 0; i < nItems; i++ )
                {
                    mpLastSizes[i*2]   = pItems[i].mnSize;
                    mpLastSizes[i*2+1] = pItems[i].mnPixSize;
                }
            }
            mnMStartPos = mnMSplitPos;
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( !IsTracking() )
    {
        Point           aPos = rMEvt.GetPosPixel();
        long            nTemp;
        ImplSplitSet*   pTempSplitSet;
        USHORT          nTempSplitPos;
        USHORT          nSplitTest = ImplTestSplit( this, aPos, nTemp, &pTempSplitSet, nTempSplitPos );
        PointerStyle    eStyle = POINTER_ARROW;
        Rectangle       aAutoHideRect;
        Rectangle       aFadeInRect;
        Rectangle       aFadeOutRect;

        ImplGetAutoHideRect( aAutoHideRect );
        ImplGetFadeInRect( aFadeInRect );
        ImplGetFadeOutRect( aFadeOutRect );
        if ( !aAutoHideRect.IsInside( aPos ) &&
             !aFadeInRect.IsInside( aPos ) &&
             !aFadeOutRect.IsInside( aPos ) )
        {
            if ( nSplitTest && !(nSplitTest & SPLIT_NOSPLIT) )
            {
                if ( nSplitTest & SPLIT_HORZ )
                    eStyle = POINTER_HSPLIT;
                else if ( nSplitTest & SPLIT_VERT )
                    eStyle = POINTER_VSPLIT;
            }
        }

        Pointer aPtr( eStyle );
        SetPointer( aPtr );
    }
}

// -----------------------------------------------------------------------

void SplitWindow::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePosPixel = rTEvt.GetMouseEvent().GetPosPixel();

    if ( mbAutoHideDown )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbAutoHideDown = FALSE;
            if ( mbAutoHidePressed )
            {
                mbAutoHidePressed = FALSE;

                if ( !rTEvt.IsTrackingCanceled() )
                {
                    mbAutoHideIn = !mbAutoHideIn;
                    ImplDrawAutoHide( FALSE );
                    AutoHide();
                }
                else
                    ImplDrawAutoHide( FALSE );
            }
        }
        else
        {
            Rectangle aTestRect;
            ImplGetAutoHideRect( aTestRect, TRUE );
            BOOL bNewPressed = aTestRect.IsInside( aMousePosPixel );
            if ( bNewPressed != mbAutoHidePressed )
            {
                mbAutoHidePressed = bNewPressed;
                ImplDrawAutoHide( FALSE );
            }
        }
    }
    else if ( mbFadeInDown )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbFadeInDown = FALSE;
            if ( mbFadeInPressed )
            {
                mbFadeInPressed = FALSE;
                ImplDrawFadeIn( FALSE );

                if ( !rTEvt.IsTrackingCanceled() )
                    FadeIn();
            }
        }
        else
        {
            Rectangle aTestRect;
            ImplGetFadeInRect( aTestRect, TRUE );
            BOOL bNewPressed = aTestRect.IsInside( aMousePosPixel );
            if ( bNewPressed != mbFadeInPressed )
            {
                mbFadeInPressed = bNewPressed;
                ImplDrawFadeIn( FALSE );
            }
        }
    }
    else if ( mbFadeOutDown )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbFadeOutDown = FALSE;
            if ( mbFadeOutPressed )
            {
                mbFadeOutPressed = FALSE;
                ImplDrawFadeOut( FALSE );

                if ( !rTEvt.IsTrackingCanceled() )
                    FadeOut();
            }
        }
        else
        {
            Rectangle aTestRect;
            ImplGetFadeOutRect( aTestRect, TRUE );
            BOOL bNewPressed = aTestRect.IsInside( aMousePosPixel );
            if ( bNewPressed != mbFadeOutPressed )
            {
                mbFadeOutPressed = bNewPressed;
                ImplDrawFadeOut( FALSE );
            }
        }
    }
    else
    {
        ImplSplitMousePos( aMousePosPixel );
        BOOL bSplit = TRUE;
        if ( mbDragFull )
        {
            if ( rTEvt.IsTrackingEnded() )
            {
                if ( rTEvt.IsTrackingCanceled() )
                {
                    ImplSplitItem*  pItems = mpSplitSet->mpItems;
                    USHORT          nItems = mpSplitSet->mnItems;
                    for ( USHORT i = 0; i < nItems; i++ )
                    {
                        pItems[i].mnSize     = mpLastSizes[i*2];
                        pItems[i].mnPixSize  = mpLastSizes[i*2+1];
                    }
                    ImplUpdate();
                    Split();
                }
                bSplit = FALSE;
            }
        }
        else
        {
            if ( rTEvt.IsTrackingEnded() )
            {
                HideTracking();
                bSplit = !rTEvt.IsTrackingCanceled();
            }
            else
            {
                ImplDrawSplitTracking( this, aMousePosPixel );
                bSplit = FALSE;
            }
        }

        if ( bSplit )
        {
            BOOL    bPropSmaller = (mnMouseModifier & KEY_SHIFT) ? TRUE : FALSE;
            BOOL    bPropGreater = (mnMouseModifier & KEY_MOD1) ? TRUE : FALSE;
            long    nDelta = mnMSplitPos-mnMStartPos;

            if ( (mnSplitTest & SPLIT_WINDOW) && !mpMainSet->mpItems )
            {
                if ( (mpSplitSet == mpMainSet) && mbBottomRight )
                    nDelta *= -1;
                ImplSetWindowSize( nDelta );
            }
            else
            {
                long nNewSize = mpSplitSet->mpItems[mnSplitPos].mnPixSize;
                if ( (mpSplitSet == mpMainSet) && mbBottomRight )
                    nNewSize -= nDelta;
                else
                    nNewSize += nDelta;
                SplitItem( mpSplitSet->mpItems[mnSplitPos].mnId, nNewSize,
                           bPropSmaller, bPropGreater );
            }

            Split();

            if ( mbDragFull )
            {
                Update();
                mnMStartPos = mnMSplitPos;
            }
        }

        if ( rTEvt.IsTrackingEnded() )
        {
            if ( mpLastSizes )
                delete mpLastSizes;
            mpLastSizes     = NULL;
            mpSplitSet      = NULL;
            mnMouseOff      = 0;
            mnMStartPos     = 0;
            mnMSplitPos     = 0;
            mnMouseModifier = 0;
            mnSplitTest     = 0;
            mnSplitPos      = 0;
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::Paint( const Rectangle& )
{
    if ( mnWinStyle & WB_BORDER )
        ImplDrawBorder( this );

    ImplDrawFadeOut( TRUE );
    ImplDrawFadeIn( TRUE );
    ImplDrawAutoHide( TRUE );

    // FrameSet-Hintergruende zeichnen
    ImplDrawBack( this, mpMainSet );

    // Splitter zeichnen
    if ( !(mnWinStyle & WB_NOSPLITDRAW) )
        ImplDrawSplit( this, mpMainSet, mbHorz, !mbBottomRight );
}

// -----------------------------------------------------------------------

void SplitWindow::Move()
{
    DockingWindow::Move();
}

// -----------------------------------------------------------------------

void SplitWindow::Resize()
{
    Size aSize = GetOutputSizePixel();
    mnDX = aSize.Width();
    mnDY = aSize.Height();

    ImplUpdate();
    Invalidate();
}

// -----------------------------------------------------------------------

void SplitWindow::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & (HELPMODE_BALLOON | HELPMODE_QUICK) )
    {
        Point       aMousePosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        Rectangle   aHelpRect;
        USHORT      nHelpResId = 0;

        ImplGetAutoHideRect( aHelpRect, TRUE );
        if ( aHelpRect.IsInside( aMousePosPixel ) )
        {
            if ( mbAutoHideIn )
                nHelpResId = SV_HELPTEXT_SPLITFIXED;
            else
                nHelpResId = SV_HELPTEXT_SPLITFLOATING;
        }
        else
        {
            ImplGetFadeInRect( aHelpRect, TRUE );
            if ( aHelpRect.IsInside( aMousePosPixel ) )
                nHelpResId = SV_HELPTEXT_FADEIN;
            else
            {
                ImplGetFadeOutRect( aHelpRect, TRUE );
                if ( aHelpRect.IsInside( aMousePosPixel ) )
                    nHelpResId = SV_HELPTEXT_FADEOUT;
            }
        }

        // Rechteck ermitteln
        if ( nHelpResId )
        {
            Point aPt = OutputToScreenPixel( aHelpRect.TopLeft() );
            aHelpRect.Left()   = aPt.X();
            aHelpRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aHelpRect.BottomRight() );
            aHelpRect.Right()  = aPt.X();
            aHelpRect.Bottom() = aPt.Y();

            // Text ermitteln und anzeigen
            XubString aStr( ResId( nHelpResId, ImplGetResMgr() ) );
            if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                Help::ShowBalloon( this, aHelpRect.Center(), aHelpRect, aStr );
            else
                Help::ShowQuickHelp( this, aHelpRect, aStr );
            return;
        }
    }

    DockingWindow::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void SplitWindow::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( IsUpdateMode() )
            ImplCalcLayout();
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsUpdateMode() && IsReallyShown() )
            ImplCalcLayout();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }

    DockingWindow::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SplitWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
    else
        DockingWindow::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void SplitWindow::InsertItem( USHORT nId, Window* pWindow, long nSize,
                              USHORT nPos, USHORT nSetId,
                              SplitWindowItemBits nBits )
{
#ifdef DBG_UTIL
    USHORT nDbgDummy;
    DBG_ASSERT( ImplFindSet( mpMainSet, nSetId ), "SplitWindow::InsertItem() - Set not exists" );
    DBG_ASSERT( !ImplFindItem( mpMainSet, nId, nDbgDummy ), "SplitWindow::InsertItem() - Id already exists" );
#endif

    // Size muss min. 1 sein
    if ( nSize < 1 )
        nSize = 1;

    ImplSplitSet* pSet       = ImplFindSet( mpMainSet, nSetId );
    ImplSplitSet* pNewSet;
    ImplSplitItem* pItem;

    // Platz fuer neues Item schaffen
    if ( nPos > pSet->mnItems )
        nPos = pSet->mnItems;
    ImplSplitItem* pNewItems = new ImplSplitItem[pSet->mnItems+1];
    if ( nPos )
        memcpy( pNewItems, pSet->mpItems, sizeof( ImplSplitItem )*nPos );
    if ( nPos < pSet->mnItems )
        memcpy( pNewItems+nPos+1, pSet->mpItems+nPos, sizeof( ImplSplitItem )*(pSet->mnItems-nPos) );
    delete pSet->mpItems;
    pSet->mpItems = pNewItems;
    pSet->mnItems++;
    pSet->mbCalcPix = TRUE;

    // Item anlegen und erweitern
    pItem           = &(pSet->mpItems[nPos]);
    memset( pItem, 0, sizeof( ImplSplitItem ) );
    pItem->mnSize   = nSize;
    pItem->mnId     = nId;
    pItem->mnBits   = nBits;

    if ( pWindow )
    {
        pItem->mpWindow         = pWindow;
        pItem->mpOrgParent      = pWindow->GetParent();

        // Window mit SplitWindow verbinden
        pWindow->Hide();
        pWindow->SetParent( this );
    }
    else
    {
        pNewSet                 = new ImplSplitSet;
        pNewSet->mpItems        = NULL;
        pNewSet->mpWallpaper    = NULL;
        pNewSet->mpBitmap       = NULL;
        pNewSet->mnLastSize     = 0;
        pNewSet->mnItems        = 0;
        pNewSet->mnId           = nId;
        pNewSet->mnSplitSize    = pSet->mnSplitSize;
        pNewSet->mbCalcPix      = TRUE;

        pItem->mpSet            = pNewSet;
    }

    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::InsertItem( USHORT nId, long nSize,
                              USHORT nPos, USHORT nSetId,
                              SplitWindowItemBits nBits )
{
    InsertItem( nId, NULL, nSize, nPos, nSetId, nBits );
}

// -----------------------------------------------------------------------

void SplitWindow::MoveItem( USHORT nId, USHORT nNewPos, USHORT nNewSetId )
{
#ifdef DBG_UTIL
    USHORT nDbgDummy;
    DBG_ASSERT( ImplFindItem( mpMainSet, nId, nDbgDummy ), "SplitWindow::MoveItem() - Id not found" );
    DBG_ASSERT( ImplFindSet( mpMainSet, nNewSetId ), "SplitWindow::MoveItem() - Set not exists" );
#endif

    USHORT          nPos;
    ImplSplitSet*    pNewSet = ImplFindSet( mpMainSet, nNewSetId );
    ImplSplitSet*    pSet    = ImplFindItem( mpMainSet, nId, nPos );
    ImplSplitItem    aTempItem;

    if ( pNewSet == pSet )
    {
        if ( nNewPos >= pNewSet->mnItems )
            nNewPos = pNewSet->mnItems-1;
        if ( nPos != nNewPos )
        {
            memcpy( &aTempItem, &(pSet->mpItems[nPos]), sizeof( aTempItem ) );
            if ( nPos < nNewPos )
            {
                memmove( pSet->mpItems+nPos, pSet->mpItems+nPos+1,
                         (nNewPos-nPos)*sizeof( ImplSplitItem ) );
            }
            else
            {
                memmove( pSet->mpItems+nNewPos+1, pSet->mpItems+nNewPos,
                         (nPos-nNewPos)*sizeof( ImplSplitItem ) );
            }
            memcpy( &(pSet->mpItems[nNewPos]), &aTempItem, sizeof( aTempItem ) );

            ImplUpdate();
        }
    }
    else
    {
        if ( nNewPos >= pNewSet->mnItems )
            nNewPos = pNewSet->mnItems;
        memcpy( &aTempItem, &(pSet->mpItems[nPos]), sizeof( aTempItem ) );
        pSet->mnItems--;
        pSet->mbCalcPix = TRUE;
        if ( pSet->mnItems )
        {
            memmove( pSet->mpItems+nPos, pSet->mpItems+nPos+1,
                     (pSet->mnItems-nPos)*sizeof( ImplSplitItem ) );
        }
        else
        {
            delete pSet->mpItems;
            pSet->mpItems = NULL;
        }
        ImplSplitItem* pNewItems = new ImplSplitItem[pNewSet->mnItems+1];
        if ( nNewPos )
            memcpy( pNewItems, pNewSet->mpItems, sizeof( ImplSplitItem )*nNewPos );
        if ( nNewPos < pNewSet->mnItems )
        {
            memcpy( pNewItems+nNewPos+1, pNewSet->mpItems+nNewPos,
                    sizeof( ImplSplitItem )*(pNewSet->mnItems-nNewPos) );
        }
        delete pNewSet->mpItems;
        pNewSet->mpItems = pNewItems;
        pNewSet->mnItems++;
        pNewSet->mbCalcPix = TRUE;
        memcpy( &(pNewSet->mpItems[nNewPos]), &aTempItem, sizeof( aTempItem ) );
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

void SplitWindow::RemoveItem( USHORT nId, BOOL bHide )
{
#ifdef DBG_UTIL
    USHORT nDbgDummy;
    DBG_ASSERT( ImplFindItem( mpMainSet, nId, nDbgDummy ), "SplitWindow::RemoveItem() - Id not found" );
#endif

    // Set suchen
    USHORT          nPos;
    ImplSplitSet*    pSet    = ImplFindItem( mpMainSet, nId, nPos );
    ImplSplitItem*   pItem   = &(pSet->mpItems[nPos]);
    Window*         pWindow = pItem->mpWindow;
    Window*         pOrgParent = pItem->mpOrgParent;

    // Evt. Set loeschen
    if ( !pWindow )
        ImplDeleteSet( pItem->mpSet );

    // Item entfernen
    pSet->mnItems--;
    pSet->mbCalcPix = TRUE;
    if ( pSet->mnItems )
    {
        memmove( pSet->mpItems+nPos, pSet->mpItems+nPos+1,
                 (pSet->mnItems-nPos)*sizeof( ImplSplitItem ) );
    }
    else
    {
        delete pSet->mpItems;
        pSet->mpItems = NULL;
    }

    ImplUpdate();

    // Window erst hier loeschen, um weniger Paints zu haben
    if ( pWindow )
    {
        // Fenster wieder herstellen
        if ( bHide || (pOrgParent != this) )
        {
            pWindow->Hide();
            pWindow->SetParent( pOrgParent );
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::Clear()
{
    // Alle Sets loeschen
    ImplDeleteSet( mpMainSet );

    // Main-Set wieder anlegen
    mpMainSet                   = new ImplSplitSet;
    mpMainSet->mpItems          = NULL;
    mpMainSet->mpWallpaper      = NULL;
    mpMainSet->mpBitmap         = NULL;
    mpMainSet->mnLastSize       = 0;
    mpMainSet->mnItems          = 0;
    mpMainSet->mnId             = 0;
    mpMainSet->mnSplitSize      = SPLITWIN_SPLITSIZE;
    mpMainSet->mbCalcPix        = TRUE;
    if ( mnWinStyle & WB_NOSPLITDRAW )
        mpMainSet->mnSplitSize -= 2;
    mpBaseSet                   = mpMainSet;

    // Und neu invalidieren
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::SetBaseSet( USHORT nSetId )
{
    mpBaseSet = ImplFindSet( mpMainSet, nSetId );
}

// -----------------------------------------------------------------------

USHORT SplitWindow::GetBaseSet() const
{
    return mpBaseSet->mnId;
}

// -----------------------------------------------------------------------

void SplitWindow::SetSplitSize( USHORT nSetId, long nSplitSize,
                                BOOL bWithChilds )
{
    ImplSplitSet* pSet = ImplFindSet( mpMainSet, nSetId );
    if ( pSet )
    {
        if ( bWithChilds )
            ImplSetSplitSize( pSet, nSplitSize );
        else
            pSet->mnSplitSize = nSplitSize;
    }
    ImplUpdate();
}

// -----------------------------------------------------------------------

long SplitWindow::GetSplitSize( USHORT nSetId ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpMainSet, nSetId );
    if ( pSet )
        return pSet->mnSplitSize;
    else
        return 0;
}

// -----------------------------------------------------------------------

void SplitWindow::SetItemBackground( USHORT nSetId )
{
    Wallpaper aWall;
    SetItemBackground( nSetId, aWall );
}

// -----------------------------------------------------------------------

void SplitWindow::SetItemBackground( USHORT nSetId, const Wallpaper& rWallpaper )
{
    ImplSplitSet* pSet = ImplFindSet( mpMainSet, nSetId );

    if ( pSet )
    {
        BOOL bUpdate = TRUE;

        if ( rWallpaper.GetStyle() == WALLPAPER_NULL )
        {
            if ( pSet->mpWallpaper )
            {
                delete pSet->mpWallpaper;
                pSet->mpWallpaper = NULL;
            }
            else
                bUpdate = FALSE;
        }
        else
        {
            // Ab jetzt muss immer invalidiert werden
            mbInvalidate = TRUE;

            if ( !pSet->mpWallpaper )
                pSet->mpWallpaper = new Wallpaper( rWallpaper );
            else
                *(pSet->mpWallpaper) = rWallpaper;
        }

        // Beim MainSet koennen wir den Background umsetzen
        if ( pSet == mpMainSet )
            ImplInitSettings();

        if ( bUpdate )
            ImplUpdateSet( pSet );
    }
}

// -----------------------------------------------------------------------

Wallpaper SplitWindow::GetItemBackground( USHORT nSetId ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpMainSet, nSetId );

    if ( pSet && pSet->mpWallpaper )
        return *(pSet->mpWallpaper);
    else
    {
        Wallpaper aWall;
        return aWall;
    }
}

// -----------------------------------------------------------------------

BOOL SplitWindow::IsItemBackground( USHORT nSetId ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpMainSet, nSetId );

    if ( pSet && pSet->mpWallpaper )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void SplitWindow::SetItemBitmap( USHORT nSetId, const Bitmap& rBitmap )
{
    ImplSplitSet* pSet = ImplFindSet( mpMainSet, nSetId );

    if ( pSet )
    {
        BOOL bUpdate = TRUE;

        if ( !rBitmap )
        {
            if ( pSet->mpBitmap )
            {
                delete pSet->mpBitmap;
                pSet->mpBitmap = NULL;
            }
            else
                bUpdate = FALSE;
        }
        else
        {
            // Ab jetzt muss immer invalidiert werden
            mbInvalidate = TRUE;

            if ( !pSet->mpBitmap )
                pSet->mpBitmap = new Bitmap( rBitmap );
            else
                *(pSet->mpBitmap) = rBitmap;
        }

        // Beim MainSet koennen wir den Background umsetzen
        if ( pSet == mpMainSet )
            ImplInitSettings();

        if ( bUpdate )
            ImplUpdateSet( pSet );
    }
}

// -----------------------------------------------------------------------

Bitmap SplitWindow::GetItemBitmap( USHORT nSetId ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpMainSet, nSetId );

    if ( pSet && pSet->mpBitmap )
        return *(pSet->mpBitmap);
    else
    {
        Bitmap aBitmap;
        return aBitmap;
    }
}

// -----------------------------------------------------------------------

void SplitWindow::SplitItem( USHORT nId, long nNewSize,
                             BOOL bPropSmall, BOOL bPropGreat )
{
    USHORT          nItems;
    USHORT          nPos;
    USHORT          nMin;
    USHORT          nMax;
    USHORT          i;
    USHORT          n;
    long            nDelta;
    long            nTempDelta;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );
    ImplSplitItem*  pItems;

    if ( !pSet )
        return;

    nItems = pSet->mnItems;
    pItems = pSet->mpItems;

    if ( mbCalc )
    {
        pItems[nPos].mnSize = nNewSize;
        return;
    }

    nDelta = nNewSize-pItems[nPos].mnPixSize;
    if ( !nDelta )
        return;

    // Bereich berechnen, der beim Splitten betroffen sein kann
    nMin = 0;
    nMax = nItems;
    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mbFixed )
        {
            if ( i < nPos )
                nMin = i+1;
            else
                nMax = i;
        }
    }

    // Wenn das Fenster sizeable ist, wird das TopSet anders behandelt
    BOOL bSmall  = TRUE;
    BOOL bGreat  = TRUE;
    if ( (pSet == mpMainSet) && (mnWinStyle & WB_SIZEABLE) )
    {
        if ( nPos < pSet->mnItems-1 )
        {
            if ( !((bPropSmall && bPropGreat) ||
                   ((nDelta > 0) && bPropSmall) ||
                   ((nDelta < 0) && bPropGreat)) )
            {
                if ( nDelta < 0 )
                    bGreat = FALSE;
                else
                    bSmall = FALSE;
            }
        }
        else
        {
            if ( nDelta < 0 )
                bGreat = FALSE;
            else
                bSmall = FALSE;
        }
    }
    else if ( nPos >= nMax )
    {
        bSmall = FALSE;
        bGreat = FALSE;
    }
    else if ( nPos && (nPos >= pSet->mnItems-1) )
    {
        nPos--;
        nDelta *= -1;
        BOOL bTemp = bPropSmall;
        bPropSmall = bPropGreat;
        bPropGreat = bTemp;
    }

    // Jetzt die Fenster splitten
    if ( nDelta < 0 )
    {
        if ( bGreat )
        {
            if ( bPropGreat )
            {
                nTempDelta = nDelta;
                do
                {
                    n = nPos+1;
                    do
                    {
                        if ( nTempDelta )
                        {
                            pItems[n].mnPixSize++;
                            nTempDelta++;
                        }
                        n++;
                    }
                    while ( n < nMax );
                }
                while ( nTempDelta );
            }
            else
                pItems[nPos+1].mnPixSize -= nDelta;
        }

        if ( bSmall )
        {
            if ( bPropSmall )
            {
                do
                {
                    n = nPos+1;
                    do
                    {
                        if ( nDelta && pItems[n-1].mnPixSize )
                        {
                            pItems[n-1].mnPixSize--;
                            nDelta++;
                        }

                        n--;
                    }
                    while ( n > nMin );
                }
                while ( nDelta );
            }
            else
            {
                n = nPos+1;
                do
                {
                    if ( pItems[n-1].mnPixSize+nDelta < 0 )
                    {
                        nDelta += pItems[n-1].mnPixSize;
                        pItems[n-1].mnPixSize = 0;
                    }
                    else
                    {
                        pItems[n-1].mnPixSize += nDelta;
                        break;
                    }
                    n--;
                }
                while ( n > nMin );
            }
        }
    }
    else
    {
        if ( bGreat )
        {
            if ( bPropGreat )
            {
                nTempDelta = nDelta;
                do
                {
                    n = nPos+1;
                    do
                    {
                        if ( nTempDelta )
                        {
                            pItems[n-1].mnPixSize++;
                            nTempDelta--;
                        }
                        n--;
                    }
                    while ( n > nMin );
                }
                while ( nTempDelta );
            }
            else
                pItems[nPos].mnPixSize += nDelta;
        }

        if ( bSmall )
        {
            if ( bPropSmall )
            {
                do
                {
                    n = nPos+1;
                    do
                    {
                        if ( nDelta && pItems[n].mnPixSize )
                        {
                            pItems[n].mnPixSize--;
                            nDelta--;
                        }

                        n++;
                    }
                    while ( n < nMax );
                }
                while ( nDelta );
            }
            else
            {
                n = nPos+1;
                do
                {
                    if ( pItems[n].mnPixSize-nDelta < 0 )
                    {
                        nDelta -= pItems[n].mnPixSize;
                        pItems[n].mnPixSize = 0;
                    }
                    else
                    {
                        pItems[n].mnPixSize -= nDelta;
                        break;
                    }
                    n++;
                }
                while ( n < nMax );
            }
        }
    }

    // Original-Groessen updaten
    ImplCalcLogSize( pItems, nItems );

    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::SetItemSize( USHORT nId, long nNewSize )
{
    USHORT          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );
    ImplSplitItem*  pItem;

    if ( !pSet )
        return;

    // Testen, ob sich Groesse aendert
    pItem = &(pSet->mpItems[nPos]);
    if ( pItem->mnSize != nNewSize )
    {
        // Neue Groesse setzen und neu durchrechnen
        pItem->mnSize = nNewSize;
        pSet->mbCalcPix = TRUE;
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

long SplitWindow::GetItemSize( USHORT nId ) const
{
    USHORT          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return pSet->mpItems[nPos].mnSize;
    else
        return 0;
}

// -----------------------------------------------------------------------

long SplitWindow::GetItemSize( USHORT nId, SplitWindowItemBits nBits ) const
{
    USHORT          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
    {
        if ( nBits == pSet->mpItems[nPos].mnBits )
            return pSet->mpItems[nPos].mnSize;
        else
        {
            ((SplitWindow*)this)->ImplCalcLayout();

            long                nRelSize = 0;
            long                nPerSize = 0;
            ImplSplitItem*      pItems;
            USHORT              nItems;
            SplitWindowItemBits nTempBits;
            USHORT              i;
            nItems = pSet->mnItems;
            pItems = pSet->mpItems;
            for ( i = 0; i < nItems; i++ )
            {
                if ( i == nPos )
                    nTempBits = nBits;
                else
                    nTempBits = pItems[i].mnBits;
                if ( nTempBits & SWIB_RELATIVESIZE )
                    nRelSize += pItems[i].mnPixSize;
                else if ( nTempBits & SWIB_PERCENTSIZE )
                    nPerSize += pItems[i].mnPixSize;
            }
            nPerSize += nRelSize;
            if ( nBits & SWIB_RELATIVESIZE )
            {
                if ( nRelSize )
                    return (pItems[nPos].mnPixSize+(nRelSize/2))/nRelSize;
                else
                    return 1;
            }
            else if ( nBits & SWIB_PERCENTSIZE )
            {
                if ( nPerSize )
                    return (pItems[nPos].mnPixSize*100)/nPerSize;
                else
                    return 1;
            }
            else
                return pItems[nPos].mnPixSize;
        }
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

void SplitWindow::SetItemBits( USHORT nId, SplitWindowItemBits nNewBits )
{
    USHORT          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );
    ImplSplitItem*  pItem;

    if ( !pSet )
        return;

    pItem = &(pSet->mpItems[nPos]);
    if ( pItem->mpWindow )
        nNewBits &= ~SWIB_COLSET;

    if ( pItem->mnBits != nNewBits )
    {
        // Neue Bits setzen und neu durchrechnen
        pItem->mnBits = nNewBits;
        pSet->mbCalcPix = TRUE;
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

SplitWindowItemBits SplitWindow::GetItemBits( USHORT nId ) const
{
    USHORT          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return pSet->mpItems[nPos].mnBits;
    else
        return 0;
}

// -----------------------------------------------------------------------

Window* SplitWindow::GetItemWindow( USHORT nId ) const
{
    USHORT          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return pSet->mpItems[nPos].mpWindow;
    else
        return NULL;
}

// -----------------------------------------------------------------------

USHORT SplitWindow::GetSet( USHORT nId ) const
{
    USHORT          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return pSet->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

BOOL SplitWindow::GetSet( USHORT nId, USHORT& rSetId, USHORT& rPos ) const
{
    ImplSplitSet* pSet = ImplFindItem( mpBaseSet, nId, rPos );
    if ( pSet )
    {
        rSetId = pSet->mnId;
        return TRUE;
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------

BOOL SplitWindow::IsItemValid( USHORT nId ) const
{
    USHORT          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

USHORT SplitWindow::GetItemId( Window* pWindow ) const
{
    return ImplFindItem( mpBaseSet, pWindow );
}

// -----------------------------------------------------------------------

USHORT SplitWindow::GetItemId( const Point& rPos ) const
{
    return ImplFindItem( mpBaseSet, rPos, mbHorz, !mbBottomRight );
}

// -----------------------------------------------------------------------

USHORT SplitWindow::GetItemPos( USHORT nId, USHORT nSetId ) const
{
    ImplSplitSet*   pSet = ImplFindSet( mpBaseSet, nSetId );
    USHORT          nPos = SPLITWINDOW_ITEM_NOTFOUND;

    if ( pSet )
    {
        for ( USHORT i = 0; i < pSet->mnItems; i++ )
        {
            if ( pSet->mpItems[i].mnId == nId )
            {
                nPos = i;
                break;
            }
        }
    }

    return nPos;
}

// -----------------------------------------------------------------------

USHORT SplitWindow::GetItemId( USHORT nPos, USHORT nSetId ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpBaseSet, nSetId );
    if ( pSet && (nPos < pSet->mnItems) )
        return pSet->mpItems[nPos].mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

USHORT SplitWindow::GetItemCount( USHORT nSetId ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpBaseSet, nSetId );
    if ( pSet )
        return pSet->mnItems;
    else
        return 0;
}

// -----------------------------------------------------------------------

void SplitWindow::ImplNewAlign()
{
    if ( mbNoAlign )
    {
        mbHorz        = FALSE;
        mbBottomRight = FALSE;
    }
    else if ( meAlign == WINDOWALIGN_TOP )
    {
        mbHorz        = TRUE;
        mbBottomRight = FALSE;
    }
    else if ( meAlign == WINDOWALIGN_BOTTOM )
    {
        mbHorz        = TRUE;
        mbBottomRight = TRUE;
    }
    else if ( meAlign == WINDOWALIGN_LEFT )
    {
        mbHorz        = FALSE;
        mbBottomRight = FALSE;
    }
    else if ( meAlign == WINDOWALIGN_RIGHT )
    {
        mbHorz        = FALSE;
        mbBottomRight = TRUE;
    }

    if ( mnWinStyle & WB_BORDER )
    {
        ImplCalcBorder( meAlign, mbNoAlign, mnLeftBorder, mnTopBorder,
                        mnRightBorder, mnBottomBorder );
    }

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::SetNoAlign( BOOL bNoAlign )
{
    bNoAlign = bNoAlign != 0;
    if ( mbNoAlign != bNoAlign )
    {
        mbNoAlign = bNoAlign;
        ImplNewAlign();
    }
}

// -----------------------------------------------------------------------

void SplitWindow::SetAlign( WindowAlign eNewAlign )
{
    if ( meAlign != eNewAlign )
    {
        meAlign = eNewAlign;
        ImplNewAlign();
    }
}

// -----------------------------------------------------------------------

Size SplitWindow::CalcWindowSizePixel( const Size& rSize, WindowAlign eAlign,
                                       WinBits nWinStyle, BOOL bExtra )
{
    long    nLeft;
    long    nTop;
    long    nRight;
    long    nBottom;
    Size    aSize = rSize;

    ImplCalcBorder( eAlign, FALSE, nLeft, nTop, nRight, nBottom );
    aSize.Width()   += nLeft+nRight;
    aSize.Height()  += nTop+nBottom;

    if ( nWinStyle & WB_SIZEABLE )
    {
        if ( (eAlign == WINDOWALIGN_TOP) || (eAlign == WINDOWALIGN_BOTTOM) )
        {
            aSize.Height() += SPLITWIN_SPLITSIZE-2;
            if ( bExtra )
                aSize.Height() += SPLITWIN_SPLITSIZEEX;
        }
        else
        {
            aSize.Width() += SPLITWIN_SPLITSIZE-2;
            if ( bExtra )
                aSize.Width() += SPLITWIN_SPLITSIZEEX;
        }
    }

    return aSize;
}

// -----------------------------------------------------------------------

void SplitWindow::ShowAutoHideButton( BOOL bShow )
{
    mbAutoHide = bShow;
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::ShowFadeInHideButton( BOOL bShow )
{
    mbFadeIn = bShow;
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::ShowFadeOutButton( BOOL bShow )
{
    mbFadeOut = bShow;
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::SetAutoHideState( BOOL bAutoHide )
{
    mbAutoHideIn = bAutoHide;
    if ( IsReallyVisible() )
    {
        Rectangle aRect;
        ImplGetAutoHideRect( aRect );
        Invalidate( aRect );
    }
}

// -----------------------------------------------------------------------

long SplitWindow::GetFadeInSize() const
{
    long n = 0;

    if ( mbHorz )
        n = mnTopBorder+mnBottomBorder;
    else
        n = mnLeftBorder+mnRightBorder;

    return n+SPLITWIN_SPLITSIZE+SPLITWIN_SPLITSIZEEX-2;
}

// -----------------------------------------------------------------------

Rectangle SplitWindow::GetAutoHideRect() const
{
    Rectangle aRect;
    ImplGetAutoHideRect( aRect, TRUE );
    return aRect;
}

// -----------------------------------------------------------------------

Rectangle SplitWindow::GetFadeInRect() const
{
    Rectangle aRect;
    ImplGetFadeInRect( aRect, TRUE );
    return aRect;
}

// -----------------------------------------------------------------------

Rectangle SplitWindow::GetFadeOutRect() const
{
    Rectangle aRect;
    ImplGetFadeOutRect( aRect, TRUE );
    return aRect;
}
