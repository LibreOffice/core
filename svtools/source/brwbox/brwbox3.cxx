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

#include <svtools/brwbox.hxx>
#include <vcl/AccessibleBrowseBoxObjType.hxx>
#include <vcl/accessiblefactory.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <tools/multisel.hxx>
#include "datwin.hxx"
#include "brwimpl.hxx"
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

// Accessibility ==============================================================

using namespace ::com::sun::star::uno;
using ::com::sun::star::accessibility::XAccessible;
using namespace ::com::sun::star::accessibility;


namespace svt
{
    using namespace ::com::sun::star::lang;
    using namespace utl;

    static Reference< XAccessible > getHeaderCell( BrowseBoxImpl::THeaderCellMap& _raHeaderCells,
                                            sal_Int32 _nPos,
                                            vcl::AccessibleBrowseBoxObjType _eType,
                                            const Reference< XAccessible >& _rParent,
                                            BrowseBox& _rBrowseBox,
                                            vcl::IAccessibleFactory const & rFactory
                                          )
    {
        Reference< XAccessible > xRet;
        BrowseBoxImpl::THeaderCellMap::iterator aFind = _raHeaderCells.find( _nPos );
        if ( aFind == _raHeaderCells.end() )
        {
            Reference< XAccessible > xAccessible = rFactory.createAccessibleBrowseBoxHeaderCell(
                _nPos,
                _rParent,
                _rBrowseBox,
                nullptr,
                _eType
            );
            aFind = _raHeaderCells.emplace( _nPos, xAccessible ).first;
        }
        if ( aFind != _raHeaderCells.end() )
            xRet = aFind->second;
        return xRet;
    }


    Reference< XAccessible > BrowseBoxImpl::getAccessibleHeaderBar( vcl::AccessibleBrowseBoxObjType _eObjType )
    {
        if ( m_pAccessible && m_pAccessible->isAlive() )
            return m_pAccessible->getHeaderBar( _eObjType );
        return nullptr;
    }


    Reference< XAccessible > BrowseBoxImpl::getAccessibleTable( )
    {
        if ( m_pAccessible && m_pAccessible->isAlive() )
            return m_pAccessible->getTable( );
        return nullptr;
    }
}


Reference< XAccessible > BrowseBox::CreateAccessible()
{
    vcl::Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "BrowseBox::CreateAccessible - parent not found" );

    if( pParent && !m_pImpl->m_pAccessible)
    {
        Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if( xAccParent.is() )
        {
            m_pImpl->m_pAccessible = getAccessibleFactory().createAccessibleBrowseBox(
                xAccParent, *this
            );
        }
    }

    Reference< XAccessible > xAccessible;
    if ( m_pImpl->m_pAccessible )
        xAccessible = m_pImpl->m_pAccessible->getMyself();

    return xAccessible;
}


// Children -------------------------------------------------------------------

Reference< XAccessible > BrowseBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    // BBINDEX_TABLE must be the table
    OSL_ENSURE(m_pImpl->m_pAccessible,"Invalid call: Accessible is null");

    return m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxTableCell(
        m_pImpl->getAccessibleTable(),
        *this,
        nullptr,
        _nRow,
        _nColumnPos,
        OFFSET_DEFAULT
    );
}


Reference< XAccessible > BrowseBox::CreateAccessibleRowHeader( sal_Int32 _nRow )
{
    return svt::getHeaderCell(
        m_pImpl->m_aRowHeaderCellMap,
        _nRow,
        vcl::BBTYPE_ROWHEADERCELL,
        m_pImpl->getAccessibleHeaderBar(vcl::BBTYPE_ROWHEADERBAR),
        *this,
        m_pImpl->m_aFactoryAccess.getFactory()
    );
}


Reference< XAccessible > BrowseBox::CreateAccessibleColumnHeader( sal_uInt16 _nColumnPos )
{
    return svt::getHeaderCell(
            m_pImpl->m_aColHeaderCellMap,
            _nColumnPos,
            vcl::BBTYPE_COLUMNHEADERCELL,
            m_pImpl->getAccessibleHeaderBar(vcl::BBTYPE_COLUMNHEADERBAR),
            *this,
            m_pImpl->m_aFactoryAccess.getFactory()
    );
}


sal_Int32 BrowseBox::GetAccessibleControlCount() const
{
    return 0;
}


Reference< XAccessible > BrowseBox::CreateAccessibleControl( sal_Int32 )
{
    SAL_WARN( "svtools", "BrowseBox::CreateAccessibleControl: to be overwritten!" );
    return nullptr;
}


// Conversions ----------------------------------------------------------------

