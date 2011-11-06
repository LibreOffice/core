/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBACCESS_CONNECTIONTOOLS_HXX
#include "connectiontools.hxx"
#endif

#ifndef DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX
#include "tablename.hxx"
#endif
#ifndef DBACCESS_SOURCE_SDBTOOLS_CONNECTION_OBJECTNAMES_HXX
#include "objectnames.hxx"
#endif
#ifndef DBACCESS_DATASOURCEMETADATA_HXX
#include "datasourcemetadata.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef COMPHELPER_NAMEDVALUECOLLECTION_HXX
#include <comphelper/namedvaluecollection.hxx>
#endif

#include <connectivity/dbtools.hxx>
#include <connectivity/statementcomposer.hxx>

#include <algorithm>

extern "C" void SAL_CALL createRegistryInfo_ConnectionTools()
{
    ::sdbtools::OAutoRegistration< ::sdbtools::ConnectionTools > aRegistration;
}

//........................................................................
namespace sdbtools
{
//........................................................................

    /** === begin UNO using === **/
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::sdb::tools::XTableName;
    using ::com::sun::star::sdb::tools::XObjectNames;
    using ::com::sun::star::sdb::tools::XDataSourceMetaData;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::XComponentContext;
    /** === end UNO using === **/

    //====================================================================
    //= ConnectionTools
    //====================================================================
    //--------------------------------------------------------------------
    ConnectionTools::ConnectionTools( const ::comphelper::ComponentContext& _rContext )
        :ConnectionDependentComponent( _rContext )
    {
    }

    //--------------------------------------------------------------------
    ConnectionTools::~ConnectionTools()
    {
    }

    //--------------------------------------------------------------------
    Reference< XTableName > SAL_CALL ConnectionTools::createTableName() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return new TableName( getContext(), getConnection() );
    }

    //--------------------------------------------------------------------
    Reference< XObjectNames > SAL_CALL ConnectionTools::getObjectNames() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return new ObjectNames( getContext(), getConnection() );
    }

    //--------------------------------------------------------------------
    Reference< XDataSourceMetaData > SAL_CALL ConnectionTools::getDataSourceMetaData() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return new DataSourceMetaData( getContext(), getConnection() );
    }
    //--------------------------------------------------------------------
    Reference< container::XNameAccess > SAL_CALL ConnectionTools::getFieldsByCommandDescriptor( ::sal_Int32 commandType, const ::rtl::OUString& command, Reference< lang::XComponent >& keepFieldsAlive ) throw (sdbc::SQLException, RuntimeException)
    {
        EntryGuard aGuard( *this );
        ::dbtools::SQLExceptionInfo aErrorInfo;
        Reference< container::XNameAccess > xRet = ::dbtools::getFieldsByCommandDescriptor(getConnection(),commandType,command,keepFieldsAlive,&aErrorInfo);
        if ( aErrorInfo.isValid() )
            aErrorInfo.doThrow();
        return xRet;
    }
    //--------------------------------------------------------------------
    Reference< sdb::XSingleSelectQueryComposer > SAL_CALL ConnectionTools::getComposer( ::sal_Int32 commandType, const ::rtl::OUString& command ) throw (::com::sun::star::uno::RuntimeException)
    {
        EntryGuard aGuard( *this );
        dbtools::StatementComposer aComposer(getConnection(), command, commandType, sal_True );
        aComposer.setDisposeComposer(sal_False);
        return aComposer.getComposer();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ConnectionTools::getImplementationName() throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL ConnectionTools::supportsService(const ::rtl::OUString & _ServiceName) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( getSupportedServiceNames() );
        const ::rtl::OUString* begin = aSupported.getConstArray();
        const ::rtl::OUString* end = aSupported.getConstArray() + aSupported.getLength();
        return ::std::find( begin, end, _ServiceName ) != end;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ConnectionTools::getSupportedServiceNames() throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ConnectionTools::getImplementationName_static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.dbaccess.ConnectionTools" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ConnectionTools::getSupportedServiceNames_static()
    {
        Sequence< ::rtl::OUString > aSupported( 1 );
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.tools.ConnectionTools" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ConnectionTools::Create(const Reference< XComponentContext >& _rxContext )
    {
        return *( new ConnectionTools( ::comphelper::ComponentContext( _rxContext ) ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ConnectionTools::initialize(const Sequence< Any > & _rArguments) throw (RuntimeException, Exception)
    {
        ::osl::MutexGuard aGuard( getMutex() );

        ::comphelper::NamedValueCollection aArguments( _rArguments );

        Reference< XConnection > xConnection;
        aArguments.get( "Connection" ) >>= xConnection;
        if ( !xConnection.is() )
            throw IllegalArgumentException();

        setWeakConnection( xConnection );
    }


//........................................................................
} // namespace sdbtools
//........................................................................

