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

#include <com/sun/star/lang/DisposedException.hpp>

#include "cell.hxx"
#include "tablerow.hxx"
#include "tableundo.hxx"
#include "svx/svdmodel.hxx"
#include "svx/svdotable.hxx"

// -----------------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::beans;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

const sal_Int32 Property_Height = 0;
const sal_Int32 Property_OptimalHeight = 1;
const sal_Int32 Property_IsVisible = 2;
const sal_Int32 Property_IsStartOfNewPage = 3;

// -----------------------------------------------------------------------------
// TableRow
// -----------------------------------------------------------------------------

TableRow::TableRow( const TableModelRef& xTableModel, sal_Int32 nRow, sal_Int32 nColumns )
: TableRowBase( getStaticPropertySetInfo() )
, mxTableModel( xTableModel )
, mnRow( nRow )
, mnHeight( 0 )
, mbOptimalHeight( sal_True )
, mbIsVisible( sal_True )
, mbIsStartOfNewPage( sal_False )
{
    if( nColumns < 20 )
        maCells.reserve( 20 );

    if( nColumns )
    {
        maCells.resize( nColumns );
        while( nColumns-- )
            maCells[ nColumns ] = mxTableModel->createCell();
    }
}

// -----------------------------------------------------------------------------

TableRow::~TableRow()
{
}

// -----------------------------------------------------------------------------

void TableRow::dispose()
{
    mxTableModel.clear();
    if( !maCells.empty() )
    {
        CellVector::iterator aIter( maCells.begin() );
        while( aIter != maCells.end() )
            (*aIter++)->dispose();
        CellVector().swap(maCells);
    }
}

// -----------------------------------------------------------------------------

void TableRow::throwIfDisposed() const throw (::com::sun::star::uno::RuntimeException)
{
    if( !mxTableModel.is() )
        throw DisposedException();
}

// -----------------------------------------------------------------------------

TableRow& TableRow::operator=( const TableRow& r )
{
    mnHeight = r.mnHeight;
    mbOptimalHeight = r.mbOptimalHeight;
    mbIsVisible = r.mbIsVisible;
    mbIsStartOfNewPage = r.mbIsStartOfNewPage;
    maName = r.maName;
    mnRow = r.mnRow;

    return *this;
}

// -----------------------------------------------------------------------------

void TableRow::insertColumns( sal_Int32 nIndex, sal_Int32 nCount, CellVector::iterator* pIter /* = 0 */  )
{
    throwIfDisposed();
    if( nCount )
    {
        if( nIndex >= static_cast< sal_Int32 >( maCells.size() ) )
            nIndex = static_cast< sal_Int32 >( maCells.size() );
        if ( pIter )
            maCells.insert( maCells.begin() + nIndex, *pIter, (*pIter) + nCount );
        else
        {
            maCells.reserve( maCells.size() + nCount );
            for ( sal_Int32 i = 0; i < nCount; i++ )
                maCells.insert( maCells.begin() + nIndex + i, mxTableModel->createCell() );
        }
    }
}

// -----------------------------------------------------------------------------

void TableRow::removeColumns( sal_Int32 nIndex, sal_Int32 nCount )
{
    throwIfDisposed();
    if( (nCount >= 0) && ( nIndex >= 0) )
    {
        if( (nIndex + nCount) < static_cast< sal_Int32 >( maCells.size() ) )
        {
            CellVector::iterator aBegin( maCells.begin() );
            while( nIndex-- && (aBegin != maCells.end()) )
                aBegin++;

            if( nCount > 1 )
            {
                CellVector::iterator aEnd( aBegin );
                while( nCount-- && (aEnd != maCells.end()) )
                    aEnd++;
                maCells.erase( aBegin, aEnd );
            }
            else
            {
                maCells.erase( aBegin );
            }
        }
        else
        {
            maCells.resize( nIndex );
        }
    }
}

// -----------------------------------------------------------------------------
// XCellRange
// -----------------------------------------------------------------------------

Reference< XCell > SAL_CALL TableRow::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException)
{
    throwIfDisposed();
    if( nRow != 0 )
        throw IndexOutOfBoundsException();

    return mxTableModel->getCellByPosition( nColumn, mnRow );
}

// -----------------------------------------------------------------------------

Reference< XCellRange > SAL_CALL TableRow::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw (IndexOutOfBoundsException, RuntimeException)
{
    throwIfDisposed();
    if( (nLeft >= 0 ) && (nTop == 0) && (nRight >= nLeft) && (nBottom == 0)  )
    {
        return mxTableModel->getCellRangeByPosition( nLeft, mnRow, nRight, mnRow );
    }
    throw IndexOutOfBoundsException();
}

// -----------------------------------------------------------------------------

Reference< XCellRange > SAL_CALL TableRow::getCellRangeByName( const OUString& /*aRange*/ ) throw (RuntimeException)
{
    throwIfDisposed();
    return Reference< XCellRange >();
}

// -----------------------------------------------------------------------------
// XNamed
// -----------------------------------------------------------------------------

