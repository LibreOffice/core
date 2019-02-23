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
#include <dbase/DResultSet.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <dbase/DIndex.hxx>
#include <dbase/DIndexIter.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <strings.hrc>

using namespace ::comphelper;

using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

ODbaseResultSet::ODbaseResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator&   _aSQLIterator)
                : file::OResultSet(pStmt,_aSQLIterator)
                ,m_bBookmarkable(true)
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),         PROPERTY_ID_ISBOOKMARKABLE,       PropertyAttribute::READONLY,&m_bBookmarkable,                cppu::UnoType<bool>::get());
}

OUString SAL_CALL ODbaseResultSet::getImplementationName(  )
{
    return OUString("com.sun.star.sdbcx.dbase.ResultSet");
}

Sequence< OUString > SAL_CALL ODbaseResultSet::getSupportedServiceNames(  )
{
    Sequence< OUString > aSupported(2);
    aSupported[0] = "com.sun.star.sdbc.ResultSet";
    aSupported[1] = "com.sun.star.sdbcx.ResultSet";
    return aSupported;
}

sal_Bool SAL_CALL ODbaseResultSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Any SAL_CALL ODbaseResultSet::queryInterface( const Type & rType )
{
    Any aRet = ODbaseResultSet_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet::queryInterface(rType);
}

 Sequence<  Type > SAL_CALL ODbaseResultSet::getTypes(  )
{
    return ::comphelper::concatSequences(OResultSet::getTypes(),ODbaseResultSet_BASE::getTypes());
}


// XRowLocate
Any SAL_CALL ODbaseResultSet::getBookmark(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    OSL_ENSURE((m_bShowDeleted || !m_aRow->isDeleted()),"getBookmark called for deleted row");

    return makeAny(static_cast<sal_Int32>((m_aRow->get())[0]->getValue()));
}

sal_Bool SAL_CALL ODbaseResultSet::moveToBookmark( const  Any& bookmark )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = false;

    return m_pTable.is() && Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),true);
}

sal_Bool SAL_CALL ODbaseResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    if(!m_pTable.is())
        return false;


    Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),false);

    return relative(rows);
}


sal_Int32 SAL_CALL ODbaseResultSet::compareBookmarks( const Any& lhs, const Any& rhs )
{
    sal_Int32 nFirst(0),nSecond(0),nResult(0);
    if ( !( lhs  >>= nFirst ) || !( rhs >>= nSecond ) )
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_INVALID_BOOKMARK);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    } // if ( !( lhs  >>= nFirst ) || !( rhs >>= nSecond ) )

    if(nFirst < nSecond)
        nResult = CompareBookmark::LESS;
    else if(nFirst > nSecond)
        nResult = CompareBookmark::GREATER;
    else
        nResult = CompareBookmark::EQUAL;

    return  nResult;
}

sal_Bool SAL_CALL ODbaseResultSet::hasOrderedBookmarks(  )
{
    return true;
}

sal_Int32 SAL_CALL ODbaseResultSet::hashBookmark( const  Any& bookmark )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return comphelper::getINT32(bookmark);
}

// XDeleteRows
Sequence< sal_Int32 > SAL_CALL ODbaseResultSet::deleteRows( const  Sequence<  Any >& /*rows*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedSQLException( "XDeleteRows::deleteRows", *this );
    return Sequence< sal_Int32 >();
}

bool ODbaseResultSet::fillIndexValues(const Reference< XColumnsSupplier> &_xIndex)
{
    Reference<XUnoTunnel> xTunnel(_xIndex,UNO_QUERY);
    if(xTunnel.is())
    {
        dbase::ODbaseIndex* pIndex = reinterpret_cast< dbase::ODbaseIndex* >( xTunnel->getSomething(dbase::ODbaseIndex::getUnoTunnelImplementationId()) );
        if(pIndex)
        {
            std::unique_ptr<dbase::OIndexIterator> pIter = pIndex->createIterator();

            if (pIter)
            {
                sal_uInt32 nRec = pIter->First();
                while (nRec != NODE_NOTFOUND)
                {
                    m_pFileSet->get().push_back(nRec);
                    nRec = pIter->Next();
                }
                m_pFileSet->setFrozen();
                return true;
            }
        }
    }
    return false;
}

::cppu::IPropertyArrayHelper & ODbaseResultSet::getInfoHelper()
{
    return *ODbaseResultSet_BASE3::getArrayHelper();
}

::cppu::IPropertyArrayHelper* ODbaseResultSet::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

void SAL_CALL ODbaseResultSet::acquire() throw()
{
    ODbaseResultSet_BASE2::acquire();
}

void SAL_CALL ODbaseResultSet::release() throw()
{
    ODbaseResultSet_BASE2::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL ODbaseResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

sal_Int32 ODbaseResultSet::getCurrentFilePos() const
{
    return m_pTable->getFilePos();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
