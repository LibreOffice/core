/*************************************************************************
 *
 *  $RCSfile: DResultSet.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-05 14:39:40 $
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
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXITER_HXX_
#include "dbase/DIndexIter.hxx"
#endif

using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace cppu;
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
    registerProperty(PROPERTY_ISBOOKMARKABLE,         PROPERTY_ID_ISBOOKMARKABLE,       PropertyAttribute::READONLY,&m_bBookmarkable,                ::getBooleanCppuType());
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
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;

    return sal_False;
}
// -------------------------------------------------------------------------
Any SAL_CALL ODbaseResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OResultSet::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ODbaseResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
 Sequence<  Type > SAL_CALL ODbaseResultSet::getTypes(  ) throw( RuntimeException)
{
    return ::utl::concatSequences(OResultSet::getTypes(),ODbaseResultSet_BASE::getTypes());
}

// -------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL ODbaseResultSet::getBookmark(  ) throw( SQLException,  RuntimeException)
{
     ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet::rBHelper.bDisposed)
        throw DisposedException();

    return makeAny((sal_Int32)(*m_aRow)[0]);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseResultSet::moveToBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet::rBHelper.bDisposed)
        throw DisposedException();

    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    return Move(OFileTable::FILE_BOOKMARK,connectivity::getINT32(bookmark),sal_True);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet::rBHelper.bDisposed)
        throw DisposedException();

    m_bRowDeleted = m_bRowInserted = m_bRowUpdated = sal_False;

    return Move(OFileTable::FILE_BOOKMARK,connectivity::getINT32(bookmark)+rows,sal_True);
}

// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseResultSet::compareBookmarks( const  Any& first, const  Any& second ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet::rBHelper.bDisposed)
        throw DisposedException();

    return (first == second) ? 0 : 2;
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
    if (OResultSet::rBHelper.bDisposed)
        throw DisposedException();

    return connectivity::getINT32(bookmark);
}
// -------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL ODbaseResultSet::deleteRows( const  Sequence<  Any >& rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet::rBHelper.bDisposed)
        throw DisposedException();

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
                    if (bOrderbyAscending[0])
                        m_pFileSet->push_back(nRec);
                    else
                        m_pFileSet->insert(m_pFileSet->begin(),nRec);
                    nRec = pIter->Next();
                }
                m_pFileSet->setFrozen();
                //  m_bFileSetFrozen = sal_True;
                //  if(!bDistinct)
                    //  SetRowCount(pFileSet->count());
                return sal_True;
            }
            delete pIter;
        }
    }
    return sal_False;
}

