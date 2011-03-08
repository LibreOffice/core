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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <comphelper/accessiblewrapper.hxx>
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>

#include <svx/AccessibleTableShape.hxx>
#include "tablecontroller.hxx"
#include "accessiblecell.hxx"

#include <algorithm>

#include <cppuhelper/implbase1.hxx>

using ::rtl::OUString;

using namespace ::accessibility;
using namespace ::sdr::table;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;

#define C2U(x) OUString(RTL_CONSTASCII_USTRINGPARAM(x))

namespace accessibility
{

struct hash
{
    std::size_t operator()( const Reference< XCell >& xCell ) const
    {
        return std::size_t( xCell.get() );
    }
};

typedef boost::unordered_map< Reference< XCell >, rtl::Reference< AccessibleCell >, hash > AccessibleCellMap;

//-----------------------------------------------------------------------------
// AccessibleTableShapeImpl
//-----------------------------------------------------------------------------

class AccessibleTableShapeImpl : public cppu::WeakImplHelper1< XModifyListener >
{
public:
    AccessibleTableShapeImpl( AccessibleShapeTreeInfo& rShapeTreeInfo );

    void init( const Reference< XAccessible>& xAccessible, const Reference< XTable >& xTable );
    void dispose();

    Reference< XAccessible > getAccessibleChild( sal_Int32 i ) throw(IndexOutOfBoundsException);
    void getColumnAndRow( sal_Int32 nChildIndex, sal_Int32& rnColumn, sal_Int32& rnRow ) throw (IndexOutOfBoundsException );

