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


#include <string.h>

#include <tools/debug.hxx>
#include <tools/rcid.h>
#include <tools/solar.h>

#include <vcl/event.hxx>
#include <vcl/wall.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
#include <vcl/help.hxx>
#include <vcl/splitwin.hxx>

#include <rsc/rsc-vcl-shared-types.hxx>

#include <svdata.hxx>
#include <svids.hrc>

// =======================================================================

// Attention: Must not contain non-PODs because array is enlarged/copied
// with the use of memmove/memcpy.
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
    sal_uInt16              mnId;
    SplitWindowItemBits mnBits;
    bool                mbFixed;
    bool                mbSubSize;
    /// Minimal width or height of the item.  -1 means no restriction.
    long                mnMinSize;
    /// Maximal width or height of the item.  -1 means no restriction.
    long                mnMaxSize;
};

struct ImplSplitSet
{
    ImplSplitItem*      mpItems;
    Wallpaper*          mpWallpaper;
    Bitmap*             mpBitmap;
    long                mnLastSize;
    long                mnSplitSize;
    sal_uInt16              mnItems;
    sal_uInt16              mnId;
    bool                mbCalcPix;
};



/** Check whether the given size is inside the valid range defined by
    [rItem.mnMinSize,rItem.mnMaxSize].  When it is not inside it then return
    the upper or lower bound, respectively. Otherwise return the given size
    unmodified.
    Note that either mnMinSize and/or mnMaxSize can be -1 in which case the
    size has not lower or upper bound.
*/
namespace {
    long ValidateSize (const long nSize, const ImplSplitItem &rItem)
    {
        if (rItem.mnMinSize>=0 && nSize<rItem.mnMinSize)
            return rItem.mnMinSize;
        else if (rItem.mnMaxSize>0 && nSize>rItem.mnMaxSize)
            return rItem.mnMaxSize;
        else
            return nSize;
    }
}


#define SPLITWIN_SPLITSIZE              3
#define SPLITWIN_SPLITSIZEEX            4
#define SPLITWIN_SPLITSIZEEXLN          6
#define SPLITWIN_SPLITSIZEAUTOHIDE      36
#define SPLITWIN_SPLITSIZEFADE          36

#define SPLIT_HORZ              ((sal_uInt16)0x0001)
#define SPLIT_VERT              ((sal_uInt16)0x0002)
#define SPLIT_WINDOW            ((sal_uInt16)0x0004)
#define SPLIT_NOSPLIT           ((sal_uInt16)0x8000)

// =======================================================================

static void ImplCalcBorder( WindowAlign eAlign, sal_Bool bNoAlign,
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
        switch ( eAlign )
        {
        case WINDOWALIGN_TOP:
            rLeft   = 2;
            rTop    = 2;
            rRight  = 2;
            rBottom = 0;
            break;
        case WINDOWALIGN_LEFT:
            rLeft   = 2;
            rTop    = 2;
            rRight  = 0;
            rBottom = 2;
            break;
        case WINDOWALIGN_BOTTOM:
            rLeft   = 2;
            rTop    = 0;
            rRight  = 2;
            rBottom = 2;
            break;
        default:
            rLeft   = 0;
            rTop    = 2;
            rRight  = 2;
            rBottom = 2;
            break;
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplDrawBorder( SplitWindow* pWin )
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
        switch ( pWin->meAlign )
        {
        case WINDOWALIGN_BOTTOM:
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
            pWin->DrawLine( Point( 0, 0 ), Point( 0, nDY-1 ) );
            pWin->DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-3 ) );

            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
            pWin->DrawLine( Point( 1, 1 ), Point( 1, nDY-3 ) );
            pWin->DrawLine( Point( nDX-1, 0 ), Point( nDX-1, nDY-1 ) );
            break;
        case WINDOWALIGN_TOP:
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( 0, 0 ), Point( nDX-1, 0 ) );
            pWin->DrawLine( Point( 0, 0 ), Point( 0, nDY-1 ) );
            pWin->DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-1 ) );

            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( 1, 1 ), Point( nDX-3, 1 ) );
            pWin->DrawLine( Point( 1, 1 ), Point( 1, nDY-1 ) );
            pWin->DrawLine( Point( nDX-1, 1 ), Point( nDX-1, nDY-1 ) );
            break;
        case WINDOWALIGN_LEFT:
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( 0, 0 ), Point( nDX-1, 0 ) );
            pWin->DrawLine( Point( 0, 0 ), Point( 0, nDY-1 ) );
            pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );

            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( 1, 1 ), Point( nDX-1, 1 ) );
            pWin->DrawLine( Point( 1, 1 ), Point( 1, nDY-3 ) );
            pWin->DrawLine( Point( 1, nDY-1 ), Point( nDX-1, nDY-1 ) );
            break;
        default:
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( 0, 0 ), Point( nDX-2, 0 ) );
            pWin->DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-3 ) );
            pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-2, nDY-2 ) );

            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( 0, 1 ), Point( nDX-3, 1 ) );
            pWin->DrawLine( Point( nDX-1, 0 ), Point( nDX-1, nDY-1 ) );
            pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
            break;
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplDrawBorderLine( SplitWindow* pWin )
{
    if ( pWin->mbFadeOut || pWin->mbAutoHide )
    {
        const StyleSettings&    rStyleSettings = pWin->GetSettings().GetStyleSettings();
        long                    nDX = pWin->mnDX;
        long                    nDY = pWin->mnDY;

        switch ( pWin->meAlign )
        {
        case WINDOWALIGN_LEFT:
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( nDX-SPLITWIN_SPLITSIZEEXLN-1, 0 ), Point( nDX-SPLITWIN_SPLITSIZEEXLN-1, nDY-3 ) );
            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( nDX-SPLITWIN_SPLITSIZEEXLN, 1 ), Point( nDX-SPLITWIN_SPLITSIZEEXLN, nDY-4 ) );
            break;
        case WINDOWALIGN_RIGHT:
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( SPLITWIN_SPLITSIZEEXLN-1, 0 ), Point( SPLITWIN_SPLITSIZEEXLN-1, nDY-3 ) );
            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( SPLITWIN_SPLITSIZEEXLN, 1 ), Point( SPLITWIN_SPLITSIZEEXLN, nDY-4 ) );
            break;
        case WINDOWALIGN_TOP:
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( 0, nDY-SPLITWIN_SPLITSIZEEXLN-1 ), Point( nDX-3, nDY-SPLITWIN_SPLITSIZEEXLN-1 ) );
            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( 1, nDY-SPLITWIN_SPLITSIZEEXLN ), Point( nDX-4, nDY-SPLITWIN_SPLITSIZEEXLN ) );
            break;
        case WINDOWALIGN_BOTTOM:
            pWin->SetLineColor( rStyleSettings.GetShadowColor() );
            pWin->DrawLine( Point( 0, SPLITWIN_SPLITSIZEEXLN-1 ), Point( nDX-3, SPLITWIN_SPLITSIZEEXLN-1 ) );
            pWin->SetLineColor( rStyleSettings.GetLightColor() );
            pWin->DrawLine( Point( 1, SPLITWIN_SPLITSIZEEXLN ), Point( nDX-4, SPLITWIN_SPLITSIZEEXLN ) );
            break;
        }
    }
}

