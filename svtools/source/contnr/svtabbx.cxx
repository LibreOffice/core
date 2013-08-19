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

#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtools.hrc>
#include <svtools/treelistentry.hxx>
#include <vcl/builder.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include "svtaccessiblefactory.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

#define MYTABMASK \
    ( SV_LBOXTAB_ADJUST_RIGHT | SV_LBOXTAB_ADJUST_LEFT | SV_LBOXTAB_ADJUST_CENTER | SV_LBOXTAB_ADJUST_NUMERIC )

// SvTreeListBox callback

void SvTabListBox::SetTabs()
{
    SvTreeListBox::SetTabs();
    if( nTabCount )
    {
        DBG_ASSERT(pTabList,"TabList ?");

        // The tree listbox has now inserted its tabs into the list. Now we
        // fluff up the list with additional tabs and adjust the rightmost tab
        // of the tree listbox.

        // Picking the rightmost tab.
        // HACK for the explorer! If ViewParent != 0, the first tab of the tree
        // listbox is calculated by the tre listbox itself! This behavior is
        // necessary for ButtonsOnRoot, as the explorer does not know in this
        // case, which additional offset it need to add to the tabs in this mode
        // -- the tree listbox knows that, though!
        /*
        if( !pViewParent )
        {
        SvLBoxTab* pFirstTab = (SvLBoxTab*)aTabs.GetObject( aTabs.Count()-1 );
        pFirstTab->SetPos( pTabList[0].GetPos() );
        pFirstTab->nFlags &= ~MYTABMASK;
        pFirstTab->nFlags |= pTabList[0].nFlags;
        }
        */

        // append all other tabs to the list
        for( sal_uInt16 nCurTab = 1; nCurTab < nTabCount; nCurTab++ )
        {
            SvLBoxTab* pTab = pTabList+nCurTab;
            AddTab( pTab->GetPos(), pTab->nFlags );
        }
    }
}

void SvTabListBox::InitEntry(SvTreeListEntry* pEntry, const OUString& rStr,
    const Image& rColl, const Image& rExp, SvLBoxButtonKind eButtonKind)
{
    SvTreeListBox::InitEntry(pEntry, rStr, rColl, rExp, eButtonKind);

    sal_Int32 nIndex = 0;
    // TODO: verify if nTabCount is always >0 here!
    const sal_uInt16 nCount = nTabCount - 1;
    for( sal_uInt16 nToken = 0; nToken < nCount; nToken++ )
    {
        const OUString aToken = GetToken(aCurEntry, nIndex);
        SvLBoxString* pStr = new SvLBoxString( pEntry, 0, aToken );
        pEntry->AddItem( pStr );
    }
}


SvTabListBox::SvTabListBox( Window* pParent, WinBits nBits )
    : SvTreeListBox( pParent, nBits )
{
    pTabList = 0;
    nTabCount = 0;
    pViewParent = 0;
    SetHighlightRange();    // select full width
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvTabListBox(Window *pParent,
    VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new SvTabListBox(pParent, nWinStyle);
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
    // delete array
    delete [] pTabList;
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

    delete [] pTabList;
    sal_uInt16 nCount = (sal_uInt16)(*pTabs);
    pTabList = new SvLBoxTab[ nCount ];
    nTabCount = nCount;

    MapMode aMMSource( eMapUnit );
    MapMode aMMDest( MAP_PIXEL );

    pTabs++;
    for( sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++, pTabs++ )
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

void SvTabListBox::SetTab( sal_uInt16 nTab,long nValue,MapUnit eMapUnit )
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

SvTreeListEntry* SvTabListBox::InsertEntry( const OUString& rText, SvTreeListEntry* pParent,
                                        sal_Bool /*bChildrenOnDemand*/,
                                        sal_uLong nPos, void* pUserData,
                                        SvLBoxButtonKind )
{
    return InsertEntryToColumn( rText, pParent, nPos, 0xffff, pUserData );
}

SvTreeListEntry* SvTabListBox::InsertEntry( const OUString& rText,
                                        const Image& rExpandedEntryBmp,
                                        const Image& rCollapsedEntryBmp,
                                        SvTreeListEntry* pParent,
                                        sal_Bool /*bChildrenOnDemand*/,
                                        sal_uLong nPos, void* pUserData,
                                        SvLBoxButtonKind )
{
    return InsertEntryToColumn( rText, rExpandedEntryBmp, rCollapsedEntryBmp,
                                pParent, nPos, 0xffff, pUserData );
}

SvTreeListEntry* SvTabListBox::InsertEntryToColumn(const OUString& rStr,SvTreeListEntry* pParent,sal_uLong nPos,sal_uInt16 nCol,
    void* pUser )
{
    OUString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += "\t";
            nCol--;
        }
    }
    aStr += rStr;
    OUString aFirstStr( aStr );
    sal_Int32 nEnd = aFirstStr.indexOf( '\t' );
    if( nEnd != -1 )
    {
        aFirstStr = aFirstStr.copy(0, nEnd);
        aCurEntry = aStr.copy(++nEnd);
    }
    else
        aCurEntry = OUString();
    return SvTreeListBox::InsertEntry( aFirstStr, pParent, sal_False, nPos, pUser );
}

