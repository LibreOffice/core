/*************************************************************************
 *
 *  $RCSfile: svtabbx.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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

/*
    Todo
    - Tokenparsing optimieren
    - Optimierungsfehler Microsoft/Win3.1 Funktion suchen
*/

// !!!HACK
#ifdef _MSC_VER
#pragma optimize( "", off )
#endif
#pragma hdrstop

#include "svtabbx.hxx"
#include "headbar.hxx"

#define MYTABMASK (SV_LBOXTAB_ADJUST_RIGHT | SV_LBOXTAB_ADJUST_LEFT |\
                 SV_LBOXTAB_ADJUST_CENTER | SV_LBOXTAB_ADJUST_NUMERIC)

// SvTreeListBox-Callback

void SvTabListBox::SetTabs()
{
    SvTreeListBox::SetTabs();
    if( nTabCount )
    {
        DBG_ASSERT(pTabList,"TabList ?");

        // die TreeListBox hat jetzt ihre Tabulatoren in die Liste eingefuegt.
        // jetzt plustern wir die Liste mit zusaetzlichen Tabulatoren auf,
        // und passen den ganz rechten Tab der Treelistbox an.

        // den ganz rechten Tab nehmen
        // HACK fuer den Explorer! Wenn der ViewParent != 0 ist, dann wird
        // der erste Tab der TreeListBox von der TreelistBox berechnet!
        // Dies wird fuer ButtonsOnRoot benoetigt, da der Explorer nicht
        // weiss, welchen zusaetzlichen Offset er in diesem Modus auf
        // den Tabulator addieren muss. Die TreeListBox weiss es!
        /*
        if( !pViewParent )
        {
        SvLBoxTab* pFirstTab = (SvLBoxTab*)aTabs.GetObject( aTabs.Count()-1 );
        pFirstTab->SetPos( pTabList[0].GetPos() );
        pFirstTab->nFlags &= ~MYTABMASK;
        pFirstTab->nFlags |= pTabList[0].nFlags;
        }
        */

        // alle anderen Tabs an Liste haengen
        for( USHORT nCurTab = 1; nCurTab < nTabCount; nCurTab++ )
        {
            SvLBoxTab* pTab = pTabList+nCurTab;
            AddTab( pTab->GetPos(), pTab->nFlags );
        }
    }
}

void SvTabListBox::InitEntry( SvLBoxEntry* pEntry, const XubString& rStr,
    const Image& rColl, const Image& rExp )
{
    SvTreeListBox::InitEntry( pEntry, rStr, rColl, rExp);
    XubString aToken;

    const xub_Unicode* pCurToken = aCurEntry.GetBuffer();
    USHORT nCurTokenLen;
    const xub_Unicode* pNextToken = GetToken( pCurToken, nCurTokenLen );
    USHORT nCount = nTabCount; nCount--;
    for( USHORT nToken = 0; nToken < nCount; nToken++ )
    {
        if( pCurToken && nCurTokenLen )
            // aToken.Assign( pCurToken, nCurTokenLen );
            aToken = XubString( pCurToken, nCurTokenLen );
        else
            aToken.Erase();
        SvLBoxString* pStr = new SvLBoxString( pEntry, 0, aToken );
        pEntry->AddItem( pStr );
        pCurToken = pNextToken;
        if( pCurToken )
            pNextToken = GetToken( pCurToken, nCurTokenLen );
        else
            nCurTokenLen = 0;
    }
}


SvTabListBox::SvTabListBox( Window* pParent, WinBits nBits )
    : SvTreeListBox( pParent, nBits )
{
    pTabList = 0;
    nTabCount = 0;
    pViewParent = 0;
    SetHighlightRange();    // ueber volle Breite selektieren
}

SvTabListBox::SvTabListBox( Window* pParent, const ResId& rResId )
    : SvTreeListBox( pParent, rResId )
{
    pTabList = 0;
    nTabCount = 0;
    pViewParent = 0;
    SvTabListBox::Resize();
    SetHighlightRange();
}

SvTabListBox::~SvTabListBox()
{
    // array-delete
    __DELETE(nTabCount) pTabList;
#ifdef DBG_UTIL
    pTabList = 0;
    nTabCount = 0;
#endif
}

