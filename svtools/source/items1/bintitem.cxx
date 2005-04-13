/*************************************************************************
 *
 *  $RCSfile: bintitem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:13:51 $
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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#ifndef _BINTITEM_HXX
#include <bintitem.hxx>
#endif

// STATIC DATA

DBG_NAME(SfxBigIntItem);

// RTTI
TYPEINIT1_AUTOFACTORY(SfxBigIntItem, SfxPoolItem);

// SfxBigIntItem

//============================================================================
SfxBigIntItem::SfxBigIntItem()
    : SfxPoolItem(0),
        aVal(0)
{
    DBG_CTOR(SfxBigIntItem, 0);
}

//============================================================================
SfxBigIntItem::SfxBigIntItem(USHORT nWhich, const BigInt& rValue)
    : SfxPoolItem(nWhich),
        aVal(rValue)
{
    DBG_CTOR(SfxBigIntItem, 0);
}

//============================================================================
SfxBigIntItem::SfxBigIntItem(USHORT nWhich, SvStream &rStream)
    : SfxPoolItem(nWhich)
{
    DBG_CTOR(SfxBigIntItem, 0);
    ByteString sTmp;
    rStream.ReadByteString(sTmp);
    BigInt aTmp(sTmp);
    aVal = aTmp;
}

//============================================================================
SfxBigIntItem::SfxBigIntItem(const SfxBigIntItem& rItem)
    : SfxPoolItem(rItem),
        aVal(rItem.aVal)
{
    DBG_CTOR(SfxBigIntItem, 0);
}

//============================================================================
SfxItemPresentation SfxBigIntItem::GetPresentation(
    SfxItemPresentation ePresentation,
    SfxMapUnit          eCoreMetric,
    SfxMapUnit          ePresentationMetric,
    XubString&           rText,
    const IntlWrapper * ) const
{
    DBG_CHKTHIS(SfxBigIntItem, 0);
    rText = aVal.GetString();
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
int SfxBigIntItem::operator==(const SfxPoolItem& rItem) const
{
    DBG_CHKTHIS(SfxBigIntItem, 0);
    DBG_ASSERT(SfxPoolItem::operator==(rItem), "unequal type");
    return ((SfxBigIntItem&)rItem).aVal == aVal;
}

//============================================================================
int SfxBigIntItem::Compare(const SfxPoolItem& rItem) const
{
    DBG_CHKTHIS(SfxBigIntItem, 0);
    DBG_ASSERT(SfxPoolItem::operator==(rItem), "unequal type");

    if (((const SfxBigIntItem&)rItem ).aVal < aVal )
        return -1;
    else if (((const SfxBigIntItem&)rItem ).aVal == aVal)
        return 0;
    else
        return 1;
}

//============================================================================
SfxPoolItem* SfxBigIntItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxBigIntItem, 0);
    return new SfxBigIntItem(*this);
}

//============================================================================
SfxPoolItem* SfxBigIntItem::Create(SvStream &rStream, USHORT) const
{
    DBG_CHKTHIS(SfxBigIntItem, 0);
    return new SfxBigIntItem(Which(), rStream);
}

//============================================================================
SvStream& SfxBigIntItem::Store(SvStream &rStream, USHORT nItemVersion) const
{
    DBG_CHKTHIS(SfxBigIntItem, 0);
    rStream.WriteByteString( aVal.GetByteString() );
    return rStream;
}

//============================================================================
SfxFieldUnit SfxBigIntItem::GetUnit() const
{
    DBG_CHKTHIS(SfxBigIntItem, 0);
    return SFX_FUNIT_NONE;
}

//============================================================================
// virtual
BOOL SfxBigIntItem::PutValue( const com::sun::star::uno::Any& rVal,
                                 BYTE nMemberId )
{
    double aValue;
    if ( rVal >>= aValue )
    {
        SetValue( aValue );
        return TRUE;
    }

    DBG_ERROR( "SfxBigIntItem::PutValue - Wrong type!" );
    return FALSE;
}

//============================================================================
// virtual
BOOL SfxBigIntItem::QueryValue( com::sun::star::uno::Any& rVal,
                                 BYTE nMemberId ) const
{
    double aValue = GetValue();
    rVal <<= aValue;
    return TRUE;
}

