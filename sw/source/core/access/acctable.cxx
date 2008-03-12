 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acctable.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:16:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <list>
#include <set>

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETABLEMODELCHANGE_HPP_
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETABLEMODELCHANGETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
// --> OD 2007-06-27 #i77106#
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif
// <--
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _CRSRSH_HXX
#include "crsrsh.hxx"
#endif
#ifndef _VISCRS_HXX
#include "viscrs.hxx"
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _FESH_HXX
#include "fesh.hxx"
#endif
#ifndef _ACCFRMOBJSLIST_HXX
#include <accfrmobjslist.hxx>
#endif
#ifndef _ACCMAP_HXX
#include "accmap.hxx"
#endif
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif
#ifndef _ACCTABLE_HXX
#include <acctable.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
};


//------------------------------------------------------------------------------

class SwAccessibleTableData_Impl
{
    Int32Set_Impl   maRows;
    Int32Set_Impl   maColumns;
    Int32PairList_Impl maExtents;   // cell extends for event processing only
    Point   maTabFrmPos;
    const SwTabFrm *mpTabFrm;
    sal_Bool mbIsInPagePreview;
    // --> OD 2007-06-27 #i77106#
    bool mbOnlyTableColumnHeader;
    // <--

    void CollectData( const SwFrm *pFrm );
    void CollectExtents( const SwFrm *pFrm );

    sal_Bool FindCell( const Point& rPos, const SwFrm *pFrm ,
                           sal_Bool bExact, const SwFrm *& rFrm ) const;

    void GetSelection( const Point& rTabPos, const SwRect& rArea,
                       const SwSelBoxes& rSelBoxes, const SwFrm *pFrm,
                       SwAccTableSelHander_Impl& rSelHdl,
                       sal_Bool bColumns ) const;

