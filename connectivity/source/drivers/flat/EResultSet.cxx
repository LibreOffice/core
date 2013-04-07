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
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include "flat/EResultSet.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>

using namespace ::comphelper;

using namespace connectivity::flat;
using namespace connectivity::file;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

//------------------------------------------------------------------------------
OFlatResultSet::OFlatResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator& _aSQLIterator)
                : file::OResultSet(pStmt,_aSQLIterator)
                ,m_bBookmarkable(sal_True)
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),         PROPERTY_ID_ISBOOKMARKABLE,       PropertyAttribute::READONLY,&m_bBookmarkable,                ::getBooleanCppuType());
}
// -------------------------------------------------------------------------
OUString SAL_CALL OFlatResultSet::getImplementationName(  ) throw ( RuntimeException)
{
    return OUString("com.sun.star.sdbcx.flat.ResultSet");
}
// -------------------------------------------------------------------------
Sequence< OUString > SAL_CALL OFlatResultSet::getSupportedServiceNames(  ) throw( RuntimeException)
{
     Sequence< OUString > aSupported(2);
    aSupported[0] = OUString("com.sun.star.sdbc.ResultSet");
    aSupported[1] = OUString("com.sun.star.sdbcx.ResultSet");
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OFlatResultSet::supportsService( const OUString& _rServiceName ) throw( RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
Any SAL_CALL OFlatResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if(rType == ::getCppuType((const Reference<XDeleteRows>*)0) || rType == ::getCppuType((const Reference<XResultSetUpdate>*)0)
        || rType == ::getCppuType((const Reference<XRowUpdate>*)0))
        return Any();

    const Any aRet = OResultSet::queryInterface(rType);
    return aRet.hasValue() ? aRet : OFlatResultSet_BASE::queryInterface(rType);
}
// -------------------------------------------------------------------------
Sequence<  Type > SAL_CALL OFlatResultSet::getTypes(  ) throw( RuntimeException)
{
    Sequence< Type > aTypes = OResultSet::getTypes();
    ::std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(*pBegin == ::getCppuType((const Reference<XDeleteRows>*)0) ||
            *pBegin == ::getCppuType((const Reference<XResultSetUpdate>*)0) ||
            *pBegin == ::getCppuType((const Reference<XRowUpdate>*)0)))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    Type* pTypes = aOwnTypes.empty() ? 0 : &aOwnTypes[0];
    Sequence< Type > aRet(pTypes, aOwnTypes.size());
    return ::comphelper::concatSequences(aRet,OFlatResultSet_BASE::getTypes());
}

// -------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL OFlatResultSet::getBookmark(  ) throw( SQLException,  RuntimeException)
{
     ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return makeAny((sal_Int32)(m_aRow->get())[0]->getValue());
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OFlatResultSet::moveToBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    return Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),sal_True);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OFlatResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),sal_False);

    return relative(rows);
}

// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OFlatResultSet::compareBookmarks( const Any& lhs, const  Any& rhs ) throw( SQLException,  RuntimeException)
{
    return (lhs == rhs) ? 0 : 2;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OFlatResultSet::hasOrderedBookmarks(  ) throw( SQLException,  RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OFlatResultSet::hashBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    return comphelper::getINT32(bookmark);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OFlatResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & OFlatResultSet::getInfoHelper()
{
    return *OFlatResultSet_BASE3::getArrayHelper();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFlatResultSet::acquire() throw()
{
    OFlatResultSet_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFlatResultSet::release() throw()
{
    OFlatResultSet_BASE2::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OFlatResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
