/*************************************************************************
 *
 *  $RCSfile: xattr.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-08 12:18:58 $
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

// include ---------------------------------------------------------------

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DASHSTYLE_HPP_
#include <com/sun/star/drawing/DashStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#include <tools/stream.hxx>

#ifndef SVX_LIGHT
#include "unoapi.hxx"
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _SVX_UNOPOLYHELPER_HXX
#include "unopolyhelper.hxx"
#endif

#include <tools/bigint.hxx>
#include <svtools/itemset.hxx>
#include "dialogs.hrc"
#include "svdstr.hrc"
#include "xattr.hxx"
#include "xtable.hxx"
#include "xoutx.hxx"
#include "dialmgr.hxx"
#include "itemtype.hxx"
#include "xdef.hxx"
#include "unomid.hxx"

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#define GLOBALOVERFLOW

/************************************************************************/

#ifdef VCL
#define VCLTOSVCOL( rCol ) (USHORT)((((USHORT)(rCol))<<8)|(rCol))
#endif

/************************************************************************/

XubString aNameOrIndexEmptyString;

TYPEINIT1_AUTOFACTORY(NameOrIndex, SfxStringItem);

/*************************************************************************
|*
|*
|*
\*************************************************************************/

long ScaleMetricValue( long nVal, long nMul, long nDiv )
{
    BigInt aVal( nVal );

    aVal *= nMul;

    if ( aVal.IsNeg() != ( nDiv < 0 ) )
        aVal-=nDiv/2; // fuer korrektes Runden
    else
        aVal+=nDiv/2; // fuer korrektes Runden

    aVal/=nDiv;

    return long( aVal );
}

/*************************************************************************
|*
|*    NameOrIndex::NameOrIndex(USHORT nWhich, long nIndex)
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

NameOrIndex::NameOrIndex(USHORT nWhich, long nIndex) :
    SfxStringItem(nWhich, aNameOrIndexEmptyString),
    nPalIndex(nIndex)
{
}

/*************************************************************************
|*
|*    NameOrIndex::NameOrIndex(USHORT nWhich, const String& rName)
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

NameOrIndex::NameOrIndex(USHORT nWhich, const XubString& rName) :
    SfxStringItem(nWhich, rName),
    nPalIndex((long)-1)
{
}

/*************************************************************************
|*
|*    NameOrIndex::NameOrIndex(USHORT nWhich, SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

NameOrIndex::NameOrIndex(USHORT nWhich, SvStream& rIn) :
    SfxStringItem(nWhich, rIn)
{
    rIn >> nPalIndex;
}

/*************************************************************************
|*
|*    NameOrIndex::NameOrIndex(const NameOrIndex& rNameOrIndex)
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

NameOrIndex::NameOrIndex(const NameOrIndex& rNameOrIndex) :
    SfxStringItem(rNameOrIndex),
    nPalIndex(rNameOrIndex.nPalIndex)
{
}

/*************************************************************************
|*
|*    int NameOrIndex::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

int NameOrIndex::operator==(const SfxPoolItem& rItem) const
{
    return ( SfxStringItem::operator==(rItem) &&
            ((const NameOrIndex&) rItem).nPalIndex == nPalIndex );
}

/*************************************************************************
|*
|*    SfxPoolItem* NameOrIndex::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

SfxPoolItem* NameOrIndex::Clone(SfxItemPool* pPool) const
{

    return new NameOrIndex(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* NameOrIndex::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

SfxPoolItem* NameOrIndex::Create(SvStream& rIn, USHORT nVer) const
{
    return new NameOrIndex(Which(), rIn);
}

/*************************************************************************
|*
|*    SvStream* NameOrIndex::Store(SvStream& rIn) const
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

SvStream& NameOrIndex::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    SfxStringItem::Store( rOut, nItemVersion );
    rOut << (INT32)nPalIndex;
    return rOut;
}

/** this static checks if the given NameOrIndex item has a unique name for its value.
    The returned String is a unique name for an item with this value in both given pools.
    Argument pPool2 can be null.
    If returned string equals NameOrIndex->GetName(), the name was already unique.
*/
String NameOrIndex::CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, const SfxItemPool* pPool2, SvxCompareValueFunc pCompareValueFunc, USHORT nPrefixResId, XPropertyList* pDefaults )
{
    sal_Bool bForceNew = sal_False;

    String aUniqueName( pCheckItem->GetName() );

    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different line end or start

    if( aUniqueName.Len() && pPool1 )
    {
        const sal_uInt16 nCount = pPool1->GetItemCount( nWhich );

        const NameOrIndex *pItem;
        for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (NameOrIndex*)pPool1->GetItem( nWhich, nSurrogate );

            if( pItem && ( pItem->GetName() == pCheckItem->GetName() ) )
            {
                // if there is already an item with the same name and the same
                // value its ok to set it
                if( !pCompareValueFunc( pItem, pCheckItem ) )
                {
                    // same name but different value, we need a new name for this item
                    aUniqueName = String();
                    bForceNew = sal_True;
                }
                break;
            }
        }
    }

    if( aUniqueName.Len() && pPool2 )
    {
        const sal_uInt16 nCount = pPool2->GetItemCount( nWhich );

        const NameOrIndex *pItem;
        for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (NameOrIndex*)pPool2->GetItem( nWhich, nSurrogate );

            if( pItem && ( pItem->GetName() == pCheckItem->GetName() ) )
            {
                // if there is already an item with the same name and the same
                // value its ok to set it
                if( !pCompareValueFunc( pItem, pCheckItem ) )
                {
                    // same name but different value, we need a new name for this item
                    aUniqueName = String();
                    bForceNew = sal_True;
                }
                break;
            }
        }
    }

    // if we have no name yet, find existing item with same conent or
    // create a unique name
    if( aUniqueName.Len() == 0 )
    {
        sal_Int32 nUserIndex = 1;
        const ResId aRes(SVX_RES(nPrefixResId));
        const String aUser( aRes );

        if( pPool1 )
        {
            const sal_uInt16 nCount = pPool1->GetItemCount( nWhich );
            const NameOrIndex *pItem;
            for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (NameOrIndex*)pPool1->GetItem( nWhich, nSurrogate );

                if( pItem && pItem->GetName().Len() )
                {
                    if( !bForceNew && pCompareValueFunc( pItem, pCheckItem ) )
                        return pItem->GetName();

                    if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                    {
                        sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                        if( nThisIndex >= nUserIndex )
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }
        }

        if( pPool2 )
        {
            const sal_uInt16 nCount = pPool2->GetItemCount( nWhich );
            const NameOrIndex *pItem;
            for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (NameOrIndex*)pPool2->GetItem( nWhich, nSurrogate );

                if( pItem && pItem->GetName().Len() )
                {
                    if( !bForceNew && pCompareValueFunc( pItem, pCheckItem ) )
                        return pItem->GetName();

                    if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                    {
                        sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                        if( nThisIndex >= nUserIndex )
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }
        }

        aUniqueName = aUser;
        aUniqueName += String::CreateFromInt32( nUserIndex );
    }

    return aUniqueName;
}

//*************************************************************************

// -------------------
// class XColorItem
// -------------------
TYPEINIT1_AUTOFACTORY(XColorItem, NameOrIndex);

/*************************************************************************
|*
|*    XColorItem::XColorItem(USHORT nWhich, long nIndex, const Color& rTheColor)
|*
\************************************************************************/

XColorItem::XColorItem(USHORT nWhich, long nIndex, const Color& rTheColor) :
    NameOrIndex(nWhich, nIndex),
    aColor(rTheColor)
{
}

/*************************************************************************
|*
|*    XColorItem::XColorItem(USHORT nWhich, const String& rName, const Color& rTheColor)
|*
\************************************************************************/

XColorItem::XColorItem(USHORT nWhich, const XubString& rName, const Color& rTheColor) :
    NameOrIndex(nWhich, rName),
    aColor(rTheColor)
{
}

/*************************************************************************
|*
|*    XColorItem::XColorItem(const XColorItem& rItem)
|*
\************************************************************************/

XColorItem::XColorItem(const XColorItem& rItem) :
    NameOrIndex(rItem),
    aColor(rItem.aColor)
{
}

/*************************************************************************
|*
|*    XColorItem::XColorItem(USHORT nWhich, SvStream& rIn)
|*
\************************************************************************/

XColorItem::XColorItem(USHORT nWhich, SvStream& rIn) :
    NameOrIndex(nWhich, rIn)
{
    if (!IsIndex())
    {
        rIn >> aColor;
    }
}

/*************************************************************************
|*
|*    XColorItem::Clone(SfxItemPool* pPool) const
|*
\************************************************************************/

SfxPoolItem* XColorItem::Clone(SfxItemPool* pPool) const
{
    return new XColorItem(*this);
}

/*************************************************************************
|*
|*    int XColorItem::operator==(const SfxPoolItem& rItem) const
|*
\************************************************************************/

int XColorItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
            ((const XColorItem&) rItem).aColor == aColor );
}

/*************************************************************************
|*
|*    SfxPoolItem* XColorItem::Create(SvStream& rIn, USHORT nVer) const
|*
\************************************************************************/

SfxPoolItem* XColorItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XColorItem(Which(), rIn);
}

/*************************************************************************
|*
|*    SfxPoolItem* XColorItem::Store(SvStream& rOut) const
|*
\************************************************************************/

SvStream& XColorItem::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if ( !IsIndex() )
    {
        rOut << aColor;
    }

    return rOut;
}

/*************************************************************************
|*
|*    const XColor& XColorItem::GetValue(const XColorTable* pTable) const
|*
\************************************************************************/

const Color& XColorItem::GetValue(const XColorTable* pTable) const
{
    if (!IsIndex())
        return aColor;
    else
        return pTable->Get(GetIndex())->GetColor();

}

sal_Bool XColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue().GetRGBColor();
    return sal_True;
}

sal_Bool XColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    rVal >>= nValue;
    SetValue( nValue );

    return sal_True;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                           Linienattribute
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------
// class XLineStyleItem
//---------------------
TYPEINIT1_AUTOFACTORY(XLineStyleItem, SfxEnumItem);

/*************************************************************************
|*
|*    XLineStyleItem::XLineStyleItem(XLineStyle eTheLineStyle)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineStyleItem::XLineStyleItem(XLineStyle eTheLineStyle) :
    SfxEnumItem(XATTR_LINESTYLE, eTheLineStyle)
{
}

/*************************************************************************
|*
|*    XLineStyleItem::XLineStyleItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineStyleItem::XLineStyleItem(SvStream& rIn) :
    SfxEnumItem(XATTR_LINESTYLE, rIn)
{
}

/*************************************************************************
|*
|*    XLineStyleItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    09.11.94
|*    Letzte Aenderung  09.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStyleItem::Clone(SfxItemPool* pPool) const
{
    return new XLineStyleItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStyleItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStyleItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineStyleItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineStyleItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            USHORT nId = 0;

            switch( (USHORT)GetValue() )
            {
                case XLINE_NONE:
                    nId = RID_SVXSTR_INVISIBLE;
                    break;
                case XLINE_SOLID:
                    nId = RID_SVXSTR_SOLID;
                    break;
            }

            if ( nId )
                rText = SVX_RESSTR( nId );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineStyleItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    ::com::sun::star::drawing::LineStyle eLS = (::com::sun::star::drawing::LineStyle)GetValue();
    rVal <<= eLS;
    return sal_True;
}

sal_Bool XLineStyleItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    ::com::sun::star::drawing::LineStyle eLS;
    if(!(rVal >>= eLS ))
        return sal_False;

    SetValue( (XLineStyle)eLS );
    return sal_True;
}

//------------------------------------------------------------------------

USHORT XLineStyleItem::GetValueCount() const
{
    return 3;
}


// ------------
// class XDash
// ------------
/*************************************************************************
|*
|*    XDash::XDash(XDashStyle, USHORT, ULONG, USHORT, ULONG, ULONG)
|*
|*    Beschreibung
|*    Ersterstellung    21.11.94
|*    Letzte Aenderung  21.11.94
|*
*************************************************************************/

