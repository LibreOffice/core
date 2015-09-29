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

#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>

#include <algorithm>
#include <list>
#include <set>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include <frmfmt.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <swtable.hxx>
#include <crsrsh.hxx>
#include <viscrs.hxx>
#include <hints.hxx>
#include <fesh.hxx>
#include <accfrmobjslist.hxx>
#include <accmap.hxx>
#include <access.hrc>
#include <acctable.hxx>

#include <layfrm.hxx>
#include <com/sun/star/accessibility/XAccessibleText.hpp>

#include <editeng/brushitem.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::sw::access;

typedef ::std::set < sal_Int32 > Int32Set_Impl;
typedef ::std::pair < sal_Int32, sal_Int32 > Int32Pair_Impl;

const unsigned int SELECTION_WITH_NUM = 10;

class SwAccTableSelHander_Impl
{
public:
    virtual void Unselect( sal_Int32 nRowOrCol, sal_Int32 nExt ) = 0;

protected:
    ~SwAccTableSelHander_Impl() {}
};

class SwAccessibleTableData_Impl
{
    SwAccessibleMap& mrAccMap;
    Int32Set_Impl   maRows;
    Int32Set_Impl   maColumns;
    ::std::list < Int32Pair_Impl > maExtents;   // cell extends for event processing only
    Point   maTabFrmPos;
    const SwTabFrm *mpTabFrm;
    bool mbIsInPagePreview;
    bool mbOnlyTableColumnHeader;

    void CollectData( const SwFrm *pFrm );
    void CollectColumnHeaderData( const SwFrm *pFrm );
    void CollectRowHeaderData( const SwFrm *pFrm );
    void CollectExtents( const SwFrm *pFrm );

    bool FindCell( const Point& rPos, const SwFrm *pFrm ,
                           bool bExact, const SwFrm *& rFrm ) const;

    void GetSelection( const Point& rTabPos, const SwRect& rArea,
                       const SwSelBoxes& rSelBoxes, const SwFrm *pFrm,
                       SwAccTableSelHander_Impl& rSelHdl,
                       bool bColumns ) const;

    // #i77106#
    inline bool IncludeRow( const SwFrm& rFrm ) const
    {
        return !mbOnlyTableColumnHeader ||
               mpTabFrm->IsInHeadline( rFrm );
    }
public:
    // #i77106# - add third optional parameter <bOnlyTableColumnHeader>, default value <false>
    SwAccessibleTableData_Impl( SwAccessibleMap& rAccMap,
                                const SwTabFrm *pTabFrm,
                                bool bIsInPagePreview,
                                bool bOnlyTableColumnHeader = false );

    const Int32Set_Impl& GetRows() const { return maRows; }
    const Int32Set_Impl& GetColumns() const { return maColumns; }

    inline Int32Set_Impl::const_iterator GetRowIter( sal_Int32 nRow ) const;
    inline Int32Set_Impl::const_iterator GetColumnIter( sal_Int32 nCol ) const;

    const SwFrm *GetCell( sal_Int32 nRow, sal_Int32 nColumn, bool bExact,
                          SwAccessibleTable *pThis ) const
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception);
    const SwFrm *GetCellAtPos( sal_Int32 nLeft, sal_Int32 nTop,
                                         bool bExact ) const;
    inline sal_Int32 GetRowCount() const;
    inline sal_Int32 GetColumnCount() const;
    bool CompareExtents( const SwAccessibleTableData_Impl& r ) const;

    void GetSelection( sal_Int32 nStart, sal_Int32 nEnd,
                       const SwSelBoxes& rSelBoxes,
                          SwAccTableSelHander_Impl& rSelHdl,
                       bool bColumns ) const;

    void CheckRowAndCol( sal_Int32 nRow, sal_Int32 nCol,
                         SwAccessibleTable *pThis ) const
        throw(lang::IndexOutOfBoundsException );

    void GetRowColumnAndExtent( const SwRect& rBox,
                                  sal_Int32& rRow, sal_Int32& rColumn,
                                  sal_Int32& rRowExtent,
                                  sal_Int32& rColumnExtent ) const;

    const Point& GetTablePos() const { return maTabFrmPos; }
    void SetTablePos( const Point& rPos ) { maTabFrmPos = rPos; }
};

void SwAccessibleTableData_Impl::CollectData( const SwFrm *pFrm )
{
    const SwAccessibleChildSList aList( *pFrm, mrAccMap );
    SwAccessibleChildSList::const_iterator aIter( aList.begin() );
    SwAccessibleChildSList::const_iterator aEndIter( aList.end() );
    while( aIter != aEndIter )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        if( pLower )
        {
            if( pLower->IsRowFrm() )
            {
                // #i77106#
                if ( IncludeRow( *pLower ) )
                {
                    maRows.insert( pLower->Frm().Top() - maTabFrmPos.getY() );
                    CollectData( pLower );
                }
            }
            else if( pLower->IsCellFrm() &&
                     rLower.IsAccessible( mbIsInPagePreview ) )
            {
                maColumns.insert( pLower->Frm().Left() - maTabFrmPos.getX() );
            }
            else
            {
                CollectData( pLower );
            }
        }
        ++aIter;
    }
}

void SwAccessibleTableData_Impl::CollectRowHeaderData( const SwFrm *pFrm )
{
    const SwAccessibleChildSList aList( *pFrm, mrAccMap );
    SwAccessibleChildSList::const_iterator aIter( aList.begin() );
    SwAccessibleChildSList::const_iterator aEndIter( aList.end() );
    while( aIter != aEndIter )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        if( pLower )
        {
            if( pLower->IsRowFrm() )
            {

                const SwTableLine* pLine = static_cast<const SwRowFrm*>(pLower)->GetTabLine();
                while( pLine->GetUpper() )
                    pLine = pLine->GetUpper()->GetUpper();

                // Headerline?
                //if(mpTabFrm->GetTable()->GetTabLines()[ 0 ] != pLine)
                //return ;

                maRows.insert( pLower->Frm().Top() - maTabFrmPos.Y() );

                CollectRowHeaderData( pLower );

            }
            else if( pLower->IsCellFrm() &&
                     rLower.IsAccessible( mbIsInPagePreview ) )
            {
                //Added by yanjun. Can't find the "GetRowHeaderFlag" function (need verify).
                //if(static_cast<SwCellFrm*>(pLower)->GetRowHeaderFlag())
                //  maColumns.insert( pLower->Frm().Left() - maTabFrmPos.X() );
            }
            else
            {
                CollectRowHeaderData( pLower );
            }
        }
        ++aIter;
    }
}

void SwAccessibleTableData_Impl::CollectColumnHeaderData( const SwFrm *pFrm )
{
    const SwAccessibleChildSList aList( *pFrm, mrAccMap );
    SwAccessibleChildSList::const_iterator aIter( aList.begin() );
    SwAccessibleChildSList::const_iterator aEndIter( aList.end() );
    while( aIter != aEndIter )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        if( pLower )
        {
            if( pLower->IsRowFrm() )
            {

                const SwTableLine* pLine = static_cast<const SwRowFrm*>(pLower)->GetTabLine();
                while( pLine->GetUpper() )
                    pLine = pLine->GetUpper()->GetUpper();

                // Headerline?
                //if(mpTabFrm->GetTable()->GetTabLines()[ 0 ] != pLine)
                //return ;

                //if the current line is now header line, then return ;
                sal_Int16 iCurrentRowIndex = mpTabFrm->GetTable()->GetTabLines().GetPos( pLine);
                if(iCurrentRowIndex >= mpTabFrm->GetTable()->_GetRowsToRepeat())
                    return ;

                maRows.insert( pLower->Frm().Top() - maTabFrmPos.Y() );

                CollectColumnHeaderData( pLower );

            }
            else if( pLower->IsCellFrm() &&
                     rLower.IsAccessible( mbIsInPagePreview ) )
            {
                maColumns.insert( pLower->Frm().Left() - maTabFrmPos.X() );
            }
            else
            {
                CollectColumnHeaderData( pLower );
            }
        }
        ++aIter;
    }
}

