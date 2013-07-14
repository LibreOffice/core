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

TYPEINIT1( SfxImageItem, SfxInt16Item );

struct SfxImageItem_Impl
{
    OUString aURL;
    long    nAngle;
    bool    bMirrored;
    int     operator == ( const SfxImageItem_Impl& rOther ) const
            { return nAngle == rOther.nAngle && bMirrored == rOther.bMirrored; }
};


SfxImageItem::SfxImageItem( sal_uInt16 which, sal_uInt16 nImage )
    : SfxInt16Item( which, nImage )
{
    pImp = new SfxImageItem_Impl;
    pImp->nAngle = 0;
    pImp->bMirrored = false;
}

SfxImageItem::SfxImageItem( const SfxImageItem& rItem )
    : SfxInt16Item( rItem )
{
    pImp = new SfxImageItem_Impl( *(rItem.pImp) );
}

SfxImageItem::~SfxImageItem()
{
    delete pImp;
}


SfxPoolItem* SfxImageItem::Clone( SfxItemPool* ) const
{
    return new SfxImageItem( *this );
}


int SfxImageItem::operator==( const SfxPoolItem& rItem ) const
{
    return( ((SfxImageItem&) rItem).GetValue() == GetValue() && (*pImp == *(((SfxImageItem&)rItem).pImp) ) );
}

bool SfxImageItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aSeq( 4 );
    aSeq[0] = ::com::sun::star::uno::makeAny( GetValue() );
    aSeq[1] = ::com::sun::star::uno::makeAny( pImp->nAngle );
    aSeq[2] = ::com::sun::star::uno::makeAny( pImp->bMirrored );
    aSeq[3] = ::com::sun::star::uno::makeAny( OUString( pImp->aURL ));

    rVal = ::com::sun::star::uno::makeAny( aSeq );
    return true;
}

bool SfxImageItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aSeq;
    if (( rVal >>= aSeq ) && ( aSeq.getLength() == 4 ))
    {
        sal_Int16     nVal = sal_Int16();
        OUString aURL;
        if ( aSeq[0] >>= nVal )
            SetValue( nVal );
        aSeq[1] >>= pImp->nAngle;
        aSeq[2] >>= pImp->bMirrored;
        if ( aSeq[3] >>= aURL )
            pImp->aURL = aURL;
        return true;
    }

    return false;
}

void SfxImageItem::SetRotation( long nValue )
{
    pImp->nAngle = nValue;
}

long SfxImageItem::GetRotation() const
{
    return pImp->nAngle;
}

void SfxImageItem::SetMirrored( bool bSet )
{
    pImp->bMirrored = bSet;
}

bool SfxImageItem::IsMirrored() const
{
    return pImp->bMirrored;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
