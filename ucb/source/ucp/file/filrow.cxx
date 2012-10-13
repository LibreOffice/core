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

#include "filrow.hxx"
#include "shell.hxx"
#include "prov.hxx"
#include <com/sun/star/script/Converter.hpp>
#include <comphelper/processfactory.hxx>

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;

// Funktion for TypeConverting


template< class _type_ >
sal_Bool convert( shell* pShell,
                  uno::Reference< script::XTypeConverter >& xConverter,
                  uno::Any& rValue,
                  _type_& aReturn  )
{
    // Try first without converting
    sal_Bool no_success = ! ( rValue >>= aReturn );

    if ( no_success )
    {
        if( ! xConverter.is() )
        {
            xConverter = uno::Reference< script::XTypeConverter >(
                script::Converter::create(comphelper::getComponentContext(pShell->m_xMultiServiceFactory)) );
        }

        try
        {
            if( rValue.hasValue() )
            {
                uno::Any aConvertedValue
                    = xConverter->convertTo( rValue,getCppuType( static_cast< const _type_* >(0) ) );
                no_success = ! ( aConvertedValue >>= aReturn );
            }
            else
                no_success = sal_True;
        }
        catch (const lang::IllegalArgumentException&)
        {
            no_success = sal_True;
        }
        catch (const script::CannotConvertException&)
        {
            no_success = sal_True;
        }
    }
    return no_success;
}


XRow_impl::XRow_impl( shell* pMyShell,const uno::Sequence< uno::Any >& seq )
    : m_aValueMap( seq ),
      m_pMyShell( pMyShell ),
      m_xProvider( pMyShell->m_pProvider ),
      m_xTypeConverter( 0 )
{
}

XRow_impl::~XRow_impl()
{
}


void SAL_CALL
XRow_impl::acquire(
           void )
  throw()
{
  OWeakObject::acquire();
}

void SAL_CALL
XRow_impl::release(
           void )
  throw()
{
  OWeakObject::release();
}


uno::Any SAL_CALL
XRow_impl::queryInterface(
              const uno::Type& rType )
  throw( uno::RuntimeException )
{
  uno::Any aRet = cppu::queryInterface( rType,
                    (static_cast< lang::XTypeProvider* >(this)),
                    (static_cast< sdbc::XRow* >(this)) );
  return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


XTYPEPROVIDER_IMPL_2( XRow_impl,
                         lang::XTypeProvider,
                      sdbc::XRow )


sal_Bool SAL_CALL
XRow_impl::wasNull(
           void )
  throw( sdbc::SQLException,
     uno::RuntimeException)
{
  return m_nWasNull;
}


rtl::OUString SAL_CALL
XRow_impl::getString(
             sal_Int32 columnIndex )
  throw( sdbc::SQLException,
     uno::RuntimeException)
{
  if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
    throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
  rtl::OUString  Value;
  osl::MutexGuard aGuard( m_aMutex );
  m_nWasNull = ::convert<rtl::OUString>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
  return Value;
}

sal_Bool SAL_CALL
XRow_impl::getBoolean(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    sal_Bool  Value( false );
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<sal_Bool>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}


sal_Int8 SAL_CALL
XRow_impl::getByte(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    sal_Int8  Value( 0 );
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<sal_Int8>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

sal_Int16 SAL_CALL
XRow_impl::getShort(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    sal_Int16  Value( 0 );
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<sal_Int16>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}


sal_Int32 SAL_CALL
XRow_impl::getInt(
          sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    sal_Int32  Value( 0 );
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<sal_Int32>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

sal_Int64 SAL_CALL
XRow_impl::getLong(
           sal_Int32 columnIndex )
  throw( sdbc::SQLException,
     uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    sal_Int64  Value( 0 );
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<sal_Int64>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

float SAL_CALL
XRow_impl::getFloat(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    float  Value( 0 );
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<float>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

double SAL_CALL
XRow_impl::getDouble(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    double  Value( 0 );
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<double>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

uno::Sequence< sal_Int8 > SAL_CALL
XRow_impl::getBytes(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    uno::Sequence< sal_Int8 >  Value(0);
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<uno::Sequence< sal_Int8 > >( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

util::Date SAL_CALL
XRow_impl::getDate(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    util::Date  Value;
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<util::Date>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

util::Time SAL_CALL
XRow_impl::getTime(
    sal_Int32 columnIndex )
  throw( sdbc::SQLException,
         uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    util::Time  Value;
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<util::Time>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

util::DateTime SAL_CALL
XRow_impl::getTimestamp(
            sal_Int32 columnIndex )
  throw( sdbc::SQLException,
     uno::RuntimeException)
{
  if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
    throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
  util::DateTime  Value;
  osl::MutexGuard aGuard( m_aMutex );
  m_nWasNull = ::convert<util::DateTime>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
  return Value;
}


uno::Reference< io::XInputStream > SAL_CALL
XRow_impl::getBinaryStream(
               sal_Int32 columnIndex )
  throw( sdbc::SQLException,
     uno::RuntimeException)
{
  if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
    throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
  uno::Reference< io::XInputStream >  Value;
  osl::MutexGuard aGuard( m_aMutex );
  m_nWasNull = ::convert<uno::Reference< io::XInputStream > >( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
  return Value;
}


uno::Reference< io::XInputStream > SAL_CALL
XRow_impl::getCharacterStream(
                  sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
  if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
      throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
  uno::Reference< io::XInputStream > Value;
  osl::MutexGuard aGuard( m_aMutex );
  m_nWasNull = ::convert< uno::Reference< io::XInputStream> >( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
  return Value;
}


uno::Any SAL_CALL
XRow_impl::getObject(
    sal_Int32 columnIndex,
    const uno::Reference< container::XNameAccess >& )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    uno::Any  Value;
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<uno::Any>( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
    return Value;
}

uno::Reference< sdbc::XRef > SAL_CALL
XRow_impl::getRef(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    uno::Reference< sdbc::XRef > Value;
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<uno::Reference< sdbc::XRef> >( m_pMyShell,
                                                          m_xTypeConverter,
                                                          m_aValueMap[ --columnIndex ],
                                                          Value );
    return Value;
}

uno::Reference< sdbc::XBlob > SAL_CALL
XRow_impl::getBlob(
           sal_Int32 columnIndex )
  throw( sdbc::SQLException,
     uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    uno::Reference< sdbc::XBlob > Value;
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<uno::Reference< sdbc::XBlob> >( m_pMyShell,
                                                           m_xTypeConverter,
                                                           m_aValueMap[ --columnIndex ],
                                                           Value );
    return Value;
}

uno::Reference< sdbc::XClob > SAL_CALL
XRow_impl::getClob(
           sal_Int32 columnIndex )
  throw( sdbc::SQLException,
     uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    uno::Reference< sdbc::XClob > Value;
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<uno::Reference< sdbc::XClob> >( m_pMyShell,
                                                           m_xTypeConverter,
                                                           m_aValueMap[ --columnIndex ],
                                                           Value );
    return Value;
}


uno::Reference< sdbc::XArray > SAL_CALL
XRow_impl::getArray(
    sal_Int32 columnIndex )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), ::rtl::OUString(), 0, uno::Any() );
    uno::Reference< sdbc::XArray > Value;
    osl::MutexGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<uno::Reference< sdbc::XArray> >( m_pMyShell,
                                                            m_xTypeConverter,
                                                            m_aValueMap[ --columnIndex ],
                                                            Value );
    return Value;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