void SwAccessibleTableData_Impl::CollectExtents( const SwFrm *pFrm )
{
    const SwAccessibleChildSList aList( *pFrm, mrAccMap );
    SwAccessibleChildSList::const_iterator aIter( aList.begin() );
    SwAccessibleChildSList::const_iterator aEndIter( aList.end() );
    while( aIter != aEndIter )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        if( pLower )
        {
            if( pLower->IsCellFrm() &&
                rLower.IsAccessible( mbIsInPagePreview ) )
            {
                sal_Int32 nRow, nCol;
                Int32Pair_Impl aCellExtents;
                GetRowColumnAndExtent( pLower->Frm(), nRow, nCol,
                                       aCellExtents.first,
                                       aCellExtents.second );

                maExtents.push_back( aCellExtents );
            }
            else
            {
                // #i77106#
                if ( !pLower->IsRowFrm() ||
                     IncludeRow( *pLower ) )
                {
                    CollectExtents( pLower );
                }
            }
        }
        ++aIter;
    }
}

bool SwAccessibleTableData_Impl::FindCell(
        const Point& rPos, const SwFrm *pFrm, bool bExact,
        const SwFrm *& rRet ) const
{
    bool bFound = false;

    const SwAccessibleChildSList aList( *pFrm, mrAccMap );
    SwAccessibleChildSList::const_iterator aIter( aList.begin() );
    SwAccessibleChildSList::const_iterator aEndIter( aList.end() );
    while( !bFound && aIter != aEndIter )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        OSL_ENSURE( pLower, "child should be a frame" );
        if( pLower )
        {
            if( rLower.IsAccessible( mbIsInPagePreview ) )
            {
                OSL_ENSURE( pLower->IsCellFrm(), "lower is not a cell frame" );
                const SwRect& rFrm = pLower->Frm();
                if( rFrm.Right() >= rPos.X() && rFrm.Bottom() >= rPos.Y() )
                {
                    // We have found the cell
                    OSL_ENSURE( rFrm.Left() <= rPos.X() && rFrm.Top() <= rPos.Y(),
                            "find frame moved to far!" );
                    bFound = true;
                    if( !bExact ||
                        (rFrm.Top() == rPos.Y() && rFrm.Left() == rPos.Y() ) )
                    {
                        rRet = pLower;
                    }
                }
            }
            else
            {
                // #i77106#
                if ( !pLower->IsRowFrm() ||
                     IncludeRow( *pLower ) )
                {
                    bFound = FindCell( rPos, pLower, bExact, rRet );
                }
            }
        }
        ++aIter;
    }

    return bFound;
}

void SwAccessibleTableData_Impl::GetSelection(
            const Point& rTabPos,
            const SwRect& rArea,
            const SwSelBoxes& rSelBoxes,
            const SwFrm *pFrm,
            SwAccTableSelHander_Impl& rSelHdl,
            bool bColumns ) const
{
    const SwAccessibleChildSList aList( *pFrm, mrAccMap );
    SwAccessibleChildSList::const_iterator aIter( aList.begin() );
    SwAccessibleChildSList::const_iterator aEndIter( aList.end() );
    while( aIter != aEndIter )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        OSL_ENSURE( pLower, "child should be a frame" );
        const SwRect& rBox = rLower.GetBox( mrAccMap );
        if( pLower && rBox.IsOver( rArea ) )
        {
            if( rLower.IsAccessible( mbIsInPagePreview ) )
            {
                OSL_ENSURE( pLower->IsCellFrm(), "lower is not a cell frame" );
                const SwCellFrm *pCFrm =
                        static_cast < const SwCellFrm * >( pLower );
                SwTableBox *pBox =
                    const_cast< SwTableBox *>( pCFrm->GetTabBox() );
                if( rSelBoxes.find( pBox ) == rSelBoxes.end() )
                {
                    const Int32Set_Impl rRowsOrCols =
                        bColumns ? maColumns : maRows;

                    sal_Int32 nPos = bColumns ? (rBox.Left() - rTabPos.X())
                                              : (rBox.Top() - rTabPos.Y());
                    Int32Set_Impl::const_iterator aSttRowOrCol(
                        rRowsOrCols.lower_bound( nPos ) );
                    sal_Int32 nRowOrCol =
                        static_cast< sal_Int32 >( ::std::distance(
                            rRowsOrCols.begin(), aSttRowOrCol ) );

                    nPos = bColumns ? (rBox.Right() - rTabPos.X())
                                    : (rBox.Bottom() - rTabPos.Y());
                    Int32Set_Impl::const_iterator aEndRowOrCol(
                        rRowsOrCols.upper_bound( nPos ) );
                    sal_Int32 nExt =
                        static_cast< sal_Int32 >( ::std::distance(
                            aSttRowOrCol, aEndRowOrCol ) );

                    rSelHdl.Unselect( nRowOrCol, nExt );
                }
            }
            else
            {
                // #i77106#
                if ( !pLower->IsRowFrm() ||
                     IncludeRow( *pLower ) )
                {
                    GetSelection( rTabPos, rArea, rSelBoxes, pLower, rSelHdl,
                                  bColumns );
                }
            }
        }
        ++aIter;
    }
}

const SwFrm *SwAccessibleTableData_Impl::GetCell(
        sal_Int32 nRow, sal_Int32 nColumn, bool,
        SwAccessibleTable *pThis ) const
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    CheckRowAndCol( nRow, nColumn, pThis );

    Int32Set_Impl::const_iterator aSttCol( GetColumnIter( nColumn ) );
    Int32Set_Impl::const_iterator aSttRow( GetRowIter( nRow ) );
    const SwFrm *pCellFrm = GetCellAtPos( *aSttCol, *aSttRow, false );

    return pCellFrm;
}

void SwAccessibleTableData_Impl::GetSelection(
                       sal_Int32 nStart, sal_Int32 nEnd,
                       const SwSelBoxes& rSelBoxes,
                          SwAccTableSelHander_Impl& rSelHdl,
                       bool bColumns ) const
{
    SwRect aArea( mpTabFrm->Frm() );
    Point aPos( aArea.Pos() );

    const Int32Set_Impl& rRowsOrColumns = bColumns ? maColumns : maRows;
    if( nStart > 0 )
    {
        Int32Set_Impl::const_iterator aStt( rRowsOrColumns.begin() );
        ::std::advance( aStt,
            static_cast< Int32Set_Impl::difference_type >( nStart ) );
        if( bColumns )
            aArea.Left( *aStt + aPos.getX() );
        else
            aArea.Top( *aStt + aPos.getY() );
    }
    if( nEnd < static_cast< sal_Int32 >( rRowsOrColumns.size() ) )
    {
        Int32Set_Impl::const_iterator aEnd( rRowsOrColumns.begin() );
        ::std::advance( aEnd,
            static_cast< Int32Set_Impl::difference_type >( nEnd ) );
        if( bColumns )
            aArea.Right( *aEnd + aPos.getX() - 1 );
        else
            aArea.Bottom( *aEnd + aPos.getY() - 1 );
    }

    GetSelection( aPos, aArea, rSelBoxes, mpTabFrm, rSelHdl, bColumns );
}

