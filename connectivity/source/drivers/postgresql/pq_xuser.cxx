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

#include "pq_xuser.hxx"
#include "pq_tools.hxx"
#include "pq_statics.hxx"

using osl::MutexGuard;
using osl::Mutex;

using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;

using com::sun::star::beans::XPropertySet;

using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

User::User( const ::rtl::Reference< RefCountedMutex > & refMutex,
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
    UserDescriptor * pUser = new UserDescriptor( m_refMutex, m_conn, m_pSettings );
    pUser->copyValuesFrom( this );

    return Reference< XPropertySet > ( pUser );
}


Sequence<Type > User::getTypes()
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<css::sdbcx::XUser>::get(),
                ReflectionBase::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();
}

Sequence< sal_Int8> User::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Any User::queryInterface( const Type & reqType )
{
    Any ret;

    ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< css::sdbcx::XUser * > ( this ) );
    return ret;
}


void User::changePassword(
    const OUString& oldPassword, const OUString& newPassword )
{
    (void) oldPassword;
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
    sal_Int32 ret = 0xffffffff;
    if (isLog(m_pSettings, LogLevel::Info))
    {
        Statics & st = getStatics();

        OUStringBuffer buf( 128 );
        buf.append( "User::getPrivileges[" + extractStringProperty( this, st.NAME ) +
                    "] got called for " + objName + "(type=" +
                    OUString::number(objType) + ")");
        log(m_pSettings, LogLevel::Info, buf.makeStringAndClear());
    }
    // all privileges
    return ret;
}

sal_Int32 User::getGrantablePrivileges( const OUString& objName, sal_Int32 objType )
{
    (void) objName; (void) objType;
    // all privileges
    return 0xffffffff;
}

void User::grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges )
{
    (void) objName; (void) objType; (void) objPrivileges;
    throw css::sdbc::SQLException("pq_driver: privilege change not implemented yet",
                                             *this, OUString(), 1, Any() );
}

void User::revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges )
{
    (void) objName; (void) objType; (void) objPrivileges;
    throw css::sdbc::SQLException("pq_driver: privilege change not implemented yet",
                                             *this, OUString(), 1, Any() );
}


UserDescriptor::UserDescriptor(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
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
    UserDescriptor * pUser = new UserDescriptor( m_refMutex, m_conn, m_pSettings );
    pUser->copyValuesFrom( this );

    return Reference< XPropertySet > ( pUser );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
