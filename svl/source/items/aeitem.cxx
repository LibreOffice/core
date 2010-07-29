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
#include "precompiled_svl.hxx"
#ifndef GCC
#endif

#include <tools/string.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#include <svl/svarray.hxx>
#include <svl/aeitem.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxAllEnumItem)

TYPEINIT1_AUTOFACTORY(SfxAllEnumItem, SfxEnumItem)

// -----------------------------------------------------------------------

struct SfxAllEnumValue_Impl
{
    sal_uInt16 nValue;
    XubString aText;
};

SV_DECL_PTRARR_DEL(SfxAllEnumValueArr, SfxAllEnumValue_Impl*, 0, 8)
SV_IMPL_PTRARR(SfxAllEnumValueArr, SfxAllEnumValue_Impl*)

// -----------------------------------------------------------------------

SfxAllEnumItem::SfxAllEnumItem() :
    SfxEnumItem(),
    pValues( 0 ),
    pDisabledValues( 0 )
{
}

SfxAllEnumItem::SfxAllEnumItem( sal_uInt16 which, sal_uInt16 nVal, const XubString &rText ):
    SfxEnumItem(which, nVal),
    pValues( 0 ),
    pDisabledValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
    InsertValue( nVal, rText );
}

// -----------------------------------------------------------------------

SfxAllEnumItem::SfxAllEnumItem(sal_uInt16 which, sal_uInt16 nVal):
    SfxEnumItem(which, nVal),
    pValues( 0 ),
    pDisabledValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
    InsertValue( nVal );
}

// -----------------------------------------------------------------------

SfxAllEnumItem::SfxAllEnumItem( sal_uInt16 which, SvStream &rStream ):
    SfxEnumItem(which, rStream),
    pValues( 0 ),
    pDisabledValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
    InsertValue( GetValue() );
}

// -----------------------------------------------------------------------


SfxAllEnumItem::SfxAllEnumItem(sal_uInt16 which):
    SfxEnumItem(which, 0),
    pValues( 0 ),
    pDisabledValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
}


// -----------------------------------------------------------------------

SfxAllEnumItem::SfxAllEnumItem(const SfxAllEnumItem &rCopy):
    SfxEnumItem(rCopy),
    pValues(0),
    pDisabledValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
    if ( !rCopy.pValues )
        return;

    pValues = new SfxAllEnumValueArr;

    for ( sal_uInt16 nPos = 0; nPos < rCopy.pValues->Count(); ++nPos )
    {
        SfxAllEnumValue_Impl *pVal = new SfxAllEnumValue_Impl;
        pVal->nValue = rCopy.pValues->GetObject(nPos)->nValue;
        pVal->aText = rCopy.pValues->GetObject(nPos)->aText;
        const SfxAllEnumValue_Impl *pTemp = pVal;
        pValues->Insert( pTemp, nPos );
    }

    if( rCopy.pDisabledValues )
    {
        pDisabledValues = new SvUShorts;
        for ( sal_uInt16 nPos = 0; nPos < rCopy.pDisabledValues->Count(); ++nPos )
        {
            pDisabledValues->Insert( rCopy.pDisabledValues->GetObject(nPos),
                                     nPos );
        }
    }
}

// -----------------------------------------------------------------------

SfxAllEnumItem::~SfxAllEnumItem()
{
    DBG_DTOR(SfxAllEnumItem, 0);
    delete pValues;
    delete pDisabledValues;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxAllEnumItem::GetValueCount() const
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    return pValues ? pValues->Count() : 0;
}

// -----------------------------------------------------------------------