XDash::XDash(XDashStyle eTheDash, USHORT nTheDots, ULONG nTheDotLen,
             USHORT nTheDashes, ULONG nTheDashLen, ULONG nTheDistance) :
    eDash(eTheDash),
    nDots(nTheDots),
    nDotLen(nTheDotLen),
    nDashes(nTheDashes),
    nDashLen(nTheDashLen),
    nDistance(nTheDistance)
{
}

/*************************************************************************
|*
|*    int XDash::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    29.11.94
|*    Letzte Aenderung  29.11.94
|*
*************************************************************************/

int XDash::operator==(const XDash& rDash) const
{
    return ( eDash      == rDash.eDash      &&
             nDots      == rDash.nDots      &&
             nDotLen    == rDash.nDotLen    &&
             nDashes    == rDash.nDashes    &&
             nDashLen   == rDash.nDashLen   &&
             nDistance  == rDash.nDistance );
}


// -------------------
// class XLineDashItem
// -------------------
TYPEINIT1_AUTOFACTORY(XLineDashItem, NameOrIndex);

/*************************************************************************
|*
|*    XLineDashItem::XLineDashItem(long nIndex, const XDash& rTheDash)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineDashItem::XLineDashItem(long nIndex, const XDash& rTheDash) :
    NameOrIndex(XATTR_LINEDASH, nIndex),
    aDash(rTheDash)
{
}

/*************************************************************************
|*
|*    XLineDashItem::XLineDashItem(const String& rName, const XDash& rTheDash)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineDashItem::XLineDashItem(const XubString& rName, const XDash& rTheDash) :
    NameOrIndex(XATTR_LINEDASH, rName),
    aDash(rTheDash)
{
}

/*************************************************************************
|*
|*    XLineDashItem::XLineDashItem(const XLineDashItem& rItem)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineDashItem::XLineDashItem(const XLineDashItem& rItem) :
    NameOrIndex(rItem),
    aDash(rItem.aDash)
{
}

/*************************************************************************
|*
|*    XLineDashItem::XLineDashItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineDashItem::XLineDashItem(SvStream& rIn) :
    NameOrIndex(XATTR_LINEDASH, rIn)
{
    if (!IsIndex())
    {
        USHORT  nSTemp;
        UINT32  nLTemp;
        INT32   nITemp;

        rIn >> nITemp; aDash.SetDashStyle((XDashStyle)nITemp);
        rIn >> nSTemp; aDash.SetDots(nSTemp);
        rIn >> nLTemp; aDash.SetDotLen(nLTemp);
        rIn >> nSTemp; aDash.SetDashes(nSTemp);
        rIn >> nLTemp; aDash.SetDashLen(nLTemp);
        rIn >> nLTemp; aDash.SetDistance(nLTemp);
    }
}

//*************************************************************************

XLineDashItem::XLineDashItem(SfxItemPool* pPool, const XDash& rTheDash)
:   NameOrIndex( XATTR_LINEDASH, -1 ),
    aDash(rTheDash)
{
}

//*************************************************************************

XLineDashItem::XLineDashItem(SfxItemPool* pPool )
: NameOrIndex(XATTR_LINEDASH, -1 )
{
}

/*************************************************************************
|*
|*    XLineDashItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineDashItem::Clone(SfxItemPool* pPool) const
{
    return new XLineDashItem(*this);
}

/*************************************************************************
|*
|*    int XLineDashItem::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

int XLineDashItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             aDash == ((const XLineDashItem&) rItem).aDash );
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineDashItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineDashItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineDashItem(rIn);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineDashItem::Store(SvStream& rOut) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SvStream& XLineDashItem::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut << (INT32) aDash.GetDashStyle();
        rOut << aDash.GetDots();
        rOut << (UINT32) aDash.GetDotLen();
        rOut << aDash.GetDashes();
        rOut << (UINT32) aDash.GetDashLen();
        rOut << (UINT32) aDash.GetDistance();
    }

    return rOut;
}

/*************************************************************************
|*
|*    const XDash& XLineDashItem::GetValue(const XDashTable* pTable) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

const XDash& XLineDashItem::GetValue(const XDashTable* pTable) const
{
    if (!IsIndex())
        return aDash;
    else
        return pTable->Get(GetIndex())->GetDash();
}

//------------------------------------------------------------------------

SfxItemPresentation XLineDashItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetName();
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

FASTBOOL XLineDashItem::HasMetrics() const
{
    return TRUE;
}

//------------------------------------------------------------------------

FASTBOOL XLineDashItem::ScaleMetrics(long nMul, long nDiv)
{
    aDash.SetDotLen( ScaleMetricValue( aDash.GetDotLen(), nMul, nDiv ) );
    aDash.SetDashLen( ScaleMetricValue( aDash.GetDashLen(), nMul, nDiv ) );
    aDash.SetDistance( ScaleMetricValue( aDash.GetDistance(), nMul, nDiv ) );
    return TRUE;
}

sal_Bool XLineDashItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    if( nMemberId == MID_NAME )
    {
        rtl::OUString aApiName;
#ifndef SVX_LIGHT
        SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
#endif
        rVal <<= aApiName;
    }
    else
    {
        const XDash& rXD = GetValue();

        ::com::sun::star::drawing::LineDash aLineDash;

        aLineDash.Style = (::com::sun::star::drawing::DashStyle)((UINT16)rXD.GetDashStyle());
        aLineDash.Dots = rXD.GetDots();
        aLineDash.DotLen = rXD.GetDotLen();
        aLineDash.Dashes = rXD.GetDashes();
        aLineDash.DashLen = rXD.GetDashLen();
        aLineDash.Distance = rXD.GetDistance();

        rVal <<= aLineDash;
    }

    return sal_True;
}

sal_Bool XLineDashItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    if( nMemberId == MID_NAME )
    {
        rtl::OUString aName;
        rVal >>= aName;
        SetName( aName );
    }
    else
    {
        ::com::sun::star::drawing::LineDash aLineDash;
        if(!(rVal >>= aLineDash))
            return sal_False;

        XDash aXDash;

        aXDash.SetDashStyle((XDashStyle)((UINT16)(aLineDash.Style)));
        aXDash.SetDots(aLineDash.Dots);
        aXDash.SetDotLen(aLineDash.DotLen);
        aXDash.SetDashes(aLineDash.Dashes);
        aXDash.SetDashLen(aLineDash.DashLen);
        aXDash.SetDistance(aLineDash.Distance);
        SetValue( aXDash );
    }

    return sal_True;
}

BOOL XLineDashItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return ((XLineDashItem*)p1)->GetValue() == ((XLineDashItem*)p2)->GetValue();
}

XLineDashItem* XLineDashItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const String aUniqueName = NameOrIndex::CheckNamedItem( this,
                                                                XATTR_LINEDASH,
                                                                &pModel->GetItemPool(),
                                                                pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                                                                XLineDashItem::CompareValueFunc,
                                                                RID_SVXSTR_DASH11,
                                                                pModel->GetDashList() );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
        {
            return new XLineDashItem( aUniqueName, aDash );
        }
    }

    return (XLineDashItem*)this;
}

// -------------------
// class XLineWidthItem
// -------------------
TYPEINIT1_AUTOFACTORY(XLineWidthItem, SfxMetricItem);

/*************************************************************************
|*
|*    XLineWidthItem::XLineWidthItem(long nWidth)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineWidthItem::XLineWidthItem(long nWidth) :
    SfxMetricItem(XATTR_LINEWIDTH, nWidth)
{
}

/*************************************************************************
|*
|*    XLineWidthItem::XLineWidthItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineWidthItem::XLineWidthItem(SvStream& rIn) :
    SfxMetricItem(XATTR_LINEWIDTH, rIn)
{
}

/*************************************************************************
|*
|*    XLineWidthItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineWidthItem::Clone(SfxItemPool* pPool) const
{
    return new XLineWidthItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineWidthItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineWidthItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineWidthItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineWidthItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetMetricText( (long) GetValue(),
                                    eCoreUnit, ePresUnit);
            rText += SVX_RESSTR( GetMetricId( ePresUnit) );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

sal_Bool XLineWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    rVal >>= nValue;

    SetValue( nValue );
    return sal_True;
}

// -------------------
// class XLineColorItem
// -------------------
TYPEINIT1_AUTOFACTORY(XLineColorItem, XColorItem);

/*************************************************************************
|*
|*    XLineColorItem::XLineColorItem(long nIndex, const Color& rTheColor)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineColorItem::XLineColorItem(long nIndex, const Color& rTheColor) :
    XColorItem(XATTR_LINECOLOR, nIndex, rTheColor)
{
}

/*************************************************************************
|*
|*    XLineColorItem::XLineColorItem(const XubString& rName, const Color& rTheColor)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineColorItem::XLineColorItem(const XubString& rName, const Color& rTheColor) :
    XColorItem(XATTR_LINECOLOR, rName, rTheColor)
{
}

/*************************************************************************
|*
|*    XLineColorItem::XLineColorItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineColorItem::XLineColorItem(SvStream& rIn) :
    XColorItem(XATTR_LINECOLOR, rIn)
{
}

/*************************************************************************
|*
|*    XLineColorItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineColorItem::Clone(SfxItemPool* pPool) const
{
    return new XLineColorItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineColorItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineColorItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineColorItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetName();
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue().GetRGBColor();
    return sal_True;
}

sal_Bool XLineColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}

// -----------------------
// class XLineStartItem
// -----------------------
TYPEINIT1_AUTOFACTORY(XLineStartItem, NameOrIndex);

/*************************************************************************
|*
|*    XLineStartItem::XLineStartItem(long nIndex)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineStartItem::XLineStartItem(long nIndex) :
    NameOrIndex(XATTR_LINESTART, nIndex)
{
}

/*************************************************************************
|*
|*    XLineStartItem::XLineStartItem(const XubString& rName,
|*                                   const XPolygon& rXPolygon)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineStartItem::XLineStartItem(const XubString& rName,
                               const XPolygon& rXPolygon):
    NameOrIndex(XATTR_LINESTART, rName),
    aXPolygon(rXPolygon)
{
}

/*************************************************************************
|*
|*    XLineStartItem::XLineStartItem(const XLineStartItem& rItem)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

XLineStartItem::XLineStartItem(const XLineStartItem& rItem) :
    NameOrIndex(rItem),
    aXPolygon(rItem.aXPolygon)
{
}

/*************************************************************************
|*
|*    XLineStartItem::XLineStartItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

XLineStartItem::XLineStartItem(SvStream& rIn) :
    NameOrIndex(XATTR_LINESTART, rIn)
{
    if (!IsIndex())
    {
        UINT32 nPoints;
        INT32  nFlags;
        rIn >> nPoints;
        aXPolygon.SetSize((USHORT)nPoints);
        for (USHORT nIndex = 0; nIndex < (USHORT)nPoints; nIndex++)
        {
            rIn >> aXPolygon[nIndex].X();
            rIn >> aXPolygon[nIndex].Y();
            rIn >> nFlags; aXPolygon.SetFlags(nIndex, (XPolyFlags)nFlags);
        }
    }
}

//*************************************************************************

XLineStartItem::XLineStartItem(SfxItemPool* pPool, const XPolygon& rXPolygon)
:   NameOrIndex( XATTR_LINESTART, -1 ),
    aXPolygon(rXPolygon)
{
}

//*************************************************************************

XLineStartItem::XLineStartItem(SfxItemPool* pPool )
: NameOrIndex(XATTR_LINESTART, -1 )
{
}

/*************************************************************************
|*
|*    XLineStartItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartItem::Clone(SfxItemPool* pPool) const
{
    return new XLineStartItem(*this);
}

/*************************************************************************
|*
|*    int XLineStartItem::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

int XLineStartItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
            ((const XLineStartItem&) rItem).aXPolygon == aXPolygon );
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStartItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineStartItem(rIn);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStartItem::Store(SvStream& rOut) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

SvStream& XLineStartItem::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        UINT32 nPoints = (UINT32)aXPolygon.GetPointCount();
        rOut << nPoints;
        for (USHORT nIndex = 0; nIndex < (USHORT)nPoints; nIndex++)
        {
            rOut << aXPolygon[nIndex].X();
            rOut << aXPolygon[nIndex].Y();
            rOut << (INT32)aXPolygon.GetFlags(nIndex);
        }
    }

    return rOut;
}

/*************************************************************************
|*
|*    const XPolygon& XLineStartItem::GetValue(const XLineEndTable* pTable)
|*                                             const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

const XPolygon& XLineStartItem::GetValue(const XLineEndTable* pTable) const
{
    if (!IsIndex())
        return aXPolygon;
    else
        return pTable->Get(GetIndex())->GetLineEnd();
}

//------------------------------------------------------------------------

SfxItemPresentation XLineStartItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetName();
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineStartItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
#ifndef SVX_LIGHT
    if( nMemberId == MID_NAME )
    {
        rtl::OUString aApiName;
        SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
        rVal <<= aApiName;
    }
    else
    {
        com::sun::star::drawing::PolyPolygonBezierCoords aBezier;
        SvxConvertXPolygonToPolyPolygonBezier( aXPolygon, aBezier );
        rVal <<= aBezier;
    }

#endif
    return sal_True;
}

sal_Bool XLineStartItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
#ifndef SVX_LIGHT
    if( nMemberId == MID_NAME )
    {
        return sal_False;
    }
    else
    {
        aXPolygon.SetSize( 0 );
        if( rVal.hasValue() && rVal.getValue() )
        {
            if( rVal.getValueType() != ::getCppuType((const com::sun::star::drawing::PolyPolygonBezierCoords*)0) )
                return sal_False;

            aXPolygon.SetSize(0);
            com::sun::star::drawing::PolyPolygonBezierCoords* pCoords = (com::sun::star::drawing::PolyPolygonBezierCoords*)rVal.getValue();
            if( pCoords->Coordinates.getLength() > 0 )
                SvxConvertPolyPolygonBezierToXPolygon( pCoords, aXPolygon );
        }
    }
#endif

    return sal_True;
}

/** this function searches in both the models pool and the styles pool for XLineStartItem
    and XLineEndItem with the same value or name and returns an item with the value of
    this item and a unique name for an item with this value. */
