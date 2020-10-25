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

#include <vcl/svtaccessiblefactory.hxx>
#include <vcl/accessiblefactory.hxx>
#include <vcl/toolkit/svtabbx.hxx>
#include <vcl/headbar.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <strings.hrc>
#include <svdata.hxx>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

constexpr SvLBoxTabFlags MYTABMASK =
    SvLBoxTabFlags::ADJUST_RIGHT | SvLBoxTabFlags::ADJUST_LEFT | SvLBoxTabFlags::ADJUST_CENTER | SvLBoxTabFlags::FORCE;

// SvTreeListBox callback

void SvTabListBox::SetTabs()
{
    SvTreeListBox::SetTabs();
    if( mvTabList.empty() )
        return;

    DBG_ASSERT(!mvTabList.empty(),"TabList ?");

    // The tree listbox has now inserted its tabs into the list. Now we
    // fluff up the list with additional tabs and adjust the rightmost tab
    // of the tree listbox.

    // Picking the rightmost tab.
    // HACK for the explorer! If ViewParent != 0, the first tab of the tree
    // listbox is calculated by the tree listbox itself! This behavior is
    // necessary for ButtonsOnRoot, as the explorer does not know in this
    // case, which additional offset it needs to add to the tabs in this mode
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
    for( sal_uInt16 nCurTab = 1; nCurTab < sal_uInt16(mvTabList.size()); nCurTab++ )
    {
        SvLBoxTab& rTab = mvTabList[nCurTab];
        AddTab( rTab.GetPos(), rTab.nFlags );
    }
}

void SvTabListBox::InitEntry(SvTreeListEntry* pEntry, const OUString& rStr,
    const Image& rColl, const Image& rExp)
{
    SvTreeListBox::InitEntry(pEntry, rStr, rColl, rExp);

    sal_Int32 nIndex = 0;
    // TODO: verify if nTabCount is always >0 here!
    const sal_uInt16 nCount = mvTabList.size() - 1;
    for( sal_uInt16 nToken = 0; nToken < nCount; nToken++ )
    {
        const OUString aToken = GetToken(aCurEntry, nIndex);
        pEntry->AddItem(std::make_unique<SvLBoxString>(aToken));
    }
}

SvTabListBox::SvTabListBox( vcl::Window* pParent, WinBits nBits )
    : SvTreeListBox( pParent, nBits )
{
    SetHighlightRange();    // select full width
}

SvTabListBox::~SvTabListBox()
{
    disposeOnce();
}

void SvTabListBox::dispose()
{
    mvTabList.clear();
    SvTreeListBox::dispose();
}

void SvTabListBox::SetTabs(sal_uInt16 nTabs, tools::Long const pTabPositions[], MapUnit eMapUnit)
{
    mvTabList.resize(nTabs);

    MapMode aMMSource( eMapUnit );
    MapMode aMMDest( MapUnit::MapPixel );

    for( sal_uInt16 nIdx = 0; nIdx < sal_uInt16(mvTabList.size()); nIdx++, pTabPositions++ )
    {
        Size aSize( *pTabPositions, 0 );
        aSize = LogicToLogic( aSize, &aMMSource, &aMMDest );
        tools::Long nNewTab = aSize.Width();
        mvTabList[nIdx].SetPos( nNewTab );
        mvTabList[nIdx].nFlags &= MYTABMASK;
    }
    SvTreeListBox::nTreeFlags |= SvTreeFlags::RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

SvTreeListEntry* SvTabListBox::InsertEntry( const OUString& rText, SvTreeListEntry* pParent,
                                        bool /*bChildrenOnDemand*/,
                                        sal_uLong nPos, void* pUserData )
{
    return InsertEntryToColumn( rText, pParent, nPos, 0xffff, pUserData );
}

SvTreeListEntry* SvTabListBox::InsertEntry( const OUString& rText,
                                        const Image& rExpandedEntryBmp,
                                        const Image& rCollapsedEntryBmp,
                                        SvTreeListEntry* pParent,
                                        bool /*bChildrenOnDemand*/,
                                        sal_uLong nPos, void* pUserData )
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
        aCurEntry.clear();
    return SvTreeListBox::InsertEntry( aFirstStr, pParent, false, nPos, pUser );
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
        aCurEntry.clear();

    return SvTreeListBox::InsertEntry(
        aFirstStr,
        rExpandedEntryBmp, rCollapsedEntryBmp,
        pParent, false, nPos, pUser );
}

