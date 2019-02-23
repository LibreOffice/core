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

#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <component/CResultSet.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>

using namespace ::comphelper;
using namespace connectivity::component;
using namespace connectivity::file;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


OComponentResultSet::OComponentResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator& _aSQLIterator)
                : file::OResultSet(pStmt,_aSQLIterator)
                ,m_bBookmarkable(true)
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),         PROPERTY_ID_ISBOOKMARKABLE,       PropertyAttribute::READONLY,&m_bBookmarkable,                cppu::UnoType<bool>::get());
}

OUString SAL_CALL OComponentResultSet::getImplementationName(  )
{
    return OUString("com.sun.star.sdbcx.component.ResultSet");
}

Sequence< OUString > SAL_CALL OComponentResultSet::getSupportedServiceNames(  )
{
    Sequence< OUString > aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OComponentResultSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Any SAL_CALL OComponentResultSet::queryInterface( const Type & rType )
{
    Any aRet = OResultSet::queryInterface(rType);
    return aRet.hasValue() ? aRet : OComponentResultSet_BASE::queryInterface(rType);
}

 Sequence<  Type > SAL_CALL OComponentResultSet::getTypes(  )
{
    return ::comphelper::concatSequences(OResultSet::getTypes(),OComponentResultSet_BASE::getTypes());
}


// XRowLocate
Any SAL_CALL OComponentResultSet::getBookmark(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return makeAny(static_cast<sal_Int32>((m_aRow->get())[0]->getValue()));
}

sal_Bool SAL_CALL OComponentResultSet::moveToBookmark( const  Any& bookmark )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = false;

    return Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),true);
}

sal_Bool SAL_CALL OComponentResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = false;

    Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),false);

    return relative(rows);
}


sal_Int32 SAL_CALL OComponentResultSet::compareBookmarks( const Any& lhs, const  Any& rhs )
{
    return (lhs == rhs) ? CompareBookmark::EQUAL : CompareBookmark::NOT_EQUAL;
}

sal_Bool SAL_CALL OComponentResultSet::hasOrderedBookmarks(  )
{
    return true;
}

sal_Int32 SAL_CALL OComponentResultSet::hashBookmark( const  Any& bookmark )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return comphelper::getINT32(bookmark);
}

// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OComponentResultSet::deleteRows( const  Sequence<  Any >& /*rows*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedSQLException( "XDeleteRows::deleteRows", *this );
    return Sequence< sal_Int32 >();
}

bool OComponentResultSet::fillIndexValues(const Reference< XColumnsSupplier> &/*_xIndex*/)
{
    //  Writer or Calc table has no index
    return false;
}

::cppu::IPropertyArrayHelper & OComponentResultSet::getInfoHelper()
{
    return *OComponentResultSet_BASE3::getArrayHelper();
}

::cppu::IPropertyArrayHelper* OComponentResultSet::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}


void SAL_CALL OComponentResultSet::acquire() throw()
{
    OComponentResultSet_BASE2::acquire();
}

void SAL_CALL OComponentResultSet::release() throw()
{
    OComponentResultSet_BASE2::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OComponentResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