XLineStartItem* XLineStartItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        XLineStartItem* pTempItem = NULL;
        const XLineStartItem* pLineStartItem = this;

        String aUniqueName( GetName() );

        if( aXPolygon.GetPointCount() == 0 )
        {
            // if the polygon is empty, check if the name is empty
            if( aUniqueName.Len() == 0 )
                return (XLineStartItem*)this;

            // force empty name for empty polygons
            return new XLineStartItem( String(), aXPolygon );
        }

        if( aXPolygon.GetPointCount() > 1 )
        {
            // check if the polygon is closed
            if( aXPolygon[0] != aXPolygon[aXPolygon.GetPointCount() - 1] )
            {
                // force a closed polygon
                XPolygon aNewPolygon( aXPolygon );
                aNewPolygon[ aXPolygon.GetPointCount() ] = aXPolygon[0];
                pTempItem = new XLineStartItem( aUniqueName, aNewPolygon );
                pLineStartItem = pTempItem;
            }
        }

        sal_Bool bForceNew = sal_False;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        sal_uInt16 nCount, nSurrogate;

        const SfxItemPool* pPool1 = &pModel->GetItemPool();
        if( aUniqueName.Len() && pPool1 )
        {
            nCount = pPool1->GetItemCount( XATTR_LINESTART );

            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetValue() != pLineStartItem->GetValue() )
                    {
                        // same name but different value, we need a new name for this item
                        aUniqueName = String();
                        bForceNew = sal_True;
                    }
                    break;
                }
            }

            if( !bForceNew )
            {
                nCount = pPool1->GetItemCount( XATTR_LINEEND );

                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetValue() != pLineStartItem->GetValue() )
                        {
                            // same name but different value, we need a new name for this item
                            aUniqueName = String();
                            bForceNew = sal_True;
                        }
                        break;
                    }
                }
            }
        }

        const SfxItemPool* pPool2 = pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL;
        if( aUniqueName.Len() && pPool2)
        {
            nCount = pPool2->GetItemCount( XATTR_LINESTART );
            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = (const XLineStartItem*)pPool2->GetItem( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetValue() != pLineStartItem->GetValue() )
                    {
                        // same name but different value, we need a new name for this item
                        aUniqueName = String();
                        bForceNew = sal_True;
                    }
                    break;
                }
            }

            if( !bForceNew )
            {
                nCount = pPool2->GetItemCount( XATTR_LINEEND );
                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool2->GetItem( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetValue() != pLineStartItem->GetValue() )
                        {
                            // same name but different value, we need a new name for this item
                            aUniqueName = String();
                            bForceNew = sal_True;
                        }
                        break;
                    }
                }
            }
        }

        // if we have no name yet, find existing item with same conent or
        // create a unique name
        if( aUniqueName.Len() == 0 )
        {
            sal_Bool bFoundExisting = sal_False;

            sal_Int32 nUserIndex = 1;
            const ResId aRes(SVX_RES(RID_SVXSTR_LINEEND));
            const String aUser( aRes );

            if( pPool1 )
            {
                nCount = pPool1->GetItemCount( XATTR_LINESTART );
                for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem( XATTR_LINESTART, nSurrogate );

                    if( pItem && pItem->GetName().Len() )
                    {
                        if( !bForceNew && pItem->GetValue() == pLineStartItem->GetValue() )
                        {
                            aUniqueName = pItem->GetName();
                            bFoundExisting = sal_True;
                            break;
                        }

                        if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                        {
                            sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }

                nCount = pPool1->GetItemCount( XATTR_LINEEND );
                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem( XATTR_LINEEND, nSurrogate );

                    if( pItem && pItem->GetName().Len() )
                    {
                        if( !bForceNew && pItem->GetValue() == pLineStartItem->GetValue() )
                        {
                            aUniqueName = pItem->GetName();
                            bFoundExisting = sal_True;
                            break;
                        }

                        if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                        {
                            sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }
            }

            if( !bFoundExisting )
            {
                aUniqueName = aUser;
                aUniqueName += sal_Unicode(' ');
                aUniqueName += String::CreateFromInt32( nUserIndex );
            }
        }

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() || pTempItem )
        {
            if( pTempItem )
            {
                pTempItem->SetName( aUniqueName );
                return pTempItem;
            }
            else
            {
                return new XLineStartItem( aUniqueName, aXPolygon );
            }
        }
    }

    return (XLineStartItem*)this;
}

// ---------------------
// class XLineEndItem
// ---------------------
TYPEINIT1_AUTOFACTORY(XLineEndItem, NameOrIndex);

/*************************************************************************
|*
|*    XLineEndItem::XLineEndItem(long nIndex)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineEndItem::XLineEndItem(long nIndex) :
    NameOrIndex(XATTR_LINEEND, nIndex)
{
}

/*************************************************************************
|*
|*    XLineEndItem::XLineEndItem(const XubString& rName,
|*                               const XPolygon& rXPolygon)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineEndItem::XLineEndItem(const XubString& rName, const XPolygon& rPolygon) :
    NameOrIndex(XATTR_LINEEND, rName),
    aXPolygon(rPolygon)
{
}

/*************************************************************************
|*
|*    XLineEndItem::XLineEndItem(const XLineEndItem& rItem)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineEndItem::XLineEndItem(const XLineEndItem& rItem) :
    NameOrIndex(rItem),
    aXPolygon(rItem.aXPolygon)
{
}

/*************************************************************************
|*
|*    XLineEndItem::XLineEndItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

XLineEndItem::XLineEndItem(SvStream& rIn) :
    NameOrIndex(XATTR_LINEEND, rIn)
{
    if (!IsIndex())
    {
        UINT32 nPoints;
        INT32  nFlags;
        rIn >> nPoints;
        aXPolygon.SetSize((USHORT)nPoints);
        for (USHORT nIndex = 0; nIndex < (USHORT)nPoints; nIndex++)
        {
            rIn >> aXPolygon[nIndex].X();
            rIn >> aXPolygon[nIndex].Y();
            rIn >> nFlags; aXPolygon.SetFlags(nIndex, (XPolyFlags)nFlags);
        }
    }
}

//*************************************************************************

XLineEndItem::XLineEndItem(SfxItemPool* pPool, const XPolygon& rXPolygon)
:   NameOrIndex( XATTR_LINEEND, -1 ),
    aXPolygon(rXPolygon)
{
}

//*************************************************************************

XLineEndItem::XLineEndItem(SfxItemPool* pPool )
: NameOrIndex(XATTR_LINEEND, -1 )
{
}

/*************************************************************************
|*
|*    XLineEndItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndItem::Clone(SfxItemPool* pPool) const
{
    return new XLineEndItem(*this);
}

/*************************************************************************
|*
|*    int XLineEndItem::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

int XLineEndItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
            ((const XLineEndItem&) rItem).aXPolygon == aXPolygon );
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineEndItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineEndItem(rIn);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineEndItem::Store(SvStream& rOut) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

SvStream& XLineEndItem::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        UINT32 nPoints = (UINT32)aXPolygon.GetPointCount();
        rOut << nPoints;
        for (USHORT nIndex = 0; nIndex < (USHORT)nPoints; nIndex++)
        {
            rOut << aXPolygon[nIndex].X();
            rOut << aXPolygon[nIndex].Y();
            rOut << (INT32)aXPolygon.GetFlags(nIndex);
        }
    }

    return rOut;
}

/*************************************************************************
|*
|*    const XPolygon& XLineEndItem::GetValue(const XLineEndTable* pTable) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

const XPolygon& XLineEndItem::GetValue(const XLineEndTable* pTable) const
{
    if (!IsIndex())
        return aXPolygon;
    else
        return pTable->Get(GetIndex())->GetLineEnd();
}


/** this function searches in both the models pool and the styles pool for XLineStartItem
    and XLineEndItem with the same value or name and returns an item with the value of
    this item and a unique name for an item with this value. */
