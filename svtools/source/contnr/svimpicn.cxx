/*************************************************************************
 *
 *  $RCSfile: svimpicn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:56 $
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

#include <limits.h>
#ifndef _METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifdef DBG_UTIL
#include <vcl/sound.hxx>
#endif

#pragma hdrstop

#include <svlbox.hxx>
#include <svicnvw.hxx>
#ifndef _SVIMPICN_HXX
#include <svimpicn.hxx>
#endif
#ifndef _SVLBITM_HXX
#include <svlbitm.hxx>
#endif
#ifndef _SVARRAY_HXX
#include "svarray.hxx"
#endif



#define VIEWMODE_ICON   0x0001  // Text unter Bitmap
#define VIEWMODE_NAME   0x0002  // Text rechts neben Bitmap
#define VIEWMODE_TEXT   0x0004  // Text ohne Bitmap

#define DD_SCROLL_PIXEL 10

// alle Angaben in Pixel

#define ICONVIEW_OFFS_BMP_STRING    3

// fuer das Bounding-Rectangle
#define LROFFS_BOUND                2
#define TBOFFS_BOUND                2

// fuer das Focus-Rectangle um Icons
#define LROFFS_ICON                 2
#define TBOFFS_ICON                 2

#define NAMEVIEW_OFFS_BMP_STRING    3

// Abstaende von Fensterraendern
#define LROFFS_WINBORDER            4
#define TBOFFS_WINBORDER            4

// Breitenoffset Highlight-Rect bei Text
#define LROFFS_TEXT                 2


#define ICNVIEWDATA(xPtr) (SvIcnVwDataEntry*)(pView->GetViewDataEntry(xPtr))
#define ICNVIEWDATA2(xPtr) (SvIcnVwDataEntry*)(pView->pView->GetViewDataEntry(xPtr))

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// -------------------------------------------------------------------------
// Hilfsfunktionen von Thomas Hosemann zur mehrzeiligen Ausgabe von
// Strings. Die Funktionen werden spaeter in StarView integriert.
// -------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

// keine doppelten Defines
#ifdef TEXT_DRAW_CLIP
#undef TEXT_DRAW_CLIP
#endif
#ifdef TEXT_DRAW_MULTILINE
#undef TEXT_DRAW_MULTILINE
#endif
#ifdef TEXT_DRAW_WORDBREAK
#undef TEXT_DRAW_WORDBREAK
#endif

// #define TEXT_DRAW_DISABLE           ((USHORT)0x0001)
// #define TEXT_DRAW_3DLOOK            ((USHORT)0x0002)
// #define TEXT_DRAW_MNEMONIC          ((USHORT)0x0004)
#define TEXT_DRAW_LEFT              ((USHORT)0x0010)
#define TEXT_DRAW_CENTER            ((USHORT)0x0020)
#define TEXT_DRAW_RIGHT             ((USHORT)0x0040)
#define TEXT_DRAW_TOP               ((USHORT)0x0080)
#define TEXT_DRAW_VCENTER           ((USHORT)0x0100)
#define TEXT_DRAW_BOTTOM            ((USHORT)0x0200)
#define TEXT_DRAW_ENDELLIPSIS       ((USHORT)0x0400)
#define TEXT_DRAW_PATHELLIPSIS      ((USHORT)0x0800)
#define TEXT_DRAW_CLIP              ((USHORT)0x1000)
#define TEXT_DRAW_MULTILINE         ((USHORT)0x2000)
#define TEXT_DRAW_WORDBREAK         ((USHORT)0x4000)

XubString GetEllipsisString( OutputDevice* pDev,
                            const XubString& rStr, long nMaxWidth,
                            USHORT nStyle = TEXT_DRAW_ENDELLIPSIS )
{
    XubString aStr = rStr;

    if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
    {
        USHORT nIndex = pDev->GetTextBreak( rStr, nMaxWidth );
        if ( nIndex != STRING_LEN )
        {
            aStr.Erase( nIndex );
            if ( nIndex > 1 )
            {
                aStr.AppendAscii("...");
                while ( aStr.Len() &&
                        (pDev->GetTextWidth( aStr ) > nMaxWidth) )
                {
                    if ( (nIndex > 1) || (nIndex == aStr.Len()) )
                        nIndex--;
                    aStr.Erase( nIndex, 1 );
                }
            }

            if ( !aStr.Len() && (nStyle & TEXT_DRAW_CLIP) )
                aStr += rStr.GetChar( 0 );
        }
    }

    return aStr;
}

class TextLineInfo
{
private:
    long        mnWidth;
    USHORT      mnIndex;
    USHORT      mnLen;

public:
                TextLineInfo( long nWidth, USHORT nIndex, USHORT nLen )
                {
                    mnWidth = nWidth;
                    mnIndex = nIndex;
                    mnLen   = nLen;
                }

    long        GetWidth() const { return mnWidth; }
    USHORT      GetIndex() const { return mnIndex; }
    USHORT      GetLen() const { return mnLen; }
};

#define MULTITEXTLINEINFO_RESIZE    16
typedef TextLineInfo* PTextLineInfo;

class MultiTextLineInfo
{
private:
    PTextLineInfo*      mpLines;
    USHORT              mnLines;
    USHORT              mnSize;

public:
                        MultiTextLineInfo();
                        ~MultiTextLineInfo();

    void                AddLine( TextLineInfo* pLine );
    void                Clear();

    TextLineInfo*       GetLine( USHORT nLine ) const
                            { return mpLines[nLine]; }
    USHORT              Count() const { return mnLines; }

private:
                        MultiTextLineInfo( const MultiTextLineInfo& );
    MultiTextLineInfo&  operator=( const MultiTextLineInfo& );
};

MultiTextLineInfo::MultiTextLineInfo()
{
    mpLines         = new PTextLineInfo[MULTITEXTLINEINFO_RESIZE];
    mnLines         = 0;
    mnSize          = MULTITEXTLINEINFO_RESIZE;
}

MultiTextLineInfo::~MultiTextLineInfo()
{
    for ( USHORT i = 0; i < mnLines; i++ )
        delete mpLines[i];
    delete mpLines;
}

void MultiTextLineInfo::AddLine( TextLineInfo* pLine )
{
    if ( mnSize == mnLines )
    {
        mnSize += MULTITEXTLINEINFO_RESIZE;
        PTextLineInfo* pNewLines = new PTextLineInfo[mnSize];
        memcpy( pNewLines, mpLines, mnLines*sizeof(PTextLineInfo) );
        mpLines = pNewLines;
    }

    mpLines[mnLines] = pLine;
    mnLines++;
}

void MultiTextLineInfo::Clear()
{
    for ( USHORT i = 0; i < mnLines; i++ )
        delete mpLines[i];
    mnLines = 0;
}

// -----------------------------------------------------------------------

long GetTextLines( OutputDevice* pDev, MultiTextLineInfo& rLineInfo,
                   long nWidth, const XubString& rStr,
                   USHORT nStyle = TEXT_DRAW_WORDBREAK )
{
    rLineInfo.Clear();
    if ( !rStr.Len() )
        return 0;
    if ( nWidth <= 0 )
        nWidth = 1;

    USHORT          nStartPos       = 0;                // Start-Position der Zeile
    USHORT          nLastLineLen    = 0;                // Zeilenlaenge bis zum vorherigen Wort
    USHORT          nLastWordPos    = 0;                // Position des letzten Wortanfangs
    USHORT          i               = 0;
    USHORT          nPos;                               // StartPositon der Zeile (nur Temp)
    USHORT          nLen;                               // Laenge der Zeile (nur Temp)
    USHORT          nStrLen         = rStr.Len();
    long            nMaxLineWidth   = 0;                // Maximale Zeilenlaenge
    long            nLineWidth;                         // Aktuelle Zeilenlaenge
    long            nLastLineWidth  = 0;                // Zeilenlaenge der letzten Zeile
    xub_Unicode          c;
    xub_Unicode          c2;
    const xub_Unicode*   pStr       = rStr.GetBuffer();
    BOOL            bHardBreak      = FALSE;

    do
    {
        c = pStr[i];

        // Auf Zeilenende ermitteln
        if ( (c == _CR) || (c == _LF) )
            bHardBreak = TRUE;
        else
            bHardBreak = FALSE;

        // Testen, ob ein Wortende erreicht ist
        if ( bHardBreak || (i == nStrLen) ||
             (((c == ' ') || (c == '-')) && (nStyle & TEXT_DRAW_WORDBREAK)) )
        {
            nLen = i-nStartPos;
            if ( c == '-' )
                nLen++;
            nLineWidth = pDev->GetTextWidth( rStr, nStartPos, nLen );

            // Findet ein Zeilenumbruch statt
            if ( bHardBreak || (i == nStrLen) ||
                ((nLineWidth >= nWidth) && (nStyle & TEXT_DRAW_WORDBREAK)) )
            {
                nPos = nStartPos;

                if ( (nLineWidth >= nWidth) && (nStyle & TEXT_DRAW_WORDBREAK) )
                {
                    nLineWidth      = nLastLineWidth;
                    nLen            = nLastLineLen;
                    nStartPos       = nLastWordPos;
                    nLastLineLen    = i-nStartPos;
                    nLastWordPos    = nStartPos+nLastLineLen+1;
                    if ( c == '-' )
                        nLastLineLen++;
                    else if ( bHardBreak && (i > nStartPos) )
                        i--;
                }
                else
                {
                    nStartPos = i;
                    // Zeilenende-Zeichen und '-' beruecksichtigen
                    if ( bHardBreak )
                    {
                        nStartPos++;
                        c2 = pStr[i+1];
                        if ( (c != c2) && ((c2 == _CR) || (c2 == _LF)) )
                        {
                            nStartPos++;
                            i++;
                        }
                    }
                    else if ( c != '-' )
                        nStartPos++;
                    nLastWordPos    = nStartPos;
                    nLastLineLen    = 0;
                }

                if ( nLineWidth > nMaxLineWidth )
                    nMaxLineWidth = nLineWidth;

                if ( nLen || bHardBreak  )
                    rLineInfo.AddLine( new TextLineInfo( nLineWidth, nPos, nLen ) );

                // Testen, ob aktuelles Wort noch auf die Zeile passt,
                // denn ansonsten mueessen wir es auftrennen
                if ( nLastLineLen )
                {
                    nLineWidth = pDev->GetTextWidth( rStr, nStartPos, nLastLineLen );
                    if ( nLineWidth > nWidth )
                    {
                        // Wenn ein Wortumbruch in einem Wort stattfindet,
                        // ist die maximale Zeilenlaenge die Laenge
                        // des laengsten Wortes
                        if ( nLineWidth > nMaxLineWidth )
                            nMaxLineWidth = nLineWidth;

                        // Solange Wort auftrennen, bis es auf eine Zeile passt
                        do
                        {
                            nPos = pDev->GetTextBreak( rStr, nWidth, nStartPos, nLastLineLen );
                            nLen = nPos-nStartPos;
                            if ( !nLen )
                            {
                                nPos++;
                                nLen++;
                            }
                            nLineWidth = pDev->GetTextWidth( rStr, nStartPos, nLen );
                            rLineInfo.AddLine( new TextLineInfo( nLineWidth, nStartPos, nLen ) );
                            nStartPos       = nPos;
                            nLastLineLen   -= nLen;
                            nLineWidth = pDev->GetTextWidth( rStr, nStartPos, nLastLineLen );
                        }
                        while ( nLineWidth > nWidth );
                    }
                    nLastLineWidth = nLineWidth;

                    // Bei Stringende muessen wir die letzte Zeile auch noch
                    // dranhaengen
                    if ( (i == nStrLen) && nLastLineLen )
                        rLineInfo.AddLine( new TextLineInfo( nLastLineWidth, nStartPos, nLastLineLen ) );
                }
                else
                    nLastLineWidth = 0;
            }
            else
            {
                nLastLineWidth  = nLineWidth;
                nLastLineLen    = nLen;
                nLastWordPos    = nStartPos+nLastLineLen;
                if ( c != '-' )
                    nLastWordPos++;
            }
        }

        i++;
    }
    while ( i <= nStrLen );

    return nMaxLineWidth;
}

// -----------------------------------------------------------------------

USHORT GetTextLines( OutputDevice* pDev, const Rectangle& rRect,
                     const XubString& rStr,
                     USHORT nStyle = TEXT_DRAW_WORDBREAK,
                     long* pMaxWidth = NULL )
{
    MultiTextLineInfo aMultiLineInfo;
    long nMaxWidth = GetTextLines( pDev, aMultiLineInfo,
                                   rRect.GetWidth(), rStr, nStyle );
    if ( pMaxWidth )
        *pMaxWidth = nMaxWidth;
    return aMultiLineInfo.Count();
}

// -----------------------------------------------------------------------

Rectangle GetTextRect( OutputDevice* pDev, const Rectangle& rRect,
                       const XubString& rStr,
                       USHORT nStyle = TEXT_DRAW_WORDBREAK )
{
    Rectangle           aRect = rRect;
    USHORT              nLines;
    long                nWidth = rRect.GetWidth();
    long                nMaxWidth;
    long                nTextHeight;

    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        MultiTextLineInfo   aMultiLineInfo;
        TextLineInfo*       pLineInfo;
        USHORT              nFormatLines;

        nMaxWidth = 0;
        GetTextLines( pDev, aMultiLineInfo, nWidth, rStr, nStyle );
        nFormatLines = aMultiLineInfo.Count();
        nTextHeight = pDev->GetTextHeight();
        nLines = (USHORT)(aRect.GetHeight()/nTextHeight);
        if ( nFormatLines <= nLines )
            nLines = nFormatLines;
        else
        {
            if ( !(nStyle & TEXT_DRAW_ENDELLIPSIS) )
                nLines = nFormatLines;
            else
                nMaxWidth = nWidth;
        }
        for ( USHORT i = 0; i < nLines; i++ )
        {
            pLineInfo = aMultiLineInfo.GetLine( i );
            if ( pLineInfo->GetWidth() > nMaxWidth )
                nMaxWidth = pLineInfo->GetWidth();
        }
    }
    else
    {
        nLines          = 1;
        nMaxWidth       = pDev->GetTextWidth( rStr );
        nTextHeight     = pDev->GetTextHeight();
        if ( (nMaxWidth > nWidth) && (nStyle & TEXT_DRAW_ENDELLIPSIS) )
            nMaxWidth = nWidth;
    }

    if ( nStyle & TEXT_DRAW_RIGHT )
        aRect.Left() = aRect.Right()-nMaxWidth+1;
    else if ( nStyle & TEXT_DRAW_CENTER )
    {
        aRect.Left() += (nWidth-nMaxWidth)/2;
        aRect.Right() = aRect.Left()+nMaxWidth-1;
    }
    else
        aRect.Right() = aRect.Left()+nMaxWidth-1;

    if ( nStyle & TEXT_DRAW_BOTTOM )
        aRect.Top() = aRect.Bottom()-(nTextHeight*nLines)+1;
    else if ( nStyle & TEXT_DRAW_VCENTER )
    {
        aRect.Top()   += (aRect.GetHeight()-(nTextHeight*nLines))/2;
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;
    }
    else
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;

    return aRect;
}

// -----------------------------------------------------------------------

void DrawText( OutputDevice* pDev, const Rectangle& rRect,
               const XubString& rStr, USHORT nStyle = 0 )
{
    if ( !rStr.Len() || rRect.IsEmpty() )
        return;

    Point       aPos    = rRect.TopLeft();
    long        nWidth  = rRect.GetWidth();
    long        nHeight = rRect.GetHeight();
    FontAlign   eAlign  = pDev->GetFont().GetAlign();

    if ( ((nWidth <= 0) || (nHeight <= 0)) && (nStyle & TEXT_DRAW_CLIP) )
        return;

    // Mehrzeiligen Text behandeln wir anders
    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        String              aLastLine;
        Region              aOldRegion;
        MultiTextLineInfo   aMultiLineInfo;
        TextLineInfo*       pLineInfo;
        long                nTextHeight     = pDev->GetTextHeight();
        long                nMaxTextWidth;
        USHORT              i;
        USHORT              nLines          = (USHORT)(nHeight/nTextHeight);
        USHORT              nFormatLines;
        BOOL                bIsClipRegion;
        nMaxTextWidth = GetTextLines( pDev, aMultiLineInfo, nWidth, rStr, nStyle );

        nFormatLines = aMultiLineInfo.Count();
        if ( nFormatLines > nLines )
        {
            if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
            {
                // Letzte Zeile zusammenbauen und kuerzen
                nFormatLines = nLines-1;
                pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                aLastLine = rStr.Copy( pLineInfo->GetIndex() );
                aLastLine.ConvertLineEnd( LINEEND_LF );
                aLastLine.SearchAndReplace( _LF, ' ' );
                aLastLine = GetEllipsisString( pDev, aLastLine, nWidth, nStyle );
                nStyle &= ~(TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM);
                nStyle |= TEXT_DRAW_TOP;
            }
        }
        else
        {
            if ( nMaxTextWidth <= nWidth )
                nStyle &= ~TEXT_DRAW_CLIP;
        }

        // Clipping setzen
        if ( nStyle & TEXT_DRAW_CLIP )
        {
            bIsClipRegion = pDev->IsClipRegion();
            if ( bIsClipRegion )
            {
                aOldRegion = pDev->GetClipRegion();
                pDev->IntersectClipRegion( rRect );
            }
            else
            {
                Region aRegion( rRect );
                pDev->SetClipRegion( aRegion );
            }
        }

        // Vertikales Alignment
        if ( nStyle & TEXT_DRAW_BOTTOM )
            aPos.Y() += nHeight-(nFormatLines*nTextHeight);
        else if ( nStyle & TEXT_DRAW_VCENTER )
            aPos.Y() += (nHeight-(nFormatLines*nTextHeight))/2;

        // Font Alignment
        if ( eAlign == ALIGN_BOTTOM )
            aPos.Y() += nTextHeight;
        else if ( eAlign == ALIGN_BASELINE )
            aPos.Y() += pDev->GetFontMetric().GetAscent();

        // Alle Zeilen ausgeben, bis auf die letzte
        for ( i = 0; i < nFormatLines; i++ )
        {
            pLineInfo = aMultiLineInfo.GetLine( i );
            if ( nStyle & TEXT_DRAW_RIGHT )
                aPos.X() += nWidth-pLineInfo->GetWidth();
            else if ( nStyle & TEXT_DRAW_CENTER )
                aPos.X() += (nWidth-pLineInfo->GetWidth())/2;
            pDev->DrawText( aPos, rStr, pLineInfo->GetIndex(), pLineInfo->GetLen() );
            aPos.Y() += nTextHeight;
            aPos.X() = rRect.Left();
        }

        // Gibt es noch eine letzte Zeile, dann diese linksbuendig ausgeben,
        // da die Zeile gekuerzt wurde
        if ( aLastLine.Len() )
            pDev->DrawText( aPos, aLastLine );

        // Clipping zuruecksetzen
        if ( nStyle & TEXT_DRAW_CLIP )
        {
            if ( bIsClipRegion )
                pDev->SetClipRegion( aOldRegion );
            else
                pDev->SetClipRegion();
        }
    }
    else
    {
        XubString    aStr = rStr;
        Size        aTextSize(pDev->GetTextWidth( aStr ), pDev->GetTextHeight());

        // Evt. Text kuerzen
        if ( aTextSize.Width() > nWidth )
        {
            if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
            {
                aStr = GetEllipsisString( pDev, rStr, nWidth, nStyle );
                nStyle &= ~(TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT);
                nStyle |= TEXT_DRAW_LEFT;
                aTextSize.Width() = pDev->GetTextWidth(aStr);
            }
        }
        else
        {
            if ( aTextSize.Height() <= nHeight )
                nStyle &= ~TEXT_DRAW_CLIP;
        }

        // Vertikales Alignment
        if ( nStyle & TEXT_DRAW_RIGHT )
            aPos.X() += nWidth-aTextSize.Width();
        else if ( nStyle & TEXT_DRAW_CENTER )
            aPos.X() += (nWidth-aTextSize.Width())/2;

        // Font Alignment
        if ( eAlign == ALIGN_BOTTOM )
            aPos.Y() += aTextSize.Height();
        else if ( eAlign == ALIGN_BASELINE )
            aPos.Y() += pDev->GetFontMetric().GetAscent();

        if ( nStyle & TEXT_DRAW_BOTTOM )
            aPos.Y() += nHeight-aTextSize.Height();
        else if ( nStyle & TEXT_DRAW_VCENTER )
            aPos.Y() += (nHeight-aTextSize.Height())/2;

        if ( nStyle & TEXT_DRAW_CLIP )
        {
            BOOL bIsClipRegion = pDev->IsClipRegion();
            if ( bIsClipRegion )
            {
                Region aOldRegion = pDev->GetClipRegion();
                pDev->IntersectClipRegion( rRect );
                pDev->DrawText( aPos, aStr );
                pDev->SetClipRegion( aOldRegion );
            }
            else
            {
                Region aRegion( rRect );
                pDev->SetClipRegion( aRegion );
                pDev->DrawText( aPos, aStr );
                pDev->SetClipRegion();
            }
        }
        else
            pDev->DrawText( aPos, aStr );
    }
}

// -----------------------------------------------------------------------


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


#define DRAWTEXT_FLAGS (TEXT_DRAW_CENTER|TEXT_DRAW_TOP|TEXT_DRAW_ENDELLIPSIS|\
                        TEXT_DRAW_CLIP|TEXT_DRAW_MULTILINE|TEXT_DRAW_WORDBREAK)


class ImpIcnCursor
{
    SvImpIconView*  pView;
    SvPtrarr*       pColumns;
    SvPtrarr*       pRows;
    BOOL*           pGridMap;
    long            nGridDX, nGridDY;
    long            nGridCols, nGridRows;
    long            nCols;
    long            nRows;
    short           nDeltaWidth;
    short           nDeltaHeight;
    SvLBoxEntry*    pCurEntry;
    void            SetDeltas();
    void            ImplCreate();
    void            Create() {  if( !pColumns ) ImplCreate(); }

    USHORT          GetSortListPos( SvPtrarr* pList, long nValue, int bVertical);
    SvLBoxEntry*    SearchCol(USHORT nCol,USHORT nTop,USHORT nBottom,USHORT nPref,
                        BOOL bDown, BOOL bSimple );
    SvLBoxEntry*    SearchRow(USHORT nRow,USHORT nRight,USHORT nLeft,USHORT nPref,
                        BOOL bRight, BOOL bSimple );

    void            ExpandGrid();
    void            CreateGridMap();
    // Rueckgabe FALSE: Eintrag liegt nicht in der GridMap. rGridx,y werden
    // dann an nGridCols, nGridRows geclippt
    BOOL            GetGrid( const Point& rDocPos, USHORT& rGridX, USHORT& rGridY ) const;
    void            SetGridUsed( USHORT nDX, USHORT nDY, BOOL bUsed )
                    {
                        pGridMap[ (nDY * nGridCols) + nDX ] = bUsed;
                    }
    BOOL            IsGridUsed( USHORT nDX, USHORT nDY )
                    {
                        return pGridMap[ (nDY * nGridCols) + nDX ];
                    }
public:
                    ImpIcnCursor( SvImpIconView* pOwner );
                    ~ImpIcnCursor();
    void            Clear( BOOL bGridToo = TRUE );

    // fuer Cursortravelling usw.
    SvLBoxEntry*    GoLeftRight( SvLBoxEntry*, BOOL bRight );
    SvLBoxEntry*    GoUpDown( SvLBoxEntry*, BOOL bDown );

    // Rueckgaebe: FALSE == Das leere Rect steht hinter dem letzten
    // Eintrag; d.h. beim naechsten Einfuegen ergibt sich das naechste
    // leere Rechteck durch Addition. Hinweis: Das Rechteck kann dann
    // ausserhalb des View-Space liegen
    BOOL            FindEmptyGridRect( Rectangle& rRect );

    // Erzeugt fuer jede Zeile (Hoehe=nGridDY) eine nach BoundRect.Left()
    // sortierte Liste der Eintraege, die in ihr stehen. Eine Liste kann
    // leer sein. Die Listen gehen in das Eigentum des Rufenden ueber und
    // muessen mit DestroyGridAdjustData geloescht werden
    void            CreateGridAjustData( SvPtrarr& pLists, SvLBoxEntry* pRow=0);
    static void     DestroyGridAdjustData( SvPtrarr& rLists );
    void            SetGridUsed( const Rectangle&, BOOL bUsed = TRUE );
};




SvImpIconView::SvImpIconView( SvIconView* pCurView, SvLBoxTreeList* pTree,
    WinBits nWinStyle ) :
    aVerSBar( pCurView, WB_DRAG | WB_VSCROLL ),
    aHorSBar( pCurView, WB_DRAG | WB_HSCROLL )
{
    pView = pCurView;
    pModel = pTree;
    pCurParent = 0;
    pZOrderList = new SvPtrarr;
    SetWindowBits( nWinStyle );
    nHorDist = 0;
    nVerDist = 0;
    nFlags = 0;
    nCurUserEvent = 0;
    nMaxVirtWidth = 200;
    pDDRefEntry = 0;
    pDDDev = 0;
    pDDBufDev = 0;
    pDDTempDev = 0;
    eTextMode = ShowTextShort;
    pImpCursor = new ImpIcnCursor( this );

    aVerSBar.SetScrollHdl( LINK( this, SvImpIconView, ScrollUpDownHdl ) );
    aHorSBar.SetScrollHdl( LINK( this, SvImpIconView, ScrollLeftRightHdl ) );
    nHorSBarHeight = aHorSBar.GetSizePixel().Height();
    nVerSBarWidth = aVerSBar.GetSizePixel().Width();

    aMouseMoveTimer.SetTimeout( 20 );
    aMouseMoveTimer.SetTimeoutHdl(LINK(this,SvImpIconView,MouseMoveTimeoutHdl));

    aEditTimer.SetTimeout( 800 );
    aEditTimer.SetTimeoutHdl(LINK(this,SvImpIconView,EditTimeoutHdl));

    Clear( TRUE );
}

SvImpIconView::~SvImpIconView()
{
    StopEditTimer();
    CancelUserEvent();
    delete pZOrderList;
    delete pImpCursor;
    delete pDDDev;
    delete pDDBufDev;
    delete pDDTempDev;
    ClearSelectedRectList();
}

void SvImpIconView::Clear( BOOL bInCtor )
{
    StopEditTimer();
    CancelUserEvent();
    nMaxBmpWidth = 0;
    nMaxBmpHeight = 0;
    nMaxTextWidth = 0;
    bMustRecalcBoundingRects = FALSE;
    nMaxBoundHeight = 0;

    //XXX
    nFlags |= F_GRID_INSERT;
    nFlags &= ~F_PAINTED;
    SetNextEntryPos( Point( LROFFS_WINBORDER, TBOFFS_WINBORDER ) );
    pCursor = 0;
    if( !bInCtor )
    {
        pImpCursor->Clear();
        aVirtOutputSize.Width() = 0;
        aVirtOutputSize.Height() = 0;
        pZOrderList->Remove(0,pZOrderList->Count());
        MapMode aMapMode( pView->GetMapMode());
        aMapMode.SetOrigin( Point() );
        pView->SetMapMode( aMapMode );
        if( pView->IsUpdateMode() )
            pView->Invalidate();
    }
    AdjustScrollBars();
}

void SvImpIconView::SetWindowBits( WinBits nWinStyle )
{
    nWinBits = nWinStyle;
    nViewMode = VIEWMODE_TEXT;
    if( nWinStyle & WB_NAME )
        nViewMode = VIEWMODE_NAME;
    if( nWinStyle & WB_ICON )
        nViewMode = VIEWMODE_ICON;
}


IMPL_LINK( SvImpIconView, ScrollUpDownHdl, ScrollBar *, pScrollBar )
{
    pView->EndEditing( TRUE );
    // Pfeil hoch: delta=-1; Pfeil runter: delta=+1
    Scroll( 0, pScrollBar->GetDelta(), TRUE );
    return 0;
}

IMPL_LINK( SvImpIconView, ScrollLeftRightHdl, ScrollBar *, pScrollBar )
{
    pView->EndEditing( TRUE );
    // Pfeil links: delta=-1; Pfeil rechts: delta=+1
    Scroll( pScrollBar->GetDelta(), 0, TRUE );
    return 0;
}

void SvImpIconView::ChangedFont()
{
    StopEditTimer();
    ImpArrange();
}


void SvImpIconView::CheckAllSizes()
{
    nMaxTextWidth = 0;
    nMaxBmpWidth = 0;
    nMaxBmpHeight = 0;
    SvLBoxEntry* pEntry = pModel->First();
    while( pEntry )
    {
        CheckSizes( pEntry );
        pEntry = pModel->Next( pEntry );
    }
}

void SvImpIconView::CheckSizes( SvLBoxEntry* pEntry,
    const SvIcnVwDataEntry* pViewData )
{
    Size aSize;

    if( !pViewData )
        pViewData = ICNVIEWDATA(pEntry);

    SvLBoxString* pStringItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if( pStringItem )
    {
        aSize = GetItemSize( pView, pEntry, pStringItem, pViewData );
        if( aSize.Width() > nMaxTextWidth )
        {
            nMaxTextWidth = aSize.Width();
            if( !(nFlags & F_GRIDMODE ) )
                bMustRecalcBoundingRects = TRUE;
        }
    }
    SvLBoxContextBmp* pBmpItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if( pBmpItem )
    {
        aSize = GetItemSize( pView, pEntry, pBmpItem, pViewData );
        if( aSize.Width() > nMaxBmpWidth )
        {
            nMaxBmpWidth = aSize.Width();
            nMaxBmpWidth += (2*LROFFS_ICON);
            if( !(nFlags & F_GRIDMODE ) )
                bMustRecalcBoundingRects = TRUE;
        }
        if( aSize.Height() > nMaxBmpHeight )
        {
            nMaxBmpHeight = aSize.Height();
            nMaxBmpHeight += (2*TBOFFS_ICON);;
            if( !(nFlags & F_GRIDMODE ) )
                bMustRecalcBoundingRects = TRUE;
        }
    }
}

void SvImpIconView::EntryInserted( SvLBoxEntry* pEntry )
{
    if( pModel->GetParent(pEntry) == pCurParent )
    {
        StopEditTimer();
        DBG_ASSERT(pZOrderList->GetPos(pEntry)==0xffff,"EntryInserted:ZOrder?");
        pZOrderList->Insert( pEntry, pZOrderList->Count() );
        if( nFlags & F_GRIDMODE )
            pImpCursor->Clear( FALSE );
        else
            pImpCursor->Clear( TRUE );
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        CheckSizes( pEntry, pViewData );
        if( pView->IsUpdateMode() )
        {
            FindBoundingRect( pEntry, pViewData );
            PaintEntry( pEntry, pViewData );
        }
        else
            InvalidateBoundingRect( pViewData->aRect );
    }
}

void SvImpIconView::RemovingEntry( SvLBoxEntry* pEntry )
{
    if( pModel->GetParent(pEntry) == pCurParent)
    {
        StopEditTimer();
        DBG_ASSERT(pZOrderList->GetPos(pEntry)!=0xffff,"RemovingEntry:ZOrder?");
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        if( IsBoundingRectValid( pViewData->aRect ) )
        {
            // bei gueltigem Bounding-Rect muss in EntryRemoved eine
            // Sonderbehandlung erfolgen
            nFlags |= F_ENTRY_REMOVED;
            pView->Invalidate( pViewData->aRect );
        }
        if( pEntry == pCursor )
        {
            SvLBoxEntry* pNewCursor = GetNewCursor();
            ShowCursor( FALSE );
            pCursor = 0; // damit er nicht deselektiert wird
            SetCursor( pNewCursor );
        }
        USHORT nPos = pZOrderList->GetPos( (void*)pEntry );
        pZOrderList->Remove( nPos, 1 );
        pImpCursor->Clear();
    }
}

void SvImpIconView::EntryRemoved()
{
    if( (nFlags & (F_ENTRY_REMOVED | F_PAINTED)) == (F_ENTRY_REMOVED | F_PAINTED))
    {
        // Ein Eintrag mit gueltigem BoundRect wurde geloescht und wir
        // haben schon mal gepaintet. In diesem Fall muessen wir die
        // Position des naechsten Eintrags, der eingefuegt wird oder noch
        // kein gueltiges BoundRect hat, "suchen" d.h. ein "Loch" in
        // der View auffuellen.
        nFlags &= ~( F_ENTRY_REMOVED | F_GRID_INSERT );
    }
}


void SvImpIconView::MovingEntry( SvLBoxEntry* pEntry )
{
    DBG_ASSERT(pEntry,"MovingEntry: 0!");
    pNextCursor = 0;
    StopEditTimer();
    if( pModel->GetParent(pEntry) == pCurParent )
    {
        DBG_ASSERT(pZOrderList->GetPos(pEntry)!=0xffff,"MovingEntry:ZOrder?");
        nFlags |= F_MOVING_SIBLING;
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        if( IsBoundingRectValid( pViewData->aRect ) )
            pView->Invalidate( pViewData->aRect );
        // falls Eintrag seinen Parent wechselt vorsichtshalber
        // die neue Cursorposition berechnen
        if( pEntry == pCursor )
            pNextCursor = GetNewCursor();
        pImpCursor->Clear();
    }
}


void SvImpIconView::EntryMoved( SvLBoxEntry* pEntry )
{
    ShowCursor( FALSE );
    SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
    if( pModel->GetParent(pEntry)==pCurParent )
    {
        if( nFlags & F_MOVING_SIBLING )
        {
            // die Neu-Positionierung eines Eintrags bei D&D innerhalb
            // einer IconView findet bereits in NotifyMoving statt
            // (MovingEntry/EntryMoved wird dann nicht mehr gerufen)
            ToTop( pEntry );
        }
        else
        {
            pImpCursor->Clear();
            pZOrderList->Insert( pEntry, pZOrderList->Count() );
            DBG_ASSERT(pZOrderList->Count()==pModel->GetChildCount(pCurParent),"EntryMoved:Bad zorder count");
            FindBoundingRect( pEntry, pViewData );
        }
        PaintEntry( pEntry, pViewData );
    }
    else
    {
        if( pEntry == pCursor )
        {
            DBG_ASSERT(pNextCursor,"EntryMoved: Next cursor bad");
            SetCursor( pNextCursor );
        }
        pImpCursor->Clear();
        USHORT nPos = pZOrderList->GetPos( (void*)pEntry );
        pZOrderList->Remove( nPos, 1 );
        pView->Select( pEntry, FALSE );
        // wenn er nochmal in dieser View auftaucht, muss sein
        // Bounding-Rect neu berechnet werden
        InvalidateBoundingRect( pViewData->aRect );
    }
    nFlags &= (~F_MOVING_SIBLING);
}

void SvImpIconView::TreeInserted( SvLBoxEntry* pEntry )
{
    EntryMoved( pEntry ); // vorlaeufig
}

void SvImpIconView::EntryExpanded( SvLBoxEntry* )
{
}

void SvImpIconView::EntryCollapsed( SvLBoxEntry*)
{
}

void SvImpIconView::CollapsingEntry( SvLBoxEntry* )
{
}

void SvImpIconView::EntrySelected( SvLBoxEntry* pEntry, BOOL bSelect )
{
    if( pModel->GetParent(pEntry) != pCurParent  )
        return;

    // bei SingleSelection dafuer sorgen, dass der Cursor immer
    // auf dem (einzigen) selektierten Eintrag steht
    if( bSelect && pCursor &&
        pView->GetSelectionMode() == SINGLE_SELECTION &&
        pEntry != pCursor )
    {
        SetCursor( pEntry );
        DBG_ASSERT(pView->GetSelectionCount()==1,"selection count?")
    }
    // bei Gummibandselektion ist uns das zu teuer
    if( !(nFlags & F_RUBBERING ))
        ToTop( pEntry );
    if( pView->IsUpdateMode() )
    {
        if( pEntry == pCursor )
            ShowCursor( FALSE );
        if( nFlags & F_RUBBERING )
            PaintEntry( pEntry );
        else
            pView->Invalidate( GetBoundingRect( pEntry ) );
        if( pEntry == pCursor )
            ShowCursor( TRUE );
    }
}

void SvImpIconView::SetNextEntryPos(const Point& rPos)
{
    aPrevBoundRect.SetPos( rPos );
    aPrevBoundRect.Right() = LONG_MAX;  // dont know
}

Point SvImpIconView::FindNextEntryPos( const Size& rBoundSize )
{
    if( nFlags & F_GRIDMODE )
    {
        if( nFlags & F_GRID_INSERT )
        {
            if( aPrevBoundRect.Right() != LONG_MAX )
            {
                // passt der naechste Entry noch in die Zeile ?
                long nNextWidth = aPrevBoundRect.Right() + nGridDX + LROFFS_WINBORDER;
                if( nNextWidth > aVirtOutputSize.Width() )
                {
                    // darf aVirtOutputSize verbreitert werden ?
                    if( nNextWidth < nMaxVirtWidth )
                    {
                        // verbreitern & in Zeile aufnehmen
                        aPrevBoundRect.Left() += nGridDX;
                    }
                    else
                    {
                        // erhoehen & neue Zeile beginnen
                        aPrevBoundRect.Top() += nGridDY;
                        aPrevBoundRect.Left() = LROFFS_WINBORDER;
                    }
                }
                else
                {
                    // in die Zeile aufnehmen
                    aPrevBoundRect.Left() += nGridDX;
                }
            }
            aPrevBoundRect.SetSize( Size( nGridDX, nGridDY ) );
        }
        else
        {
            if( !pImpCursor->FindEmptyGridRect( aPrevBoundRect ) )
            {
                // mitten in den Entries gibts keine Loecher mehr,
                // wir koennen also wieder ins "Fast Insert" springen
                nFlags |= F_GRID_INSERT;
            }
        }
    }
    else
    {
        if( aPrevBoundRect.Right() != LONG_MAX )
        {
            // passt der naechste Entry noch in die Zeile ?
            long nNextWidth=aPrevBoundRect.Right()+rBoundSize.Width()+LROFFS_BOUND+nHorDist;
            if( nNextWidth > aVirtOutputSize.Width() )
            {
                // darf aVirtOutputSize verbreitert werden ?
                if( nNextWidth < nMaxVirtWidth )
                {
                    // verbreitern & in Zeile aufnehmen
                    aPrevBoundRect.SetPos( aPrevBoundRect.TopRight() );
                    aPrevBoundRect.Left() += nHorDist;
                }
                else
                {
                    // erhoehen & neue Zeile beginnen
                    aPrevBoundRect.Top() += nMaxBoundHeight + nVerDist + TBOFFS_BOUND;
                    aPrevBoundRect.Left() = LROFFS_WINBORDER;
                }
            }
            else
            {
                // in die Zeile aufnehmen
                aPrevBoundRect.SetPos( aPrevBoundRect.TopRight() );
                aPrevBoundRect.Left() += nHorDist;
            }
        }
        aPrevBoundRect.SetSize( rBoundSize );
    }
    return aPrevBoundRect.TopLeft();
}

void SvImpIconView::ResetVirtSize()
{
    StopEditTimer();
    aVirtOutputSize.Width() = 0;
    aVirtOutputSize.Height() = 0;
    BOOL bLockedEntryFound = FALSE;
    nFlags &= (~F_GRID_INSERT);
    SvLBoxEntry* pCur = pModel->FirstChild( pCurParent );
    while( pCur )
    {
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pCur);
        if( pViewData->IsEntryPosLocked() )
        {
            // VirtSize u.a. anpassen
            if( !IsBoundingRectValid( pViewData->aRect ) )
                FindBoundingRect( pCur, pViewData );
            else
                AdjustVirtSize( pViewData->aRect );
            bLockedEntryFound = TRUE;
        }
        else
            InvalidateBoundingRect( pViewData->aRect );

        pCur = pModel->NextSibling( pCur );
    }
    if( !bLockedEntryFound )
    {
        //XXX
        nFlags |= F_GRID_INSERT;
    }

    SetNextEntryPos( Point( LROFFS_WINBORDER, TBOFFS_WINBORDER ) );
    pImpCursor->Clear();
}


void SvImpIconView::AdjustVirtSize( const Rectangle& rRect )
{
    long nHeightOffs = 0;
    long nWidthOffs = 0;

    if( aVirtOutputSize.Width() < (rRect.Right()+LROFFS_WINBORDER) )
        nWidthOffs = (rRect.Right()+LROFFS_WINBORDER) - aVirtOutputSize.Width();

    if( aVirtOutputSize.Height() < (rRect.Bottom()+TBOFFS_WINBORDER) )
        nHeightOffs = (rRect.Bottom()+TBOFFS_WINBORDER) - aVirtOutputSize.Height();

    if( nWidthOffs || nHeightOffs )
    {
        Range aRange;
        aVirtOutputSize.Width() += nWidthOffs;
        aRange.Max() = aVirtOutputSize.Width();
        aHorSBar.SetRange( aRange );

        aVirtOutputSize.Height() += nHeightOffs;
        aRange.Max() = aVirtOutputSize.Height();
        aVerSBar.SetRange( aRange );

        pImpCursor->Clear();
        AdjustScrollBars();
    }
}

void SvImpIconView::Arrange()
{
    nMaxVirtWidth = aOutputSize.Width();
    ImpArrange();
}

void SvImpIconView::ImpArrange()
{
    StopEditTimer();
    ShowCursor( FALSE );
    ResetVirtSize();
    bMustRecalcBoundingRects = FALSE;
    MapMode aMapMode( pView->GetMapMode());
    aMapMode.SetOrigin( Point() );
    pView->SetMapMode( aMapMode );
    CheckAllSizes();
    RecalcAllBoundingRectsSmart();
    pView->Invalidate();
    ShowCursor( TRUE );
}

void SvImpIconView::Paint( const Rectangle& rRect )
{
    if( !pView->IsUpdateMode() )
        return;

#if defined(DBG_UTIL) && defined(OV_DRAWGRID)
    if( nFlags & F_GRIDMODE )
    {
        Color aOldColor = pView->GetLineColor();
        Color aNewColor( COL_BLACK );
        pView->SetLineColor( aNewColor );
        Point aOffs( pView->GetMapMode().GetOrigin());
        Size aXSize( pView->GetOutputSizePixel() );
        for( long nDX = nGridDX; nDX <= aXSize.Width(); nDX += nGridDX )
        {
            Point aStart( nDX+LROFFS_BOUND, 0 );
            Point aEnd( nDX+LROFFS_BOUND, aXSize.Height());
            aStart -= aOffs;
            aEnd -= aOffs;
            pView->DrawLine( aStart, aEnd );
        }
        for( long nDY = nGridDY; nDY <= aXSize.Height(); nDY += nGridDY )
        {
            Point aStart( 0, nDY+TBOFFS_BOUND );
            Point aEnd( aXSize.Width(), nDY+TBOFFS_BOUND );
            aStart -= aOffs;
            aEnd -= aOffs;
            pView->DrawLine( aStart, aEnd );
        }
        pView->SetLineColor( aOldColor );
    }
#endif
    nFlags |= F_PAINTED;

    if( !(pModel->HasChilds( pCurParent ) ))
        return;
    if( !pCursor )
        pCursor = pModel->FirstChild( pCurParent );

    USHORT nCount = pZOrderList->Count();
    if( !nCount )
        return;

    SvPtrarr* pNewZOrderList = new SvPtrarr;
    SvPtrarr* pPaintedEntries = new SvPtrarr;

    USHORT nPos = 0;
    while( nCount )
    {
        SvLBoxEntry* pEntry = (SvLBoxEntry*)(pZOrderList->GetObject(nPos ));
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        const Rectangle& rBoundRect = GetBoundingRect( pEntry, pViewData );
        if( rRect.IsOver( rBoundRect ) )
        {
            PaintEntry( pEntry, rBoundRect.TopLeft(), pViewData );
            // Eintraege, die neu gezeichnet werden, auf Top setzen
            pPaintedEntries->Insert( pEntry, pPaintedEntries->Count() );
        }
        else
            pNewZOrderList->Insert( pEntry, pNewZOrderList->Count() );

        nCount--;
        nPos++;
    }
    delete pZOrderList;
    pZOrderList = pNewZOrderList;
    nCount = pPaintedEntries->Count();
    if( nCount )
    {
        for( USHORT nCur = 0; nCur < nCount; nCur++ )
            pZOrderList->Insert( pPaintedEntries->GetObject( nCur ),pZOrderList->Count());
    }
    delete pPaintedEntries;

    Rectangle aRect;
    if( GetResizeRect( aRect ))
        PaintResizeRect( aRect );
}

BOOL SvImpIconView::GetResizeRect( Rectangle& rRect )
{
    if( aHorSBar.IsVisible() && aVerSBar.IsVisible() )
    {
        const MapMode& rMapMode = pView->GetMapMode();
        Point aOrigin( rMapMode.GetOrigin());
        aOrigin *= -1;
        aOrigin.X() += aOutputSize.Width();
        aOrigin.Y() += aOutputSize.Height();
        rRect.SetPos( aOrigin );
        rRect.SetSize( Size( nVerSBarWidth, nHorSBarHeight));
        return TRUE;
    }
    return FALSE;
}

void SvImpIconView::PaintResizeRect( const Rectangle& rRect )
{
    const StyleSettings& rStyleSettings = pView->GetSettings().GetStyleSettings();
    Color aNewColor = rStyleSettings.GetFaceColor();
    Color aOldColor = pView->GetFillColor();
    pView->SetFillColor( aNewColor );
    pView->DrawRect( rRect );
    pView->SetFillColor( aOldColor );
}

void SvImpIconView::RepaintSelectionItems()
{
    DBG_ERROR("RepaintSelectionItems");
    pView->Invalidate(); // vorlaeufig
}

SvLBoxItem* SvImpIconView::GetItem( SvLBoxEntry* pEntry,
                    const Point& rAbsPos )
{
    Rectangle aRect;
    SvLBoxString* pStringItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if( pStringItem )
    {
        aRect = CalcTextRect( pEntry, pStringItem );
        if( aRect.IsInside( rAbsPos ) )
            return pStringItem;
    }
    SvLBoxContextBmp* pBmpItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if( pBmpItem )
    {
        aRect = CalcBmpRect( pEntry );
        if( aRect.IsInside( rAbsPos ) )
            return pBmpItem;
    }
    return 0;
}

void SvImpIconView::CalcDocPos( Point& aMaeuschenPos )
{
    aMaeuschenPos -= pView->GetMapMode().GetOrigin();
}

void SvImpIconView::MouseButtonDown( const MouseEvent& rMEvt)
{
    StopEditTimer();
    pView->GrabFocus();
    Point aDocPos( rMEvt.GetPosPixel() );
    if(aDocPos.X()>=aOutputSize.Width() || aDocPos.Y()>=aOutputSize.Height())
        return;
    CalcDocPos( aDocPos );
    SvLBoxEntry* pEntry = GetEntry( aDocPos );
    if( !pEntry )
    {
        if( pView->GetSelectionMode() != SINGLE_SELECTION )
        {
            if( !rMEvt.IsMod1() )  // Ctrl
            {
                pView->SelectAll( FALSE );
                ClearSelectedRectList();
            }
            else
                nFlags |= F_ADD_MODE;
            nFlags |= F_RUBBERING;
            aCurSelectionRect.SetPos( aDocPos );
            pView->CaptureMouse();
        }
        return;
    }

    BOOL bSelected = pView->IsSelected( pEntry );
    BOOL bEditingEnabled = pView->IsInplaceEditingEnabled();

    if( rMEvt.GetClicks() == 2 )
    {
        DeselectAllBut( pEntry );
        pView->pHdlEntry = pEntry;
        pView->DoubleClickHdl();
    }
    else
    {
        // Inplace-Editing ?
        if( rMEvt.IsMod2() )  // Alt?
        {
            if( bEditingEnabled )
            {
                SvLBoxItem* pItem = GetItem(pEntry,aDocPos);
                if( pItem )
                    pView->EditingRequest( pEntry, pItem, aDocPos);
            }
        }
        else if( pView->GetSelectionMode() == SINGLE_SELECTION )
        {
            DeselectAllBut( pEntry );
            SetCursor( pEntry );
            pView->Select( pEntry, TRUE );
            if( bEditingEnabled && bSelected && !rMEvt.GetModifier() &&
                rMEvt.IsLeft() && IsTextHit( pEntry, aDocPos ) )
            {
                nFlags |= F_START_EDITTIMER_IN_MOUSEUP;
            }
        }
        else
        {
            if( !rMEvt.GetModifier() )
            {
                if( !bSelected )
                {
                    DeselectAllBut( pEntry );
                    SetCursor( pEntry );
                    pView->Select( pEntry, TRUE );
                }
                else
                {
                    // erst im Up deselektieren, falls Move per D&D!
                    nFlags |= F_DOWN_DESELECT;
                    if( bEditingEnabled && IsTextHit( pEntry, aDocPos ) &&
                        rMEvt.IsLeft())
                    {
                        nFlags |= F_START_EDITTIMER_IN_MOUSEUP;
                    }
                }
            }
            else if( rMEvt.IsMod1() )
                nFlags |= F_DOWN_CTRL;
        }
    }
}

void SvImpIconView::MouseButtonUp( const MouseEvent& rMEvt )
{
    aMouseMoveTimer.Stop();
    pView->ReleaseMouse();
    // HACK, da Einar noch nicht PrepareCommandEvent aufruft
    if( rMEvt.IsRight() && (nFlags & (F_DOWN_CTRL | F_DOWN_DESELECT) ))
        nFlags &= ~(F_DOWN_CTRL | F_DOWN_DESELECT);

    if( nFlags & F_RUBBERING )
    {
        aMouseMoveTimer.Stop();
        AddSelectedRect( aCurSelectionRect );
        HideSelectionRect();
        nFlags &= ~(F_RUBBERING | F_ADD_MODE);
    }

    SvLBoxEntry* pEntry = pView->GetEntry( rMEvt.GetPosPixel(), TRUE );
    if( pEntry )
    {
        if( nFlags & F_DOWN_CTRL )
        {
            // Ctrl & MultiSelection
            ToggleSelection( pEntry );
            SetCursor( pEntry );
        }
        else if( nFlags & F_DOWN_DESELECT )
        {
            DeselectAllBut( pEntry );
            SetCursor( pEntry );
            pView->Select( pEntry, TRUE );
        }
    }

    nFlags &= ~(F_DOWN_CTRL | F_DOWN_DESELECT);
    if( nFlags & F_START_EDITTIMER_IN_MOUSEUP )
    {
        StartEditTimer();
        nFlags &= ~F_START_EDITTIMER_IN_MOUSEUP;
    }
}

void SvImpIconView::MouseMove( const MouseEvent& rMEvt )
{
    if( nFlags & F_RUBBERING )
    {
        const Point& rPosPixel = rMEvt.GetPosPixel();
        if( !aMouseMoveTimer.IsActive() )
        {
            aMouseMoveEvent = rMEvt;
            aMouseMoveTimer.Start();
            // ausserhalb des Fensters liegende Move-Events muessen
            // vom Timer kommen, damit die Scrollgeschwindigkeit
            // unabhaengig von Mausbewegungen ist.
            if( rPosPixel.X() < 0 || rPosPixel.Y() < 0 )
                return;
            const Size& rSize = pView->GetOutputSizePixel();
            if( rPosPixel.X() > rSize.Width() || rPosPixel.Y() > rSize.Height())
                return;
        }

        if( &rMEvt != &aMouseMoveEvent )
            aMouseMoveEvent = rMEvt;

        long nScrollDX, nScrollDY;

        CalcScrollOffsets(rMEvt.GetPosPixel(),nScrollDX,nScrollDY,FALSE );
        BOOL bSelRectHidden = FALSE;
        if( nScrollDX || nScrollDY )
        {
            HideSelectionRect();
            bSelRectHidden = TRUE;
            pView->Scroll( nScrollDX, nScrollDY );
        }
        Point aDocPos( rMEvt.GetPosPixel() );
        aDocPos = pView->PixelToLogic( aDocPos );
        Rectangle aRect( aCurSelectionRect.TopLeft(), aDocPos );
        if( aRect != aCurSelectionRect )
        {
            HideSelectionRect();
            bSelRectHidden = TRUE;
            BOOL bAdd = (nFlags & F_ADD_MODE) ? TRUE : FALSE;
            SelectRect( aRect, bAdd, &aSelectedRectList );
        }
        if( bSelRectHidden )
            DrawSelectionRect( aRect );
    }
}

BOOL SvImpIconView::KeyInput( const KeyEvent& rKEvt )
{
    StopEditTimer();
    BOOL bKeyUsed = TRUE;
    BOOL bMod1 = rKEvt.GetKeyCode().IsMod1();
    BOOL bInAddMode = (BOOL)((nFlags & F_ADD_MODE) != 0);
    int bDeselAll = (pView->GetSelectionMode() != SINGLE_SELECTION) &&
                    !bInAddMode;
    SvLBoxEntry* pNewCursor;
    USHORT nCode = rKEvt.GetKeyCode().GetCode();
    switch( nCode )
    {
        case KEY_UP:
            if( pCursor )
            {
                MakeVisible( pCursor );
                pNewCursor = pImpCursor->GoUpDown(pCursor,FALSE);
                if( pNewCursor )
                {
                    if( bDeselAll )
                        pView->SelectAll( FALSE );
                    ShowCursor( FALSE );
                    MakeVisible( pNewCursor );
                    SetCursor( pNewCursor );
                    if( !bInAddMode )
                        pView->Select( pCursor, TRUE );
                }
                else
                {
                    Rectangle aRect( GetBoundingRect( pCursor ) );
                    if( aRect.Top())
                    {
                        aRect.Bottom() -= aRect.Top();
                        aRect.Top() = 0;
                        MakeVisible( aRect );
                    }
                }
            }
            break;

        case KEY_DOWN:
            if( pCursor )
            {
                pNewCursor=pImpCursor->GoUpDown( pCursor,TRUE );
                if( pNewCursor )
                {
                    MakeVisible( pCursor );
                    if( bDeselAll )
                        pView->SelectAll( FALSE );
                    ShowCursor( FALSE );
                    MakeVisible( pNewCursor );
                    SetCursor( pNewCursor );
                    if( !bInAddMode )
                        pView->Select( pCursor, TRUE );
                }
            }
            break;

        case KEY_RIGHT:
            if( pCursor )
            {
                pNewCursor=pImpCursor->GoLeftRight(pCursor,TRUE );
                if( pNewCursor )
                {
                    MakeVisible( pCursor );
                    if( bDeselAll )
                        pView->SelectAll( FALSE );
                    ShowCursor( FALSE );
                    MakeVisible( pNewCursor );
                    SetCursor( pNewCursor );
                    if( !bInAddMode )
                        pView->Select( pCursor, TRUE );
                }
            }
            break;

        case KEY_LEFT:
            if( pCursor )
            {
                MakeVisible( pCursor );
                pNewCursor = pImpCursor->GoLeftRight(pCursor,FALSE );
                if( pNewCursor )
                {
                    if( bDeselAll )
                        pView->SelectAll( FALSE );
                    ShowCursor( FALSE );
                    MakeVisible( pNewCursor );
                    SetCursor( pNewCursor );
                    if( !bInAddMode )
                        pView->Select( pCursor, TRUE );
                }
                else
                {
                    Rectangle aRect( GetBoundingRect(pCursor));
                    if( aRect.Left() )
                    {
                        aRect.Right() -= aRect.Left();
                        aRect.Left() = 0;
                        MakeVisible( aRect );
                    }
                }
            }
            break;

        case KEY_ESCAPE:
            if( nFlags & F_RUBBERING )
            {
                HideSelectionRect();
                pView->SelectAll( FALSE );
                nFlags &= ~F_RUBBERING;
            }
            break;

        case KEY_F8:
            if( rKEvt.GetKeyCode().IsShift() )
            {
                if( nFlags & F_ADD_MODE )
                    nFlags &= (~F_ADD_MODE);
                else
                    nFlags |= F_ADD_MODE;
            }
            break;

#ifdef OS2
        case KEY_F9:
            if( rKEvt.GetKeyCode().IsShift() )
            {
                if( pCursor && pView->IsInplaceEditingEnabled() )
                    pView->EditEntry( pCursor );
            }
            break;
#endif

        case KEY_SPACE:
            if( pCursor )
            {
                ToggleSelection( pCursor );
            }
            break;


        case KEY_PAGEDOWN:
            break;
        case KEY_PAGEUP:
            break;

        case KEY_ADD:
        case KEY_DIVIDE :
            if( bMod1 )
                pView->SelectAll( TRUE );
            break;

        case KEY_SUBTRACT:
        case KEY_COMMA :
            if( bMod1 )
                pView->SelectAll( FALSE );
            break;

        case KEY_RETURN:
            if( bMod1 )
            {
                if( pCursor && pView->IsInplaceEditingEnabled() )
                    pView->EditEntry( pCursor );
            }
            break;

        default:
            bKeyUsed = FALSE;

    }
    return bKeyUsed;
}


void SvImpIconView::PositionScrollBars( long nRealWidth, long nRealHeight )
{
    // hor scrollbar
    Point aPos( 0, nRealHeight );
    aPos.Y() -= nHorSBarHeight;

#ifdef WIN
    // vom linken und unteren Rand ein Pixel abschneiden
    aPos.Y()++;
    aPos.X()--;
#endif
#ifdef OS2
    aPos.Y()++;
#endif
    if( aHorSBar.GetPosPixel() != aPos )
        aHorSBar.SetPosPixel( aPos );

    // ver scrollbar
    aPos.X() = nRealWidth; aPos.Y() = 0;
    aPos.X() -= nVerSBarWidth;

#if defined(WIN) || defined(WNT)
    aPos.X()++;
    aPos.Y()--;
#endif

#ifdef OS2
    aPos.Y()--;
    aPos.X()++;
#endif

#ifdef MAC
    aPos.Y()--;
    aPos.X()++;
#endif
    if( aVerSBar.GetPosPixel() != aPos )
        aVerSBar.SetPosPixel( aPos );
}



void SvImpIconView::AdjustScrollBars( BOOL bVirtSizeGrowedOnly )
{
    long nVirtHeight = aVirtOutputSize.Height();
    long nVirtWidth = aVirtOutputSize.Width();

    Size aOSize( pView->Control::GetOutputSizePixel() );
    long nRealHeight = aOSize.Height();
    long nRealWidth = aOSize.Width();

    PositionScrollBars( nRealWidth, nRealHeight );

    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );

    long nVisibleWidth;
    if( nRealWidth > nVirtWidth )
        nVisibleWidth = nVirtWidth + aOrigin.X();
    else
        nVisibleWidth = nRealWidth;

    long nVisibleHeight;
    if( nRealHeight > nVirtHeight )
        nVisibleHeight = nVirtHeight + aOrigin.Y();
    else
        nVisibleHeight = nRealHeight;

    int bVerSBar = pView->nWindowStyle & WB_VSCROLL;
    int bHorSBar = pView->nWindowStyle & WB_HSCROLL;

    USHORT nResult = 0;
    if( nVirtHeight )
    {
        // activate ver scrollbar ?
        if( bVerSBar || ( nVirtHeight > nVisibleHeight) )
        {
            nResult = 0x0001;
            nRealWidth -= nVerSBarWidth;

            if( nRealWidth > nVirtWidth )
                nVisibleWidth = nVirtWidth + aOrigin.X();
            else
                nVisibleWidth = nRealWidth;

            nFlags |= F_HOR_SBARSIZE_WITH_VBAR;
        }
        // activate hor scrollbar ?
        if( bHorSBar || (nVirtWidth > nVisibleWidth) )
        {
            nResult |= 0x0002;
            nRealHeight -= nHorSBarHeight;

            if( nRealHeight > nVirtHeight )
                nVisibleHeight = nVirtHeight + aOrigin.Y();
            else
                nVisibleHeight = nRealHeight;

            // brauchen wir jetzt doch eine senkrechte Scrollbar ?
            if( !(nResult & 0x0001) &&  // nur wenn nicht schon da
                ( (nVirtHeight > nVisibleHeight) || bVerSBar) )
            {
                nResult = 3; // both are active
                nRealWidth -= nVerSBarWidth;

                if( nRealWidth > nVirtWidth )
                    nVisibleWidth = nVirtWidth + aOrigin.X();
                else
                    nVisibleWidth = nRealWidth;

                nFlags |= F_VER_SBARSIZE_WITH_HBAR;
            }
        }
    }

    // size ver scrollbar
    long nThumb = aVerSBar.GetThumbPos();
    Size aSize( nVerSBarWidth, nRealHeight );
#if defined(WIN) || defined(WNT)
    aSize.Height() += 2;
#endif
#ifdef OS2
    aSize.Height() += 3;
#endif
#ifdef MAC
    aSize.Height() += 2;
#endif
    if( aSize != aVerSBar.GetSizePixel() )
        aVerSBar.SetSizePixel( aSize );
    aVerSBar.SetVisibleSize( nVisibleHeight );
    aVerSBar.SetPageSize( (nVisibleHeight*75)/100 );
    if( nResult & 0x0001 )
    {
        aVerSBar.SetThumbPos( nThumb );
        aVerSBar.Show();
    }
    else
    {
        aVerSBar.SetThumbPos( 0 );
        aVerSBar.Hide();
    }

    // size hor scrollbar
    nThumb = aHorSBar.GetThumbPos();
    aSize.Width() = nRealWidth;
    aSize.Height() = nHorSBarHeight;
#if defined(WIN) || defined(WNT)
    aSize.Width()++;
#endif
#ifdef OS2
    aSize.Width() += 3;
    if( nResult & 0x0001 ) // vertikale Scrollbar ?
        aSize.Width()--;
#endif
#if defined(WIN) || defined(WNT)
    if( nResult & 0x0001 ) // vertikale Scrollbar ?
    {
        aSize.Width()++;
        nRealWidth++;
    }
#endif
    if( aSize != aHorSBar.GetSizePixel() )
        aHorSBar.SetSizePixel( aSize );
    aHorSBar.SetVisibleSize( nVisibleWidth ); //nRealWidth );
    aHorSBar.SetPageSize( (nVisibleWidth*75)/100 );
    if( nResult & 0x0002 )
    {
        aHorSBar.SetThumbPos( nThumb );
        aHorSBar.Show();
    }
    else
    {
        aHorSBar.SetThumbPos( 0 );
        aHorSBar.Hide();
    }

#ifdef OS2
    nRealWidth++;
#endif
    aOutputSize.Width() = nRealWidth;
#if defined(WIN) || defined(WNT)
    if( nResult & 0x0002 ) // hor scrollbar ?
        nRealHeight++; // weil unterer Rand geclippt wird
#endif
#ifdef OS2
    if( nResult & 0x0002 ) // hor scrollbar ?
        nRealHeight++;
#endif
    aOutputSize.Height() = nRealHeight;
}

void __EXPORT SvImpIconView::Resize()
{
    StopEditTimer();
    Rectangle aRect;
    if( GetResizeRect(aRect) )
        pView->Invalidate( aRect );
    aOutputSize = pView->GetOutputSizePixel();
    pImpCursor->Clear();

#if 1
    const Size& rSize = pView->Control::GetOutputSizePixel();
    PositionScrollBars( rSize.Width(), rSize.Height() );
    // Die ScrollBars werden asynchron ein/ausgeblendet, damit abgeleitete
    // Klassen im Resize ein Arrange durchfuehren koennen, ohne dass
    // die ScrollBars aufblitzen (SfxExplorerIconView!)
    nCurUserEvent = Application::PostUserEvent(LINK(this,SvImpIconView,UserEventHdl),0);
#else
    AdjustScrollBars();
    if( GetResizeRect(aRect) )
        PaintResizeRect( aRect );
#endif
}

BOOL SvImpIconView::CheckHorScrollBar()
{
    if( !pZOrderList || !aHorSBar.IsVisible() )
        return FALSE;
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    if(!(pView->nWindowStyle & WB_HSCROLL) && !aOrigin.X() )
    {
        long nWidth = aOutputSize.Width();
        USHORT nCount = pZOrderList->Count();
        long nMostRight = 0;
        for( USHORT nCur = 0; nCur < nCount; nCur++ )
        {
            SvLBoxEntry* pEntry = (SvLBoxEntry*)pZOrderList->operator[](nCur);
            long nRight = GetBoundingRect(pEntry).Right();
            if( nRight > nWidth )
                return FALSE;
            if( nRight > nMostRight )
                nMostRight = nRight;
        }
        aHorSBar.Hide();
        aOutputSize.Height() += nHorSBarHeight;
        aVirtOutputSize.Width() = nMostRight;
        aHorSBar.SetThumbPos( 0 );
        Range aRange;
        aRange.Max() = nMostRight - 1;
        aHorSBar.SetRange( aRange  );
        if( aVerSBar.IsVisible() )
        {
            Size aSize( aVerSBar.GetSizePixel());
            aSize.Height() += nHorSBarHeight;
            aVerSBar.SetSizePixel( aSize );
        }
        return TRUE;
    }
    return FALSE;
}

BOOL SvImpIconView::CheckVerScrollBar()
{
    if( !pZOrderList || !aVerSBar.IsVisible() )
        return FALSE;
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    if(!(pView->nWindowStyle & WB_VSCROLL) && !aOrigin.Y() )
    {
        long nDeepest = 0;
        long nHeight = aOutputSize.Height();
        USHORT nCount = pZOrderList->Count();
        for( USHORT nCur = 0; nCur < nCount; nCur++ )
        {
            SvLBoxEntry* pEntry = (SvLBoxEntry*)pZOrderList->operator[](nCur);
            long nBottom = GetBoundingRect(pEntry).Bottom();
            if( nBottom > nHeight )
                return FALSE;
            if( nBottom > nDeepest )
                nDeepest = nBottom;
        }
        aVerSBar.Hide();
        aOutputSize.Width() += nVerSBarWidth;
        aVirtOutputSize.Height() = nDeepest;
        aVerSBar.SetThumbPos( 0 );
        Range aRange;
        aRange.Max() = nDeepest - 1;
        aVerSBar.SetRange( aRange  );
        if( aHorSBar.IsVisible() )
        {
            Size aSize( aHorSBar.GetSizePixel());
            aSize.Width() += nVerSBarWidth;
            aHorSBar.SetSizePixel( aSize );
        }
        return TRUE;
    }
    return FALSE;
}


// blendet Scrollbars aus, wenn sie nicht mehr benoetigt werden
void SvImpIconView::CheckScrollBars()
{
    CheckVerScrollBar();
    if( CheckHorScrollBar() )
        CheckVerScrollBar();
}


void __EXPORT SvImpIconView::GetFocus()
{
    if( pCursor )
    {
        pView->SetEntryFocus( pCursor, TRUE );
        ShowCursor( TRUE );
    }
}

void __EXPORT SvImpIconView::LoseFocus()
{
    StopEditTimer();
    if( pCursor )
        pView->SetEntryFocus( pCursor,FALSE );
    ShowCursor( FALSE );
}

void SvImpIconView::UpdateAll()
{
    AdjustScrollBars();
    pImpCursor->Clear();
    pView->Invalidate();
}

void SvImpIconView::PaintEntry( SvLBoxEntry* pEntry, SvIcnVwDataEntry* pViewData )
{
    Point aPos( GetEntryPos( pEntry ) );
    PaintEntry( pEntry, aPos, pViewData );
}

void SvImpIconView::PaintEmphasis( const Rectangle& rRect, BOOL bSelected,
                                   BOOL bInUse, BOOL bCursored,
                                   OutputDevice* pOut )
{
    // HACK fuer D&D
    if( nFlags & F_NO_EMPHASIS )
        return;

    if( !pOut )
        pOut = pView;

    // Selektion painten
    Color aOldFillColor =  pOut->GetFillColor();
    Color aOldLineColor =  pOut->GetLineColor();
    Color aNewColor;
    const StyleSettings& rStyleSettings = pOut->GetSettings().GetStyleSettings();
    if( bSelected )
    {
        aNewColor = rStyleSettings.GetHighlightColor();
    }
    else
    {
#ifndef OS2
        aNewColor =rStyleSettings.GetFieldColor();
#else
        aNewColor = pOut->GetBackground().GetColor();
#endif
    }

    if( bCursored )
    {
        pOut->SetLineColor( Color( COL_BLACK ) );
    }
    pOut->SetFillColor( aNewColor );
    pOut->DrawRect( rRect );
    pOut->SetFillColor( aOldFillColor );
    pOut->SetLineColor( aOldLineColor );
}

void SvImpIconView::PaintItem( const Rectangle& rRect,
    SvLBoxItem* pItem, SvLBoxEntry* pEntry, USHORT nPaintFlags,
    OutputDevice* pOut )
{
    if( nViewMode == VIEWMODE_ICON && pItem->IsA() == SV_ITEM_ID_LBOXSTRING )
    {
        const String& rStr = ((SvLBoxString*)pItem)->GetText();
        DrawText( pOut, rRect, rStr, DRAWTEXT_FLAGS );
    }
    else
    {
        Point aPos( rRect.TopLeft() );
        const Size& rSize = GetItemSize( pView, pEntry, pItem );
        if( nPaintFlags & PAINTFLAG_HOR_CENTERED )
            aPos.X() += (rRect.GetWidth() - rSize.Width() ) / 2;
        if( nPaintFlags & PAINTFLAG_VER_CENTERED )
            aPos.Y() += (rRect.GetHeight() - rSize.Height() ) / 2;
        pItem->Paint( aPos, *(SvLBox*)pOut, 0, pEntry );
    }
}

void SvImpIconView::PaintEntry( SvLBoxEntry* pEntry, const Point& rPos,
    SvIcnVwDataEntry* pViewData, OutputDevice* pOut )
{
    if( !pView->IsUpdateMode() )
        return;

    if( !pOut )
        pOut = pView;

    SvLBoxContextBmp* pBmpItem;

    pView->PreparePaint( pEntry );

    if( !pViewData )
        pViewData = ICNVIEWDATA(pEntry);

    SvLBoxString* pStringItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));

    BOOL bSelected  = pViewData->IsSelected();
    BOOL bCursored  = pViewData->IsCursored();
    BOOL bInUse     = pEntry->HasInUseEmphasis();

    Font aTempFont( pOut->GetFont() );
    // waehrend D&D nicht die Fontfarbe wechseln, da sonst auch die
    // Emphasis gezeichnet werden muss! (weisser Adler auf weissem Grund)
    if( bSelected && !(nFlags & F_NO_EMPHASIS) )
    {
        const StyleSettings& rStyleSettings = pOut->GetSettings().GetStyleSettings();
        Font aNewFont( aTempFont );
        aNewFont.SetColor( rStyleSettings.GetHighlightTextColor() );
        pOut->SetFont( aNewFont );
    }
    Rectangle aTextRect( CalcTextRect(pEntry,pStringItem,&rPos,FALSE,pViewData));
    Rectangle aBmpRect( CalcBmpRect(pEntry, &rPos, pViewData ) );

    switch( nViewMode )
    {
        case VIEWMODE_ICON:
            pBmpItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
            PaintEmphasis( aBmpRect, bSelected, bInUse, bCursored, pOut );
            PaintItem( aBmpRect, pBmpItem, pEntry,
                PAINTFLAG_HOR_CENTERED | PAINTFLAG_VER_CENTERED, pOut );
            PaintEmphasis( aTextRect, bSelected, FALSE, FALSE, pOut );
            PaintItem( aTextRect, pStringItem, pEntry, PAINTFLAG_HOR_CENTERED, pOut );
            break;

        case VIEWMODE_NAME:
            pBmpItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
            PaintEmphasis( aBmpRect, bSelected, bInUse, bCursored, pOut );
            PaintItem( aBmpRect, pBmpItem, pEntry, PAINTFLAG_VER_CENTERED, pOut );
            PaintEmphasis( aTextRect, bSelected, FALSE, FALSE, pOut );
            PaintItem( aTextRect, pStringItem, pEntry,PAINTFLAG_VER_CENTERED, pOut );
            break;

        case VIEWMODE_TEXT:
            PaintEmphasis( aTextRect, bSelected, FALSE, bCursored, pOut );
            PaintItem( aTextRect, pStringItem, pEntry, PAINTFLAG_VER_CENTERED, pOut );
            break;
    }
    pOut->SetFont( aTempFont );
}

void SvImpIconView::SetEntryPos( SvLBoxEntry* pEntry, const Point& rPos,
    BOOL bAdjustAtGrid, BOOL bCheckScrollBars )
{
    if( pModel->GetParent(pEntry) == pCurParent )
    {
        ShowCursor( FALSE );
        long nVirtHeightOffs = 0;
        long nVirtWidthOffs = 0;
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        Rectangle aBoundRect( GetBoundingRect( pEntry, pViewData ));
        pView->Invalidate( aBoundRect );
        ToTop( pEntry );
        if( rPos != aBoundRect.TopLeft() )
        {
            Point aGridOffs = pViewData->aGridRect.TopLeft() -
                              pViewData->aRect.TopLeft();
            pImpCursor->Clear();
            nFlags &= ~F_GRID_INSERT;
            aBoundRect.SetPos( rPos );
            pViewData->aRect = aBoundRect;
            pViewData->aGridRect.SetPos( rPos + aGridOffs );
            AdjustVirtSize( aBoundRect );
        }
        //HACK(Billigloesung, die noch verbessert werden muss)
        if( bAdjustAtGrid )
        {
            AdjustAtGrid( pEntry );
            ToTop( pEntry );
        }
        if( bCheckScrollBars && pView->IsUpdateMode() )
            CheckScrollBars();

        PaintEntry( pEntry, pViewData );
        ShowCursor( TRUE );
    }
}

void SvImpIconView::ViewDataInitialized( SvLBoxEntry*)
{
}

void SvImpIconView::ModelHasEntryInvalidated( SvListEntry* pEntry )
{
    if( pEntry == pCursor )
        ShowCursor( FALSE );
    SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
    pView->Invalidate( pViewData->aRect );

    if( nFlags & F_GRIDMODE )
        Center( (SvLBoxEntry*)pEntry, pViewData );
    else
        pViewData->aRect.SetSize( CalcBoundingSize(
            (SvLBoxEntry*)pEntry, pViewData ) );

    ViewDataInitialized( (SvLBoxEntry*)pEntry );
    pView->Invalidate( pViewData->aRect );
    if( pEntry == pCursor )
        ShowCursor( TRUE );
}


void SvImpIconView::InvalidateEntry( SvLBoxEntry* pEntry )
{
    const Rectangle& rRect = GetBoundingRect( pEntry );
    pView->Invalidate( rRect );
}

void SvImpIconView::SetNoSelection()
{
}

void SvImpIconView::SetDragDropMode( DragDropMode )
{
}

void SvImpIconView::SetSelectionMode( SelectionMode )
{
}

BOOL SvImpIconView::IsEntryInView( SvLBoxEntry* )
{
    return FALSE;
}

SvLBoxEntry* SvImpIconView::GetDropTarget( const Point& rPos )
{
    Point aDocPos( rPos );
    CalcDocPos( aDocPos );
    SvLBoxEntry* pTarget = GetEntry( aDocPos );
    if( !pTarget || !pTarget->HasChilds() )
        pTarget = pCurParent;
    return pTarget;
}

SvLBoxEntry* SvImpIconView::GetEntry( const Point& rDocPos )
{
    CheckBoundingRects();
    SvLBoxEntry* pTarget = 0;
    // Z-Order-Liste vom Ende her absuchen
    USHORT nCount = pZOrderList->Count();
    while( nCount )
    {
        nCount--;
        SvLBoxEntry* pEntry = (SvLBoxEntry*)(pZOrderList->GetObject(nCount));
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        if( pViewData->aRect.IsInside( rDocPos ) )
        {
            pTarget = pEntry;
            break;
        }
    }
    return pTarget;
}

SvLBoxEntry* SvImpIconView::GetNextEntry( const Point& rDocPos, SvLBoxEntry* pCurEntry )
{
    CheckBoundingRects();
    SvLBoxEntry* pTarget = 0;
    USHORT nStartPos = pZOrderList->GetPos( (void*)pCurEntry );
    if( nStartPos != USHRT_MAX )
    {
        USHORT nCount = pZOrderList->Count();
        for( USHORT nCur = nStartPos+1; nCur < nCount; nCur++ )
        {
            SvLBoxEntry* pEntry = (SvLBoxEntry*)(pZOrderList->GetObject(nCur));
            SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
            if( pViewData->aRect.IsInside( rDocPos ) )
            {
                pTarget = pEntry;
                break;
            }
        }
    }
    return pTarget;
}

SvLBoxEntry* SvImpIconView::GetPrevEntry( const Point& rDocPos, SvLBoxEntry* pCurEntry )
{
    CheckBoundingRects();
    SvLBoxEntry* pTarget = 0;
    USHORT nStartPos = pZOrderList->GetPos( (void*)pCurEntry );
    if( nStartPos != USHRT_MAX && nStartPos != 0 )
    {
        nStartPos--;
        do
        {
            SvLBoxEntry* pEntry = (SvLBoxEntry*)(pZOrderList->GetObject(nStartPos));
            SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
            if( pViewData->aRect.IsInside( rDocPos ) )
            {
                pTarget = pEntry;
                break;
            }
        } while( nStartPos > 0 );
    }
    return pTarget;
}


Point SvImpIconView::GetEntryPos( SvLBoxEntry* pEntry )
{
    SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
    DBG_ASSERT(pViewData,"Entry not in model")
    return pViewData->aRect.TopLeft();
}

const Rectangle& SvImpIconView::GetBoundingRect( SvLBoxEntry* pEntry, SvIcnVwDataEntry* pViewData )
{
    if( !pViewData )
        pViewData = ICNVIEWDATA(pEntry);
    DBG_ASSERT(pViewData,"Entry not in model")
    if( !IsBoundingRectValid( pViewData->aRect ))
        FindBoundingRect( pEntry, pViewData );
    return pViewData->aRect;
}

void SvImpIconView::SetSpaceBetweenEntries( long nHor, long nVer )
{
    nHorDist = nHor;
    nVerDist = nVer;
}

Rectangle SvImpIconView::CalcBmpRect( SvLBoxEntry* pEntry, const Point* pPos,
    SvIcnVwDataEntry* pViewData  )
{
    if( !pViewData )
        pViewData = ICNVIEWDATA(pEntry);

    Rectangle aBound = GetBoundingRect( pEntry, pViewData );
    if( pPos )
        aBound.SetPos( *pPos );
    Point aPos( aBound.TopLeft() );

    switch( nViewMode )
    {
        case VIEWMODE_ICON:
        {
            aPos.X() += ( aBound.GetWidth() - nMaxBmpWidth ) / 2;
            Size aSize( nMaxBmpWidth, nMaxBmpHeight );
            // das Bitmap-Rechteck soll nicht das TextRect beruehren
            aSize.Height() -= 3;
            return Rectangle( aPos, aSize );
        }

        case VIEWMODE_NAME:
            return Rectangle( aPos,
                Size( nMaxBmpWidth, aBound.GetHeight() ));

        case VIEWMODE_TEXT:
            return Rectangle( aPos, aBound.GetSize() );

        default:
        {
            Rectangle aRect;
            return aRect;
        }
    }
}

Rectangle SvImpIconView::CalcTextRect( SvLBoxEntry* pEntry,
    SvLBoxString* pItem, const Point* pPos, BOOL bForInplaceEdit,
    SvIcnVwDataEntry* pViewData )
{
    long nBmpHeight, nBmpWidth;

    if( !pItem )
        pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));

    if( !pViewData )
        pViewData = ICNVIEWDATA(pEntry);

    Size aTextSize( GetItemSize( pView, pEntry, pItem, pViewData ));
    aTextSize.Width() += 2*LROFFS_TEXT;

    Size aContextBmpSize(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP)->GetSize(pView,pEntry));
    Rectangle aBound = GetBoundingRect( pEntry, pViewData );
    if( pPos )
        aBound.SetPos( *pPos );
    Point aPos( aBound.TopLeft() );

    switch( nViewMode )
    {
        case VIEWMODE_ICON:
            nBmpHeight = aContextBmpSize.Height();
            if( nBmpHeight < nMaxBmpHeight )
                nBmpHeight = nMaxBmpHeight;
            aPos.Y() += nBmpHeight;

            // beim Inplace-Editieren, spendieren wir ein bisschen mehr Platz
            if( bForInplaceEdit )
            {
                // 20% rauf
                long nMinWidth = (( (aContextBmpSize.Width()*10) / 100 ) * 2 ) +
                                 aContextBmpSize.Width();
                if( nMinWidth > aBound.GetWidth() )
                    nMinWidth = aBound.GetWidth();

                if( aTextSize.Width() < nMinWidth )
                    aTextSize.Width() = nMinWidth;

                // beim Inplace-Ed. darfs auch untere Eintraege ueberlappen
                Rectangle aMaxGridTextRect = CalcMaxTextRect(pEntry, pViewData);
#ifndef VCL
                aMaxGridTextRect.Bottom() = LONG_MAX - 1;
                aMaxGridTextRect = GetTextRect( pView, aMaxGridTextRect,pItem->GetText(), DRAWTEXT_FLAGS );
#endif
                Size aOptSize = aMaxGridTextRect.GetSize();
                if( aOptSize.Height() > aTextSize.Height() )
                    aTextSize.Height() = aOptSize.Height();
            }


            aPos.X() += ( aBound.GetWidth() - aTextSize.Width() ) / 2;
            break;

        case VIEWMODE_NAME:
            nBmpWidth = aContextBmpSize.Width();
            if( nBmpWidth < nMaxBmpWidth )
                nBmpWidth = nMaxBmpWidth;
            aPos.X() += nBmpWidth;
            // vertikal ausrichten
            aPos.Y() += ( nBmpWidth - aTextSize.Height() ) / 2;
            break;
    }

    Rectangle aRect( aPos, aTextSize );
// KNALLT BEIM D&D, WENN GECLIPPT WIRD (In DrawText von Thomas)
//  ClipAtVirtOutRect( aRect );
    return aRect;
}


long SvImpIconView::CalcBoundingWidth( SvLBoxEntry* pEntry,
    const SvIcnVwDataEntry* pViewData ) const
{
    DBG_ASSERT(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP),"No Bitmaps")
    DBG_ASSERT(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING),"No Text")
    long nStringWidth = GetItemSize( pView, pEntry, pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING),pViewData).Width();
    nStringWidth += 2*LROFFS_TEXT;
    long nBmpWidth = pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP)->GetSize(pView,pEntry).Width();
    long nWidth = 0;

    switch( nViewMode )
    {
        case VIEWMODE_ICON:
            nWidth = Max( nStringWidth, nBmpWidth );
            nWidth = Max( nWidth, nMaxBmpWidth );
            break;

        case VIEWMODE_NAME:
            nWidth = Max( nBmpWidth, nMaxBmpWidth );
            nWidth += NAMEVIEW_OFFS_BMP_STRING;  // Abstand Bitmap String
            nWidth += nStringWidth;
            break;

        case VIEWMODE_TEXT:
            nWidth = nStringWidth;
            break;
    }
    return nWidth;
}

long SvImpIconView::CalcBoundingHeight( SvLBoxEntry* pEntry,
    const SvIcnVwDataEntry* pViewData ) const
{
    DBG_ASSERT(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP),"No Bitmaps")
    DBG_ASSERT(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING),"No Text")
    long nStringHeight = GetItemSize(pView,pEntry,pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING),pViewData).Height();
    long nBmpHeight = pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP)->GetSize(pView,pEntry).Height();
    long nHeight = 0;

    switch( nViewMode )
    {
        case VIEWMODE_ICON:
            nHeight = Max( nBmpHeight, nMaxBmpHeight );
            nHeight += ICONVIEW_OFFS_BMP_STRING; // Abstand Bitmap String
            nHeight += nStringHeight;
            break;

        case VIEWMODE_NAME:
            nHeight = Max( nBmpHeight, nMaxBmpHeight );
            nHeight = Max( nHeight, nStringHeight );
            break;

        case VIEWMODE_TEXT:
            nHeight = nStringHeight;
            break;
    }
    if( nHeight > nMaxBoundHeight )
    {
        ((SvImpIconView*)this)->nMaxBoundHeight = nHeight;
        ((SvImpIconView*)this)->aHorSBar.SetLineSize( nHeight / 2 );
        ((SvImpIconView*)this)->aVerSBar.SetLineSize( nHeight / 2 );
    }
    return nHeight;
}

Size SvImpIconView::CalcBoundingSize( SvLBoxEntry* pEntry,
    SvIcnVwDataEntry* pViewData ) const
{
    if( !pViewData )
        pViewData = ICNVIEWDATA(pEntry);
    return Size( CalcBoundingWidth(pEntry,pViewData),
                 CalcBoundingHeight(pEntry,pViewData) );
}

void SvImpIconView::RecalcAllBoundingRects()
{
    nMaxBoundHeight = 0;
    pZOrderList->Remove(0, pZOrderList->Count() );
    SvLBoxEntry* pEntry = pModel->FirstChild( pCurParent );
    while( pEntry )
    {
        FindBoundingRect( pEntry );
        pZOrderList->Insert( pEntry, pZOrderList->Count() );
        pEntry = pModel->NextSibling( pEntry );
    }
    bMustRecalcBoundingRects = FALSE;
    AdjustScrollBars();
}

void SvImpIconView::RecalcAllBoundingRectsSmart()
{
    nMaxBoundHeight = 0;
    pZOrderList->Remove(0, pZOrderList->Count() );
    SvLBoxEntry* pEntry = pModel->FirstChild( pCurParent );
    while( pEntry )
    {
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        if( IsBoundingRectValid( pViewData->aRect ))
        {
            Size aBoundSize( pViewData->aRect.GetSize() );
            if( aBoundSize.Height() > nMaxBoundHeight )
                nMaxBoundHeight = aBoundSize.Height();
            pZOrderList->Insert( pEntry, pZOrderList->Count() );
        }
        else
        {
            FindBoundingRect( pEntry, pViewData );
        }
        pZOrderList->Insert( pEntry, pZOrderList->Count() );
        pEntry = pModel->NextSibling( pEntry );
    }
    AdjustScrollBars();
}

void SvImpIconView::UpdateBoundingRects()
{
    SvLBoxEntry* pEntry = pModel->FirstChild( pCurParent );
    while( pEntry )
    {
        GetBoundingRect( pEntry );
        pEntry = pModel->NextSibling( pEntry );
    }
}

void SvImpIconView::FindBoundingRect( SvLBoxEntry* pEntry,
    SvIcnVwDataEntry* pViewData )
{
    if( !pViewData )
        pViewData = ICNVIEWDATA(pEntry);

    Size aSize( CalcBoundingSize( pEntry, pViewData ) );
    Point aPos;

    DBG_ASSERT(!pViewData->IsEntryPosLocked(),"Locked entry pos in FindBoundingRect");
    // damits in der IconView nicht drunter & drueber geht
    if( pViewData->IsEntryPosLocked() && IsBoundingRectValid(pViewData->aRect) )
    {
        AdjustVirtSize( pViewData->aRect );
        return;
    }

    aPos = FindNextEntryPos( aSize );

    if( nFlags & F_GRIDMODE )
    {
        Rectangle aGridRect( aPos, Size(nGridDX, nGridDY) );
        pViewData->aGridRect = aGridRect;
        Center( pEntry, pViewData );
        AdjustVirtSize( pViewData->aRect );
        pImpCursor->SetGridUsed( pViewData->aRect );
    }
    else
    {
        pViewData->aRect = Rectangle( aPos, aSize );
        AdjustVirtSize( pViewData->aRect );
    }
}


void SvImpIconView::SetCursor( SvLBoxEntry* pEntry )
{
    if( pEntry == pCursor )
        return;

    ShowCursor( FALSE );
    if( pCursor )
    {
        pView->SetEntryFocus( pCursor, FALSE );
        if( pView->GetSelectionMode() == SINGLE_SELECTION )
            pView->Select( pCursor, FALSE );
    }
    pCursor = pEntry;
    ToTop( pCursor );
    if( pCursor )
    {
        pView->SetEntryFocus(pCursor, TRUE );
        if( pView->GetSelectionMode() == SINGLE_SELECTION )
            pView->Select( pCursor, TRUE );
        ShowCursor( TRUE );
    }
}


void SvImpIconView::ShowCursor( BOOL bShow )
{
    if( !pCursor || !bShow || !pView->HasFocus() )
    {
        pView->HideFocus();
        return;
    }
    Rectangle aRect ( CalcFocusRect( pCursor ) );
    pView->ShowFocus( aRect );
}


void SvImpIconView::HideDDIcon()
{
    pView->Update();
    ImpHideDDIcon();
    pDDBufDev = pDDDev;
    pDDDev = 0;
}

void SvImpIconView::ImpHideDDIcon()
{
    if( pDDDev )
    {
        Size aSize( pDDDev->GetOutputSizePixel() );
        // pView restaurieren
        pView->DrawOutDev( aDDLastRectPos, aSize, Point(), aSize, *pDDDev );
    }
}


void SvImpIconView::ShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPosPix )
{
    pView->Update();
    if( pRefEntry != pDDRefEntry )
    {
        DELETEZ(pDDDev);
        DELETEZ(pDDBufDev);
    }
    BOOL bSelected = pView->SvListView::Select( pRefEntry, FALSE );
    if( !pDDDev )
    {
        if( pDDBufDev )
        {
            // nicht bei jedem Move ein Device anlegen, da dies besonders
            // auf Remote-Clients zu langsam ist
            pDDDev = pDDBufDev;
            pDDBufDev = 0;
        }
        else
        {
            pDDDev = new VirtualDevice( *pView );
            pDDDev->SetFont( pView->GetFont() );
        }
    }
    else
    {
        ImpHideDDIcon();
    }
    const Rectangle& rRect = GetBoundingRect( pRefEntry );
    pDDDev->SetOutputSizePixel( rRect.GetSize() );

    Point aPos( rPosPix );
    CalcDocPos( aPos );

    Size aSize( pDDDev->GetOutputSizePixel() );
    pDDRefEntry = pRefEntry;
    aDDLastEntryPos = aPos;
    aDDLastRectPos = aPos;

    // Hintergrund sichern
    pDDDev->DrawOutDev( Point(), aSize, aPos, aSize, *pView );
    // Icon in pView malen
    nFlags |= F_NO_EMPHASIS;
    PaintEntry( pRefEntry, aPos );
    nFlags &= ~F_NO_EMPHASIS;
    if( bSelected )
        pView->SvListView::Select( pRefEntry, TRUE );
}

void SvImpIconView::HideShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPosPix )
{
/*  In Notfaellen folgenden flackernden Code aktivieren:

        HideDDIcon();
        ShowDDIcon( pRefEntry, rPosPix );
        return;
*/
    if( !pDDDev )
    {
        ShowDDIcon( pRefEntry, rPosPix );
        return;
    }

    if( pRefEntry != pDDRefEntry )
    {
        HideDDIcon();
        ShowDDIcon( pRefEntry, rPosPix );
        return;
    }

    Point aEmptyPoint;

    Point aCurEntryPos( rPosPix );
    CalcDocPos( aCurEntryPos );

    const Rectangle& rRect = GetBoundingRect( pRefEntry );
    Size aEntrySize( rRect.GetSize() );
    Rectangle aPrevEntryRect( aDDLastEntryPos, aEntrySize );
    Rectangle aCurEntryRect( aCurEntryPos, aEntrySize );

    if( !aPrevEntryRect.IsOver( aCurEntryRect ) )
    {
        HideDDIcon();
        ShowDDIcon( pRefEntry, rPosPix );
        return;
    }

    // Ueberlappung des neuen und alten D&D-Pointers!

    Rectangle aFullRect( aPrevEntryRect.Union( aCurEntryRect ) );
    if( !pDDTempDev )
    {
        pDDTempDev = new VirtualDevice( *pView );
        pDDTempDev->SetFont( pView->GetFont() );
    }

    Size aFullSize( aFullRect.GetSize() );
    Point aFullPos( aFullRect.TopLeft() );

    pDDTempDev->SetOutputSizePixel( aFullSize );

    // Hintergrund (mit dem alten D&D-Pointer!) sichern
    pDDTempDev->DrawOutDev( aEmptyPoint, aFullSize, aFullPos, aFullSize, *pView );
    // den alten Buffer in den neuen Buffer pasten
    aDDLastRectPos = aDDLastRectPos - aFullPos;

    pDDTempDev->DrawOutDev(
        aDDLastRectPos,
        pDDDev->GetOutputSizePixel(),
        aEmptyPoint,
        pDDDev->GetOutputSizePixel(),
        *pDDDev );

    // Swap
    VirtualDevice* pTemp = pDDDev;
    pDDDev = pDDTempDev;
    pDDTempDev = pTemp;

    // in den restaurierten Hintergrund den neuen D&D-Pointer zeichnen
    pDDTempDev->SetOutputSizePixel( pDDDev->GetOutputSizePixel() );
    pDDTempDev->DrawOutDev(
        aEmptyPoint, aFullSize, aEmptyPoint, aFullSize, *pDDDev );
    Point aRelPos = aCurEntryPos - aFullPos;
    nFlags |= F_NO_EMPHASIS;
    PaintEntry( pRefEntry, aRelPos, 0, pDDTempDev );
    nFlags &= ~F_NO_EMPHASIS;

    aDDLastRectPos = aFullPos;
    aDDLastEntryPos = aCurEntryPos;

    pView->DrawOutDev(
        aDDLastRectPos,
        pDDDev->GetOutputSizePixel(),
        aEmptyPoint,
        pDDDev->GetOutputSizePixel(),
        *pDDTempDev );

    BOOL bSelected = pView->SvListView::Select( pRefEntry, FALSE );
    if( bSelected )
        pView->SvListView::Select( pRefEntry, TRUE );
}

