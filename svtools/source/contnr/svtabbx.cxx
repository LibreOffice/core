/*************************************************************************
 *
 *  $RCSfile: svtabbx.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 15:48:25 $
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

#ifndef _SVTABBX_HXX
#include "svtabbx.hxx"
#endif
#ifndef _HEADBAR_HXX
#include "headbar.hxx"
#endif
#ifndef _SVTOOLS_SVTDATA_HXX
#include "svtdata.hxx"
#endif
#ifndef _SVTOOLS_HRC
#include "svtools.hrc"
#endif

#ifndef _SVTOOLS_ACCESSIBLETABLISTBOX_HXX
#include "accessibletablistbox.hxx"
#endif
#ifndef _SVTOOLS_ACCESSIBILEBROWSEBOXTABLECELL_HXX
#include "AccessibleBrowseBoxTableCell.hxx"
#endif
#ifndef _SVTOOLS_ACCESSIBILEBROWSEBOXHEADERCELL_HXX
#include "AccessibleBrowseBoxHeaderCell.hxx"
#endif
#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOXHEADERBAR_HXX
#include "AccessibleBrowseBoxHeaderBar.hxx"
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

#define MYTABMASK \
    ( SV_LBOXTAB_ADJUST_RIGHT | SV_LBOXTAB_ADJUST_LEFT | SV_LBOXTAB_ADJUST_CENTER | SV_LBOXTAB_ADJUST_NUMERIC )

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

String SvTabListBox::GetEntryText( SvLBoxEntry* pEntry ) const
{
    return GetEntryText( pEntry, 0xffff );
}

String SvTabListBox::GetEntryText( SvLBoxEntry* pEntry, USHORT nCol ) const
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
                    aResult += static_cast<SvLBoxString*>( pStr )->GetText();
                }
                else
                {
                    if( nCol == 0 )
                        return static_cast<SvLBoxString*>( pStr )->GetText();
                    nCol--;
                }
            }
            nCur++;
        }
    }
    return aResult;
}

String SvTabListBox::GetEntryText( ULONG nPos, USHORT nCol ) const
{
    SvLBoxEntry* pEntry = SvTreeListBox::GetEntry( nPos );
    return GetEntryText( pEntry, nCol );
}

void SvTabListBox::SetEntryText( const XubString& rStr, ULONG nPos, USHORT nCol )
{
    SvLBoxEntry* pEntry = SvTreeListBox::GetEntry( nPos );
    SetEntryText( rStr, pEntry, nCol );
}

void SvTabListBox::SetEntryText( const XubString& rStr, SvLBoxEntry* pEntry, USHORT nCol )
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

ULONG SvTabListBox::GetEntryPos( const SvLBoxEntry* pEntry ) const
{
    ULONG nPos = 0;
    SvLBoxEntry* pTmpEntry = First();
    while( pTmpEntry )
    {
        if ( pTmpEntry == pEntry )
            return nPos;
        pTmpEntry = Next( pTmpEntry );
        ++nPos;
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

String SvTabListBox::GetTabEntryText( ULONG nPos, USHORT nCol ) const
{
    SvLBoxEntry* pEntry = SvTreeListBox::GetEntry( nPos );
    DBG_ASSERT( pEntry, "GetTabEntryText(): Invalid entry " );
    XubString aResult;
    if ( pEntry )
    {
        USHORT nCount = pEntry->ItemCount();
        USHORT nCur = ( 0 == nCol && IsCellFocusEnabled() ) ? GetCurrentTabPos() : 0;
        while( nCur < nCount )
        {
            SvLBoxItem* pStr = pEntry->GetItem( nCur );
            if ( pStr->IsA() == SV_ITEM_ID_LBOXSTRING )
            {
                if ( nCol == 0xffff )
                {
                    if ( aResult.Len() )
                        aResult += '\t';
                    aResult += static_cast<SvLBoxString*>( pStr )->GetText();
                }
                else
                {
                    if ( nCol == 0 )
                    {
                        String sRet = static_cast<SvLBoxString*>( pStr )->GetText();
                        if ( sRet.Len() == 0 )
                            sRet = String( SvtResId( STR_SVT_ACC_EMPTY_FIELD ) );
                        return sRet;
                    }
                    --nCol;
                }
            }
            ++nCur;
        }
    }
    return aResult;
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

    m_bFirstPaint   ( TRUE ),
    m_pHeaderBar    ( NULL ),
    m_pAccessible   ( NULL )

{
}

// -----------------------------------------------------------------------

SvHeaderTabListBox::SvHeaderTabListBox( Window* pParent, const ResId& rResId ) :

    SvTabListBox( pParent, rResId ),

    m_bFirstPaint   ( TRUE ),
    m_pHeaderBar    ( NULL ),
    m_pAccessible   ( NULL )

{
}

// -----------------------------------------------------------------------

SvHeaderTabListBox::~SvHeaderTabListBox()
{
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::Paint( const Rectangle& rRect )
{
    if ( m_bFirstPaint )
    {
        m_bFirstPaint = FALSE;
        RepaintScrollBars();
    }
    SvTabListBox::Paint( rRect );
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::InitHeaderBar( HeaderBar* pHeaderBar )
{
    DBG_ASSERT( !m_pHeaderBar, "header bar already initialized" );
    DBG_ASSERT( pHeaderBar, "invalid header bar initialization" );
    m_pHeaderBar = pHeaderBar;
    SetScrolledHdl( LINK( this, SvHeaderTabListBox, ScrollHdl_Impl ) );
    m_pHeaderBar->SetCreateAccessibleHdl( LINK( this, SvHeaderTabListBox, CreateAccessibleHdl_Impl ) );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvHeaderTabListBox, ScrollHdl_Impl, SvTabListBox*, EMPTYARG )
{
    m_pHeaderBar->SetOffset( -GetXOffset() );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvHeaderTabListBox, CreateAccessibleHdl_Impl, HeaderBar*, EMPTYARG )
{
    Window* pParent = m_pHeaderBar->GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvHeaderTabListBox..CreateAccessibleHdl_Impl - accessible parent not found" );
    if ( pParent )
    {
        ::com::sun::star::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            Reference< XAccessible > xAccessible =
                new ::svt::AccessibleBrowseBoxHeaderBar( xAccParent, *this, ::svt::BBTYPE_COLUMNHEADERBAR );
            m_pHeaderBar->SetAccessible( xAccessible );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------
long SvHeaderTabListBox::GetRowCount() const
{
    return GetEntryCount();
}
// -----------------------------------------------------------------------
sal_uInt16 SvHeaderTabListBox::GetColumnCount() const
{
    return m_pHeaderBar->GetItemCount();
}
// -----------------------------------------------------------------------
sal_Int32 SvHeaderTabListBox::GetCurrRow() const
{
    sal_Int32 nRet = -1;
    SvLBoxEntry* pEntry = GetCurEntry();
    if ( pEntry )
    {
        ULONG nCount = GetEntryCount();
        for ( ULONG i = 0; i < nCount; ++i )
        {
            if ( pEntry == GetEntry(i) )
            {
                nRet = i;
                break;
            }
        }
    }

    return nRet;
}
// -----------------------------------------------------------------------
sal_uInt16 SvHeaderTabListBox::GetCurrColumn() const
{
    sal_uInt16 nPos = GetCurrentTabPos() - 1;
    return nPos;
}
// -----------------------------------------------------------------------
::rtl::OUString SvHeaderTabListBox::GetRowDescription( sal_Int32 _nRow ) const
{
    return ::rtl::OUString( GetEntryText( _nRow ) );
}
// -----------------------------------------------------------------------
::rtl::OUString SvHeaderTabListBox::GetColumnDescription( sal_uInt16 _nColumn ) const
{
    return ::rtl::OUString( m_pHeaderBar->GetItemText( m_pHeaderBar->GetItemId( _nColumn ) ) );
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::HasRowHeader() const
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::IsCellFocusable() const
{
    return IsCellFocusEnabled();
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumn )
{
    sal_Bool bRet = ( IsCellFocusEnabled() == TRUE );
    if ( bRet )
    {
        // first set cursor to _nRow
        SetCursor( GetEntry( _nRow ), TRUE );
        // then set the focus into _nColumn
        bRet = ( SetCurrentTabPos( _nColumn ) == true );
    }
    return bRet;
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::SetNoSelection()
{
    SvLBox::SelectAll( FALSE );
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::SelectAll()
{
    SvLBox::SelectAll( TRUE );
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::SelectAll( BOOL bSelect, BOOL bPaint )
{
    // overwritten just to disambiguate the SelectAll() from the base' class SelectAll( BOOl, BOOL )
    SvTabListBox::SelectAll( bSelect, bPaint );
}

// -----------------------------------------------------------------------
void SvHeaderTabListBox::SelectRow( long _nRow, BOOL _bSelect, BOOL _bExpand )
{
    Select( GetEntry( _nRow ), _bSelect );
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::SelectColumn( sal_uInt16 _nColumn, sal_Bool _bSelect )
{
}
// -----------------------------------------------------------------------
sal_Int32 SvHeaderTabListBox::GetSelectedRowCount() const
{
    return GetSelectionCount();
}
// -----------------------------------------------------------------------
sal_Int32 SvHeaderTabListBox::GetSelectedColumnCount() const
{
    return 0;
}
// -----------------------------------------------------------------------
BOOL SvHeaderTabListBox::IsRowSelected( long _nRow ) const
{
    SvLBoxEntry* pEntry = GetEntry( _nRow );
    return ( pEntry && IsSelected( pEntry ) );
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::IsColumnSelected( long _nColumn ) const
{
    return FALSE;
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::GetAllSelectedRows( ::com::sun::star::uno::Sequence< sal_Int32 >& _rRows ) const
{
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::GetAllSelectedColumns( ::com::sun::star::uno::Sequence< sal_Int32 >& _rColumns ) const
{
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumn ) const
{
    return sal_True;
}
// -----------------------------------------------------------------------
String SvHeaderTabListBox::GetCellText( long _nRow, USHORT _nColumn ) const
{
    return ::rtl::OUString( GetTabEntryText( _nRow, _nColumn ) );
}
// -----------------------------------------------------------------------
Rectangle SvHeaderTabListBox::calcHeaderRect( sal_Bool _bIsColumnBar, BOOL _bOnScreen )
{
    Rectangle aRect;
    if ( _bIsColumnBar )
    {
        Window* pParent = NULL;
        if ( !_bOnScreen )
            pParent = m_pHeaderBar->GetAccessibleParentWindow();

        aRect = m_pHeaderBar->GetWindowExtentsRelative( pParent );
    }
    return aRect;
}
// -----------------------------------------------------------------------
Rectangle SvHeaderTabListBox::calcTableRect( BOOL _bOnScreen )
{
    Window* pParent = NULL;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    Rectangle aRect( GetWindowExtentsRelative( pParent ) );
    return aRect;
}
// -----------------------------------------------------------------------
Rectangle SvHeaderTabListBox::GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumn, BOOL _bIsHeader, BOOL _bOnScreen )
{
    DBG_ASSERT( !_bIsHeader || 0 == _nRow, "invalid parameters" );
    Rectangle aRect;
    SvLBoxEntry* pEntry = GetEntry( _nRow );
    if ( pEntry )
    {
        aRect = _bIsHeader ? calcHeaderRect( sal_True, FALSE ) : GetBoundingRect( pEntry );
        Point aTopLeft = aRect.TopLeft();
        DBG_ASSERT( m_pHeaderBar->GetItemCount() > _nColumn, "invalid column" );
        Rectangle aItemRect = m_pHeaderBar->GetItemRect( m_pHeaderBar->GetItemId( _nColumn ) );
        aTopLeft.X() = aItemRect.Left();
        Size aSize = aItemRect.GetSize();
        aRect = Rectangle( aTopLeft, aSize );
        Window* pParent = NULL;
        if ( !_bOnScreen )
            pParent = GetAccessibleParentWindow();
        aTopLeft = aRect.TopLeft();
        aTopLeft += GetWindowExtentsRelative( pParent ).TopLeft();
        aRect = Rectangle( aTopLeft, aRect.GetSize() );
    }

    return aRect;
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumn )
{
    OSL_ENSURE( m_pAccessible, "Invalid call: Accessible is null" );
    return new ::svt::AccessibleBrowseBoxTableCell( m_pAccessible->getAccessibleChild( 0 ), *this, NULL, _nRow, _nColumn );
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleRowHeader( sal_Int32 _nRow )
{
    Reference< XAccessible > xHeader;
    return xHeader;
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleColumnHeader( sal_uInt16 _nColumn )
{
    // first call? -> initial list
    if ( m_aAccessibleChildren.empty() )
        m_aAccessibleChildren.assign( GetColumnCount(), Reference< XAccessible >() );

    // get header
    Reference< XAccessible > xChild = m_aAccessibleChildren[ _nColumn ];
    // already exists?
    if ( !xChild.is() )
    {
        // no -> create new header cell
        ::svt::AccessibleBrowseBoxHeaderCell* pNew = new ::svt::AccessibleBrowseBoxHeaderCell(
            _nColumn, m_pAccessible->getHeaderBar( ::svt::BBTYPE_COLUMNHEADERBAR ),
            *this, NULL, ::svt::BBTYPE_COLUMNHEADERCELL );

        // insert into list
        xChild = pNew;
        m_aAccessibleChildren[ _nColumn ] = xChild;
    }

    return xChild;
}
// -----------------------------------------------------------------------
sal_Int32 SvHeaderTabListBox::GetAccessibleControlCount() const
{
    return -1;
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleControl( sal_Int32 _nIndex )
{
    Reference< XAccessible > xControl;
    return xControl;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint )
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::ConvertPointToCellAddress( sal_Int32& _rnRow, sal_uInt16& _rnColPos, const Point& _rPoint )
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::ConvertPointToRowHeader( sal_Int32& _rnRow, const Point& _rPoint )
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::ConvertPointToColumnHeader( sal_uInt16& _rnColPos, const Point& _rPoint )
{
    return sal_False;
}
// -----------------------------------------------------------------------
::rtl::OUString SvHeaderTabListBox::GetAccessibleName( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    ::rtl::OUString aRetText;
    switch( _eType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
            aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HeaderTabListBox" ) );
            break;

        case ::svt::BBTYPE_TABLE:
            aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HeaderTabListBoxTable" ) );
            break;

        case ::svt::BBTYPE_COLUMNHEADERBAR:
            aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ColumnHeaderBar of HeaderTabListBox" ) );
            break;

        case ::svt::BBTYPE_TABLECELL:
        {
            sal_uInt16 nColumnCount = GetColumnCount();
            sal_Int32 nRow = _nPos / nColumnCount;
            sal_uInt16 nColumn  = _nPos % nColumnCount;
            aRetText = GetEntryText( nRow, nColumn );
            break;
        }
        case ::svt::BBTYPE_COLUMNHEADERCELL:
        {
            aRetText = m_pHeaderBar->GetItemText( m_pHeaderBar->GetItemId( (USHORT)_nPos ) );
            break;
        }

        case ::svt::BBTYPE_ROWHEADERBAR:
        case ::svt::BBTYPE_ROWHEADERCELL:
            aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "error" ) );
            break;

        default:
            OSL_ENSURE(0,"BrowseBox::GetAccessibleName: invalid enum!");
    }
    return aRetText;
}
// -----------------------------------------------------------------------
::rtl::OUString SvHeaderTabListBox::GetAccessibleDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    ::rtl::OUString aRetText;

    switch( _eType )
    {
        case ::svt::BBTYPE_TABLECELL:
        {
            static const String sVar1( RTL_CONSTASCII_USTRINGPARAM( "%1" ) );
            static const String sVar2( RTL_CONSTASCII_USTRINGPARAM( "%2" ) );

            sal_uInt16 nColumnCount = GetColumnCount();
            sal_Int32 nRow = _nPos / nColumnCount;
            sal_uInt16 nColumn  = _nPos % nColumnCount;

            String aText( SvtResId( STR_SVT_ACC_DESC_TABLISTBOX ) );
            aText.SearchAndReplace( sVar1, String::CreateFromInt32( nRow ) );
            aText.SearchAndReplace( sVar2, m_pHeaderBar->GetItemText( m_pHeaderBar->GetItemId( nColumn ) ) );
            aRetText = aText;
            break;
        }
    }

    return aRetText;
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& _rStateSet, ::svt::AccessibleBrowseBoxObjType _eType ) const
{
    switch( _eType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
        case ::svt::BBTYPE_TABLE:
        {
            _rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            if ( HasFocus() )
                _rStateSet.AddState( AccessibleStateType::FOCUSED );
            if ( IsActive() )
                _rStateSet.AddState( AccessibleStateType::ACTIVE );
            if ( IsEnabled() )
                _rStateSet.AddState( AccessibleStateType::ENABLED );
            if ( IsReallyVisible() )
                _rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( _eType == ::svt::BBTYPE_TABLE )
            {
                _rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
                _rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
            }
            break;
        }

        case ::svt::BBTYPE_COLUMNHEADERBAR:
        {
            sal_Int32 nCurRow = GetCurrRow();
            sal_uInt16 nCurColumn = GetCurrColumn();
            if ( IsCellVisible( nCurRow, nCurColumn ) )
                _rStateSet.AddState( AccessibleStateType::VISIBLE );
            _rStateSet.AddState( AccessibleStateType::TRANSIENT );
            break;
        }

        case ::svt::BBTYPE_ROWHEADERCELL:
        case ::svt::BBTYPE_COLUMNHEADERCELL:
        {
            _rStateSet.AddState( AccessibleStateType::VISIBLE );
            _rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            _rStateSet.AddState( AccessibleStateType::TRANSIENT );
            break;
        }
    }
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumn ) const
{
    _rStateSet.AddState( AccessibleStateType::SELECTABLE );
    _rStateSet.AddState( AccessibleStateType::TRANSIENT );

    if ( IsCellVisible( _nRow, _nColumn ) )
        _rStateSet.AddState( AccessibleStateType::VISIBLE );

    if ( IsSelected( GetEntry( _nRow ) ) )
    {
        _rStateSet.AddState( AccessibleStateType::ACTIVE );
        _rStateSet.AddState( AccessibleStateType::SELECTED );
    }
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::GrabTableFocus()
{
    GrabFocus();
}
// -----------------------------------------------------------------------
BOOL SvHeaderTabListBox::GetGlyphBoundRects( const Point& rOrigin, const String& rStr, int nIndex, int nLen, int nBase, MetricVector& rVector )
{
    return Control::GetGlyphBoundRects( rOrigin, rStr, nIndex, nLen, nBase, rVector );
}
// -----------------------------------------------------------------------
Rectangle SvHeaderTabListBox::GetWindowExtentsRelative( Window *pRelativeWindow )
{
    return Control::GetWindowExtentsRelative( pRelativeWindow );
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::GrabFocus()
{
    Control::GrabFocus();
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::GetAccessible( BOOL bCreate )
{
    return Control::GetAccessible( bCreate );
}
// -----------------------------------------------------------------------
Window* SvHeaderTabListBox::GetAccessibleParentWindow() const
{
    return Control::GetAccessibleParentWindow();
}
// -----------------------------------------------------------------------
Window* SvHeaderTabListBox::GetWindow() const
{
    return const_cast< SvHeaderTabListBox* >( this );
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::CreateAccessible()
{
    Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvHeaderTabListBox::::CreateAccessible - accessible parent not found" );

    Reference< XAccessible > xAccessible = m_pAccessible;
    if( pParent && !m_pAccessible )
    {
        Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            m_pAccessible = new ::svt::AccessibleTabListBox( xAccParent, *this );
            xAccessible = m_pAccessible;
        }
    }
    return xAccessible;
}
// -----------------------------------------------------------------------------
Rectangle SvHeaderTabListBox::GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnId,sal_Int32 _nIndex)
{
    Rectangle aRect;
    return aRect;
}
// -----------------------------------------------------------------------------
sal_Int32 SvHeaderTabListBox::GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnId,const Point& _rPoint)
{
    String sText = GetCellText(_nRow,_nColumnId);
    MetricVector aRects;
    if ( GetGlyphBoundRects(Point(0,0),sText,0,STRING_LEN,0,aRects) )
    {
        for (MetricVector::iterator aIter = aRects.begin(); aIter != aRects.end(); ++aIter)
        {
            if( aIter->IsInside(_rPoint) )
                return aIter - aRects.begin();
        }
    }

    return -1;
}
// -----------------------------------------------------------------------------