const SwFrm *SwAccessibleTableData_Impl::GetCellAtPos(
        sal_Int32 nLeft, sal_Int32 nTop, bool bExact ) const
{
    Point aPos( mpTabFrm->Frm().Pos() );
    aPos.Move( nLeft, nTop );
    const SwFrm *pRet = 0;
    FindCell( aPos, mpTabFrm, bExact, pRet );

    return pRet;
}

inline sal_Int32 SwAccessibleTableData_Impl::GetRowCount() const
{
    sal_Int32 count =  static_cast< sal_Int32 >( maRows.size() ) ;
    count = (count <=0)? 1:count;
    return count;
}

inline sal_Int32 SwAccessibleTableData_Impl::GetColumnCount() const
{
    return static_cast< sal_Int32 >( maColumns.size() );
}

bool SwAccessibleTableData_Impl::CompareExtents(
                                const SwAccessibleTableData_Impl& rCmp ) const
{
    if( maExtents.size() != rCmp.maExtents.size() )
        return false;

    return ::std::equal(maExtents.begin(), maExtents.end(), rCmp.maExtents.begin());
}

SwAccessibleTableData_Impl::SwAccessibleTableData_Impl( SwAccessibleMap& rAccMap,
                                                        const SwTabFrm *pTabFrm,
                                                        bool bIsInPagePreview,
                                                        bool bOnlyTableColumnHeader )
    : mrAccMap( rAccMap )
    , maTabFrmPos( pTabFrm->Frm().Pos() )
    , mpTabFrm( pTabFrm )
    , mbIsInPagePreview( bIsInPagePreview )
    , mbOnlyTableColumnHeader( bOnlyTableColumnHeader )
{
    CollectData( mpTabFrm );
    CollectExtents( mpTabFrm );
}

inline Int32Set_Impl::const_iterator SwAccessibleTableData_Impl::GetRowIter(
        sal_Int32 nRow ) const
{
    Int32Set_Impl::const_iterator aCol( GetRows().begin() );
    if( nRow > 0 )
    {
        ::std::advance( aCol,
                    static_cast< Int32Set_Impl::difference_type >( nRow ) );
    }
    return aCol;
}

inline Int32Set_Impl::const_iterator SwAccessibleTableData_Impl::GetColumnIter(
        sal_Int32 nColumn ) const
{
    Int32Set_Impl::const_iterator aCol = GetColumns().begin();
    if( nColumn > 0 )
    {
        ::std::advance( aCol,
                    static_cast< Int32Set_Impl::difference_type >( nColumn ) );
    }
    return aCol;
}

void SwAccessibleTableData_Impl::CheckRowAndCol(
        sal_Int32 nRow, sal_Int32 nCol, SwAccessibleTable *pThis ) const
    throw(lang::IndexOutOfBoundsException )
{
    if( ( nRow < 0 || nRow >= static_cast< sal_Int32 >( maRows.size() ) ) ||
        ( nCol < 0 || nCol >= static_cast< sal_Int32 >( maColumns.size() ) ) )
    {
        uno::Reference < XAccessibleTable > xThis( pThis );
        lang::IndexOutOfBoundsException aExcept(
               OUString( "row or column index out of range" ),
               xThis );
        throw aExcept;
    }
}

void SwAccessibleTableData_Impl::GetRowColumnAndExtent(
        const SwRect& rBox,
        sal_Int32& rRow, sal_Int32& rColumn,
        sal_Int32& rRowExtent, sal_Int32& rColumnExtent ) const
{
    Int32Set_Impl::const_iterator aStt(
                maRows.lower_bound( rBox.Top() - maTabFrmPos.Y() ) );
    Int32Set_Impl::const_iterator aEnd(
                maRows.upper_bound( rBox.Bottom() - maTabFrmPos.Y() ) );
    rRow =
         static_cast< sal_Int32 >( ::std::distance( maRows.begin(), aStt ) );
    rRowExtent =
         static_cast< sal_Int32 >( ::std::distance( aStt, aEnd ) );

    aStt = maColumns.lower_bound( rBox.Left() - maTabFrmPos.X() );
    aEnd = maColumns.upper_bound( rBox.Right() - maTabFrmPos.X() );
    rColumn =
         static_cast< sal_Int32 >( ::std::distance( maColumns.begin(), aStt ) );
    rColumnExtent =
         static_cast< sal_Int32 >( ::std::distance( aStt, aEnd ) );
}

class SwAccSingleTableSelHander_Impl : public SwAccTableSelHander_Impl
{
    bool bSelected;

public:

    inline SwAccSingleTableSelHander_Impl();

    virtual ~SwAccSingleTableSelHander_Impl() {}

    inline bool IsSelected() const { return bSelected; }

    virtual void Unselect( sal_Int32, sal_Int32 ) SAL_OVERRIDE;
};

inline SwAccSingleTableSelHander_Impl::SwAccSingleTableSelHander_Impl() :
    bSelected( true )
{
}

void SwAccSingleTableSelHander_Impl::Unselect( sal_Int32, sal_Int32 )
{
    bSelected = false;
}

class SwAccAllTableSelHander_Impl : public SwAccTableSelHander_Impl

{
    ::std::vector< bool > aSelected;
    sal_Int32 nCount;

public:
    explicit SwAccAllTableSelHander_Impl(sal_Int32 nSize)
        : aSelected(nSize, true)
        , nCount(nSize)
    {
    }

    uno::Sequence < sal_Int32 > GetSelSequence();

    virtual void Unselect( sal_Int32 nRowOrCol, sal_Int32 nExt ) SAL_OVERRIDE;
    virtual ~SwAccAllTableSelHander_Impl();
};

SwAccAllTableSelHander_Impl::~SwAccAllTableSelHander_Impl()
{
}

uno::Sequence < sal_Int32 > SwAccAllTableSelHander_Impl::GetSelSequence()
{
    OSL_ENSURE( nCount >= 0, "underflow" );
    uno::Sequence < sal_Int32 > aRet( nCount );
    sal_Int32 *pRet = aRet.getArray();
    sal_Int32 nPos = 0;
    size_t nSize = aSelected.size();
    for( size_t i=0; i < nSize && nPos < nCount; i++ )
    {
        if( aSelected[i] )
        {
            *pRet++ = i;
            nPos++;
        }
    }

    OSL_ENSURE( nPos == nCount, "count is wrong" );

    return aRet;
}

void SwAccAllTableSelHander_Impl::Unselect( sal_Int32 nRowOrCol,
                                            sal_Int32 nExt )
{
    OSL_ENSURE( static_cast< size_t >( nRowOrCol ) < aSelected.size(),
             "index to large" );
    OSL_ENSURE( static_cast< size_t >( nRowOrCol+nExt ) <= aSelected.size(),
             "extent to large" );
    while( nExt )
    {
        if( aSelected[static_cast< size_t >( nRowOrCol )] )
        {
            aSelected[static_cast< size_t >( nRowOrCol )] = false;
            nCount--;
        }
        nExt--;
        nRowOrCol++;
    }
}

const SwSelBoxes *SwAccessibleTable::GetSelBoxes() const
{
    const SwSelBoxes *pSelBoxes = 0;
    const SwCrsrShell *pCSh = GetCrsrShell();
    if( (pCSh != NULL) && pCSh->IsTableMode() )
    {
        pSelBoxes = &pCSh->GetTableCrsr()->GetSelectedBoxes();
    }

    return pSelBoxes;
}