void SvImpIconView::ShowTargetEmphasis( SvLBoxEntry* pEntry, BOOL bShow )
{
    CheckBoundingRects();
    Rectangle aRect;
    if( pEntry != pCurParent &&
        (pEntry->HasChilds() || pEntry->HasChildsOnDemand()) )
        aRect = CalcBmpRect( pEntry );
    else
    {
        aRect.SetSize( aOutputSize );
        const MapMode& rMapMode = pView->GetMapMode();
        Point aOrigin( rMapMode.GetOrigin());
        aOrigin *= -1; // in Doc-Koord wandeln
        aRect.SetPos( aOrigin );
        aRect.Left()++; aRect.Top()++;
        aRect.Right()--; aRect.Bottom()--;
    }
    ImpDrawXORRect( aRect );
}

BOOL SvImpIconView::NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
    SvLBoxEntry*& rpNewPar, ULONG& rNewChildPos )
{
    if( pTarget == pCurParent && pModel->GetParent(pEntry) == pCurParent )
    {
        // D&D innerhalb einer Childlist
        StopEditTimer();
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        Size aSize( pViewData->aRect.GetSize() );
        Point aNewPos = FindNextEntryPos( aSize );
        AdjustVirtSize( Rectangle( aNewPos, aSize ) );
        SetEntryPos( pEntry, aNewPos, FALSE, TRUE );
        return FALSE;
    }
    return pView->SvLBox::NotifyMoving(pTarget,pEntry,rpNewPar,rNewChildPos);
}

