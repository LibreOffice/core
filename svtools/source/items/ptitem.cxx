/*************************************************************************
 *
 *  $RCSfile: ptitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:01 $
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

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include "sbx.hxx"
#include "args.hxx"
#include "poolitem.hxx"
#include "ptitem.hxx"
#include "memberid.hrc"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxPointItem);


// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxPointItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem()
{
    DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem( USHORT nW, const Point& rVal ) :
    SfxPoolItem( nW ),
    aVal( rVal )
{
    DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem( USHORT nW, SvStream &rStream ) :
    SfxPoolItem( nW )
{
    DBG_CTOR(SfxPointItem, 0);
    rStream >> aVal;
}

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem( const SfxPointItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxPointItem::GetPresentation
(
    SfxItemPresentation     ePresentation,
    SfxMapUnit              eCoreMetric,
    SfxMapUnit              ePresentationMetric,
    XubString&              rText,
    const International *
)   const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    rText = UniString::CreateFromInt32(aVal.X());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    rText += UniString::CreateFromInt32(aVal.Y());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxPointItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SfxPointItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxPointItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    return new SfxPointItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxPointItem::Create(SvStream &rStream, USHORT nVersion ) const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    Point aStr;
    rStream >> aStr;
    return new SfxPointItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SfxPointItem::Store(SvStream &rStream, USHORT nItemVersion) const
{
    DBG_CHKTHIS(SfxPointItem, 0);
    rStream << aVal;
    return rStream;
}

// -----------------------------------------------------------------------

BOOL SfxPointItem::QueryValue( com::sun::star::uno::Any& rVal,
                               BYTE nMemberId ) const
{
    rVal <<= com::sun::star::awt::Point( aVal.getX(), aVal.getY() );
    return TRUE;
}

// -----------------------------------------------------------------------

BOOL SfxPointItem::PutValue( const com::sun::star::uno::Any& rVal,
                             BYTE nMemberId )
{
    BOOL bRet = FALSE;
    com::sun::star::awt::Point aValue;
    if ( rVal >>= aValue )
    {
        aVal.setX( aValue.X );
        aVal.setY( aValue.Y );
        bRet = TRUE;
    }
    return bRet;
}



