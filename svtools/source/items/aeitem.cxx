/*************************************************************************
 *
 *  $RCSfile: aeitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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

#include <tools/string.hxx>

#include <svarray.hxx>
#include "aeitem.hxx"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxAllEnumItem);

// -----------------------------------------------------------------------

struct SfxAllEnumValue_Impl
{
    USHORT nValue;
    XubString aText;
};

SV_DECL_PTRARR_DEL(SfxAllEnumValueArr, SfxAllEnumValue_Impl*, 0, 8);
SV_IMPL_PTRARR(SfxAllEnumValueArr, SfxAllEnumValue_Impl*);

// -----------------------------------------------------------------------

SfxAllEnumItem::SfxAllEnumItem( USHORT nWhich, USHORT nVal, const XubString &rText ):
    SfxEnumItem(nWhich, nVal),
    pValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
    InsertValue( nVal, rText );
}

// -----------------------------------------------------------------------

SfxAllEnumItem::SfxAllEnumItem(USHORT nWhich, USHORT nVal):
    SfxEnumItem(nWhich, nVal),
    pValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
    InsertValue( nVal );
}

// -----------------------------------------------------------------------

SfxAllEnumItem::SfxAllEnumItem( USHORT nWhich, SvStream &rStream ):
    SfxEnumItem(nWhich, rStream),
    pValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
    InsertValue( GetValue() );
}

// -----------------------------------------------------------------------


SfxAllEnumItem::SfxAllEnumItem(USHORT nWhich):
    SfxEnumItem(nWhich, 0),
    pValues( 0 )
{
    DBG_CTOR(SfxAllEnumItem, 0);
}


// -----------------------------------------------------------------------

SfxAllEnumItem::SfxAllEnumItem(const SfxAllEnumItem &rCopy):
    SfxEnumItem(rCopy),
    pValues(0)
{
    DBG_CTOR(SfxAllEnumItem, 0);
    if ( !rCopy.pValues )
        return;

    pValues = new SfxAllEnumValueArr;
    for ( USHORT nPos = 0; nPos < rCopy.pValues->Count(); ++nPos )
    {
        SfxAllEnumValue_Impl *pVal = new SfxAllEnumValue_Impl;
        pVal->nValue = rCopy.pValues->GetObject(nPos)->nValue;
        pVal->aText = rCopy.pValues->GetObject(nPos)->aText;
        const SfxAllEnumValue_Impl *pTemp = pVal;
        pValues->Insert( pTemp, nPos );
    }
}

// -----------------------------------------------------------------------

SfxAllEnumItem::~SfxAllEnumItem()
{
    DBG_DTOR(SfxAllEnumItem, 0);
    delete pValues;
}

// -----------------------------------------------------------------------

USHORT SfxAllEnumItem::GetValueCount() const
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    return pValues ? pValues->Count() : 0;
}

// -----------------------------------------------------------------------

XubString SfxAllEnumItem::GetValueTextByPos( USHORT nPos ) const
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    DBG_ASSERT( pValues && nPos < pValues->Count(), "enum overflow" );
    return pValues->GetObject(nPos)->aText;
}

// -----------------------------------------------------------------------

USHORT SfxAllEnumItem::GetValueByPos( USHORT nPos ) const
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

SfxPoolItem* SfxAllEnumItem::Create( SvStream & rStream, USHORT nVersion ) const
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    return new SfxAllEnumItem( Which(), rStream );
}


// -----------------------------------------------------------------------

USHORT SfxAllEnumItem::_GetPosByValue( USHORT nVal ) const

/*  [Beschreibung]

    Im Ggs. zu <SfxEnumItemInterface::GetPosByValue(USHORT)const> liefert
    diese interne Methode bei nicht vorhandenen Values die Position,
    an der der Wert liegen w"urde.
*/

{
    DBG_CHKTHIS(SfxAllEnumItem, 0);

    if ( !pValues )
        return 0;

    //!O: binaere Suche oder SortArray verwenden
    USHORT nPos;
    for ( nPos = 0; nPos < pValues->Count(); ++nPos )
        if ( pValues->GetObject(nPos)->nValue >= nVal )
            return nPos;
    return nPos;
}

// -----------------------------------------------------------------------

USHORT SfxAllEnumItem::GetPosByValue( USHORT nValue ) const

/*  [Beschreibung]

    Liefert im Gegensatz zu <SfxEnumItemInterface::GetPosByValue(USHORT)const>
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

void SfxAllEnumItem::InsertValue( USHORT nValue, const XubString &rValue )
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

void SfxAllEnumItem::InsertValue( USHORT nValue )
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

// -----------------------------------------------------------------------

void SfxAllEnumItem::RemoveValue( USHORT nValue )
{
    DBG_CHKTHIS(SfxAllEnumItem, 0);
    USHORT nPos = GetPosByValue(nValue);
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



