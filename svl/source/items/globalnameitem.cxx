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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/script/Converter.hpp>

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
bool SfxGlobalNameItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    com::sun::star::uno::Reference < com::sun::star::script::XTypeConverter > xConverter
            ( com::sun::star::script::Converter::create( ::comphelper::getProcessComponentContext() ));
    com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    com::sun::star::uno::Any aNew;

    try { aNew = xConverter->convertTo( rVal, ::getCppuType((const com::sun::star::uno::Sequence < sal_Int8 >*)0) ); }
    catch (com::sun::star::uno::Exception&) {}
    aNew >>= aSeq;
    if ( aSeq.getLength() == 16 )
    {
        m_aName.MakeFromMemory( (void*) aSeq.getConstArray() );
        return true;
    }

    OSL_FAIL( "SfxGlobalNameItem::PutValue - Wrong type!" );
    return true;
}

//----------------------------------------------------------------------------
// virtual
bool SfxGlobalNameItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
{
       com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
    void* pData = ( void* ) &m_aName.GetCLSID();
    memcpy( aSeq.getArray(), pData, 16 );
    rVal <<= aSeq;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
