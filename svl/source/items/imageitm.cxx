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


#include <svl/imageitm.hxx>
#include <com/sun/star/uno/Sequence.hxx>


SfxPoolItem* SfxImageItem::CreateDefault() { return new SfxImageItem; }

struct SfxImageItem_Impl
{
    OUString aURL;
    long    nAngle;
    bool    bMirrored;
    bool    operator == ( const SfxImageItem_Impl& rOther ) const
            { return nAngle == rOther.nAngle && bMirrored == rOther.bMirrored; }
};


SfxImageItem::SfxImageItem( sal_uInt16 which )
    : SfxInt16Item( which, 0 ),
          pImpl( new SfxImageItem_Impl)
{
    pImpl->nAngle = 0;
    pImpl->bMirrored = false;
}

SfxImageItem::SfxImageItem( const SfxImageItem& rItem )
    : SfxInt16Item( rItem ),
      pImpl( new SfxImageItem_Impl( *rItem.pImpl ) )
{
}

SfxImageItem::~SfxImageItem()
{
}


SfxPoolItem* SfxImageItem::Clone( SfxItemPool* ) const
{
    return new SfxImageItem( *this );
}


bool SfxImageItem::operator==( const SfxPoolItem& rItem ) const
{
    return (static_cast<const SfxImageItem&>(rItem).GetValue() == GetValue()) &&
           (*pImpl == *static_cast<const SfxImageItem&>(rItem).pImpl);
}

bool SfxImageItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    css::uno::Sequence< css::uno::Any > aSeq( 4 );
    aSeq[0] <<= GetValue();
    aSeq[1] <<= pImpl->nAngle;
    aSeq[2] <<= pImpl->bMirrored;
    aSeq[3] <<= pImpl->aURL;

    rVal <<= aSeq;
    return true;
}

bool SfxImageItem::PutValue( const css::uno::Any& rVal, sal_uInt8 )
{
    css::uno::Sequence< css::uno::Any > aSeq;
    if (( rVal >>= aSeq ) && ( aSeq.getLength() == 4 ))
    {
        sal_Int16     nVal = sal_Int16();
        OUString aURL;
        if ( aSeq[0] >>= nVal )
            SetValue( nVal );
        aSeq[1] >>= pImpl->nAngle;
        aSeq[2] >>= pImpl->bMirrored;
        if ( aSeq[3] >>= aURL )
            pImpl->aURL = aURL;
        return true;
    }

    return false;
}

void SfxImageItem::SetRotation( long nValue )
{
    pImpl->nAngle = nValue;
}

long SfxImageItem::GetRotation() const
{
    return pImpl->nAngle;
}

void SfxImageItem::SetMirrored( bool bSet )
{
    pImpl->bMirrored = bSet;
}

bool SfxImageItem::IsMirrored() const
{
    return pImpl->bMirrored;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