XLineEndItem* XLineEndItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        XLineEndItem* pTempItem = NULL;
        const XLineEndItem* pLineEndItem = this;

        String aUniqueName( GetName() );

        if( aXPolygon.GetPointCount() == 0 )
        {
            // if the polygon is empty, check if the name is empty
            if( aUniqueName.Len() == 0 )
                return (XLineEndItem*)this;

            // force empty name for empty polygons
            return new XLineEndItem( String(), aXPolygon );
        }

        if( aXPolygon.GetPointCount() > 1 )
        {
            // check if the polygon is closed
            if( aXPolygon[0] != aXPolygon[aXPolygon.GetPointCount() - 1] )
            {
                // force a closed polygon
                XPolygon aNewPolygon( aXPolygon );
                aNewPolygon[ aXPolygon.GetPointCount() ] = aXPolygon[0];
                pTempItem = new XLineEndItem( aUniqueName, aNewPolygon );
                pLineEndItem = pTempItem;
            }
        }

        sal_Bool bForceNew = sal_False;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        sal_uInt16 nCount, nSurrogate;

        const SfxItemPool* pPool1 = &pModel->GetItemPool();
        if( aUniqueName.Len() && pPool1 )
        {
            nCount = pPool1->GetItemCount( XATTR_LINESTART );

            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetValue() != pLineEndItem->GetValue() )
                    {
                        // same name but different value, we need a new name for this item
                        aUniqueName = String();
                        bForceNew = sal_True;
                    }
                    break;
                }
            }

            if( !bForceNew )
            {
                nCount = pPool1->GetItemCount( XATTR_LINEEND );

                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetValue() != pLineEndItem->GetValue() )
                        {
                            // same name but different value, we need a new name for this item
                            aUniqueName = String();
                            bForceNew = sal_True;
                        }
                        break;
                    }
                }
            }
        }

        const SfxItemPool* pPool2 = pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL;
        if( aUniqueName.Len() && pPool2)
        {
            nCount = pPool2->GetItemCount( XATTR_LINESTART );
            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = (const XLineStartItem*)pPool2->GetItem( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetValue() != pLineEndItem->GetValue() )
                    {
                        // same name but different value, we need a new name for this item
                        aUniqueName = String();
                        bForceNew = sal_True;
                    }
                    break;
                }
            }

            if( !bForceNew )
            {
                nCount = pPool2->GetItemCount( XATTR_LINEEND );
                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool2->GetItem( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetValue() != pLineEndItem->GetValue() )
                        {
                            // same name but different value, we need a new name for this item
                            aUniqueName = String();
                            bForceNew = sal_True;
                        }
                        break;
                    }
                }
            }
        }

        // if we have no name yet, find existing item with same conent or
        // create a unique name
        if( aUniqueName.Len() == 0 )
        {
            sal_Bool bFoundExisting = sal_False;

            sal_Int32 nUserIndex = 1;
            const ResId aRes(SVX_RES(RID_SVXSTR_LINEEND));
            const String aUser( aRes );

            if( pPool1 )
            {
                nCount = pPool1->GetItemCount( XATTR_LINESTART );
                for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem( XATTR_LINESTART, nSurrogate );

                    if( pItem && pItem->GetName().Len() )
                    {
                        if( !bForceNew && pItem->GetValue() == pLineEndItem->GetValue() )
                        {
                            aUniqueName = pItem->GetName();
                            bFoundExisting = sal_True;
                            break;
                        }

                        if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                        {
                            sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }

                nCount = pPool1->GetItemCount( XATTR_LINEEND );
                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem( XATTR_LINEEND, nSurrogate );

                    if( pItem && pItem->GetName().Len() )
                    {
                        if( !bForceNew && pItem->GetValue() == pLineEndItem->GetValue() )
                        {
                            aUniqueName = pItem->GetName();
                            bFoundExisting = sal_True;
                            break;
                        }

                        if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                        {
                            sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }
            }

            if( !bFoundExisting )
            {
                aUniqueName = aUser;
                aUniqueName += sal_Unicode(' ');
                aUniqueName += String::CreateFromInt32( nUserIndex );
            }
        }

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() || pTempItem )
        {
            if( pTempItem )
            {
                pTempItem->SetName( aUniqueName );
                return pTempItem;
            }
            else
            {
                return new XLineEndItem( aUniqueName, aXPolygon );
            }
        }
    }

    return (XLineEndItem*)this;
}


//------------------------------------------------------------------------

SfxItemPresentation XLineEndItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetName();
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineEndItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
#ifndef SVX_LIGHT
    if( nMemberId == MID_NAME )
    {
        rtl::OUString aApiName;
        SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
        rVal <<= aApiName;
    }
    else
    {
        com::sun::star::drawing::PolyPolygonBezierCoords aBezier;
        SvxConvertXPolygonToPolyPolygonBezier( aXPolygon, aBezier );
        rVal <<= aBezier;
    }
#endif
    return sal_True;
}

sal_Bool XLineEndItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
#ifndef SVX_LIGHT
    if( nMemberId == MID_NAME )
    {
        return sal_False;
    }
    else
    {
        aXPolygon.SetSize( 0 );
        if( rVal.hasValue() && rVal.getValue() )
        {
            if( rVal.getValueType() != ::getCppuType((const com::sun::star::drawing::PolyPolygonBezierCoords*)0) )
                return sal_False;

            aXPolygon.SetSize(0);
            com::sun::star::drawing::PolyPolygonBezierCoords* pCoords = (com::sun::star::drawing::PolyPolygonBezierCoords*)rVal.getValue();
            if( pCoords->Coordinates.getLength() > 0 )
                SvxConvertPolyPolygonBezierToXPolygon( pCoords, aXPolygon );
        }
    }
#endif
    return sal_True;
}

// ----------------------------
// class XLineStartWidthItem
// ----------------------------
TYPEINIT1_AUTOFACTORY(XLineStartWidthItem, SfxMetricItem);

/*************************************************************************
|*
|*    XLineStartWidthItem::XLineStartWidthItem(long nWidth)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineStartWidthItem::XLineStartWidthItem(long nWidth) :
    SfxMetricItem(XATTR_LINESTARTWIDTH, nWidth)
{
}

/*************************************************************************
|*
|*    XLineStartWidthItem::XLineStartWidthItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineStartWidthItem::XLineStartWidthItem(SvStream& rIn) :
    SfxMetricItem(XATTR_LINESTARTWIDTH, rIn)
{
}

/*************************************************************************
|*
|*    XLineStartWidthItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartWidthItem::Clone(SfxItemPool* pPool) const
{
    return new XLineStartWidthItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStartWidthItem::Create(SvStream& rIn, USHORT nVer)
|*                                              const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartWidthItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineStartWidthItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineStartWidthItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetMetricText( (long) GetValue(),
                                    eCoreUnit, ePresUnit);
            rText += SVX_RESSTR( GetMetricId( ePresUnit) );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineStartWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

sal_Bool XLineStartWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    rVal >>= nValue;
    SetValue( nValue );
    return sal_True;
}



// --------------------------
// class XLineEndWidthItem
// --------------------------
TYPEINIT1_AUTOFACTORY(XLineEndWidthItem, SfxMetricItem);

/*************************************************************************
|*
|*    XLineEndWidthItem::XLineEndWidthItem(long nWidth)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineEndWidthItem::XLineEndWidthItem(long nWidth) :
   SfxMetricItem(XATTR_LINEENDWIDTH, nWidth)
{
}

/*************************************************************************
|*
|*    XLineEndWidthItem::XLineEndWidthItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineEndWidthItem::XLineEndWidthItem(SvStream& rIn) :
    SfxMetricItem(XATTR_LINEENDWIDTH, rIn)
{
}

/*************************************************************************
|*
|*    XLineEndWidthItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndWidthItem::Clone(SfxItemPool* pPool) const
{
    return new XLineEndWidthItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineEndWidthItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndWidthItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineEndWidthItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineEndWidthItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetMetricText( (long) GetValue(),
                                    eCoreUnit, ePresUnit);
            rText += SVX_RESSTR( GetMetricId( ePresUnit) );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineEndWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

sal_Bool XLineEndWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    rVal >>= nValue;
    SetValue( nValue );
    return sal_True;
}


// -----------------------------
// class XLineStartCenterItem
// -----------------------------
TYPEINIT1_AUTOFACTORY(XLineStartCenterItem, SfxBoolItem);

/*************************************************************************
|*
|*    XLineStartCenterItem::XLineStartCenterItem(BOOL bStartCenter)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineStartCenterItem::XLineStartCenterItem(BOOL bStartCenter) :
    SfxBoolItem(XATTR_LINESTARTCENTER, bStartCenter)
{
}

/*************************************************************************
|*
|*    XLineStartCenterItem::XLineStartCenterItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineStartCenterItem::XLineStartCenterItem(SvStream& rIn) :
    SfxBoolItem(XATTR_LINESTARTCENTER, rIn)
{
}

/*************************************************************************
|*
|*    XLineStartCenterItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartCenterItem::Clone(SfxItemPool* pPool) const
{
    return new XLineStartCenterItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStartCenterItem::Create(SvStream& rIn, USHORT nVer)
|*                                              const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartCenterItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineStartCenterItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineStartCenterItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = XubString( ResId( GetValue() ? RID_SVXSTR_CENTERED :
                            RID_SVXSTR_NOTCENTERED, DIALOG_MGR() ) );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineStartCenterItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bValue = GetValue();
    rVal.setValue( &bValue, ::getCppuBooleanType()  );
    return sal_True;
}

sal_Bool XLineStartCenterItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    if( !rVal.hasValue() || rVal.getValueType() != ::getCppuBooleanType() )
        return sal_False;

    SetValue( *(sal_Bool*)rVal.getValue() );
    return sal_True;
}


// ---------------------------
// class XLineEndCenterItem
// ---------------------------
TYPEINIT1_AUTOFACTORY(XLineEndCenterItem, SfxBoolItem);

/*************************************************************************
|*
|*    XLineEndCenterItem::XLineEndCenterItem(BOOL bEndCenter)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineEndCenterItem::XLineEndCenterItem(BOOL bEndCenter) :
    SfxBoolItem(XATTR_LINEENDCENTER, bEndCenter)
{
}

/*************************************************************************
|*
|*    XLineEndCenterItem::XLineEndCenterItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineEndCenterItem::XLineEndCenterItem(SvStream& rIn) :
    SfxBoolItem(XATTR_LINEENDCENTER, rIn)
{
}

/*************************************************************************
|*
|*    XLineEndCenterItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndCenterItem::Clone(SfxItemPool* pPool) const
{
    return new XLineEndCenterItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineEndCenterItem::Create(SvStream& rIn, USHORT nVer)
|*                                              const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndCenterItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineEndCenterItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineEndCenterItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = XubString( ResId( GetValue() ? RID_SVXSTR_CENTERED :
                            RID_SVXSTR_NOTCENTERED, DIALOG_MGR() ) );
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

sal_Bool XLineEndCenterItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bValue = GetValue();
    rVal.setValue( &bValue, ::getCppuBooleanType()  );
    return sal_True;
}

BOOL XLineEndCenterItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    if( !rVal.hasValue() || rVal.getValueType() != ::getCppuBooleanType() )
        return sal_False;

    SetValue( *(sal_Bool*)rVal.getValue() );
    return sal_True;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                           Fuellattribute
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// --------------------
// class XFillStyleItem
// --------------------
TYPEINIT1_AUTOFACTORY(XFillStyleItem, SfxEnumItem);

/*************************************************************************
|*
|*    XFillStyleItem::XFillStyleItem(XFillStyle eFillStyle)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillStyleItem::XFillStyleItem(XFillStyle eFillStyle) :
    SfxEnumItem(XATTR_FILLSTYLE, eFillStyle)
{
}

/*************************************************************************
|*
|*    XFillStyleItem::XFillStyleItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillStyleItem::XFillStyleItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FILLSTYLE, rIn)
{
}

/*************************************************************************
|*
|*    XFillStyleItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    09.11.94
|*    Letzte Aenderung  09.11.94
|*
*************************************************************************/

