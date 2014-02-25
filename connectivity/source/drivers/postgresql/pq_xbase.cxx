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
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

#include "pq_tools.hxx"
#include "pq_xbase.hxx"

using osl::MutexGuard;

using com::sun::star::uno::Any;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;

using com::sun::star::beans::Property;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::XMultiPropertySet;
using com::sun::star::beans::XFastPropertySet;
using com::sun::star::beans::XPropertySet;

namespace pq_sdbc_driver
{

ReflectionBase::ReflectionBase(
    const OUString &implName,
    const ::com::sun::star::uno::Sequence< OUString > &supportedServices,
    const ::rtl::Reference< RefCountedMutex > refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &conn,
    ConnectionSettings *pSettings,
    cppu::IPropertyArrayHelper & props /* must survive this object !*/ )
    : OComponentHelper( refMutex->mutex ),
      OPropertySetHelper( OComponentHelper::rBHelper ),
      m_implName( implName ),
      m_supportedServices( supportedServices ),
      m_refMutex( refMutex ),
      m_conn( conn ),
      m_pSettings( pSettings ),
      m_propsDesc( props ),
      m_values( props.getProperties().getLength() )
{}

cppu::IPropertyArrayHelper & ReflectionBase::getInfoHelper()
{
    return m_propsDesc;
}

sal_Bool ReflectionBase::convertFastPropertyValue(
    ::com::sun::star::uno::Any & rConvertedValue,
    ::com::sun::star::uno::Any & rOldValue,
    sal_Int32 nHandle,
    const ::com::sun::star::uno::Any& rValue )
    throw (::com::sun::star::lang::IllegalArgumentException)
{

    rOldValue = m_values[nHandle];
    rConvertedValue = rValue;     // TODO !!! implement correct conversion !
    m_values[nHandle] = rValue;
    return sal_True;
}

void ReflectionBase::setPropertyValue_NoBroadcast_public(
    const OUString & name, const com::sun::star::uno::Any & value )
{
    sal_Int32 nHandle = m_propsDesc.getHandleByName( name );
    if( -1 == nHandle  )
    {
        OUStringBuffer buf(128);
        buf.appendAscii( "Unknown property '" );
        buf.append( name );
        buf.appendAscii( "' in " );
        buf.append( m_implName );
        throw com::sun::star::uno::RuntimeException( buf.makeStringAndClear() , *this );
    }
    setFastPropertyValue_NoBroadcast( nHandle , value );
}

void ReflectionBase::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle,
    const ::com::sun::star::uno::Any& rValue )
    throw (::com::sun::star::uno::Exception, std::exception)
{
//     OUString s;
//     rValue >>= s;
//     printf( "setting value (handle %d):%s\n" ,
//             nHandle, OUStringToOString(s, RTL_TEXTENCODING_ASCII_US).getStr() );
    m_values[nHandle] = rValue;
}

void ReflectionBase::getFastPropertyValue(
    ::com::sun::star::uno::Any& rValue,
    sal_Int32 nHandle ) const
{
    rValue = m_values[nHandle];
//     OUString s;
//     rValue >>= s;
//     printf( "getting value (handle %d):%s\n" ,
//             nHandle, OUStringToOString(s, RTL_TEXTENCODING_ASCII_US).getStr() );

}

Reference < ::com::sun::star::beans::XPropertySetInfo >  ReflectionBase::getPropertySetInfo()
        throw(com::sun::star::uno::RuntimeException, std::exception)
{
    return OPropertySetHelper::createPropertySetInfo( m_propsDesc );
}

OUString ReflectionBase::getImplementationName()
        throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_implName;
}

sal_Bool ReflectionBase::supportsService(const OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > ReflectionBase::getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_supportedServices;
}


Sequence< com::sun::star::uno::Type > ReflectionBase::getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception )
{
    osl::MutexGuard guard( m_refMutex->mutex );
    static cppu::OTypeCollection collection(
        getCppuType( (Reference< XPropertySet >*) 0 ),
        getCppuType( (Reference< XFastPropertySet > *) 0 ),
        getCppuType( (Reference< XMultiPropertySet > *) 0 ),
        getCppuType( (Reference< com::sun::star::lang::XServiceInfo > *) 0 ),
        getCppuType( (Reference< com::sun::star::sdbcx::XDataDescriptorFactory > *) 0 ),
        getCppuType( (Reference< com::sun::star::container::XNamed > *) 0 ),
        OComponentHelper::getTypes());
    return collection.getTypes();
}


com::sun::star::uno::Any ReflectionBase::queryInterface(
    const com::sun::star::uno::Type & reqType )
    throw (com::sun::star::uno::RuntimeException, std::exception)
{
    Any ret;
    ret = OComponentHelper::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< com::sun::star::beans::XPropertySet * > ( this ),
            static_cast< com::sun::star::beans::XMultiPropertySet * > ( this ),
            static_cast< com::sun::star::lang::XServiceInfo * > ( this ),
            static_cast< com::sun::star::beans::XFastPropertySet * > ( this ) ,
            static_cast< com::sun::star::sdbcx::XDataDescriptorFactory * > ( this ),
            static_cast< com::sun::star::container::XNamed * > ( this ) );
    return ret;

}

Sequence< sal_Int8> ReflectionBase::getImplementationId() throw( RuntimeException, std::exception )
{
    static cppu::OImplementationId *pId;
    if( ! pId )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static cppu::OImplementationId id(false);
            pId = &id;
        }
    }
    return pId->getImplementationId();
}

void ReflectionBase::copyValuesFrom( const Reference< XPropertySet > & set )
{
    Reference< XPropertySetInfo > info = set->getPropertySetInfo();
    if( info.is () )
    {
        Reference< XPropertySetInfo > myPropInfo = getPropertySetInfo();

        Sequence< Property > props = info->getProperties();
        for( int i = 0 ; i < props.getLength() ; i ++ )
        {
            if( myPropInfo->hasPropertyByName( props[i].Name ) )
                setPropertyValue_NoBroadcast_public(
                    props[i].Name, set->getPropertyValue( props[i].Name ) );
        }
    }
}

OUString ReflectionBase::getName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    Statics & st = getStatics();
    if( getInfoHelper().hasPropertyByName( st.SCHEMA_NAME ) )
        return concatQualified(
            extractStringProperty( this, getStatics().SCHEMA_NAME  ),
            extractStringProperty( this, getStatics().NAME  ) );
    else
        return extractStringProperty( this, getStatics().NAME );
}


void ReflectionBase::setName( const OUString& /* aName */ )
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    throw RuntimeException(
        OUString( "pq_sdbc::ReflectionBase::setName not implemented" ),
        *this );
    //setPropertyValue( getStatics().NAME , makeAny( aName ) );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
