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

#include "cell.hxx"
#include "tablerow.hxx"
#include "tablerows.hxx"



using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;



namespace sdr { namespace table {

TableRows::TableRows( const TableModelRef& xTableModel )
: mxTableModel( xTableModel )
{
}



TableRows::~TableRows()
{
    dispose();
}



void TableRows::dispose()
{
    mxTableModel.clear();
}



void TableRows::throwIfDisposed() const throw (css::uno::RuntimeException)
{
    if( !mxTableModel.is() )
        throw DisposedException();
}


// XTableRows


void SAL_CALL TableRows::insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (RuntimeException, std::exception)
{
    throwIfDisposed();
    mxTableModel->insertRows( nIndex, nCount );
}



void SAL_CALL TableRows::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (RuntimeException, std::exception)
{
    throwIfDisposed();
    mxTableModel->removeRows( nIndex, nCount );
}


// XIndexAccess


sal_Int32 SAL_CALL TableRows::getCount() throw (RuntimeException, std::exception)
{
    throwIfDisposed();
    return mxTableModel->getRowCount();
}



Any SAL_CALL TableRows::getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception)
{
    throwIfDisposed();
    return Any( Reference< XCellRange >( static_cast< XCellRange* >( mxTableModel->getRow( Index ).get() ) ) );
}


// XElementAccess


Type SAL_CALL TableRows::getElementType() throw (RuntimeException, std::exception)
{
    throwIfDisposed();
    return cppu::UnoType<XCellRange>::get();
}



sal_Bool SAL_CALL TableRows::hasElements() throw (RuntimeException, std::exception)
{
    throwIfDisposed();
    return mxTableModel->getRowCount() != 0;
}



} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