void SvTabListBox::SetTabs( long* pTabs, MapUnit eMapUnit )
{
    DBG_ASSERT(pTabs,"SetTabs:NULL-Ptr");
    if( !pTabs )
        return;

    __DELETE(nTabCount) pTabList;
    USHORT nCount = (USHORT)(*pTabs);
    pTabList = new SvLBoxTab[ nCount ];
    nTabCount = nCount;

    MapMode aMMSource( eMapUnit );
    MapMode aMMDest( MAP_PIXEL );

    pTabs++;
    for( USHORT nIdx = 0; nIdx < nCount; nIdx++, pTabs++ )
    {
        Size aSize( *pTabs, 0 );
        aSize = LogicToLogic( aSize, &aMMSource, &aMMDest );
        long nNewTab = aSize.Width();
        pTabList[nIdx].SetPos( nNewTab );
        pTabList[nIdx].nFlags=(SV_LBOXTAB_ADJUST_LEFT| SV_LBOXTAB_INV_ALWAYS);
    }
    SvTreeListBox::nTreeFlags |= TREEFLAG_RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

void SvTabListBox::SetTab( USHORT nTab,long nValue,MapUnit eMapUnit )
{
    DBG_ASSERT(nTab<nTabCount,"Invalid Tab-Pos");
    if( nTab < nTabCount )
    {
        DBG_ASSERT(pTabList,"TabList?");
        MapMode aMMSource( eMapUnit );
        MapMode aMMDest( MAP_PIXEL );
        Size aSize( nValue, 0 );
        aSize = LogicToLogic( aSize, &aMMSource, &aMMDest );
        nValue = aSize.Width();
        pTabList[ nTab ].SetPos( nValue );
        SvTreeListBox::nTreeFlags |= TREEFLAG_RECALCTABS;
        if( IsUpdateMode() )
            Invalidate();
    }
}

#if SUPD < 375

SvLBoxEntry* SvTabListBox::InsertEntry(const XubString& rStr,SvLBoxEntry* pParent,ULONG nPos,USHORT nCol )
{
    XubString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += '\t';
            nCol--;
        }
    }
    aStr += rStr;
    XubString aFirstStr( aStr );
    USHORT nEnd = aFirstStr.Search( '\t' );
    if( nEnd != STRING_NOTFOUND )
    {
        aFirstStr.Cut( nEnd );
        aCurEntry = aStr;
        aCurEntry.Erase( 0, ++nEnd );
    }
    else
        aCurEntry.Erase();
    return SvTreeListBox::InsertEntry( aFirstStr, pParent, FALSE, nPos );
}

SvLBoxEntry* SvTabListBox::InsertEntry( const XubString& rStr,
    const Image& rExpandedEntryBmp, const Image& rCollapsedEntryBmp,
    SvLBoxEntry* pParent,ULONG nPos,USHORT nCol )
{
    XubString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += '\t';
            nCol--;
        }
    }
    aStr += rStr;
    XubString aFirstStr( aStr );
    USHORT nEnd = aFirstStr.Search( '\t' );
    if( nEnd != STRING_NOTFOUND )
    {
        aFirstStr.Cut( nEnd );
        aCurEntry = aStr;
        aCurEntry.Erase( 0, ++nEnd );
    }
    else
        aCurEntry.Erase();

    return SvTreeListBox::InsertEntry(
        aFirstStr,
        rExpandedEntryBmp, rCollapsedEntryBmp,
        pParent, FALSE, nPos );
}


SvLBoxEntry* SvTabListBox::InsertEntry( const XubString& rStr, ULONG nPos,
    USHORT nCol )
{
    return InsertEntry( rStr,0,nPos, nCol );
}

SvLBoxEntry* SvTabListBox::InsertEntry(const XubString& rStr,SvLBoxEntry* pParent,ULONG nPos,USHORT nCol,
    void* pUser )
{
    XubString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += '\t';
            nCol--;
        }
    }
    aStr += rStr;
    XubString aFirstStr( aStr );
    USHORT nEnd = aFirstStr.Search( '\t' );
    if( nEnd != STRING_NOTFOUND )
    {
        aFirstStr.Cut( nEnd );
        aCurEntry = aStr;
        aCurEntry.Erase( 0, ++nEnd );
    }
    else
        aCurEntry.Erase();
    return SvTreeListBox::InsertEntry( aFirstStr, pParent, FALSE, nPos, pUser );
}

SvLBoxEntry* SvTabListBox::InsertEntry( const XubString& rStr,
    const Image& rExpandedEntryBmp, const Image& rCollapsedEntryBmp,
    SvLBoxEntry* pParent,ULONG nPos,USHORT nCol, void* pUser )
{
    XubString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += '\t';
            nCol--;
        }
    }
    aStr += rStr;
    XubString aFirstStr( aStr );
    USHORT nEnd = aFirstStr.Search( '\t' );
    if( nEnd != STRING_NOTFOUND )
    {
        aFirstStr.Cut( nEnd );
        aCurEntry = aStr;
        aCurEntry.Erase( 0, ++nEnd );
    }
    else
        aCurEntry.Erase();

    return SvTreeListBox::InsertEntry(
        aFirstStr,
        rExpandedEntryBmp, rCollapsedEntryBmp,
        pParent, FALSE, nPos, pUser );
}