OUString SvTabListBox::GetEntryText( SvTreeListEntry* pEntry ) const
{
    return GetEntryText( pEntry, 0xffff );
}

OUString SvTabListBox::GetEntryText( SvTreeListEntry* pEntry, sal_uInt16 nCol )
{
    DBG_ASSERT(pEntry,"GetEntryText:Invalid Entry");
    OUStringBuffer aResult;
    if( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCur = 0;
        while( nCur < nCount )
        {
            const SvLBoxItem& rStr = pEntry->GetItem( nCur );
            if (rStr.GetType() == SvLBoxItemType::String)
            {
                if( nCol == 0xffff )
                {
                    if (!aResult.isEmpty())
                        aResult.append("\t");
                    aResult.append(static_cast<const SvLBoxString&>(rStr).GetText());
                }
                else
                {
                    if( nCol == 0 )
                        return static_cast<const SvLBoxString&>(rStr).GetText();
                    nCol--;
                }
            }
            nCur++;
        }
    }
    return aResult.makeStringAndClear();
}

OUString SvTabListBox::GetEntryText( sal_uLong nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos( nPos );
    return GetEntryText( pEntry, nCol );
}

OUString SvTabListBox::GetCellText( sal_uLong nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos( nPos );
    DBG_ASSERT( pEntry, "SvTabListBox::GetCellText(): Invalid Entry" );
    OUString aResult;
    if (pEntry && pEntry->ItemCount() > o3tl::make_unsigned(nCol+1))
    {
        const SvLBoxItem& rStr = pEntry->GetItem( nCol + 1 );
        if (rStr.GetType() == SvLBoxItemType::String)
            aResult = static_cast<const SvLBoxString&>(rStr).GetText();
    }
    return aResult;
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

// static
OUString SvTabListBox::GetToken( const OUString &sStr, sal_Int32& nIndex )
{
    return sStr.getToken(0, '\t', nIndex);
}

OUString SvTabListBox::GetTabEntryText( sal_uLong nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = SvTreeListBox::GetEntry( nPos );
    DBG_ASSERT( pEntry, "GetTabEntryText(): Invalid entry " );
    OUStringBuffer aResult;
    if ( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCur = 0;
        while( nCur < nCount )
        {
            const SvLBoxItem& rBoxItem = pEntry->GetItem( nCur );
            if (rBoxItem.GetType() == SvLBoxItemType::String)
            {
                if ( nCol == 0xffff )
                {
                    if (!aResult.isEmpty())
                        aResult.append("\t");
                    aResult.append(static_cast<const SvLBoxString&>(rBoxItem).GetText());
                }
                else
                {
                    if ( nCol == 0 )
                    {
                        OUString sRet = static_cast<const SvLBoxString&>(rBoxItem).GetText();
                        if ( sRet.isEmpty() )
                            sRet = VclResId( STR_SVT_ACC_EMPTY_FIELD );
                        return sRet;
                    }
                    --nCol;
                }
            }
            ++nCur;
        }
    }
    return aResult.makeStringAndClear();
}

SvTreeListEntry* SvTabListBox::GetEntryOnPos( sal_uLong _nEntryPos ) const
{
    SvTreeListEntry* pEntry = nullptr;
    sal_uLong i, nPos = 0, nCount = GetLevelChildCount( nullptr );
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

    return nullptr;
}

void SvTabListBox::SetTabJustify( sal_uInt16 nTab, SvTabJustify eJustify)
{
    DBG_ASSERT(nTab<mvTabList.size(),"GetTabPos:Invalid Tab");
    if( nTab >= mvTabList.size() )
        return;
    SvLBoxTab& rTab = mvTabList[ nTab ];
    SvLBoxTabFlags nFlags = rTab.nFlags;
    nFlags &= ~MYTABMASK;
    // see SvLBoxTab::CalcOffset for force, which only matters for centering
    nFlags |= static_cast<SvLBoxTabFlags>(eJustify) | SvLBoxTabFlags::FORCE;
    rTab.nFlags = nFlags;
    SvTreeListBox::nTreeFlags |= SvTreeFlags::RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

void SvTabListBox::SetTabEditable(sal_uInt16 nTab, bool bEditable)
{
    DBG_ASSERT(nTab<mvTabList.size(),"GetTabPos:Invalid Tab");
    if( nTab >= mvTabList.size() )
        return;
    SvLBoxTab& rTab = mvTabList[ nTab ];
    if (bEditable)
        rTab.nFlags |= SvLBoxTabFlags::EDITABLE;
    else
        rTab.nFlags &= ~SvLBoxTabFlags::EDITABLE;
}

tools::Long SvTabListBox::GetLogicTab( sal_uInt16 nTab )
{
    if( SvTreeListBox::nTreeFlags & SvTreeFlags::RECALCTABS )
        SetTabs();

    DBG_ASSERT(nTab<mvTabList.size(),"GetTabPos:Invalid Tab");
    return aTabs[ nTab ]->GetPos();
}

namespace vcl
{
    struct SvHeaderTabListBoxImpl
    {
        VclPtr<HeaderBar>       m_pHeaderBar;
        AccessibleFactoryAccess m_aFactoryAccess;

        SvHeaderTabListBoxImpl() : m_pHeaderBar( nullptr ) { }
    };
}

SvHeaderTabListBox::SvHeaderTabListBox( vcl::Window* pParent, WinBits nWinStyle )
    : SvTabListBox(pParent, nWinStyle)
    , m_bFirstPaint(true)
    , m_pImpl(new ::vcl::SvHeaderTabListBoxImpl)
    , m_pAccessible(nullptr)
{
}

SvHeaderTabListBox::~SvHeaderTabListBox()
{
    disposeOnce();
}

void SvHeaderTabListBox::dispose()
{
    m_pImpl.reset();
    SvTabListBox::dispose();
}

void SvHeaderTabListBox::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect )
{
    if (m_bFirstPaint)
    {
        m_bFirstPaint = false;
    }
    SvTabListBox::Paint(rRenderContext, rRect);
}

void SvHeaderTabListBox::InitHeaderBar( HeaderBar* pHeaderBar )
{
    DBG_ASSERT( !m_pImpl->m_pHeaderBar, "header bar already initialized" );
    DBG_ASSERT( pHeaderBar, "invalid header bar initialization" );
    m_pImpl->m_pHeaderBar = pHeaderBar;
    SetScrolledHdl( LINK( this, SvHeaderTabListBox, ScrollHdl_Impl ) );
    m_pImpl->m_pHeaderBar->SetCreateAccessibleHdl( LINK( this, SvHeaderTabListBox, CreateAccessibleHdl_Impl ) );
}

HeaderBar* SvHeaderTabListBox::GetHeaderBar()
{
    return m_pImpl ? m_pImpl->m_pHeaderBar : nullptr;
}

bool SvHeaderTabListBox::IsItemChecked( SvTreeListEntry* pEntry, sal_uInt16 nCol )
{
    SvButtonState eState = SvButtonState::Unchecked;
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>( pEntry->GetItem( nCol + 1 ) );

    if (rItem.GetType() == SvLBoxItemType::Button)
    {
        SvItemStateFlags nButtonFlags = rItem.GetButtonFlags();
        eState = SvLBoxButtonData::ConvertToButtonState( nButtonFlags );
    }

    return ( eState == SvButtonState::Checked );
}

SvTreeListEntry* SvHeaderTabListBox::InsertEntryToColumn(
    const OUString& rStr, SvTreeListEntry* pParent, sal_uLong nPos, sal_uInt16 nCol, void* pUserData )
{
    SvTreeListEntry* pEntry = SvTabListBox::InsertEntryToColumn( rStr, pParent, nPos, nCol, pUserData );
    RecalculateAccessibleChildren();
    return pEntry;
}

SvTreeListEntry* SvHeaderTabListBox::InsertEntryToColumn(
    const OUString& rStr, const Image& rExpandedEntryBmp, const Image& rCollapsedEntryBmp,
    SvTreeListEntry* pParent, sal_uLong nPos, sal_uInt16 nCol, void* pUserData )
{
    SvTreeListEntry* pEntry = SvTabListBox::InsertEntryToColumn(
        rStr, rExpandedEntryBmp, rCollapsedEntryBmp, pParent, nPos, nCol, pUserData );
    RecalculateAccessibleChildren();
    return pEntry;
}

sal_uLong SvHeaderTabListBox::Insert(
    SvTreeListEntry* pEnt, SvTreeListEntry* pPar, sal_uLong nPos )
{
    sal_uLong n = SvTabListBox::Insert( pEnt, pPar, nPos );
    RecalculateAccessibleChildren();
    return n;
}

sal_uLong SvHeaderTabListBox::Insert( SvTreeListEntry* pEntry, sal_uLong nRootPos )
{
    sal_uLong nPos = SvTabListBox::Insert( pEntry, nRootPos );
    RecalculateAccessibleChildren();
    return nPos;
}

IMPL_LINK_NOARG(SvHeaderTabListBox, ScrollHdl_Impl, SvTreeListBox*, void)
{
    m_pImpl->m_pHeaderBar->SetOffset( -GetXOffset() );
}

IMPL_LINK_NOARG(SvHeaderTabListBox, CreateAccessibleHdl_Impl, HeaderBar*, void)
{
    vcl::Window* pParent = m_pImpl->m_pHeaderBar->GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvHeaderTabListBox..CreateAccessibleHdl_Impl - accessible parent not found" );
    if ( pParent )
    {
        css::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            Reference< XAccessible > xAccessible = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxHeaderBar(
                xAccParent, *this, ::vcl::BBTYPE_COLUMNHEADERBAR );
            m_pImpl->m_pHeaderBar->SetAccessible( xAccessible );
        }
    }
}

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