    // --> OD 2007-06-27 #i77106#
    inline bool IncludeRow( const SwFrm& rFrm ) const
    {
        return !mbOnlyTableColumnHeader ||
               mpTabFrm->IsInHeadline( rFrm );
    }
    // <--
public:
    // --> OD 2007-06-27 #i77106#
    // add third optional parameter <bOnlyTableColumnHeader>, default value <false>
    SwAccessibleTableData_Impl( const SwTabFrm *pTabFrm,
                                sal_Bool bIsInPagePreview,
                                bool bOnlyTableColumnHeader = false );
    // <--

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
    const SwFrmOrObjSList aList( pFrm );
    SwFrmOrObjSList::const_iterator aIter( aList.begin() );
    SwFrmOrObjSList::const_iterator aEndIter( aList.end() );
    while( aIter != aEndIter )
    {
        const SwFrmOrObj& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        if( pLower )
        {
            if( pLower->IsRowFrm() )
            {
                // --> OD 2007-06-27 #i77106#
                if ( IncludeRow( *pLower ) )
                {
                    maRows.insert( pLower->Frm().Top() - maTabFrmPos.Y() );
                    CollectData( pLower );
                }
                // <--
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
    const SwFrmOrObjSList aList( pFrm );
    SwFrmOrObjSList::const_iterator aIter( aList.begin() );
    SwFrmOrObjSList::const_iterator aEndIter( aList.end() );
    while( aIter != aEndIter )
    {
        const SwFrmOrObj& rLower = *aIter;
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
                // --> OD 2007-06-27 #i77106#
                if ( !pLower->IsRowFrm() ||
                     IncludeRow( *pLower ) )
                {
                    CollectExtents( pLower );
                }
                // <--
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

    const SwFrmOrObjSList aList( pFrm );
    SwFrmOrObjSList::const_iterator aIter( aList.begin() );
    SwFrmOrObjSList::const_iterator aEndIter( aList.end() );
    while( !bFound && aIter != aEndIter )
    {
        const SwFrmOrObj& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        ASSERT( pLower, "child should be a frame" );
        if( pLower )
        {
            if( rLower.IsAccessible( mbIsInPagePreview ) )
            {
                ASSERT( pLower->IsCellFrm(), "lower is not a cell frame" );
                const SwRect& rFrm = pLower->Frm();
                if( rFrm.Right() >= rPos.X() && rFrm.Bottom() >= rPos.Y() )
                {
                    // We have found the cell
                    ASSERT( rFrm.Left() <= rPos.X() && rFrm.Top() <= rPos.Y(),
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
                // --> OD 2007-06-27 #i77106#
                if ( !pLower->IsRowFrm() ||
                     IncludeRow( *pLower ) )
                {
                    bFound = FindCell( rPos, pLower, bExact, rRet );
                }
                // <--
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
    const SwFrmOrObjSList aList( pFrm );
    SwFrmOrObjSList::const_iterator aIter( aList.begin() );
    SwFrmOrObjSList::const_iterator aEndIter( aList.end() );
    while( aIter != aEndIter )
    {
        const SwFrmOrObj& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        ASSERT( pLower, "child should be a frame" );
        const SwRect& rBox = rLower.GetBox();
        if( pLower && rBox.IsOver( rArea ) )
        {
            if( rLower.IsAccessible( mbIsInPagePreview ) )
            {
                ASSERT( pLower->IsCellFrm(), "lower is not a cell frame" );
                const SwCellFrm *pCFrm =
                        static_cast < const SwCellFrm * >( pLower );
                SwTableBox *pBox =
                    const_cast< SwTableBox *>( pCFrm->GetTabBox() ); //SVPtrArr!
                if( !rSelBoxes.Seek_Entry( pBox ) )
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
                // --> OD 2007-06-27 #i77106#
                if ( !pLower->IsRowFrm() ||
                     IncludeRow( *pLower ) )
                {
                    GetSelection( rTabPos, rArea, rSelBoxes, pLower, rSelHdl,
                                  bColumns );
                }
                // <--
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

    Int32PairList_Impl::const_iterator aIter( maExtents.begin() );
    Int32PairList_Impl::const_iterator aEndIter( maExtents.end() );
    Int32PairList_Impl::const_iterator aCmpIter( rCmp.maExtents.begin() );
    while( aIter != aEndIter )
    {
        if( *aIter != *aCmpIter )
            return sal_False;

        ++aIter;
        ++aCmpIter;
    }

    return sal_True;
}

SwAccessibleTableData_Impl::SwAccessibleTableData_Impl( const SwTabFrm *pTabFrm,
                                                        sal_Bool bIsInPagePreview,
                                                        bool bOnlyTableColumnHeader )
    : maTabFrmPos( pTabFrm->Frm().Pos() ),
      mpTabFrm( pTabFrm ),
      mbIsInPagePreview( bIsInPagePreview ),
      // --> OD 2007-06-27 #i77106#
      mbOnlyTableColumnHeader( bOnlyTableColumnHeader )
      // <--
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
    ASSERT( nCount >= 0, "underflow" );
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

    ASSERT( nPos == nCount, "count is wrong" );

    return aRet;
}

void SwAccAllTableSelHander_Impl::Unselect( sal_Int32 nRowOrCol,
                                            sal_Int32 nExt )
{
    ASSERT( static_cast< size_t >( nRowOrCol ) < aSelected.size(),
             "index to large" );
    ASSERT( static_cast< size_t >( nRowOrCol+nExt ) <= aSelected.size(),
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
        pSelBoxes = &pCSh->GetTableCrsr()->GetBoxes();
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
    DBG_ASSERT( nChildIndex >= 0, "Illegal child index." );
    // --> OD 2007-06-27 #i77106#
    DBG_ASSERT( nChildIndex < const_cast<SwAccessibleTable*>(this)->getAccessibleChildCount(), "Illegal child index." );
    // <--

    const SwTableBox* pBox = NULL;

    // get table box for 'our' table cell
    SwFrmOrObj aCell( GetChild( nChildIndex ) );
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

    DBG_ASSERT( pBox != NULL, "We need the table box." );
    return pBox;
}

sal_Bool SwAccessibleTable::IsChildSelected( sal_Int32 nChildIndex ) const
{
    sal_Bool bRet = sal_False;
    const SwSelBoxes* pSelBoxes = GetSelBoxes();
    if( pSelBoxes )
    {
        const SwTableBox* pBox = GetTableBox( nChildIndex );
        DBG_ASSERT( pBox != NULL, "We need the table box." );
        bRet = pSelBoxes->Seek_Entry( const_cast<SwTableBox*>( pBox ) );
    }

    return bRet;
}

sal_Int32 SwAccessibleTable::GetIndexOfSelectedChild(
                sal_Int32 nSelectedChildIndex ) const
{
    // iterate over all children to n-th isAccessibleChildSelected()
    // --> OD 2007-06-27 #i77106#
    sal_Int32 nChildren = const_cast<SwAccessibleTable*>(this)->getAccessibleChildCount();
    // <--
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
    vos::OGuard aGuard(Application::GetSolarMutex());

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
    vos::OGuard aGuard(Application::GetSolarMutex());

    delete mpTableData;
}

void SwAccessibleTable::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    const SwTabFrm *pTabFrm = static_cast< const SwTabFrm * >( GetFrm() );
    switch( nWhich )
    {
    case RES_NAME_CHANGED:
        if( pTabFrm )
        {
            const SwFrmFmt *pFrmFmt = pTabFrm->GetFmt();
            ASSERT( pFrmFmt == GetRegisteredIn(), "invalid frame" );

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
        if( GetRegisteredIn() ==
                static_cast< SwModify *>( static_cast< SwPtrMsgPoolItem * >( pOld )->pObject ) )
            pRegisteredIn->Remove( this );
        break;

    default:
        SwClient::Modify( pOld, pNew );
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

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleTable::getImplementationId()
        throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

// --> OD 2007-06-28 #i77106#
SwAccessibleTableData_Impl* SwAccessibleTable::CreateNewTableData()
{
    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( GetFrm() );
    return new SwAccessibleTableData_Impl( pTabFrm, IsInPagePreview() );
}
// <--

void SwAccessibleTable::UpdateTableData()
{
    // --> OD 2007-06-28 #i77106# - usage of new method <CreateNewTableData()>
    delete mpTableData;
    mpTableData = CreateNewTableData();
    // <--
}

void SwAccessibleTable::ClearTableData()
{
    delete mpTableData;
    mpTableData = 0;
}

OUString SAL_CALL SwAccessibleTable::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return sDesc;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleRowCount()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleTable )

    return  GetTableData().GetRowCount();
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleColumnCount(  )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleTable )

    return GetTableData().GetColumnCount();
}

OUString SAL_CALL SwAccessibleTable::getAccessibleRowDescription(
            sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    // TODO: Is there any reasonable we can do here?
    OUString sTmpDesc;

    GetTableData().CheckRowAndCol(nRow, 0, this);

    return sTmpDesc;
}

OUString SAL_CALL SwAccessibleTable::getAccessibleColumnDescription(
            sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    // TODO: Is there any reasonable we can do here?
    OUString sTmpDesc;

    GetTableData().CheckRowAndCol(0, nColumn, this);

    return sTmpDesc;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleRowExtentAt(
            sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Int32 nExtend = -1;

    vos::OGuard aGuard(Application::GetSolarMutex());

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

    vos::OGuard aGuard(Application::GetSolarMutex());

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
    // --> OD 2007-06-29 #i77106#
    return new SwAccessibleTableColHeaders(
                        GetMap(), static_cast< const SwTabFrm *>( GetFrm() ) );
}

uno::Sequence< sal_Int32 > SAL_CALL SwAccessibleTable::getSelectedAccessibleRows()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

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
    vos::OGuard aGuard(Application::GetSolarMutex());

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
    vos::OGuard aGuard(Application::GetSolarMutex());

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
    vos::OGuard aGuard(Application::GetSolarMutex());

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

    vos::OGuard aGuard(Application::GetSolarMutex());

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

    vos::OGuard aGuard(Application::GetSolarMutex());

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
                const_cast< SwTableBox *>( pCFrm->GetTabBox() ); //SVPtrArr!
            bRet = pSelBoxes->Seek_Entry( pBox );
        }
    }

    return bRet;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleIndex(
            sal_Int32 nRow, sal_Int32 nColumn )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Int32 nRet = -1;

    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleTable )

    SwFrmOrObj aCell( GetTableData().GetCell( nRow, nColumn, sal_False, this ));
    if( aCell.IsValid() )
        nRet = GetChildIndex( aCell );

    return nRet;
}

sal_Int32 SAL_CALL SwAccessibleTable::getAccessibleRow( sal_Int32 nChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Int32 nRet = -1;

    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleTable )

    // --> OD 2007-06-27 #i77106#
    if ( ( nChildIndex < 0 ) ||
         ( nChildIndex >= getAccessibleChildCount() ) )
    {
        throw lang::IndexOutOfBoundsException();
    }
    // <--

    SwFrmOrObj aCell( GetChild( nChildIndex ) );
    if( aCell.GetSwFrm()  )
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
        ASSERT( !aCell.IsValid(), "SwAccessibleTable::getAccessibleColumn:"
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

    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleTable )

    // --> OD 2007-06-27 #i77106#
    if ( ( nChildIndex < 0 ) ||
         ( nChildIndex >= getAccessibleChildCount() ) )
    {
        throw lang::IndexOutOfBoundsException();
    }
    // <--

    SwFrmOrObj aCell( GetChild( nChildIndex ) );
    if( aCell.GetSwFrm()  )
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
        ASSERT( !aCell.IsValid(), "SwAccessibleTable::getAccessibleColumn:"
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( HasTableData() )
        GetTableData().SetTablePos( GetFrm()->Frm().Pos() );

    SwAccessibleContext::InvalidatePosOrSize( rOldBox );
}

void SwAccessibleTable::Dispose( sal_Bool bRecursive )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );

    SwAccessibleContext::Dispose( bRecursive );
}

void SwAccessibleTable::DisposeChild( const SwFrmOrObj& rChildFrmOrObj,
                                    sal_Bool bRecursive )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    const SwFrm *pFrm = rChildFrmOrObj.GetSwFrm();
    ASSERT( pFrm, "frame expected" );
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

void SwAccessibleTable::InvalidateChildPosOrSize( const SwFrmOrObj& rChildFrmOrObj,
                                                  const SwRect& rOldBox )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( HasTableData() )
    {
        ASSERT( !HasTableData() ||
                GetFrm()->Frm().Pos() == GetTableData().GetTablePos(),
                "table has invalid position" );
        if( HasTableData() )
        {
            // --> OD 2007-06-28 #i77106#
            SwAccessibleTableData_Impl *pNewTableData = CreateNewTableData();
            // <--
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

    // There are two reason why this method has been called. The first one
    // is there is no context for pFrm. The method is them called by
    // the map, and we have to call our superclass.
    // The other situation is that we have been call by a call to get notified
    // about its change. We then must not call the superclass
    ASSERT( rChildFrmOrObj.GetSwFrm(), "frame expected" );
    uno::Reference< XAccessible > xAcc( GetMap()->GetContext( rChildFrmOrObj.GetSwFrm(), sal_False ) );
    if( !xAcc.is() )
        SwAccessibleContext::InvalidateChildPosOrSize( rChildFrmOrObj, rOldBox );
}


//
//  XAccessibleSelection
//

void SAL_CALL SwAccessibleTable::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleTable );

    // --> OD 2007-06-27 #i77106#
    if( (nChildIndex < 0) || (nChildIndex >= getAccessibleChildCount()) )
    // <--
        throw lang::IndexOutOfBoundsException();

    // preliminaries: get 'our' table box, and get the cursor shell
    const SwTableBox* pBox = GetTableBox( nChildIndex );
    DBG_ASSERT( pBox != NULL, "We need the table box." );

    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell == NULL )
        return;

    // --> OD 2004-11-16 #111714# - assure, that child, indentified by the given
    // index, isn't already selected.
    if ( IsChildSelected( nChildIndex ) )
    {
        return;
    }
    // <--

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
        USHORT nPos =
            pSelectedTable->GetTable().GetTabLines().GetPos( pUpper );
        if( nPos == USHRT_MAX )
            pSelectedTable = NULL;
    }

    // create the new selection
    const SwStartNode* pStartNode = pBox->GetSttNd();
    if( pSelectedTable == NULL || !pCrsrShell->GetTblCrs() )
    {
        // if we're in the wrong table, or there's no table selection
        // at all, then select the current table cell.
//      SwPaM* pPaM = pCrsrShell->GetCrsr();
//      pPaM->DeleteMark();
//      *(pPaM->GetPoint()) = SwPosition( *pStartNode );
//      pPaM->Move( fnMoveForward, fnGoNode );
// //   pCrsrShell->SelTblBox();

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

        // if only one box is selected, we select this one in
        // order to maintain our table selection
//        if( aPaM.GetPoint()->nNode.GetNode().FindTableBoxStartNode() ==
//           aPaM.GetMark()->nNode.GetNode().FindTableBoxStartNode() )
//        {
// //            pCrsrShell->SelTblBox();
//         }
//         else
//         {
            // finally; set the selection. This will call UpdateCursor
            // on the cursor shell, too.
//            pCrsrShell->KillPams();
 //           pCrsrShell->SetSelection( aPaM );
//         }
    }
}


sal_Bool SAL_CALL SwAccessibleTable::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleTable );

    // --> OD 2007-06-27 #i77106#
    if( (nChildIndex < 0) || (nChildIndex >= getAccessibleChildCount()) )
    // <--
        throw lang::IndexOutOfBoundsException();

    return IsChildSelected( nChildIndex );
}

void SAL_CALL SwAccessibleTable::clearAccessibleSelection(  )
    throw ( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

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
    // --> OD 2007-06-27 #i77106#
    selectAccessibleChild( getAccessibleChildCount()-1 );
    // <--
}

sal_Int32 SAL_CALL SwAccessibleTable::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleTable );

    // iterate over all children and count isAccessibleChildSelected()
    sal_Int32 nCount = 0;

    // --> OD 2007-06-27 #i71106#
    sal_Int32 nChildren = getAccessibleChildCount();
    // <--
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleTable );

    // paremter checking (part 1): index lower 0
    if( nSelectedChildIndex < 0 )
        throw lang::IndexOutOfBoundsException();

    sal_Int32 nChildIndex = GetIndexOfSelectedChild( nSelectedChildIndex );

    // parameter checking (part 2): index higher than selected children?
    if( nChildIndex < 0 )
        throw lang::IndexOutOfBoundsException();

    // --> OD 2007-06-28 #i77106#
    if ( nChildIndex >= getAccessibleChildCount() )
    {
        throw lang::IndexOutOfBoundsException();
    }
    // <--

    return getAccessibleChild( nChildIndex );
}

