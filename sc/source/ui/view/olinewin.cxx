/*************************************************************************
 *
 *  $RCSfile: olinewin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2002-08-14 12:24:42 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif


// INCLUDE ---------------------------------------------------------------

#include "olinewin.hxx"
#include "olinetab.hxx"
#include "document.hxx"                     // GetOutline
#include "sc.hrc"
#include "dbfunc.hxx"                       // Funktionen

#define SC_OL_BITMAPSIZE    12

#define SC_OL_IMAGE_PLUS        9
#define SC_OL_IMAGE_MINUS       10
#define SC_OL_IMAGE_NOTPRESSED  11
#define SC_OL_IMAGE_PRESSED     12

//==================================================================

ScOutlineWindow::ScOutlineWindow( Window* pParent, ScOutlineMode eNewMode,
                                    ScViewData* pData, ScSplitPos eNewWhich ) :
    Window( pParent ),
    eMode( eNewMode ),
    pViewData( pData ),
    eWhich( eNewWhich ),
    nHeaderSize( 0 ),
    bHitMode( FALSE ),
    aColor( COL_BLACK ),
    pSymbols( NULL )
{
    ImplInitSettings();
}


__EXPORT ScOutlineWindow::~ScOutlineWindow()
{
}


BOOL ScOutlineWindow::IsFirst(USHORT nPos)
{
    //  sind alle Spalten vor dieser ausgeblendet?

    BOOL bHor = (eMode==SC_OUTLINE_HOR);
    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();
    for (;;)
    {
        if (nPos==0)
            return TRUE;
        BOOL bHidden;
        if (bHor)
            bHidden = (pDoc->GetColFlags(nPos-1,nTab)&CR_HIDDEN)!=0;
        else
            bHidden = (pDoc->GetRowFlags(nPos-1,nTab)&CR_HIDDEN)!=0;
        if (bHidden)
            --nPos;
        else
            return FALSE;
    }
}


BOOL ScOutlineWindow::GetEntryPos( ScOutlineEntry* pEntry,
                                    long& rFirstEntry, long& rSecondEntry, long& rBitmapEntry,
                                    ScOutlineEntry* pPrevious )
{
    BOOL bHor = (eMode==SC_OUTLINE_HOR);
    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();

    USHORT nStart = pEntry->GetStart();
    USHORT nEnd   = pEntry->GetEnd();

    if (bHor)
    {
        rFirstEntry = nHeaderSize + pViewData->GetScrPos( nStart, 0, eWhich, TRUE ).X();
        rSecondEntry = nHeaderSize + pViewData->GetScrPos( nEnd+1, 0, eWhich, TRUE ).X();
    }
    else
    {
        rFirstEntry = nHeaderSize + pViewData->GetScrPos( 0, nStart, eWhich, TRUE ).Y();
        rSecondEntry = nHeaderSize + pViewData->GetScrPos( 0, nEnd+1, eWhich, TRUE ).Y();
    }
    //  bAllowNeg = TRUE bei GetScrPos -> auch ausserhalb des sichtbaren Bereichs weiterzaehlen,
    //  sonst wuerde die Bitmap nach oben geschoben, wenn die Position ganz unten ist.

    BOOL bHidden;
    if (bHor)
        bHidden = (pDoc->GetColFlags(nStart,nTab)&CR_HIDDEN)!=0;
    else
        bHidden = (pDoc->GetRowFlags(nStart,nTab)&CR_HIDDEN)!=0;

    if (bHidden)
        rBitmapEntry = rFirstEntry - SC_OL_BITMAPSIZE / 2;
    else
        rBitmapEntry = rFirstEntry + 1;
    rBitmapEntry = Min( (long)rBitmapEntry,
                        (long)(( rFirstEntry+rSecondEntry-SC_OL_BITMAPSIZE ) / 2 ));

        // Anpassungen

    if (bHidden && IsFirst(nStart))
        rBitmapEntry = rFirstEntry;             // ganz links nicht abschneiden

    if (!bHidden && pPrevious)                  // ausgeblendeten nicht mit naechstem verdecken
    {
         USHORT nPrevEnd = pPrevious->GetEnd();
         if (nPrevEnd+1 == nStart)
         {
            BOOL bPrevHidden;
            if (bHor)
                bPrevHidden = (pDoc->GetColFlags(nPrevEnd,nTab)&CR_HIDDEN)!=0;
            else
                bPrevHidden = (pDoc->GetRowFlags(nPrevEnd,nTab)&CR_HIDDEN)!=0;

            if (bPrevHidden)
            {
                if ( IsFirst( pPrevious->GetStart() ) )
                    rBitmapEntry = rFirstEntry + SC_OL_BITMAPSIZE;
                else
                    rBitmapEntry = rFirstEntry + SC_OL_BITMAPSIZE / 2;
            }
         }
    }

    //  rFirstEntry und rSecondEntry koennen auf den sichtbaren Bereich begrenzt werden,
    //  der "Haken" unten/rechts wird nur gezeichnet, wenn die letzte Zelle sichtbar ist.
    //  Ohne Begrenzung wird in die Ziffernfelder hineingemalt.
    if ( rFirstEntry < nHeaderSize )
        rFirstEntry = nHeaderSize;
    if ( rSecondEntry < nHeaderSize )
        rSecondEntry = nHeaderSize;

        // Filter

    BOOL bVisible;
    if (bHor)
        bVisible = TRUE;                    // horizontal: keine Filter
    else
    {
        bVisible = FALSE;
        USHORT nRow;
        for (nRow=nStart; nRow<=nEnd && !bVisible; nRow++)
            if (!pDoc->IsFiltered(nRow,nTab))
                bVisible = TRUE;
    }

    return bVisible;
}


void ScOutlineWindow::ImplDrawImage( const Point& rPos, sal_uInt16 nId )
{
    DBG_ASSERT( pSymbols, "ScOutlineWindow::ImplDrawImage - no images" );
    const Image& rImage = pSymbols->GetImage( nId );
    SetLineColor();
    SetFillColor( GetBackground().GetColor() );
    DrawRect( Rectangle( rPos, rImage.GetSizePixel() ) );
    DrawImage( rPos, rImage );
}

void ScOutlineWindow::ImplDrawBorder( const Point& rPos, bool bPressed )
{
    DBG_ASSERT( pSymbols, "ScOutlineWindow::ImplDrawBorder - no images" );
    DrawImage( rPos, pSymbols->GetImage( bPressed ? SC_OL_IMAGE_PRESSED : SC_OL_IMAGE_NOTPRESSED ) );
}


#define GETPREV(nLevel,nEntryNo) (nEntryNo ? pArray->GetEntry( nLevel, nEntryNo-1 ) : 0)


void __EXPORT ScOutlineWindow::Paint( const Rectangle& rRect )
{
    USHORT nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    if (!pTable)
        return;

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();
    BOOL bHor = (eMode==SC_OUTLINE_HOR);
    ScOutlineArray* pArray = bHor ? pTable->GetColArray() : pTable->GetRowArray();
    USHORT nDepth = pArray->GetDepth();
    USHORT nLevel;

    Size aSize = GetOutputSizePixel();
    long nAllSize = bHor ? aSize.Height() : aSize.Width();
    long nDestPos = nAllSize - 1;

    SetLineColor( aColor );
    if (bHor)
        DrawLine(Point(0,nDestPos), Point(aSize.Width()-1,nDestPos));
    else
        DrawLine(Point(nDestPos,0), Point(nDestPos,aSize.Height()-1));

    Point aFirstPos;
    long& nFirstLevel = bHor ? aFirstPos.Y() : aFirstPos.X();
    long& nFirstEntry = bHor ? aFirstPos.X() : aFirstPos.Y();

    Point aSecondPos;
    long& nSecondLevel = bHor ? aSecondPos.Y() : aSecondPos.X();
    long& nSecondEntry = bHor ? aSecondPos.X() : aSecondPos.Y();

    if (nHeaderSize)
    {
                                            //  Ebenen - Nummern
                                            //

        nFirstEntry = ( nHeaderSize - SC_OL_BITMAPSIZE ) / 2;
        nFirstLevel = ( nAllSize - (nDepth+1) * SC_OL_BITMAPSIZE ) / 2;

        for (nLevel=0; nLevel<=nDepth; nLevel++)
        {
            ImplDrawImage( aFirstPos, nLevel + 1 );
            nFirstLevel += SC_OL_BITMAPSIZE;
        }

        long nStart = nHeaderSize-1;
        SetLineColor( aColor );
        if (bHor)
            DrawLine(Point(nStart,0),Point(nStart,nDestPos));
        else
            DrawLine(Point(0,nStart),Point(nDestPos,nStart));
    }

    USHORT nStartIndex;
    USHORT nEndIndex;
    if (bHor)
    {
        nStartIndex = pViewData->GetPosX( WhichH(eWhich) );
        nEndIndex = nStartIndex + pViewData->VisibleCellsX( WhichH(eWhich) );
        while ( nStartIndex>0 ? (pDoc->GetColFlags(nStartIndex-1,nTab) & CR_HIDDEN) : FALSE )
            --nStartIndex;
    }
    else
    {
        nStartIndex = pViewData->GetPosY( WhichV(eWhich) );
        nEndIndex = nStartIndex + pViewData->VisibleCellsY( WhichV(eWhich) );
        while ( nStartIndex>0 ? (pDoc->GetRowFlags(nStartIndex-1,nTab) & CR_HIDDEN) : FALSE )
            --nStartIndex;
    }

        //
        //      Outline-Gruppen
        //

    nFirstLevel = ( nAllSize - (nDepth+1) * SC_OL_BITMAPSIZE ) / 2;
    for (nLevel=0; nLevel<nDepth; nLevel++)
    {
        short nEntryCount = (short) pArray->GetCount( nLevel );
        short nEntryNo;
        for (nEntryNo=0; nEntryNo<nEntryCount; nEntryNo++)
        {
            ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntryNo );
            BOOL bDoThis = FALSE;

            long nBitmapEntry;
            USHORT nStart = pEntry->GetStart();
            USHORT nEnd   = pEntry->GetEnd();

            if ( nEnd >= nStartIndex && nStart <= nEndIndex )
                if (GetEntryPos( pEntry, nFirstEntry, nSecondEntry, nBitmapEntry, GETPREV(nLevel,nEntryNo) ))
                    if (pEntry->IsVisible())                // nicht von hoeherer Ebene verdeckt
                        bDoThis = TRUE;

            if (bDoThis)
            {
                Point aOldFirstPos = aFirstPos;
                BOOL bDraw = FALSE;
                BOOL bLeftOut = ( nStart < nStartIndex );

                if (pEntry->IsHidden())
                    bDraw = FALSE;                                              // ausgeblendet
                else
                    bDraw = TRUE;                                               // eingeblendet

                if (bDraw)
                {
                    SetLineColor();
                    SetFillColor( aColor );

                    nSecondLevel = nFirstLevel;
                    nSecondLevel += 1;
                    if (!bLeftOut)
                        nFirstEntry += 1;
                    nSecondEntry -= 2;
                    DrawRect( Rectangle( aFirstPos, aSecondPos ) );             // Linie

                    if ( nEnd <= nEndIndex )
                    {
                        aFirstPos = aSecondPos;
                        nFirstEntry -= 1;
                        nSecondLevel += SC_OL_BITMAPSIZE / 3;
                        DrawRect( Rectangle( aFirstPos, aSecondPos ) );         // rechts
                    }

                    //                  links wird von Bitmap verdeckt
                }

                aFirstPos = aOldFirstPos;
            }
        }

        for (nEntryNo=nEntryCount-1; nEntryNo>=0; nEntryNo--)       // short
        {
            ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntryNo );
            BOOL bDoThis = FALSE;

            long nBitmapEntry;
            USHORT nStart = pEntry->GetStart();
            USHORT nEnd   = pEntry->GetEnd();

            if ( nEnd >= nStartIndex && nStart <= nEndIndex + 1 )   // +1 wegen Verschiebung
                if (GetEntryPos( pEntry, nFirstEntry, nSecondEntry, nBitmapEntry, GETPREV(nLevel,nEntryNo) ))
                    if (pEntry->IsVisible())                // nicht von hoeherer Ebene verdeckt
                        bDoThis = TRUE;

            if (bDoThis)
            {
                Point aOldFirstPos = aFirstPos;
                BOOL bLeftOut = ( nStart < nStartIndex );

                if (!bLeftOut)
                {
                    aFirstPos = aOldFirstPos;
                    nFirstEntry = nBitmapEntry;

                    BOOL bClip = ( nBitmapEntry < (long) nHeaderSize );
                    if (bClip)
                    {
                        if (bHor)
                            SetClipRegion( Rectangle( Point(nHeaderSize,0),
                                                Point(aSize.Width()-1,aSize.Height()-1) ) );
                        else
                            SetClipRegion( Rectangle( Point(0,nHeaderSize),
                                                Point(aSize.Width()-1,aSize.Height()-1) ) );
                    }

                    if (pEntry->IsHidden())
                        ImplDrawImage( aFirstPos, SC_OL_IMAGE_PLUS );
                    else
                        ImplDrawImage( aFirstPos, SC_OL_IMAGE_MINUS );

                    if (bClip)
                        SetClipRegion();
                }

                aFirstPos = aOldFirstPos;
            }
        }

        nFirstLevel += SC_OL_BITMAPSIZE;
    }
}


BOOL ScOutlineWindow::ButtonHit( const Point& rPos, USHORT& rLevel, USHORT& rEntry, BOOL& rHeader,
                                Point& rImagePos )
{
    USHORT nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    if (!pTable)
        return FALSE;

    BOOL bHor = (eMode==SC_OUTLINE_HOR);
    ScOutlineArray* pArray = bHor ? pTable->GetColArray() : pTable->GetRowArray();
    USHORT nDepth = pArray->GetDepth();
    Size aSize = GetOutputSizePixel();
    long nAllSize = bHor ? aSize.Height() : aSize.Width();

    USHORT nStartIndex;
    USHORT nEndIndex;
    if (bHor)
    {
        nStartIndex = pViewData->GetPosX( WhichH(eWhich) );
        nEndIndex = nStartIndex + pViewData->VisibleCellsX( WhichH(eWhich) );
        while ( nStartIndex>0 ? (pDoc->GetColFlags(nStartIndex-1,nTab) & CR_HIDDEN) : FALSE )
            --nStartIndex;
    }
    else
    {
        nStartIndex = pViewData->GetPosY( WhichV(eWhich) );
        nEndIndex = nStartIndex + pViewData->VisibleCellsY( WhichV(eWhich) );
        while ( nStartIndex>0 ? (pDoc->GetRowFlags(nStartIndex-1,nTab) & CR_HIDDEN) : FALSE )
            --nStartIndex;
    }

    long nEntryPos;
    long nLevelPos;
    long nEntryMouse = bHor ? rPos.X() : rPos.Y();
    long nLevelMouse = bHor ? rPos.Y() : rPos.X();

    USHORT nLevel;
    nLevelPos = ( nAllSize - (nDepth+1) * SC_OL_BITMAPSIZE ) / 2;
    for (nLevel=0; nLevel<=nDepth; nLevel++)
    {
        if ( nLevelMouse >= nLevelPos && nLevelMouse <= nLevelPos + SC_OL_BITMAPSIZE )
        {
            if (nHeaderSize)                // Ebenen-Nummern
            {
                nEntryPos = ( nHeaderSize - SC_OL_BITMAPSIZE ) / 2;
                if ( nEntryMouse >= nEntryPos && nEntryMouse <= nEntryPos + SC_OL_BITMAPSIZE )
                {
                    rLevel = nLevel;
                    rEntry = 0;
                    rHeader = TRUE;
                    if (bHor)
                        rImagePos = Point( nEntryPos, nLevelPos );
                    else
                        rImagePos = Point( nLevelPos, nEntryPos );
                    return TRUE;
                }
            }

            if (nLevel<nDepth)              // Outline-Handles
            {
                USHORT nEntryCount = pArray->GetCount( nLevel );
                for (USHORT nEntryNo=0; nEntryNo<nEntryCount; nEntryNo++)
                {
                    ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntryNo );

                    USHORT nStart = pEntry->GetStart();
                    if ( nStart >= nStartIndex && nStart <= nEndIndex )
                    {
                        long nDummy1;
                        long nDummy2;
                        if (GetEntryPos( pEntry, nDummy1, nDummy2, nEntryPos, GETPREV(nLevel,nEntryNo) ))
                        {
                            if (pEntry->IsVisible())
                            {
                                if ( nEntryMouse >= nEntryPos && nEntryMouse <= nEntryPos + SC_OL_BITMAPSIZE )
                                {
                                    rLevel = nLevel;
                                    rEntry = nEntryNo;
                                    rHeader = FALSE;
                                    if (bHor)
                                        rImagePos = Point( nEntryPos, nLevelPos );
                                    else
                                        rImagePos = Point( nLevelPos, nEntryPos );
                                    return TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
        nLevelPos += SC_OL_BITMAPSIZE;
    }

    return FALSE;
}


BOOL ScOutlineWindow::LineHit( const Point& rPos, USHORT& rLevel, USHORT& rEntry )
{
    USHORT nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    if (!pTable)
        return FALSE;

    BOOL bHor = (eMode==SC_OUTLINE_HOR);
    ScOutlineArray* pArray = bHor ? pTable->GetColArray() : pTable->GetRowArray();
    USHORT nDepth = pArray->GetDepth();
    Size aSize = GetOutputSizePixel();
    long nAllSize = bHor ? aSize.Height() : aSize.Width();

    USHORT nStartIndex;
    USHORT nEndIndex;
    if (bHor)
    {
        nStartIndex = pViewData->GetPosX( WhichH(eWhich) );
        nEndIndex = nStartIndex + pViewData->VisibleCellsX( WhichH(eWhich) );
    }
    else
    {
        nStartIndex = pViewData->GetPosY( WhichV(eWhich) );
        nEndIndex = nStartIndex + pViewData->VisibleCellsY( WhichV(eWhich) );
    }

    long nEntryStart;
    long nEntryEnd;
    long nLevelPos;
    long nEntryMouse = bHor ? rPos.X() : rPos.Y();
    long nLevelMouse = bHor ? rPos.Y() : rPos.X();

    USHORT nLevel;
    nLevelPos = ( nAllSize - (nDepth+1) * SC_OL_BITMAPSIZE ) / 2;
    for (nLevel=0; nLevel<nDepth; nLevel++)
    {
        if ( nLevelMouse >= nLevelPos && nLevelMouse <= nLevelPos + SC_OL_BITMAPSIZE / 2 )
        {
            ScOutlineEntry* pEntry;
            USHORT nEntryNo = pArray->GetCount( nLevel );           // Rueckwaerts !
            while (nEntryNo)
            {
                --nEntryNo;
                pEntry = pArray->GetEntry( nLevel, nEntryNo );

                USHORT nStart = pEntry->GetStart();
                USHORT nEnd   = pEntry->GetEnd();
                if ( nEnd >= nStartIndex && nStart <= nEndIndex )
                {
                    long nDummy;
                    if (GetEntryPos( pEntry, nEntryStart, nEntryEnd, nDummy, GETPREV(nLevel,nEntryNo) ))
                    {
                        if (pEntry->IsVisible())
                        {
                            if ( nEntryMouse >= nEntryStart && nEntryMouse < nEntryEnd )
                            {
                                rLevel = nLevel;
                                rEntry = nEntryNo;
                                return TRUE;
                            }
                        }
                    }
                }
            }
        }
        nLevelPos += SC_OL_BITMAPSIZE;
    }

    return FALSE;
}


void ScOutlineWindow::DoFunction( USHORT nLevel, USHORT nEntry, BOOL bHeader )
{
    BOOL bHor = (eMode==SC_OUTLINE_HOR);

    if (bHeader)
        pViewData->GetView()->SelectLevel( bHor, nLevel );
    else
    {
        ScDocument* pDoc = pViewData->GetDocument();
        USHORT nTab = pViewData->GetTabNo();
        ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
        ScOutlineArray* pArray = bHor ? pTable->GetColArray() : pTable->GetRowArray();
        ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntry );

        if (pEntry->IsHidden())
            pViewData->GetView()->ShowOutline( bHor, nLevel, nEntry );
        else
            pViewData->GetView()->HideOutline( bHor, nLevel, nEntry );
    }
}


void __EXPORT ScOutlineWindow::MouseMove( const MouseEvent& rMEvt )
{
    USHORT nLevel;
    USHORT nEntry;
    BOOL bHeader;
    Point aDummy;

    if ( bHitMode )
    {
        BOOL bHit = FALSE;
        if ( ButtonHit( rMEvt.GetPosPixel(), nLevel, nEntry, bHeader, aDummy ) )
            if ( nLevel == nHitLevel && nEntry == nHitEntry && bHeader == bHitHeader )
                bHit = TRUE;

        if (bHit)
        {
            if (!bIsInverted)
                ImplDrawBorder( aImagePos, true );
            bIsInverted = TRUE;
        }
        else
        {
            if (bIsInverted)
                ImplDrawBorder( aImagePos, false );
            bIsInverted = FALSE;
        }
    }
}


void __EXPORT ScOutlineWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    USHORT nLevel;
    USHORT nEntry;
    BOOL bHeader;
    Point aDummy;

    if ( bHitMode )
    {
        if ( bIsInverted )
            ImplDrawBorder( aImagePos, false );

        if ( ButtonHit( rMEvt.GetPosPixel(), nLevel, nEntry, bHeader, aDummy ) )
            if ( nLevel == nHitLevel && nEntry == nHitEntry && bHeader == bHitHeader )
                DoFunction( nLevel, nEntry, bHeader );

        bHitMode = FALSE;
    }
}


void __EXPORT ScOutlineWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    USHORT nLevel;
    USHORT nEntry;
    BOOL bHeader;

    if ( ButtonHit( rMEvt.GetPosPixel(), nLevel, nEntry, bHeader, aImagePos ) )
    {
        bHitMode = TRUE;
        nHitLevel = nLevel;
        nHitEntry = nEntry;
        bHitHeader = bHeader;

        ImplDrawBorder( aImagePos, true );
        bIsInverted = TRUE;
    }
    else if ( rMEvt.GetClicks() == 2 )
    {
        if ( LineHit( rMEvt.GetPosPixel(), nLevel, nEntry ) )
            DoFunction( nLevel, nEntry, FALSE );
    }
}


void ScOutlineWindow::SetHeaderSize( USHORT nNewSize )
{
    if ( nNewSize != nHeaderSize )
    {
        nHeaderSize = nNewSize;
        Invalidate();
    }
}


long ScOutlineWindow::GetDepthSize()
{
    long nSize = 0;

    const ScOutlineTable* pTable = pViewData->GetDocument()->GetOutlineTable(pViewData->GetTabNo());
    if (pTable)
    {
        BOOL bHor = (eMode==SC_OUTLINE_HOR);
        const ScOutlineArray* pArray = bHor ? pTable->GetColArray() : pTable->GetRowArray();
        USHORT nDepth = pArray->GetDepth();

        if ( nDepth )
            nSize = ( (nDepth+1) * SC_OL_BITMAPSIZE ) + 5;
    }

    return nSize;
}


void ScOutlineWindow::ScrollPixel( long nDiff )
{
    BOOL bHor = (eMode==SC_OUTLINE_HOR);
    if (nHeaderSize)
    {
        long nStart = nHeaderSize;
        long nEnd;
        long nSize;
        Size aSize = GetOutputSizePixel();
        if (bHor)
        {
            nEnd = aSize.Width() - 1;
            nSize = aSize.Height() - 1;
        }
        else
        {
            nEnd = aSize.Height() - 1;
            nSize = aSize.Width() - 1;
        }
        long nInvStart;
        long nInvEnd;

        if (nDiff < 0)
        {
            nStart -= nDiff;
            nInvStart = nEnd + nDiff;
            nInvEnd = nEnd;
        }
        else
        {
            nEnd -= nDiff;
            nInvStart = nStart;
            nInvEnd = nStart + nDiff;
        }

        if (bHor)
        {
            Scroll( nDiff, 0, Rectangle( nStart, 0, nEnd, nSize ) );
            Invalidate( Rectangle( nInvStart, 0, nInvEnd, nSize ) );
        }
        else
        {
            Scroll( 0, nDiff, Rectangle( 0, nStart, nSize, nEnd ) );
            Invalidate( Rectangle( 0, nInvStart, nSize, nInvEnd ) );
        }
        Update();
    }
    else
    {
        if (bHor)
            Scroll( nDiff, 0 );
        else
            Scroll( 0, nDiff );
    }
}

void ScOutlineWindow::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( rStyleSettings.GetFaceColor() );
    aColor = rStyleSettings.GetButtonTextColor();
    pSymbols = ScGlobal::GetOutlineSymbols( GetBackground().GetColor().IsDark() );

    Invalidate();
}

// -----------------------------------------------------------------------

void ScOutlineWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}