bool SvHeaderTabListBox::IsCellCheckBox( sal_Int32 _nRow, sal_uInt16 _nColumn, TriState& _rState )
{
    bool bRet = false;
    SvTreeListEntry* pEntry = GetEntry( _nRow );
    if ( pEntry )
    {
        sal_uInt16 nItemCount = pEntry->ItemCount();
        if ( nItemCount > ( _nColumn + 1 ) )
        {
            SvLBoxItem& rItem = pEntry->GetItem( _nColumn + 1 );
            if (rItem.GetType() == SvLBoxItemType::Button)
            {
                bRet = true;
                _rState = ( ( static_cast<SvLBoxButton&>(rItem).GetButtonFlags() & SvItemStateFlags::UNCHECKED ) == SvItemStateFlags::NONE )
                            ? TRISTATE_TRUE : TRISTATE_FALSE;
            }
        }
        else
        {
            SAL_WARN( "svtools.contnr", "SvHeaderTabListBox::IsCellCheckBox(): column out of range" );
        }
    }
    return bRet;
}
sal_Int32 SvHeaderTabListBox::GetRowCount() const
{
    return GetEntryCount();
}

sal_uInt16 SvHeaderTabListBox::GetColumnCount() const
{
    return m_pImpl->m_pHeaderBar->GetItemCount();
}

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

