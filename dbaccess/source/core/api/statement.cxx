/*************************************************************************
 *
 *  $RCSfile: statement.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-25 07:32:18 $
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
#ifndef _DBA_COREAPI_STATEMENT_HXX_
#include <statement.hxx>
#endif
#ifndef _DBA_COREAPI_RESULTSET_HXX_
#include <resultset.hxx>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace ::com::sun::star::sdbc;
//using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;
using namespace dbtools;

DBG_NAME(OStatementBase);

//--------------------------------------------------------------------------
OStatementBase::OStatementBase(const Reference< XConnection > & _xConn,
                               const Reference< XInterface > & _xStatement)
               :OSubComponent(m_aMutex, _xConn)
               ,OPropertySetHelper(OComponentHelper::rBHelper)
               ,m_bUseBookmarks(sal_False)

{
    DBG_CTOR(OStatementBase, NULL);
    m_xAggregateAsSet = Reference< XPropertySet >(_xStatement, UNO_QUERY);
    m_xAggregateAsCancellable = Reference< ::com::sun::star::util::XCancellable > (m_xAggregateAsSet, UNO_QUERY);
}

//--------------------------------------------------------------------------
OStatementBase::~OStatementBase()
{
    DBG_DTOR(OStatementBase, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OStatementBase::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XWarningsSupplier > *)0 ),
                           ::getCppuType( (const Reference< XCloseable > *)0 ),
                           ::getCppuType( (const Reference< XMultipleResults > *)0 ),
                           ::getCppuType( (const Reference< XPreparedBatchExecution > *)0 ),
                           ::getCppuType( (const Reference< ::com::sun::star::util::XCancellable > *)0 ),
                            OSubComponent::getTypes() );

    return aTypes.getTypes();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OStatementBase::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OSubComponent::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ),
                    static_cast< XWarningsSupplier * >( this ),
                    static_cast< XCloseable * >( this ),
                    static_cast< XPreparedBatchExecution * >( this ),
                    static_cast< XMultipleResults * >( this ),
                    static_cast< ::com::sun::star::util::XCancellable * >( this ));
    return aIface;
}

//--------------------------------------------------------------------------
void OStatementBase::acquire() throw (RuntimeException)
{
    OSubComponent::acquire();
}

//--------------------------------------------------------------------------
void OStatementBase::release() throw (RuntimeException)
{
    OSubComponent::release();
}

//------------------------------------------------------------------------------
void OStatementBase::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_aResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_aResultSet = NULL;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OStatementBase::disposing()
{
    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    // free pending results
    disposeResultSet();

    // free the original statement
    {
        MutexGuard aGuard(m_aCancelMutex);
        m_xAggregateAsCancellable = NULL;
    }

    Reference< XCloseable > (m_xAggregateAsSet, UNO_QUERY)->close();
    m_xAggregateAsSet = NULL;

    // free the parent at last
    OSubComponent::disposing();
}

// XCloseable
//------------------------------------------------------------------------------
void OStatementBase::close(void) throw( SQLException, RuntimeException )
{
    {
        MutexGuard aGuard( m_aMutex );
        if (OComponentHelper::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}

// OPropertySetHelper
//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OStatementBase::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

// comphelper::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatementBase::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(10)
        DECL_PROP0(CURSORNAME,              ::rtl::OUString);
        DECL_PROP0_BOOL(USE_ESCAPE_PROCESSING);
        DECL_PROP0(FETCHDIRECTION,          sal_Int32);
        DECL_PROP0(FETCHSIZE,               sal_Int32);
        DECL_PROP0(MAXFIELDSIZE,            sal_Int32);
        DECL_PROP0(MAXROWS,                 sal_Int32);
        DECL_PROP0(QUERYTIMEOUT,            sal_Int32);
        DECL_PROP0(RESULTSETCONCURRENCY,    sal_Int32);
        DECL_PROP0(RESULTSETTYPE,           sal_Int32);
        DECL_PROP0_BOOL(USEBOOKMARKS);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OStatementBase::getInfoHelper()
{
    return *getArrayHelper();
}

//------------------------------------------------------------------------------
sal_Bool OStatementBase::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue ) throw( IllegalArgumentException  )
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_USEBOOKMARKS:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bUseBookmarks);
            if (bModified && m_xAggregateAsSet->getPropertySetInfo()->hasPropertyByName(PROPERTY_USEBOOKMARKS))
                m_xAggregateAsSet->setPropertyValue(PROPERTY_USEBOOKMARKS, rConvertedValue);
            break;
        default:
        {
            // get the property name
            ::rtl::OUString aPropName;
            sal_Int16 nAttributes;
            getInfoHelper().fillPropertyMembersByHandle(&aPropName, &nAttributes, nHandle);
            OSL_ENSHURE(aPropName.getLength(), "property not found?");

            // now set the value
            m_xAggregateAsSet->setPropertyValue(aPropName, rValue);
        }
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OStatementBase::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
{
    switch (nHandle)
    {
        // the other properties are set in convertFast...
        case PROPERTY_ID_USEBOOKMARKS:
        {
            m_bUseBookmarks = ::comphelper::getBOOL(rValue);
        }   break;
    }
}

//------------------------------------------------------------------------------
void OStatementBase::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_USEBOOKMARKS:
            rValue.setValue(&m_bUseBookmarks, getBooleanCppuType());
            break;
        default:
        {
            // get the property name
            ::rtl::OUString aPropName;
            sal_Int16 nAttributes;
            const_cast<OStatementBase*>(this)->getInfoHelper().
                fillPropertyMembersByHandle(&aPropName, &nAttributes, nHandle);
            OSL_ENSHURE(aPropName.getLength(), "property not found?");
            // now read the value
            rValue = m_xAggregateAsSet->getPropertyValue(aPropName);
        }
    }
}

// XWarningsSupplier
//------------------------------------------------------------------------------
Any OStatementBase::getWarnings(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XWarningsSupplier >(m_xAggregateAsSet, UNO_QUERY)->getWarnings();
}

//------------------------------------------------------------------------------
void OStatementBase::clearWarnings(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XWarningsSupplier >(m_xAggregateAsSet, UNO_QUERY)->clearWarnings();
}

// ::com::sun::star::util::XCancellable
//------------------------------------------------------------------------------
void OStatementBase::cancel(void) throw( RuntimeException )
{
    // no blocking as cancel is typically called from a different thread
    ClearableMutexGuard aGuard(m_aCancelMutex);
    if (m_xAggregateAsCancellable.is())
        m_xAggregateAsCancellable->cancel();
    // else do nothing
}

// XMultipleResults
//------------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OStatementBase::getResultSet(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    // first check the meta data
    if (!Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData()->supportsMultipleResultSets())
        throw FunctionSequenceException(*this);

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getResultSet();
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OStatementBase::getUpdateCount(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    // first check the meta data
    if (!Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData()->supportsMultipleResultSets())
        throw FunctionSequenceException(*this);

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getUpdateCount();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OStatementBase::getMoreResults(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    // first check the meta data
    if (!Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData()->supportsMultipleResultSets())
        throw FunctionSequenceException(*this);

    // free the previous results
    disposeResultSet();

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getMoreResults();
}

// XPreparedBatchExecution
//------------------------------------------------------------------------------
void SAL_CALL OStatementBase::addBatch(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    // first check the meta data
    if (!Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData()->supportsBatchUpdates())
        throw FunctionSequenceException(*this);

    Reference< XPreparedBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->addBatch();
}

//------------------------------------------------------------------------------
void SAL_CALL OStatementBase::clearBatch(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    // first check the meta data
    if (!Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData()->supportsBatchUpdates())
        throw FunctionSequenceException(*this);

    Reference< XPreparedBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->clearBatch();
}

//------------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OStatementBase::executeBatch(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    // first check the meta data
    if (!Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData()->supportsBatchUpdates())
        throw FunctionSequenceException(*this);

    // free the previous results
    disposeResultSet();

    return Reference< XPreparedBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->executeBatch();
}

//************************************************************
//  OStatement
//************************************************************
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OStatement::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XServiceInfo > *)0 ),
                           ::getCppuType( (const Reference< XStatement > *)0 ),
                            OStatementBase::getTypes() );

    return aTypes.getTypes();
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OStatement::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OStatement::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OStatementBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XServiceInfo * >( this ),
                    static_cast< XStatement * >( this ));
    return aIface;
}

//--------------------------------------------------------------------------
void OStatement::acquire() throw (RuntimeException)
{
    OStatementBase::acquire();
}

//--------------------------------------------------------------------------
void OStatement::release() throw (RuntimeException)
{
    OStatementBase::release();
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OStatement::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OStatement");
}

//------------------------------------------------------------------------------
sal_Bool OStatement::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OStatement::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS.getArray()[0] = SERVICE_SDBC_STATEMENT;
    aSNS.getArray()[1] = SERVICE_SDB_STATEMENT;
    return aSNS;
}

// XStatement
//------------------------------------------------------------------------------
Reference< XResultSet > OStatement::executeQuery(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    disposeResultSet();

    Reference< XResultSet > xResultSet;
    Reference< XResultSet > xDrvResultSet = Reference< XStatement >(m_xAggregateAsSet, UNO_QUERY)->executeQuery(sql);
    if (xDrvResultSet.is())
    {
        sal_Bool bCaseSensitive = Reference< XConnection >(m_xParent, UNO_QUERY)->getMetaData()->supportsMixedCaseQuotedIdentifiers();
        xResultSet = new OResultSet(xDrvResultSet, *this, bCaseSensitive);

        // keep the resultset weak
        m_aResultSet = xResultSet;
    }
    return xResultSet;
}

//------------------------------------------------------------------------------
sal_Int32 OStatement::executeUpdate(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    disposeResultSet();

    return Reference< XStatement >(m_xAggregateAsSet, UNO_QUERY)->executeUpdate(sql);
}

//------------------------------------------------------------------------------
sal_Bool OStatement::execute(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    disposeResultSet();
    return Reference< XStatement >(m_xAggregateAsSet, UNO_QUERY)->execute(sql);
}

//------------------------------------------------------------------------------
Reference< XConnection > OStatement::getConnection(void) throw( SQLException, RuntimeException )
{
    return Reference< XConnection > (m_xParent, UNO_QUERY);
}

