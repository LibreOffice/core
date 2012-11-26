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

#include <svl/imageitm.hxx>
#include <com/sun/star/uno/Sequence.hxx>

struct SfxImageItem_Impl
{
    String  aURL;
    long    nAngle;
    sal_Bool    bMirrored;
    int     operator == ( const SfxImageItem_Impl& rOther ) const
            { return nAngle == rOther.nAngle && bMirrored == rOther.bMirrored; }
};

//---------------------------------------------------------

SfxImageItem::SfxImageItem( sal_uInt16 which, sal_uInt16 nImage )
    : SfxInt16Item( which, nImage )
{
    pImp = new SfxImageItem_Impl;
    pImp->nAngle = 0;
    pImp->bMirrored = sal_False;
}

SfxImageItem::SfxImageItem( sal_uInt16 which, const String& rURL )
    : SfxInt16Item( which, 0 )
{
    pImp = new SfxImageItem_Impl;
    pImp->nAngle = 0;
    pImp->bMirrored = sal_False;
    pImp->aURL = rURL;
}

SfxImageItem::SfxImageItem( const SfxImageItem& rItem )
    : SfxInt16Item( rItem )
{
    pImp = new SfxImageItem_Impl( *(rItem.pImp) );
}

//---------------------------------------------------------
SfxImageItem::~SfxImageItem()
{
    delete pImp;
}

//---------------------------------------------------------

SfxPoolItem* SfxImageItem::Clone( SfxItemPool* ) const
{
    return new SfxImageItem( *this );
}

//---------------------------------------------------------

int SfxImageItem::operator==( const SfxPoolItem& rItem ) const
{
    return( ((SfxImageItem&) rItem).GetValue() == GetValue() && (*pImp == *(((SfxImageItem&)rItem).pImp) ) );
}

sal_Bool SfxImageItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aSeq( 4 );
    aSeq[0] = ::com::sun::star::uno::makeAny( GetValue() );
    aSeq[1] = ::com::sun::star::uno::makeAny( pImp->nAngle );
    aSeq[2] = ::com::sun::star::uno::makeAny( pImp->bMirrored );
    aSeq[3] = ::com::sun::star::uno::makeAny( rtl::OUString( pImp->aURL ));

    rVal = ::com::sun::star::uno::makeAny( aSeq );
    return sal_True;
}

sal_Bool SfxImageItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aSeq;
    if (( rVal >>= aSeq ) && ( aSeq.getLength() == 4 ))
    {
        sal_Int16     nVal = sal_Int16();
        rtl::OUString aURL;
        if ( aSeq[0] >>= nVal )
            SetValue( nVal );
        aSeq[1] >>= pImp->nAngle;
        aSeq[2] >>= pImp->bMirrored;
        if ( aSeq[3] >>= aURL )
            pImp->aURL = aURL;
        return sal_True;
    }

    return sal_False;
}

void SfxImageItem::SetRotation( long nValue )
{
    pImp->nAngle = nValue;
}

long SfxImageItem::GetRotation() const
{
    return pImp->nAngle;
}

void SfxImageItem::SetMirrored( sal_Bool bSet )
{
    pImp->bMirrored = bSet;
}

sal_Bool SfxImageItem::IsMirrored() const
{
    return pImp->bMirrored;
}

String SfxImageItem::GetURL() const
{
    return pImp->aURL;
}

