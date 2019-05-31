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

#include <svl/lckbitem.hxx>
#include <tools/stream.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>


SfxPoolItem* SfxLockBytesItem::CreateDefault() { return new SfxLockBytesItem; }


SfxLockBytesItem::SfxLockBytesItem()
{
}


SfxLockBytesItem::~SfxLockBytesItem()
{
}


bool SfxLockBytesItem::operator==( const SfxPoolItem& rItem ) const
{
    return static_cast<const SfxLockBytesItem&>(rItem)._xVal == _xVal;
}


SfxPoolItem* SfxLockBytesItem::Clone(SfxItemPool *) const
{
    return new SfxLockBytesItem( *this );
}


// virtual
bool SfxLockBytesItem::PutValue( const css::uno::Any& rVal, sal_uInt8 )
{
    css::uno::Sequence< sal_Int8 > aSeq;
    if ( rVal >>= aSeq )
    {
        if ( aSeq.hasElements() )
        {
            SvMemoryStream* pStream = new SvMemoryStream();
            pStream->WriteBytes( aSeq.getConstArray(), aSeq.getLength() );
            pStream->Seek(0);

            _xVal = new SvLockBytes( pStream, true );
        }
        else
            _xVal = nullptr;

        return true;
    }

    OSL_FAIL( "SfxLockBytesItem::PutValue - Wrong type!" );
    return true;
}

// virtual
bool SfxLockBytesItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    if ( _xVal.is() )
    {
        sal_uInt32 nLen;
        SvLockBytesStat aStat;

        if ( _xVal->Stat( &aStat ) == ERRCODE_NONE )
            nLen = aStat.nSize;
        else
            return false;

        std::size_t nRead = 0;
        css::uno::Sequence< sal_Int8 > aSeq( nLen );

        _xVal->ReadAt( 0, aSeq.getArray(), nLen, &nRead );
        rVal <<= aSeq;
    }
    else
    {
        css::uno::Sequence< sal_Int8 > aSeq( 0 );
        rVal <<= aSeq;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