SvTreeListEntry* SvTabListBox::InsertEntryToColumn( const OUString& rStr,
    const Image& rExpandedEntryBmp, const Image& rCollapsedEntryBmp,
    SvTreeListEntry* pParent,sal_uLong nPos,sal_uInt16 nCol, void* pUser )
{
    OUString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += "\t";
            nCol--;
        }
    }
    aStr += rStr;
    OUString aFirstStr( aStr );
    sal_Int32 nEnd = aFirstStr.indexOf('\t');
    if (nEnd != -1)
    {
        aFirstStr = aFirstStr.copy(0, nEnd);
        aCurEntry = aStr.copy(++nEnd);
    }
    else
        aCurEntry = OUString();

    return SvTreeListBox::InsertEntry(
        aFirstStr,
        rExpandedEntryBmp, rCollapsedEntryBmp,
        pParent, sal_False, nPos, pUser );
}

SvTreeListEntry* SvTabListBox::InsertEntryToColumn( const OUString& rStr, sal_uLong nPos,
    sal_uInt16 nCol, void* pUser )
{
    return InsertEntryToColumn( rStr,0,nPos, nCol, pUser );
}

OUString SvTabListBox::GetEntryText( SvTreeListEntry* pEntry ) const
{
    return GetEntryText( pEntry, 0xffff );
}

OUString SvTabListBox::GetEntryText( SvTreeListEntry* pEntry, sal_uInt16 nCol ) const
{
    DBG_ASSERT(pEntry,"GetEntryText:Invalid Entry");
    OUString aResult;
    if( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCur = 0;
        while( nCur < nCount )
        {
            const SvLBoxItem* pStr = pEntry->GetItem( nCur );
            if (pStr->GetType() == SV_ITEM_ID_LBOXSTRING)
            {
                if( nCol == 0xffff )
                {
                    if (!aResult.isEmpty())
                        aResult += "\t";
                    aResult += static_cast<const SvLBoxString*>(pStr)->GetText();
                }
                else
                {
                    if( nCol == 0 )
                        return static_cast<const SvLBoxString*>(pStr)->GetText();
                    nCol--;
                }
            }
            nCur++;
        }
    }
    return aResult;
}

OUString SvTabListBox::GetEntryText( sal_uLong nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos( nPos );
    return GetEntryText( pEntry, nCol );
}

void SvTabListBox::SetEntryText(const OUString& rStr, sal_uLong nPos, sal_uInt16 nCol)
{
    SvTreeListEntry* pEntry = SvTreeListBox::GetEntry( nPos );
    SetEntryText( rStr, pEntry, nCol );
}

void SvTabListBox::SetEntryText(const OUString& rStr, SvTreeListEntry* pEntry, sal_uInt16 nCol)
{
    DBG_ASSERT(pEntry,"SetEntryText:Invalid Entry");
    if( !pEntry )
        return;

    OUString sOldText = GetEntryText(pEntry, nCol);
    if (sOldText == rStr)
        return;

    sal_Int32 nIndex = 0;
    const sal_uInt16 nTextColumn = nCol;
    const sal_uInt16 nCount = pEntry->ItemCount();
    for (sal_uInt16 nCur = 0; nCur < nCount; ++nCur)
    {
        SvLBoxItem* pStr = pEntry->GetItem( nCur );
        if (pStr && pStr->GetType() == SV_ITEM_ID_LBOXSTRING)
        {
            if (!nCol || nCol==0xFFFF)
            {
                const OUString aTemp(GetToken(rStr, nIndex));
                ((SvLBoxString*)pStr)->SetText( aTemp );
                if (!nCol && nIndex<0)
                    break;
            }
            else
            {
                --nCol;
            }
        }
    }
    GetModel()->InvalidateEntry( pEntry );

    TabListBoxEventData* pData = new TabListBoxEventData( pEntry, nTextColumn, sOldText );
    ImplCallEventListeners( VCLEVENT_TABLECELL_NAMECHANGED, pData );
    delete pData;
}