SvLBoxEntry* SvTabListBox::InsertEntry( const XubString& rStr, ULONG nPos,
    USHORT nCol, void* pUser )
{
    return InsertEntry( rStr,0,nPos, nCol, pUser );
}

#else

SvLBoxEntry* SvTabListBox::InsertEntry(const XubString& rStr,SvLBoxEntry* pParent,ULONG nPos,USHORT nCol,
    void* pUser )
{
    XubString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += '\t';
            nCol--;
        }
    }
    aStr += rStr;
    XubString aFirstStr( aStr );
    USHORT nEnd = aFirstStr.Search( '\t' );
    if( nEnd != STRING_NOTFOUND )
    {
        aFirstStr.Erase( nEnd );
        aCurEntry = aStr;
        aCurEntry.Erase( 0, ++nEnd );
    }
    else
        aCurEntry.Erase();
    return SvTreeListBox::InsertEntry( aFirstStr, pParent, FALSE, nPos, pUser );
}

SvLBoxEntry* SvTabListBox::InsertEntry( const XubString& rStr,
    const Image& rExpandedEntryBmp, const Image& rCollapsedEntryBmp,
    SvLBoxEntry* pParent,ULONG nPos,USHORT nCol, void* pUser )
{
    XubString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += '\t';
            nCol--;
        }
    }
    aStr += rStr;
    XubString aFirstStr( aStr );
    USHORT nEnd = aFirstStr.Search( '\t' );
    if( nEnd != STRING_NOTFOUND )
    {
        aFirstStr.Erase( nEnd );
        aCurEntry = aStr;
        aCurEntry.Erase( 0, ++nEnd );
    }
    else
        aCurEntry.Erase();

    return SvTreeListBox::InsertEntry(
        aFirstStr,
        rExpandedEntryBmp, rCollapsedEntryBmp,
        pParent, FALSE, nPos, pUser );
}


SvLBoxEntry* SvTabListBox::InsertEntry( const XubString& rStr, ULONG nPos,
    USHORT nCol, void* pUser )
{
    return InsertEntry( rStr,0,nPos, nCol, pUser );
}

#endif

XubString SvTabListBox::GetEntryText( ULONG nPos, USHORT nCol ) const
{
    SvLBoxEntry* pEntry = SvTreeListBox::GetEntry( nPos );
    return GetEntryText( pEntry, nCol );
}

XubString SvTabListBox::GetEntryText( SvLBoxEntry* pEntry, USHORT nCol ) const
{
    DBG_ASSERT(pEntry,"GetEntryText:Invalid Entry");
    XubString aResult;
    if( pEntry )
    {
        USHORT nCount = pEntry->ItemCount();
        USHORT nCur = 0;
        while( nCur < nCount )
        {
            SvLBoxItem* pStr = pEntry->GetItem( nCur );
            if( pStr->IsA() == SV_ITEM_ID_LBOXSTRING )
            {
                if( nCol == 0xffff )
                {
                    if( aResult.Len() )
                        aResult += '\t';
                    aResult += ((SvLBoxString*)pStr)->GetText();
                }
                else
                {
                    if( nCol == 0 )
                        return ((SvLBoxString*)pStr)->GetText();
                    nCol--;
                }
            }
            nCur++;
        }
    }
    return aResult;
}

void SvTabListBox::SetEntryText( const XubString& rStr, ULONG nPos,
    USHORT nCol )
{
    SvLBoxEntry* pEntry = SvTreeListBox::GetEntry( nPos );
    SetEntryText( rStr, pEntry, nCol );
}