BOOL SvImpIconView::NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
    SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos )
{
    return pView->SvLBox::NotifyCopying(pTarget,pEntry,rpNewParent,rNewChildPos);
}

void SvImpIconView::WriteDragServerInfo( const Point& rPos, SvLBoxDDInfo* pInfo)
{
    SvLBoxEntry* pCurEntry = GetCurEntry();
    Point aEntryPos;
    if( pCurEntry )
    {
        aEntryPos = rPos;
        aEntryPos -= GetEntryPos( pCurEntry );
    }
    pInfo->nMouseRelX = aEntryPos.X();
    pInfo->nMouseRelY = aEntryPos.Y();
}

void SvImpIconView::ReadDragServerInfo( const Point& rPos, SvLBoxDDInfo* pInfo )
{
    Point aDropPos( rPos );
    aDropPos.X() -= pInfo->nMouseRelX;
    aDropPos.Y() -= pInfo->nMouseRelY;
    SetNextEntryPos( aDropPos );
}

void SvImpIconView::InvalidateBoundingRect( SvLBoxEntry* pEntry )
{
    SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
    InvalidateBoundingRect( pViewData->aRect );
}

void SvImpIconView::PrepareCommandEvent( const CommandEvent& rCEvt )
{
    aMouseMoveTimer.Stop();
    StopEditTimer();
    nFlags |= F_CMD_ARRIVED;
    SvLBoxEntry* pEntry = pView->GetEntry( rCEvt.GetMousePosPixel(), TRUE );
    if( (nFlags & F_DOWN_CTRL) && pEntry && !pView->IsSelected(pEntry) )
        pView->Select( pEntry, TRUE );
    nFlags &= ~(F_DOWN_CTRL | F_DOWN_DESELECT);
}

