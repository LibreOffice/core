/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove


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
#include <bf_svtools/bintitem.hxx>
#endif

// STATIC DATA
namespace binfilter
{

DBG_NAME(SfxBigIntItem)

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
SfxBigIntItem::SfxBigIntItem(USHORT which, SvStream &rStream)
    : SfxPoolItem(which)
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
    SfxItemPresentation /*ePresentation*/,
    SfxMapUnit          /*eCoreMetric*/,
    SfxMapUnit          /*ePresentationMetric*/,
    XubString&           rText,
    const ::IntlWrapper * ) const
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
SvStream& SfxBigIntItem::Store(SvStream &rStream, USHORT ) const
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
BOOL SfxBigIntItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE )
{
    double aValue = 0.0;
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
BOOL SfxBigIntItem::QueryValue( ::com::sun::star::uno::Any& rVal,	BYTE ) const
{
    double aValue = GetValue();
    rVal <<= aValue;
    return TRUE;
}

}