    // XModifyListener
    virtual void SAL_CALL modified( const EventObject& aEvent ) throw (RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    AccessibleShapeTreeInfo& mrShapeTreeInfo;
    Reference< XTable > mxTable;
    AccessibleCellMap maChildMap;
    Reference< XAccessible> mxAccessible;
};

//-----------------------------------------------------------------------------

AccessibleTableShapeImpl::AccessibleTableShapeImpl( AccessibleShapeTreeInfo& rShapeTreeInfo )
: mrShapeTreeInfo( rShapeTreeInfo )
{
}

//-----------------------------------------------------------------------------

void AccessibleTableShapeImpl::init( const Reference< XAccessible>& xAccessible, const Reference< XTable >& xTable )
{
    mxAccessible = xAccessible;
    mxTable = xTable;

    if( mxTable.is() )
    {
        Reference< XModifyListener > xListener( this );
        mxTable->addModifyListener( xListener );
    }
}

//-----------------------------------------------------------------------------

void AccessibleTableShapeImpl::dispose()
{
    if( mxTable.is() )
    {
        Reference< XModifyListener > xListener( this );
        mxTable->removeModifyListener( xListener );
        mxTable.clear();
    }
    mxAccessible.clear();
}

//-----------------------------------------------------------------------------

Reference< XAccessible > AccessibleTableShapeImpl::getAccessibleChild( sal_Int32 nChildIndex ) throw(IndexOutOfBoundsException)
{
    sal_Int32 nColumn = 0, nRow = 0;
    getColumnAndRow( nChildIndex, nColumn, nRow );

    Reference< XCell > xCell( mxTable->getCellByPosition( nColumn, nRow ) );
    AccessibleCellMap::iterator iter( maChildMap.find( xCell ) );

    if( iter != maChildMap.end() )
    {
        Reference< XAccessible > xChild( (*iter).second.get() );
        return xChild;
    }
    else
    {
        CellRef xCellRef( dynamic_cast< Cell* >( xCell.get() ) );

        rtl::Reference< AccessibleCell > xAccessibleCell( new AccessibleCell( mxAccessible, xCellRef, nChildIndex, mrShapeTreeInfo ) );

        maChildMap[xCell] = xAccessibleCell;

        xAccessibleCell->Init();

        Reference< XAccessible > xChild( xAccessibleCell.get() );
        return xChild;
    }
}

//-----------------------------------------------------------------------------

void AccessibleTableShapeImpl::getColumnAndRow( sal_Int32 nChildIndex, sal_Int32& rnColumn, sal_Int32& rnRow ) throw (IndexOutOfBoundsException )
{
    rnRow = 0;
    rnColumn = nChildIndex;

    if( mxTable.is() )
    {
        const sal_Int32 nColumnCount = mxTable->getColumnCount();
        while( rnColumn >= nColumnCount )
        {
            rnRow++;
            rnColumn -= nColumnCount;
        }

        if( rnRow < mxTable->getRowCount() )
            return;
    }

    throw IndexOutOfBoundsException();
}

// XModifyListener
void SAL_CALL AccessibleTableShapeImpl::modified( const EventObject& /*aEvent*/ ) throw (RuntimeException)
{
    if( mxTable.is() ) try
    {
        // structural changes may have happened to the table, validate all accessible cell instances
        AccessibleCellMap aTempChildMap;
        aTempChildMap.swap( maChildMap );

        // first move all still existing cells to maChildMap again and update their index

        const sal_Int32 nRowCount = mxTable->getRowCount();
        const sal_Int32 nColCount = mxTable->getColumnCount();

        sal_Int32 nChildIndex = 0;

        for( sal_Int32 nRow = 0; nRow < nRowCount; ++nRow )
        {
            for( sal_Int32 nCol = 0; nCol < nColCount; ++nCol )
            {
                Reference< XCell > xCell( mxTable->getCellByPosition( nCol, nRow ) );
                AccessibleCellMap::iterator iter( aTempChildMap.find( xCell ) );

                if( iter != aTempChildMap.end() )
                {
                    rtl::Reference< AccessibleCell > xAccessibleCell( (*iter).second );
                    xAccessibleCell->setIndexInParent( nChildIndex );
                    xAccessibleCell->CommitChange(AccessibleEventId::VISIBLE_DATA_CHANGED, Any(), Any());

                    // move still existing cell from temporary child map to our child map
                    maChildMap[xCell] = xAccessibleCell;
                    aTempChildMap.erase( iter );
                }

                ++nChildIndex;
            }
        }

        // all accessible cell instances still left in aTempChildMap must be disposed
        // as they are no longer part of the table

        for( AccessibleCellMap::iterator iter( aTempChildMap.begin() ); iter != aTempChildMap.end(); ++iter )
        {
            (*iter).second->dispose();
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("svx::AccessibleTableShape::modified(), exception caught!");
    }
}

// XEventListener
void SAL_CALL AccessibleTableShapeImpl::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
{
}

//-----------------------------------------------------------------------------
// AccessibleTableShape
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

AccessibleTableShape::AccessibleTableShape( const AccessibleShapeInfo& rShapeInfo, const AccessibleShapeTreeInfo& rShapeTreeInfo)
: AccessibleTableShape_Base(rShapeInfo, rShapeTreeInfo)
, mxImpl( new AccessibleTableShapeImpl( maShapeTreeInfo ) )
{
}

//-----------------------------------------------------------------------------

AccessibleTableShape::~AccessibleTableShape (void)
{
}

//-----------------------------------------------------------------------------

void AccessibleTableShape::Init()
{
    try
    {

        Reference< XPropertySet > xSet( mxShape, UNO_QUERY_THROW );
        Reference< XTable > xTable( xSet->getPropertyValue(C2U("Model")), UNO_QUERY_THROW );

        mxImpl->init( this, xTable );
    }
    catch( Exception& )
    {
        OSL_FAIL("AccessibleTableShape::init(), exception caught?");
    }

    AccessibleTableShape_Base::Init();
}

//-----------------------------------------------------------------------------

SvxTableController* AccessibleTableShape::getTableController()
{
    SdrView* pView = maShapeTreeInfo.GetSdrView ();
    if( pView )
        return dynamic_cast< SvxTableController* >( pView->getSelectionController().get() );
    else
        return 0;
}

//-----------------------------------------------------------------------------
// XInterface
//-----------------------------------------------------------------------------

Any SAL_CALL AccessibleTableShape::queryInterface( const Type& aType ) throw (RuntimeException)
{
    return AccessibleTableShape_Base::queryInterface( aType );
}

//-----------------------------------------------------------------------------

void SAL_CALL AccessibleTableShape::acquire(  ) throw ()
{
    AccessibleTableShape_Base::acquire();
}

//-----------------------------------------------------------------------------

void SAL_CALL AccessibleTableShape::release(  ) throw ()
{
    AccessibleTableShape_Base::release();
}

//-----------------------------------------------------------------------------
// XAccessible
//-----------------------------------------------------------------------------

Reference< XAccessibleContext > SAL_CALL AccessibleTableShape::getAccessibleContext(void) throw (RuntimeException)
{
    return AccessibleShape::getAccessibleContext ();
}

//-----------------------------------------------------------------------------
OUString SAL_CALL AccessibleTableShape::getImplementationName(void) throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.accessibility.AccessibleTableShape" ) );
}

//-----------------------------------------------------------------------------

OUString AccessibleTableShape::CreateAccessibleBaseName(void) throw (RuntimeException)
{
    return OUString (RTL_CONSTASCII_USTRINGPARAM("TableShape"));;
}

//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getAccessibleChildCount( ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mxImpl->mxTable.is() ? mxImpl->mxTable->getRowCount() * mxImpl->mxTable->getColumnCount() : 0;
}

//--------------------------------------------------------------------
Reference< XAccessible > SAL_CALL AccessibleTableShape::getAccessibleChild( sal_Int32 i ) throw(IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ThrowIfDisposed();

    return mxImpl->getAccessibleChild( i );
}

//--------------------------------------------------------------------
Reference< XAccessibleRelationSet > SAL_CALL AccessibleTableShape::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    return AccessibleShape::getAccessibleRelationSet( );
}

