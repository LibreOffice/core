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
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>

#include "pq_xview.hxx"
#include "pq_xviews.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;
using osl::Mutex;

using com::sun::star::container::XNameAccess;
using com::sun::star::container::XIndexAccess;
using com::sun::star::container::ElementExistException;
using com::sun::star::container::NoSuchElementException;

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;

using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::IndexOutOfBoundsException;

using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::XFastPropertySet;
using com::sun::star::beans::XMultiPropertySet;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::Property;

using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

View::View( const ::rtl::Reference< RefCountedMutex > & refMutex,
            const Reference< com::sun::star::sdbc::XConnection > & connection,
            ConnectionSettings *pSettings)
    : ReflectionBase(
        getStatics().refl.view.implName,
        getStatics().refl.view.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.view.pProps )
{}

Reference< XPropertySet > View::createDataDescriptor(  ) throw (RuntimeException)
{
    ViewDescriptor * pView = new ViewDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pView->copyValuesFrom( this );

    return Reference< XPropertySet > ( pView );
}

void View::rename( const OUString& newName )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::container::ElementExistException,
               ::com::sun::star::uno::RuntimeException)
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
        catch( com::sun::star::sdbc::SQLException &e )
        {
            OUString buf( e.Message + "(NOTE: Only postgresql server >= V8.1 support changing a table's schema)" );
            e.Message = buf;
            throw;
        }

    }
    if( ! oldName.equals( newTableName ) )
    {
        OUStringBuffer buf(128);
        buf.appendAscii( "ALTER TABLE" );
        bufferQuoteQualifiedIdentifier( buf, schema, oldName, m_pSettings );
        buf.appendAscii( "RENAME TO" );
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

Sequence<Type > View::getTypes() throw( RuntimeException )
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( (Reference< com::sun::star::sdbcx::XRename> *) 0 ),
                ReflectionBase::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();
}

Sequence< sal_Int8> View::getImplementationId() throw( RuntimeException )
{
    return getStatics().refl.view.implementationId;
}

Any View::queryInterface( const Type & reqType ) throw (RuntimeException)
{
    Any ret;

    ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< com::sun::star::sdbcx::XRename * > ( this )
            );
    return ret;
}

OUString View::getName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    Statics & st = getStatics();
    return concatQualified(
        extractStringProperty( this, st.SCHEMA_NAME ),
        extractStringProperty( this, st.NAME ) );
}

void View::setName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException)
{
    rename( aName );
}

//____________________________________________________________________________________________

ViewDescriptor::ViewDescriptor(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const Reference< com::sun::star::sdbc::XConnection > & connection,
    ConnectionSettings *pSettings)
    : ReflectionBase(
        getStatics().refl.viewDescriptor.implName,
        getStatics().refl.viewDescriptor.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.viewDescriptor.pProps )
{}

Reference< XPropertySet > ViewDescriptor::createDataDescriptor(  ) throw (RuntimeException)
{
    ViewDescriptor * pView = new ViewDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pView->copyValuesFrom( this );

    return Reference< XPropertySet > ( pView );
}


}
