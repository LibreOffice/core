/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "FResultSet.hxx"
#include "StatementCommonBase.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <propertyids.hxx>
#include <TConnection.hxx>

using namespace ::connectivity::firebird;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::std;

OStatement_Base::OStatement_Base(OConnection* _pConnection)
    : OStatement_BASE(_pConnection->getMutex()),
      OPropertySetHelper(OStatement_BASE::rBHelper),
      m_pConnection(_pConnection),
      rBHelper(OStatement_BASE::rBHelper)
{
    m_pConnection->acquire();
}

OStatement_Base::~OStatement_Base()
{
}

void OStatement_Base::disposeResultSet()
{
    //free the cursor if alive
//     uno::Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
//     if (xComp.is())
//         xComp->dispose();
//     m_xResultSet = uno::Reference< XResultSet>();
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
    ::cppu::OTypeCollection aTypes(
        ::cppu::UnoType< uno::Reference< XMultiPropertySet > >::get(),
        ::cppu::UnoType< uno::Reference< XFastPropertySet > >::get(),
        ::cppu::UnoType< uno::Reference< XPropertySet > >::get());

    return concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    // cancel the current sql statement
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "close().");

    {
        MutexGuard aGuard(m_pConnection->getMutex());
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}


OUString OStatement_Base::sanitizeSqlString(const OUString& sqlIn)
{
    // TODO: verify this is all we need.
    static const sal_Unicode pattern('"');
    static const sal_Unicode empty(' ');
    return sqlIn.replace(pattern, empty);
}

int OStatement_Base::prepareAndDescribeStatement(const OUString& sqlIn,
                                                 isc_stmt_handle& aStatementHandle,
                                                 XSQLDA*& pOutSqlda,
                                                 XSQLDA* pInSqlda)
{
    MutexGuard aGuard(m_pConnection->getMutex());

    const OUString sql = sanitizeSqlString(sqlIn);

    if (!pOutSqlda)
    {
        pOutSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(10));
        pOutSqlda->version = SQLDA_VERSION1;
        pOutSqlda->sqln = 10;
    }

    int aErr = 0;

    aErr = isc_dsql_allocate_statement(m_statusVector,
                                       &m_pConnection->getDBHandle(),
                                       &aStatementHandle);

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_allocate_statement failed");
        return aErr;
    }
    else
    {
        aErr = isc_dsql_prepare(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &aStatementHandle,
                                0,
                                OUStringToOString(sql, RTL_TEXTENCODING_UTF8).getStr(),
                                1,
                                pInSqlda);
    }

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_prepare failed");
        return aErr;
    }
    else
    {
        aErr = isc_dsql_describe(m_statusVector,
                                 &aStatementHandle,
                                 1,
                                 pOutSqlda);
    }

    // Ensure we have enough space in pOutSqlda
    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_describe failed");
        return aErr;
    }
    else if (!aErr && (pOutSqlda->sqld > pOutSqlda->sqln))
    {
        int n = pOutSqlda->sqld;
        free(pOutSqlda);
        pOutSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(n));
        pOutSqlda->version = SQLDA_VERSION1;
        aErr = isc_dsql_describe(m_statusVector,
                                 &aStatementHandle,
                                 1,
                                 pOutSqlda);
    }

    // Process each XSQLVAR parameter structure in the output XSQLDA
    if (aErr)
    {
        SAL_WARN("connectivity.firebird","isc_dsql_describe failed when resizing pOutSqlda");
        return aErr;
    }
    else
    {
        mallocSQLVAR(pOutSqlda);
    }

    return aErr;
}

// ---- XMultipleResults - UNSUPPORTED ----------------------------------------
uno::Reference< XResultSet > SAL_CALL OStatement_Base::getResultSet() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
    return uno::Reference< XResultSet >();
//     MutexGuard aGuard( m_aMutex );
//     checkDisposed(OStatement_BASE::rBHelper.bDisposed);

//     return m_xResultSet;
}

sal_Bool SAL_CALL OStatement_Base::getMoreResults() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
    return sal_False;
//     MutexGuard aGuard( m_aMutex );
//     checkDisposed(OStatement_BASE::rBHelper.bDisposed);
}

sal_Int32 SAL_CALL OStatement_Base::getUpdateCount() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
    return 0;
}


// ---- XWarningsSupplier - UNSUPPORTED ----------------------------------------
Any SAL_CALL OStatement_Base::getWarnings() throw(SQLException, RuntimeException)
{
    return Any();
}

void SAL_CALL OStatement_Base::clearWarnings() throw(SQLException, RuntimeException)
{
}

::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    // this properties are define by the service statement
    // they must in alphabetic order
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  OUString);
    DECL_BOOL_PROP0(ESCAPEPROCESSING);
    DECL_PROP0(FETCHDIRECTION,sal_Int32);
    DECL_PROP0(FETCHSIZE,   sal_Int32);
    DECL_PROP0(MAXFIELDSIZE,sal_Int32);
    DECL_PROP0(MAXROWS,     sal_Int32);
    DECL_PROP0(QUERYTIMEOUT,sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,sal_Int32);
    DECL_BOOL_PROP0(USEBOOKMARKS);

    return new ::cppu::OPropertyArrayHelper(aProps);
}


::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    return *const_cast<OStatement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
    throw (IllegalArgumentException)
{
    (void) rConvertedValue;
    (void) rOldValue;
    (void) nHandle;
    (void) rValue;
    sal_Bool bConverted = sal_False;
    // here we have to try to convert
    return bConverted;
}
// -------------------------------------------------------------------------
void OStatement_Base::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    (void) rValue;
    // set the value to what ever is necessary
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OStatement_Base::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    (void) rValue;
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}

void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}

void SAL_CALL OStatement_Base::release() throw()
{
    OStatement_BASE::release();
}

uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