//--------------------------------------------------------------------

sal_Int16 SAL_CALL AccessibleTableShape::getAccessibleRole (void) throw (RuntimeException)
{
    return AccessibleRole::TABLE;
}

//--------------------------------------------------------------------

void SAL_CALL AccessibleTableShape::disposing (void)
{
    mxImpl->dispose();

    // let the base do it's stuff
    AccessibleShape::disposing();
}

//--------------------------------------------------------------------
// XAccessibleTable
//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getAccessibleRowCount() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mxImpl->mxTable.is() ? mxImpl->mxTable->getRowCount() : 0;
}

//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getAccessibleColumnCount(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mxImpl->mxTable.is() ? mxImpl->mxTable->getColumnCount() : 0;
}

//--------------------------------------------------------------------

OUString SAL_CALL AccessibleTableShape::getAccessibleRowDescription( sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException)
{
    checkCellPosition( 0, nRow );
    return OUString();
}

//--------------------------------------------------------------------

OUString SAL_CALL AccessibleTableShape::getAccessibleColumnDescription( sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    checkCellPosition( nColumn, 0 );
    return OUString();
}

//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    checkCellPosition( nColumn, nRow );
    if( mxImpl->mxTable.is() )
    {
        Reference< XMergeableCell > xCell( mxImpl->mxTable->getCellByPosition( nColumn, nRow ), UNO_QUERY );
        if( xCell.is() )
            return xCell->getRowSpan();
    }
    return 1;
}

//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    checkCellPosition( nColumn, nRow );
    if( mxImpl->mxTable.is() )
    {
        Reference< XMergeableCell > xCell( mxImpl->mxTable->getCellByPosition( nColumn, nRow ), UNO_QUERY );
        if( xCell.is() )
            return xCell->getColumnSpan();
    }
    return 1;
}

//--------------------------------------------------------------------

Reference< XAccessibleTable > SAL_CALL AccessibleTableShape::getAccessibleRowHeaders(  ) throw (RuntimeException)
{
    Reference< XAccessibleTable > xRet( this ); // todo
    return xRet;
}

//--------------------------------------------------------------------

Reference< XAccessibleTable > SAL_CALL AccessibleTableShape::getAccessibleColumnHeaders(  ) throw (RuntimeException)
{
    Reference< XAccessibleTable > xRet( this ); // todo
    return xRet;
}

//--------------------------------------------------------------------

Sequence< sal_Int32 > SAL_CALL AccessibleTableShape::getSelectedAccessibleRows(  ) throw (RuntimeException)
{
    Sequence< sal_Int32 > aRet;
    return aRet;
}

//--------------------------------------------------------------------

