/*************************************************************************
 *
 *  $RCSfile: fmitems.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:16 $
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

#pragma hdrstop
#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

//========================================================================
// class FmFormInfoItem
//========================================================================
TYPEINIT1(FmFormInfoItem, SfxPoolItem);

//------------------------------------------------------------------------------
sal_Bool FmFormInfoItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_Int8 nMemberId ) const
{
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmFormInfoItem::SetValue(const ::com::sun::star::uno::Any& rVal, sal_Int8 nMemberId )
{
    sal_Bool bRet = sal_False;
    return bRet;
}

//------------------------------------------------------------------------------
int FmFormInfoItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
    return ( aInfo == ( (FmFormInfoItem&)rAttr ).GetInfo() );
}

//------------------------------------------------------------------------------
SfxPoolItem* FmFormInfoItem::Clone( SfxItemPool* ) const
{
    return new FmFormInfoItem( *this );
}

//------------------------------------------------------------------------------
SvStream& FmFormInfoItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << (long) aInfo.Pos;
    rStrm << (long) aInfo.Count;
    return rStrm;
}

//------------------------------------------------------------------------------
SfxPoolItem* FmFormInfoItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int32 nCount, nPos;
    rStrm >> nPos >> nCount;

    FmFormInfoItem* pAttr = new FmFormInfoItem(Which(), FmFormInfo(nPos, nCount) );
    return pAttr;
}


//========================================================================
// class FmInterfaceItem
//========================================================================
TYPEINIT1(FmInterfaceItem, SfxPoolItem);

//------------------------------------------------------------------------------
sal_Bool FmInterfaceItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_Int8 nMemberId ) const
{
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmInterfaceItem::SetValue(const ::com::sun::star::uno::Any& rVal, sal_Int8 nMemberId )
{
    return sal_False;
}

//------------------------------------------------------------------------------
int FmInterfaceItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
    return( xInterface == ((FmInterfaceItem&)rAttr).GetInterface() );
}

//------------------------------------------------------------------------------
SfxPoolItem* FmInterfaceItem::Clone( SfxItemPool* ) const
{
    return new FmInterfaceItem( *this );
}

//------------------------------------------------------------------------------
SvStream& FmInterfaceItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    return rStrm;
}

//------------------------------------------------------------------------------
SfxPoolItem* FmInterfaceItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    return new FmInterfaceItem( *this );
}