sal_uInt16 SvHeaderTabListBox::GetCurrColumn() const
{
    return 0;
}

OUString SvHeaderTabListBox::GetRowDescription( sal_Int32 _nRow ) const
{
    return GetEntryText( _nRow );
}

OUString SvHeaderTabListBox::GetColumnDescription( sal_uInt16 _nColumn ) const
{
    return m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( _nColumn ) );
}

bool SvHeaderTabListBox::HasRowHeader() const
{
    return false;
}

bool SvHeaderTabListBox::GoToCell( sal_Int32 /*_nRow*/, sal_uInt16 /*_nColumn*/ )
{
    return false;
}

void SvHeaderTabListBox::SetNoSelection()
{
    SvTreeListBox::SelectAll(false);
}

void SvHeaderTabListBox::SelectAll()
{
    SvTreeListBox::SelectAll(true);
}

void SvHeaderTabListBox::SelectRow( sal_Int32 _nRow, bool _bSelect, bool )
{
    Select( GetEntry( _nRow ), _bSelect );
}

void SvHeaderTabListBox::SelectColumn( sal_uInt16, bool )
{
}

sal_Int32 SvHeaderTabListBox::GetSelectedRowCount() const
{
    return GetSelectionCount();
}

sal_Int32 SvHeaderTabListBox::GetSelectedColumnCount() const
{
    return 0;
}