void SvImpIconView::Command( const CommandEvent& rCEvt )
{
    PrepareCommandEvent( rCEvt );
    if( rCEvt.GetCommand() == COMMAND_STARTDRAG )
    {
        nFlags |= F_DRAG_SOURCE;
        if( GetSelectionCount() )
        {
            ShowCursor( FALSE );
            pView->BeginDrag( rCEvt.GetMousePosPixel() );
            ShowCursor( TRUE );
        }
        nFlags &= (~F_DRAG_SOURCE);
    }
}

void SvImpIconView::ToTop( SvLBoxEntry* pEntry )
{
    DBG_ASSERT(pZOrderList->GetPos(pEntry)!=0xffff,"ToTop:ZOrder?");
    if( pZOrderList->GetObject( pZOrderList->Count() -1 ) != pEntry )
    {
        USHORT nPos = pZOrderList->GetPos( (void*)pEntry );
        pZOrderList->Remove( nPos, 1 );
        pZOrderList->Insert( pEntry, pZOrderList->Count() );
    }
}

void SvImpIconView::SetCurParent( SvLBoxEntry* pNewParent )
{
    Clear();
    pCurParent = pNewParent;
    ImpArrange();
}

void SvImpIconView::ClipAtVirtOutRect( Rectangle& rRect ) const
{
    if( rRect.Bottom() >= aVirtOutputSize.Height() )
        rRect.Bottom() = aVirtOutputSize.Height() - 1;
    if( rRect.Right() >= aVirtOutputSize.Width() )
        rRect.Right() = aVirtOutputSize.Width() - 1;
    if( rRect.Top() < 0 )
        rRect.Top() = 0;
    if( rRect.Left() < 0 )
        rRect.Left() = 0;
}

