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

#include "connectiontools.hxx"
#include "tablename.hxx"
#include "objectnames.hxx"
#include "datasourcemetadata.hxx"

#include <comphelper/namedvaluecollection.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/statementcomposer.hxx>

#include <algorithm>

extern "C" void SAL_CALL createRegistryInfo_ConnectionTools()
{
    ::sdbtools::OAutoRegistration< ::sdbtools::ConnectionTools > aRegistration;
}

namespace sdbtools
{

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

    // ConnectionTools
    ConnectionTools::ConnectionTools( const Reference<XComponentContext>& _rContext )
        :ConnectionDependentComponent( _rContext )
    {
    }

    ConnectionTools::~ConnectionTools()
    {
    }

    Reference< XTableName > SAL_CALL ConnectionTools::createTableName() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return new TableName( getContext(), getConnection() );
    }

    Reference< XObjectNames > SAL_CALL ConnectionTools::getObjectNames() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return new ObjectNames( getContext(), getConnection() );
    }

    Reference< XDataSourceMetaData > SAL_CALL ConnectionTools::getDataSourceMetaData() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return new DataSourceMetaData( getContext(), getConnection() );
    }
    Reference< container::XNameAccess > SAL_CALL ConnectionTools::getFieldsByCommandDescriptor( ::sal_Int32 commandType, const OUString& command, Reference< lang::XComponent >& keepFieldsAlive ) throw (sdbc::SQLException, RuntimeException)
    {
        EntryGuard aGuard( *this );
        ::dbtools::SQLExceptionInfo aErrorInfo;
        Reference< container::XNameAccess > xRet = ::dbtools::getFieldsByCommandDescriptor(getConnection(),commandType,command,keepFieldsAlive,&aErrorInfo);
        if ( aErrorInfo.isValid() )
            aErrorInfo.doThrow();
        return xRet;
    }
    Reference< sdb::XSingleSelectQueryComposer > SAL_CALL ConnectionTools::getComposer( ::sal_Int32 commandType, const OUString& command ) throw (::com::sun::star::uno::RuntimeException)
    {
        EntryGuard aGuard( *this );
        dbtools::StatementComposer aComposer(getConnection(), command, commandType, sal_True );
        aComposer.setDisposeComposer(sal_False);
        return aComposer.getComposer();
    }

    OUString SAL_CALL ConnectionTools::getImplementationName() throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    ::sal_Bool SAL_CALL ConnectionTools::supportsService(const OUString & _ServiceName) throw (RuntimeException)
    {
        Sequence< OUString > aSupported( getSupportedServiceNames() );
        const OUString* begin = aSupported.getConstArray();
        const OUString* end = aSupported.getConstArray() + aSupported.getLength();
        return ::std::find( begin, end, _ServiceName ) != end;
    }

    Sequence< OUString > SAL_CALL ConnectionTools::getSupportedServiceNames() throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    OUString SAL_CALL ConnectionTools::getImplementationName_static()
    {
        return OUString( "com.sun.star.comp.dbaccess.ConnectionTools" );
    }

    Sequence< OUString > SAL_CALL ConnectionTools::getSupportedServiceNames_static()
    {
        Sequence< OUString > aSupported( 1 );
        aSupported[0] = OUString( "com.sun.star.sdb.tools.ConnectionTools" );
        return aSupported;
    }

    Reference< XInterface > SAL_CALL ConnectionTools::Create(const Reference< XComponentContext >& _rxContext )
    {
        return *( new ConnectionTools( Reference<XComponentContext>( _rxContext ) ) );
    }

    void SAL_CALL ConnectionTools::initialize(const Sequence< Any > & _rArguments) throw (RuntimeException, Exception)
    {
        ::osl::MutexGuard aGuard( getMutex() );

        Reference< XConnection > xConnection;
        if (_rArguments.getLength()==1 && (_rArguments[0] >>= xConnection))
        {
        }
        else
        {
            ::comphelper::NamedValueCollection aArguments( _rArguments );
            aArguments.get( "Connection" ) >>= xConnection;
        }
        if ( !xConnection.is() )
            throw IllegalArgumentException();

        setWeakConnection( xConnection );
    }

} // namespace sdbtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
