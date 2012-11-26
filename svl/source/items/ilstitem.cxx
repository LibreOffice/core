/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <svl/ilstitem.hxx>

#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>

IMPL_POOLITEM_FACTORY(SfxIntegerListItem)

SfxIntegerListItem::SfxIntegerListItem()
{
}

SfxIntegerListItem::SfxIntegerListItem( sal_uInt16 which, const SvULongs& rList )
    : SfxPoolItem( which )
{
    m_aList.realloc( rList.Count() );
    for ( sal_uInt16 n=0; n<rList.Count(); n++ )
        m_aList[n] = rList[n];
}

SfxIntegerListItem::SfxIntegerListItem( const SfxIntegerListItem& rItem )
    : SfxPoolItem( rItem )
{
    m_aList = rItem.m_aList;
}

SfxIntegerListItem::~SfxIntegerListItem()
{
}

int SfxIntegerListItem::operator==( const SfxPoolItem& rPoolItem ) const
{
    if ( !dynamic_cast< const SfxIntegerListItem* >(&rPoolItem) )
        return sal_False;

    const SfxIntegerListItem rItem = (const SfxIntegerListItem&) rPoolItem;
    return rItem.m_aList == m_aList;
}

SfxPoolItem* SfxIntegerListItem::Clone( SfxItemPool * ) const
{
    return new SfxIntegerListItem( *this );
}

sal_Bool SfxIntegerListItem::PutValue  ( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    ::com::sun::star::uno::Reference < ::com::sun::star::script::XTypeConverter > xConverter
            ( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.script.Converter")),
            ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Any aNew;
    try { aNew = xConverter->convertTo( rVal, ::getCppuType((const ::com::sun::star::uno::Sequence < sal_Int32 >*)0) ); }
    catch (::com::sun::star::uno::Exception&)
    {
        return sal_False;
    }

    return ( aNew >>= m_aList );
}

sal_Bool SfxIntegerListItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
{
    rVal <<= m_aList;
    return sal_True;
}

void SfxIntegerListItem::GetList( SvULongs& rList ) const
{
    for ( sal_Int32 n=0; n<m_aList.getLength(); n++ )
        rList.Insert( m_aList[n], sal::static_int_cast< sal_uInt16 >(n) );
}
