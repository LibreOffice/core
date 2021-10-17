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
#include "filtask.hxx"
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

using namespace fileaccess;
using namespace com::sun::star;
using namespace css::uno;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

// Function for TypeConverting

template< class _type_ >
static bool convert( TaskManager const * pShell,
                  uno::Reference< script::XTypeConverter >& xConverter,
                  const uno::Any& rValue,
                  _type_& aReturn  )
{
    // Try first without converting
    bool no_success = ! ( rValue >>= aReturn );

    if ( no_success )
    {
        if( ! xConverter.is() )
        {
            xConverter = script::Converter::create(pShell->m_xContext);
        }

        try
        {
            if( rValue.hasValue() )
            {
                uno::Any aConvertedValue
                    = xConverter->convertTo( rValue,cppu::UnoType<_type_>::get() );
                no_success = ! ( aConvertedValue >>= aReturn );
            }
            else
                no_success = true;
        }
        catch (const lang::IllegalArgumentException&)
        {
            no_success = true;
        }
        catch (const script::CannotConvertException&)
        {
            no_success = true;
        }
    }
    return no_success;
}


XRow_impl::XRow_impl( TaskManager* pMyShell,const uno::Sequence< uno::Any >& seq )
    : m_aValueMap( seq ),
      m_nWasNull(false),
      m_pMyShell( pMyShell )
{
}

XRow_impl::~XRow_impl()
{
}


sal_Bool SAL_CALL
XRow_impl::wasNull()
{
  return m_nWasNull;
}


OUString SAL_CALL
XRow_impl::getString(
             sal_Int32 columnIndex )
{
  if( isIndexOutOfBounds( columnIndex ) )
    throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
  return getValue<OUString>(columnIndex);
}

sal_Bool SAL_CALL
XRow_impl::getBoolean(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<bool>(columnIndex);
}


sal_Int8 SAL_CALL
XRow_impl::getByte(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<sal_Int8>(columnIndex);
}

sal_Int16 SAL_CALL
XRow_impl::getShort(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<sal_Int16>(columnIndex);
}


sal_Int32 SAL_CALL
XRow_impl::getInt(
          sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<sal_Int32>(columnIndex);
}

sal_Int64 SAL_CALL
XRow_impl::getLong(
           sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<sal_Int64>(columnIndex);
}

float SAL_CALL
XRow_impl::getFloat(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<float>(columnIndex);
}

double SAL_CALL
XRow_impl::getDouble(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<double>(columnIndex);
}

uno::Sequence< sal_Int8 > SAL_CALL
XRow_impl::getBytes(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<uno::Sequence< sal_Int8 >>(columnIndex);
}

util::Date SAL_CALL
XRow_impl::getDate(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<util::Date>(columnIndex);
}

util::Time SAL_CALL
XRow_impl::getTime(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<util::Time>(columnIndex);
}

util::DateTime SAL_CALL
XRow_impl::getTimestamp(
            sal_Int32 columnIndex )
{
  if( isIndexOutOfBounds( columnIndex ) )
    throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
  return getValue<util::DateTime>(columnIndex);
}


uno::Reference< io::XInputStream > SAL_CALL
XRow_impl::getBinaryStream(
               sal_Int32 columnIndex )
{
  if( isIndexOutOfBounds( columnIndex ) )
    throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
  return getValue<uno::Reference< io::XInputStream >>(columnIndex);
}


uno::Reference< io::XInputStream > SAL_CALL
XRow_impl::getCharacterStream(
                  sal_Int32 columnIndex )
{
  if( isIndexOutOfBounds( columnIndex ) )
      throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
  return getValue<uno::Reference< io::XInputStream >>(columnIndex);
}


uno::Any SAL_CALL
XRow_impl::getObject(
    sal_Int32 columnIndex,
    const uno::Reference< container::XNameAccess >& )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    std::scoped_lock aGuard( m_aMutex );
    uno::Any Value = m_aValueMap[columnIndex - 1];
    m_nWasNull = !Value.hasValue();
    return Value;
}

uno::Reference< sdbc::XRef > SAL_CALL
XRow_impl::getRef(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<uno::Reference< sdbc::XRef >>(columnIndex);
}

uno::Reference< sdbc::XBlob > SAL_CALL
XRow_impl::getBlob(
           sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<uno::Reference< sdbc::XBlob >>(columnIndex);
}

uno::Reference< sdbc::XClob > SAL_CALL
XRow_impl::getClob(
           sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<uno::Reference< sdbc::XClob >>(columnIndex);
}


uno::Reference< sdbc::XArray > SAL_CALL
XRow_impl::getArray(
    sal_Int32 columnIndex )
{
    if( isIndexOutOfBounds( columnIndex ) )
        throw sdbc::SQLException( THROW_WHERE, uno::Reference< uno::XInterface >(), OUString(), 0, uno::Any() );
    return getValue<uno::Reference< sdbc::XArray >>(columnIndex);
}

bool
XRow_impl::isIndexOutOfBounds(sal_Int32 nIndex) const
{
    return nIndex < 1 || m_aValueMap.getLength() < nIndex;
}

template<typename T>
T XRow_impl::getValue(sal_Int32 columnIndex)
{
    T aValue{};
    std::scoped_lock aGuard( m_aMutex );
    m_nWasNull = ::convert<T>( m_pMyShell, m_xTypeConverter, m_aValueMap[ --columnIndex ], aValue );
    return aValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
