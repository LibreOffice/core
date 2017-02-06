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


#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include "cell.hxx"
#include "tablerow.hxx"
#include "tableundo.hxx"
#include "svx/svdmodel.hxx"
#include "svx/svdotable.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::beans;


namespace sdr { namespace table {

const sal_Int32 Property_Height = 0;
const sal_Int32 Property_OptimalHeight = 1;
const sal_Int32 Property_IsVisible = 2;
const sal_Int32 Property_IsStartOfNewPage = 3;

TableRow::TableRow( const TableModelRef& xTableModel, sal_Int32 nRow, sal_Int32 nColumns )
: TableRowBase( getStaticPropertySetInfo() )
, mxTableModel( xTableModel )
, mnRow( nRow )
, mnHeight( 0 )
, mbOptimalHeight( true )
, mbIsVisible( true )
, mbIsStartOfNewPage( false )
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


TableRow::~TableRow()
{
}


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


void TableRow::throwIfDisposed() const
{
    if( !mxTableModel.is() )
        throw DisposedException();
}


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


void TableRow::removeColumns( sal_Int32 nIndex, sal_Int32 nCount )
{
    throwIfDisposed();
    if( (nCount >= 0) && ( nIndex >= 0) )
    {
        if( (nIndex + nCount) < static_cast< sal_Int32 >( maCells.size() ) )
        {
            CellVector::iterator aBegin( maCells.begin() );
            while( nIndex-- && (aBegin != maCells.end()) )
                ++aBegin;

            if( nCount > 1 )
            {
                CellVector::iterator aEnd( aBegin );
                while( nCount-- && (aEnd != maCells.end()) )
                    ++aEnd;
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

const TableModelRef& TableRow::getModel() const
{
    return mxTableModel;
}

// XCellRange


Reference< XCell > SAL_CALL TableRow::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
{
    throwIfDisposed();
    if( nRow != 0 )
        throw IndexOutOfBoundsException();

    return mxTableModel->getCellByPosition( nColumn, mnRow );
}


Reference< XCellRange > SAL_CALL TableRow::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
{
    throwIfDisposed();
    if( (nLeft >= 0 ) && (nTop == 0) && (nRight >= nLeft) && (nBottom == 0)  )
    {
        return mxTableModel->getCellRangeByPosition( nLeft, mnRow, nRight, mnRow );
    }
    throw IndexOutOfBoundsException();
}


Reference< XCellRange > SAL_CALL TableRow::getCellRangeByName( const OUString& /*aRange*/ )
{
    throwIfDisposed();
    return Reference< XCellRange >();
}


// XNamed


OUString SAL_CALL TableRow::getName()
{
    return maName;
}


void SAL_CALL TableRow::setName( const OUString& aName )
{
    maName = aName;
}


// XFastPropertySet


void SAL_CALL TableRow::setFastPropertyValue( sal_Int32 nHandle, const Any& aValue )
{
    bool bOk = false;
    bool bChange = false;

    TableRowUndo* pUndo = nullptr;

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
            bool bOptimalHeight = mbOptimalHeight;
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
            bool bIsVisible = mbIsVisible;
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
            bool bIsStartOfNewPage = mbIsStartOfNewPage;
            bOk = aValue >>= bIsStartOfNewPage;
            if( bOk && (mbIsStartOfNewPage != bIsStartOfNewPage) )
            {
                mbIsStartOfNewPage = bIsStartOfNewPage;
                bChange = true;
            }
            break;
        }
    default:
        delete pUndo;
        throw UnknownPropertyException( OUString::number(nHandle), static_cast<cppu::OWeakObject*>(this));
    }
    if( !bOk )
    {
        delete pUndo;
        throw IllegalArgumentException();
    }

    if( bChange )
    {
        if( pUndo )
        {
            pModel->AddUndo( pUndo );
            pUndo = nullptr;
        }
        mxTableModel->setModified(true);
    }

    delete pUndo;
}


Any SAL_CALL TableRow::getFastPropertyValue( sal_Int32 nHandle )
{
    switch( nHandle )
    {
    case Property_Height:           return Any( mnHeight );
    case Property_OptimalHeight:    return Any( mbOptimalHeight );
    case Property_IsVisible:        return Any( mbIsVisible );
    case Property_IsStartOfNewPage: return Any( mbIsStartOfNewPage );
    default:                        throw UnknownPropertyException( OUString::number(nHandle), static_cast<cppu::OWeakObject*>(this));
    }
}


rtl::Reference< FastPropertySetInfo > TableRow::getStaticPropertySetInfo()
{
    static rtl::Reference< FastPropertySetInfo > xInfo;
    if( !xInfo.is() )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !xInfo.is() )
        {
            PropertyVector aProperties(6);

            aProperties[0].Name = "Height";
            aProperties[0].Handle = Property_Height;
            aProperties[0].Type = ::cppu::UnoType<sal_Int32>::get();
            aProperties[0].Attributes = 0;

            aProperties[1].Name = "OptimalHeight";
            aProperties[1].Handle = Property_OptimalHeight;
            aProperties[1].Type = cppu::UnoType<bool>::get();
            aProperties[1].Attributes = 0;

            aProperties[2].Name = "IsVisible";
            aProperties[2].Handle = Property_IsVisible;
            aProperties[2].Type = cppu::UnoType<bool>::get();
            aProperties[2].Attributes = 0;

            aProperties[3].Name = "IsStartOfNewPage";
            aProperties[3].Handle = Property_IsStartOfNewPage;
            aProperties[3].Type = cppu::UnoType<bool>::get();
            aProperties[3].Attributes = 0;

            aProperties[4].Name = "Size";
            aProperties[4].Handle = Property_Height;
            aProperties[4].Type = ::cppu::UnoType<sal_Int32>::get();
            aProperties[4].Attributes = 0;

            aProperties[5].Name = "OptimalSize";
            aProperties[5].Handle = Property_OptimalHeight;
            aProperties[5].Type = cppu::UnoType<bool>::get();
            aProperties[5].Attributes = 0;

            xInfo.set( new FastPropertySetInfo(aProperties) );
        }
    }

    return xInfo;
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
