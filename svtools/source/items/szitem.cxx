/*************************************************************************
 *
 *  $RCSfile: szitem.cxx,v $
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

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#include "args.hxx"
#include "poolitem.hxx"
#include "szitem.hxx"
#include "memberid.hrc"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxSizeItem);

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxSizeItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem()
{
    DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( USHORT nW, const Size& rVal ) :
    SfxPoolItem( nW ),
    aVal( rVal )
{
    DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( USHORT nW, SvStream &rStream ) :
    SfxPoolItem( nW )
{
    DBG_CTOR(SfxSizeItem, 0);
    rStream >> aVal;
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( const SfxSizeItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxSizeItem::GetPresentation
(
    SfxItemPresentation     ePresentation,
    SfxMapUnit              eCoreMetric,
    SfxMapUnit              ePresentationMetric,
    XubString&              rText,
    const International *
)   const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    rText = UniString::CreateFromInt32(aVal.Width());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    rText += UniString::CreateFromInt32(aVal.Height());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxSizeItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SfxSizeItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxSizeItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    return new SfxSizeItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxSizeItem::Create(SvStream &rStream, USHORT nVersion ) const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    Size aStr;
    rStream >> aStr;
    return new SfxSizeItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SfxSizeItem::Store(SvStream &rStream, USHORT nItemVersion) const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    rStream << aVal;
    return rStream;
}

// -----------------------------------------------------------------------
BOOL  SfxSizeItem::QueryValue( com::sun::star::uno::Any& rVal,
                               BYTE nMemberId ) const
{
    Size aTmp(aVal);
    if(CONVERT_TWIPS&nMemberId)
    {
        aTmp.Height() = ( aTmp.Height() * 127 + 36) / 72;
        aTmp.Width() = ( aTmp.Width() * 127 + 36) / 72;
    }
    rVal <<= com::sun::star::awt::Size( aTmp.getWidth(), aTmp.getHeight() );
    return TRUE;
}

// -----------------------------------------------------------------------
BOOL SfxSizeItem::PutValue( const com::sun::star::uno::Any& rVal,
                            BYTE nMemberId )
{
    BOOL bRet = FALSE;
    com::sun::star::awt::Size aValue;
    if ( rVal >>= aValue )
    {
        Size aTmp( aValue.Width, aValue.Height );
        if(CONVERT_TWIPS&nMemberId)
        {
            aTmp.Height() = ( aTmp.Height() * 72 + 63) / 127;
            aTmp.Width() = ( aTmp.Width() * 72 + 63) / 127;
        }
        aVal = aTmp;
        bRet = TRUE;
    }
    return bRet;
}