// rRect: Bereich des Dokumentes (in Dokumentkoordinaten), der
// sichtbar gemacht werden soll.
// bScrBar == TRUE: Das Rect wurde aufgrund eines ScrollBar-Events berechnet

void SvImpIconView::MakeVisible( const Rectangle& rRect, BOOL bScrBar )
{
    Rectangle aRect( rRect );
    ClipAtVirtOutRect( aRect );
    MapMode aMapMode( pView->GetMapMode() );
    Point aOrigin( aMapMode.GetOrigin() );
    // in Dokumentkoordinate umwandeln
    aOrigin *= -1;

    Rectangle aOutputArea( aOrigin, aOutputSize );
    if( aOutputArea.IsInside( aRect ) )
        return; // ist schon sichtbar

    long nDy;
    if( aRect.Top() < aOutputArea.Top() )
    {
        // nach oben scrollen (nDy < 0)
        nDy = aRect.Top() - aOutputArea.Top();
    }
    else if( aRect.Bottom() > aOutputArea.Bottom() )
    {
        // nach unten scrollen (nDy > 0)
        nDy = aRect.Bottom() - aOutputArea.Bottom();
    }
    else
        nDy = 0;

    long nDx;
    if( aRect.Left() < aOutputArea.Left() )
    {
        // nach links scrollen (nDx < 0)
        nDx = aRect.Left() - aOutputArea.Left();
    }
    else if( aRect.Right() > aOutputArea.Right() )
    {
        // nach rechts scrollen (nDx > 0)
        nDx = aRect.Right() - aOutputArea.Right();
    }
    else
        nDx = 0;

    aOrigin.X() += nDx;
    aOrigin.Y() += nDy;
    aOutputArea.SetPos( aOrigin );

    pView->Update();

    // Origin fuer SV invertieren (damit wir in
    // Dokumentkoordinaten scrollen/painten koennen)
    aOrigin *= -1;
    aMapMode.SetOrigin( aOrigin );
    pView->SetMapMode( aMapMode );

    // in umgekehrte Richtung scrollen!
    pView->Control::Scroll( -nDx, -nDy, aOutputArea, TRUE );
    if( aHorSBar.IsVisible() || aVerSBar.IsVisible() )
    {
        if( !bScrBar )
        {
            aOrigin *= -1;
            // Thumbs korrigieren
            if(aHorSBar.IsVisible() && aHorSBar.GetThumbPos() != aOrigin.X())
                aHorSBar.SetThumbPos( aOrigin.X() );
            if(aVerSBar.IsVisible() && aVerSBar.GetThumbPos() != aOrigin.Y())
                aVerSBar.SetThumbPos( aOrigin.Y() );
        }
    }
    // pruefen, ob ScrollBars noch benoetigt werden
    CheckScrollBars();
    pView->Update();
}