bool BrowseBox::ConvertPointToCellAddress(
        sal_Int32& rnRow, sal_uInt16& rnColumnPos, const Point& rPoint )
{
    //! TODO has to be checked
    rnRow = GetRowAtYPosPixel(rPoint.Y());
    rnColumnPos = GetColumnAtXPosPixel(rPoint.X());
    return rnRow != BROWSER_INVALIDID && rnColumnPos != BROWSER_INVALIDID;
}


bool BrowseBox::ConvertPointToRowHeader( sal_Int32& rnRow, const Point& rPoint )
{
    rnRow = GetRowAtYPosPixel(rPoint.Y());
    //  sal_uInt16 nColumnId = GetColumnAtXPosPixel(rPoint.X());
    return rnRow != BROWSER_INVALIDID;// && nColumnId == 0;
}


bool BrowseBox::ConvertPointToColumnHeader( sal_uInt16& _rnColumnPos, const Point& _rPoint )
{
    _rnColumnPos = GetColumnAtXPosPixel(_rPoint.X());
    return _rnColumnPos != BROWSER_INVALIDID;
}


bool BrowseBox::ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint )
{
    //! TODO has to be checked
    sal_Int32 nRow = 0;
    sal_uInt16 nColumn = 0;
    bool bRet = ConvertPointToCellAddress(nRow,nColumn,_rPoint);
    if ( bRet )
        _rnIndex = nRow * ColCount() + nColumn;

    return bRet;
}


// Object data and state ------------------------------------------------------

OUString BrowseBox::GetAccessibleObjectName( ::vcl::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition) const
{
    OUString aRetText;
    switch( eObjType )
    {
        case ::vcl::BBTYPE_BROWSEBOX:
            aRetText = "BrowseBox";
            break;
        case ::vcl::BBTYPE_TABLE:
            aRetText = "Table";
            break;
        case ::vcl::BBTYPE_ROWHEADERBAR:
            aRetText = "RowHeaderBar";
            break;
        case ::vcl::BBTYPE_COLUMNHEADERBAR:
            aRetText = "ColumnHeaderBar";
            break;
        case ::vcl::BBTYPE_TABLECELL:
            if( ColCount() !=0 && GetRowCount()!=0)
            {

                sal_Int32 columnId = _nPosition % ColCount() +1;
                aRetText = GetColumnDescription( sal_Int16( columnId ) );
                sal_Int32 rowId = _nPosition / GetRowCount() + 1;
                aRetText += OUString::number(rowId);
            }
            else
                aRetText = "TableCell";
#if OSL_DEBUG_LEVEL > 0
            aRetText += " ["
                        + OUString::number(sal_Int32(GetCurRow()))
                        + ","
                        + OUString::number(sal_Int32(GetCurColumnId()))
                        + "]";
#endif
            break;
        case ::vcl::BBTYPE_ROWHEADERCELL:
            {
                sal_Int32 rowId = _nPosition + 1;
                aRetText = OUString::number( rowId );
            }
#if OSL_DEBUG_LEVEL > 0
            aRetText += " ["
                        + OUString::number(sal_Int32(GetCurRow()))
                        + ","
                        + OUString::number(sal_Int32(GetCurColumnId()))
                        + "]";
#endif
            break;
        case ::vcl::BBTYPE_COLUMNHEADERCELL:
            aRetText = GetColumnDescription( sal_Int16( _nPosition ) );
#if OSL_DEBUG_LEVEL > 0
            aRetText += " ["
                        + OUString::number(sal_Int32(GetCurRow()))
                        + ","
                        + OUString::number(sal_Int32(GetCurColumnId()))
                        + "]";
#endif
            break;
        default:
            OSL_FAIL("BrowseBox::GetAccessibleName: invalid enum!");
    }
    return aRetText;
}


OUString BrowseBox::GetAccessibleObjectDescription( ::vcl::AccessibleBrowseBoxObjType eObjType,sal_Int32 ) const
{
    OUString aRetText;
    switch( eObjType )
    {
        case ::vcl::BBTYPE_BROWSEBOX:
            aRetText = "BrowseBox description";
            break;
        case ::vcl::BBTYPE_TABLE:
            //  aRetText = "TABLE description";
            break;
        case ::vcl::BBTYPE_ROWHEADERBAR:
            //  aRetText = "ROWHEADERBAR description";
            break;
        case ::vcl::BBTYPE_COLUMNHEADERBAR:
            //  aRetText = "COLUMNHEADERBAR description";
            break;
        case ::vcl::BBTYPE_TABLECELL:
            //  aRetText = "TABLECELL description";
            break;
        case ::vcl::BBTYPE_ROWHEADERCELL:
            //  aRetText = "ROWHEADERCELL description";
            break;
        case ::vcl::BBTYPE_COLUMNHEADERCELL:
            //  aRetText = "COLUMNHEADERCELL description";
            break;
        case ::vcl::BBTYPE_CHECKBOXCELL:
            break;
    }
    return aRetText;
}