// -----------------------------------------------------------------------

static ImplSplitSet* ImplFindSet( ImplSplitSet* pSet, sal_uInt16 nId )
{
    if ( pSet->mnId == nId )
        return pSet;

    sal_uInt16          i;
    sal_uInt16          nItems = pSet->mnItems;
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

static ImplSplitSet* ImplFindItem( ImplSplitSet* pSet, sal_uInt16 nId, sal_uInt16& rPos )
{
    sal_uInt16          i;
    sal_uInt16          nItems = pSet->mnItems;
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

static sal_uInt16 ImplFindItem( ImplSplitSet* pSet, Window* pWindow )
{
    sal_uInt16          i;
    sal_uInt16          nItems = pSet->mnItems;
    ImplSplitItem*  pItems = pSet->mpItems;

    for ( i = 0; i < nItems; i++ )
    {
        if ( pItems[i].mpWindow == pWindow )
            return pItems[i].mnId;
        else
        {
            if ( pItems[i].mpSet )
            {
                sal_uInt16 nId = ImplFindItem( pItems[i].mpSet, pWindow );
                if ( nId )
                    return nId;
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

static sal_uInt16 ImplFindItem( ImplSplitSet* pSet, const Point& rPos,
                            sal_Bool bRows, sal_Bool bDown = sal_True )
{
    sal_uInt16          i;
    sal_uInt16          nItems = pSet->mnItems;
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
    sal_uInt16          i;
    sal_uInt16          nItems = pSet->mnItems;
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

    delete [] pItems;
    delete pSet;
}

// -----------------------------------------------------------------------

static void ImplCalcSet( ImplSplitSet* pSet,
                         long nSetLeft, long nSetTop,
                         long nSetWidth, long nSetHeight,
                         sal_Bool bRows, sal_Bool bDown = sal_True )
{
    if ( !pSet->mpItems )
        return;

    sal_uInt16              i;
    sal_uInt16              j;
    sal_uInt16              nMins;
    sal_uInt16              nCalcItems;
    sal_uInt16              nItems = pSet->mnItems;
    sal_uInt16              nVisItems;
    sal_uInt16              nAbsItems;
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
    bool                bEmpty;

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

        pSet->mbCalcPix  = false;
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
            if ( (nAbsItems < (sal_uInt16)(std::abs( nSizeDelta ))) && nSizeWinSize )
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
                        pItems[i].mbSubSize = false;

                        if ( j >= 2 )
                            pItems[i].mbSubSize = true;
                        else
                        {
                            if ( !(pItems[i].mnBits & SWIB_INVISIBLE) )
                            {
                                if ( (nSizeDelta > 0) || pItems[i].mnPixSize )
                                {
                                    if ( j >= 1 )
                                        pItems[i].mbSubSize = true;
                                    else
                                    {
                                        if ( (j == 0) && (pItems[i].mnBits & (SWIB_RELATIVESIZE | SWIB_PERCENTSIZE)) )
                                            pItems[i].mbSubSize = true;
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
            bEmpty = true;
        else
        {
            bEmpty = false;
            if ( bDown )
            {
                if ( nPos+pItems[i].mnPixSize > nMaxPos )
                    bEmpty = true;
            }
            else
            {
                nPos -= pItems[i].mnPixSize;
                if ( nPos < nMaxPos )
                    bEmpty = true;
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
        pItems[i].mbFixed = false;
        if ( pItems[i].mnBits & SWIB_FIXED )
            pItems[i].mbFixed = true;
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
                        pItems[i].mbFixed = true;
                        break;
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplCalcSet2( SplitWindow* pWindow, ImplSplitSet* pSet, sal_Bool bHide,
                                sal_Bool bRows, sal_Bool /*bDown*/ )
{
    sal_uInt16          i;
    sal_uInt16          nItems = pSet->mnItems;
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
            sal_Bool bTempHide = bHide;
            if ( !pItems[i].mnWidth || !pItems[i].mnHeight )
                bTempHide = sal_True;
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

static void ImplCalcLogSize( ImplSplitItem* pItems, sal_uInt16 nItems )
{
    // Original-Groessen updaten
    sal_uInt16  i;
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

void SplitWindow::ImplDrawBack( SplitWindow* pWindow, const Rectangle& rRect,
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

void SplitWindow::ImplDrawBack( SplitWindow* pWindow, ImplSplitSet* pSet )
{
    sal_uInt16          i;
    sal_uInt16          nItems = pSet->mnItems;
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
                           sal_Bool bRows, sal_Bool bDown = sal_True )
{
    if ( !pSet->mpItems )
        return;

    sal_uInt16                  i;
    sal_uInt16                  nItems = pSet->mnItems;
    long                    nPos;
    long                    nTop;
    long                    nBottom;
    ImplSplitItem*          pItems = pSet->mpItems;
    const StyleSettings&    rStyleSettings = pWindow->GetSettings().GetStyleSettings();

    bool bFlat = (pWindow->GetStyle() & WB_FLATSPLITDRAW) == WB_FLATSPLITDRAW;

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

sal_uInt16 SplitWindow::ImplTestSplit( ImplSplitSet* pSet, const Point& rPos,
                                   long& rMouseOff, ImplSplitSet** ppFoundSet, sal_uInt16& rFoundPos,
                                   sal_Bool bRows, sal_Bool /*bDown*/ )
{
    if ( !pSet->mpItems )
        return 0;

    sal_uInt16          i;
    sal_uInt16          nSplitTest;
    sal_uInt16          nItems = pSet->mnItems;
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

sal_uInt16 SplitWindow::ImplTestSplit( SplitWindow* pWindow, const Point& rPos,
                                   long& rMouseOff, ImplSplitSet** ppFoundSet, sal_uInt16& rFoundPos )
{
    // Resizable SplitWindow muss anders behandelt werden
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
            nSplitSize += SPLITWIN_SPLITSIZEEXLN;
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

void SplitWindow::ImplDrawSplitTracking( SplitWindow* pThis, const Point& rPos )
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
            aRect.Left()  += SPLITWIN_SPLITSIZEEXLN;
            aRect.Right() += SPLITWIN_SPLITSIZEEXLN;
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
            aRect.Top()    += SPLITWIN_SPLITSIZEEXLN;
            aRect.Bottom() += SPLITWIN_SPLITSIZEEXLN;
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
    pNewSet->mbCalcPix      = true;

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
    mbDragFull              = sal_False;
    mbHorz                  = sal_True;
    mbBottomRight           = sal_False;
    mbCalc                  = sal_False;
    mbRecalc                = sal_True;
    mbInvalidate            = sal_True;
    mbAutoHide              = sal_False;
    mbFadeIn                = sal_False;
    mbFadeOut               = sal_False;
    mbAutoHideIn            = sal_False;
    mbAutoHideDown          = sal_False;
    mbFadeInDown            = sal_False;
    mbFadeOutDown           = sal_False;
    mbAutoHidePressed       = sal_False;
    mbFadeInPressed         = sal_False;
    mbFadeOutPressed        = sal_False;
    mbFadeNoButtonMode      = sal_False;
    mbNoAlign               = sal_False;

    if ( nStyle & WB_NOSPLITDRAW )
    {
        pNewSet->mnSplitSize -= 2;
        mbInvalidate = sal_False;
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

SplitWindow::~SplitWindow()
{
    // Sets loeschen
    ImplDeleteSet( mpMainSet );
    mpMainSet = NULL; //NULL for base-class callbacks during dtoring
}

// -----------------------------------------------------------------------

void SplitWindow::ImplSetWindowSize( long nDelta )
{
    if ( !nDelta )
        return;

    Size aSize = GetSizePixel();
    switch ( meAlign )
    {
    case WINDOWALIGN_TOP:
        aSize.Height() += nDelta;
        SetSizePixel( aSize );
        break;
    case WINDOWALIGN_BOTTOM:
    {
        maDragRect.Top() += nDelta;
        Point aPos = GetPosPixel();
        aPos.Y() -= nDelta;
        aSize.Height() += nDelta;
        SetPosSizePixel( aPos, aSize );
        break;
    }
    case WINDOWALIGN_LEFT:
        aSize.Width() += nDelta;
        SetSizePixel( aSize );
        break;
    case WINDOWALIGN_RIGHT:
    default:
    {
        maDragRect.Left() += nDelta;
        Point aPos = GetPosPixel();
        aPos.X() -= nDelta;
        aSize.Width() += nDelta;
        SetPosSizePixel( aPos, aSize );
        break;
    }
    }

    SplitResize();
}

// -----------------------------------------------------------------------

Size SplitWindow::CalcLayoutSizePixel( const Size& aNewSize )
{
    Size aSize( aNewSize );
    long nSplitSize = mpMainSet->mnSplitSize-2;

    if ( mbAutoHide || mbFadeOut )
        nSplitSize += SPLITWIN_SPLITSIZEEXLN;

    // Wenn Fenster sizeable ist, wird die groesse automatisch nach
    // dem MainSet festgelegt, wenn kein relatives Fenster enthalten
    // ist
    if ( mnWinStyle & WB_SIZEABLE )
    {
        long    nCurSize;
        long    nCalcSize = 0;
        sal_uInt16  i;

        for ( i = 0; i < mpMainSet->mnItems; i++ )
        {
            if ( mpMainSet->mpItems[i].mnBits & (SWIB_RELATIVESIZE | SWIB_PERCENTSIZE) )
                break;
            else
                nCalcSize += mpMainSet->mpItems[i].mnSize;
        }

        if ( i == mpMainSet->mnItems )
        {
            long    nDelta = 0;
            Point   aPos = GetPosPixel();

            if ( mbHorz )
                nCurSize = aNewSize.Height()-mnTopBorder-mnBottomBorder;
            else
                nCurSize = aNewSize.Width()-mnLeftBorder-mnRightBorder;
            nCurSize -= nSplitSize;
            nCurSize -= (mpMainSet->mnItems-1)*mpMainSet->mnSplitSize;

            nDelta = nCalcSize-nCurSize;
            if ( !nDelta )
                return aSize;

            switch ( meAlign )
            {
            case WINDOWALIGN_TOP:
                aSize.Height() += nDelta;
                break;
            case WINDOWALIGN_BOTTOM:
                aPos.Y() -= nDelta;
                aSize.Height() += nDelta;
                break;
            case WINDOWALIGN_LEFT:
                aSize.Width() += nDelta;
                break;
            case WINDOWALIGN_RIGHT:
            default:
                aPos.X() -= nDelta;
                aSize.Width() += nDelta;
                break;
            }
        }
    }

    return aSize;
}

// -----------------------------------------------------------------------

void SplitWindow::ImplCalcLayout()
{
    if ( !mbCalc || !mbRecalc || !mpMainSet->mpItems )
        return;

    long nSplitSize = mpMainSet->mnSplitSize-2;
    if ( mbAutoHide || mbFadeOut )
        nSplitSize += SPLITWIN_SPLITSIZEEXLN;

    // Wenn Fenster sizeable ist, wird die groesse automatisch nach
    // dem MainSet festgelegt, wenn kein relatives Fenster enthalten
    // ist
    if ( mnWinStyle & WB_SIZEABLE )
    {
        long    nCurSize;
        long    nCalcSize = 0;
        sal_uInt16  i;

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

            mbRecalc = sal_False;
            ImplSetWindowSize( nCalcSize-nCurSize );
            mbRecalc = sal_True;
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
    ImplCalcSet2( this, mpMainSet, sal_False, mbHorz, !mbBottomRight );
    mbCalc = sal_False;
}

// -----------------------------------------------------------------------

void SplitWindow::ImplUpdate()
{
    mbCalc = sal_True;

    if ( IsReallyShown() && IsUpdateMode() && mbRecalc )
    {
        if ( mpMainSet->mpItems )
            ImplCalcLayout();
        else
            Invalidate();
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

void SplitWindow::ImplGetButtonRect( Rectangle& rRect, long nEx, sal_Bool bTest ) const
{
    long nSplitSize = mpMainSet->mnSplitSize-2;
    if ( mbAutoHide || mbFadeOut || mbFadeIn )
        nSplitSize += SPLITWIN_SPLITSIZEEX;

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

    switch ( meAlign )
    {
    case WINDOWALIGN_TOP:
        rRect.Left()    = mnLeftBorder+nEx;
        rRect.Top()     = mnDY-mnBottomBorder-nSplitSize;
        rRect.Right()   = rRect.Left()+SPLITWIN_SPLITSIZEAUTOHIDE;
        rRect.Bottom()  = mnDY-mnBottomBorder-1;
        if ( bTest )
        {
            rRect.Top()     -= mnTopBorder;
            rRect.Bottom()  += mnBottomBorder;
        }
        break;
    case WINDOWALIGN_BOTTOM:
        rRect.Left()    = mnLeftBorder+nEx;
        rRect.Top()     = mnTopBorder;
        rRect.Right()   = rRect.Left()+SPLITWIN_SPLITSIZEAUTOHIDE;
        rRect.Bottom()  = mnTopBorder+nSplitSize-1;
        if ( bTest )
        {
            rRect.Top()     -= mnTopBorder;
            rRect.Bottom()  += mnBottomBorder;
        }
        break;
    case WINDOWALIGN_LEFT:
        rRect.Left()    = mnDX-mnRightBorder-nSplitSize;
        rRect.Top()     = mnTopBorder+nEx;
        rRect.Right()   = mnDX-mnRightBorder-1;
        rRect.Bottom()  = rRect.Top()+SPLITWIN_SPLITSIZEAUTOHIDE;
        if ( bTest )
        {
            rRect.Left()    -= mnLeftBorder;
            rRect.Right()   += mnRightBorder;
        }
        break;
    case WINDOWALIGN_RIGHT:
        rRect.Left()    = mnLeftBorder;
        rRect.Top()     = mnTopBorder+nEx;
        rRect.Right()   = mnLeftBorder+nSplitSize-1;
        rRect.Bottom()  = rRect.Top()+SPLITWIN_SPLITSIZEAUTOHIDE;
        if ( bTest )
        {
            rRect.Left()    -= mnLeftBorder;
            rRect.Right()   += mnRightBorder;
        }
        break;
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplGetAutoHideRect( Rectangle& rRect, sal_Bool bTest ) const
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

void SplitWindow::ImplGetFadeInRect( Rectangle& rRect, sal_Bool bTest ) const
{
    Rectangle aRect;

    if ( mbFadeIn )
        ImplGetButtonRect( aRect, 0, bTest );

    rRect = aRect;
}

// -----------------------------------------------------------------------

void SplitWindow::ImplGetFadeOutRect( Rectangle& rRect, sal_Bool ) const
{
    Rectangle aRect;

    if ( mbFadeOut )
        ImplGetButtonRect( aRect, 0, sal_False );

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
            if ( (i < nEx1) || ((i > nEx2 ) && (i < nRight-3)) )
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
            if ( (i < nEx1) || ((i > nEx2 ) && (i < nBottom-3)) )
            {
                DrawPixel( Point( nCenter+2, i ), rStyleSettings.GetLightColor() );
                DrawPixel( Point( nCenter+2+1, i+1 ), rStyleSettings.GetShadowColor() );
            }
            i += 2;
        }
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplDrawAutoHide( sal_Bool bInPaint )
{
    if ( mbAutoHide )
    {
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
                ResMgr* pResMgr = ImplGetResMgr();
                if( pResMgr )
                {
                    Color aNonAlphaMask( 0x00, 0x00, 0xFF );
                    pSVData->maCtrlData.mpSplitHPinImgList = new ImageList(4);
                    pSVData->maCtrlData.mpSplitHPinImgList->InsertFromHorizontalBitmap
                        ( ResId( SV_RESID_BITMAP_SPLITHPIN, *pResMgr ), 4, &aNonAlphaMask );
                }
                }
            pImageList = pSVData->maCtrlData.mpSplitHPinImgList;
        }
        else
        {
            if ( !pSVData->maCtrlData.mpSplitVPinImgList )
            {
                ResMgr* pResMgr = ImplGetResMgr();
                pSVData->maCtrlData.mpSplitVPinImgList = new ImageList(4);
                if( pResMgr )
                {
                    Color aNonAlphaMask( 0x00, 0x00, 0xFF );
                    pSVData->maCtrlData.mpSplitVPinImgList->InsertFromHorizontalBitmap
                        ( ResId( SV_RESID_BITMAP_SPLITVPIN, *pResMgr ), 4, &aNonAlphaMask );
                }
            }
            pImageList = pSVData->maCtrlData.mpSplitVPinImgList;
                }

        // Image ermitteln und zurueckgeben
        sal_uInt16 nId;
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

void SplitWindow::ImplDrawFadeArrow( const Point& rPt, sal_Bool bHorz, sal_Bool bLeft )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    int x( rPt.X() );
    int y( rPt.Y() );

    Color aCol;
    if( !bHorz )
    {
        int dx = 1;
        if( bLeft )
        {
            x ++;
            dx = -1;
        }

        x++; y++;
        aCol = Color( COL_WHITE );
        DrawPixel( Point(x, y), aCol );
        DrawPixel( Point(x, y+1), aCol );
        DrawPixel( Point(x, y+2), aCol );
        DrawPixel( Point(x+dx, y+1), aCol );

        x--; y--;
        aCol = rStyleSettings.GetDarkShadowColor();
        DrawPixel( Point(x, y), rStyleSettings.GetDarkShadowColor() );
        DrawPixel( Point(x, y+1), rStyleSettings.GetDarkShadowColor() );
        DrawPixel( Point(x, y+2), rStyleSettings.GetDarkShadowColor() );
        DrawPixel( Point(x+dx, y+1), rStyleSettings.GetDarkShadowColor() );
    }
    else
    {
        int dy = 1;
        if( bLeft )
        {
            y ++;
            dy = -1;
        }

        x++; y++;
        aCol = Color( COL_WHITE );
        DrawPixel( Point(x, y), aCol );
        DrawPixel( Point(x+1, y), aCol );
        DrawPixel( Point(x+2, y), aCol );
        DrawPixel( Point(x+1, y+dy), aCol );

        x--; y--;
        aCol = rStyleSettings.GetDarkShadowColor();
        DrawPixel( Point(x, y), aCol );
        DrawPixel( Point(x+1, y), aCol );
        DrawPixel( Point(x+2, y), aCol );
        DrawPixel( Point(x+1, y+dy), aCol );
    }
}

void SplitWindow::ImplDrawGrip( const Rectangle& rRect, sal_Bool bHorz, sal_Bool bLeft )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if( rRect.IsInside( GetPointerPosPixel() ) )
    {
        DrawWallpaper( rRect, Wallpaper( Color( COL_WHITE ) ) );
        DrawSelectionBackground( rRect, 2, sal_False, sal_False, sal_False );
    }

    if( bHorz )
    {
        int width = (int) (0.5 * rRect.getWidth() + 0.5);
        int i = rRect.Left() + (rRect.getWidth() - width) / 2;
        width += i;
        const int y = rRect.Top() + 1;
        ImplDrawFadeArrow( Point( i-8, y), bHorz, bLeft );
        while( i <= width )
        {

            DrawPixel( Point(i, y), rStyleSettings.GetDarkShadowColor() );
            DrawPixel( Point(i+1, y), rStyleSettings.GetShadowColor() );

            DrawPixel( Point(i, y+1), rStyleSettings.GetShadowColor() );
            DrawPixel( Point(i+1, y+1), rStyleSettings.GetFaceColor() );
            DrawPixel( Point(i+2, y+1), Color(COL_WHITE) );

            DrawPixel( Point(i+1, y+2), Color(COL_WHITE) );
            DrawPixel( Point(i+2, y+2), Color(COL_WHITE) );
            i+=4;
        }
        ImplDrawFadeArrow( Point( i+3, y), bHorz, bLeft );
    }
    else
    {
        int height = (int) (0.5 * rRect.getHeight() + 0.5);
        int i = rRect.Top() + (rRect.getHeight() - height) / 2;
        height += i;
        const int x = rRect.Left() + 1;
        ImplDrawFadeArrow( Point( x, i-8), bHorz, bLeft );
        while( i <= height )
        {

            DrawPixel( Point(x, i), rStyleSettings.GetDarkShadowColor() );
            DrawPixel( Point(x+1, i), rStyleSettings.GetShadowColor() );

            DrawPixel( Point(x, i+1), rStyleSettings.GetShadowColor() );
            DrawPixel( Point(x+1, i+1), rStyleSettings.GetFaceColor() );
            DrawPixel( Point(x+2, i+1), Color(COL_WHITE) );

            DrawPixel( Point(x+1, i+2), Color(COL_WHITE) );
            DrawPixel( Point(x+2, i+2), Color(COL_WHITE) );
            i+=4;
        }
        ImplDrawFadeArrow( Point( x, i+3), bHorz, bLeft );
    }
}

void SplitWindow::ImplDrawFadeIn( sal_Bool bInPaint )
{
    if ( mbFadeIn )
    {
        Rectangle       aTempRect;
        ImplGetFadeInRect( aTempRect );

        sal_Bool bLeft = sal_True;
        switch ( meAlign )
        {
        case WINDOWALIGN_TOP:
        case WINDOWALIGN_LEFT:
            bLeft = sal_False;
            break;
        case WINDOWALIGN_BOTTOM:
        case WINDOWALIGN_RIGHT:
        default:
            bLeft = sal_True;
            break;
        }

        if ( !bInPaint )
            Erase( aTempRect );

        ImplDrawGrip( aTempRect, (meAlign == WINDOWALIGN_TOP) || (meAlign == WINDOWALIGN_BOTTOM), bLeft );
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ImplDrawFadeOut( sal_Bool bInPaint )
{
    if ( mbFadeOut )
    {
        Rectangle       aTempRect;
        ImplGetFadeOutRect( aTempRect );

        sal_Bool bLeft = sal_True;
        switch ( meAlign )
        {
        case WINDOWALIGN_BOTTOM:
        case WINDOWALIGN_RIGHT:
            bLeft = sal_False;
            break;
        case WINDOWALIGN_TOP:
        case WINDOWALIGN_LEFT:
        default:
            bLeft = sal_True;
            break;
        }

        if ( !bInPaint )
            Erase( aTempRect );

        ImplDrawGrip( aTempRect, (meAlign == WINDOWALIGN_TOP) || (meAlign == WINDOWALIGN_BOTTOM), bLeft );
    }
}

// -----------------------------------------------------------------------
void SplitWindow::ImplStartSplit( const MouseEvent& rMEvt )
{
    Point aMousePosPixel = rMEvt.GetPosPixel();
    mnSplitTest = ImplTestSplit( this, aMousePosPixel, mnMouseOff, &mpSplitSet, mnSplitPos );

    if ( mnSplitTest && !(mnSplitTest & SPLIT_NOSPLIT) )
    {
        ImplSplitItem*  pSplitItem;
        long            nCurMaxSize;
        sal_uInt16          nTemp;
        bool            bDown;
        bool            bPropSmaller;

        mnMouseModifier = rMEvt.GetModifier();
        if ( !(mnMouseModifier & KEY_SHIFT) || (mnSplitPos+1 >= mpSplitSet->mnItems) )
            bPropSmaller = false;
        else
            bPropSmaller = true;

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
            bDown = true;
            if ( (mpSplitSet == mpMainSet) && mbBottomRight )
                bDown = false;

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
            sal_uInt16          nItems = mpSplitSet->mnItems;
            mpLastSizes = new long[nItems*2];
            for ( sal_uInt16 i = 0; i < nItems; i++ )
            {
                mpLastSizes[i*2]   = pItems[i].mnSize;
                mpLastSizes[i*2+1] = pItems[i].mnPixSize;
            }
        }
        mnMStartPos = mnMSplitPos;

        PointerStyle eStyle = POINTER_ARROW;
        if ( mnSplitTest & SPLIT_HORZ )
            eStyle = POINTER_HSPLIT;
        else if ( mnSplitTest & SPLIT_VERT )
            eStyle = POINTER_VSPLIT;

        Pointer aPtr( eStyle );
        SetPointer( aPtr );
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

    mbFadeNoButtonMode = sal_False;
    ImplGetAutoHideRect( aTestRect, sal_True );
    if ( aTestRect.IsInside( aMousePosPixel ) )
    {
        mbAutoHideDown = sal_True;
        mbAutoHidePressed = sal_True;
        ImplDrawAutoHide( sal_False );
    }
    else
    {
        ImplGetFadeOutRect( aTestRect, sal_True );
        if ( aTestRect.IsInside( aMousePosPixel ) )
        {
            mbFadeOutDown = sal_True;
            mbFadeOutPressed = sal_True;
            ImplDrawFadeOut( sal_False );
        }
        else
        {
            ImplGetFadeInRect( aTestRect, sal_True );
            if ( aTestRect.IsInside( aMousePosPixel ) )
            {
                mbFadeInDown = sal_True;
                mbFadeInPressed = sal_True;
                ImplDrawFadeIn( sal_False );
            }
            else if ( !aTestRect.IsEmpty() && !(mnWinStyle & WB_SIZEABLE) )
            {
                mbFadeNoButtonMode = sal_True;
                FadeIn();
                return;
            }
        }
    }

    if ( mbAutoHideDown || mbFadeInDown || mbFadeOutDown )
        StartTracking();
    else
        ImplStartSplit( rMEvt );
}

// -----------------------------------------------------------------------

void SplitWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( !IsTracking() )
    {
        Point           aPos = rMEvt.GetPosPixel();
        long            nTemp;
        ImplSplitSet*   pTempSplitSet;
        sal_uInt16          nTempSplitPos;
        sal_uInt16          nSplitTest = ImplTestSplit( this, aPos, nTemp, &pTempSplitSet, nTempSplitPos );
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
            mbAutoHideDown = sal_False;
            if ( mbAutoHidePressed )
            {
                mbAutoHidePressed = sal_False;

                if ( !rTEvt.IsTrackingCanceled() )
                {
                    mbAutoHideIn = !mbAutoHideIn;
                    ImplDrawAutoHide( sal_False );
                    AutoHide();
                }
                else
                    ImplDrawAutoHide( sal_False );
            }
        }
        else
        {
            Rectangle aTestRect;
            ImplGetAutoHideRect( aTestRect, sal_True );
            sal_Bool bNewPressed = aTestRect.IsInside( aMousePosPixel );
            if ( bNewPressed != mbAutoHidePressed )
            {
                mbAutoHidePressed = bNewPressed;
                ImplDrawAutoHide( sal_False );
            }
        }
    }
    else if ( mbFadeInDown )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbFadeInDown = sal_False;
            if ( mbFadeInPressed )
            {
                mbFadeInPressed = sal_False;
                ImplDrawFadeIn( sal_False );

                if ( !rTEvt.IsTrackingCanceled() )
                    FadeIn();
            }
        }
        else
        {
            Rectangle aTestRect;
            ImplGetFadeInRect( aTestRect, sal_True );
            sal_Bool bNewPressed = aTestRect.IsInside( aMousePosPixel );
            if ( bNewPressed != mbFadeInPressed )
            {
                mbFadeInPressed = bNewPressed;
                ImplDrawFadeIn( sal_False );
            }
        }
    }
    else if ( mbFadeOutDown )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbFadeOutDown = sal_False;
            if ( mbFadeOutPressed )
            {
                mbFadeOutPressed = sal_False;
                ImplDrawFadeOut( sal_False );

                if ( !rTEvt.IsTrackingCanceled() )
                    FadeOut();
            }
        }
        else
        {
            Rectangle aTestRect;
            ImplGetFadeOutRect( aTestRect, sal_True );
            sal_Bool bNewPressed = aTestRect.IsInside( aMousePosPixel );
            if ( bNewPressed == sal_False )
            {
                mbFadeOutPressed = bNewPressed;
                ImplDrawFadeOut( sal_False );

                // We need a mouseevent with a position inside the button for the
                // ImplStartSplit function!
                MouseEvent aOrgMEvt = rTEvt.GetMouseEvent();
                MouseEvent aNewMEvt = MouseEvent( aTestRect.Center(), aOrgMEvt.GetClicks(),
                                                  aOrgMEvt.GetMode(), aOrgMEvt.GetButtons(),
                                                  aOrgMEvt.GetModifier() );

                ImplStartSplit( aNewMEvt );
                mbFadeOutDown = sal_False;
            }
        }
    }
    else
    {
        ImplSplitMousePos( aMousePosPixel );
        bool bSplit = true;
        if ( mbDragFull )
        {
            if ( rTEvt.IsTrackingEnded() )
            {
                if ( rTEvt.IsTrackingCanceled() )
                {
                    ImplSplitItem*  pItems = mpSplitSet->mpItems;
                    sal_uInt16          nItems = mpSplitSet->mnItems;
                    for ( sal_uInt16 i = 0; i < nItems; i++ )
                    {
                        pItems[i].mnSize     = mpLastSizes[i*2];
                        pItems[i].mnPixSize  = mpLastSizes[i*2+1];
                    }
                    ImplUpdate();
                    Split();
                }
                bSplit = false;
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
                bSplit = false;
            }
        }

        if ( bSplit )
        {
            sal_Bool    bPropSmaller = (mnMouseModifier & KEY_SHIFT) ? sal_True : sal_False;
            sal_Bool    bPropGreater = (mnMouseModifier & KEY_MOD1) ? sal_True : sal_False;
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
            delete [] mpLastSizes;
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

long SplitWindow::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != NULL )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            Rectangle aFadeInRect;
            Rectangle aFadeOutRect;
            ImplGetFadeInRect( aFadeInRect );
            ImplGetFadeOutRect( aFadeOutRect );

            if ( aFadeInRect.IsInside( GetPointerPosPixel() ) != aFadeInRect.IsInside( GetLastPointerPosPixel() ) )
                Invalidate( aFadeInRect );
            if ( aFadeOutRect.IsInside( GetPointerPosPixel() ) != aFadeOutRect.IsInside( GetLastPointerPosPixel() ) )
                Invalidate( aFadeOutRect );

            if( pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow() )
            {
                Invalidate( aFadeInRect );
                Invalidate( aFadeOutRect );
            }
        }
    }
    return Window::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void SplitWindow::Paint( const Rectangle& )
{
    if ( mnWinStyle & WB_BORDER )
        ImplDrawBorder( this );

    ImplDrawBorderLine( this );
    ImplDrawFadeOut( sal_True );
    ImplDrawFadeIn( sal_True );
    ImplDrawAutoHide( sal_True );

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
    // no keyboard help for splitwin
    if ( rHEvt.GetMode() & (HELPMODE_BALLOON | HELPMODE_QUICK) && !rHEvt.KeyboardActivated() )
    {
        Point       aMousePosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        Rectangle   aHelpRect;
        sal_uInt16      nHelpResId = 0;

        ImplGetAutoHideRect( aHelpRect, sal_True );
        if ( aHelpRect.IsInside( aMousePosPixel ) )
        {
            if ( mbAutoHideIn )
                nHelpResId = SV_HELPTEXT_SPLITFIXED;
            else
                nHelpResId = SV_HELPTEXT_SPLITFLOATING;
        }
        else
        {
            ImplGetFadeInRect( aHelpRect, sal_True );
            if ( aHelpRect.IsInside( aMousePosPixel ) )
                nHelpResId = SV_HELPTEXT_FADEIN;
            else
            {
                ImplGetFadeOutRect( aHelpRect, sal_True );
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
            OUString aStr;
            ResMgr* pResMgr = ImplGetResMgr();
            if( pResMgr )
                aStr = ResId( nHelpResId, *pResMgr ).toString();
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
    switch ( nType )
    {
    case STATE_CHANGE_INITSHOW:
        if ( IsUpdateMode() )
            ImplCalcLayout();
        break;
    case STATE_CHANGE_UPDATEMODE:
        if ( IsUpdateMode() && IsReallyShown() )
            ImplCalcLayout();
        break;
    case STATE_CHANGE_CONTROLBACKGROUND:
        ImplInitSettings();
        Invalidate();
        break;
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

void SplitWindow::InsertItem( sal_uInt16 nId, Window* pWindow, long nSize,
                              sal_uInt16 nPos, sal_uInt16 nSetId,
                              SplitWindowItemBits nBits )
{
#ifdef DBG_UTIL
    sal_uInt16 nDbgDummy;
    DBG_ASSERT( ImplFindSet( mpMainSet, nSetId ), "SplitWindow::InsertItem() - Set not exists" );
    DBG_ASSERT( !ImplFindItem( mpMainSet, nId, nDbgDummy ), "SplitWindow::InsertItem() - Id already exists" );
#endif

    // Size has to be at least 1.
    if ( nSize < 1 )
        nSize = 1;

    ImplSplitSet* pSet       = ImplFindSet( mpMainSet, nSetId );
    ImplSplitSet* pNewSet;
    ImplSplitItem* pItem;

    // Make room for the new item.
    if ( nPos > pSet->mnItems )
        nPos = pSet->mnItems;
    ImplSplitItem* pNewItems = new ImplSplitItem[pSet->mnItems+1];
    if ( nPos )
        memcpy( pNewItems, pSet->mpItems, sizeof( ImplSplitItem )*nPos );
    if ( nPos < pSet->mnItems )
        memcpy( pNewItems+nPos+1, pSet->mpItems+nPos, sizeof( ImplSplitItem )*(pSet->mnItems-nPos) );
    delete[] pSet->mpItems;
    pSet->mpItems = pNewItems;
    pSet->mnItems++;
    pSet->mbCalcPix = true;

    // Create and initialize item.
    pItem           = &(pSet->mpItems[nPos]);
    memset( pItem, 0, sizeof( ImplSplitItem ) );
    pItem->mnSize   = nSize;
    pItem->mnId     = nId;
    pItem->mnBits   = nBits;
    pItem->mnMinSize=-1;
    pItem->mnMaxSize=-1;

    if ( pWindow )
    {
        pItem->mpWindow         = pWindow;
        pItem->mpOrgParent      = pWindow->GetParent();

        // Attach window to SplitWindow.
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
        pNewSet->mbCalcPix      = true;

        pItem->mpSet            = pNewSet;
    }

    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::InsertItem( sal_uInt16 nId, long nSize,
                              sal_uInt16 nPos, sal_uInt16 nSetId,
                              SplitWindowItemBits nBits )
{
    InsertItem( nId, NULL, nSize, nPos, nSetId, nBits );
}

// -----------------------------------------------------------------------

void SplitWindow::RemoveItem( sal_uInt16 nId, sal_Bool bHide )
{
#ifdef DBG_UTIL
    sal_uInt16 nDbgDummy;
    DBG_ASSERT( ImplFindItem( mpMainSet, nId, nDbgDummy ), "SplitWindow::RemoveItem() - Id not found" );
#endif

    // Set suchen
    sal_uInt16          nPos;
    ImplSplitSet*    pSet    = ImplFindItem( mpMainSet, nId, nPos );
    ImplSplitItem*   pItem   = &(pSet->mpItems[nPos]);
    Window*         pWindow = pItem->mpWindow;
    Window*         pOrgParent = pItem->mpOrgParent;

    // Evt. Set loeschen
    if ( !pWindow )
        ImplDeleteSet( pItem->mpSet );

    // Item entfernen
    pSet->mnItems--;
    pSet->mbCalcPix = true;
    if ( pSet->mnItems )
    {
        memmove( pSet->mpItems+nPos, pSet->mpItems+nPos+1,
                 (pSet->mnItems-nPos)*sizeof( ImplSplitItem ) );
    }
    else
    {
        delete[] pSet->mpItems;
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
    mpMainSet->mbCalcPix        = true;
    if ( mnWinStyle & WB_NOSPLITDRAW )
        mpMainSet->mnSplitSize -= 2;
    mpBaseSet                   = mpMainSet;

    // Und neu invalidieren
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::SplitItem( sal_uInt16 nId, long nNewSize,
                             sal_Bool bPropSmall, sal_Bool bPropGreat )
{
    sal_uInt16          nItems;
    sal_uInt16          nPos;
    sal_uInt16          nMin;
    sal_uInt16          nMax;
    sal_uInt16          i;
    sal_uInt16          n;
    long            nDelta;
    long            nTempDelta;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );
    ImplSplitItem*  pItems;

    if ( !pSet )
        return;

    nItems = pSet->mnItems;
    pItems = pSet->mpItems;

    // When there is an explicit minimum or maximum size then move nNewSize
    // into that range (when it is not yet already in it.)
    nNewSize = ValidateSize(nNewSize, pItems[nPos]);

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
    bool bSmall  = true;
    bool bGreat  = true;
    if ( (pSet == mpMainSet) && (mnWinStyle & WB_SIZEABLE) )
    {
        if ( nPos < pSet->mnItems-1 )
        {
            if ( !((bPropSmall && bPropGreat) ||
                   ((nDelta > 0) && bPropSmall) ||
                   ((nDelta < 0) && bPropGreat)) )
            {
                if ( nDelta < 0 )
                    bGreat = false;
                else
                    bSmall = false;
            }
        }
        else
        {
            if ( nDelta < 0 )
                bGreat = false;
            else
                bSmall = false;
        }
    }
    else if ( nPos >= nMax )
    {
        bSmall = false;
        bGreat = false;
    }
    else if ( nPos && (nPos >= pSet->mnItems-1) )
    {
        nPos--;
        nDelta *= -1;
        sal_Bool bTemp = bPropSmall;
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

void SplitWindow::SetItemSize( sal_uInt16 nId, long nNewSize )
{
    sal_uInt16          nPos;
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
        pSet->mbCalcPix = true;
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

long SplitWindow::GetItemSize( sal_uInt16 nId ) const
{
    sal_uInt16          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return pSet->mpItems[nPos].mnSize;
    else
        return 0;
}

// -----------------------------------------------------------------------

long SplitWindow::GetItemSize( sal_uInt16 nId, SplitWindowItemBits nBits ) const
{
    sal_uInt16          nPos;
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
            sal_uInt16              nItems;
            SplitWindowItemBits nTempBits;
            sal_uInt16              i;
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




void SplitWindow::SetItemSizeRange (sal_uInt16 nId, const Range aRange)
{
    sal_uInt16 nPos;
    ImplSplitSet* pSet = ImplFindItem(mpBaseSet, nId, nPos);

    if (pSet != NULL)
    {
        pSet->mpItems[nPos].mnMinSize = aRange.Min();
        pSet->mpItems[nPos].mnMaxSize = aRange.Max();
    }
}

// -----------------------------------------------------------------------

sal_uInt16 SplitWindow::GetSet( sal_uInt16 nId ) const
{
    sal_uInt16          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return pSet->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

sal_Bool SplitWindow::IsItemValid( sal_uInt16 nId ) const
{
    sal_uInt16          nPos;
    ImplSplitSet* pSet = mpBaseSet ? ImplFindItem(mpBaseSet, nId, nPos) : NULL;

    if ( pSet )
        return sal_True;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_uInt16 SplitWindow::GetItemId( Window* pWindow ) const
{
    return ImplFindItem( mpBaseSet, pWindow );
}

// -----------------------------------------------------------------------

sal_uInt16 SplitWindow::GetItemId( const Point& rPos ) const
{
    return ImplFindItem( mpBaseSet, rPos, mbHorz, !mbBottomRight );
}

// -----------------------------------------------------------------------

sal_uInt16 SplitWindow::GetItemPos( sal_uInt16 nId, sal_uInt16 nSetId ) const
{
    ImplSplitSet*   pSet = ImplFindSet( mpBaseSet, nSetId );
    sal_uInt16          nPos = SPLITWINDOW_ITEM_NOTFOUND;

    if ( pSet )
    {
        for ( sal_uInt16 i = 0; i < pSet->mnItems; i++ )
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

sal_uInt16 SplitWindow::GetItemId( sal_uInt16 nPos, sal_uInt16 nSetId ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpBaseSet, nSetId );
    if ( pSet && (nPos < pSet->mnItems) )
        return pSet->mpItems[nPos].mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

sal_uInt16 SplitWindow::GetItemCount( sal_uInt16 nSetId ) const
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
        mbHorz        = sal_False;
        mbBottomRight = sal_False;
    }
    else
    {
        switch ( meAlign )
        {
        case WINDOWALIGN_TOP:
            mbHorz        = sal_True;
            mbBottomRight = sal_False;
            break;
        case WINDOWALIGN_BOTTOM:
            mbHorz        = sal_True;
            mbBottomRight = sal_True;
            break;
        case WINDOWALIGN_LEFT:
            mbHorz        = sal_False;
            mbBottomRight = sal_False;
            break;
        case WINDOWALIGN_RIGHT:
            mbHorz        = sal_False;
            mbBottomRight = sal_True;
            break;
        }
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

void SplitWindow::SetAlign( WindowAlign eNewAlign )
{
    if ( meAlign != eNewAlign )
    {
        meAlign = eNewAlign;
        ImplNewAlign();
    }
}

// -----------------------------------------------------------------------

void SplitWindow::ShowAutoHideButton( sal_Bool bShow )
{
    mbAutoHide = bShow;
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::ShowFadeInHideButton( sal_Bool bShow )
{
    mbFadeIn = bShow;
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::ShowFadeOutButton( sal_Bool bShow )
{
    mbFadeOut = bShow;
    ImplUpdate();
}

// -----------------------------------------------------------------------

void SplitWindow::SetAutoHideState( sal_Bool bAutoHide )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