SvLBoxEntry* SvImpIconView::GetNewCursor()
{
    SvLBoxEntry* pNewCursor;
    if( pCursor )
    {
        pNewCursor = pImpCursor->GoLeftRight( pCursor, FALSE );
        if( !pNewCursor )
        {
            pNewCursor = pImpCursor->GoLeftRight( pCursor, TRUE );
            if( !pNewCursor )
            {
                pNewCursor = pImpCursor->GoUpDown( pCursor, FALSE );
                if( !pNewCursor )
                    pNewCursor = pImpCursor->GoUpDown( pCursor, TRUE );
            }
        }
    }
    else
        pNewCursor = pModel->FirstChild( pCurParent );
    DBG_ASSERT(!pNewCursor|| (pCursor&&pCursor!=pNewCursor),"GetNewCursor failed");
    return pNewCursor;
}


USHORT SvImpIconView:: GetSelectionCount() const
{
    USHORT nSelected = 0;
    SvLBoxEntry* pEntry = pModel->FirstChild( pCurParent);
    while( pEntry )
    {
        if( pView->IsSelected( pEntry ) )
            nSelected++;
        pEntry = pModel->NextSibling( pEntry );
    }
    return nSelected;
}


void SvImpIconView::ToggleSelection( SvLBoxEntry* pEntry )
{
    BOOL bSel;
    if( pView->IsSelected( pEntry ) )
        bSel = FALSE;
    else
        bSel = TRUE;
    pView->Select( pEntry, bSel );
}

void SvImpIconView::DeselectAllBut( SvLBoxEntry* pThisEntryNot )
{
    ClearSelectedRectList();
    SvLBoxEntry* pEntry = pModel->FirstChild( pCurParent );
    while( pEntry )
    {
        if( pEntry != pThisEntryNot && pView->IsSelected( pEntry ))
            pView->Select( pEntry, FALSE );
        pEntry = pModel->NextSibling( pEntry );
    }
}

#define ICN_ROWS    50
#define ICN_COLS    30

ImpIcnCursor::ImpIcnCursor( SvImpIconView* pOwner )
{
    pView       = pOwner;
    pColumns    = 0;
    pRows       = 0;
    pCurEntry   = 0;
    nDeltaWidth = 0;
    nDeltaHeight= 0;
    nCols       = 0;
    nRows       = 0;
    nGridCols   = 0;
    nGridRows   = 0;
    pGridMap    = 0;
}

ImpIcnCursor::~ImpIcnCursor()
{
    delete[] pColumns;
    delete[] pRows;
    delete pGridMap;
}

USHORT ImpIcnCursor::GetSortListPos( SvPtrarr* pList, long nValue,
    int bVertical )
{
    USHORT nCount = (USHORT)pList->Count();
    if( !nCount )
        return 0;

    USHORT nCurPos = 0;
    long nPrevValue = LONG_MIN;
    while( nCount )
    {
        const Rectangle& rRect=
            pView->GetBoundingRect((SvLBoxEntry*)(pList->GetObject(nCurPos)));
        long nCurValue;
        if( bVertical )
            nCurValue = rRect.Top();
        else
            nCurValue = rRect.Left();
        if( nValue >= nPrevValue && nValue <= nCurValue )
            return (USHORT)nCurPos;
        nPrevValue = nCurValue;
        nCount--;
        nCurPos++;
    }
    return pList->Count();
}

void ImpIcnCursor::ImplCreate()
{
    pView->CheckBoundingRects();
    DBG_ASSERT(pColumns==0&&pRows==0,"ImplCreate: Not cleared");

    SetDeltas();

    pColumns = new SvPtrarr[ nCols ];
    pRows = new SvPtrarr[ nRows ];

    DELETEZ(pGridMap);

    SvLBoxTreeList* pModel = pView->pModel;
    SvLBoxEntry* pEntry = pModel->FirstChild( pView->pCurParent );
    while( pEntry )
    {
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA2(pEntry);
        // const Rectangle& rRect = pView->GetBoundingRect( pEntry );
        Rectangle rRect( pView->CalcBmpRect( pEntry,0,pViewData ) );
        short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / nDeltaHeight );
        short nX = (short)( ((rRect.Left()+rRect.Right())/2) / nDeltaWidth );

        // Rundungsfehler abfangen
        if( nY >= nRows )
            nY = nRows - 1;
        if( nX >= nCols )
            nX = nCols - 1;

        USHORT nIns = GetSortListPos( &pColumns[nX], rRect.Top(), TRUE );
        pColumns[ nX ].Insert( pEntry, nIns );

        nIns = GetSortListPos( &pRows[nY], rRect.Left(), FALSE );
        pRows[ nY ].Insert( pEntry, nIns );

        pViewData->nX = nX;
        pViewData->nY = nY;

        pEntry = pModel->NextSibling( pEntry );
    }
}