SfxPoolItem* XFillStyleItem::Clone(SfxItemPool* pPool) const
{
    return new XFillStyleItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillStyleItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillStyleItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFillStyleItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XFillStyleItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = 0;

            switch( (USHORT)GetValue() )
            {
                case XFILL_NONE:
                    nId = RID_SVXSTR_INVISIBLE;
                    break;
                case XFILL_SOLID:
                    nId = RID_SVXSTR_SOLID;
                    break;
                case XFILL_GRADIENT:
                    nId = RID_SVXSTR_GRADIENT;
                    break;
                case XFILL_HATCH:
                    nId = RID_SVXSTR_HATCH;
                    break;
                case XFILL_BITMAP:
                    nId = RID_SVXSTR_BITMAP;
                    break;
            }

            if ( nId )
                rText = SVX_RESSTR( nId );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

USHORT XFillStyleItem::GetValueCount() const
{
    return 5;
}

// -----------------------------------------------------------------------
sal_Bool XFillStyleItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    ::com::sun::star::drawing::FillStyle eFS = (::com::sun::star::drawing::FillStyle)GetValue();

    rVal <<= eFS;

    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool XFillStyleItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    ::com::sun::star::drawing::FillStyle eFS;
    if(!(rVal >>= eFS))
        return sal_False;

    SetValue( (XFillStyle)eFS );

    return sal_True;
}


// -------------------
// class XFillColorItem
// -------------------
TYPEINIT1_AUTOFACTORY(XFillColorItem, XColorItem);

/*************************************************************************
|*
|*    XFillColorItem::XFillColorItem(long nIndex, const Color& rTheColor)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillColorItem::XFillColorItem(long nIndex, const Color& rTheColor) :
    XColorItem(XATTR_FILLCOLOR, nIndex, rTheColor)
{
}

/*************************************************************************
|*
|*    XFillColorItem::XFillColorItem(const XubString& rName, const Color& rTheColor)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillColorItem::XFillColorItem(const XubString& rName, const Color& rTheColor) :
    XColorItem(XATTR_FILLCOLOR, rName, rTheColor)
{
}

/*************************************************************************
|*
|*    XFillColorItem::XFillColorItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillColorItem::XFillColorItem(SvStream& rIn) :
    XColorItem(XATTR_FILLCOLOR, rIn)
{
}

/*************************************************************************
|*
|*    XFillColorItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillColorItem::Clone(SfxItemPool* pPool) const
{
    return new XFillColorItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillColorItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillColorItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFillColorItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XFillColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetName();
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

sal_Bool XFillColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue().GetRGBColor();

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool XFillColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue ))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}



// ----------------
// class XGradient
// ----------------

/*************************************************************************
|*
|*    XGradient::XGradient(XGradientStyle, const Color&, const Color&,
|*                         long, USHORT, USHORT, USHORT)
|*
|*    Beschreibung
|*    Ersterstellung    21.11.94
|*    Letzte Aenderung  21.11.94
|*
*************************************************************************/

XGradient::XGradient(const Color& rStart, const Color& rEnd,
                     XGradientStyle eTheStyle, long nTheAngle, USHORT nXOfs,
                     USHORT nYOfs, USHORT nTheBorder,
                     USHORT nStartIntens, USHORT nEndIntens,
                     USHORT nSteps) :
    aStartColor(rStart),
    aEndColor(rEnd),
    eStyle(eTheStyle),
    nAngle(nTheAngle),
    nBorder(nTheBorder),
    nOfsX(nXOfs),
    nOfsY(nYOfs),
    nIntensStart(nStartIntens),
    nIntensEnd(nEndIntens),
    nStepCount(nSteps)
{
}

/*************************************************************************
|*
|*    int XGradient::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    29.11.94
|*    Letzte Aenderung  29.11.94
|*
*************************************************************************/

int XGradient::operator==(const XGradient& rGradient) const
{
    return ( eStyle         == rGradient.eStyle         &&
             aStartColor    == rGradient.aStartColor    &&
             aEndColor      == rGradient.aEndColor      &&
             nAngle         == rGradient.nAngle         &&
             nBorder        == rGradient.nBorder        &&
             nOfsX          == rGradient.nOfsX          &&
             nOfsY          == rGradient.nOfsY          &&
             nIntensStart   == rGradient.nIntensStart   &&
             nIntensEnd     == rGradient.nIntensEnd     &&
             nStepCount     == rGradient.nStepCount );
}


// -----------------------
// class XFillGradientItem
// -----------------------
TYPEINIT1_AUTOFACTORY(XFillGradientItem, NameOrIndex);

/*************************************************************************
|*
|*    XFillGradientItem::XFillGradientItem(long nIndex,
|*                                       const XGradient& rTheGradient)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillGradientItem::XFillGradientItem(long nIndex,
                                   const XGradient& rTheGradient) :
    NameOrIndex(XATTR_FILLGRADIENT, nIndex),
    aGradient(rTheGradient)
{
}

/*************************************************************************
|*
|*    XFillGradientItem::XFillGradientItem(const XubString& rName,
|*                                       const XGradient& rTheGradient)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillGradientItem::XFillGradientItem(const XubString& rName,
                                   const XGradient& rTheGradient) :
    NameOrIndex(XATTR_FILLGRADIENT, rName),
    aGradient(rTheGradient)
{
}

/*************************************************************************
|*
|*    XFillGradientItem::XFillGradientItem(const XFillGradientItem& rItem)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillGradientItem::XFillGradientItem(const XFillGradientItem& rItem) :
    NameOrIndex(rItem),
    aGradient(rItem.aGradient)
{
}

/*************************************************************************
|*
|*    XFillGradientItem::XFillGradientItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillGradientItem::XFillGradientItem(SvStream& rIn, USHORT nVer) :
    NameOrIndex(XATTR_FILLGRADIENT, rIn),
    aGradient(COL_BLACK, COL_WHITE)
{
    if (!IsIndex())
    {
        USHORT nUSTemp;
        USHORT nRed;
        USHORT nGreen;
        USHORT nBlue;
        INT16  nITemp;
        INT32  nLTemp;

        rIn >> nITemp; aGradient.SetGradientStyle((XGradientStyle)nITemp);
        rIn >> nRed;
        rIn >> nGreen;
        rIn >> nBlue;
#ifdef VCL
        Color aCol;
        aCol = Color( (BYTE)( nRed >> 8 ), (BYTE)( nGreen >> 8 ), (BYTE)( nBlue >> 8 ) );
#else
        Color aCol( COL_USER );
        aCol = Color( nRed, nGreen, nBlue );
        aCol.SetColorName(COL_USER);
#endif
        aGradient.SetStartColor( aCol );

        rIn >> nRed;
        rIn >> nGreen;
        rIn >> nBlue;
#ifdef VCL
        aCol = Color( (BYTE)( nRed >> 8 ), (BYTE)( nGreen >> 8 ), (BYTE)( nBlue >> 8 ) );
#else
        aCol = Color( nRed, nGreen, nBlue );
        aCol.SetColorName(COL_USER);
#endif
        aGradient.SetEndColor(aCol);
        rIn >> nLTemp; aGradient.SetAngle(nLTemp);
        rIn >> nUSTemp; aGradient.SetBorder(nUSTemp);
        rIn >> nUSTemp; aGradient.SetXOffset(nUSTemp);
        rIn >> nUSTemp; aGradient.SetYOffset(nUSTemp);
        rIn >> nUSTemp; aGradient.SetStartIntens(nUSTemp);
        rIn >> nUSTemp; aGradient.SetEndIntens(nUSTemp);

        // bei neueren Versionen wird zusaetzlich
        // die Schrittweite mit eingelesen
        if (nVer >= 1)
        {
            rIn >> nUSTemp; aGradient.SetSteps(nUSTemp);
        }
    }
}

//*************************************************************************

XFillGradientItem::XFillGradientItem(SfxItemPool* pPool, const XGradient& rTheGradient)
:   NameOrIndex( XATTR_FILLGRADIENT, -1 ),
    aGradient(rTheGradient)
{
}

//*************************************************************************

XFillGradientItem::XFillGradientItem(SfxItemPool* pPool )
: NameOrIndex(XATTR_FILLGRADIENT, -1 )
{
}

/*************************************************************************
|*
|*    XFillGradientItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillGradientItem::Clone(SfxItemPool* pPool) const
{
    return new XFillGradientItem(*this);
}

/*************************************************************************
|*
|*    int XFillGradientItem::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

int XFillGradientItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             aGradient == ((const XFillGradientItem&) rItem).aGradient );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillGradientItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillGradientItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFillGradientItem(rIn, nVer);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillGradientItem::Store(SvStream& rOut) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SvStream& XFillGradientItem::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut << (INT16)aGradient.GetGradientStyle();

        USHORT nTmp;

        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetRed() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetGreen() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetBlue() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetRed() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetGreen() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetBlue() ); rOut << nTmp;

        rOut << (INT32) aGradient.GetAngle();
        rOut << aGradient.GetBorder();
        rOut << aGradient.GetXOffset();
        rOut << aGradient.GetYOffset();
        rOut << aGradient.GetStartIntens();
        rOut << aGradient.GetEndIntens();
        rOut << aGradient.GetSteps();
    }

    return rOut;
}

/*************************************************************************
|*
|*    const XGradient& XFillGradientItem::GetValue(const XGradientTable* pTable)
|*                                                                 const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

const XGradient& XFillGradientItem::GetValue(const XGradientTable* pTable) const
{
    if (!IsIndex())
        return aGradient;
    else
        return pTable->Get(GetIndex())->GetGradient();
}


/*************************************************************************
|*
|*    USHORT XFillGradientItem::GetVersion() const
|*
|*    Beschreibung
|*    Ersterstellung    01.11.95
|*    Letzte Aenderung  01.11.95
|*
*************************************************************************/

