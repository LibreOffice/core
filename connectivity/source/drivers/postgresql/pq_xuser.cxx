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

#include <sal/log.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/sdbc/SQLException.hpp>

#include "pq_xuser.hxx"
#include "pq_tools.hxx"
#include "pq_statics.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;

using com::sun::star::beans::XPropertySet;

using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

User::User( const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
            const Reference< css::sdbc::XConnection > & connection,
            ConnectionSettings *pSettings )
    : ReflectionBase(
        getStatics().refl.user.implName,
        getStatics().refl.user.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.user.pProps )
{}

Reference< XPropertySet > User::createDataDescriptor(  )
{
    rtl::Reference<UserDescriptor> pUser = new UserDescriptor( m_xMutex, m_conn, m_pSettings );
    pUser->copyValuesFrom( this );

    return Reference< XPropertySet > ( pUser );
}


Sequence<Type > User::getTypes()
{
    static cppu::OTypeCollection collection(
        cppu::UnoType<css::sdbcx::XUser>::get(),
        ReflectionBase::getTypes());

    return collection.getTypes();
}

Sequence< sal_Int8> User::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Any User::queryInterface( const Type & reqType )
{
    Any ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< css::sdbcx::XUser * > ( this ) );
    return ret;
}


void User::changePassword(
    const OUString&, const OUString& newPassword )
{
    OUStringBuffer buf(128);
    buf.append( "ALTER USER " );
    bufferQuoteIdentifier( buf, extractStringProperty( this, getStatics().NAME ), m_pSettings );
    buf.append( " PASSWORD " );
    bufferQuoteConstant( buf, newPassword, m_pSettings );
    Reference< XStatement > stmt = m_conn->createStatement();
    DisposeGuard guard( stmt );
    stmt->executeUpdate( buf.makeStringAndClear() );
}

sal_Int32 User::getPrivileges( const OUString& objName, sal_Int32 objType )
{
    SAL_INFO("connectivity.postgresql", "User::getPrivileges[\"Name\"] got called for " << objName << "(type=" << objType << ")");
    // all privileges
    return 0xffffffff;
}

sal_Int32 User::getGrantablePrivileges( const OUString&, sal_Int32 )
{
    // all privileges
    return 0xffffffff;
}

void User::grantPrivileges( const OUString&, sal_Int32, sal_Int32 )
{
    throw css::sdbc::SQLException(u"pq_driver: privilege change not implemented yet"_ustr,
                                             *this, OUString(), 1, Any() );
}

void User::revokePrivileges( const OUString&, sal_Int32, sal_Int32 )
{
    throw css::sdbc::SQLException(u"pq_driver: privilege change not implemented yet"_ustr,
                                             *this, OUString(), 1, Any() );
}


UserDescriptor::UserDescriptor(
    const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
    const Reference< css::sdbc::XConnection > & connection,
    ConnectionSettings *pSettings )
    : ReflectionBase(
        getStatics().refl.userDescriptor.implName,
        getStatics().refl.userDescriptor.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.userDescriptor.pProps )
{}

Reference< XPropertySet > UserDescriptor::createDataDescriptor(  )
{
    rtl::Reference<UserDescriptor> pUser = new UserDescriptor( m_xMutex, m_conn, m_pSettings );
    pUser->copyValuesFrom( this );

    return Reference< XPropertySet > ( pUser );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
