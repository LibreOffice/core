/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::sw::access;

const sal_Char sServiceName[] = "com.sun.star.table.AccessibleTableView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleTableView";

typedef ::std::less < sal_Int32 > Int32Less_Impl;
typedef ::std::set < sal_Int32, Int32Less_Impl > Int32Set_Impl;

typedef ::std::pair < sal_Int32, sal_Int32 > Int32Pair_Impl;
typedef ::std::list < Int32Pair_Impl > Int32PairList_Impl;

class SwAccTableSelHander_Impl
{
public:
    virtual void Unselect( sal_Int32 nRowOrCol, sal_Int32 nExt ) = 0;

protected:
    ~SwAccTableSelHander_Impl() {}
};


//------------------------------------------------------------------------------

class SwAccessibleTableData_Impl
{
    SwAccessibleMap& mrAccMap;
    Int32Set_Impl   maRows;
    Int32Set_Impl   maColumns;
    Int32PairList_Impl maExtents;   // cell extends for event processing only
    Point   maTabFrmPos;
    const SwTabFrm *mpTabFrm;
    sal_Bool mbIsInPagePreview;
    bool mbOnlyTableColumnHeader;

    void CollectData( const SwFrm *pFrm );
    void CollectExtents( const SwFrm *pFrm );

    sal_Bool FindCell( const Point& rPos, const SwFrm *pFrm ,
                           sal_Bool bExact, const SwFrm *& rFrm ) const;

    void GetSelection( const Point& rTabPos, const SwRect& rArea,
                       const SwSelBoxes& rSelBoxes, const SwFrm *pFrm,
                       SwAccTableSelHander_Impl& rSelHdl,
                       sal_Bool bColumns ) const;

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
                                sal_Bool bIsInPagePreview,
                                bool bOnlyTableColumnHeader = false );

    const Int32Set_Impl& GetRows() const { return maRows; }
    const Int32Set_Impl& GetColumns() const { return maColumns; }

    inline Int32Set_Impl::const_iterator GetRowIter( sal_Int32 nRow ) const;
    inline Int32Set_Impl::const_iterator GetColumnIter( sal_Int32 nCol ) const;

    const SwFrm *GetCell( sal_Int32 nRow, sal_Int32 nColumn, sal_Bool bExact,
                          SwAccessibleTable *pThis ) const
        throw(lang::IndexOutOfBoundsException );
    const SwFrm *GetCellAtPos( sal_Int32 nLeft, sal_Int32 nTop,
                                         sal_Bool bExact ) const;
    inline sal_Int32 GetRowCount() const;
    inline sal_Int32 GetColumnCount() const;
    sal_Bool CompareExtents( const SwAccessibleTableData_Impl& r ) const;

    void GetSelection( sal_Int32 nStart, sal_Int32 nEnd,
                       const SwSelBoxes& rSelBoxes,
                          SwAccTableSelHander_Impl& rSelHdl,
                       sal_Bool bColumns ) const;

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
                    maRows.insert( pLower->Frm().Top() - maTabFrmPos.Y() );
                    CollectData( pLower );
                }
            }
            else if( pLower->IsCellFrm() &&
                     rLower.IsAccessible( mbIsInPagePreview ) )
            {
                maColumns.insert( pLower->Frm().Left() - maTabFrmPos.X() );
            }
            else
            {
                CollectData( pLower );
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

sal_Bool SwAccessibleTableData_Impl::FindCell(
        const Point& rPos, const SwFrm *pFrm, sal_Bool bExact,
        const SwFrm *& rRet ) const
{
    sal_Bool bFound = sal_False;

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
                    bFound = sal_True;
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
            sal_Bool bColumns ) const
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
        sal_Int32 nRow, sal_Int32 nColumn, sal_Bool,
        SwAccessibleTable *pThis ) const
    throw(lang::IndexOutOfBoundsException )
{
    CheckRowAndCol( nRow, nColumn, pThis );

    Int32Set_Impl::const_iterator aSttCol( GetColumnIter( nColumn ) );
    Int32Set_Impl::const_iterator aSttRow( GetRowIter( nRow ) );
    const SwFrm *pCellFrm = GetCellAtPos( *aSttCol, *aSttRow, sal_False );

    return pCellFrm;
}