USHORT XFillGradientItem::GetVersion( USHORT nFileFormatVersion ) const
{
    // !!! this version number also represents the version number of superclasses
    // !!! (e.g. XFillFloatTransparenceItem); if you make any changes here,
    // !!! the superclass is also affected
    return 1;
}

//------------------------------------------------------------------------

SfxItemPresentation XFillGradientItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetName();
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------
sal_Bool XFillGradientItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    if( nMemberId == MID_NAME )
    {
        rtl::OUString aApiName;
#ifndef SVX_LIGHT
        SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
#endif
        rVal <<= aApiName;
    }
    else
    {
        const XGradient& aXGradient = GetValue();
        ::com::sun::star::awt::Gradient aGradient;

        aGradient.Style = (::com::sun::star::awt::GradientStyle) aXGradient.GetGradientStyle();
        aGradient.StartColor = (INT32)aXGradient.GetStartColor().GetColor();
        aGradient.EndColor = (INT32)aXGradient.GetEndColor().GetColor();
        aGradient.Angle = (short)aXGradient.GetAngle();
        aGradient.Border = aXGradient.GetBorder();
        aGradient.XOffset = aXGradient.GetXOffset();
        aGradient.YOffset = aXGradient.GetYOffset();
        aGradient.StartIntensity = aXGradient.GetStartIntens();
        aGradient.EndIntensity = aXGradient.GetEndIntens();
        aGradient.StepCount = aXGradient.GetSteps();

        rVal <<= aGradient;
    }

    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool XFillGradientItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    ::com::sun::star::awt::Gradient aGradient;
    if(!(rVal >>= aGradient))
        return sal_False;

    XGradient aXGradient;

    aXGradient.SetGradientStyle( (XGradientStyle) aGradient.Style );
    aXGradient.SetStartColor( aGradient.StartColor );
    aXGradient.SetEndColor( aGradient.EndColor );
    aXGradient.SetAngle( aGradient.Angle );
    aXGradient.SetBorder( aGradient.Border );
    aXGradient.SetXOffset( aGradient.XOffset );
    aXGradient.SetYOffset( aGradient.YOffset );
    aXGradient.SetStartIntens( aGradient.StartIntensity );
    aXGradient.SetEndIntens( aGradient.EndIntensity );
    aXGradient.SetSteps( aGradient.StepCount );

    SetValue( aXGradient );

    return sal_True;
}

BOOL XFillGradientItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return ((XFillGradientItem*)p1)->GetValue() == ((XFillGradientItem*)p2)->GetValue();
}

XFillGradientItem* XFillGradientItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const String aUniqueName = NameOrIndex::CheckNamedItem( this,
                                                                XATTR_FILLGRADIENT,
                                                                &pModel->GetItemPool(),
                                                                pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                                                                XFillGradientItem::CompareValueFunc,
                                                                RID_SVXSTR_GRADIENT,
                                                                pModel->GetGradientList() );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
        {
            return new XFillGradientItem( aUniqueName, aGradient );
        }
    }

    return (XFillGradientItem*)this;
}

// ----------------------------------
// class XFillFloatTransparenceItem -
// ----------------------------------

TYPEINIT1_AUTOFACTORY( XFillFloatTransparenceItem, XFillGradientItem );

// -----------------------------------------------------------------------------

XFillFloatTransparenceItem::XFillFloatTransparenceItem() :
    bEnabled( FALSE )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//------------------------------------------------------------------------

XFillFloatTransparenceItem::XFillFloatTransparenceItem( long nIndex, const XGradient& rGradient, BOOL bEnable ) :
    XFillGradientItem   ( nIndex, rGradient ),
    bEnabled            ( bEnable )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//------------------------------------------------------------------------

XFillFloatTransparenceItem::XFillFloatTransparenceItem(const XubString& rName, const XGradient& rGradient, BOOL bEnable ) :
    XFillGradientItem   ( rName, rGradient ),
    bEnabled            ( bEnable )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//------------------------------------------------------------------------

XFillFloatTransparenceItem::XFillFloatTransparenceItem( const XFillFloatTransparenceItem& rItem ) :
    XFillGradientItem   ( rItem ),
    bEnabled            ( rItem.bEnabled )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//------------------------------------------------------------------------

XFillFloatTransparenceItem::XFillFloatTransparenceItem( SvStream& rIn, USHORT nVer ) :
    XFillGradientItem   ( rIn, nVer )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
    rIn >> bEnabled;
}

//*************************************************************************

XFillFloatTransparenceItem::XFillFloatTransparenceItem(SfxItemPool* pPool, const XGradient& rTheGradient, BOOL bEnable )
:   XFillGradientItem   ( -1, rTheGradient ),
    bEnabled            ( bEnable )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//*************************************************************************

XFillFloatTransparenceItem::XFillFloatTransparenceItem(SfxItemPool* pPool )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//------------------------------------------------------------------------

int XFillFloatTransparenceItem::operator==( const SfxPoolItem& rItem ) const
{
    // don't compare name of items !
    return ( ( GetValue() == ((const XFillGradientItem&)rItem).GetValue() ) &&
             ( bEnabled == ( (XFillFloatTransparenceItem&) rItem ).bEnabled )  );
}

//------------------------------------------------------------------------

SfxPoolItem* XFillFloatTransparenceItem::Clone( SfxItemPool* pPool ) const
{
    return new XFillFloatTransparenceItem( *this );
}

//------------------------------------------------------------------------

SfxPoolItem* XFillFloatTransparenceItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return( ( 0 == nVer ) ? Clone( NULL ) : new XFillFloatTransparenceItem( rIn, nVer ) );
}

//------------------------------------------------------------------------

SvStream& XFillFloatTransparenceItem::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    XFillGradientItem::Store( rOut, nItemVersion );
    rOut << bEnabled;
    return rOut;
}

//------------------------------------------------------------------------

USHORT XFillFloatTransparenceItem::GetVersion( USHORT nFileFormatVersion ) const
{
    // !!! if version number of this object must be increased, please   !!!
    // !!! increase version number of base class XFillGradientItem      !!!
    return XFillGradientItem::GetVersion( nFileFormatVersion );
}

//------------------------------------------------------------------------

sal_Bool XFillFloatTransparenceItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    return XFillGradientItem::QueryValue( rVal, nMemberId );
}

//------------------------------------------------------------------------

sal_Bool XFillFloatTransparenceItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    return XFillGradientItem::PutValue( rVal, nMemberId );
}

//------------------------------------------------------------------------

SfxItemPresentation XFillFloatTransparenceItem::GetPresentation(    SfxItemPresentation ePres,
                                                                    SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
                                                                    XubString& rText,
                                                                    const International * pInternational ) const
{
    return XFillGradientItem::GetPresentation( ePres, eCoreUnit, ePresUnit, rText, pInternational );
}

BOOL XFillFloatTransparenceItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return  ((XFillFloatTransparenceItem*)p1)->IsEnabled() == ((XFillFloatTransparenceItem*)p2)->IsEnabled() &&
            ((XFillFloatTransparenceItem*)p1)->GetValue()  == ((XFillFloatTransparenceItem*)p2)->GetValue();
}

XFillFloatTransparenceItem* XFillFloatTransparenceItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const String aUniqueName = NameOrIndex::CheckNamedItem( this,
                                                                XATTR_FILLFLOATTRANSPARENCE,
                                                                &pModel->GetItemPool(),
                                                                pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                                                                XFillFloatTransparenceItem::CompareValueFunc,
                                                                SIP_XA_FILLFLOATTRANSPARENCE,
                                                                NULL );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
        {
            return new XFillFloatTransparenceItem( aUniqueName, GetValue(), bEnabled );
        }
    }

    return (XFillFloatTransparenceItem*)this;
}

// -------------
// class XHatch
// -------------

/*************************************************************************
|*
|*    XHatch::XHatch(XHatchStyle, const Color&, long, long)
|*
|*    Beschreibung
|*    Ersterstellung    21.11.94
|*    Letzte Aenderung  21.11.94
|*
*************************************************************************/

XHatch::XHatch(const Color& rCol, XHatchStyle eTheStyle, long nTheDistance,
               long nTheAngle) :
    aColor(rCol),
    eStyle(eTheStyle),
    nDistance(nTheDistance),
    nAngle(nTheAngle)
{
}

/*************************************************************************
|*
|*    int XHatch::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    29.11.94
|*    Letzte Aenderung  29.11.94
|*
*************************************************************************/

int XHatch::operator==(const XHatch& rHatch) const
{
    return ( eStyle     == rHatch.eStyle    &&
             aColor     == rHatch.aColor    &&
             nDistance  == rHatch.nDistance &&
             nAngle     == rHatch.nAngle );
}


// -----------------------
// class XFillHatchItem
// -----------------------
TYPEINIT1_AUTOFACTORY(XFillHatchItem, NameOrIndex);