void SvTabListBox::SetEntryText( const XubString& rStr, SvLBoxEntry* pEntry,
    USHORT nCol )
{
    DBG_ASSERT(pEntry,"SetEntryText:Invalid Entry");
    if( !pEntry )
        return;

    const xub_Unicode* pCurToken = rStr.GetBuffer();
    USHORT nCurTokenLen;
    const xub_Unicode* pNextToken = GetToken( pCurToken, nCurTokenLen );

    XubString aTemp;
    USHORT nCount = pEntry->ItemCount();
    USHORT nCur = 0;
    while( nCur < nCount )
    {
        SvLBoxItem* pStr = pEntry->GetItem( nCur );
        if( pStr && pStr->IsA() == SV_ITEM_ID_LBOXSTRING )
        {
            if( nCol == 0xffff )
            {
                if( pCurToken )
                    aTemp = XubString( pCurToken, nCurTokenLen );
                else
                    aTemp.Erase(); // alle Spalten ohne Token loeschen
                ((SvLBoxString*)pStr)->SetText( pEntry, aTemp );
                pCurToken = pNextToken;
                pNextToken = GetToken( pCurToken, nCurTokenLen );
            }
            else
            {
                if( !nCol )
                {
                    aTemp = XubString( pCurToken, nCurTokenLen );
                    ((SvLBoxString*)pStr)->SetText( pEntry, aTemp );
                    if( !pNextToken )
                        break;
                    pCurToken = pNextToken;
                    pNextToken = GetToken( pCurToken, nCurTokenLen );
                }
                else
                    nCol--;
            }
        }
        nCur++;
    }
    GetModel()->InvalidateEntry( pEntry );
}



ULONG SvTabListBox::GetEntryPos( const XubString& rStr, USHORT nCol )
{
    ULONG nPos = 0;
    SvLBoxEntry* pEntry = First();
    while( pEntry )
    {
        XubString aStr( GetEntryText( pEntry, nCol ));
        if( aStr == rStr )
            return nPos;
        pEntry = Next( pEntry );
        nPos++;
    }
    return 0xffffffff;
}

void __EXPORT SvTabListBox::Resize()
{
    SvTreeListBox::Resize();
}

// static
const xub_Unicode* SvTabListBox::GetToken( const xub_Unicode* pPtr, USHORT& rLen )
{
    if( !pPtr || *pPtr == 0 )
    {
        rLen = 0;
        return 0;
    }
    xub_Unicode c = *pPtr;
    USHORT nLen = 0;
    while( c != '\t' && c != 0 )
    {
        pPtr++;
        nLen++;
        c = *pPtr;
    }
    if( c )
        pPtr++; // Tab ueberspringen
    else
        pPtr = 0;
    rLen = nLen;
    return pPtr;
}


void SvTabListBox::SetTabJustify( USHORT nTab, SvTabJustify eJustify)
{
    if( nTab >= nTabCount )
        return;
    SvLBoxTab* pTab = &(pTabList[ nTab ]);
    USHORT nFlags = pTab->nFlags;
    nFlags &= (~MYTABMASK);
    nFlags |= (USHORT)eJustify;
    pTab->nFlags = nFlags;
    SvTreeListBox::nTreeFlags |= TREEFLAG_RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

SvTabJustify SvTabListBox::GetTabJustify( USHORT nTab ) const
{
    SvTabJustify eResult = AdjustLeft;
    if( nTab >= nTabCount )
        return eResult;
    SvLBoxTab* pTab = &(pTabList[ nTab ]);
    USHORT nFlags = pTab->nFlags;
    nFlags &= MYTABMASK;
    eResult = (SvTabJustify)nFlags;
    return eResult;
}

long SvTabListBox::GetLogicTab( USHORT nTab )
{
    if( SvTreeListBox::nTreeFlags & TREEFLAG_RECALCTABS )
        ((SvTabListBox*)this)->SetTabs();

    DBG_ASSERT(nTab<nTabCount,"GetTabPos:Invalid Tab");
    return ((SvLBoxTab*)aTabs.GetObject( nTab ))->GetPos();
}

// class SvHeaderTabListBox ----------------------------------------------

SvHeaderTabListBox::SvHeaderTabListBox( Window* pParent, WinBits nWinStyle ) :

    SvTabListBox( pParent, nWinStyle ),

    mbFirstPaint( TRUE )

{
}

// -----------------------------------------------------------------------

SvHeaderTabListBox::~SvHeaderTabListBox()
{
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::Paint( const Rectangle& rRect )
{
    if ( mbFirstPaint )
    {
        mbFirstPaint = FALSE;
        RepaintScrollBars();
    }
    SvTabListBox::Paint( rRect );
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::InitHeaderBar( HeaderBar* pHeaderBar )
{
    mpHeaderBar = pHeaderBar;
    SetScrolledHdl( LINK( this, SvHeaderTabListBox, ScrollHdl_Impl ) );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvHeaderTabListBox, ScrollHdl_Impl, SvTabListBox*, pList )
{
    mpHeaderBar->SetOffset( -GetXOffset() );
    return 0;
}