Sequence< sal_Int32 > SAL_CALL AccessibleTableShape::getSelectedAccessibleColumns(  ) throw (RuntimeException)
{
    Sequence< sal_Int32 > aRet;
    return aRet;
}

//--------------------------------------------------------------------

sal_Bool SAL_CALL AccessibleTableShape::isAccessibleRowSelected( sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    checkCellPosition( 0, nRow );
    return sal_False;
}

//--------------------------------------------------------------------

sal_Bool SAL_CALL AccessibleTableShape::isAccessibleColumnSelected( sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    checkCellPosition( nColumn, 0 );
    return sal_False;
}

//--------------------------------------------------------------------

Reference< XAccessible > SAL_CALL AccessibleTableShape::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    checkCellPosition( nColumn, nRow );

    sal_Int32 nChildIndex = 0;
    if( mxImpl->mxTable.is() )
        nChildIndex = mxImpl->mxTable->getColumnCount() * nRow + nColumn;

    return getAccessibleChild( nChildIndex );
}

//--------------------------------------------------------------------

Reference< XAccessible > SAL_CALL AccessibleTableShape::getAccessibleCaption(  ) throw (RuntimeException)
{
    Reference< XAccessible > xRet;
    return xRet;
}

//--------------------------------------------------------------------

Reference< XAccessible > SAL_CALL AccessibleTableShape::getAccessibleSummary(  ) throw (RuntimeException)
{
    Reference< XAccessible > xRet;
    return xRet;
}

//--------------------------------------------------------------------

sal_Bool SAL_CALL AccessibleTableShape::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    checkCellPosition( nColumn, nRow );

    SvxTableController* pController = getTableController();
    if( pController && pController->hasSelectedCells() )
    {
        CellPos aFirstPos, aLastPos;
        pController->getSelectedCells( aFirstPos, aLastPos );
        if( (aFirstPos.mnRow <= nRow) && (aFirstPos.mnCol <= nColumn) && (nRow <= aLastPos.mnRow) && (nColumn <= aLastPos.mnCol) )
            return sal_True;
    }

    return sal_False;
}

//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    checkCellPosition( nColumn, nRow );
    return  mxImpl->mxTable.is() ? (nRow * mxImpl->mxTable->getColumnCount() + nColumn) : 0;
}

//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getAccessibleRow( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    sal_Int32 nColumn = 0, nRow = 0;
    mxImpl->getColumnAndRow( nChildIndex, nColumn, nRow );
    return nRow;
}

//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getAccessibleColumn( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    sal_Int32 nColumn = 0, nRow = 0;
    mxImpl->getColumnAndRow( nChildIndex, nColumn, nRow );
    return nChildIndex;
}

//--------------------------------------------------------------------
// XAccessibleSelection
//--------------------------------------------------------------------

void SAL_CALL AccessibleTableShape::selectAccessibleChild( sal_Int32 nChildIndex ) throw ( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    CellPos aPos;
    mxImpl->getColumnAndRow( nChildIndex, aPos.mnCol, aPos.mnRow );

    // todo, select table shape?!?
    SvxTableController* pController = getTableController();
    if( pController )
    {
        CellPos aFirstPos( aPos ), aLastPos( aPos );
        if( pController->hasSelectedCells() )
        {
            pController->getSelectedCells( aFirstPos, aLastPos );

            aFirstPos.mnRow = std::min( aFirstPos.mnRow, aPos.mnRow );
            aFirstPos.mnCol = std::min( aFirstPos.mnCol, aPos.mnCol );
            aLastPos.mnRow = std::max( aLastPos.mnRow, aPos.mnRow );
            aLastPos.mnCol = std::max( aLastPos.mnCol, aPos.mnCol );
        }
        pController->setSelectedCells( aFirstPos, aLastPos );
    }
}

//--------------------------------------------------------------------

sal_Bool SAL_CALL AccessibleTableShape::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw ( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    CellPos aPos;
    mxImpl->getColumnAndRow( nChildIndex, aPos.mnCol, aPos.mnRow );

    return isAccessibleSelected(aPos.mnCol, aPos.mnRow);
}

//--------------------------------------------------------------------

