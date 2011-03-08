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
#include "precompiled_connectivity.hxx"
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include "calc/CResultSet.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
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

//------------------------------------------------------------------------------
OCalcResultSet::OCalcResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator& _aSQLIterator)
                : file::OResultSet(pStmt,_aSQLIterator)
                ,m_bBookmarkable(sal_True)
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),         PROPERTY_ID_ISBOOKMARKABLE,       PropertyAttribute::READONLY,&m_bBookmarkable,                ::getBooleanCppuType());
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OCalcResultSet::getImplementationName(  ) throw ( RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.calc.ResultSet"));
}
// -------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OCalcResultSet::getSupportedServiceNames(  ) throw( RuntimeException)
{
     Sequence< ::rtl::OUString > aSupported(2);
    aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.ResultSet"));
    aSupported[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.ResultSet"));
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OCalcResultSet::supportsService( const ::rtl::OUString& _rServiceName ) throw( RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
Any SAL_CALL OCalcResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OResultSet::queryInterface(rType);
    return aRet.hasValue() ? aRet : OCalcResultSet_BASE::queryInterface(rType);
}
// -------------------------------------------------------------------------
 Sequence<  Type > SAL_CALL OCalcResultSet::getTypes(  ) throw( RuntimeException)
{
    return ::comphelper::concatSequences(OResultSet::getTypes(),OCalcResultSet_BASE::getTypes());
}

// -------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL OCalcResultSet::getBookmark(  ) throw( SQLException,  RuntimeException)
{
     ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return makeAny((sal_Int32)(m_aRow->get())[0]->getValue());
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OCalcResultSet::moveToBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    return Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),sal_True);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OCalcResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),sal_False);

    return relative(rows);
}

// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCalcResultSet::compareBookmarks( const Any& lhs, const  Any& rhs ) throw( SQLException,  RuntimeException)
{
    return (lhs == rhs) ? 0 : 2;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OCalcResultSet::hasOrderedBookmarks(  ) throw( SQLException,  RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCalcResultSet::hashBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return comphelper::getINT32(bookmark);
}
// -------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OCalcResultSet::deleteRows( const  Sequence<  Any >& /*rows*/ ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedException( "XDeleteRows::deleteRows", *this );
    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------
sal_Bool OCalcResultSet::fillIndexValues(const Reference< XColumnsSupplier> &/*_xIndex*/)
{
    //  Calc table has no index
    return sal_False;
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OCalcResultSet::getInfoHelper()
{
    return *OCalcResultSet_BASE3::getArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OCalcResultSet::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OCalcResultSet::acquire() throw()
{
    OCalcResultSet_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OCalcResultSet::release() throw()
{
    OCalcResultSet_BASE2::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OCalcResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