void ImpIcnCursor::CreateGridMap()
{
    if( pGridMap )
        return;

    const Size& rSize = pView->aVirtOutputSize;
    long nWidth = rSize.Width();
    if( nWidth < pView->nMaxVirtWidth )
        nWidth = pView->nMaxVirtWidth;
    nWidth -= 2*LROFFS_WINBORDER;
    if( nWidth <= 0 )
        nWidth = 1;

    nGridDX = pView->nGridDX;
    nGridDY = pView->nGridDY;

    // Hinweis: Wegen der Abrundung bei Berechnung von nGridCols
    // ist es moeglich, dass Eintrage nicht im Grid liegen. Diese
    // wurden typischerweise manuell verschoben und gelockt
    nGridCols = nWidth / nGridDX;
    if( !nGridCols ) nGridCols = 1;

    nGridRows = rSize.Height() / nGridDY;
    // nRows nicht abrunden, da zur Vermeidung von Ueberlappungen
    // das gesamte BoundingRect des Eintrags zur Markierung im Grid
    // herangezogen wird.
    if( (nGridRows * nGridDY) < rSize.Height() )
        nGridRows++;
    else if( !nGridRows )
        nGridRows = 1;

    //XXX
    //nGridRows += 50; // in fuenfziger-Schritten

    pGridMap = new BOOL[ nGridRows*nGridCols];
    memset( (void*)pGridMap, 0, nGridRows*nGridCols );

    SvLBoxTreeList* pModel = pView->pModel;
    SvLBoxEntry* pEntry = pModel->FirstChild( pView->pCurParent );
    while( pEntry )
    {
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA2(pEntry);
        const Rectangle& rRect = pViewData->aRect;
        // nur, wenn der Entry schon plaziert ist
        if( pView->IsBoundingRectValid( rRect ))
        {
            // Alle vom Eintrag beruehrten Grids kennzeichnen
            SetGridUsed( pView->GetBoundingRect( pEntry, pViewData ) );
        }
        pEntry = pModel->NextSibling( pEntry );
    }
}

BOOL ImpIcnCursor::GetGrid( const Point& rDocPos, USHORT& rGridX, USHORT& rGridY ) const
{
    Point aPos( rDocPos );
    aPos.X() -= LROFFS_WINBORDER;
    aPos.Y() -= TBOFFS_WINBORDER;
    rGridX = (USHORT)(aPos.X() / nGridDX);
    rGridY = (USHORT)(aPos.Y() / nGridDY);
    BOOL bInGrid = TRUE;
    if( rGridX >= nGridCols )
    {
        rGridX = nGridCols - 1;
        bInGrid = FALSE;
    }
    if( rGridY >= nGridRows )
    {
        rGridY = nGridRows - 1;
        if( !bInGrid )
            return FALSE; // beide Koordinaten nicht im Grid
    }
    return TRUE;
}

void ImpIcnCursor::SetGridUsed( const Rectangle& rRect, BOOL bUsed  )
{
    CreateGridMap();
    USHORT nTLX, nTLY, nBRX, nBRY;

    BOOL bTLInGrid = GetGrid( rRect.TopLeft(), nTLX, nTLY );
    BOOL bBRInGrid = GetGrid( rRect.BottomRight(), nBRX, nBRY );

    if( !bTLInGrid && !bBRInGrid )
        return;

    for( USHORT nCurY = nTLY; nCurY <= nBRY; nCurY++ )
    {
        for( USHORT nCurX = nTLX; nCurX <= nBRX; nCurX++ )
        {
            SetGridUsed( nCurX, nCurY, bUsed );
        }
    }
}

void ImpIcnCursor::Clear( BOOL bGridToo )
{
    if( pColumns )
    {
        delete[] pColumns;
        delete[] pRows;
        pColumns = 0;
        pRows = 0;
        pCurEntry = 0;
        nDeltaWidth = 0;
        nDeltaHeight = 0;
    }
    if( bGridToo && pGridMap )
    {
        DELETEZ(pGridMap);
        nGridRows = 0;
        nGridCols = 0;
    }
}

SvLBoxEntry* ImpIcnCursor::SearchCol(USHORT nCol,USHORT nTop,USHORT nBottom,
    USHORT nPref, BOOL bDown, BOOL bSimple  )
{
    DBG_ASSERT(pCurEntry,"SearchCol: No reference entry");
    SvPtrarr* pList = &(pColumns[ nCol ]);
    USHORT nCount = pList->Count();
    if( !nCount )
        return 0;

    const Rectangle& rRefRect = pView->GetBoundingRect(pCurEntry);

    if( bSimple )
    {
        USHORT nListPos = pList->GetPos( pCurEntry );
        DBG_ASSERT(nListPos!=0xffff,"Entry not in Col-List");
        if( bDown )
        {
            while( nListPos < nCount-1 )
            {
                nListPos++;
                SvLBoxEntry* pEntry = (SvLBoxEntry*)pList->GetObject( nListPos );
                const Rectangle& rRect = pView->GetBoundingRect( pEntry );
                if( rRect.Top() > rRefRect.Top() )
                    return pEntry;
            }
            return 0;
        }
        else
        {
            while( nListPos )
            {
                nListPos--;
                if( nListPos < nCount )
                {
                    SvLBoxEntry* pEntry = (SvLBoxEntry*)pList->GetObject( nListPos );
                    const Rectangle& rRect = pView->GetBoundingRect( pEntry );
                    if( rRect.Top() < rRefRect.Top() )
                        return pEntry;
                }
            }
            return 0;
        }
    }

    if( nTop > nBottom )
    {
        USHORT nTemp = nTop;
        nTop = nBottom;
        nBottom = nTemp;
    }
    long nMinDistance = LONG_MAX;
    SvLBoxEntry* pResult = 0;
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        SvLBoxEntry* pEntry = (SvLBoxEntry*)(pList->GetObject( nCur ));
        if( pEntry != pCurEntry )
        {
            SvIcnVwDataEntry* pViewData = ICNVIEWDATA2(pEntry);
            USHORT nY = pViewData->nY;
            if( nY >= nTop && nY <= nBottom )
            {
                const Rectangle& rRect = pView->GetBoundingRect( pEntry );
                long nDistance = rRect.Top() - rRefRect.Top();
                if( nDistance < 0 )
                    nDistance *= -1;
                if( nDistance && nDistance < nMinDistance )
                {
                    nMinDistance = nDistance;
                    pResult = pEntry;
                }
            }
        }
    }
    return pResult;
}

SvLBoxEntry* ImpIcnCursor::SearchRow(USHORT nRow,USHORT nLeft,USHORT nRight,
    USHORT nPref, BOOL bRight, BOOL bSimple )
{
    DBG_ASSERT(pCurEntry,"SearchRow: No reference entry");
    SvPtrarr* pList = &(pRows[ nRow ]);
    USHORT nCount = pList->Count();
    if( !nCount )
        return 0;

    const Rectangle& rRefRect = pView->GetBoundingRect(pCurEntry);

    if( bSimple )
    {
        USHORT nListPos = pList->GetPos( pCurEntry );
        DBG_ASSERT(nListPos!=0xffff,"Entry not in Row-List");
        if( bRight )
        {
            while( nListPos < nCount-1 )
            {
                nListPos++;
                SvLBoxEntry* pEntry = (SvLBoxEntry*)pList->GetObject( nListPos );
                const Rectangle& rRect = pView->GetBoundingRect( pEntry );
                if( rRect.Left() > rRefRect.Left() )
                    return pEntry;
            }
            return 0;
        }
        else
        {
            while( nListPos )
            {
                nListPos--;
                if( nListPos < nCount )
                {
                    SvLBoxEntry* pEntry = (SvLBoxEntry*)pList->GetObject( nListPos );
                    const Rectangle& rRect = pView->GetBoundingRect( pEntry );
                    if( rRect.Left() < rRefRect.Left() )
                        return pEntry;
                }
            }
            return 0;
        }

    }
    if( nRight < nLeft )
    {
        USHORT nTemp = nRight;
        nRight = nLeft;
        nLeft = nTemp;
    }
    long nMinDistance = LONG_MAX;
    SvLBoxEntry* pResult = 0;
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        SvLBoxEntry* pEntry = (SvLBoxEntry*)(pList->GetObject( nCur ));
        if( pEntry != pCurEntry )
        {
            SvIcnVwDataEntry* pViewData = ICNVIEWDATA2(pEntry);
            USHORT nX = pViewData->nX;
            if( nX >= nLeft && nX <= nRight )
            {
                const Rectangle& rRect = pView->GetBoundingRect( pEntry );
                long nDistance = rRect.Left() - rRefRect.Left();
                if( nDistance < 0 )
                    nDistance *= -1;
                if( nDistance && nDistance < nMinDistance )
                {
                    nMinDistance = nDistance;
                    pResult = pEntry;
                }
            }
        }
    }
    return pResult;
}



/*
    Sucht ab dem uebergebenen Eintrag den naechsten rechts- bzw.
    linksstehenden. Suchverfahren am Beispiel bRight = TRUE:

                  c
                b c
              a b c
            S 1 1 1      ====> Suchrichtung
              a b c
                b c
                  c

    S : Startposition
    1 : erstes Suchrechteck
    a,b,c : 2., 3., 4. Suchrechteck
*/

SvLBoxEntry* ImpIcnCursor::GoLeftRight( SvLBoxEntry* pEntry, BOOL bRight )
{
    SvLBoxEntry* pResult;
    pCurEntry = pEntry;
    Create();
    SvIcnVwDataEntry* pViewData = ICNVIEWDATA2(pEntry);
    USHORT nY = pViewData->nY;
    USHORT nX = pViewData->nX;
    DBG_ASSERT(nY< nRows,"GoLeftRight:Bad column");
    DBG_ASSERT(nX< nCols,"GoLeftRight:Bad row");
    // Nachbar auf gleicher Zeile ?
    if( bRight )
        pResult = SearchRow( nY, nX ,nCols-1, nX, TRUE, TRUE );
    else
        pResult = SearchRow( nY, nX ,0, nX, FALSE, TRUE );
    if( pResult )
        return pResult;

    long nCurCol = nX;

    long nColOffs, nLastCol;
    if( bRight )
    {
        nColOffs = 1;
        nLastCol = nCols;
    }
    else
    {
        nColOffs = -1;
        nLastCol = -1;   // 0-1
    }

    USHORT nRowMin = nY;
    USHORT nRowMax = nY;
    do
    {
        SvLBoxEntry* pEntry = SearchCol((USHORT)nCurCol,nRowMin,nRowMax,nY,TRUE, FALSE);
        if( pEntry )
            return pEntry;
        if( nRowMin )
            nRowMin--;
        if( nRowMax < (nRows-1))
            nRowMax++;
        nCurCol += nColOffs;
    } while( nCurCol != nLastCol );
    return 0;
}

SvLBoxEntry* ImpIcnCursor::GoUpDown( SvLBoxEntry* pEntry, BOOL bDown)
{
    SvLBoxEntry* pResult;
    pCurEntry = pEntry;
    Create();
    SvIcnVwDataEntry* pViewData = ICNVIEWDATA2(pEntry);
    USHORT nY = pViewData->nY;
    USHORT nX = pViewData->nX;
    DBG_ASSERT(nY<nRows,"GoUpDown:Bad column");
    DBG_ASSERT(nX<nCols,"GoUpDown:Bad row");

    // Nachbar in gleicher Spalte ?
    if( bDown )
        pResult = SearchCol( nX, nY ,nRows-1, nY, TRUE, TRUE );
    else
        pResult = SearchCol( nX, nY ,0, nY, FALSE, TRUE );
    if( pResult )
        return pResult;

    long nCurRow = nY;

    long nRowOffs, nLastRow;
    if( bDown )
    {
        nRowOffs = 1;
        nLastRow = nRows;
    }
    else
    {
        nRowOffs = -1;
        nLastRow = -1;   // 0-1
    }

    USHORT nColMin = nX;
    USHORT nColMax = nX;
    do
    {
        SvLBoxEntry* pEntry = SearchRow((USHORT)nCurRow,nColMin,nColMax,nX,TRUE, FALSE);
        if( pEntry )
            return pEntry;
        if( nColMin )
            nColMin--;
        if( nColMax < (nCols-1))
            nColMax++;
        nCurRow += nRowOffs;
    } while( nCurRow != nLastRow );
    return 0;
}

void ImpIcnCursor::SetDeltas()
{
    const Size& rSize = pView->aVirtOutputSize;
    if( pView->nFlags & F_GRIDMODE )
    {
        nGridDX = pView->nGridDX;
        nGridDY = pView->nGridDY;
    }
    else
    {
        nGridDX = 20;
        nGridDY = 20;
    }
    nCols = rSize.Width() / nGridDX;
    if( !nCols )
        nCols = 1;
    nRows = rSize.Height() / nGridDY;
    if( (nRows * nGridDY) < rSize.Height() )
        nRows++;
    if( !nRows )
        nRows = 1;

    nDeltaWidth = (short)(rSize.Width() / nCols);
    nDeltaHeight = (short)(rSize.Height() / nRows);
    if( !nDeltaHeight )
    {
        nDeltaHeight = 1;
        DBG_WARNING("SetDeltas:Bad height");
    }
    if( !nDeltaWidth )
    {
        nDeltaWidth = 1;
        DBG_WARNING("SetDeltas:Bad width");
    }
}


void ImpIcnCursor::ExpandGrid()
{
    if( pGridMap )
    {
        long nNewGridRows = nGridRows + 20;
        unsigned char* pTempMap = new unsigned char[ nNewGridRows * nGridCols ];
        memset( pTempMap, nNewGridRows * nGridCols, 0 );
        memcpy( pTempMap, pGridMap, nGridRows * nGridCols );
        delete pGridMap;
        pGridMap = pTempMap;
        nGridRows = nNewGridRows;
    }
}

BOOL ImpIcnCursor::FindEmptyGridRect( Rectangle& rRect )
{
    CreateGridMap();
    USHORT nCount = (USHORT)(nGridCols * nGridRows);
    if( !nCount )
        return FALSE;
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        if( !pGridMap[ nCur ] )
        {
            USHORT nCol = (USHORT)(nCur % nGridCols);
            USHORT nRow = (USHORT)(nCur / nGridCols);
            rRect.Top() = nRow * nGridDY + TBOFFS_WINBORDER;
            rRect.Bottom() = rRect.Top() + nGridDY;
            rRect.Left() = nCol * nGridDX+ LROFFS_WINBORDER;
            rRect.Right() = rRect.Left() + nGridDX;
            SetGridUsed( nCol, nRow, TRUE );

            //XXX
            //if( nRow + 5 > nGridRows )
            //  ExpandGrid();
            DBG_ASSERT(pGridMap[nCur],"SetGridUsed failed");
            return TRUE;
        }
    }
    // Gridmap ist voll: Um eine Zeile erweitern
    rRect.Top() = nGridRows * nGridDY + TBOFFS_WINBORDER;
    rRect.Bottom() = rRect.Top() + nGridDY;
    rRect.Left() = LROFFS_WINBORDER;
    rRect.Right() = rRect.Left() + nGridDX;
    return FALSE;
    //XXX
    //ExpandGrid();
    //return TRUE;
}

void ImpIcnCursor::CreateGridAjustData( SvPtrarr& rLists, SvLBoxEntry* pRefEntry)
{
    if( !pRefEntry )
    {
        USHORT nRows = (USHORT)(pView->aVirtOutputSize.Height() / pView->nGridDY);
        nRows++; // wg. Abrundung!

        if( !nRows )
            return;
        for( USHORT nCurList = 0; nCurList < nRows; nCurList++ )
        {
            SvPtrarr* pRow = new SvPtrarr;
            rLists.Insert( (void*)pRow, nCurList );
        }
        SvLBoxEntry* pEntry = pView->pModel->FirstChild( pView->pCurParent );
        while( pEntry )
        {
            const Rectangle& rRect = pView->GetBoundingRect( pEntry );
            short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / pView->nGridDY );
            USHORT nIns = GetSortListPos((SvPtrarr*)rLists[nY],rRect.Left(),FALSE);
            ((SvPtrarr*)rLists[ nY ])->Insert( pEntry, nIns );
            pEntry = pView->pModel->NextSibling( pEntry );
        }
    }
    else
    {
        // Aufbau eines hor. "Schlauchs" auf der RefEntry-Zeile

        // UEBERLEGEN: BoundingRect nehmen wg. Ueberlappungen???

        Rectangle rRefRect( pView->CalcBmpRect( pRefEntry ) );
        //const Rectangle& rRefRect = pView->GetBoundingRect( pRefEntry );
        short nRefRow = (short)( ((rRefRect.Top()+rRefRect.Bottom())/2) / pView->nGridDY );
        SvPtrarr* pRow = new SvPtrarr;
        rLists.Insert( (void*)pRow, 0 );
        SvLBoxEntry* pEntry = pView->pModel->FirstChild( pView->pCurParent );
        while( pEntry )
        {
            Rectangle rRect( pView->CalcBmpRect(pEntry) );
            //const Rectangle& rRect = pView->GetBoundingRect( pEntry );
            short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / pView->nGridDY );
            if( nY == nRefRow )
            {
                USHORT nIns = GetSortListPos( pRow, rRect.Left(), FALSE );
                pRow->Insert( pEntry, nIns );
            }
            pEntry = pView->pModel->NextSibling( pEntry );
        }
    }
}