OUString BrowseBox::GetRowDescription( sal_Int32 ) const
{
    return OUString();
}


OUString BrowseBox::GetColumnDescription( sal_uInt16 _nColumn ) const
{
    return GetColumnTitle( GetColumnId( _nColumn ) );
}


void BrowseBox::FillAccessibleStateSet(
        ::utl::AccessibleStateSetHelper& rStateSet,
        ::vcl::AccessibleBrowseBoxObjType eObjType ) const
{
    switch( eObjType )
    {
        case ::vcl::BBTYPE_BROWSEBOX:
        case ::vcl::BBTYPE_TABLE:

            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            if ( HasFocus() )
                rStateSet.AddState( AccessibleStateType::FOCUSED );
            if ( IsActive() )
                rStateSet.AddState( AccessibleStateType::ACTIVE );
            if ( GetUpdateMode() )
                rStateSet.AddState( AccessibleStateType::EDITABLE );
            if ( IsEnabled() )
            {
                rStateSet.AddState( AccessibleStateType::ENABLED );
                rStateSet.AddState( AccessibleStateType::SENSITIVE );
            }
            if ( IsReallyVisible() )
                rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( eObjType == ::vcl::BBTYPE_TABLE )
                rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );

            break;
        case ::vcl::BBTYPE_ROWHEADERBAR:
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( GetSelectRowCount() )
                rStateSet.AddState( AccessibleStateType::FOCUSED );
            rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
            break;
        case ::vcl::BBTYPE_COLUMNHEADERBAR:
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::VISIBLE );
            if ( GetSelectColumnCount() )
                rStateSet.AddState( AccessibleStateType::FOCUSED );
            rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
            break;
        case ::vcl::BBTYPE_TABLECELL:
            {
                sal_Int32 nRow = GetCurRow();
                sal_uInt16 nColumn = GetCurColumnId();
                if ( IsFieldVisible(nRow,nColumn) )
                    rStateSet.AddState( AccessibleStateType::VISIBLE );
                if ( !IsFrozen( nColumn ) )
                    rStateSet.AddState( AccessibleStateType::FOCUSABLE );
                rStateSet.AddState( AccessibleStateType::TRANSIENT );
            }
            break;
        case ::vcl::BBTYPE_ROWHEADERCELL:
        case ::vcl::BBTYPE_COLUMNHEADERCELL:
        case ::vcl::BBTYPE_CHECKBOXCELL:
            OSL_FAIL("Illegal call here!");
            break;
    }
}

void BrowseBox::FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet,
                                               sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const
{
    //! TODO check if the state is valid for table cells
    if ( IsCellVisible( _nRow, _nColumnPos ) )
        _rStateSet.AddState( AccessibleStateType::VISIBLE );
    if ( GetCurrRow() == _nRow && GetCurrColumn() == _nColumnPos )
        _rStateSet.AddState( AccessibleStateType::FOCUSED );
    else // only transient when column is not focused
        _rStateSet.AddState( AccessibleStateType::TRANSIENT );
}


void BrowseBox::GrabTableFocus()
{
    GrabFocus();
}

OUString BrowseBox::GetCellText(long, sal_uInt16 ) const
{
    SAL_WARN("svtools", "This method has to be implemented by the derived classes! BUG!!");
    return OUString();
}


void BrowseBox::commitHeaderBarEvent(sal_Int16 nEventId,
        const Any& rNewValue, const Any& rOldValue, bool _bColumnHeaderBar )
{
    if ( isAccessibleAlive() )
        m_pImpl->m_pAccessible->commitHeaderBarEvent( nEventId,
            rNewValue, rOldValue, _bColumnHeaderBar );
}

void BrowseBox::commitTableEvent( sal_Int16 _nEventId, const Any& _rNewValue, const Any& _rOldValue )
{
    if ( isAccessibleAlive() )
        m_pImpl->m_pAccessible->commitTableEvent( _nEventId, _rNewValue, _rOldValue );
}

void BrowseBox::commitBrowseBoxEvent( sal_Int16 _nEventId, const Any& _rNewValue, const Any& _rOldValue )
{
    if ( isAccessibleAlive() )
        m_pImpl->m_pAccessible->commitEvent( _nEventId, _rNewValue, _rOldValue);
}