OUString SAL_CALL TableRow::getName() throw (RuntimeException)
{
    return maName;
}

// -----------------------------------------------------------------------------

void SAL_CALL TableRow::setName( const OUString& aName ) throw (RuntimeException)
{
    maName = aName;
}

// -----------------------------------------------------------------------------
// XFastPropertySet
// -----------------------------------------------------------------------------

void SAL_CALL TableRow::setFastPropertyValue( sal_Int32 nHandle, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, RuntimeException)
{
    bool bOk = false;
    bool bChange = false;

    TableRowUndo* pUndo = 0;

    SdrModel* pModel = mxTableModel->getSdrTableObj()->GetModel();

    const bool bUndo = mxTableModel.is() && mxTableModel->getSdrTableObj() && mxTableModel->getSdrTableObj()->IsInserted() && pModel && pModel->IsUndoEnabled();

    if( bUndo )
    {
        TableRowRef xThis( this );
        pUndo = new TableRowUndo( xThis );
    }

    switch( nHandle )
    {
    case Property_Height:
        {
            sal_Int32 nHeight = mnHeight;
            bOk = aValue >>= nHeight;
            if( bOk && (mnHeight != nHeight) )
            {
                mnHeight = nHeight;
                mbOptimalHeight = mnHeight == 0;
                bChange = true;
            }
            break;
        }

    case Property_OptimalHeight:
        {
            sal_Bool bOptimalHeight = mbOptimalHeight;
            bOk = aValue >>= bOptimalHeight;
            if( bOk && (mbOptimalHeight != bOptimalHeight) )
            {
                mbOptimalHeight = bOptimalHeight;
                if( bOptimalHeight )
                    mnHeight = 0;
                bChange = true;
            }
            break;
        }
    case Property_IsVisible:
        {
            sal_Bool bIsVisible = mbIsVisible;
            bOk = aValue >>= bIsVisible;
            if( bOk && (mbIsVisible != bIsVisible) )
            {
                mbIsVisible = bIsVisible;
                bChange = true;
            }
            break;
        }

    case Property_IsStartOfNewPage:
        {
            sal_Bool bIsStartOfNewPage = mbIsStartOfNewPage;
            bOk = aValue >>= bIsStartOfNewPage;
            if( bOk && (mbIsStartOfNewPage != bIsStartOfNewPage) )
            {
                mbIsStartOfNewPage = bIsStartOfNewPage;
                bChange = true;
            }
            break;
        }
    default:
        throw UnknownPropertyException();
    }
    if( !bOk )
        throw IllegalArgumentException();

    if( bChange )
    {
        if( pUndo )
        {
            pModel->AddUndo( pUndo );
            pUndo = 0;
        }
        mxTableModel->setModified(sal_True);
    }

    if( pUndo )
        delete pUndo;
}

// -----------------------------------------------------------------------------

Any SAL_CALL TableRow::getFastPropertyValue( sal_Int32 nHandle ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    switch( nHandle )
    {
    case Property_Height:           return Any( mnHeight );
    case Property_OptimalHeight:    return Any( mbOptimalHeight );
    case Property_IsVisible:        return Any( mbIsVisible );
    case Property_IsStartOfNewPage: return Any( mbIsStartOfNewPage );
    default:                        throw UnknownPropertyException();
    }
}

// -----------------------------------------------------------------------------

rtl::Reference< ::comphelper::FastPropertySetInfo > TableRow::getStaticPropertySetInfo()
{
    static rtl::Reference< ::comphelper::FastPropertySetInfo > xInfo;
    if( !xInfo.is() )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !xInfo.is() )
        {
            comphelper::PropertyVector aProperties(6);

            aProperties[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) );
            aProperties[0].Handle = Property_Height;
            aProperties[0].Type = ::getCppuType((const sal_Int32*)0);
            aProperties[0].Attributes = 0;

            aProperties[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "OptimalHeight" ) );
            aProperties[1].Handle = Property_OptimalHeight;
            aProperties[1].Type = ::getBooleanCppuType();
            aProperties[1].Attributes = 0;

            aProperties[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsVisible" ) );
            aProperties[2].Handle = Property_IsVisible;
            aProperties[2].Type = ::getBooleanCppuType();
            aProperties[2].Attributes = 0;

            aProperties[3].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsStartOfNewPage" ) );
            aProperties[3].Handle = Property_IsStartOfNewPage;
            aProperties[3].Type = ::getBooleanCppuType();
            aProperties[3].Attributes = 0;

            aProperties[4].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) );
            aProperties[4].Handle = Property_Height;
            aProperties[4].Type = ::getCppuType((const sal_Int32*)0);
            aProperties[4].Attributes = 0;

            aProperties[5].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "OptimalSize" ) );
            aProperties[5].Handle = Property_OptimalHeight;
            aProperties[5].Type = ::getBooleanCppuType();
            aProperties[5].Attributes = 0;

            xInfo.set( new ::comphelper::FastPropertySetInfo(aProperties) );
        }
    }

    return xInfo;
}

// -----------------------------------------------------------------------------


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