//static
void ImpIcnCursor::DestroyGridAdjustData( SvPtrarr& rLists )
{
    USHORT nCount = rLists.Count();
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        SvPtrarr* pArr = (SvPtrarr*)rLists[ nCur ];
        delete pArr;
    }
    rLists.Remove( 0, rLists.Count() );
}

void SvImpIconView::SetGrid( long nDX, long nDY )
{
    nGridDX = nDX;
    nGridDY = nDY;
    nFlags |= F_GRIDMODE;
}

Rectangle SvImpIconView::CalcMaxTextRect( const SvLBoxEntry* pEntry,
    const SvIcnVwDataEntry* pViewData ) const
{
    Rectangle aRect = pViewData->aGridRect;
    long nBmpHeight = ((SvLBoxEntry*)pEntry)->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP)->GetSize(pView,(SvLBoxEntry*)pEntry).Height();
    aRect.Top() += nBmpHeight;
    aRect.Top() += ICONVIEW_OFFS_BMP_STRING;
    if( aRect.Top() > aRect.Bottom())
        aRect.Top() = aRect.Bottom();
    aRect.Left() += LROFFS_BOUND;
    aRect.Left()++;
    aRect.Right() -= LROFFS_BOUND;
    aRect.Right()--;
    if( aRect.Left() > aRect.Right())
        aRect.Left() = aRect.Right();
    if( GetTextMode( pEntry, pViewData ) == ShowTextFull )
        aRect.Bottom() = LONG_MAX;
    return aRect;
}

void SvImpIconView::Center( SvLBoxEntry* pEntry,
    SvIcnVwDataEntry* pViewData ) const
{
    SvLBoxString* pStringItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    const String& rEntryText = pStringItem->GetText();

    Rectangle aTextRect = CalcMaxTextRect(pEntry,pViewData);
    aTextRect = GetTextRect( pView, aTextRect, rEntryText, DRAWTEXT_FLAGS );
    pViewData->aTextSize = aTextRect.GetSize();

    pViewData->aRect = pViewData->aGridRect;
    Size aSize( CalcBoundingSize( pEntry, pViewData ) );
    long nBorder = pViewData->aGridRect.GetWidth() - aSize.Width();
    pViewData->aRect.Left() += nBorder / 2;
    pViewData->aRect.Right() -= nBorder / 2;
    pViewData->aRect.Bottom() = pViewData->aRect.Top() + aSize.Height();
}


// Die Deltas entsprechen Offsets, um die die View auf dem Doc verschoben wird
// links, hoch: Offsets < 0
// rechts, runter: Offsets > 0
void SvImpIconView::Scroll( long nDeltaX, long nDeltaY, BOOL bScrollBar )
{
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    // in Dokumentkoordinate umwandeln
    aOrigin *= -1;
    aOrigin.Y() += nDeltaY;
    aOrigin.X() += nDeltaX;
    Rectangle aRect( aOrigin, aOutputSize );
    MakeVisible( aRect, bScrollBar );
}


const Size& SvImpIconView::GetItemSize( SvIconView* pView,
    SvLBoxEntry* pEntry, SvLBoxItem* pItem, const SvIcnVwDataEntry* pViewData) const
{
    if( (nFlags & F_GRIDMODE) && pItem->IsA() == SV_ITEM_ID_LBOXSTRING )
    {
        if( !pViewData )
            pViewData = ICNVIEWDATA(pEntry);
        return pViewData->aTextSize;
    }
    else
        return pItem->GetSize( pView, pEntry );
}

Rectangle SvImpIconView::CalcFocusRect( SvLBoxEntry* pEntry )
{
#if !defined(OS2)
    SvLBoxString* pStringItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    DBG_ASSERT(pStringItem,"Text not set");
    return CalcTextRect( pEntry, pStringItem );
#else
    return CalcBmpRect( pEntry );
#endif
}


void SvImpIconView::SelectRect( const Rectangle& rRect, BOOL bAdd,
    SvPtrarr* pOtherRects, short nBorderOffs )
{
    if( !pZOrderList || !pZOrderList->Count() )
        return;

    CheckBoundingRects();
    pView->Update();
    USHORT nCount = pZOrderList->Count();

    Rectangle aRect( rRect );
    aRect.Justify();
    if( nBorderOffs )
    {
        aRect.Left() -= nBorderOffs;
        aRect.Right() += nBorderOffs;
        aRect.Top() -= nBorderOffs;
        aRect.Bottom() += nBorderOffs;
    }
    BOOL bCalcOverlap = (bAdd && pOtherRects && pOtherRects->Count()) ? TRUE : FALSE;

    for( USHORT nPos = 0; nPos < nCount; nPos++ )
    {
        SvLBoxEntry* pEntry = (SvLBoxEntry*)(pZOrderList->GetObject(nPos ));

        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        DBG_ASSERT(pViewData,"Entry not in model")
        if( !IsBoundingRectValid( pViewData->aRect ))
            FindBoundingRect( pEntry, pViewData );
        const Rectangle& rBoundRect = pViewData->aRect;
        BOOL bSelected = pViewData->IsSelected();

        BOOL bOverlaps;
        if( bCalcOverlap )
            bOverlaps = IsOver( pOtherRects, rBoundRect );
        else
            bOverlaps = FALSE;
        BOOL bOver = aRect.IsOver( rBoundRect );

        if( bOver && !bOverlaps )
        {
            // Ist im neuen Selektionsrechteck und in keinem alten
            // => selektieren
            if( !bSelected )
                pView->Select( pEntry, TRUE );
        }
        else if( !bAdd )
        {
            // ist ausserhalb des Selektionsrechtecks
            // => Selektion entfernen
            if( bSelected )
                pView->Select( pEntry, FALSE );
        }
        else if( bAdd && bOverlaps )
        {
            // Der Eintrag befindet sich in einem alten (=>Aufspannen
            // mehrerer Rechtecke mit Ctrl!) Selektionsrechteck

            // Hier ist noch ein Bug! Der Selektionsstatus eines Eintrags
            // in einem vorherigen Rechteck, muss restauriert werden, wenn
            // er vom aktuellen Selektionsrechteck beruehrt wurde, jetzt aber
            // nicht mehr in ihm liegt. Ich gehe hier der Einfachheit halber
            // pauschal davon aus, dass die Eintraege in den alten Rechtecken
            // alle selektiert sind. Ebenso ist es falsch, die Schnittmenge
            // nur zu deselektieren.
            // Loesungsmoeglichkeit: Snapshot der Selektion vor dem Auf-
            // spannen des Rechtecks merken
            if( rBoundRect.IsOver( rRect))
            {
                // Schnittmenge zwischen alten Rects & aktuellem Rect desel.
                if( bSelected )
                    pView->Select( pEntry, FALSE );
            }
            else
            {
                // Eintrag eines alten Rects selektieren
                if( !bSelected )
                    pView->Select( pEntry, TRUE );
            }
        }
        else if( !bOver && bSelected )
        {
            // Der Eintrag liegt voellig ausserhalb und wird deshalb desel.
            pView->Select( pEntry, FALSE );
        }
    }
    pView->Update();
}

BOOL SvImpIconView::IsOver( SvPtrarr* pRectList, const Rectangle& rBoundRect ) const
{
    USHORT nCount = pRectList->Count();
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        Rectangle* pRect = (Rectangle*)pRectList->GetObject( nCur );
        if( rBoundRect.IsOver( *pRect ))
            return TRUE;
    }
    return FALSE;
}

void SvImpIconView::AddSelectedRect( const Rectangle& rRect, short nBorderOffs )
{
    Rectangle* pRect = new Rectangle( rRect );
    pRect->Justify();
    if( nBorderOffs )
    {
        pRect->Left() -= nBorderOffs;
        pRect->Right() += nBorderOffs;
        pRect->Top() -= nBorderOffs;
        pRect->Bottom() += nBorderOffs;
    }
    aSelectedRectList.Insert( (void*)pRect, aSelectedRectList.Count() );
}

void SvImpIconView::ClearSelectedRectList()
{
    USHORT nCount = aSelectedRectList.Count();
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        Rectangle* pRect = (Rectangle*)aSelectedRectList.GetObject( nCur );
        delete pRect;
    }
    aSelectedRectList.Remove( 0, aSelectedRectList.Count() );
}


void SvImpIconView::DrawSelectionRect( const Rectangle& rRect )
{
    pView->HideTracking();
    nFlags |= F_SELRECT_VISIBLE;
    pView->ShowTracking( rRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
    aCurSelectionRect = rRect;
}

void SvImpIconView::HideSelectionRect()
{
    if( nFlags & F_SELRECT_VISIBLE )
    {
        pView->HideTracking();
        nFlags &= ~F_SELRECT_VISIBLE;
    }
}

void SvImpIconView::ImpDrawXORRect( const Rectangle& rRect )
{
    RasterOp eOldOp = pView->GetRasterOp();
    pView->SetRasterOp( ROP_XOR );
    Color aOldColor = pView->GetFillColor();
    pView->SetFillColor();
    pView->DrawRect( rRect );
    pView->SetFillColor( aOldColor );
    pView->SetRasterOp( eOldOp );
}

void SvImpIconView::CalcScrollOffsets( const Point& rPosPixel,
    long& rX, long& rY, BOOL bInDragDrop, USHORT nBorderWidth)
{
    // Scrolling der View, falls sich der Mauszeiger im Grenzbereich des
    // Fensters befindet
    long nPixelToScrollX = 0;
    long nPixelToScrollY = 0;
    Size aWndSize = aOutputSize;

    nBorderWidth = (USHORT)(Min( (long)(aWndSize.Height()-1), (long)nBorderWidth ));
    nBorderWidth = (USHORT)(Min( (long)(aWndSize.Width()-1), (long)nBorderWidth ));

    if ( rPosPixel.X() < nBorderWidth )
    {
        if( bInDragDrop )
            nPixelToScrollX = -DD_SCROLL_PIXEL;
        else
            nPixelToScrollX = rPosPixel.X()- nBorderWidth;
    }
    else if ( rPosPixel.X() > aWndSize.Width() - nBorderWidth )
    {
        if( bInDragDrop )
            nPixelToScrollX = DD_SCROLL_PIXEL;
        else
            nPixelToScrollX = rPosPixel.X() - (aWndSize.Width() - nBorderWidth);
    }
    if ( rPosPixel.Y() < nBorderWidth )
    {
        if( bInDragDrop )
            nPixelToScrollY = -DD_SCROLL_PIXEL;
        else
            nPixelToScrollY = rPosPixel.Y() - nBorderWidth;
    }
    else if ( rPosPixel.Y() > aWndSize.Height() - nBorderWidth )
    {
        if( bInDragDrop )
            nPixelToScrollY = DD_SCROLL_PIXEL;
        else
            nPixelToScrollY = rPosPixel.Y() - (aWndSize.Height() - nBorderWidth);
    }

    rX = nPixelToScrollX;
    rY = nPixelToScrollY;
}

IMPL_LINK(SvImpIconView, MouseMoveTimeoutHdl, Timer*, pTimer )
{
    pTimer->Start();
    MouseMove( aMouseMoveEvent );
    return 0;
}

void SvImpIconView::EndTracking()
{
    pView->ReleaseMouse();
    if( nFlags & F_RUBBERING )
    {
        aMouseMoveTimer.Stop();
        nFlags &= ~(F_RUBBERING | F_ADD_MODE);
    }
}

BOOL SvImpIconView::IsTextHit( SvLBoxEntry* pEntry, const Point& rDocPos )
{
    SvLBoxString* pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if( pItem )
    {
        Rectangle aRect( CalcTextRect( pEntry, pItem ));
        if( aRect.IsInside( rDocPos ) )
            return TRUE;
    }
    return FALSE;
}

IMPL_LINK(SvImpIconView, EditTimeoutHdl, Timer*, pTimer )
{
    SvLBoxEntry* pEntry = GetCurEntry();
    if( pView->IsInplaceEditingEnabled() && pEntry &&
        pView->IsSelected( pEntry ))
    {
        pView->EditEntry( pEntry );
    }
    return 0;
}


//
// Funktionen zum Ausrichten der Eintraege am Grid
//

// pStart == 0: Alle Eintraege werden ausgerichtet
// sonst: Alle Eintraege der Zeile ab einschliesslich pStart werden ausgerichtet
void SvImpIconView::AdjustAtGrid( SvLBoxEntry* pStart )
{
    SvPtrarr aLists;
    pImpCursor->CreateGridAjustData( aLists, pStart );
    USHORT nCount = aLists.Count();
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        AdjustAtGrid( *(SvPtrarr*)aLists[ nCur ], pStart );
    }
    ImpIcnCursor::DestroyGridAdjustData( aLists );
    CheckScrollBars();
}

// Richtet eine Zeile aus, erweitert ggf. die Breite; Bricht die Zeile nicht um
void SvImpIconView::AdjustAtGrid( const SvPtrarr& rRow, SvLBoxEntry* pStart )
{
    if( !rRow.Count() )
        return;

    BOOL bGo;
    if( !pStart )
        bGo = TRUE;
    else
        bGo = FALSE;

    long nCurRight = 0;
    for( USHORT nCur = 0; nCur < rRow.Count(); nCur++ )
    {
        SvLBoxEntry* pCur = (SvLBoxEntry*)rRow[ nCur ];
        if( !bGo && pCur == pStart )
            bGo = TRUE;

        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pCur);
        // Massgebend (fuer das menschliche Auge) ist die Bitmap, da sonst
        // durch lange Texte der Eintrag stark springen kann
        const Rectangle& rBoundRect = GetBoundingRect( pCur, pViewData );
        Rectangle aCenterRect( CalcBmpRect( pCur, 0, pViewData ));
        if( bGo && !pViewData->IsEntryPosLocked() )
        {
            long nWidth = aCenterRect.GetSize().Width();
            Point aNewPos( AdjustAtGrid( aCenterRect, rBoundRect ) );
            while( aNewPos.X() < nCurRight )
                aNewPos.X() += nGridDX;
            if( aNewPos != rBoundRect.TopLeft() )
                SetEntryPos( pCur, aNewPos );
            nCurRight = aNewPos.X() + nWidth;
        }
        else
        {
            nCurRight = rBoundRect.Right();
        }
    }
}

// Richtet Rect am Grid aus, garantiert jedoch nicht, dass die
// neue Pos. frei ist. Die Pos. kann fuer SetEntryPos verwendet werden.
// Das CenterRect beschreibt den Teil des BoundRects, der fuer
// die Berechnung des Ziel-Rechtecks verwendet wird.
Point SvImpIconView::AdjustAtGrid( const Rectangle& rCenterRect,
    const Rectangle& rBoundRect ) const
{
    Point aPos( rCenterRect.TopLeft() );
    Size aSize( rCenterRect.GetSize() );

    aPos.X() -= LROFFS_WINBORDER;
    aPos.Y() -= TBOFFS_WINBORDER;

    // align (ref ist mitte des rects)
    short nGridX = (short)((aPos.X()+(aSize.Width()/2)) / nGridDX);
    short nGridY = (short)((aPos.Y()+(aSize.Height()/2)) / nGridDY);
    aPos.X() = nGridX * nGridDX;
    aPos.Y() = nGridY * nGridDY;
    // hor. center
    aPos.X() += (nGridDX - rBoundRect.GetSize().Width() ) / 2;

    aPos.X() += LROFFS_WINBORDER;
    aPos.Y() += TBOFFS_WINBORDER;

    return aPos;
}


void SvImpIconView::SetTextMode( SvIconViewTextMode eMode, SvLBoxEntry* pEntry )
{
    if( !pEntry )
    {
        if( eTextMode != eMode )
        {
            if( eTextMode == ShowTextDontKnow )
                eTextMode = ShowTextShort;
            eTextMode = eMode;
            pView->Arrange();
        }
    }
    else
    {
        SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pEntry);
        if( pViewData->eTextMode != eMode )
        {
            pViewData->eTextMode = eMode;
            pModel->InvalidateEntry( pEntry );
            AdjustVirtSize( pViewData->aRect );
        }
    }
}

SvIconViewTextMode SvImpIconView::GetTextMode( const SvLBoxEntry* pEntry,
    const SvIcnVwDataEntry* pViewData ) const
{
    if( !pEntry )
        return eTextMode;
    else
    {
        if( !pViewData )
            pViewData = ICNVIEWDATA(((SvLBoxEntry*)pEntry));
        return pViewData->GetTextMode();
    }
}

SvIconViewTextMode SvImpIconView::GetEntryTextModeSmart( const SvLBoxEntry* pEntry,
    const SvIcnVwDataEntry* pViewData ) const
{
    DBG_ASSERT(pEntry,"GetEntryTextModeSmart: Entry not set");
    if( !pViewData )
        pViewData = ICNVIEWDATA(((SvLBoxEntry*)pEntry));
    SvIconViewTextMode eMode = pViewData->GetTextMode();
    if( eMode == ShowTextDontKnow )
        return eTextMode;
    return eMode;
}

void SvImpIconView::ShowFocusRect( const SvLBoxEntry* pEntry )
{
    if( !pEntry )
        pView->HideFocus();
    else
    {
        Rectangle aRect ( CalcFocusRect( (SvLBoxEntry*)pEntry ) );
        pView->ShowFocus( aRect );
    }
}

IMPL_LINK(SvImpIconView, UserEventHdl, void*, EMPTYARG )
{
    nCurUserEvent = 0;
    AdjustScrollBars();
    Rectangle aRect;
    if( GetResizeRect(aRect) )
        PaintResizeRect( aRect );
    return 0;
}

void SvImpIconView::CancelUserEvent()
{
    if( nCurUserEvent )
    {
        Application::RemoveUserEvent( nCurUserEvent );
        nCurUserEvent = 0;
    }
}