::vcl::IAccessibleFactory& BrowseBox::getAccessibleFactory()
{
    return m_pImpl->m_aFactoryAccess.getFactory();
}

bool BrowseBox::isAccessibleAlive( ) const
{
    return ( nullptr != m_pImpl->m_pAccessible ) && m_pImpl->m_pAccessible->isAlive();
}

// IAccessibleTableProvider

sal_Int32 BrowseBox::GetCurrRow() const
{
    return GetCurRow();
}

sal_uInt16 BrowseBox::GetCurrColumn() const
{
    return GetColumnPos( GetCurColumnId() );
}

bool BrowseBox::HasRowHeader() const
{
    return ( GetColumnId( 0 ) == HandleColumnId ); // HandleColumn == RowHeader
}

bool BrowseBox::GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumn )
{
    return GoToRowColumnId( _nRow, GetColumnId( _nColumn ) );
}

void BrowseBox::SelectColumn( sal_uInt16 _nColumn, bool _bSelect )
{
    SelectColumnPos( _nColumn, _bSelect );
}

bool BrowseBox::IsColumnSelected( long _nColumn ) const
{
    return ( pColSel && (0 <= _nColumn) && (_nColumn <= 0xFFF) ) &&
        pColSel->IsSelected( static_cast< sal_uInt16 >( _nColumn ) );
}

sal_Int32 BrowseBox::GetSelectedRowCount() const
{
    return GetSelectRowCount();
}

sal_Int32 BrowseBox::GetSelectedColumnCount() const
{
    const MultiSelection* pColumnSel = GetColumnSelection();
    return pColumnSel ? pColumnSel->GetSelectCount() : 0;
}

void BrowseBox::GetAllSelectedRows( css::uno::Sequence< sal_Int32 >& _rRows ) const
{
    sal_Int32 nCount = GetSelectRowCount();
    if( nCount )
    {
        _rRows.realloc( nCount );
        _rRows[ 0 ] = const_cast< BrowseBox* >( this )->FirstSelectedRow();
        for( sal_Int32 nIndex = 1; nIndex < nCount; ++nIndex )
            _rRows[ nIndex ] = const_cast< BrowseBox* >( this )->NextSelectedRow();
        DBG_ASSERT( const_cast< BrowseBox* >( this )->NextSelectedRow() == BROWSER_ENDOFSELECTION,
                    "BrowseBox::GetAllSelectedRows - too many selected rows found" );
    }
}

void BrowseBox::GetAllSelectedColumns( css::uno::Sequence< sal_Int32 >& _rColumns ) const
{
    const MultiSelection* pColumnSel = GetColumnSelection();
    sal_Int32 nCount = GetSelectedColumnCount();
    if( !(pColumnSel && nCount) )
        return;

    _rColumns.realloc( nCount );

    sal_Int32 nIndex = 0;
    const size_t nRangeCount = pColumnSel->GetRangeCount();
    for( size_t nRange = 0; nRange < nRangeCount; ++nRange )
    {
        const Range& rRange = pColumnSel->GetRange( nRange );
        // loop has to include aRange.Max()
        for( sal_Int32 nCol = rRange.Min(); nCol <= static_cast<sal_Int32>(rRange.Max()); ++nCol )
        {
            DBG_ASSERT( nIndex < nCount,
                "GetAllSelectedColumns - range overflow" );
            _rColumns[ nIndex ] = nCol;
            ++nIndex;
        }
    }
}

bool BrowseBox::IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const
{
    return IsFieldVisible( _nRow, GetColumnId( _nColumnPos ) );
}

OUString BrowseBox::GetAccessibleCellText(long _nRow, sal_uInt16 _nColPos) const
{
    return GetCellText( _nRow, GetColumnId( _nColPos ) );
}


bool BrowseBox::GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, MetricVector& rVector )
{
    return Control::GetGlyphBoundRects( rOrigin, rStr, nIndex, nLen, rVector );
}

tools::Rectangle BrowseBox::GetWindowExtentsRelative( vcl::Window *pRelativeWindow ) const
{
    return Control::GetWindowExtentsRelative( pRelativeWindow );
}

void BrowseBox::GrabFocus()
{
    Control::GrabFocus();
}

Reference< XAccessible > BrowseBox::GetAccessible()
{
    return Control::GetAccessible();
}

vcl::Window* BrowseBox::GetAccessibleParentWindow() const
{
    return Control::GetAccessibleParentWindow();
}

vcl::Window* BrowseBox::GetWindowInstance()
{
    return this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