bool SvHeaderTabListBox::IsRowSelected( sal_Int32 _nRow ) const
{
    SvTreeListEntry* pEntry = GetEntry( _nRow );
    return ( pEntry && IsSelected( pEntry ) );
}

bool SvHeaderTabListBox::IsColumnSelected( sal_Int32 ) const
{
    return false;
}

void SvHeaderTabListBox::GetAllSelectedRows( css::uno::Sequence< sal_Int32 >& ) const
{
}

void SvHeaderTabListBox::GetAllSelectedColumns( css::uno::Sequence< sal_Int32 >& ) const
{
}

bool SvHeaderTabListBox::IsCellVisible( sal_Int32, sal_uInt16 ) const
{
    return true;
}

OUString SvHeaderTabListBox::GetAccessibleCellText( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const
{
    return GetTabEntryText(_nRow, _nColumnPos);
}

tools::Rectangle SvHeaderTabListBox::calcHeaderRect( bool _bIsColumnBar, bool _bOnScreen )
{
    tools::Rectangle aRect;
    if ( _bIsColumnBar )
    {
        vcl::Window* pParent = nullptr;
        if ( !_bOnScreen )
            pParent = m_pImpl->m_pHeaderBar->GetAccessibleParentWindow();

        aRect = m_pImpl->m_pHeaderBar->GetWindowExtentsRelative( pParent );
    }
    return aRect;
}

tools::Rectangle SvHeaderTabListBox::calcTableRect( bool _bOnScreen )
{
    vcl::Window* pParent = nullptr;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    tools::Rectangle aRect( GetWindowExtentsRelative( pParent ) );
    return aRect;
}

tools::Rectangle SvHeaderTabListBox::GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumn, bool _bIsHeader, bool _bOnScreen )
{
    DBG_ASSERT( !_bIsHeader || 0 == _nRow, "invalid parameters" );
    tools::Rectangle aRect;
    SvTreeListEntry* pEntry = GetEntry( _nRow );
    if ( pEntry )
    {
        aRect = _bIsHeader ? calcHeaderRect( true, false ) : GetBoundingRect( pEntry );
        Point aTopLeft = aRect.TopLeft();
        DBG_ASSERT( m_pImpl->m_pHeaderBar->GetItemCount() > _nColumn, "invalid column" );
        tools::Rectangle aItemRect = m_pImpl->m_pHeaderBar->GetItemRect( m_pImpl->m_pHeaderBar->GetItemId( _nColumn ) );
        aTopLeft.setX( aItemRect.Left() );
        Size aSize = aItemRect.GetSize();
        aRect = tools::Rectangle( aTopLeft, aSize );
        vcl::Window* pParent = nullptr;
        if ( !_bOnScreen )
            pParent = GetAccessibleParentWindow();
        aTopLeft = aRect.TopLeft();
        aTopLeft += GetWindowExtentsRelative( pParent ).TopLeft();
        aRect = tools::Rectangle( aTopLeft, aRect.GetSize() );
    }

    return aRect;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    OSL_ENSURE( m_pAccessible, "Invalid call: Accessible is null" );

    Reference< XAccessible > xChild;

    TriState eState = TRISTATE_INDET;
    bool bIsCheckBox = IsCellCheckBox( _nRow, _nColumnPos, eState );
    if ( bIsCheckBox )
        xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleCheckBoxCell(
                m_pAccessible->getHeaderBar(), *this, nullptr, _nRow, _nColumnPos, eState, false );
    else
        xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxTableCell(
                m_pAccessible->getHeaderBar(), *this, nullptr, _nRow, _nColumnPos, OFFSET_NONE );

    return xChild;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleRowHeader( sal_Int32 )
{
    Reference< XAccessible > xHeader;
    return xHeader;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleColumnHeader( sal_uInt16 _nColumn )
{
    // first call? -> initial list
    if ( m_aAccessibleChildren.empty() )
    {
        const sal_uInt16 nColumnCount = GetColumnCount();
        m_aAccessibleChildren.assign( nColumnCount, Reference< XAccessible >() );
    }

    // get header
    Reference< XAccessible > xChild = m_aAccessibleChildren[ _nColumn ];
    // already exists?
    if ( !xChild.is() && m_pAccessible )
    {
        // no -> create new header cell
        xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxHeaderCell(
            _nColumn, m_pAccessible->getHeaderBar(),
            *this, nullptr, ::vcl::BBTYPE_COLUMNHEADERCELL
        );

        // insert into list
        m_aAccessibleChildren[ _nColumn ] = xChild;
    }
    return xChild;
}

sal_Int32 SvHeaderTabListBox::GetAccessibleControlCount() const
{
    return -1;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleControl( sal_Int32 )
{
    Reference< XAccessible > xControl;
    return xControl;
}

bool SvHeaderTabListBox::ConvertPointToControlIndex( sal_Int32&, const Point& )
{
    return false;
}

bool SvHeaderTabListBox::ConvertPointToCellAddress( sal_Int32&, sal_uInt16&, const Point& )
{
    return false;
}

bool SvHeaderTabListBox::ConvertPointToRowHeader( sal_Int32&, const Point& )
{
    return false;
}

bool SvHeaderTabListBox::ConvertPointToColumnHeader( sal_uInt16&, const Point& )
{
    return false;
}

OUString SvHeaderTabListBox::GetAccessibleObjectName( ::vcl::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    OUString aRetText;
    switch( _eType )
    {
        case ::vcl::BBTYPE_BROWSEBOX:
        case ::vcl::BBTYPE_TABLE:
        case ::vcl::BBTYPE_COLUMNHEADERBAR:
            // should be empty now (see #i63983)
            aRetText.clear();
            break;

        case ::vcl::BBTYPE_TABLECELL:
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
        case ::vcl::BBTYPE_CHECKBOXCELL:
        {
            break; // checkbox cells have no name
        }
        case ::vcl::BBTYPE_COLUMNHEADERCELL:
        {
            aRetText = m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( static_cast<sal_uInt16>(_nPos) ) );
            break;
        }

        case ::vcl::BBTYPE_ROWHEADERBAR:
        case ::vcl::BBTYPE_ROWHEADERCELL:
            aRetText = "error";
            break;

        default:
            OSL_FAIL("BrowseBox::GetAccessibleName: invalid enum!");
    }
    return aRetText;
}

OUString SvHeaderTabListBox::GetAccessibleObjectDescription( ::vcl::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    OUString aRetText;

    if( _eType == ::vcl::BBTYPE_TABLECELL && _nPos != -1 )
    {
        sal_uInt16 nColumnCount = GetColumnCount();
        if (nColumnCount > 0)
        {
            sal_Int32 nRow = _nPos / nColumnCount;
            sal_uInt16 nColumn  = static_cast< sal_uInt16 >( _nPos % nColumnCount );

            OUString aText( VclResId(STR_SVT_ACC_DESC_TABLISTBOX) );
            aText = aText.replaceFirst( "%1", OUString::number( nRow ) );
            OUString sColHeader = m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( nColumn ) );
            if ( sColHeader.isEmpty() )
                sColHeader = OUString::number( nColumn );
            aText = aText.replaceFirst( "%2", sColHeader );
            aRetText = aText;
        }
    }

    return aRetText;
}

void SvHeaderTabListBox::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& _rStateSet, ::vcl::AccessibleBrowseBoxObjType _eType ) const
{
    switch( _eType )
    {
        case ::vcl::BBTYPE_BROWSEBOX:
        case ::vcl::BBTYPE_TABLE:
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
            if ( _eType == ::vcl::BBTYPE_TABLE )
            {

                _rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
                _rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
            }
            break;
        }

        case ::vcl::BBTYPE_COLUMNHEADERBAR:
        {
            sal_Int32 nCurRow = GetCurrRow();
            sal_uInt16 nCurColumn = GetCurrColumn();
            if ( IsCellVisible( nCurRow, nCurColumn ) )
                _rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( IsEnabled() )
                _rStateSet.AddState( AccessibleStateType::ENABLED );
            _rStateSet.AddState( AccessibleStateType::TRANSIENT );
            break;
        }

        case ::vcl::BBTYPE_ROWHEADERCELL:
        case ::vcl::BBTYPE_COLUMNHEADERCELL:
        {
            _rStateSet.AddState( AccessibleStateType::VISIBLE );
            _rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            _rStateSet.AddState( AccessibleStateType::TRANSIENT );
            if ( IsEnabled() )
                _rStateSet.AddState( AccessibleStateType::ENABLED );
            break;
        }
        default:
            break;
    }
}

