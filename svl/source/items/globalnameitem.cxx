/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/script/XTypeConverter.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <svl/globalnameitem.hxx>

// STATIC DATA -----------------------------------------------------------


// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxGlobalNameItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxGlobalNameItem::SfxGlobalNameItem()
{
}

// -----------------------------------------------------------------------

SfxGlobalNameItem::SfxGlobalNameItem( sal_uInt16 nW, const SvGlobalName& rName )
:   SfxPoolItem( nW ),
    m_aName( rName )
{
}

// -----------------------------------------------------------------------

SfxGlobalNameItem::~SfxGlobalNameItem()
{
}

// -----------------------------------------------------------------------

int SfxGlobalNameItem::operator==( const SfxPoolItem& rItem ) const
{
    return ((SfxGlobalNameItem&)rItem).m_aName == m_aName;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxGlobalNameItem::Clone(SfxItemPool *) const
{
    return new SfxGlobalNameItem( *this );
}

//----------------------------------------------------------------------------
// virtual
sal_Bool SfxGlobalNameItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    com::sun::star::uno::Reference < com::sun::star::script::XTypeConverter > xConverter
            ( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.script.Converter")),
            com::sun::star::uno::UNO_QUERY );
    com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    com::sun::star::uno::Any aNew;

    try { aNew = xConverter->convertTo( rVal, ::getCppuType((const com::sun::star::uno::Sequence < sal_Int8 >*)0) ); }
    catch (com::sun::star::uno::Exception&) {}
    aNew >>= aSeq;
    if ( aSeq.getLength() == 16 )
    {
        m_aName.MakeFromMemory( (void*) aSeq.getConstArray() );
        return sal_True;
    }

    DBG_ERROR( "SfxGlobalNameItem::PutValue - Wrong type!" );
    return sal_False;
}

//----------------------------------------------------------------------------
// virtual
sal_Bool SfxGlobalNameItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
{
       com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
    void* pData = ( void* ) &m_aName.GetCLSID();
    memcpy( aSeq.getArray(), pData, 16 );
    rVal <<= aSeq;
    return sal_True;
}