void SwAccessibleTableData_Impl::GetSelection(
                       sal_Int32 nStart, sal_Int32 nEnd,
                       const SwSelBoxes& rSelBoxes,
                          SwAccTableSelHander_Impl& rSelHdl,
                       sal_Bool bColumns ) const
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
            aArea.Left( *aStt + aPos.X() );
        else
            aArea.Top( *aStt + aPos.Y() );
    }
    if( nEnd < static_cast< sal_Int32 >( rRowsOrColumns.size() ) )
    {
        Int32Set_Impl::const_iterator aEnd( rRowsOrColumns.begin() );
        ::std::advance( aEnd,
            static_cast< Int32Set_Impl::difference_type >( nEnd ) );
        if( bColumns )
            aArea.Right( *aEnd + aPos.X() - 1 );
        else
            aArea.Bottom( *aEnd + aPos.Y() - 1 );
    }

    GetSelection( aPos, aArea, rSelBoxes, mpTabFrm, rSelHdl, bColumns );
}

const SwFrm *SwAccessibleTableData_Impl::GetCellAtPos(
        sal_Int32 nLeft, sal_Int32 nTop, sal_Bool bExact ) const
{
    Point aPos( mpTabFrm->Frm().Pos() );
    aPos.Move( nLeft, nTop );
    const SwFrm *pRet = 0;
    FindCell( aPos, mpTabFrm, bExact, pRet );

    return pRet;
}

inline sal_Int32 SwAccessibleTableData_Impl::GetRowCount() const
{
    return static_cast< sal_Int32 >( maRows.size() );
}

inline sal_Int32 SwAccessibleTableData_Impl::GetColumnCount() const
{
    return static_cast< sal_Int32 >( maColumns.size() );
}

sal_Bool SwAccessibleTableData_Impl::CompareExtents(
                                const SwAccessibleTableData_Impl& rCmp ) const
{
    if( maExtents.size() != rCmp.maExtents.size() )
        return sal_False;

    return ::std::equal(maExtents.begin(), maExtents.end(), rCmp.maExtents.begin()) ? sal_True : sal_False;
}

SwAccessibleTableData_Impl::SwAccessibleTableData_Impl( SwAccessibleMap& rAccMap,
                                                        const SwTabFrm *pTabFrm,
                                                        sal_Bool bIsInPagePreview,
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
               OUString( RTL_CONSTASCII_USTRINGPARAM(
                       "row or column index out of range") ),
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

//------------------------------------------------------------------------------

class SwAccSingleTableSelHander_Impl : public SwAccTableSelHander_Impl
{
    sal_Bool bSelected;

public:

    inline SwAccSingleTableSelHander_Impl();

    virtual ~SwAccSingleTableSelHander_Impl() {}

    inline sal_Bool IsSelected() const { return bSelected; }

    virtual void Unselect( sal_Int32, sal_Int32 );
};

inline SwAccSingleTableSelHander_Impl::SwAccSingleTableSelHander_Impl() :
    bSelected( sal_True )
{
}

void SwAccSingleTableSelHander_Impl::Unselect( sal_Int32, sal_Int32 )
{
    bSelected = sal_False;
}

//------------------------------------------------------------------------------

class SwAccAllTableSelHander_Impl : public SwAccTableSelHander_Impl

{
    ::std::vector< sal_Bool > aSelected;
    sal_Int32 nCount;

public:

    inline SwAccAllTableSelHander_Impl( sal_Int32 nSize );

    uno::Sequence < sal_Int32 > GetSelSequence();

    virtual void Unselect( sal_Int32 nRowOrCol, sal_Int32 nExt );
    virtual ~SwAccAllTableSelHander_Impl();
};

SwAccAllTableSelHander_Impl::~SwAccAllTableSelHander_Impl()
{
}

inline SwAccAllTableSelHander_Impl::SwAccAllTableSelHander_Impl( sal_Int32 nSize ) :
    aSelected( nSize, sal_True ),
    nCount( nSize )
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
            aSelected[static_cast< size_t >( nRowOrCol )] = sal_False;
            nCount--;
        }
        nExt--;
        nRowOrCol++;
    }
}

