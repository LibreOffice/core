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

#include <connectiontools.hxx>
#include "tablename.hxx"
#include "objectnames.hxx"
#include "datasourcemetadata.hxx"

#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/statementcomposer.hxx>

namespace sdbtools
{

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdb::tools::XTableName;
    using ::com::sun::star::sdb::tools::XObjectNames;
    using ::com::sun::star::sdb::tools::XDataSourceMetaData;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Any;
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

    Reference< XTableName > SAL_CALL ConnectionTools::createTableName()
    {
        EntryGuard aGuard( *this );
        return new TableName( getContext(), getConnection() );
    }

    Reference< XObjectNames > SAL_CALL ConnectionTools::getObjectNames()
    {
        EntryGuard aGuard( *this );
        return new ObjectNames( getContext(), getConnection() );
    }

    Reference< XDataSourceMetaData > SAL_CALL ConnectionTools::getDataSourceMetaData()
    {
        EntryGuard aGuard( *this );
        return new DataSourceMetaData( getContext(), getConnection() );
    }
    Reference< container::XNameAccess > SAL_CALL ConnectionTools::getFieldsByCommandDescriptor( ::sal_Int32 commandType, const OUString& command, Reference< lang::XComponent >& keepFieldsAlive )
    {
        EntryGuard aGuard( *this );
        ::dbtools::SQLExceptionInfo aErrorInfo;
        Reference< container::XNameAccess > xRet = ::dbtools::getFieldsByCommandDescriptor(getConnection(),commandType,command,keepFieldsAlive,&aErrorInfo);
        if ( aErrorInfo.isValid() )
            aErrorInfo.doThrow();
        return xRet;
    }
    Reference< sdb::XSingleSelectQueryComposer > SAL_CALL ConnectionTools::getComposer( ::sal_Int32 commandType, const OUString& command )
    {
        EntryGuard aGuard( *this );
        dbtools::StatementComposer aComposer(getConnection(), command, commandType, true );
        aComposer.setDisposeComposer(false);
        return aComposer.getComposer();
    }

    OUString SAL_CALL ConnectionTools::getImplementationName()
    {
        return u"com.sun.star.comp.dbaccess.ConnectionTools"_ustr;
    }

    sal_Bool SAL_CALL ConnectionTools::supportsService(const OUString & ServiceName)
    {
        return cppu::supportsService(this, ServiceName);
    }

    Sequence< OUString > SAL_CALL ConnectionTools::getSupportedServiceNames()
    {
        return  { u"com.sun.star.sdb.tools.ConnectionTools"_ustr };
    }

    void SAL_CALL ConnectionTools::initialize(const Sequence< Any > & _rArguments)
    {
        ::osl::MutexGuard aGuard( getMutex() );

        Reference< XConnection > xConnection;
        if (_rArguments.getLength()==1 && (_rArguments[0] >>= xConnection))
        {
        }
        else
        {
            ::comphelper::NamedValueCollection aArguments( _rArguments );
            aArguments.get( u"Connection"_ustr ) >>= xConnection;
        }
        if ( !xConnection.is() )
            throw IllegalArgumentException();

        setWeakConnection( xConnection );
    }

} // namespace sdbtools

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_dbaccess_ConnectionTools_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new sdbtools::ConnectionTools(context));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
