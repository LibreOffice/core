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

#include "tablecolumn.hxx"
#include "tableundo.hxx"
#include "svx/svdmodel.hxx"
#include "svx/svdotable.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::beans;


namespace sdr { namespace table {

const sal_Int32 Property_Width = 0;
const sal_Int32 Property_OptimalWidth = 1;
const sal_Int32 Property_IsVisible = 2;
const sal_Int32 Property_IsStartOfNewPage = 3;


// TableRow


TableColumn::TableColumn( const TableModelRef& xTableModel, sal_Int32 nColumn )
: TableColumnBase( getStaticPropertySetInfo() )
, mxTableModel( xTableModel )
, mnColumn( nColumn )
, mnWidth( 0 )
, mbOptimalWidth( true )
, mbIsVisible( true )
, mbIsStartOfNewPage( false )
{
}


TableColumn::~TableColumn()
{
}


void TableColumn::dispose()
{
    mxTableModel.clear();
}


void TableColumn::throwIfDisposed() const
{
    if( !mxTableModel.is() )
        throw DisposedException();
}


TableColumn& TableColumn::operator=( const TableColumn& r )
{
    mnWidth = r.mnWidth;
    mbOptimalWidth = r.mbOptimalWidth;
    mbIsVisible = r.mbIsVisible;
    mbIsStartOfNewPage = r.mbIsStartOfNewPage;
    maName = r.maName;
    mnColumn = r.mnColumn;

    return *this;
}


// XCellRange


Reference< XCell > SAL_CALL TableColumn::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
{
    throwIfDisposed();
    if( nColumn != 0 )
        throw IndexOutOfBoundsException();

    return mxTableModel->getCellByPosition( mnColumn, nRow );
}


Reference< XCellRange > SAL_CALL TableColumn::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
{
    throwIfDisposed();
    if( (nTop >= 0 ) && (nLeft == 0) && (nBottom >= nTop) && (nRight == 0)  )
    {
        return mxTableModel->getCellRangeByPosition( mnColumn, nTop, mnColumn, nBottom );
    }
    throw IndexOutOfBoundsException();
}


Reference< XCellRange > SAL_CALL TableColumn::getCellRangeByName( const OUString& /*aRange*/ )
{
    return Reference< XCellRange >();
}


// XNamed


OUString SAL_CALL TableColumn::getName()
{
    return maName;
}


void SAL_CALL TableColumn::setName( const OUString& aName )
{
    maName = aName;
}


// XFastPropertySet


void SAL_CALL TableColumn::setFastPropertyValue( sal_Int32 nHandle, const Any& aValue )
{
    bool bOk = false;
    bool bChange = false;

    SdrModel* pModel = mxTableModel->getSdrTableObj()->GetModel();

    TableColumnUndo* pUndo = nullptr;
    if( mxTableModel.is() && mxTableModel->getSdrTableObj() && mxTableModel->getSdrTableObj()->IsInserted() && pModel && pModel->IsUndoEnabled() )
    {
        TableColumnRef xThis( this );
        pUndo = new TableColumnUndo( xThis );
    }

    switch( nHandle )
    {
    case Property_Width:
        {
            sal_Int32 nWidth = mnWidth;
            bOk = aValue >>= nWidth;
            if( bOk && (nWidth != mnWidth) )
            {
                mnWidth = nWidth;
                mbOptimalWidth = mnWidth == 0;
                bChange = true;
            }
            break;
        }
    case Property_OptimalWidth:
        {
            bool bOptimalWidth = mbOptimalWidth;
            bOk = aValue >>= bOptimalWidth;
            if( bOk && (mbOptimalWidth != bOptimalWidth) )
            {
                mbOptimalWidth = bOptimalWidth;
                if( bOptimalWidth )
                    mnWidth = 0;
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


Any SAL_CALL TableColumn::getFastPropertyValue( sal_Int32 nHandle )
{
    switch( nHandle )
    {
    case Property_Width:            return Any( mnWidth );
    case Property_OptimalWidth:     return Any( mbOptimalWidth );
    case Property_IsVisible:        return Any( mbIsVisible );
    case Property_IsStartOfNewPage: return Any( mbIsStartOfNewPage );
    default:                        throw UnknownPropertyException( OUString::number(nHandle), static_cast<cppu::OWeakObject*>(this));
    }
}


rtl::Reference< FastPropertySetInfo > TableColumn::getStaticPropertySetInfo()
{
    static rtl::Reference< FastPropertySetInfo > xInfo;
    if( !xInfo.is() )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !xInfo.is() )
        {
            PropertyVector aProperties(6);

            aProperties[0].Name = "Width";
            aProperties[0].Handle = Property_Width;
            aProperties[0].Type = ::cppu::UnoType<sal_Int32>::get();
            aProperties[0].Attributes = 0;

            aProperties[1].Name = "OptimalWidth";
            aProperties[1].Handle = Property_OptimalWidth;
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
            aProperties[4].Handle = Property_Width;
            aProperties[4].Type = ::cppu::UnoType<sal_Int32>::get();
            aProperties[4].Attributes = 0;

            aProperties[5].Name = "OptimalSize";
            aProperties[5].Handle = Property_OptimalWidth;
            aProperties[5].Type = cppu::UnoType<bool>::get();
            aProperties[5].Attributes = 0;

            xInfo.set( new FastPropertySetInfo(aProperties) );
        }
    }

    return xInfo;
}

TableModelRef const & TableColumn::getModel() const
{
    return mxTableModel;
}

sal_Int32 TableColumn::getWidth() const
{
    return mnWidth;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