//------------------------------------------------------------------------------

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

sal_Bool SwAccessibleTable::IsChildSelected( sal_Int32 nChildIndex ) const
{
    sal_Bool bRet = sal_False;
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

    // MULTISELECTABLE
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

    const SwFrmFmt *pFrmFmt = pTabFrm->GetFmt();
    const_cast< SwFrmFmt * >( pFrmFmt )->Add( this );
    const String& rName = pFrmFmt->GetName();

    OUStringBuffer aBuffer( rName.Len() + 4 );
    aBuffer.append( OUString(rName) );
    aBuffer.append( static_cast<sal_Unicode>( '-' ) );
    aBuffer.append( static_cast<sal_Int32>( pTabFrm->GetPhyPageNum() ) );

    SetName( aBuffer.makeStringAndClear() );

    OUString sArg1( static_cast< const SwTabFrm * >( GetFrm() )
                                        ->GetFmt()->GetName() );
    OUString sArg2( GetFormattedPageNumber() );

    sDesc = GetResource( STR_ACCESS_TABLE_DESC, &sArg1, &sArg2 );
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
            const SwFrmFmt *pFrmFmt = pTabFrm->GetFmt();
            OSL_ENSURE( pFrmFmt == GetRegisteredIn(), "invalid frame" );

            OUString sOldName( GetName() );

            const String& rNewTabName = pFrmFmt->GetName();
            OUStringBuffer aBuffer( rNewTabName.Len() + 4 );
            aBuffer.append( OUString(rNewTabName) );
            aBuffer.append( static_cast<sal_Unicode>( '-' ) );
            aBuffer.append( static_cast<sal_Int32>( pTabFrm->GetPhyPageNum() ) );

            SetName( aBuffer.makeStringAndClear() );
            if( sOldName != GetName() )
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::NAME_CHANGED;
                aEvent.OldValue <<= sOldName;
                aEvent.NewValue <<= GetName();
                FireAccessibleEvent( aEvent );
            }

            OUString sOldDesc( sDesc );
            OUString sArg1( rNewTabName );
            OUString sArg2( GetFormattedPageNumber() );

            sDesc = GetResource( STR_ACCESS_TABLE_DESC, &sArg1, &sArg2 );
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
        if( GetRegisteredIn() ==
                static_cast< SwModify *>( static_cast< const SwPtrMsgPoolItem * >( pOld )->pObject ) )
            GetRegisteredInNonConst()->Remove( this );
        break;

    default:
        // mba: former call to base class method removed as it is meant to handle only RES_OBJECTDYING
        break;
    }
}

uno::Any SwAccessibleTable::queryInterface( const uno::Type& rType )
    throw (uno::RuntimeException)
{
    uno::Any aRet;
    if ( rType == ::getCppuType( static_cast< uno::Reference< XAccessibleTable > * >( 0 ) ) )
    {
        uno::Reference<XAccessibleTable> xThis( this );
           aRet <<= xThis;
    }
    else if ( rType == ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) ) )
    {
        uno::Reference<XAccessibleSelection> xSelection( this );
        aRet <<= xSelection;
    }
    else
    {
        aRet = SwAccessibleContext::queryInterface(rType);
    }

    return aRet;
}

//====== XTypeProvider ====================================================
uno::Sequence< uno::Type > SAL_CALL SwAccessibleTable::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 2 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleTable > * >( 0 ) );

    return aTypes;
}

