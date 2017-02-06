/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include "pq_xview.hxx"
#include "pq_xviews.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;
using osl::Mutex;

using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;

using com::sun::star::beans::XPropertySet;

using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

View::View( const ::rtl::Reference< RefCountedMutex > & refMutex,
            const Reference< css::sdbc::XConnection > & connection,
            ConnectionSettings *pSettings)
    : ReflectionBase(
        getStatics().refl.view.implName,
        getStatics().refl.view.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.view.pProps )
{}

Reference< XPropertySet > View::createDataDescriptor(  )
{
    ViewDescriptor * pView = new ViewDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pView->copyValuesFrom( this );

    return Reference< XPropertySet > ( pView );
}

void View::rename( const OUString& newName )
{
    MutexGuard guard( m_refMutex->mutex );

    Statics & st = getStatics();

    OUString oldName = extractStringProperty(this,st.NAME );
    OUString schema = extractStringProperty(this,st.SCHEMA_NAME );
    OUString fullOldName = concatQualified( schema, oldName );

    OUString newTableName;
    OUString newSchemaName;
    // OOo2.0 passes schema + dot + new-table-name while
    // OO1.1.x passes new Name without schema
    // in case name contains a dot, it is interpreted as schema.tablename
    if( newName.indexOf( '.' ) >= 0 )
    {
        splitConcatenatedIdentifier( newName, &newSchemaName, &newTableName );
    }
    else
    {
        newTableName = newName;
        newSchemaName = schema;
    }
    OUString fullNewName = concatQualified( newSchemaName, newTableName );

    if( ! schema.equals( newSchemaName ) )
    {
        try
        {
            OUStringBuffer buf(128);
            buf.append( "ALTER TABLE" );
            bufferQuoteQualifiedIdentifier(buf, schema, oldName, m_pSettings );
            buf.append( "SET SCHEMA" );
            bufferQuoteIdentifier( buf, newSchemaName, m_pSettings );
            Reference< XStatement > statement = m_conn->createStatement();
            statement->executeUpdate( buf.makeStringAndClear() );
            setPropertyValue_NoBroadcast_public( st.SCHEMA_NAME, makeAny(newSchemaName) );
            disposeNoThrow( statement );
            schema = newSchemaName;
        }
        catch( css::sdbc::SQLException &e )
        {
            OUString buf( e.Message + "(NOTE: Only postgresql server >= V8.1 support changing a table's schema)" );
            e.Message = buf;
            throw;
        }

    }
    if( ! oldName.equals( newTableName ) )
    {
        OUStringBuffer buf(128);
        buf.append( "ALTER TABLE" );
        bufferQuoteQualifiedIdentifier( buf, schema, oldName, m_pSettings );
        buf.append( "RENAME TO" );
        bufferQuoteIdentifier( buf, newTableName, m_pSettings );
        Reference< XStatement > statement = m_conn->createStatement();
        statement->executeUpdate( buf.makeStringAndClear() );
        setPropertyValue_NoBroadcast_public( st.NAME, makeAny(newTableName) );
    }

    // inform the container of the name change !
    if( m_pSettings->views.is() )
    {
        m_pSettings->pViewsImpl->rename( fullOldName, fullNewName );
    }
}

Sequence<Type > View::getTypes()
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<css::sdbcx::XRename>::get(),
                ReflectionBase::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();
}

Sequence< sal_Int8> View::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Any View::queryInterface( const Type & reqType )
{
    Any ret;

    ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< css::sdbcx::XRename * > ( this )
            );
    return ret;
}

OUString View::getName(  )
{
    Statics & st = getStatics();
    return concatQualified(
        extractStringProperty( this, st.SCHEMA_NAME ),
        extractStringProperty( this, st.NAME ) );
}

void View::setName( const OUString& aName )
{
    rename( aName );
}


ViewDescriptor::ViewDescriptor(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const Reference< css::sdbc::XConnection > & connection,
    ConnectionSettings *pSettings)
    : ReflectionBase(
        getStatics().refl.viewDescriptor.implName,
        getStatics().refl.viewDescriptor.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.viewDescriptor.pProps )
{}

Reference< XPropertySet > ViewDescriptor::createDataDescriptor(  )
{
    ViewDescriptor * pView = new ViewDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pView->copyValuesFrom( this );

    return Reference< XPropertySet > ( pView );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