void SvHeaderTabListBox::FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumn ) const
{
    _rStateSet.AddState( AccessibleStateType::SELECTABLE );
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
    if ( IsEnabled() )
        _rStateSet.AddState( AccessibleStateType::ENABLED );
}

void SvHeaderTabListBox::GrabTableFocus()
{
    GrabFocus();
}

bool SvHeaderTabListBox::GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, MetricVector& rVector )
{
    return Control::GetGlyphBoundRects( rOrigin, rStr, nIndex, nLen, rVector );
}

tools::Rectangle SvHeaderTabListBox::GetWindowExtentsRelative(const vcl::Window *pRelativeWindow) const
{
    return Control::GetWindowExtentsRelative( pRelativeWindow );
}

void SvHeaderTabListBox::GrabFocus()
{
    Control::GrabFocus();
}

Reference< XAccessible > SvHeaderTabListBox::GetAccessible()
{
    return Control::GetAccessible();
}

vcl::Window* SvHeaderTabListBox::GetAccessibleParentWindow() const
{
    return Control::GetAccessibleParentWindow();
}

vcl::Window* SvHeaderTabListBox::GetWindowInstance()
{
    return this;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessible()
{
    vcl::Window* pParent = GetAccessibleParentWindow();
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

tools::Rectangle SvHeaderTabListBox::GetFieldCharacterBounds(sal_Int32,sal_Int32,sal_Int32)
{
    return tools::Rectangle();
}

sal_Int32 SvHeaderTabListBox::GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint)
{
    OUString sText = GetAccessibleCellText( _nRow, static_cast< sal_uInt16 >( _nColumnPos ) );
    MetricVector aRects;
    if ( GetGlyphBoundRects(Point(0,0), sText, 0, sText.getLength(), aRects) )
    {
        sal_Int32 nPos = 0;
        for (auto const& rectangle : aRects)
        {
            if( rectangle.IsInside(_rPoint) )
                return nPos;
            ++nPos;
        }
    }

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