namespace
{
    class theSwAccessibleTableImplementationId : public rtl::Static< UnoTunnelIdInit, theSwAccessibleTableImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleTable::getImplementationId()
        throw(uno::RuntimeException)
{
    return theSwAccessibleTableImplementationId::get().getSeq();
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

OUString SAL_CALL SwAccessibleTable::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return sDesc;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleRowCount()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    return  GetTableData().GetRowCount();
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleColumnCount(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    return GetTableData().GetColumnCount();
}

OUString SAL_CALL SwAccessibleTable::getAccessibleRowDescription(
            sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Int32 nExtend = -1;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    GetTableData().CheckRowAndCol( nRow, nColumn, this );

    Int32Set_Impl::const_iterator aSttCol(
                                    GetTableData().GetColumnIter( nColumn ) );
    Int32Set_Impl::const_iterator aSttRow(
                                    GetTableData().GetRowIter( nRow ) );
    const SwFrm *pCellFrm = GetTableData().GetCellAtPos( *aSttCol, *aSttRow,
                                                         sal_False );
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Int32 nExtend = -1;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    GetTableData().CheckRowAndCol( nRow, nColumn, this );

    Int32Set_Impl::const_iterator aSttCol(
                                    GetTableData().GetColumnIter( nColumn ) );
    Int32Set_Impl::const_iterator aSttRow(
                                    GetTableData().GetRowIter( nRow ) );
    const SwFrm *pCellFrm = GetTableData().GetCellAtPos( *aSttCol, *aSttRow,
                                                         sal_False );
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
    throw (uno::RuntimeException)
{
    // Row headers aren't supported
    return uno::Reference< XAccessibleTable >();
}

uno::Reference< XAccessibleTable > SAL_CALL
        SwAccessibleTable::getAccessibleColumnHeaders(  )
    throw (uno::RuntimeException)
{
    // #i87532# - assure that return accesible object is empty,
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
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    const SwSelBoxes *pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        sal_Int32 nRows = GetTableData().GetRowCount();
        SwAccAllTableSelHander_Impl aSelRows( nRows  );

        GetTableData().GetSelection( 0, nRows, *pSelBoxes, aSelRows,
                                      sal_False );

        return aSelRows.GetSelSequence();
    }
    else
    {
        return uno::Sequence< sal_Int32 >( 0 );
    }
}

uno::Sequence< sal_Int32 > SAL_CALL SwAccessibleTable::getSelectedAccessibleColumns()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    const SwSelBoxes *pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        sal_Int32 nCols = GetTableData().GetColumnCount();
        SwAccAllTableSelHander_Impl aSelCols( nCols );

        GetTableData().GetSelection( 0, nCols, *pSelBoxes, aSelCols, sal_True );

        return aSelCols.GetSelSequence();
    }
    else
    {
        return uno::Sequence< sal_Int32 >( 0 );
    }
}

sal_Bool SAL_CALL SwAccessibleTable::isAccessibleRowSelected( sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    GetTableData().CheckRowAndCol( nRow, 0, this );

    sal_Bool bRet;
    const SwSelBoxes *pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        SwAccSingleTableSelHander_Impl aSelRow;
        GetTableData().GetSelection( nRow, nRow+1, *pSelBoxes, aSelRow,
                                     sal_False );
        bRet = aSelRow.IsSelected();
    }
    else
    {
        bRet = sal_False;
    }

    return bRet;
}