void SwAccessibleTable::FireTableChangeEvent(
        const SwAccessibleTableData_Impl& rTableData )
{
    AccessibleTableModelChange aModelChange;
    aModelChange.Type = AccessibleTableModelChangeType::UPDATE;
    aModelChange.FirstRow = 0;
    aModelChange.LastRow = rTableData.GetRowCount() - 1;
    aModelChange.FirstColumn = 0;
    aModelChange.LastColumn = rTableData.GetColumnCount() - 1;

    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::TABLE_MODEL_CHANGED;
    aEvent.NewValue <<= aModelChange;

    FireAccessibleEvent( aEvent );
}

const SwTableBox* SwAccessibleTable::GetTableBox( sal_Int32 nChildIndex ) const
{
    OSL_ENSURE( nChildIndex >= 0, "Illegal child index." );
    OSL_ENSURE( nChildIndex < const_cast<SwAccessibleTable*>(this)->getAccessibleChildCount(), "Illegal child index." ); // #i77106#

    const SwTableBox* pBox = NULL;

    // get table box for 'our' table cell
    SwAccessibleChild aCell( GetChild( *(const_cast<SwAccessibleMap*>(GetMap())), nChildIndex ) );
    if( aCell.GetSwFrm()  )
    {
        const SwFrm* pChildFrm = aCell.GetSwFrm();
        if( (pChildFrm != NULL) && pChildFrm->IsCellFrm() )
        {
            const SwCellFrm* pCellFrm =
                static_cast<const SwCellFrm*>( pChildFrm );
            pBox = pCellFrm->GetTabBox();
        }
    }

    OSL_ENSURE( pBox != NULL, "We need the table box." );
    return pBox;
}

bool SwAccessibleTable::IsChildSelected( sal_Int32 nChildIndex ) const
{
    bool bRet = false;
    const SwSelBoxes* pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        const SwTableBox* pBox = GetTableBox( nChildIndex );
        OSL_ENSURE( pBox != NULL, "We need the table box." );
        bRet = pSelBoxes->find( const_cast<SwTableBox*>( pBox ) ) != pSelBoxes->end();
    }

    return bRet;
}

sal_Int32 SwAccessibleTable::GetIndexOfSelectedChild(
                sal_Int32 nSelectedChildIndex ) const
{
    // iterate over all children to n-th isAccessibleChildSelected()
    sal_Int32 nChildren = const_cast<SwAccessibleTable*>(this)->getAccessibleChildCount(); // #i77106#
    if( nSelectedChildIndex >= nChildren )
        return -1L;

    sal_Int32 n = 0;
    while( n < nChildren )
    {
        if( IsChildSelected( n ) )
        {
            if( 0 == nSelectedChildIndex )
                break;
            else
                --nSelectedChildIndex;
        }
        ++n;
    }

    return n < nChildren ? n : -1L;
}

void SwAccessibleTable::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );
    //Add resizable state to table
    rStateSet.AddState( AccessibleStateType::RESIZABLE );
    // MULTISELECTABLE
    rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell  )
        rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
}

SwAccessibleTable::SwAccessibleTable(
        SwAccessibleMap* pInitMap,
        const SwTabFrm* pTabFrm  ) :
    SwAccessibleContext( pInitMap, AccessibleRole::TABLE, pTabFrm ),
    mpTableData( 0 )
{
    SolarMutexGuard aGuard;

    const SwFrameFormat *pFrameFormat = pTabFrm->GetFormat();
    const_cast< SwFrameFormat * >( pFrameFormat )->Add( this );

    SetName( pFrameFormat->GetName() + "-" + OUString::number( pTabFrm->GetPhyPageNum() ) );

    const OUString sArg1( static_cast< const SwTabFrm * >( GetFrm() )->GetFormat()->GetName() );
    const OUString sArg2( GetFormattedPageNumber() );

    sDesc = GetResource( STR_ACCESS_TABLE_DESC, &sArg1, &sArg2 );
    UpdateTableData();
}

SwAccessibleTable::~SwAccessibleTable()
{
    SolarMutexGuard aGuard;

    delete mpTableData;
}

void SwAccessibleTable::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    const SwTabFrm *pTabFrm = static_cast< const SwTabFrm * >( GetFrm() );
    switch( nWhich )
    {
    case RES_NAME_CHANGED:
        if( pTabFrm )
        {
            const SwFrameFormat *pFrameFormat = pTabFrm->GetFormat();
            OSL_ENSURE( pFrameFormat == GetRegisteredIn(), "invalid frame" );

            const OUString sOldName( GetName() );
            const OUString sNewTabName = pFrameFormat->GetName();

            SetName( sNewTabName + "-" + OUString::number( pTabFrm->GetPhyPageNum() ) );

            if( sOldName != GetName() )
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::NAME_CHANGED;
                aEvent.OldValue <<= sOldName;
                aEvent.NewValue <<= GetName();
                FireAccessibleEvent( aEvent );
            }

            const OUString sOldDesc( sDesc );
            const OUString sArg2( GetFormattedPageNumber() );

            sDesc = GetResource( STR_ACCESS_TABLE_DESC, &sNewTabName, &sArg2 );
            if( sDesc != sOldDesc )
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::DESCRIPTION_CHANGED;
                aEvent.OldValue <<= sOldDesc;
                aEvent.NewValue <<= sDesc;
                FireAccessibleEvent( aEvent );
            }
        }
        break;

    case RES_OBJECTDYING:
        // mba: it seems that this class intentionally does not call code in base class SwClient
        if( pOld && ( GetRegisteredIn() == static_cast< SwModify *>( static_cast< const SwPtrMsgPoolItem * >( pOld )->pObject ) ) )
            GetRegisteredInNonConst()->Remove( this );
        break;

    default:
        // mba: former call to base class method removed as it is meant to handle only RES_OBJECTDYING
        break;
    }
}

uno::Any SwAccessibleTable::queryInterface( const uno::Type& rType )
    throw (uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if ( rType == cppu::UnoType<XAccessibleTable>::get() )
    {
        uno::Reference<XAccessibleTable> xThis( this );
           aRet <<= xThis;
    }
    else if ( rType == cppu::UnoType<XAccessibleSelection>::get() )
    {
        uno::Reference<XAccessibleSelection> xSelection( this );
        aRet <<= xSelection;
    }
    else if ( rType == cppu::UnoType<XAccessibleTableSelection>::get() )
    {
        uno::Reference<XAccessibleTableSelection> xTableExtent( this );
        aRet <<= xTableExtent;
    }
    else
    {
        aRet = SwAccessibleContext::queryInterface(rType);
    }

    return aRet;
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SwAccessibleTable::getTypes()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 2 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex++] = cppu::UnoType<XAccessibleSelection>::get();
    pTypes[nIndex++] = cppu::UnoType<XAccessibleTable>::get();

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleTable::getImplementationId()
        throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// #i77106#
SwAccessibleTableData_Impl* SwAccessibleTable::CreateNewTableData()
{
    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( GetFrm() );
    return new SwAccessibleTableData_Impl( *GetMap(), pTabFrm, IsInPagePreview() );
}

void SwAccessibleTable::UpdateTableData()
{
    // #i77106# - usage of new method <CreateNewTableData()>
    delete mpTableData;
    mpTableData = CreateNewTableData();
}

void SwAccessibleTable::ClearTableData()
{
    delete mpTableData;
    mpTableData = 0;
}

OUString SAL_CALL SwAccessibleTable::getAccessibleDescription()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return sDesc;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleRowCount()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    return  GetTableData().GetRowCount();
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleColumnCount(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    return GetTableData().GetColumnCount();
}

