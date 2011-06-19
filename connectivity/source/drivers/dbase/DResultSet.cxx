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
#include "dbase/DResultSet.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include "dbase/DIndex.hxx"
#include "dbase/DIndexIter.hxx"
#include "dbase/DCode.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/dbase_res.hrc"

using namespace ::comphelper;

using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
//------------------------------------------------------------------------------
ODbaseResultSet::ODbaseResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator&   _aSQLIterator)
                : file::OResultSet(pStmt,_aSQLIterator)
                ,m_bBookmarkable(sal_True)
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),         PROPERTY_ID_ISBOOKMARKABLE,       PropertyAttribute::READONLY,&m_bBookmarkable,                ::getBooleanCppuType());
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODbaseResultSet::getImplementationName(  ) throw ( RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.dbase.ResultSet"));
}
// -------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODbaseResultSet::getSupportedServiceNames(  ) throw( RuntimeException)
{
     Sequence< ::rtl::OUString > aSupported(2);
    aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.ResultSet"));
    aSupported[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.ResultSet"));
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseResultSet::supportsService( const ::rtl::OUString& _rServiceName ) throw( RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
Any SAL_CALL ODbaseResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODbaseResultSet_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OResultSet::queryInterface(rType);
}
// -------------------------------------------------------------------------
 Sequence<  Type > SAL_CALL ODbaseResultSet::getTypes(  ) throw( RuntimeException)
{
    return ::comphelper::concatSequences(OResultSet::getTypes(),ODbaseResultSet_BASE::getTypes());
}

// -------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL ODbaseResultSet::getBookmark(  ) throw( SQLException,  RuntimeException)
{
     ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    OSL_ENSURE((m_bShowDeleted || !m_aRow->isDeleted()),"getBookmark called for deleted row");

    return makeAny((sal_Int32)(m_aRow->get())[0]->getValue());
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseResultSet::moveToBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    return m_pTable ? Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),sal_True) : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    if(!m_pTable)
        return sal_False;


    Move(IResultSetHelper::BOOKMARK,comphelper::getINT32(bookmark),sal_False);

    return relative(rows);
}

// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseResultSet::compareBookmarks( const Any& lhs, const Any& rhs ) throw( SQLException,  RuntimeException)
{
    sal_Int32 nFirst(0),nSecond(0),nResult(0);
    if ( !( lhs  >>= nFirst ) || !( rhs >>= nSecond ) )
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sMessage = aResources.getResourceString(STR_INVALID_BOOKMARK);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    } // if ( !( lhs  >>= nFirst ) || !( rhs >>= nSecond ) )

    // have a look at CompareBookmark
    // we can't use the names there because we already have defines with the same name from the parser
    if(nFirst < nSecond)
        nResult = -1;
    else if(nFirst > nSecond)
        nResult = 1;
    else
        nResult = 0;

    return  nResult;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseResultSet::hasOrderedBookmarks(  ) throw( SQLException,  RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseResultSet::hashBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return comphelper::getINT32(bookmark);
}
// -------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL ODbaseResultSet::deleteRows( const  Sequence<  Any >& /*rows*/ ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedException( "XDeleteRows::deleteRows", *this );
    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------
sal_Bool ODbaseResultSet::fillIndexValues(const Reference< XColumnsSupplier> &_xIndex)
{
    Reference<XUnoTunnel> xTunnel(_xIndex,UNO_QUERY);
    if(xTunnel.is())
    {
        dbase::ODbaseIndex* pIndex = reinterpret_cast< dbase::ODbaseIndex* >( xTunnel->getSomething(dbase::ODbaseIndex::getUnoTunnelImplementationId()) );
        if(pIndex)
        {
            dbase::OIndexIterator* pIter = pIndex->createIterator(NULL,NULL);

            if (pIter)
            {
                sal_uInt32 nRec = pIter->First();
                while (nRec != SQL_COLUMN_NOTFOUND)
                {
                    if (m_aOrderbyAscending[0])
                        m_pFileSet->get().push_back(nRec);
                    else
                        m_pFileSet->get().insert(m_pFileSet->get().begin(),nRec);
                    nRec = pIter->Next();
                }
                m_pFileSet->setFrozen();
                delete pIter;
                return sal_True;
            }
            delete pIter;
        }
    }
    return sal_False;
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & ODbaseResultSet::getInfoHelper()
{
    return *ODbaseResultSet_BASE3::getArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODbaseResultSet::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODbaseResultSet::acquire() throw()
{
    ODbaseResultSet_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODbaseResultSet::release() throw()
{
    ODbaseResultSet_BASE2::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL ODbaseResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
OSQLAnalyzer* ODbaseResultSet::createAnalyzer()
{
    return new OFILEAnalyzer(m_pTable->getConnection());
}
// -----------------------------------------------------------------------------
sal_Int32 ODbaseResultSet::getCurrentFilePos() const
{
    return m_pTable->getFilePos();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