sal_Bool SAL_CALL SwAccessibleTable::isAccessibleColumnSelected(
        sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    GetTableData().CheckRowAndCol( 0, nColumn, this );

    sal_Bool bRet;
    const SwSelBoxes *pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        SwAccSingleTableSelHander_Impl aSelCol;

        GetTableData().GetSelection( nColumn, nColumn+1, *pSelBoxes, aSelCol,
                                     sal_True );
        bRet = aSelCol.IsSelected();
    }
    else
    {
        bRet = sal_False;
    }

    return bRet;
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleTable::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Reference< XAccessible > xRet;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    const SwFrm *pCellFrm =
                    GetTableData().GetCell( nRow, nColumn, sal_False, this );
    if( pCellFrm )
        xRet = GetMap()->GetContext( pCellFrm, sal_True );

    return xRet;
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleTable::getAccessibleCaption()
    throw (uno::RuntimeException)
{
    // captions aren't supported
    return uno::Reference< XAccessible >();
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleTable::getAccessibleSummary()
    throw (uno::RuntimeException)
{
    // summaries aren't supported
    return uno::Reference< XAccessible >();
}

sal_Bool SAL_CALL SwAccessibleTable::isAccessibleSelected(
            sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Bool bRet = sal_False;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    const SwFrm *pFrm =
                    GetTableData().GetCell( nRow, nColumn, sal_False, this );
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Int32 nRet = -1;

    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleTable )

    SwAccessibleChild aCell( GetTableData().GetCell( nRow, nColumn, sal_False, this ));
    if ( aCell.IsValid() )
    {
        nRet = GetChildIndex( *(GetMap()), aCell );
    }

    return nRet;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleRow( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
        throw( uno::RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleTable::supportsService(
        const OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleTable::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

void SwAccessibleTable::InvalidatePosOrSize( const SwRect& rOldBox )
{
    SolarMutexGuard aGuard;

    if( HasTableData() )
        GetTableData().SetTablePos( GetFrm()->Frm().Pos() );

    SwAccessibleContext::InvalidatePosOrSize( rOldBox );
}

void SwAccessibleTable::Dispose( sal_Bool bRecursive )
{
    SolarMutexGuard aGuard;

    if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

    SwAccessibleContext::Dispose( bRecursive );
}

void SwAccessibleTable::DisposeChild( const SwAccessibleChild& rChildFrmOrObj,
                                      sal_Bool bRecursive )
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
    // is there is no context for pFrm. The method is them called by
    // the map, and we have to call our superclass.
    // The other situation is that we have been call by a call to get notified
    // about its change. We then must not call the superclass
    uno::Reference< XAccessible > xAcc( GetMap()->GetContext( pFrm, sal_False ) );
    if( !xAcc.is() )
        SwAccessibleContext::DisposeChild( rChildFrmOrObj, bRecursive );
}

void SwAccessibleTable::InvalidateChildPosOrSize( const SwAccessibleChild& rChildFrmOrObj,
                                                  const SwRect& rOldBox )
{
    SolarMutexGuard aGuard;

    if( HasTableData() )
    {
        OSL_ENSURE( !HasTableData() ||
                GetFrm()->Frm().Pos() == GetTableData().GetTablePos(),
                "table has invalid position" );
        if( HasTableData() )
        {
            SwAccessibleTableData_Impl *pNewTableData = CreateNewTableData(); // #i77106#
            if( !pNewTableData->CompareExtents( GetTableData() ) )
            {
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


//
//  XAccessibleSelection
//

void SAL_CALL SwAccessibleTable::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
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

    // assure, that child, indentified by the given index, isn't already selected.
    if ( IsChildSelected( nChildIndex ) )
    {
        return;
    }

    // now we can start to do the work: check whether we already have
    // a table selection (in 'our' table). If so, extend the
    // selection, else select the current cell.

    // if we have a selection in a table, check if it's in the
    // same table that we're trying to select in
    const SwTableNode* pSelectedTable = pCrsrShell->IsCrsrInTbl();
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
    if( pSelectedTable == NULL || !pCrsrShell->GetTblCrs() )
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
        SwPaM *pPaM = pCrsrShell->GetTblCrs() ? pCrsrShell->GetTblCrs()
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
        const SwPaM *pPaM = pCrsrShell->GetTblCrs() ? pCrsrShell->GetTblCrs()
                                                    : pCrsrShell->GetCrsr();
        *(aPaM.GetMark()) = *pPaM->GetMark();
        Select( aPaM );

    }
}


sal_Bool SAL_CALL SwAccessibleTable::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleTable );

    if( (nChildIndex < 0) || (nChildIndex >= getAccessibleChildCount()) ) // #i77106#
        throw lang::IndexOutOfBoundsException();

    return IsChildSelected( nChildIndex );
}

void SAL_CALL SwAccessibleTable::clearAccessibleSelection(  )
    throw ( uno::RuntimeException )
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
    throw ( uno::RuntimeException )
{
    // first clear selection, then select first and last child
    clearAccessibleSelection();
    selectAccessibleChild( 0 );
    selectAccessibleChild( getAccessibleChildCount()-1 ); // #i77106#
}

sal_Int32 SAL_CALL SwAccessibleTable::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException )
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
            uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleTable );

    // paremter checking (part 1): index lower 0
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
            uno::RuntimeException )
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
    SwPaM *pPaM = pCrsrShell->GetTblCrs() ? pCrsrShell->GetTblCrs()
                                                : pCrsrShell->GetCrsr();
    sal_Bool bDeselectPoint =
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
    pPaM = pCrsrShell->GetTblCrs() ? pCrsrShell->GetTblCrs()
                                        : pCrsrShell->GetCrsr();
    *pPaM->GetPoint() = *pPaM->GetMark();
    pCrsrShell->EndAction();
}