OUString SAL_CALL SwAccessibleTable::getAccessibleRowDescription(
            sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    // #i87532# - determine table cell in <nRow>th row and
    // in first column of row header table and return its text content.
    OUString sRowDesc;

    GetTableData().CheckRowAndCol(nRow, 0, this);

    uno::Reference< XAccessibleTable > xTableRowHeader = getAccessibleRowHeaders();
    if ( xTableRowHeader.is() )
    {
        uno::Reference< XAccessible > xRowHeaderCell =
                        xTableRowHeader->getAccessibleCellAt( nRow, 0 );
        OSL_ENSURE( xRowHeaderCell.is(),
                "<SwAccessibleTable::getAccessibleRowDescription(..)> - missing row header cell -> serious issue." );
        uno::Reference< XAccessibleContext > xRowHeaderCellContext =
                                    xRowHeaderCell->getAccessibleContext();
        const sal_Int32 nCellChildCount( xRowHeaderCellContext->getAccessibleChildCount() );
        for ( sal_Int32 nChildIndex = 0; nChildIndex < nCellChildCount; ++nChildIndex )
        {
            uno::Reference< XAccessible > xChild = xRowHeaderCellContext->getAccessibleChild( nChildIndex );
            uno::Reference< XAccessibleText > xChildText( xChild, uno::UNO_QUERY );
            if ( xChildText.is() )
            {
                sRowDesc = sRowDesc + xChildText->getText();
            }
        }
    }

    return sRowDesc;
}

OUString SAL_CALL SwAccessibleTable::getAccessibleColumnDescription(
            sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    // #i87532# - determine table cell in first row and
    // in <nColumn>th column of column header table and return its text content.
    OUString sColumnDesc;

    GetTableData().CheckRowAndCol(0, nColumn, this);

    uno::Reference< XAccessibleTable > xTableColumnHeader = getAccessibleColumnHeaders();
    if ( xTableColumnHeader.is() )
    {
        uno::Reference< XAccessible > xColumnHeaderCell =
                        xTableColumnHeader->getAccessibleCellAt( 0, nColumn );
        OSL_ENSURE( xColumnHeaderCell.is(),
                "<SwAccessibleTable::getAccessibleColumnDescription(..)> - missing column header cell -> serious issue." );
        uno::Reference< XAccessibleContext > xColumnHeaderCellContext =
                                    xColumnHeaderCell->getAccessibleContext();
        const sal_Int32 nCellChildCount( xColumnHeaderCellContext->getAccessibleChildCount() );
        for ( sal_Int32 nChildIndex = 0; nChildIndex < nCellChildCount; ++nChildIndex )
        {
            uno::Reference< XAccessible > xChild = xColumnHeaderCellContext->getAccessibleChild( nChildIndex );
            uno::Reference< XAccessibleText > xChildText( xChild, uno::UNO_QUERY );
            if ( xChildText.is() )
            {
                sColumnDesc = sColumnDesc + xChildText->getText();
            }
        }
    }

    return sColumnDesc;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleRowExtentAt(
            sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    sal_Int32 nExtend = -1;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    UpdateTableData();
    GetTableData().CheckRowAndCol( nRow, nColumn, this );

    Int32Set_Impl::const_iterator aSttCol(
                                    GetTableData().GetColumnIter( nColumn ) );
    Int32Set_Impl::const_iterator aSttRow(
                                    GetTableData().GetRowIter( nRow ) );
    const SwFrm *pCellFrm = GetTableData().GetCellAtPos( *aSttCol, *aSttRow,
                                                         false );
    if( pCellFrm )
    {
        sal_Int32 nBottom = pCellFrm->Frm().Bottom();
        nBottom -= GetFrm()->Frm().Top();
        Int32Set_Impl::const_iterator aEndRow(
                GetTableData().GetRows().upper_bound( nBottom ) );
        nExtend =
             static_cast< sal_Int32 >( ::std::distance( aSttRow, aEndRow ) );
    }

    return nExtend;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleColumnExtentAt(
               sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    sal_Int32 nExtend = -1;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )
    UpdateTableData();

    GetTableData().CheckRowAndCol( nRow, nColumn, this );

    Int32Set_Impl::const_iterator aSttCol(
                                    GetTableData().GetColumnIter( nColumn ) );
    Int32Set_Impl::const_iterator aSttRow(
                                    GetTableData().GetRowIter( nRow ) );
    const SwFrm *pCellFrm = GetTableData().GetCellAtPos( *aSttCol, *aSttRow,
                                                         false );
    if( pCellFrm )
    {
        sal_Int32 nRight = pCellFrm->Frm().Right();
        nRight -= GetFrm()->Frm().Left();
        Int32Set_Impl::const_iterator aEndCol(
                GetTableData().GetColumns().upper_bound( nRight ) );
        nExtend =
             static_cast< sal_Int32 >( ::std::distance( aSttCol, aEndCol ) );
    }

    return nExtend;
}

uno::Reference< XAccessibleTable > SAL_CALL
        SwAccessibleTable::getAccessibleRowHeaders(  )
    throw (uno::RuntimeException, std::exception)
{
    // Row headers aren't supported
    return uno::Reference< XAccessibleTable >();
}

uno::Reference< XAccessibleTable > SAL_CALL
        SwAccessibleTable::getAccessibleColumnHeaders(  )
    throw (uno::RuntimeException, std::exception)
{
    // #i87532# - assure that return accessible object is empty,
    // if no column header exists.
    SwAccessibleTableColHeaders* pTableColHeaders =
        new SwAccessibleTableColHeaders( GetMap(), static_cast< const SwTabFrm *>( GetFrm() ) );
    uno::Reference< XAccessibleTable > xTableColumnHeaders( pTableColHeaders );
    if ( pTableColHeaders->getAccessibleChildCount() <= 0 )
    {
        return uno::Reference< XAccessibleTable >();
    }

    return xTableColumnHeaders;
}

uno::Sequence< sal_Int32 > SAL_CALL SwAccessibleTable::getSelectedAccessibleRows()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    const SwSelBoxes *pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        sal_Int32 nRows = GetTableData().GetRowCount();
        SwAccAllTableSelHander_Impl aSelRows( nRows  );

        GetTableData().GetSelection( 0, nRows, *pSelBoxes, aSelRows,
                                      false );

        return aSelRows.GetSelSequence();
    }
    else
    {
        return uno::Sequence< sal_Int32 >( 0 );
    }
}

uno::Sequence< sal_Int32 > SAL_CALL SwAccessibleTable::getSelectedAccessibleColumns()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    const SwSelBoxes *pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        sal_Int32 nCols = GetTableData().GetColumnCount();
        SwAccAllTableSelHander_Impl aSelCols( nCols );

        GetTableData().GetSelection( 0, nCols, *pSelBoxes, aSelCols, true );

        return aSelCols.GetSelSequence();
    }
    else
    {
        return uno::Sequence< sal_Int32 >( 0 );
    }
}

