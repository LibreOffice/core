/*************************************************************************
 *
 *  $RCSfile: FStatement.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-09 12:34:19 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_FILE_OSTATEMENT_HXX_
#include "file/FStatement.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_OCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_ORESULTSET_HXX_
#include "file/FResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#define THROW_SQL(x) \
    OTools::ThrowException(x,m_aStatementHandle,SQL_HANDLE_STMT,*this)

using namespace connectivity::file;
using namespace connectivity;
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
//------------------------------------------------------------------------------
OStatement_Base::OStatement_Base(OConnection* _pConnection ) :  OStatement_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer(OStatement_BASE::rBHelper)
    ,rBHelper(OStatement_BASE::rBHelper)
    ,m_pConnection(_pConnection)
    ,m_pParseTree(NULL)
    ,m_aSQLIterator(_pConnection->createCatalog()->getTables(),_pConnection->getMetaData(),NULL)
{
    m_pConnection->acquire();

    sal_Int32 nAttrib = 0;

    registerProperty(PROPERTY_CURSORNAME,       PROPERTY_ID_CURSORNAME,         nAttrib,&m_aCursorName,     ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_MAXFIELDSIZE,     PROPERTY_ID_MAXFIELDSIZE,       nAttrib,&m_nMaxFieldSize,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_MAXROWS,          PROPERTY_ID_MAXROWS,            nAttrib,&m_nMaxRows,        ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_QUERYTIMEOUT,     PROPERTY_ID_QUERYTIMEOUT,       nAttrib,&m_nQueryTimeOut,   ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_FETCHSIZE,        PROPERTY_ID_FETCHSIZE,          nAttrib,&m_nFetchSize,      ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_RESULTSETTYPE,    PROPERTY_ID_RESULTSETTYPE,      nAttrib,&m_nResultSetType,  ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_FETCHDIRECTION,   PROPERTY_ID_FETCHDIRECTION,     nAttrib,&m_nFetchDirection, ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_ESCAPEPROCESSING, PROPERTY_ID_ESCAPEPROCESSING,   nAttrib,&m_bEscapeProcessing,::getCppuBooleanType());

    registerProperty(PROPERTY_RESULTSETCONCURRENCY,     PROPERTY_ID_RESULTSETCONCURRENCY,   nAttrib,&m_nResultSetConcurrency,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
}
//------------------------------------------------------------------------------
void OStatement_Base::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet = Reference< XResultSet>();
}
//------------------------------------------------------------------------------
void OStatement_BASE2::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    disposeResultSet();

    if (m_pConnection)
        m_pConnection->release();

    dispose_ChildImpl();
    OStatement_Base::disposing();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::release() throw(RuntimeException)
{
    relase_ChildImpl();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
        Any aRet = OStatement_BASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OStatement_Base::getTypes(  ) throw(RuntimeException)
{
        ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                                                        ::getCppuType( (const Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                                                        ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (OStatement_BASE::rBHelper.bDisposed)
                        throw DisposedException();
    }
    dispose();
}
// -------------------------------------------------------------------------

void OStatement_Base::reset() throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
                throw DisposedException();

    clearWarnings ();

    if (m_xResultSet.get().is())
        clearMyResultSet();
}
//--------------------------------------------------------------------
// clearMyResultSet
// If a ResultSet was created for this Statement, close it
//--------------------------------------------------------------------

void OStatement_Base::clearMyResultSet () throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference<XCloseable> xCloseable;
    if(::utl::query_interface(m_xResultSet.get(),xCloseable))
        xCloseable->close();
    m_xResultSet = Reference< XResultSet>();
}
//--------------------------------------------------------------------
// setWarning
// Sets the warning
//--------------------------------------------------------------------

void OStatement_Base::setWarning (const SQLWarning &ex) throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_aLastWarning = ex;
}

// -------------------------------------------------------------------------

sal_Bool SAL_CALL OStatement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    //  ::rtl::OString aSql(::rtl::OUStringToOString(sql,osl_getThreadTextEncoding()));
        Reference< XResultSet > xRS = executeQuery(sql);

    return m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT || m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT_COUNT;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XResultSet > xRS = NULL;

    String aErr;
    m_pParseTree = m_aParser.parseTree(aErr,sql);
    if(m_pParseTree)
    {
        m_aSQLIterator.setParseTree(m_pParseTree);
        m_aSQLIterator.traverseAll();
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if(xTabs.begin() == xTabs.end())
            throw SQLException(aErr,*this,::rtl::OUString(),0,Any());

        OResultSet* pResult = createResultSet();
        pResult->OpenImpl();
        xRS = pResult;

    }
    else
        throw SQLException(aErr,*this,::rtl::OUString(),0,Any());

    // Execute the statement.  If execute returns true, a result
    // set exists.
    return xRS;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OStatement_Base::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XResultSet > xRS = NULL;

    String aErr;
    m_pParseTree = m_aParser.parseTree(aErr,sql);
    if(m_pParseTree)
    {
        m_aSQLIterator.setParseTree(m_pParseTree);
        m_aSQLIterator.traverseAll();
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if(xTabs.begin() == xTabs.end())
            throw SQLException(aErr,*this,::rtl::OUString(),0,Any());

        OResultSet* pResult = createResultSet();
        pResult->OpenImpl();
        xRS = pResult;
        return pResult->getRowCountResult();
    }
    else
        throw SQLException(aErr,*this,::rtl::OUString(),0,Any());

    return 0;
}
// -------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    return *const_cast<OStatement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
OResultSet* OStatement::createResultSet()
{
    return new OResultSet(this,m_aSQLIterator);
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbc.driver.file.Statement","com.sun.star.sdbc.Statement");

