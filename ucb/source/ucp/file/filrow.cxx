/*************************************************************************
 *
 *  $RCSfile: filrow.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _FILROW_HXX_
#include "filrow.hxx"
#endif
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif

using namespace fileaccess;
using namespace com::sun::star;
//using namespace com::sun::star::ucb;


// Funktion for TypeConverting


template< class _type_ >
sal_Bool convert( shell* pShell,
                  uno::Reference< script::XTypeConverter >& xConverter,
                  uno::Any& rValue,
                  _type_& aReturn  )
{
    sal_Bool no_success = ! ( rValue >>= aReturn );

    if ( no_success )
    {
        if( ! xConverter.is() )
        {
            xConverter = uno::Reference< script::XTypeConverter >(
                pShell->m_xMultiServiceFactory->createInstance(
                    rtl::OUString::createFromAscii( "com.sun.star.script.Converter" ) ),uno::UNO_QUERY );

/*          DBG_ASSERT( m_xTypeConverter.is(),
                        "PropertyValueSet::getTypeConverter() - "
                        "Service 'com.sun.star.script.Converter' n/a!" );*/
        }
        try
        {
            uno::Any aConvertedValue
                = xConverter->convertTo( rValue,getCppuType( static_cast< const _type_* >(0) ) );
            no_success = ! ( aConvertedValue >>= aReturn );
        }
        catch ( lang::IllegalArgumentException )
        {
            no_success = sal_True;
        }
        catch ( script::CannotConvertException )
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
  throw( uno::RuntimeException )
{
  OWeakObject::acquire();
}

void SAL_CALL
XRow_impl::release(
           void )
  throw( uno::RuntimeException )
{
  OWeakObject::release();
}


uno::Any SAL_CALL
XRow_impl::queryInterface(
              const uno::Type& rType )
  throw( uno::RuntimeException )
{
  uno::Any aRet = cppu::queryInterface( rType,
                    SAL_STATIC_CAST( sdbc::XRow*,this) );
  return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



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
    throw sdbc::SQLException();
  rtl::OUString  Value;
  vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    sal_Bool  Value( false );
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    sal_Int8  Value( 0 );
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    sal_Int16  Value( 0 );
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    sal_Int32  Value( 0 );
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    sal_Int64  Value( 0 );
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    float  Value( 0 );
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    double  Value( 0 );
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    uno::Sequence< sal_Int8 >  Value(0);
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    util::Date  Value;
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    util::Time  Value;
    vos::OGuard aGuard( m_aMutex );
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
    throw sdbc::SQLException();
  util::DateTime  Value;
  vos::OGuard aGuard( m_aMutex );
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
    throw sdbc::SQLException();
  uno::Reference< io::XInputStream >  Value;
  vos::OGuard aGuard( m_aMutex );
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
      throw sdbc::SQLException();
  uno::Reference< io::XInputStream > Value;
  vos::OGuard aGuard( m_aMutex );
  m_nWasNull = ::convert< uno::Reference< io::XInputStream> >( m_pMyShell,m_xTypeConverter,m_aValueMap[ --columnIndex ],Value );
  return Value;
}


uno::Any SAL_CALL
XRow_impl::getObject(
    sal_Int32 columnIndex,
    const uno::Reference< container::XNameAccess >& typeMap )
    throw( sdbc::SQLException,
           uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_aValueMap.getLength() )
        throw sdbc::SQLException();
    uno::Any  Value;
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    uno::Reference< sdbc::XRef > Value;
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    uno::Reference< sdbc::XBlob > Value;
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    uno::Reference< sdbc::XClob > Value;
    vos::OGuard aGuard( m_aMutex );
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
        throw sdbc::SQLException();
    uno::Reference< sdbc::XArray > Value;
    vos::OGuard aGuard( m_aMutex );
    m_nWasNull = ::convert<uno::Reference< sdbc::XArray> >( m_pMyShell,
                                                            m_xTypeConverter,
                                                            m_aValueMap[ --columnIndex ],
                                                            Value );
    return Value;
}