sal_Bool SAL_CALL SwAccessibleTable::isAccessibleRowSelected( sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    GetTableData().CheckRowAndCol( nRow, 0, this );

    bool bRet;
    const SwSelBoxes *pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        SwAccSingleTableSelHander_Impl aSelRow;
        GetTableData().GetSelection( nRow, nRow+1, *pSelBoxes, aSelRow,
                                     false );
        bRet = aSelRow.IsSelected();
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

sal_Bool SAL_CALL SwAccessibleTable::isAccessibleColumnSelected(
        sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    GetTableData().CheckRowAndCol( 0, nColumn, this );

    bool bRet;
    const SwSelBoxes *pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        SwAccSingleTableSelHander_Impl aSelCol;

        GetTableData().GetSelection( nColumn, nColumn+1, *pSelBoxes, aSelCol,
                                     true );
        bRet = aSelCol.IsSelected();
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleTable::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    uno::Reference< XAccessible > xRet;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    const SwFrm *pCellFrm =
                    GetTableData().GetCell( nRow, nColumn, false, this );
    if( pCellFrm )
        xRet = GetMap()->GetContext( pCellFrm );

    return xRet;
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleTable::getAccessibleCaption()
    throw (uno::RuntimeException, std::exception)
{
    // captions aren't supported
    return uno::Reference< XAccessible >();
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleTable::getAccessibleSummary()
    throw (uno::RuntimeException, std::exception)
{
    // summaries aren't supported
    return uno::Reference< XAccessible >();
}

sal_Bool SAL_CALL SwAccessibleTable::isAccessibleSelected(
            sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    bool bRet = false;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    const SwFrm *pFrm =
                    GetTableData().GetCell( nRow, nColumn, false, this );
    if( pFrm && pFrm->IsCellFrm() )
    {
        const SwSelBoxes *pSelBoxes = GetSelBoxes();
        if( pSelBoxes )
        {
            const SwCellFrm *pCFrm = static_cast < const SwCellFrm * >( pFrm );
            SwTableBox *pBox =
                const_cast< SwTableBox *>( pCFrm->GetTabBox() );
            bRet = pSelBoxes->find( pBox ) != pSelBoxes->end();
        }
    }

    return bRet;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleIndex(
            sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    sal_Int32 nRet = -1;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    SwAccessibleChild aCell( GetTableData().GetCell( nRow, nColumn, false, this ));
    if ( aCell.IsValid() )
    {
        nRet = GetChildIndex( *(GetMap()), aCell );
    }

    return nRet;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleRow( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    sal_Int32 nRet = -1;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    // #i77106#
    if ( ( nChildIndex < 0 ) ||
         ( nChildIndex >= getAccessibleChildCount() ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    SwAccessibleChild aCell( GetChild( *(GetMap()), nChildIndex ) );
    if ( aCell.GetSwFrm()  )
    {
        sal_Int32 nTop = aCell.GetSwFrm()->Frm().Top();
        nTop -= GetFrm()->Frm().Top();
        Int32Set_Impl::const_iterator aRow(
                GetTableData().GetRows().lower_bound( nTop ) );
        nRet = static_cast< sal_Int32 >( ::std::distance(
                    GetTableData().GetRows().begin(), aRow ) );
    }
    else
    {
        OSL_ENSURE( !aCell.IsValid(), "SwAccessibleTable::getAccessibleColumn:"
                "aCell not expected to be valid.");

        throw lang::IndexOutOfBoundsException();
    }

    return nRet;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleColumn(
        sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    sal_Int32 nRet = -1;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    // #i77106#
    if ( ( nChildIndex < 0 ) ||
         ( nChildIndex >= getAccessibleChildCount() ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    SwAccessibleChild aCell( GetChild( *(GetMap()), nChildIndex ) );
    if ( aCell.GetSwFrm()  )
    {
        sal_Int32 nLeft = aCell.GetSwFrm()->Frm().Left();
        nLeft -= GetFrm()->Frm().Left();
        Int32Set_Impl::const_iterator aCol(
                GetTableData().GetColumns().lower_bound( nLeft ) );
        nRet = static_cast< sal_Int32 >( ::std::distance(
                    GetTableData().GetColumns().begin(), aCol ) );
    }
    else
    {
        OSL_ENSURE( !aCell.IsValid(), "SwAccessibleTable::getAccessibleColumn:"
                "aCell not expected to be valid.");

        throw lang::IndexOutOfBoundsException();
    }

    return nRet;
}

OUString SAL_CALL SwAccessibleTable::getImplementationName()
        throw( uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.Writer.SwAccessibleTableView");
}

sal_Bool SAL_CALL SwAccessibleTable::supportsService(
        const OUString& sTestServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sTestServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleTable::getSupportedServiceNames()
        throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.table.AccessibleTableView";
    pArray[1] = sAccessibleServiceName;
    return aRet;
}

void SwAccessibleTable::InvalidatePosOrSize( const SwRect& rOldBox )
{
    SolarMutexGuard aGuard;

    //need to update children
    SwAccessibleTableData_Impl *pNewTableData = CreateNewTableData();
    if( !pNewTableData->CompareExtents( GetTableData() ) )
    {
        delete mpTableData;
        mpTableData = pNewTableData;
        FireTableChangeEvent(*mpTableData);
    }
    if( HasTableData() )
        GetTableData().SetTablePos( GetFrm()->Frm().Pos() );

    SwAccessibleContext::InvalidatePosOrSize( rOldBox );
}

void SwAccessibleTable::Dispose( bool bRecursive )
{
    SolarMutexGuard aGuard;

    if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

    SwAccessibleContext::Dispose( bRecursive );
}

void SwAccessibleTable::DisposeChild( const SwAccessibleChild& rChildFrmOrObj,
                                      bool bRecursive )
{
    SolarMutexGuard aGuard;

    const SwFrm *pFrm = rChildFrmOrObj.GetSwFrm();
    OSL_ENSURE( pFrm, "frame expected" );
    if( HasTableData() )
    {
        FireTableChangeEvent( GetTableData() );
        ClearTableData();
    }

    // There are two reason why this method has been called. The first one
    // is there is no context for pFrm. The method is then called by
    // the map, and we have to call our superclass.
    // The other situation is that we have been call by a call to get notified
    // about its change. We then must not call the superclass
    uno::Reference< XAccessible > xAcc( GetMap()->GetContext( pFrm, false ) );
    if( !xAcc.is() )
        SwAccessibleContext::DisposeChild( rChildFrmOrObj, bRecursive );
}

void SwAccessibleTable::InvalidateChildPosOrSize( const SwAccessibleChild& rChildFrmOrObj,
                                                  const SwRect& rOldBox )
{
    SolarMutexGuard aGuard;

    if( HasTableData() )
    {
        SAL_WARN_IF( HasTableData() &&
                GetFrm()->Frm().Pos() != GetTableData().GetTablePos(), "sw.core",
                "table has invalid position" );
        if( HasTableData() )
        {
            SwAccessibleTableData_Impl *pNewTableData = CreateNewTableData(); // #i77106#
            if( !pNewTableData->CompareExtents( GetTableData() ) )
            {
                if (pNewTableData->GetRowCount() != mpTableData->GetRowCount()
                    && 1 < GetTableData().GetRowCount())
                {
                    Int32Set_Impl::const_iterator aSttCol( GetTableData().GetColumnIter( 0 ) );
                    Int32Set_Impl::const_iterator aSttRow( GetTableData().GetRowIter( 1 ) );
                    const SwFrm *pCellFrm = GetTableData().GetCellAtPos( *aSttCol, *aSttRow, false );
                    Int32Set_Impl::const_iterator aSttCol2( pNewTableData->GetColumnIter( 0 ) );
                    Int32Set_Impl::const_iterator aSttRow2( pNewTableData->GetRowIter( 0 ) );
                    const SwFrm *pCellFrm2 = pNewTableData->GetCellAtPos( *aSttCol2, *aSttRow2, false );

                    if(pCellFrm == pCellFrm2)
                    {
                        AccessibleTableModelChange aModelChange;
                        aModelChange.Type = AccessibleTableModelChangeType::UPDATE;
                        aModelChange.FirstRow = 0;
                        aModelChange.LastRow = mpTableData->GetRowCount() - 1;
                        aModelChange.FirstColumn = 0;
                        aModelChange.LastColumn = mpTableData->GetColumnCount() - 1;

                        AccessibleEventObject aEvent;
                        aEvent.EventId = AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED;
                        aEvent.NewValue <<= aModelChange;

                        FireAccessibleEvent( aEvent );
                    }
                }
                else
                    FireTableChangeEvent( GetTableData() );
                ClearTableData();
                mpTableData = pNewTableData;
            }
            else
            {
                delete pNewTableData;
            }
        }
    }

    // #i013961# - always call super class method
    SwAccessibleContext::InvalidateChildPosOrSize( rChildFrmOrObj, rOldBox );
}

// XAccessibleSelection

void SAL_CALL SwAccessibleTable::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleTable );

    if( (nChildIndex < 0) || (nChildIndex >= getAccessibleChildCount()) ) // #i77106#
        throw lang::IndexOutOfBoundsException();

    // preliminaries: get 'our' table box, and get the cursor shell
    const SwTableBox* pBox = GetTableBox( nChildIndex );
    OSL_ENSURE( pBox != NULL, "We need the table box." );

    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell == NULL )
        return;

    // assure, that child, identified by the given index, isn't already selected.
    if ( IsChildSelected( nChildIndex ) )
    {
        return;
    }

    // now we can start to do the work: check whether we already have
    // a table selection (in 'our' table). If so, extend the
    // selection, else select the current cell.

    // if we have a selection in a table, check if it's in the
    // same table that we're trying to select in
    const SwTableNode* pSelectedTable = pCrsrShell->IsCrsrInTable();
    if( pSelectedTable != NULL )
    {
        // get top-most table line
        const SwTableLine* pUpper = pBox->GetUpper();
        while( pUpper->GetUpper() != NULL )
            pUpper = pUpper->GetUpper()->GetUpper();
        sal_uInt16 nPos =
            pSelectedTable->GetTable().GetTabLines().GetPos( pUpper );
        if( nPos == USHRT_MAX )
            pSelectedTable = NULL;
    }

    // create the new selection
    const SwStartNode* pStartNode = pBox->GetSttNd();
    if( pSelectedTable == NULL || !pCrsrShell->GetTableCrs() )
    {
        pCrsrShell->StartAction();
        // Set cursor into current cell. This deletes any table cursor.
        SwPaM aPaM( *pStartNode );
        aPaM.Move( fnMoveForward, fnGoNode );
        Select( aPaM );
        // Move cursor to the end of the table creating a selection and a table
        // cursor.
        pCrsrShell->SetMark();
        pCrsrShell->MoveTable( fnTableCurr, fnTableEnd );
        // now set the cursor into the cell again.
        SwPaM *pPaM = pCrsrShell->GetTableCrs() ? pCrsrShell->GetTableCrs()
                                                    : pCrsrShell->GetCrsr();
        *pPaM->GetPoint() = *pPaM->GetMark();
        pCrsrShell->EndAction();
        // we now have one cell selected!
    }
    else
    {
        // if the cursor is already in this table,
        // expand the current selection (i.e., set
        // point to new position; keep mark)
        SwPaM aPaM( *pStartNode );
        aPaM.Move( fnMoveForward, fnGoNode );
        aPaM.SetMark();
        const SwPaM *pPaM = pCrsrShell->GetTableCrs() ? pCrsrShell->GetTableCrs()
                                                    : pCrsrShell->GetCrsr();
        *(aPaM.GetMark()) = *pPaM->GetMark();
        Select( aPaM );

    }
}

sal_Bool SAL_CALL SwAccessibleTable::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleTable );

    if( (nChildIndex < 0) || (nChildIndex >= getAccessibleChildCount()) ) // #i77106#
        throw lang::IndexOutOfBoundsException();

    return IsChildSelected( nChildIndex );
}

void SAL_CALL SwAccessibleTable::clearAccessibleSelection(  )
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable );

    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell != NULL )
    {
        pCrsrShell->StartAction();
        pCrsrShell->ClearMark();
        pCrsrShell->EndAction();
    }
}

void SAL_CALL SwAccessibleTable::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException, std::exception )
{
    // first clear selection, then select first and last child
    clearAccessibleSelection();
    selectAccessibleChild( 0 );
    selectAccessibleChild( getAccessibleChildCount()-1 ); // #i77106#
}

sal_Int32 SAL_CALL SwAccessibleTable::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleTable );

    // iterate over all children and count isAccessibleChildSelected()
    sal_Int32 nCount = 0;

    sal_Int32 nChildren = getAccessibleChildCount(); // #i71106#
    for( sal_Int32 n = 0; n < nChildren; n++ )
        if( IsChildSelected( n ) )
            nCount++;

    return nCount;
}

