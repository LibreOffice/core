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
    const ::rtl::OUString &implName,
    const ::com::sun::star::uno::Sequence< rtl::OUString > &supportedServices,
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
    const rtl::OUString & name, const com::sun::star::uno::Any & value )
{
    sal_Int32 nHandle = m_propsDesc.getHandleByName( name );
    if( -1 == nHandle  )
    {
        rtl::OUStringBuffer buf(128);
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
    throw (::com::sun::star::uno::Exception)
{
//     rtl::OUString s;
//     rValue >>= s;
//     printf( "setting value (handle %d):%s\n" ,
//             nHandle, rtl::OUStringToOString(s, RTL_TEXTENCODING_ASCII_US).getStr() );
    m_values[nHandle] = rValue;
}

void ReflectionBase::getFastPropertyValue(
    ::com::sun::star::uno::Any& rValue,
    sal_Int32 nHandle ) const
{
    rValue = m_values[nHandle];
//     rtl::OUString s;
//     rValue >>= s;
//     printf( "getting value (handle %d):%s\n" ,
//             nHandle, rtl::OUStringToOString(s, RTL_TEXTENCODING_ASCII_US).getStr() );

}

Reference < ::com::sun::star::beans::XPropertySetInfo >  ReflectionBase::getPropertySetInfo()
        throw(com::sun::star::uno::RuntimeException)
{
    return OPropertySetHelper::createPropertySetInfo( m_propsDesc );
}

rtl::OUString ReflectionBase::getImplementationName()
        throw(::com::sun::star::uno::RuntimeException)
{
    return m_implName;
}

sal_Bool ReflectionBase::supportsService(const rtl::OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException)
{
    for( int i = 0 ; i < m_supportedServices.getLength() ; i ++ )
        if( m_supportedServices[i] == ServiceName )
            return sal_True;
    return sal_False;
}

Sequence< rtl::OUString > ReflectionBase::getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException)
{
    return m_supportedServices;
}


Sequence< com::sun::star::uno::Type > ReflectionBase::getTypes()
        throw( com::sun::star::uno::RuntimeException )
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
    throw (com::sun::star::uno::RuntimeException)
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

Sequence< sal_Int8> ReflectionBase::getImplementationId() throw( RuntimeException )
{
    static cppu::OImplementationId *pId;
    if( ! pId )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static cppu::OImplementationId id(sal_False);
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

::rtl::OUString ReflectionBase::getName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    Statics & st = getStatics();
    if( getInfoHelper().hasPropertyByName( st.SCHEMA_NAME ) )
        return concatQualified(
            extractStringProperty( this, getStatics().SCHEMA_NAME  ),
            extractStringProperty( this, getStatics().NAME  ) );
    else
        return extractStringProperty( this, getStatics().NAME );
}


void ReflectionBase::setName( const ::rtl::OUString& /* aName */ )
    throw (::com::sun::star::uno::RuntimeException)
{
    throw RuntimeException(
        rtl::OUString( "pq_sdbc::ReflectionBase::setName not implemented" ),
        *this );
    //setPropertyValue( getStatics().NAME , makeAny( aName ) );
}


}