// --> OD 2004-11-16 #111714# - index has to be treated as global child index.
void SAL_CALL SwAccessibleTable::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleTable );

    SwCrsrShell* pCrsrShell = GetCrsrShell();

    // --> OD 2004-11-16 #111714# - index has to be treated as global child index
    if ( !pCrsrShell )
        throw lang::IndexOutOfBoundsException();

    // assure, that given child index is in bounds.
    // --> OD 2007-06-27 #i77106#
    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
    // <--
        throw lang::IndexOutOfBoundsException();

    // assure, that child, identified by the given index, is selected.
    if ( !IsChildSelected( nChildIndex ) )
        return;
    // <--

    const SwTableBox* pBox = GetTableBox( nChildIndex );
    DBG_ASSERT( pBox != NULL, "We need the table box." );

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

// --> OD 2007-06-28 #i77106#
// implementation of class <SwAccessibleTableColHeaders>
SwAccessibleTableColHeaders::SwAccessibleTableColHeaders( SwAccessibleMap *pMap2,
                                                          const SwTabFrm *pTabFrm )
    : SwAccessibleTable( pMap2, pTabFrm )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    const SwFrmFmt *pFrmFmt = pTabFrm->GetFmt();
    const_cast< SwFrmFmt * >( pFrmFmt )->Add( this );
    const String& rName = pFrmFmt->GetName();

    OUStringBuffer aBuffer( rName.Len() + 15 + 6 );
    aBuffer.append( OUString(rName) );
    aBuffer.append( String::CreateFromAscii("-ColumnHeaders-") );
    aBuffer.append( static_cast<sal_Int32>( pTabFrm->GetPhyPageNum() ) );

    SetName( aBuffer.makeStringAndClear() );

    OUStringBuffer aBuffer2( rName.Len() + 14 );
    aBuffer2.append( OUString(rName) );
    aBuffer2.append( String::CreateFromAscii("-ColumnHeaders") );
    OUString sArg1( aBuffer2.makeStringAndClear() );
    OUString sArg2( GetFormattedPageNumber() );

    OUString sDesc2 = GetResource( STR_ACCESS_TABLE_DESC, &sArg1, &sArg2 );
    SetDesc( sDesc2 );
}

SwAccessibleTableData_Impl* SwAccessibleTableColHeaders::CreateNewTableData()
{
    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( GetFrm() );
    return new SwAccessibleTableData_Impl( pTabFrm, IsInPagePreview(), true );
}


void SwAccessibleTableColHeaders::Modify( SfxPoolItem * /*pOld*/, SfxPoolItem * /*pNew*/ )
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    sal_Int32 nCount = 0;

    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( GetFrm() );
    const SwFrmOrObjSList aVisList( GetVisArea(), pTabFrm );
    SwFrmOrObjSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwFrmOrObj& rLower = *aIter;
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
                nCount += SwAccessibleFrame::GetChildCount( GetVisArea(),
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