// #i77106# - implementation of class <SwAccessibleTableColHeaders>
SwAccessibleTableColHeaders::SwAccessibleTableColHeaders( SwAccessibleMap *pMap2,
                                                          const SwTabFrm *pTabFrm )
    : SwAccessibleTable( pMap2, pTabFrm )
{
    SolarMutexGuard aGuard;

    const SwFrmFmt *pFrmFmt = pTabFrm->GetFmt();
    const_cast< SwFrmFmt * >( pFrmFmt )->Add( this );
    const String& rName = pFrmFmt->GetName();

    OUStringBuffer aBuffer( rName.Len() + 15 + 6 );
    aBuffer.append( OUString(rName) );
    aBuffer.append( rtl::OUString("-ColumnHeaders-") );
    aBuffer.append( static_cast<sal_Int32>( pTabFrm->GetPhyPageNum() ) );

    SetName( aBuffer.makeStringAndClear() );

    OUStringBuffer aBuffer2( rName.Len() + 14 );
    aBuffer2.append( OUString(rName) );
    aBuffer2.append( rtl::OUString("-ColumnHeaders") );
    OUString sArg1( aBuffer2.makeStringAndClear() );
    OUString sArg2( GetFormattedPageNumber() );

    OUString sDesc2 = GetResource( STR_ACCESS_TABLE_DESC, &sArg1, &sArg2 );
    SetDesc( sDesc2 );

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

//=====  XInterface  ======================================================
uno::Any SAL_CALL SwAccessibleTableColHeaders::queryInterface( const uno::Type& aType )
        throw (uno::RuntimeException)
{
    return SwAccessibleTable::queryInterface( aType );
}

//=====  XAccessibleContext  ==============================================
sal_Int32 SAL_CALL SwAccessibleTableColHeaders::getAccessibleChildCount(void)
        throw (uno::RuntimeException)
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
        throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    if ( nIndex < 0 || nIndex >= getAccessibleChildCount() )
    {
        throw lang::IndexOutOfBoundsException();
    }

    return SwAccessibleTable::getAccessibleChild( nIndex );
}

//=====  XAccessibleTable  ================================================
uno::Reference< XAccessibleTable >
        SAL_CALL SwAccessibleTableColHeaders::getAccessibleRowHeaders()
        throw (uno::RuntimeException)
{
    return uno::Reference< XAccessibleTable >();
}

uno::Reference< XAccessibleTable >
        SAL_CALL SwAccessibleTableColHeaders::getAccessibleColumnHeaders()
        throw (uno::RuntimeException)
{
    return uno::Reference< XAccessibleTable >();
}

//=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL SwAccessibleTableColHeaders::getImplementationName (void)
        throw (uno::RuntimeException)
{
    static const sal_Char sImplName[] = "com.sun.star.comp.Writer.SwAccessibleTableColumnHeadersView";
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplName));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
