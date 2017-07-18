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

#include "writer/WResultSet.hxx"

#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>

using namespace com::sun::star;

namespace connectivity
{
namespace writer
{

OWriterResultSet::OWriterResultSet(file::OStatement_Base* pStmt, connectivity::OSQLParseTreeIterator& _aSQLIterator)
    : file::OResultSet(pStmt,_aSQLIterator)
    ,m_bBookmarkable(true)
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE), PROPERTY_ID_ISBOOKMARKABLE, beans::PropertyAttribute::READONLY,&m_bBookmarkable, cppu::UnoType<bool>::get());
}

OUString SAL_CALL OWriterResultSet::getImplementationName()
{
    return OUString("com.sun.star.sdbcx.writer.ResultSet");
}

uno::Sequence<OUString> SAL_CALL OWriterResultSet::getSupportedServiceNames()
{
    uno::Sequence<OUString> aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OWriterResultSet::supportsService(const OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

uno::Any SAL_CALL OWriterResultSet::queryInterface(const uno::Type& rType)
{
    uno::Any aRet = OResultSet::queryInterface(rType);
    return aRet.hasValue() ? aRet : OWriterResultSet_BASE::queryInterface(rType);
}

uno::Sequence<uno::Type> SAL_CALL OWriterResultSet::getTypes()
{
    return ::comphelper::concatSequences(OResultSet::getTypes(), OWriterResultSet_BASE::getTypes());
}

uno::Any SAL_CALL OWriterResultSet::getBookmark()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(file::OResultSet_BASE::rBHelper.bDisposed);

    return uno::makeAny((sal_Int32)(m_aRow->get())[0]->getValue());
}

sal_Bool SAL_CALL OWriterResultSet::moveToBookmark(const uno::Any& bookmark)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(file::OResultSet_BASE::rBHelper.bDisposed);

    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = false;

    return Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),true);
}

sal_Bool SAL_CALL OWriterResultSet::moveRelativeToBookmark(const uno::Any& bookmark, sal_Int32 rows)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(file::OResultSet_BASE::rBHelper.bDisposed);

    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = false;

    Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),false);

    return relative(rows);
}

sal_Int32 SAL_CALL OWriterResultSet::compareBookmarks(const uno::Any& lhs, const uno::Any& rhs)
{
    return (lhs == rhs) ? css::sdbcx::CompareBookmark::EQUAL : css::sdbcx::CompareBookmark::NOT_EQUAL;
}

sal_Bool SAL_CALL OWriterResultSet::hasOrderedBookmarks()
{
    return true;
}

sal_Int32 SAL_CALL OWriterResultSet::hashBookmark(const uno::Any& bookmark)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(file::OResultSet_BASE::rBHelper.bDisposed);

    return comphelper::getINT32(bookmark);
}

uno::Sequence<sal_Int32> SAL_CALL OWriterResultSet::deleteRows(const uno::Sequence<uno::Any>& /*rows*/)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(file::OResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedSQLException("XDeleteRows::deleteRows", *this);
    return uno::Sequence<sal_Int32>();
}

bool OWriterResultSet::fillIndexValues(const uno::Reference<css::sdbcx::XColumnsSupplier>& /*_xIndex*/)
{
    // Writer table have no index.
    return false;
}

::cppu::IPropertyArrayHelper& OWriterResultSet::getInfoHelper()
{
    return *OWriterResultSet_BASE3::getArrayHelper();
}

cppu::IPropertyArrayHelper* OWriterResultSet::createArrayHelper() const
{
    uno::Sequence<beans::Property> aProps;
    describeProperties(aProps);
    return new cppu::OPropertyArrayHelper(aProps);
}

void SAL_CALL OWriterResultSet::acquire() throw()
{
    OWriterResultSet_BASE2::acquire();
}

void SAL_CALL OWriterResultSet::release() throw()
{
    OWriterResultSet_BASE2::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OWriterResultSet::getPropertySetInfo()
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

} // namespace writer
} // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
