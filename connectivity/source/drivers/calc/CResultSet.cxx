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
#include "calc/CResultSet.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>

using namespace ::comphelper;
using namespace connectivity::calc;
using namespace connectivity::file;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


OCalcResultSet::OCalcResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator& _aSQLIterator)
                : file::OResultSet(pStmt,_aSQLIterator)
                ,m_bBookmarkable(sal_True)
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),         PROPERTY_ID_ISBOOKMARKABLE,       PropertyAttribute::READONLY,&m_bBookmarkable,                ::getBooleanCppuType());
}

OUString SAL_CALL OCalcResultSet::getImplementationName(  ) throw ( RuntimeException, std::exception)
{
    return OUString("com.sun.star.sdbcx.calc.ResultSet");
}

Sequence< OUString > SAL_CALL OCalcResultSet::getSupportedServiceNames(  ) throw( RuntimeException, std::exception)
{
     Sequence< OUString > aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OCalcResultSet::supportsService( const OUString& _rServiceName ) throw( RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Any SAL_CALL OCalcResultSet::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    Any aRet = OResultSet::queryInterface(rType);
    return aRet.hasValue() ? aRet : OCalcResultSet_BASE::queryInterface(rType);
}

 Sequence<  Type > SAL_CALL OCalcResultSet::getTypes(  ) throw( RuntimeException, std::exception)
{
    return ::comphelper::concatSequences(OResultSet::getTypes(),OCalcResultSet_BASE::getTypes());
}


// XRowLocate
Any SAL_CALL OCalcResultSet::getBookmark(  ) throw( SQLException,  RuntimeException, std::exception)
{
     ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return makeAny((sal_Int32)(m_aRow->get())[0]->getValue());
}

sal_Bool SAL_CALL OCalcResultSet::moveToBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    return Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),sal_True);
}

sal_Bool SAL_CALL OCalcResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows ) throw( SQLException,  RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),sal_False);

    return relative(rows);
}


sal_Int32 SAL_CALL OCalcResultSet::compareBookmarks( const Any& lhs, const  Any& rhs ) throw( SQLException,  RuntimeException, std::exception)
{
    return (lhs == rhs) ? 0 : 2;
}

sal_Bool SAL_CALL OCalcResultSet::hasOrderedBookmarks(  ) throw( SQLException,  RuntimeException, std::exception)
{
    return sal_True;
}

sal_Int32 SAL_CALL OCalcResultSet::hashBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return comphelper::getINT32(bookmark);
}

// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OCalcResultSet::deleteRows( const  Sequence<  Any >& /*rows*/ ) throw( SQLException,  RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedException( "XDeleteRows::deleteRows", *this );
    return Sequence< sal_Int32 >();
}

sal_Bool OCalcResultSet::fillIndexValues(const Reference< XColumnsSupplier> &/*_xIndex*/)
{
    //  Calc table has no index
    return sal_False;
}

::cppu::IPropertyArrayHelper & OCalcResultSet::getInfoHelper()
{
    return *OCalcResultSet_BASE3::getArrayHelper();
}

::cppu::IPropertyArrayHelper* OCalcResultSet::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}


void SAL_CALL OCalcResultSet::acquire() throw()
{
    OCalcResultSet_BASE2::acquire();
}

void SAL_CALL OCalcResultSet::release() throw()
{
    OCalcResultSet_BASE2::release();
}

::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OCalcResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
