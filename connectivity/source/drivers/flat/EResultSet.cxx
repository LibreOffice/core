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
#include <flat/EResultSet.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::comphelper;

using namespace connectivity::flat;
using namespace connectivity::file;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


OFlatResultSet::OFlatResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator& _aSQLIterator)
                : file::OResultSet(pStmt,_aSQLIterator)
                ,m_bBookmarkable(true)
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),         PROPERTY_ID_ISBOOKMARKABLE,       PropertyAttribute::READONLY,&m_bBookmarkable,                cppu::UnoType<bool>::get());
}

OUString SAL_CALL OFlatResultSet::getImplementationName(  )
{
    return OUString("com.sun.star.sdbcx.flat.ResultSet");
}

Sequence< OUString > SAL_CALL OFlatResultSet::getSupportedServiceNames(  )
{
    Sequence< OUString > aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL OFlatResultSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Any SAL_CALL OFlatResultSet::queryInterface( const Type & rType )
{
    if(rType == cppu::UnoType<XDeleteRows>::get()|| rType == cppu::UnoType<XResultSetUpdate>::get()
        || rType == cppu::UnoType<XRowUpdate>::get())
        return Any();

    const Any aRet = OResultSet::queryInterface(rType);
    return aRet.hasValue() ? aRet : OFlatResultSet_BASE::queryInterface(rType);
}

Sequence<  Type > SAL_CALL OFlatResultSet::getTypes(  )
{
    Sequence< Type > aTypes = OResultSet::getTypes();
    std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(*pBegin == cppu::UnoType<XDeleteRows>::get()||
            *pBegin == cppu::UnoType<XResultSetUpdate>::get()||
            *pBegin == cppu::UnoType<XRowUpdate>::get()))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    Sequence< Type > aRet(aOwnTypes.data(), aOwnTypes.size());
    return ::comphelper::concatSequences(aRet,OFlatResultSet_BASE::getTypes());
}


// XRowLocate
Any SAL_CALL OFlatResultSet::getBookmark(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return makeAny(static_cast<sal_Int32>((m_aRow->get())[0]->getValue()));
}

sal_Bool SAL_CALL OFlatResultSet::moveToBookmark( const  Any& bookmark )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = false;

    return Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),true);
}

sal_Bool SAL_CALL OFlatResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = false;

    Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),false);

    return relative(rows);
}


sal_Int32 SAL_CALL OFlatResultSet::compareBookmarks( const Any& lhs, const  Any& rhs )
{
    return (lhs == rhs) ? CompareBookmark::EQUAL : CompareBookmark::NOT_EQUAL;
}

sal_Bool SAL_CALL OFlatResultSet::hasOrderedBookmarks(  )
{
    return true;
}

sal_Int32 SAL_CALL OFlatResultSet::hashBookmark( const  Any& bookmark )
{
    return comphelper::getINT32(bookmark);
}

IPropertyArrayHelper* OFlatResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

IPropertyArrayHelper & OFlatResultSet::getInfoHelper()
{
    return *OFlatResultSet_BASE3::getArrayHelper();
}

void SAL_CALL OFlatResultSet::acquire() throw()
{
    OFlatResultSet_BASE2::acquire();
}

void SAL_CALL OFlatResultSet::release() throw()
{
    OFlatResultSet_BASE2::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OFlatResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