OUString SvTabListBox::GetCellText( sal_uLong nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos( nPos );
    DBG_ASSERT( pEntry, "SvTabListBox::GetCellText(): Invalid Entry" );
    OUString aResult;
    if (pEntry && pEntry->ItemCount() > static_cast<size_t>(nCol+1))
    {
        const SvLBoxItem* pStr = pEntry->GetItem( nCol + 1 );
        if (pStr && pStr->GetType() == SV_ITEM_ID_LBOXSTRING)
            aResult = static_cast<const SvLBoxString*>(pStr)->GetText();
    }
    return aResult;
}

sal_uLong SvTabListBox::GetEntryPos( const OUString& rStr, sal_uInt16 nCol )
{
    sal_uLong nPos = 0;
    SvTreeListEntry* pEntry = First();
    while( pEntry )
    {
        OUString aStr( GetEntryText( pEntry, nCol ));
        if( aStr == rStr )
            return nPos;
        pEntry = Next( pEntry );
        nPos++;
    }
    return 0xffffffff;
}

sal_uLong SvTabListBox::GetEntryPos( const SvTreeListEntry* pEntry ) const
{
    sal_uLong nPos = 0;
    SvTreeListEntry* pTmpEntry = First();
    while( pTmpEntry )
    {
        if ( pTmpEntry == pEntry )
            return nPos;
        pTmpEntry = Next( pTmpEntry );
        ++nPos;
    }
    return 0xffffffff;
}

void SvTabListBox::Resize()
{
    SvTreeListBox::Resize();
}

// static
OUString SvTabListBox::GetToken( const OUString &sStr, sal_Int32& nIndex )
{
    return sStr.getToken(0, '\t', nIndex);
}