/*************************************************************************
|*
|*    XFillHatchItem::XFillHatchItem(long nIndex,
|*                                 const XHatch& rTheHatch)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillHatchItem::XFillHatchItem(long nIndex,
                             const XHatch& rTheHatch) :
    NameOrIndex(XATTR_FILLHATCH, nIndex),
    aHatch(rTheHatch)
{
}

/*************************************************************************
|*
|*    XFillHatchItem::XFillHatchItem(const XubString& rName,
|*                                 const XHatch& rTheHatch)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillHatchItem::XFillHatchItem(const XubString& rName,
                             const XHatch& rTheHatch) :
    NameOrIndex(XATTR_FILLHATCH, rName),
    aHatch(rTheHatch)
{
}

/*************************************************************************
|*
|*    XFillHatchItem::XFillHatchItem(const XFillHatchItem& rItem)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillHatchItem::XFillHatchItem(const XFillHatchItem& rItem) :
    NameOrIndex(rItem),
    aHatch(rItem.aHatch)
{
}

/*************************************************************************
|*
|*    XFillHatchItem::XFillHatchItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillHatchItem::XFillHatchItem(SvStream& rIn) :
    NameOrIndex(XATTR_FILLHATCH, rIn),
    aHatch(COL_BLACK)
{
    if (!IsIndex())
    {
        USHORT nRed;
        USHORT nGreen;
        USHORT nBlue;
        INT16  nITemp;
        INT32  nLTemp;

        rIn >> nITemp; aHatch.SetHatchStyle((XHatchStyle)nITemp);
        rIn >> nRed;
        rIn >> nGreen;
        rIn >> nBlue;

#ifdef VCL
        Color aCol;
        aCol = Color( (BYTE)( nRed >> 8 ), (BYTE)( nGreen >> 8 ), (BYTE)( nBlue >> 8 ) );
#else
        Color aCol( COL_USER );
        aCol = Color( nRed, nGreen, nBlue );
        aCol.SetColorName(COL_USER);
#endif
        aHatch.SetColor(aCol);
        rIn >> nLTemp; aHatch.SetDistance(nLTemp);
        rIn >> nLTemp; aHatch.SetAngle(nLTemp);
    }
}

//*************************************************************************

XFillHatchItem::XFillHatchItem(SfxItemPool* pPool, const XHatch& rTheHatch)
:   NameOrIndex( XATTR_FILLHATCH, -1 ),
    aHatch(rTheHatch)
{
}

//*************************************************************************

XFillHatchItem::XFillHatchItem(SfxItemPool* pPool )
: NameOrIndex(XATTR_FILLHATCH, -1 )
{
}

/*************************************************************************
|*
|*    XFillHatchItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillHatchItem::Clone(SfxItemPool* pPool) const
{
    return new XFillHatchItem(*this);
}

/*************************************************************************
|*
|*    int XFillHatchItem::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

int XFillHatchItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             aHatch == ((const XFillHatchItem&) rItem).aHatch );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillHatchItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillHatchItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFillHatchItem(rIn);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillHatchItem::Store(SvStream& rOut) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SvStream& XFillHatchItem::Store( SvStream& rOut, USHORT nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut << (INT16)aHatch.GetHatchStyle();

#ifdef VCL
        USHORT nTmp;
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetRed() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetGreen() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetBlue() ); rOut << nTmp;
#else
        rOut << aHatch.GetColor().GetRed();
        rOut << aHatch.GetColor().GetGreen();
        rOut << aHatch.GetColor().GetBlue();
#endif

        rOut << (INT32) aHatch.GetDistance();
        rOut << (INT32) aHatch.GetAngle();
    }

    return rOut;
}

/*************************************************************************
|*
|*    const XHatch& XFillHatchItem::GetValue(const XHatchTable* pTable) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

const XHatch& XFillHatchItem::GetValue(const XHatchTable* pTable) const
{
    if (!IsIndex())
        return aHatch;
    else
        return pTable->Get(GetIndex())->GetHatch();
}

//------------------------------------------------------------------------

SfxItemPresentation XFillHatchItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetName();
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

FASTBOOL XFillHatchItem::HasMetrics() const
{
    return TRUE;
}

//------------------------------------------------------------------------

FASTBOOL XFillHatchItem::ScaleMetrics(long nMul, long nDiv)
{
    aHatch.SetDistance( ScaleMetricValue( aHatch.GetDistance(), nMul, nDiv ) );
    return TRUE;
}

// -----------------------------------------------------------------------
sal_Bool XFillHatchItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    if( nMemberId == MID_NAME )
    {
        rtl::OUString aApiName;
#ifndef SVX_LIGHT
        SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
#endif
        rVal <<= aApiName;
    }
    else
    {
        ::com::sun::star::drawing::Hatch aUnoHatch;

        aUnoHatch.Style = (::com::sun::star::drawing::HatchStyle)aHatch.GetHatchStyle();
        aUnoHatch.Color = aHatch.GetColor().GetColor();
        aUnoHatch.Distance = aHatch.GetDistance();
        aUnoHatch.Angle = aHatch.GetAngle();
        rVal <<= aUnoHatch;
    }

    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool XFillHatchItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    ::com::sun::star::drawing::Hatch aUnoHatch;
    if(!(rVal >>= aUnoHatch))
        return sal_False;

    aHatch.SetHatchStyle( (XHatchStyle)aUnoHatch.Style );
    aHatch.SetColor( aUnoHatch.Color );
    aHatch.SetDistance( aUnoHatch.Distance );
    aHatch.SetAngle( aUnoHatch.Angle );
    return sal_True;
}

BOOL XFillHatchItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return ((XFillHatchItem*)p1)->GetValue() == ((XFillHatchItem*)p2)->GetValue();
}

XFillHatchItem* XFillHatchItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const String aUniqueName = NameOrIndex::CheckNamedItem( this,
                                                                XATTR_FILLHATCH,
                                                                &pModel->GetItemPool(),
                                                                pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                                                                XFillHatchItem::CompareValueFunc,
                                                                RID_SVXSTR_HATCH10,
                                                                pModel->GetHatchList() );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
        {
            return new XFillHatchItem( aUniqueName, aHatch );
        }
    }

    return (XFillHatchItem*)this;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                          FormText-Attribute
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------------
// class XFormTextStyleItem
//-------------------------
TYPEINIT1_AUTOFACTORY(XFormTextStyleItem, SfxEnumItem);

/*************************************************************************
|*
|*    XFormTextStyleItem::XFormTextStyleItem()
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

XFormTextStyleItem::XFormTextStyleItem(XFormTextStyle eTheStyle) :
    SfxEnumItem(XATTR_FORMTXTSTYLE, eTheStyle)
{
}

/*************************************************************************
|*
|*    XFormTextStyleItem::XFormTextStyleItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

XFormTextStyleItem::XFormTextStyleItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FORMTXTSTYLE, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextStyleItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextStyleItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextStyleItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextStyleItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextStyleItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextStyleItem(rIn);
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

USHORT XFormTextStyleItem::GetValueCount() const
{
    return 5;
}


//-------------------------
// class XFormTextAdjustItem
//-------------------------
TYPEINIT1_AUTOFACTORY(XFormTextAdjustItem, SfxEnumItem);

/*************************************************************************
|*
|*    XFormTextAdjustItem::XFormTextAdjustItem()
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

XFormTextAdjustItem::XFormTextAdjustItem(XFormTextAdjust eTheAdjust) :
    SfxEnumItem(XATTR_FORMTXTADJUST, eTheAdjust)
{
}

/*************************************************************************
|*
|*    XFormTextAdjustItem::XFormTextAdjustItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

XFormTextAdjustItem::XFormTextAdjustItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FORMTXTADJUST, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextAdjustItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextAdjustItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextAdjustItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextAdjustItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextAdjustItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextAdjustItem(rIn);
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

USHORT XFormTextAdjustItem::GetValueCount() const
{
    return 4;
}


//----------------------------
// class XFormTextDistanceItem
//----------------------------
TYPEINIT1_AUTOFACTORY(XFormTextDistanceItem, SfxMetricItem);

/*************************************************************************
|*
|*    XFormTextDistanceItem::XFormTextDistanceItem()
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

XFormTextDistanceItem::XFormTextDistanceItem(long nDist) :
    SfxMetricItem(XATTR_FORMTXTDISTANCE, nDist)
{
}

/*************************************************************************
|*
|*    XFormTextDistanceItem::XFormTextDistanceItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

XFormTextDistanceItem::XFormTextDistanceItem(SvStream& rIn) :
    SfxMetricItem(XATTR_FORMTXTDISTANCE, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextDistanceItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextDistanceItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextDistanceItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextDistanceItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextDistanceItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextDistanceItem(rIn);
}

//-------------------------
// class XFormTextStartItem
//-------------------------
TYPEINIT1_AUTOFACTORY(XFormTextStartItem, SfxMetricItem);

/*************************************************************************
|*
|*    XFormTextStartItem::XFormTextStartItem(long nStart)
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

XFormTextStartItem::XFormTextStartItem(long nStart) :
    SfxMetricItem(XATTR_FORMTXTSTART, nStart)
{
}

/*************************************************************************
|*
|*    XFormTextStartItem::XFormTextStartItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

XFormTextStartItem::XFormTextStartItem(SvStream& rIn) :
    SfxMetricItem(XATTR_FORMTXTSTART, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextStartItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextStartItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextStartItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextStartItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextStartItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextStartItem(rIn);
}

// -------------------------
// class XFormTextMirrorItem
// -------------------------
TYPEINIT1_AUTOFACTORY(XFormTextMirrorItem, SfxBoolItem);

/*************************************************************************
|*
|*    XFormTextMirrorItem::XFormTextMirrorItem(BOOL bMirror)
|*
|*    Ersterstellung    06.02.95 ESO
|*    Letzte Aenderung  06.02.95 ESO
|*
*************************************************************************/

XFormTextMirrorItem::XFormTextMirrorItem(BOOL bMirror) :
    SfxBoolItem(XATTR_FORMTXTMIRROR, bMirror)
{
}

/*************************************************************************
|*
|*    XFormTextMirrorItem::XFormTextMirrorItem(SvStream& rIn)
|*
|*    Ersterstellung    06.02.95 ESO
|*    Letzte Aenderung  06.02.95 ESO
|*
*************************************************************************/

XFormTextMirrorItem::XFormTextMirrorItem(SvStream& rIn) :
    SfxBoolItem(XATTR_FORMTXTMIRROR, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextMirrorItem::Clone(SfxItemPool* pPool) const
|*
|*    Ersterstellung    06.02.95 ESO
|*    Letzte Aenderung  06.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextMirrorItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextMirrorItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextMirrorItem::Create(SvStream& rIn, USHORT nVer)
|*                                              const
|*
|*    Ersterstellung    06.02.95 ESO
|*    Letzte Aenderung  06.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextMirrorItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextMirrorItem(rIn);
}


// --------------------------
// class XFormTextOutlineItem
// --------------------------
TYPEINIT1_AUTOFACTORY(XFormTextOutlineItem, SfxBoolItem);

/*************************************************************************
|*
|*    XFormTextOutlineItem::XFormTextOutlineItem()
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

XFormTextOutlineItem::XFormTextOutlineItem(BOOL bOutline) :
    SfxBoolItem(XATTR_FORMTXTOUTLINE, bOutline)
{
}

/*************************************************************************
|*
|*    XFormTextOutlineItem::XFormTextOutlineItem(SvStream& rIn)
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

XFormTextOutlineItem::XFormTextOutlineItem(SvStream& rIn) :
    SfxBoolItem(XATTR_FORMTXTOUTLINE, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextOutlineItem::Clone(SfxItemPool* pPool) const
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextOutlineItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextOutlineItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextOutlineItem::Create(SvStream& rIn, USHORT nVer)
|*                                              const
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextOutlineItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextOutlineItem(rIn);
}

//--------------------------
// class XFormTextShadowItem
//--------------------------
TYPEINIT1_AUTOFACTORY(XFormTextShadowItem, SfxEnumItem);

/*************************************************************************
|*
|*    XFormTextShadowItem::XFormTextShadowItem()
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

XFormTextShadowItem::XFormTextShadowItem(XFormTextShadow eFormTextShadow) :
    SfxEnumItem(XATTR_FORMTXTSHADOW, eFormTextShadow)
{
}

/*************************************************************************
|*
|*    XFormTextShadowItem::XFormTextShadowItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

XFormTextShadowItem::XFormTextShadowItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FORMTXTSHADOW, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextShadowItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextShadowItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextShadowItem(rIn);
}


/*************************************************************************
|*
|*
|*
\*************************************************************************/

