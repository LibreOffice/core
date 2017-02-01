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
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/sequence.hxx>

#include "pq_tools.hxx"
#include "pq_xbase.hxx"

using osl::MutexGuard;

using com::sun::star::uno::Any;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;

using com::sun::star::beans::Property;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::XPropertySet;

namespace pq_sdbc_driver
{

ReflectionBase::ReflectionBase(
    const OUString &implName,
    const css::uno::Sequence< OUString > &supportedServices,
    const ::rtl::Reference< RefCountedMutex >& refMutex,
    const css::uno::Reference< css::sdbc::XConnection > &conn,
    ConnectionSettings *pSettings,
    cppu::IPropertyArrayHelper & props /* must survive this object !*/ )
    : ReflectionBase_BASE( refMutex->mutex ),
      OPropertySetHelper( ReflectionBase_BASE::rBHelper ),
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
    css::uno::Any & rConvertedValue,
    css::uno::Any & rOldValue,
    sal_Int32 nHandle,
    const css::uno::Any& rValue )
{

    rOldValue = m_values[nHandle];
    rConvertedValue = rValue;     // TODO !!! implement correct conversion !
    m_values[nHandle] = rValue;
    return true;
}

void ReflectionBase::setPropertyValue_NoBroadcast_public(
    const OUString & name, const css::uno::Any & value )
{
    sal_Int32 nHandle = m_propsDesc.getHandleByName( name );
    if( -1 == nHandle  )
    {
        throw css::uno::RuntimeException(
            "Unknown property '" + name + "' in " + m_implName,
            *this );
    }
    setFastPropertyValue_NoBroadcast( nHandle , value );
}

void ReflectionBase::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle,
    const css::uno::Any& rValue )
{
//     OUString s;
//     rValue >>= s;
//     printf( "setting value (handle %d):%s\n" ,
//             nHandle, OUStringToOString(s, RTL_TEXTENCODING_ASCII_US).getStr() );
    m_values[nHandle] = rValue;
}

void ReflectionBase::getFastPropertyValue(
    css::uno::Any& rValue,
    sal_Int32 nHandle ) const
{
    rValue = m_values[nHandle];
//     OUString s;
//     rValue >>= s;
//     printf( "getting value (handle %d):%s\n" ,
//             nHandle, OUStringToOString(s, RTL_TEXTENCODING_ASCII_US).getStr() );

}

Reference < css::beans::XPropertySetInfo >  ReflectionBase::getPropertySetInfo()
{
    return OPropertySetHelper::createPropertySetInfo( m_propsDesc );
}

OUString ReflectionBase::getImplementationName()
{
    return m_implName;
}

sal_Bool ReflectionBase::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > ReflectionBase::getSupportedServiceNames()
{
    return m_supportedServices;
}


Sequence< css::uno::Type > ReflectionBase::getTypes()
{
    osl::MutexGuard guard( m_refMutex->mutex );
    static Sequence< css::uno::Type > collection(
            ::comphelper::concatSequences(
                ::cppu::OPropertySetHelper::getTypes(),
                ReflectionBase_BASE::getTypes() ) );
    return collection;
}


css::uno::Any ReflectionBase::queryInterface(
    const css::uno::Type & reqType )
{
    Any ret = ReflectionBase_BASE::queryInterface( reqType );
    return ret.hasValue() ? ret : OPropertySetHelper::queryInterface( reqType );

}

Sequence< sal_Int8> ReflectionBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
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

OUString ReflectionBase::getName(  )
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
{
    throw RuntimeException(
        "pq_sdbc::ReflectionBase::setName not implemented",
        *this );
    //setPropertyValue( getStatics().NAME , makeAny( aName ) );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
