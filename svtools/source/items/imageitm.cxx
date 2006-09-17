/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imageitm.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:59:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "imageitm.hxx"

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

TYPEINIT1( SfxImageItem, SfxInt16Item );

struct SfxImageItem_Impl
{
    String  aURL;
    long    nAngle;
    BOOL    bMirrored;
    int     operator == ( const SfxImageItem_Impl& rOther ) const
            { return nAngle == rOther.nAngle && bMirrored == rOther.bMirrored; }
};

//---------------------------------------------------------

SfxImageItem::SfxImageItem( USHORT which, UINT16 nImage )
    : SfxInt16Item( which, nImage )
{
    pImp = new SfxImageItem_Impl;
    pImp->nAngle = 0;
    pImp->bMirrored = FALSE;
}

SfxImageItem::SfxImageItem( USHORT which, const String& rURL )
    : SfxInt16Item( which, 0 )
{
    pImp = new SfxImageItem_Impl;
    pImp->nAngle = 0;
    pImp->bMirrored = FALSE;
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

BOOL SfxImageItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE ) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aSeq( 4 );
    aSeq[0] = ::com::sun::star::uno::makeAny( GetValue() );
    aSeq[1] = ::com::sun::star::uno::makeAny( pImp->nAngle );
    aSeq[2] = ::com::sun::star::uno::makeAny( pImp->bMirrored );
    aSeq[3] = ::com::sun::star::uno::makeAny( rtl::OUString( pImp->aURL ));

    rVal = ::com::sun::star::uno::makeAny( aSeq );
    return TRUE;
}

BOOL SfxImageItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE )
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
        return TRUE;
    }

    return FALSE;
}

void SfxImageItem::SetRotation( long nValue )
{
    pImp->nAngle = nValue;
}

long SfxImageItem::GetRotation() const
{
    return pImp->nAngle;
}

void SfxImageItem::SetMirrored( BOOL bSet )
{
    pImp->bMirrored = bSet;
}

BOOL SfxImageItem::IsMirrored() const
{
    return pImp->bMirrored;
}

String SfxImageItem::GetURL() const
{
    return pImp->aURL;
}