USHORT XFormTextShadowItem::GetValueCount() const
{
    return 3;
}


// -------------------------------
// class XFormTextShadowColorItem
// -------------------------------
TYPEINIT1_AUTOFACTORY(XFormTextShadowColorItem, XColorItem);

/*************************************************************************
|*
|*    XFormTextShadowColorItem::XFormTextShadowColorItem()
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

XFormTextShadowColorItem::XFormTextShadowColorItem(long nIndex,
                                                     const Color& rTheColor) :
    XColorItem(XATTR_FORMTXTSHDWCOLOR, nIndex, rTheColor)
{
}

/*************************************************************************
|*
|*    XFormTextShadowColorItem::XFormTextShadowColorItem(const XubString& rName, const Color& rTheColor)
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

XFormTextShadowColorItem::XFormTextShadowColorItem(const XubString& rName,
                                                     const Color& rTheColor) :
    XColorItem(XATTR_FORMTXTSHDWCOLOR, rName, rTheColor)
{
}

/*************************************************************************
|*
|*    XFormTextShadowColorItem::XFormTextShadowColorItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

XFormTextShadowColorItem::XFormTextShadowColorItem(SvStream& rIn) :
    XColorItem(XATTR_FORMTXTSHDWCOLOR, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextShadowColorItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowColorItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextShadowColorItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowColorItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowColorItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextShadowColorItem(rIn);
}

//------------------------------
// class XFormTextShadowXValItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XFormTextShadowXValItem, SfxMetricItem);

/*************************************************************************
|*
|*    XFormTextShadowXValItem::XFormTextShadowXValItem(long)
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

XFormTextShadowXValItem::XFormTextShadowXValItem(long nVal) :
    SfxMetricItem(XATTR_FORMTXTSHDWXVAL, nVal)
{
}

/*************************************************************************
|*
|*    XFormTextShadowXValItem::XFormTextShadowXValItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

XFormTextShadowXValItem::XFormTextShadowXValItem(SvStream& rIn) :
    SfxMetricItem(XATTR_FORMTXTSHDWXVAL, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextShadowXValItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowXValItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextShadowXValItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowXValItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowXValItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextShadowXValItem(rIn);
}

//------------------------------
// class XFormTextShadowYValItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XFormTextShadowYValItem, SfxMetricItem);

/*************************************************************************
|*
|*    XFormTextShadowYValItem::XFormTextShadowYValItem(long)
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

XFormTextShadowYValItem::XFormTextShadowYValItem(long nVal) :
    SfxMetricItem(XATTR_FORMTXTSHDWYVAL, nVal)
{
}

/*************************************************************************
|*
|*    XFormTextShadowYValItem::XFormTextShadowYValItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

XFormTextShadowYValItem::XFormTextShadowYValItem(SvStream& rIn) :
    SfxMetricItem(XATTR_FORMTXTSHDWYVAL, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextShadowYValItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowYValItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextShadowYValItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowYValItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowYValItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextShadowYValItem(rIn);
}

//---------------------------
// class XFormTextStdFormItem
//---------------------------
TYPEINIT1_AUTOFACTORY(XFormTextStdFormItem, SfxEnumItem);

/*************************************************************************
|*
|*    XFormTextStdFormItem::XFormTextStdFormItem()
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

XFormTextStdFormItem::XFormTextStdFormItem(XFormTextStdForm eFormTextStdForm) :
    SfxEnumItem(XATTR_FORMTXTSTDFORM, eFormTextStdForm)
{
}

/*************************************************************************
|*
|*    XFormTextStdFormItem::XFormTextStdFormItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

XFormTextStdFormItem::XFormTextStdFormItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FORMTXTSTDFORM, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextStdFormItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextStdFormItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextStdFormItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextStdFormItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextStdFormItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextStdFormItem(rIn);
}


/*************************************************************************
|*
|*
|*
\*************************************************************************/

USHORT XFormTextStdFormItem::GetValueCount() const
{
    return 3;
}


// --------------------------
// class XFormTextHideFormItem
// --------------------------
TYPEINIT1_AUTOFACTORY(XFormTextHideFormItem, SfxBoolItem);

/*************************************************************************
|*
|*    XFormTextHideFormItem::XFormTextHideFormItem()
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

XFormTextHideFormItem::XFormTextHideFormItem(BOOL bHide) :
    SfxBoolItem(XATTR_FORMTXTHIDEFORM, bHide)
{
}

/*************************************************************************
|*
|*    XFormTextHideFormItem::XFormTextHideFormItem(SvStream& rIn)
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

XFormTextHideFormItem::XFormTextHideFormItem(SvStream& rIn) :
    SfxBoolItem(XATTR_FORMTXTHIDEFORM, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextHideFormItem::Clone(SfxItemPool* pPool) const
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextHideFormItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextHideFormItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextHideFormItem::Create(SvStream& rIn, USHORT nVer)
|*                                              const
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextHideFormItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextHideFormItem(rIn);
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                              SetItems
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(XLineAttrSetItem, SfxSetItem);

/*************************************************************************
|*
|* Konstruktoren fuer Linienattribute-SetItem
|*
\************************************************************************/

XLineAttrSetItem::XLineAttrSetItem( SfxItemSet* pItemSet ) :
    SfxSetItem( XATTRSET_LINE, pItemSet)
{
}

/************************************************************************/

XLineAttrSetItem::XLineAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_LINE,
        new SfxItemSet( *pItemPool, XATTR_LINE_FIRST, XATTR_LINE_LAST))
{
}

/************************************************************************/

XLineAttrSetItem::XLineAttrSetItem( const XLineAttrSetItem& rLineAttr ) :
    SfxSetItem( rLineAttr )
{
}

/************************************************************************/

XLineAttrSetItem::XLineAttrSetItem( const XLineAttrSetItem& rLineAttr,
                                    SfxItemPool* pItemPool) :
    SfxSetItem( rLineAttr, pItemPool )
{
}

/*************************************************************************
|*
|* Clone-Funktion
|*
\************************************************************************/

SfxPoolItem* XLineAttrSetItem::Clone( SfxItemPool* pPool ) const
{
    return new XLineAttrSetItem( *this, pPool );
}

/*************************************************************************
|*
|* SetItem aus Stream erzeugen
|*
\************************************************************************/

SfxPoolItem* XLineAttrSetItem::Create( SvStream& rStream, USHORT nVersion ) const
{
    SfxItemSet *pSet = new SfxItemSet( *GetItemSet().GetPool(),
                                    XATTR_LINE_FIRST, XATTR_LINE_LAST);
    pSet->Load( rStream );
    return new XLineAttrSetItem( pSet );
}

/*************************************************************************
|*
|* SetItem in Stream speichern
|*
\************************************************************************/

SvStream& XLineAttrSetItem::Store( SvStream& rStream, USHORT nItemVersion ) const
{
    return SfxSetItem::Store( rStream, nItemVersion );
}


TYPEINIT1(XFillAttrSetItem, SfxSetItem);

/*************************************************************************
|*
|* Konstruktoren fuer Fuellattribute-SetItem
|*
\************************************************************************/

XFillAttrSetItem::XFillAttrSetItem( SfxItemSet* pItemSet ) :
    SfxSetItem( XATTRSET_FILL, pItemSet)
{
}

/************************************************************************/

XFillAttrSetItem::XFillAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_FILL,
        new SfxItemSet( *pItemPool, XATTR_FILL_FIRST, XATTR_FILL_LAST))
{
}

/************************************************************************/

XFillAttrSetItem::XFillAttrSetItem( const XFillAttrSetItem& rFillAttr ) :
    SfxSetItem( rFillAttr )
{
}

/************************************************************************/

XFillAttrSetItem::XFillAttrSetItem( const XFillAttrSetItem& rFillAttr,
                                    SfxItemPool* pItemPool ) :
    SfxSetItem( rFillAttr, pItemPool )
{
}

/*************************************************************************
|*
|* Clone-Funktion
|*
\************************************************************************/

SfxPoolItem* XFillAttrSetItem::Clone( SfxItemPool* pPool ) const
{
    return new XFillAttrSetItem( *this, pPool );
}

/*************************************************************************
|*
|* SetItem aus Stream erzeugen
|*
\************************************************************************/

SfxPoolItem* XFillAttrSetItem::Create( SvStream& rStream, USHORT nVersion ) const
{
    SfxItemSet *pSet = new SfxItemSet( *GetItemSet().GetPool(),
                                    XATTR_FILL_FIRST, XATTR_FILL_LAST);
    pSet->Load( rStream );
    return new XFillAttrSetItem( pSet );
}

/*************************************************************************
|*
|* SetItem in Stream speichern
|*
\************************************************************************/

SvStream& XFillAttrSetItem::Store( SvStream& rStream, USHORT nItemVersion ) const
{
    return SfxSetItem::Store( rStream, nItemVersion );
}


TYPEINIT1(XTextAttrSetItem, SfxSetItem);

/*************************************************************************
|*
|* Konstruktoren fuer Textattribute-SetItem
|*
\************************************************************************/

XTextAttrSetItem::XTextAttrSetItem( SfxItemSet* pItemSet ) :
    SfxSetItem( XATTRSET_TEXT, pItemSet)
{
}

/************************************************************************/

XTextAttrSetItem::XTextAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_TEXT,
        new SfxItemSet( *pItemPool, XATTR_TEXT_FIRST, XATTR_TEXT_LAST))
{
}

/************************************************************************/

XTextAttrSetItem::XTextAttrSetItem( const XTextAttrSetItem& rTextAttr ) :
    SfxSetItem( rTextAttr )
{
}

/************************************************************************/

XTextAttrSetItem::XTextAttrSetItem( const XTextAttrSetItem& rTextAttr,
                                    SfxItemPool* pItemPool ) :
    SfxSetItem( rTextAttr, pItemPool )
{
}

/*************************************************************************
|*
|* Clone-Funktion
|*
\************************************************************************/

SfxPoolItem* XTextAttrSetItem::Clone( SfxItemPool* pPool ) const
{
    return new XTextAttrSetItem( *this, pPool );
}

/*************************************************************************
|*
|* SetItem aus Stream erzeugen
|*
\************************************************************************/

SfxPoolItem* XTextAttrSetItem::Create( SvStream& rStream, USHORT nVersion ) const
{
    SfxItemSet *pSet = new SfxItemSet( *GetItemSet().GetPool(),
                                    XATTR_TEXT_FIRST, XATTR_TEXT_LAST);
    pSet->Load( rStream );
    return new XTextAttrSetItem( pSet );
}

/*************************************************************************
|*
|* SetItem in Stream speichern
|*
\************************************************************************/

SvStream& XTextAttrSetItem::Store( SvStream& rStream, USHORT nItemVersion ) const
{
    return SfxSetItem::Store( rStream, nItemVersion );
}