XubString SfxAllEnumItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    DBG_ASSERT( pValues && nPos < pValues->Count(), "enum overflow" );
    return pValues->GetObject(nPos)->aText;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxAllEnumItem::GetValueByPos( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    DBG_ASSERT( pValues && nPos < pValues->Count(), "enum overflow" );
    return pValues->GetObject(nPos)->nValue;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxAllEnumItem::Clone( SfxItemPool * ) const
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    return new SfxAllEnumItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxAllEnumItem::Create( SvStream & rStream, sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    return new SfxAllEnumItem( Which(), rStream );
}


// -----------------------------------------------------------------------

sal_uInt16 SfxAllEnumItem::_GetPosByValue( sal_uInt16 nVal ) const

/*  [Beschreibung]

    Im Ggs. zu <SfxEnumItemInterface::GetPosByValue(sal_uInt16)const> liefert
    diese interne Methode bei nicht vorhandenen Values die Position,
    an der der Wert liegen w"urde.
*/

{
    DBG_CHKTHIS(SfxAllEnumItem, 0);

    if ( !pValues )
        return 0;

    //!O: binaere Suche oder SortArray verwenden
    sal_uInt16 nPos;
    for ( nPos = 0; nPos < pValues->Count(); ++nPos )
        if ( pValues->GetObject(nPos)->nValue >= nVal )
            return nPos;
    return nPos;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxAllEnumItem::GetPosByValue( sal_uInt16 nValue ) const

/*  [Beschreibung]

    Liefert im Gegensatz zu <SfxEnumItemInterface::GetPosByValue(sal_uInt16)const>
    immer nValue zur"uck, solange nicht mindestens ein Wert mit einer der
    Methoden <SfxAllEnumItem::InsertValue()> eingef"ugt wurde.
*/

{
    DBG_CHKTHIS(SfxAllEnumItem, 0);

    if ( !pValues || !pValues->Count() )
        return nValue;

    return SfxEnumItem::GetPosByValue( nValue );
}

// -----------------------------------------------------------------------

void SfxAllEnumItem::InsertValue( sal_uInt16 nValue, const XubString &rValue )
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    SfxAllEnumValue_Impl *pVal = new SfxAllEnumValue_Impl;
    pVal->nValue = nValue;
    pVal->aText = rValue;
    const SfxAllEnumValue_Impl *pTemp = pVal;
    if ( !pValues )
        pValues = new SfxAllEnumValueArr;
    else if ( GetPosByValue( nValue ) != USHRT_MAX )
        // remove when exists
        RemoveValue( nValue );
    // then insert
    pValues->Insert( pTemp, _GetPosByValue(nValue) ); //! doppelte?!
}

// -----------------------------------------------------------------------

void SfxAllEnumItem::InsertValue( sal_uInt16 nValue )
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    SfxAllEnumValue_Impl *pVal = new SfxAllEnumValue_Impl;
    pVal->nValue = nValue;
    pVal->aText = XubString::CreateFromInt32( nValue );
    const SfxAllEnumValue_Impl *pTemp = pVal;
    if ( !pValues )
        pValues = new SfxAllEnumValueArr;

    pValues->Insert( pTemp, _GetPosByValue(nValue) ); //! doppelte?!
}

void SfxAllEnumItem::DisableValue( sal_uInt16 nValue )
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    if ( !pDisabledValues )
        pDisabledValues = new SvUShorts;

    pDisabledValues->Insert( nValue, pDisabledValues->Count() );
}

sal_Bool SfxAllEnumItem::IsEnabled( sal_uInt16 nValue ) const
{
    if ( pDisabledValues )
    {
        for ( sal_uInt16 i=0; i<pDisabledValues->Count(); i++ )
            if ( (*pDisabledValues)[i] == nValue )
                return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

void SfxAllEnumItem::RemoveValue( sal_uInt16 nValue )
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    sal_uInt16 nPos = GetPosByValue(nValue);
    DBG_ASSERT( nPos != USHRT_MAX, "removing value not in enum" );
    pValues->Remove( nPos );
}

// -----------------------------------------------------------------------


void SfxAllEnumItem::RemoveAllValues()
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    if ( pValues )
        pValues->DeleteAndDestroy( 0, pValues->Count() );
}



