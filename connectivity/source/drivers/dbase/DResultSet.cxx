/*************************************************************************
 *
 *  $RCSfile: DResultSet.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:23:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif
#ifndef _CONNECTIVITY_DBASE_DRESULTSET_HXX_
#include "dbase/DResultSet.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXITER_HXX_
#include "dbase/DIndexIter.hxx"
#endif
#ifndef CONNECTIVITY_DBASE_DCODE_HXX
#include "dbase/DCode.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;

using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
//  using namespace com::sun::star::container;
//  using namespace com::sun::star::util;
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
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.dbase.ResultSet");
}
// -------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODbaseResultSet::getSupportedServiceNames(  ) throw( RuntimeException)
{
     Sequence< ::rtl::OUString > aSupported(2);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.ResultSet");
    aSupported[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.ResultSet");
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

    return makeAny((sal_Int32)(*m_aRow)[0]->getValue());
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
sal_Int32 SAL_CALL ODbaseResultSet::compareBookmarks( const  Any& first, const  Any& second ) throw( SQLException,  RuntimeException)
{
    sal_Int32 nFirst,nSecond,nResult;
    first  >>= nFirst;
    second >>= nSecond;

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
Sequence< sal_Int32 > SAL_CALL ODbaseResultSet::deleteRows( const  Sequence<  Any >& rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------
sal_Bool ODbaseResultSet::fillIndexValues(const Reference< XColumnsSupplier> &_xIndex)
{
    Reference<XUnoTunnel> xTunnel(_xIndex,UNO_QUERY);
    if(xTunnel.is())
    {
        dbase::ODbaseIndex* pIndex = (dbase::ODbaseIndex*)xTunnel->getSomething(dbase::ODbaseIndex::getUnoTunnelImplementationId());
        if(pIndex)
        {
            dbase::OIndexIterator* pIter = pIndex->createIterator(NULL,NULL);

            if (pIter)
            {
                sal_uInt32 nRec = pIter->First();
                while (nRec != SQL_COLUMN_NOTFOUND)
                {
                    if (m_aOrderbyAscending[0])
                        m_pFileSet->push_back(nRec);
                    else
                        m_pFileSet->insert(m_pFileSet->begin(),nRec);
                    nRec = pIter->Next();
                }
                m_pFileSet->setFrozen();
                //  m_bFileSetFrozen = sal_True;
                //  if(!bDistinct)
                    //  SetRowCount(pFileSet->count());
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
    return new OFILEAnalyzer();
}
// -----------------------------------------------------------------------------
sal_Int32 ODbaseResultSet::getCurrentFilePos() const
{
    return m_pTable->getFilePos();
}
// -----------------------------------------------------------------------------