OUString SvTabListBox::GetTabEntryText( sal_uLong nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = SvTreeListBox::GetEntry( nPos );
    DBG_ASSERT( pEntry, "GetTabEntryText(): Invalid entry " );
    OUString aResult;
    if ( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCur = ( 0 == nCol && IsCellFocusEnabled() ) ? GetCurrentTabPos() : 0;
        while( nCur < nCount )
        {
            const SvLBoxItem* pStr = pEntry->GetItem( nCur );
            if (pStr->GetType() == SV_ITEM_ID_LBOXSTRING)
            {
                if ( nCol == 0xffff )
                {
                    if (!aResult.isEmpty())
                        aResult += "\t";
                    aResult += static_cast<const SvLBoxString*>(pStr)->GetText();
                }
                else
                {
                    if ( nCol == 0 )
                    {
                        String sRet = static_cast<const SvLBoxString*>(pStr)->GetText();
                        if ( sRet.Len() == 0 )
                            sRet = SVT_RESSTR( STR_SVT_ACC_EMPTY_FIELD );
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

SvTreeListEntry* SvTabListBox::GetEntryOnPos( sal_uLong _nEntryPos ) const
{
    SvTreeListEntry* pEntry = NULL;
    sal_uLong i, nPos = 0, nCount = GetLevelChildCount( NULL );
    for ( i = 0; i < nCount; ++i )
    {
        SvTreeListEntry* pParent = GetEntry(i);
        if ( nPos == _nEntryPos )
        {
            pEntry = pParent;
            break;
        }
        else
        {
            nPos++;
            pEntry = GetChildOnPos( pParent, _nEntryPos, nPos );
            if ( pEntry )
                break;
        }
    }

    return pEntry;
}

SvTreeListEntry* SvTabListBox::GetChildOnPos( SvTreeListEntry* _pParent, sal_uLong _nEntryPos, sal_uLong& _rPos ) const
{
    sal_uLong i, nCount = GetLevelChildCount( _pParent );
    for ( i = 0; i < nCount; ++i )
    {
        SvTreeListEntry* pParent = GetEntry( _pParent, i );
        if ( _rPos == _nEntryPos )
            return pParent;
        else
        {
            _rPos++;
            SvTreeListEntry* pEntry = GetChildOnPos( pParent, _nEntryPos, _rPos );
            if ( pEntry )
                return pEntry;
        }
    }

    return NULL;
}

void SvTabListBox::SetTabJustify( sal_uInt16 nTab, SvTabJustify eJustify)
{
    if( nTab >= nTabCount )
        return;
    SvLBoxTab* pTab = &(pTabList[ nTab ]);
    sal_uInt16 nFlags = pTab->nFlags;
    nFlags &= (~MYTABMASK);
    nFlags |= (sal_uInt16)eJustify;
    pTab->nFlags = nFlags;
    SvTreeListBox::nTreeFlags |= TREEFLAG_RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

long SvTabListBox::GetLogicTab( sal_uInt16 nTab )
{
    if( SvTreeListBox::nTreeFlags & TREEFLAG_RECALCTABS )
        ((SvTabListBox*)this)->SetTabs();

    DBG_ASSERT(nTab<nTabCount,"GetTabPos:Invalid Tab");
    return aTabs[ nTab ]->GetPos();
}

// class SvHeaderTabListBoxImpl ------------------------------------------

namespace svt
{
    struct SvHeaderTabListBoxImpl
    {
        HeaderBar*              m_pHeaderBar;
        AccessibleFactoryAccess m_aFactoryAccess;

        SvHeaderTabListBoxImpl() : m_pHeaderBar( NULL ) { }
    };
}

// class SvHeaderTabListBox ----------------------------------------------

SvHeaderTabListBox::SvHeaderTabListBox( Window* pParent, WinBits nWinStyle ) :

    SvTabListBox( pParent, nWinStyle ),

    m_bFirstPaint   ( sal_True ),
    m_pImpl         ( new ::svt::SvHeaderTabListBoxImpl ),
    m_pAccessible   ( NULL )
{
}

// -----------------------------------------------------------------------

SvHeaderTabListBox::SvHeaderTabListBox( Window* pParent, const ResId& rResId ) :

    SvTabListBox( pParent, rResId ),

    m_bFirstPaint   ( sal_True ),
    m_pImpl         ( new ::svt::SvHeaderTabListBoxImpl ),
    m_pAccessible   ( NULL )
{
}

// -----------------------------------------------------------------------

SvHeaderTabListBox::~SvHeaderTabListBox()
{
    delete m_pImpl;
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::Paint( const Rectangle& rRect )
{
    if ( m_bFirstPaint )
    {
        m_bFirstPaint = sal_False;
        RepaintScrollBars();
    }
    SvTabListBox::Paint( rRect );
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::InitHeaderBar( HeaderBar* pHeaderBar )
{
    DBG_ASSERT( !m_pImpl->m_pHeaderBar, "header bar already initialized" );
    DBG_ASSERT( pHeaderBar, "invalid header bar initialization" );
    m_pImpl->m_pHeaderBar = pHeaderBar;
    SetScrolledHdl( LINK( this, SvHeaderTabListBox, ScrollHdl_Impl ) );
    m_pImpl->m_pHeaderBar->SetCreateAccessibleHdl( LINK( this, SvHeaderTabListBox, CreateAccessibleHdl_Impl ) );
}

// -----------------------------------------------------------------------

sal_Bool SvHeaderTabListBox::IsItemChecked( SvTreeListEntry* pEntry, sal_uInt16 nCol ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton* pItem = (SvLBoxButton*)( pEntry->GetItem( nCol + 1 ) );

    if (pItem && pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        sal_uInt16 nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }

    return ( eState == SV_BUTTON_CHECKED );
}

// -----------------------------------------------------------------------

SvTreeListEntry* SvHeaderTabListBox::InsertEntryToColumn(
    const OUString& rStr, sal_uLong nPos, sal_uInt16 nCol, void* pUserData )
{
    SvTreeListEntry* pEntry = SvTabListBox::InsertEntryToColumn( rStr, nPos, nCol, pUserData );
    RecalculateAccessibleChildren();
    return pEntry;
}

// -----------------------------------------------------------------------

SvTreeListEntry* SvHeaderTabListBox::InsertEntryToColumn(
    const OUString& rStr, SvTreeListEntry* pParent, sal_uLong nPos, sal_uInt16 nCol, void* pUserData )
{
    SvTreeListEntry* pEntry = SvTabListBox::InsertEntryToColumn( rStr, pParent, nPos, nCol, pUserData );
    RecalculateAccessibleChildren();
    return pEntry;
}

// -----------------------------------------------------------------------

SvTreeListEntry* SvHeaderTabListBox::InsertEntryToColumn(
    const OUString& rStr, const Image& rExpandedEntryBmp, const Image& rCollapsedEntryBmp,
    SvTreeListEntry* pParent, sal_uLong nPos, sal_uInt16 nCol, void* pUserData )
{
    SvTreeListEntry* pEntry = SvTabListBox::InsertEntryToColumn(
        rStr, rExpandedEntryBmp, rCollapsedEntryBmp, pParent, nPos, nCol, pUserData );
    RecalculateAccessibleChildren();
    return pEntry;
}

// -----------------------------------------------------------------------

sal_uLong SvHeaderTabListBox::Insert(
    SvTreeListEntry* pEnt, SvTreeListEntry* pPar, sal_uLong nPos )
{
    sal_uLong n = SvTabListBox::Insert( pEnt, pPar, nPos );
    RecalculateAccessibleChildren();
    return n;
}

// -----------------------------------------------------------------------

sal_uLong SvHeaderTabListBox::Insert( SvTreeListEntry* pEntry, sal_uLong nRootPos )
{
    sal_uLong nPos = SvTabListBox::Insert( pEntry, nRootPos );
    RecalculateAccessibleChildren();
    return nPos;
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::RemoveEntry( SvTreeListEntry* _pEntry )
{
    GetModel()->Remove( _pEntry );
    m_aAccessibleChildren.clear();
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::Clear()
{
    SvTabListBox::Clear();
    m_aAccessibleChildren.clear();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvHeaderTabListBox, ScrollHdl_Impl)
{
    m_pImpl->m_pHeaderBar->SetOffset( -GetXOffset() );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvHeaderTabListBox, CreateAccessibleHdl_Impl)
{
    Window* pParent = m_pImpl->m_pHeaderBar->GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvHeaderTabListBox..CreateAccessibleHdl_Impl - accessible parent not found" );
    if ( pParent )
    {
        ::com::sun::star::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            Reference< XAccessible > xAccessible = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxHeaderBar(
                xAccParent, *this, ::svt::BBTYPE_COLUMNHEADERBAR );
            m_pImpl->m_pHeaderBar->SetAccessible( xAccessible );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

void SvHeaderTabListBox::RecalculateAccessibleChildren()
{
    if ( !m_aAccessibleChildren.empty() )
    {
        sal_uInt32 nCount = ( GetRowCount() + 1 ) * GetColumnCount();
        if ( m_aAccessibleChildren.size() < nCount )
            m_aAccessibleChildren.resize( nCount );
        else
        {
            DBG_ASSERT( m_aAccessibleChildren.size() == nCount, "wrong children count" );
        }
    }
}

// -----------------------------------------------------------------------

sal_Bool SvHeaderTabListBox::IsCellCheckBox( long _nRow, sal_uInt16 _nColumn, TriState& _rState )
{
    bool bRet = false;
    SvTreeListEntry* pEntry = GetEntry( _nRow );
    if ( pEntry )
    {
        sal_uInt16 nItemCount = pEntry->ItemCount();
        if ( nItemCount > ( _nColumn + 1 ) )
        {
            SvLBoxButton* pItem = (SvLBoxButton*)( pEntry->GetItem( _nColumn + 1 ) );
            if (pItem && pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
            {
                bRet = true;
                _rState = ( ( pItem->GetButtonFlags() & SV_ITEMSTATE_UNCHECKED ) == 0 )
                            ? STATE_CHECK : STATE_NOCHECK;
            }
        }
        else
        {
            SAL_WARN( "svtools.contnr", "SvHeaderTabListBox::IsCellCheckBox(): column out of range" );
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------
long SvHeaderTabListBox::GetRowCount() const
{
    return GetEntryCount();
}
// -----------------------------------------------------------------------
sal_uInt16 SvHeaderTabListBox::GetColumnCount() const
{
    return m_pImpl->m_pHeaderBar->GetItemCount();
}
// -----------------------------------------------------------------------
sal_Int32 SvHeaderTabListBox::GetCurrRow() const
{
    sal_Int32 nRet = -1;
    SvTreeListEntry* pEntry = GetCurEntry();
    if ( pEntry )
    {
        sal_uLong nCount = GetEntryCount();
        for ( sal_uLong i = 0; i < nCount; ++i )
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
OUString SvHeaderTabListBox::GetRowDescription( sal_Int32 _nRow ) const
{
    return OUString( GetEntryText( _nRow ) );
}
// -----------------------------------------------------------------------
OUString SvHeaderTabListBox::GetColumnDescription( sal_uInt16 _nColumn ) const
{
    return OUString( m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( _nColumn ) ) );
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
    sal_Bool bRet = ( IsCellFocusEnabled() == sal_True );
    if ( bRet )
    {
        // first set cursor to _nRow
        SetCursor( GetEntry( _nRow ), sal_True );
        // then set the focus into _nColumn
        bRet = ( SetCurrentTabPos( _nColumn ) == true );
    }
    return bRet;
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::SetNoSelection()
{
    SvTreeListBox::SelectAll(false);
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::SelectAll()
{
    SvTreeListBox::SelectAll(true);
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::SelectAll( sal_Bool bSelect, sal_Bool bPaint )
{
    // overwritten just to disambiguate the SelectAll() from the base' class SelectAll( BOOl, sal_Bool )
    SvTabListBox::SelectAll( bSelect, bPaint );
}

// -----------------------------------------------------------------------
void SvHeaderTabListBox::SelectRow( long _nRow, sal_Bool _bSelect, sal_Bool )
{
    Select( GetEntry( _nRow ), _bSelect );
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::SelectColumn( sal_uInt16, sal_Bool )
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
bool SvHeaderTabListBox::IsRowSelected( long _nRow ) const
{
    SvTreeListEntry* pEntry = GetEntry( _nRow );
    return ( pEntry && IsSelected( pEntry ) );
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::IsColumnSelected( long ) const
{
    return sal_False;
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::GetAllSelectedRows( ::com::sun::star::uno::Sequence< sal_Int32 >& ) const
{
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::GetAllSelectedColumns( ::com::sun::star::uno::Sequence< sal_Int32 >& ) const
{
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::IsCellVisible( sal_Int32, sal_uInt16 ) const
{
    return sal_True;
}
// -----------------------------------------------------------------------
OUString SvHeaderTabListBox::GetAccessibleCellText( long _nRow, sal_uInt16 _nColumnPos ) const
{
    return GetTabEntryText(_nRow, _nColumnPos);
}
// -----------------------------------------------------------------------
Rectangle SvHeaderTabListBox::calcHeaderRect( sal_Bool _bIsColumnBar, sal_Bool _bOnScreen )
{
    Rectangle aRect;
    if ( _bIsColumnBar )
    {
        Window* pParent = NULL;
        if ( !_bOnScreen )
            pParent = m_pImpl->m_pHeaderBar->GetAccessibleParentWindow();

        aRect = m_pImpl->m_pHeaderBar->GetWindowExtentsRelative( pParent );
    }
    return aRect;
}
// -----------------------------------------------------------------------
Rectangle SvHeaderTabListBox::calcTableRect( sal_Bool _bOnScreen )
{
    Window* pParent = NULL;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    Rectangle aRect( GetWindowExtentsRelative( pParent ) );
    return aRect;
}
// -----------------------------------------------------------------------
Rectangle SvHeaderTabListBox::GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumn, sal_Bool _bIsHeader, sal_Bool _bOnScreen )
{
    DBG_ASSERT( !_bIsHeader || 0 == _nRow, "invalid parameters" );
    Rectangle aRect;
    SvTreeListEntry* pEntry = GetEntry( _nRow );
    if ( pEntry )
    {
        aRect = _bIsHeader ? calcHeaderRect( sal_True, sal_False ) : GetBoundingRect( pEntry );
        Point aTopLeft = aRect.TopLeft();
        DBG_ASSERT( m_pImpl->m_pHeaderBar->GetItemCount() > _nColumn, "invalid column" );
        Rectangle aItemRect = m_pImpl->m_pHeaderBar->GetItemRect( m_pImpl->m_pHeaderBar->GetItemId( _nColumn ) );
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
Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    OSL_ENSURE( m_pAccessible, "Invalid call: Accessible is null" );

    Reference< XAccessible > xChild;
    sal_Int32 nIndex = -1;

    if ( !AreChildrenTransient() )
    {
        const sal_uInt16 nColumnCount = GetColumnCount();

        // first call? -> initial list
        if ( m_aAccessibleChildren.empty() )
        {
            sal_Int32 nCount = ( GetRowCount() + 1 ) * nColumnCount;
            m_aAccessibleChildren.assign( nCount, Reference< XAccessible >() );
        }

        nIndex = ( _nRow * nColumnCount ) + _nColumnPos + nColumnCount;
        xChild = m_aAccessibleChildren[ nIndex ];
    }

    if ( !xChild.is() )
    {
        TriState eState = STATE_DONTKNOW;
        sal_Bool bIsCheckBox = IsCellCheckBox( _nRow, _nColumnPos, eState );
        if ( bIsCheckBox )
            xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleCheckBoxCell(
                m_pAccessible->getAccessibleChild( 0 ), *this, NULL, _nRow, _nColumnPos, eState, sal_False );
        else
            xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxTableCell(
                m_pAccessible->getAccessibleChild( 0 ), *this, NULL, _nRow, _nColumnPos, OFFSET_NONE );

        // insert into list
        if ( !AreChildrenTransient() )
            m_aAccessibleChildren[ nIndex ] = xChild;
    }

    return xChild;
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleRowHeader( sal_Int32 )
{
    Reference< XAccessible > xHeader;
    return xHeader;
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleColumnHeader( sal_uInt16 _nColumn )
{
    // first call? -> initial list
    if ( m_aAccessibleChildren.empty() )
    {
        const sal_uInt16 nColumnCount = GetColumnCount();
        sal_Int32 nCount = AreChildrenTransient() ?
                nColumnCount : ( GetRowCount() + 1 ) * nColumnCount;
        m_aAccessibleChildren.assign( nCount, Reference< XAccessible >() );
    }

    // get header
    Reference< XAccessible > xChild = m_aAccessibleChildren[ _nColumn ];
    // already exists?
    if ( !xChild.is() && m_pAccessible )
    {
        // no -> create new header cell
        xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxHeaderCell(
            _nColumn, m_pAccessible->getHeaderBar( ::svt::BBTYPE_COLUMNHEADERBAR ),
            *this, NULL, ::svt::BBTYPE_COLUMNHEADERCELL
        );

        // insert into list
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
Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleControl( sal_Int32 )
{
    Reference< XAccessible > xControl;
    return xControl;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::ConvertPointToControlIndex( sal_Int32&, const Point& )
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::ConvertPointToCellAddress( sal_Int32&, sal_uInt16&, const Point& )
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::ConvertPointToRowHeader( sal_Int32&, const Point& )
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvHeaderTabListBox::ConvertPointToColumnHeader( sal_uInt16&, const Point& )
{
    return sal_False;
}
// -----------------------------------------------------------------------
OUString SvHeaderTabListBox::GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    OUString aRetText;
    switch( _eType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
        case ::svt::BBTYPE_TABLE:
        case ::svt::BBTYPE_COLUMNHEADERBAR:
            // should be empty now (see #i63983)
            aRetText = OUString();
            break;

        case ::svt::BBTYPE_TABLECELL:
        {
            // here we need a valid pos, we can not handle -1
            if ( _nPos >= 0 )
            {
                sal_uInt16 nColumnCount = GetColumnCount();
                if (nColumnCount > 0)
                {
                    sal_Int32 nRow = _nPos / nColumnCount;
                    sal_uInt16 nColumn  = static_cast< sal_uInt16 >( _nPos % nColumnCount );
                    aRetText = GetCellText( nRow, nColumn );
                }
            }
            break;
        }
        case ::svt::BBTYPE_CHECKBOXCELL:
        {
            break; // checkbox cells have no name
        }
        case ::svt::BBTYPE_COLUMNHEADERCELL:
        {
            aRetText = m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( (sal_uInt16)_nPos ) );
            break;
        }

        case ::svt::BBTYPE_ROWHEADERBAR:
        case ::svt::BBTYPE_ROWHEADERCELL:
            aRetText = OUString( "error" );
            break;

        default:
            OSL_FAIL("BrowseBox::GetAccessibleName: invalid enum!");
    }
    return aRetText;
}
// -----------------------------------------------------------------------
OUString SvHeaderTabListBox::GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    OUString aRetText;

    if( _eType == ::svt::BBTYPE_TABLECELL && _nPos != -1 )
    {
        const String sVar1( RTL_CONSTASCII_USTRINGPARAM( "%1" ) );
        const String sVar2( RTL_CONSTASCII_USTRINGPARAM( "%2" ) );

        sal_uInt16 nColumnCount = GetColumnCount();
        if (nColumnCount > 0)
        {
            sal_Int32 nRow = _nPos / nColumnCount;
            sal_uInt16 nColumn  = static_cast< sal_uInt16 >( _nPos % nColumnCount );

            String aText( SVT_RESSTR(STR_SVT_ACC_DESC_TABLISTBOX) );
            aText.SearchAndReplace( sVar1, OUString::number( nRow ) );
            String sColHeader = m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( nColumn ) );
            if ( sColHeader.Len() == 0 )
                sColHeader = OUString::number( nColumn );
            aText.SearchAndReplace( sVar2, sColHeader );
            aRetText = aText;
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
            {
                _rStateSet.AddState( AccessibleStateType::ENABLED );
                _rStateSet.AddState( AccessibleStateType::SENSITIVE );
            }
            if ( IsReallyVisible() )
                _rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( _eType == ::svt::BBTYPE_TABLE )
            {

                if ( AreChildrenTransient() )
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
        default:
            break;
    }
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumn ) const
{
    _rStateSet.AddState( AccessibleStateType::SELECTABLE );
    if ( AreChildrenTransient() )
        _rStateSet.AddState( AccessibleStateType::TRANSIENT );

    if ( IsCellVisible( _nRow, _nColumn ) )
    {
        _rStateSet.AddState( AccessibleStateType::VISIBLE );
        _rStateSet.AddState( AccessibleStateType::ENABLED );
    }

    if ( IsRowSelected( _nRow ) )
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
sal_Bool SvHeaderTabListBox::GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, int nBase, MetricVector& rVector )
{
    return Control::GetGlyphBoundRects( rOrigin, rStr, nIndex, nLen, nBase, rVector );
}
// -----------------------------------------------------------------------
Rectangle SvHeaderTabListBox::GetWindowExtentsRelative( Window *pRelativeWindow ) const
{
    return Control::GetWindowExtentsRelative( pRelativeWindow );
}
// -----------------------------------------------------------------------
void SvHeaderTabListBox::GrabFocus()
{
    Control::GrabFocus();
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::GetAccessible( sal_Bool bCreate )
{
    return Control::GetAccessible( bCreate );
}
// -----------------------------------------------------------------------
Window* SvHeaderTabListBox::GetAccessibleParentWindow() const
{
    return Control::GetAccessibleParentWindow();
}
// -----------------------------------------------------------------------
Window* SvHeaderTabListBox::GetWindowInstance()
{
    return this;
}
// -----------------------------------------------------------------------
Reference< XAccessible > SvHeaderTabListBox::CreateAccessible()
{
    Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvHeaderTabListBox::::CreateAccessible - accessible parent not found" );

    Reference< XAccessible > xAccessible;
    if ( m_pAccessible ) xAccessible = m_pAccessible->getMyself();

    if( pParent && !m_pAccessible )
    {
        Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            m_pAccessible = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleTabListBox( xAccParent, *this );
            if ( m_pAccessible )
                xAccessible = m_pAccessible->getMyself();
        }
    }
    return xAccessible;
}
// -----------------------------------------------------------------------------
Rectangle SvHeaderTabListBox::GetFieldCharacterBounds(sal_Int32,sal_Int32,sal_Int32)
{
    Rectangle aRect;
    return aRect;
}
// -----------------------------------------------------------------------------
sal_Int32 SvHeaderTabListBox::GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint)
{
    OUString sText = GetAccessibleCellText( _nRow, static_cast< sal_uInt16 >( _nColumnPos ) );
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