void SAL_CALL AccessibleTableShape::clearAccessibleSelection() throw ( RuntimeException )
{
   SolarMutexGuard aSolarGuard;

    SvxTableController* pController = getTableController();
    if( pController )
        pController->clearSelection();
}
//--------------------------------------------------------------------

void SAL_CALL AccessibleTableShape::selectAllAccessibleChildren() throw ( RuntimeException )
{
   SolarMutexGuard aSolarGuard;

   // todo: force selection of shape?
    SvxTableController* pController = getTableController();
    if( pController )
        pController->selectAll();
}

//--------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleTableShape::getSelectedAccessibleChildCount() throw ( RuntimeException )
{
    SolarMutexGuard aSolarGuard;

    SvxTableController* pController = getTableController();
    if( pController && pController->hasSelectedCells() )
    {
        CellPos aFirstPos, aLastPos;
        pController->getSelectedCells( aFirstPos, aLastPos );

        const sal_Int32 nSelectedColumns = std::max( (sal_Int32)0, aLastPos.mnCol - aFirstPos.mnCol ) + 1;
        const sal_Int32 nSelectedRows = std::max( (sal_Int32)0, aLastPos.mnRow - aFirstPos.mnRow ) + 1;
        return nSelectedRows * nSelectedColumns;
    }

    return 0;
}

//--------------------------------------------------------------------

Reference< XAccessible > SAL_CALL AccessibleTableShape::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw ( IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    SvxTableController* pController = getTableController();
    if( pController && pController->hasSelectedCells() )
    {
        CellPos aFirstPos, aLastPos;
        pController->getSelectedCells( aFirstPos, aLastPos );

        const sal_Int32 nSelectedColumns = std::max( (sal_Int32)0, aLastPos.mnCol - aFirstPos.mnCol ) + 1;
        const sal_Int32 nSelectedRows = std::max( (sal_Int32)0, aLastPos.mnRow - aFirstPos.mnRow ) + 1;

        if( nSelectedChildIndex < (nSelectedRows * nSelectedColumns) )
        {
            while( nSelectedChildIndex >= nSelectedColumns )
            {
                aFirstPos.mnRow++;
                nSelectedChildIndex -= nSelectedColumns;
            }
            return getAccessibleCellAt( nSelectedColumns, aFirstPos.mnRow );
        }
    }

    throw IndexOutOfBoundsException();
}

//--------------------------------------------------------------------

void SAL_CALL AccessibleTableShape::deselectAccessibleChild( sal_Int32 nChildIndex )  throw ( IndexOutOfBoundsException, RuntimeException )
{
   SolarMutexGuard aSolarGuard;
    CellPos aPos;
    mxImpl->getColumnAndRow( nChildIndex, aPos.mnCol, aPos.mnRow );

    // todo, select table shape?!?
    SvxTableController* pController = getTableController();
    if( pController && pController->hasSelectedCells() )
    {
        CellPos aFirstPos, aLastPos;
        pController->getSelectedCells( aFirstPos, aLastPos );

        // create a selection where aPos is not part of anymore
        aFirstPos.mnRow = std::min( aFirstPos.mnRow, aPos.mnRow+1 );
        aFirstPos.mnCol = std::min( aFirstPos.mnCol, aPos.mnCol+1 );
        aLastPos.mnRow = std::max( aLastPos.mnRow, aPos.mnRow-1 );
        aLastPos.mnCol = std::max( aLastPos.mnCol, aPos.mnCol-1 );

        // new selection may be invalid (child to deselect is not at a border of the selection but in between)
        if( (aFirstPos.mnRow > aLastPos.mnRow) || (aFirstPos.mnCol > aLastPos.mnCol) )
            pController->clearSelection(); // if selection is invalid, clear all
        else
            pController->setSelectedCells( aFirstPos, aLastPos );
    }
}

//--------------------------------------------------------------------

void AccessibleTableShape::checkCellPosition( sal_Int32 nCol, sal_Int32 nRow ) throw ( IndexOutOfBoundsException )
{
    if( (nCol >= 0) && (nRow >= 0) && mxImpl->mxTable.is() && (nCol < mxImpl->mxTable->getColumnCount()) && (nRow < mxImpl->mxTable->getRowCount()) )
        return;

    throw IndexOutOfBoundsException();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