uno::Reference<XAccessible> SAL_CALL SwAccessibleTable::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleTable );

    // parameter checking (part 1): index lower 0
    if( nSelectedChildIndex < 0 )
        throw lang::IndexOutOfBoundsException();

    sal_Int32 nChildIndex = GetIndexOfSelectedChild( nSelectedChildIndex );

    // parameter checking (part 2): index higher than selected children?
    if( nChildIndex < 0 )
        throw lang::IndexOutOfBoundsException();

    // #i77106#
    if ( nChildIndex >= getAccessibleChildCount() )
    {
        throw lang::IndexOutOfBoundsException();
    }

    return getAccessibleChild( nChildIndex );
}

// index has to be treated as global child index.
void SAL_CALL SwAccessibleTable::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleTable );

    SwCrsrShell* pCrsrShell = GetCrsrShell();

    // index has to be treated as global child index
    if ( !pCrsrShell )
        throw lang::IndexOutOfBoundsException();

    // assure, that given child index is in bounds.
    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() ) // #i77106#
        throw lang::IndexOutOfBoundsException();

    // assure, that child, identified by the given index, is selected.
    if ( !IsChildSelected( nChildIndex ) )
        return;

    const SwTableBox* pBox = GetTableBox( nChildIndex );
    OSL_ENSURE( pBox != NULL, "We need the table box." );

    // If we unselect point, then set cursor to mark. If we clear another
    // selected box, then set cursor to point.
    // reduce selection to mark.
    SwPaM *pPaM = pCrsrShell->GetTableCrs() ? pCrsrShell->GetTableCrs()
                                                : pCrsrShell->GetCrsr();
    bool bDeselectPoint =
        pBox->GetSttNd() ==
            pPaM->GetPoint()->nNode.GetNode().FindTableBoxStartNode();

    SwPaM aPaM( bDeselectPoint ? *pPaM->GetMark() : *pPaM->GetPoint() );

    pCrsrShell->StartAction();

    // Set cursor into either point or mark
    Select( aPaM );
    // Move cursor to the end of the table creating a selection and a table
    // cursor.
    pCrsrShell->SetMark();
    pCrsrShell->MoveTable( fnTableCurr, fnTableEnd );
    // now set the cursor into the cell again.
    pPaM = pCrsrShell->GetTableCrs() ? pCrsrShell->GetTableCrs()
                                        : pCrsrShell->GetCrsr();
    *pPaM->GetPoint() = *pPaM->GetMark();
    pCrsrShell->EndAction();
}

sal_Int32 SAL_CALL SwAccessibleTable::getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    const SvxBrushItem &rBack = GetFrm()->GetAttrSet()->GetBackground();
    sal_uInt32 crBack = rBack.GetColor().GetColor();

    if (COL_AUTO == crBack)
    {
        uno::Reference<XAccessible> xAccDoc = getAccessibleParent();
        if (xAccDoc.is())
        {
            uno::Reference<XAccessibleComponent> xCompoentDoc(xAccDoc,uno::UNO_QUERY);
            if (xCompoentDoc.is())
            {
                crBack = (sal_uInt32)xCompoentDoc->getBackground();
            }
        }
    }
    return crBack;
}

void SwAccessibleTable::FireSelectionEvent( )
{
    AccessibleEventObject aEvent;

    aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_REMOVE;

    for (Cells_t::iterator vi = m_vecCellRemove.begin();
            vi != m_vecCellRemove.end(); ++vi)
    {
        // fdo#57197: check if the object is still alive
        uno::Reference<XAccessible> const xAcc(vi->second);
        if (xAcc.is())
        {
            SwAccessibleContext *const pAccCell(vi->first);
            assert(pAccCell);
            pAccCell->FireAccessibleEvent(aEvent);
        }
    }

    if (m_vecCellAdd.size() <= SELECTION_WITH_NUM)
    {
        aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_ADD;
        for (Cells_t::iterator vi = m_vecCellAdd.begin();
                vi != m_vecCellAdd.end(); ++vi)
        {
            // fdo#57197: check if the object is still alive
            uno::Reference<XAccessible> const xAcc(vi->second);
            if (xAcc.is())
            {
                SwAccessibleContext *const pAccCell(vi->first);
                assert(pAccCell);
                pAccCell->FireAccessibleEvent(aEvent);
            }
        }
        return ;
    }
    else
    {
        aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_WITHIN;
        FireAccessibleEvent(aEvent);
    }
}

void SwAccessibleTable::AddSelectionCell(
        SwAccessibleContext *const pAccCell, bool const bAddOrRemove)
{
    uno::Reference<XAccessible> const xTmp(pAccCell);
    if (bAddOrRemove)
    {
        m_vecCellAdd.push_back(std::make_pair(pAccCell, xTmp));
    }
    else
    {
        m_vecCellRemove.push_back(std::make_pair(pAccCell, xTmp));
    }
}

// XAccessibleTableSelection
sal_Bool SAL_CALL SwAccessibleTable::selectRow( sal_Int32 row )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if( isAccessibleColumnSelected( row ) )
        return sal_True;

    long lColumnCount = getAccessibleColumnCount();
    for(long lCol = 0; lCol < lColumnCount; lCol ++)
    {
        long lChildIndex = getAccessibleIndex(row, lCol);
        selectAccessibleChild(lChildIndex);
    }

    return sal_True;
}
sal_Bool SAL_CALL SwAccessibleTable::selectColumn( sal_Int32 column )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if( isAccessibleColumnSelected( column ) )
        return sal_True;

    long lRowCount = getAccessibleRowCount();

    for(long lRow = 0; lRow < lRowCount; lRow ++)
    {
        long lChildIndex = getAccessibleIndex(lRow, column);
        selectAccessibleChild(lChildIndex);
    }
    return sal_True;
}

sal_Bool SAL_CALL SwAccessibleTable::unselectRow( sal_Int32 row )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard g;

    if( isAccessibleSelected( row , 0 ) &&  isAccessibleSelected( row , getAccessibleColumnCount()-1 ) )
    {
        SwCrsrShell* pCrsrShell = GetCrsrShell();
        if( pCrsrShell != NULL )
        {
            pCrsrShell->StartAction();
            pCrsrShell->ClearMark();
            pCrsrShell->EndAction();
            return sal_True;
        }
    }
    return sal_True;
}

sal_Bool SAL_CALL SwAccessibleTable::unselectColumn( sal_Int32 column )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard g;

    if( isAccessibleSelected( 0 , column ) &&  isAccessibleSelected( getAccessibleRowCount()-1,column))
    {
        SwCrsrShell* pCrsrShell = GetCrsrShell();
        if( pCrsrShell != NULL )
        {
            pCrsrShell->StartAction();
            pCrsrShell->ClearMark();
            pCrsrShell->EndAction();
            return sal_True;
        }
    }
    return sal_True;
}

// #i77106# - implementation of class <SwAccessibleTableColHeaders>
SwAccessibleTableColHeaders::SwAccessibleTableColHeaders( SwAccessibleMap *pMap2,
                                                          const SwTabFrm *pTabFrm )
    : SwAccessibleTable( pMap2, pTabFrm )
{
    SolarMutexGuard aGuard;

    const SwFrameFormat *pFrameFormat = pTabFrm->GetFormat();
    const_cast< SwFrameFormat * >( pFrameFormat )->Add( this );
    const OUString aName = pFrameFormat->GetName() + "-ColumnHeaders";

    SetName( aName + "-" + OUString::number( pTabFrm->GetPhyPageNum() ) );

    const OUString sArg2( GetFormattedPageNumber() );

    SetDesc( GetResource( STR_ACCESS_TABLE_DESC, &aName, &sArg2 ) );

    NotRegisteredAtAccessibleMap(); // #i85634#
}

SwAccessibleTableData_Impl* SwAccessibleTableColHeaders::CreateNewTableData()
{
    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( GetFrm() );
    return new SwAccessibleTableData_Impl( *(GetMap()), pTabFrm, IsInPagePreview(), true );
}

void SwAccessibleTableColHeaders::Modify( const SfxPoolItem * /*pOld*/, const SfxPoolItem * /*pNew*/ )
{
}

// XInterface
uno::Any SAL_CALL SwAccessibleTableColHeaders::queryInterface( const uno::Type& aType )
        throw (uno::RuntimeException, std::exception)
{
    return SwAccessibleTable::queryInterface( aType );
}

// XAccessibleContext
sal_Int32 SAL_CALL SwAccessibleTableColHeaders::getAccessibleChildCount()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    sal_Int32 nCount = 0;

    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( GetFrm() );
    const SwAccessibleChildSList aVisList( GetVisArea(), *pTabFrm, *(GetMap()) );
    SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        if( rLower.IsAccessible( IsInPagePreview() ) )
        {
            nCount++;
        }
        else if( rLower.GetSwFrm() )
        {
            // There are no unaccessible SdrObjects that count
            if ( !rLower.GetSwFrm()->IsRowFrm() ||
                 pTabFrm->IsInHeadline( *(rLower.GetSwFrm()) ) )
            {
                nCount += SwAccessibleFrame::GetChildCount( *(GetMap()),
                                                            GetVisArea(),
                                                            rLower.GetSwFrm(),
                                                            IsInPagePreview() );
            }
        }
        ++aIter;
    }

    return nCount;
}

uno::Reference< XAccessible> SAL_CALL
        SwAccessibleTableColHeaders::getAccessibleChild (sal_Int32 nIndex)
        throw (uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception)
{
    if ( nIndex < 0 || nIndex >= getAccessibleChildCount() )
    {
        throw lang::IndexOutOfBoundsException();
    }

    return SwAccessibleTable::getAccessibleChild( nIndex );
}

// XAccessibleTable
uno::Reference< XAccessibleTable >
        SAL_CALL SwAccessibleTableColHeaders::getAccessibleRowHeaders()
        throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< XAccessibleTable >();
}

uno::Reference< XAccessibleTable >
        SAL_CALL SwAccessibleTableColHeaders::getAccessibleColumnHeaders()
        throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< XAccessibleTable >();
}

// XServiceInfo

OUString SAL_CALL SwAccessibleTableColHeaders::getImplementationName()
        throw (uno::RuntimeException, std::exception)
{
    static const sal_Char sImplName[] = "com.sun.star.comp.Writer.SwAccessibleTableColumnHeadersView";
    return OUString(sImplName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
