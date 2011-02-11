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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/DashStyle.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svl/itempool.hxx>
#include <editeng/memberids.hrc>
#include <tools/stream.hxx>

#include "svx/unoapi.hxx"
#include <svl/style.hxx>
#include "unopolyhelper.hxx"

#include <tools/bigint.hxx>
#include <svl/itemset.hxx>
#include <svx/dialogs.hrc>
#include "svx/svdstr.hrc"
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/itemtype.hxx>
#include <svx/xdef.hxx>
#include <svx/unomid.hxx>
#include <svx/svdmodel.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>

#include <stdio.h>

using namespace ::rtl;
using namespace ::com::sun::star;

#define GLOBALOVERFLOW

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

/************************************************************************/

#define VCLTOSVCOL( rCol ) (sal_uInt16)((((sal_uInt16)(rCol))<<8)|(rCol))

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
|*    NameOrIndex::NameOrIndex(sal_uInt16 nWhich, sal_Int32 nIndex)
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

NameOrIndex::NameOrIndex(sal_uInt16 _nWhich, sal_Int32 nIndex) :
    SfxStringItem(_nWhich, aNameOrIndexEmptyString),
    nPalIndex(nIndex)
{
}

/*************************************************************************
|*
|*    NameOrIndex::NameOrIndex(sal_uInt16 nWhich, const String& rName)
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

NameOrIndex::NameOrIndex(sal_uInt16 _nWhich, const XubString& rName) :
    SfxStringItem(_nWhich, rName),
    nPalIndex(-1)
{
}

/*************************************************************************
|*
|*    NameOrIndex::NameOrIndex(sal_uInt16 nWhich, SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

NameOrIndex::NameOrIndex(sal_uInt16 _nWhich, SvStream& rIn) :
    SfxStringItem(_nWhich, rIn)
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

SfxPoolItem* NameOrIndex::Clone(SfxItemPool* /*pPool*/) const
{

    return new NameOrIndex(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* NameOrIndex::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    14.11.94
|*    Letzte Aenderung  14.11.94
|*
*************************************************************************/

SfxPoolItem* NameOrIndex::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SvStream& NameOrIndex::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    SfxStringItem::Store( rOut, nItemVersion );
    rOut << nPalIndex;
    return rOut;
}

/** this static checks if the given NameOrIndex item has a unique name for its value.
    The returned String is a unique name for an item with this value in both given pools.
    Argument pPool2 can be null.
    If returned string equals NameOrIndex->GetName(), the name was already unique.
*/
String NameOrIndex::CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, const SfxItemPool* /*pPool2*/, SvxCompareValueFunc pCompareValueFunc, sal_uInt16 nPrefixResId, XPropertyList* pDefaults )
{
    sal_Bool bForceNew = sal_False;

    String aUniqueName;
    SvxUnogetInternalNameForItem( nWhich, pCheckItem->GetName(), aUniqueName );

    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different line end or start

    if( aUniqueName.Len() && pPool1 )
    {
        const sal_uInt32 nCount = pPool1->GetItemCount2( nWhich );

        const NameOrIndex *pItem;
        for( sal_uInt32 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (NameOrIndex*)pPool1->GetItem2( nWhich, nSurrogate );

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
        String aUser( aRes );
        aUser += sal_Unicode( ' ' );

        if( pDefaults )
        {
            const int nCount = pDefaults->Count();
            int nIndex;
            for( nIndex = 0; nIndex < nCount; nIndex++ )
            {
                XPropertyEntry* pEntry = pDefaults->Get( nIndex, 0 );
                if( pEntry )
                {
                    bool bFound = false;

                    switch( nWhich )
                    {
                    case XATTR_FILLBITMAP:
                        bFound =  (((XFillBitmapItem*)pCheckItem)->GetBitmapValue().GetGraphicObject().GetUniqueID() ==
                            ((XBitmapEntry*)pEntry)->GetXBitmap().GetGraphicObject().GetUniqueID());
                        break;
                    case XATTR_LINEDASH:
                        bFound = (((XLineDashItem*)pCheckItem)->GetDashValue() == ((XDashEntry*)pEntry) ->GetDash());
                        break;
                    case XATTR_LINESTART:
                        bFound = (((XLineStartItem*)pCheckItem)->GetLineStartValue() == ((XLineEndEntry*)pEntry)->GetLineEnd());
                        break;
                    case XATTR_LINEEND:
                        bFound = (((XLineEndItem*)pCheckItem)->GetLineEndValue() == ((XLineEndEntry*)pEntry)->GetLineEnd());
                        break;
                    case XATTR_FILLGRADIENT:
                        bFound = (((XFillGradientItem*)pCheckItem)->GetGradientValue() == ((XGradientEntry*)pEntry)->GetGradient());
                        break;
                    case XATTR_FILLHATCH:
                        bFound = (((XFillHatchItem*)pCheckItem)->GetHatchValue() == ((XHatchEntry*)pEntry)->GetHatch());
                        break;
                    }

                    if( bFound )
                    {
                        aUniqueName = pEntry->GetName();
                        break;
                    }
                    else
                    {
                        sal_Int32 nThisIndex = pEntry->GetName().Copy( aUser.Len() ).ToInt32();
                        if( nThisIndex >= nUserIndex )
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }
        }

        if( (aUniqueName.Len() == 0) && pPool1 )
        {
            const sal_uInt32 nCount = pPool1->GetItemCount2( nWhich );
            const NameOrIndex *pItem;
            for( sal_uInt32 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (NameOrIndex*)pPool1->GetItem2( nWhich, nSurrogate );

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
            aUniqueName = aUser;
            aUniqueName += String::CreateFromInt32( nUserIndex );
        }
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
|*    XColorItem::XColorItem(sal_uInt16 nWhich, sal_Int32 nIndex, const Color& rTheColor)
|*
\************************************************************************/

XColorItem::XColorItem(sal_uInt16 _nWhich, sal_Int32 nIndex, const Color& rTheColor) :
    NameOrIndex(_nWhich, nIndex),
    aColor(rTheColor)
{
}

/*************************************************************************
|*
|*    XColorItem::XColorItem(sal_uInt16 nWhich, const String& rName, const Color& rTheColor)
|*
\************************************************************************/

XColorItem::XColorItem(sal_uInt16 _nWhich, const XubString& rName, const Color& rTheColor) :
    NameOrIndex(_nWhich, rName),
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
|*    XColorItem::XColorItem(sal_uInt16 nWhich, SvStream& rIn)
|*
\************************************************************************/

XColorItem::XColorItem(sal_uInt16 _nWhich, SvStream& rIn) :
    NameOrIndex(_nWhich, rIn)
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

SfxPoolItem* XColorItem::Clone(SfxItemPool* /*pPool*/) const
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
|*    SfxPoolItem* XColorItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
\************************************************************************/

SfxPoolItem* XColorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XColorItem(Which(), rIn);
}

/*************************************************************************
|*
|*    SfxPoolItem* XColorItem::Store(SvStream& rOut) const
|*
\************************************************************************/

SvStream& XColorItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
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
|*    const XColor& XColorItem::GetColorValue(const XColorTable* pTable) const
|*
\************************************************************************/

const Color& XColorItem::GetColorValue(const XColorTable* pTable) const
{
    if (!IsIndex())
        return aColor;
    else
        return pTable->GetColor(GetIndex())->GetColor();

}

sal_Bool XColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetColorValue().GetRGBColor();
    return sal_True;
}

sal_Bool XColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetColorValue( nValue );

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
    SfxEnumItem(XATTR_LINESTYLE, sal::static_int_cast< sal_uInt16 >(eTheLineStyle))
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

SfxPoolItem* XLineStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStyleItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStyleItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStyleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineStyleItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineStyleItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
            sal_uInt16 nId = 0;

            switch( (sal_uInt16)GetValue() )
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineStyleItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    ::com::sun::star::drawing::LineStyle eLS = (::com::sun::star::drawing::LineStyle)GetValue();
    rVal <<= eLS;
    return sal_True;
}

sal_Bool XLineStyleItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    ::com::sun::star::drawing::LineStyle eLS;
    if(!(rVal >>= eLS ))
    {
        // also try an int (for Basic)
        sal_Int32 nLS = 0;
        if(!(rVal >>= nLS))
            return sal_False;
        eLS = (::com::sun::star::drawing::LineStyle)nLS;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( eLS ) );
    return sal_True;
}

//------------------------------------------------------------------------

sal_uInt16 XLineStyleItem::GetValueCount() const
{
    return 3;
}


// ------------
// class XDash
// ------------
/*************************************************************************
|*
|*    XDash::XDash(XDashStyle, sal_uInt16, sal_uIntPtr, sal_uInt16, sal_uIntPtr, sal_uIntPtr)
|*
|*    Beschreibung
|*    Ersterstellung    21.11.94
|*    Letzte Aenderung  21.11.94
|*
*************************************************************************/

XDash::XDash(XDashStyle eTheDash, sal_uInt16 nTheDots, sal_uIntPtr nTheDotLen,
             sal_uInt16 nTheDashes, sal_uIntPtr nTheDashLen, sal_uIntPtr nTheDistance) :
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

bool XDash::operator==(const XDash& rDash) const
{
    return ( eDash      == rDash.eDash      &&
             nDots      == rDash.nDots      &&
             nDotLen    == rDash.nDotLen    &&
             nDashes    == rDash.nDashes    &&
             nDashLen   == rDash.nDashLen   &&
             nDistance  == rDash.nDistance );
}

// XDash is translated into an array of doubles which describe the lengths of the
// dashes, dots and empty passages. It returns the complete length of the full DashDot
// sequence and fills the given vetor of doubles accordingly (also resizing, so deleting it).
static double SMALLEST_DASH_WIDTH(26.95);
double XDash::CreateDotDashArray(::std::vector< double >& rDotDashArray, double fLineWidth) const
{
    double fFullDotDashLen(0.0);
    const sal_uInt16 nNumDotDashArray = (GetDots() + GetDashes()) * 2;
    rDotDashArray.resize( nNumDotDashArray, 0.0 );
    sal_uInt16 a;
    sal_uInt16 nIns(0);
    double fDashDotDistance = (double)GetDistance();
    double fSingleDashLen = (double)GetDashLen();
    double fSingleDotLen = (double)GetDotLen();

    if(GetDashStyle() == XDASH_RECTRELATIVE || GetDashStyle() == XDASH_ROUNDRELATIVE)
    {
        if(fLineWidth != 0.0)
        {
            double fFactor = fLineWidth / 100.0;

            if(GetDashes())
            {
                if(GetDashLen())
                {
                    // is a dash
                    fSingleDashLen *= fFactor;
                }
                else
                {
                    // is a dot
                    fSingleDashLen = fLineWidth;
                }
            }

            if(GetDots())
            {
                if(GetDotLen())
                {
                    // is a dash
                    fSingleDotLen *= fFactor;
                }
                else
                {
                    // is a dot
                    fSingleDotLen = fLineWidth;
                }
            }

            if(GetDashes() || GetDots())
            {
                if(GetDistance())
                {
                    fDashDotDistance *= fFactor;
                }
                else
                {
                    fDashDotDistance = fLineWidth;
                }
            }
        }
        else
        {
            if(GetDashes())
            {
                if(GetDashLen())
                {
                    // is a dash
                    fSingleDashLen = (SMALLEST_DASH_WIDTH * fSingleDashLen) / 100.0;
                }
                else
                {
                    // is a dot
                    fSingleDashLen = SMALLEST_DASH_WIDTH;
                }
            }

            if(GetDots())
            {
                if(GetDotLen())
                {
                    // is a dash
                    fSingleDotLen = (SMALLEST_DASH_WIDTH * fSingleDotLen) / 100.0;
                }
                else
                {
                    // is a dot
                    fSingleDotLen = SMALLEST_DASH_WIDTH;
                }
            }

            if(GetDashes() || GetDots())
            {
                if(GetDistance())
                {
                    // dash as distance
                    fDashDotDistance = (SMALLEST_DASH_WIDTH * fDashDotDistance) / 100.0;
                }
                else
                {
                    // dot as distance
                    fDashDotDistance = SMALLEST_DASH_WIDTH;
                }
            }
        }
    }
    else
    {
        // smallest dot size compare value
        double fDotCompVal(fLineWidth != 0.0 ? fLineWidth : SMALLEST_DASH_WIDTH);

        // absolute values
        if(GetDashes())
        {
            if(GetDashLen())
            {
                // is a dash
                if(fSingleDashLen < SMALLEST_DASH_WIDTH)
                {
                    fSingleDashLen = SMALLEST_DASH_WIDTH;
                }
            }
            else
            {
                // is a dot
                if(fSingleDashLen < fDotCompVal)
                {
                    fSingleDashLen = fDotCompVal;
                }
            }
        }

        if(GetDots())
        {
            if(GetDotLen())
            {
                // is a dash
                if(fSingleDotLen < SMALLEST_DASH_WIDTH)
                {
                    fSingleDotLen = SMALLEST_DASH_WIDTH;
                }
            }
            else
            {
                // is a dot
                if(fSingleDotLen < fDotCompVal)
                {
                    fSingleDotLen = fDotCompVal;
                }
            }
        }

        if(GetDashes() || GetDots())
        {
            if(GetDistance())
            {
                // dash as distance
                if(fDashDotDistance < SMALLEST_DASH_WIDTH)
                {
                    fDashDotDistance = SMALLEST_DASH_WIDTH;
                }
            }
            else
            {
                // dot as distance
                if(fDashDotDistance < fDotCompVal)
                {
                    fDashDotDistance = fDotCompVal;
                }
            }
        }
    }

    for(a=0;a<GetDots();a++)
    {
        rDotDashArray[nIns++] = fSingleDotLen;
        fFullDotDashLen += fSingleDotLen;
        rDotDashArray[nIns++] = fDashDotDistance;
        fFullDotDashLen += fDashDotDistance;
    }

    for(a=0;a<GetDashes();a++)
    {
        rDotDashArray[nIns++] = fSingleDashLen;
        fFullDotDashLen += fSingleDashLen;
        rDotDashArray[nIns++] = fDashDotDistance;
        fFullDotDashLen += fDashDotDistance;
    }

    return fFullDotDashLen;
}

// -------------------
// class XLineDashItem
// -------------------
TYPEINIT1_AUTOFACTORY(XLineDashItem, NameOrIndex);

/*************************************************************************
|*
|*    XLineDashItem::XLineDashItem(sal_Int32 nIndex, const XDash& rTheDash)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineDashItem::XLineDashItem(sal_Int32 nIndex, const XDash& rTheDash) :
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
        sal_uInt16  nSTemp;
        sal_uInt32  nLTemp;
        sal_Int32   nITemp;

        rIn >> nITemp; aDash.SetDashStyle((XDashStyle)nITemp);
        rIn >> nSTemp; aDash.SetDots(nSTemp);
        rIn >> nLTemp; aDash.SetDotLen(nLTemp);
        rIn >> nSTemp; aDash.SetDashes(nSTemp);
        rIn >> nLTemp; aDash.SetDashLen(nLTemp);
        rIn >> nLTemp; aDash.SetDistance(nLTemp);
    }
}

//*************************************************************************

XLineDashItem::XLineDashItem(SfxItemPool* /*pPool*/, const XDash& rTheDash)
:   NameOrIndex( XATTR_LINEDASH, -1 ),
    aDash(rTheDash)
{
}

//*************************************************************************

XLineDashItem::XLineDashItem(SfxItemPool* /*pPool*/)
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

SfxPoolItem* XLineDashItem::Clone(SfxItemPool* /*pPool*/) const
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
|*    SfxPoolItem* XLineDashItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineDashItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SvStream& XLineDashItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut << (sal_Int32) aDash.GetDashStyle();
        rOut << aDash.GetDots();
        rOut << (sal_uInt32) aDash.GetDotLen();
        rOut << aDash.GetDashes();
        rOut << (sal_uInt32) aDash.GetDashLen();
        rOut << (sal_uInt32) aDash.GetDistance();
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

const XDash& XLineDashItem::GetDashValue(const XDashTable* pTable) const // GetValue -> GetDashValue
{
    if (!IsIndex())
        return aDash;
    else
        return pTable->GetDash(GetIndex())->GetDash();
}

//------------------------------------------------------------------------

SfxItemPresentation XLineDashItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

//------------------------------------------------------------------------

FASTBOOL XLineDashItem::HasMetrics() const
{
    return sal_True;
}

//------------------------------------------------------------------------

FASTBOOL XLineDashItem::ScaleMetrics(long nMul, long nDiv)
{
    aDash.SetDotLen( ScaleMetricValue( aDash.GetDotLen(), nMul, nDiv ) );
    aDash.SetDashLen( ScaleMetricValue( aDash.GetDashLen(), nMul, nDiv ) );
    aDash.SetDistance( ScaleMetricValue( aDash.GetDistance(), nMul, nDiv ) );
    return sal_True;
}

sal_Bool XLineDashItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue > aPropSeq( 2 );

            ::com::sun::star::drawing::LineDash aLineDash;

            const XDash& rXD = GetDashValue();
            aLineDash.Style = (::com::sun::star::drawing::DashStyle)((sal_uInt16)rXD.GetDashStyle());
            aLineDash.Dots = rXD.GetDots();
            aLineDash.DotLen = rXD.GetDotLen();
            aLineDash.Dashes = rXD.GetDashes();
            aLineDash.DashLen = rXD.GetDashLen();
            aLineDash.Distance = rXD.GetDistance();

            rtl::OUString aApiName;
            SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
            aPropSeq[0].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ));
            aPropSeq[0].Value   = uno::makeAny( aApiName );
            aPropSeq[1].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LineDash" ));
            aPropSeq[1].Value   = uno::makeAny( aLineDash );
            rVal = uno::makeAny( aPropSeq );
            break;
        }

        case MID_NAME:
        {
            rtl::OUString aApiName;
            SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
            rVal <<= aApiName;
            break;
        }

        case MID_LINEDASH:
        {
            const XDash& rXD = GetDashValue();

            ::com::sun::star::drawing::LineDash aLineDash;

            aLineDash.Style = (::com::sun::star::drawing::DashStyle)((sal_uInt16)rXD.GetDashStyle());
            aLineDash.Dots = rXD.GetDots();
            aLineDash.DotLen = rXD.GetDotLen();
            aLineDash.Dashes = rXD.GetDashes();
            aLineDash.DashLen = rXD.GetDashLen();
            aLineDash.Distance = rXD.GetDistance();

            rVal <<= aLineDash;
            break;
        }

        case MID_LINEDASH_STYLE:
        {
            const XDash& rXD = GetDashValue();
            rVal <<= (::com::sun::star::drawing::DashStyle)((sal_Int16)rXD.GetDashStyle());
            break;
        }

        case MID_LINEDASH_DOTS:
        {
            const XDash& rXD = GetDashValue();
            rVal <<= rXD.GetDots();
            break;
        }

        case MID_LINEDASH_DOTLEN:
        {
            const XDash& rXD = GetDashValue();
            rVal <<= rXD.GetDotLen();
            break;
        }

        case MID_LINEDASH_DASHES:
        {
            const XDash& rXD = GetDashValue();
            rVal <<= rXD.GetDashes();
            break;
        }

        case MID_LINEDASH_DASHLEN:
        {
            const XDash& rXD = GetDashValue();
            rVal <<= rXD.GetDashLen();
            break;
        }

        case MID_LINEDASH_DISTANCE:
        {
            const XDash& rXD = GetDashValue();
            rVal <<= rXD.GetDistance();
            break;
        }

        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
}

sal_Bool XLineDashItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue >   aPropSeq;
            ::com::sun::star::drawing::LineDash     aLineDash;
            rtl::OUString                           aName;
            sal_Bool                                bLineDash( sal_False );

            if ( rVal >>= aPropSeq )
            {
                for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
                {
                    if ( aPropSeq[n].Name.equalsAsciiL( "Name", 4  ))
                        aPropSeq[n].Value >>= aName;
                    else if ( aPropSeq[n].Name.equalsAsciiL( "LineDash", 8 ))
                    {
                        if ( aPropSeq[n].Value >>= aLineDash )
                            bLineDash = sal_True;
                    }
                }

                SetName( aName );
                if ( bLineDash )
                {
                    XDash aXDash;

                    aXDash.SetDashStyle((XDashStyle)((sal_uInt16)(aLineDash.Style)));
                    aXDash.SetDots(aLineDash.Dots);
                    aXDash.SetDotLen(aLineDash.DotLen);
                    aXDash.SetDashes(aLineDash.Dashes);
                    aXDash.SetDashLen(aLineDash.DashLen);
                    aXDash.SetDistance(aLineDash.Distance);

                    if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
                        aXDash.SetDots(1);

                    SetDashValue( aXDash );
                }

                return sal_True;
            }

            return sal_False;
        }

        case MID_NAME:
        {
            rtl::OUString aName;
            if (!(rVal >>= aName))
                return sal_False;
            SetName( aName );
            break;
        }

        case MID_LINEDASH:
        {
            ::com::sun::star::drawing::LineDash aLineDash;
            if(!(rVal >>= aLineDash))
                return sal_False;

            XDash aXDash;

            aXDash.SetDashStyle((XDashStyle)((sal_uInt16)(aLineDash.Style)));
            aXDash.SetDots(aLineDash.Dots);
            aXDash.SetDotLen(aLineDash.DotLen);
            aXDash.SetDashes(aLineDash.Dashes);
            aXDash.SetDashLen(aLineDash.DashLen);
            aXDash.SetDistance(aLineDash.Distance);

            if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
                aXDash.SetDots(1);

            SetDashValue( aXDash );
            break;
        }

        case MID_LINEDASH_STYLE:
        {
            sal_Int16 nVal = sal_Int16();
            if(!(rVal >>= nVal))
                return sal_False;

            XDash aXDash = GetDashValue();
            aXDash.SetDashStyle((XDashStyle)((sal_uInt16)(nVal)));

            if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
                aXDash.SetDots(1);

            SetDashValue( aXDash );

            break;
        }

        case MID_LINEDASH_DOTS:
        case MID_LINEDASH_DASHES:
        {
            sal_Int16 nVal = sal_Int16();
            if(!(rVal >>= nVal))
                return sal_False;

            XDash aXDash = GetDashValue();
            if ( nMemberId == MID_LINEDASH_DOTS )
                aXDash.SetDots( nVal );
            else
                aXDash.SetDashes( nVal );

            if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
                aXDash.SetDots(1);

            SetDashValue( aXDash );
            break;
        }

        case MID_LINEDASH_DOTLEN:
        case MID_LINEDASH_DASHLEN:
        case MID_LINEDASH_DISTANCE:
        {
            sal_Int32 nVal = 0;
            if(!(rVal >>= nVal))
                return sal_False;

            XDash aXDash = GetDashValue();
            if ( nMemberId == MID_LINEDASH_DOTLEN )
                aXDash.SetDotLen( nVal );
            else if ( nMemberId == MID_LINEDASH_DASHLEN )
                aXDash.SetDashLen( nVal );
            else
                aXDash.SetDistance( nVal );

            if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
                aXDash.SetDots(1);

            SetDashValue( aXDash );
            break;
        }
    }

    return sal_True;
}

sal_Bool XLineDashItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return ((XLineDashItem*)p1)->GetDashValue() == ((XLineDashItem*)p2)->GetDashValue();
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

SfxPoolItem* XLineWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineWidthItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineWidthItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineWidthItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineWidthItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineWidthItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const IntlWrapper * pIntl
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
                                    eCoreUnit, ePresUnit, pIntl);
            rText += SVX_RESSTR( GetMetricId( ePresUnit) );
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Int32 nValue = GetValue();
    if( 0 != (nMemberId&CONVERT_TWIPS) )
        nValue = TWIP_TO_MM100(nValue);

    rVal <<= nValue;
    return sal_True;
}

sal_Bool XLineWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    if( 0 != (nMemberId&CONVERT_TWIPS) )
        nValue = MM100_TO_TWIP(nValue);

    SetValue( nValue );
    return sal_True;
}

// -------------------
// class XLineColorItem
// -------------------
TYPEINIT1_AUTOFACTORY(XLineColorItem, XColorItem);

/*************************************************************************
|*
|*    XLineColorItem::XLineColorItem(sal_Int32 nIndex, const Color& rTheColor)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineColorItem::XLineColorItem(sal_Int32 nIndex, const Color& rTheColor) :
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

SfxPoolItem* XLineColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineColorItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineColorItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineColorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineColorItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetColorValue().GetRGBColor();
    return sal_True;
}

sal_Bool XLineColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return sal_False;

    SetColorValue( nValue );
    return sal_True;
}

//////////////////////////////////////////////////////////////////////////////
// tooling for simple spooling B2DPolygon to file and back

namespace
{
    void streamOutB2DPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, SvStream& rOut)
    {
        const sal_uInt32 nPolygonCount(rPolyPolygon.count());
        rOut << nPolygonCount;

        for(sal_uInt32 a(0L); a < nPolygonCount; a++)
        {
            const basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(a));
            const sal_uInt32 nPointCount(aCandidate.count());
            const sal_uInt8 bClosed(aCandidate.isClosed() ? 1 : 0);
            const sal_uInt8 bControlPoints(aCandidate.areControlPointsUsed() ? 1 : 0);
            rOut << nPointCount;
            rOut << bClosed;
            rOut << bControlPoints;

            for(sal_uInt32 b(0L); b < nPointCount; b++)
            {
                const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(b));
                rOut << aPoint.getX();
                rOut << aPoint.getY();

                if(bControlPoints)
                {
                    const sal_uInt8 bEdgeIsCurve(aCandidate.isPrevControlPointUsed(b) || aCandidate.isNextControlPointUsed(b) ? 1 : 0);
                    rOut << bEdgeIsCurve;

                    if(bEdgeIsCurve)
                    {
                        const basegfx::B2DVector aControlVectorA(aCandidate.getPrevControlPoint(b));
                        rOut << aControlVectorA.getX();
                        rOut << aControlVectorA.getY();

                        const basegfx::B2DVector aControlVectorB(aCandidate.getNextControlPoint(b));
                        rOut << aControlVectorB.getX();
                        rOut << aControlVectorB.getY();
                    }
                }
            }
        }
    }

    basegfx::B2DPolyPolygon streamInB2DPolyPolygon(SvStream& rIn)
    {
        basegfx::B2DPolyPolygon aRetval;
        sal_uInt32 nPolygonCount;
        rIn >> nPolygonCount;

        for(sal_uInt32 a(0L); a < nPolygonCount; a++)
        {
            sal_uInt32 nPointCount;
            sal_uInt8 bClosed;
            sal_uInt8 bControlPoints;

            rIn >> nPointCount;
            rIn >> bClosed;
            rIn >> bControlPoints;

            basegfx::B2DPolygon aCandidate;
            aCandidate.setClosed(0 != bClosed);

            for(sal_uInt32 b(0L); b < nPointCount; b++)
            {
                double fX, fY;
                rIn >> fX;
                rIn >> fY;
                aCandidate.append(basegfx::B2DPoint(fX, fY));

                if(0 != bControlPoints)
                {
                    sal_uInt8 bEdgeIsCurve;
                    rIn >> bEdgeIsCurve;

                    if(0 != bEdgeIsCurve)
                    {
                        rIn >> fX;
                        rIn >> fY;
                        aCandidate.setPrevControlPoint(b, basegfx::B2DVector(fX, fY));

                        rIn >> fX;
                        rIn >> fY;
                        aCandidate.setNextControlPoint(b, basegfx::B2DVector(fX, fY));
                    }
                }
            }

            aRetval.append(aCandidate);
        }

        return aRetval;
    }
}

//////////////////////////////////////////////////////////////////////////////

// -----------------------
// class XLineStartItem
// -----------------------
TYPEINIT1_AUTOFACTORY(XLineStartItem, NameOrIndex);

/*************************************************************************
|*
|*    XLineStartItem::XLineStartItem(sal_Int32 nIndex)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineStartItem::XLineStartItem(sal_Int32 nIndex)
:   NameOrIndex(XATTR_LINESTART, nIndex)
{
}

/*************************************************************************
|*
|*    XLineStartItem::XLineStartItem(const XubString& rName,
|*                                   const basegfx::B2DPolyPolygon& rXPolygon)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineStartItem::XLineStartItem(const XubString& rName, const basegfx::B2DPolyPolygon& rPolyPolygon)
:   NameOrIndex(XATTR_LINESTART, rName),
    maPolyPolygon(rPolyPolygon)
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

XLineStartItem::XLineStartItem(const XLineStartItem& rItem)
:   NameOrIndex(rItem),
    maPolyPolygon(rItem.maPolyPolygon)
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
        maPolyPolygon = streamInB2DPolyPolygon(rIn);
    }
}

//*************************************************************************

XLineStartItem::XLineStartItem(SfxItemPool* /*pPool*/, const basegfx::B2DPolyPolygon& rPolyPolygon)
:   NameOrIndex( XATTR_LINESTART, -1 ),
    maPolyPolygon(rPolyPolygon)
{
}

//*************************************************************************

XLineStartItem::XLineStartItem(SfxItemPool* /*pPool*/)
:   NameOrIndex(XATTR_LINESTART, -1 )
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

SfxPoolItem* XLineStartItem::Clone(SfxItemPool* /*pPool*/) const
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
    return ( NameOrIndex::operator==(rItem) && ((const XLineStartItem&) rItem).maPolyPolygon == maPolyPolygon );
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStartItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SvStream& XLineStartItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        streamOutB2DPolyPolygon(maPolyPolygon, rOut);
    }

    return rOut;
}

/*************************************************************************
|*
|*    const basegfx::B2DPolyPolygon& XLineStartItem::GetValue(const XLineEndTable* pTable)
|*                                             const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

basegfx::B2DPolyPolygon XLineStartItem::GetLineStartValue(const XLineEndTable* pTable) const
{
    if (!IsIndex())
    {
        return maPolyPolygon;
    }
    else
    {
        return pTable->GetLineEnd(GetIndex())->GetLineEnd();
    }
}

//------------------------------------------------------------------------

SfxItemPresentation XLineStartItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
        return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineStartItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    if( nMemberId == MID_NAME )
    {
        rtl::OUString aApiName;
        SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
        rVal <<= aApiName;
    }
    else
    {
        com::sun::star::drawing::PolyPolygonBezierCoords aBezier;
        SvxConvertB2DPolyPolygonToPolyPolygonBezier( maPolyPolygon, aBezier );
        rVal <<= aBezier;
    }

    return sal_True;
}

sal_Bool XLineStartItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    if( nMemberId == MID_NAME )
    {
        return sal_False;
    }
    else
    {
        maPolyPolygon.clear();

        if( rVal.hasValue() && rVal.getValue() )
        {
            if( rVal.getValueType() != ::getCppuType((const com::sun::star::drawing::PolyPolygonBezierCoords*)0) )
                return sal_False;

            com::sun::star::drawing::PolyPolygonBezierCoords* pCoords = (com::sun::star::drawing::PolyPolygonBezierCoords*)rVal.getValue();
            if( pCoords->Coordinates.getLength() > 0 )
            {
                maPolyPolygon = SvxConvertPolyPolygonBezierToB2DPolyPolygon( pCoords );
                // #i72807# close line start/end polygons hard
                // maPolyPolygon.setClosed(true);
            }
        }
    }

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

        if( !maPolyPolygon.count() )
        {
            // if the polygon is empty, check if the name is empty
            if( aUniqueName.Len() == 0 )
                return (XLineStartItem*)this;

            // force empty name for empty polygons
            return new XLineStartItem( String(), maPolyPolygon );
        }

        if( maPolyPolygon.count() > 1L )
        {
            // check if the polygon is closed
            if(!maPolyPolygon.isClosed())
            {
                // force a closed polygon
                basegfx::B2DPolyPolygon aNew(maPolyPolygon);
                aNew.setClosed(true);
                pTempItem = new XLineStartItem( aUniqueName, aNew );
                pLineStartItem = pTempItem;
            }
        }

        sal_Bool bForceNew = sal_False;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        sal_uInt32 nCount, nSurrogate;

        const SfxItemPool* pPool1 = &pModel->GetItemPool();
        if( aUniqueName.Len() && pPool1 )
        {
            nCount = pPool1->GetItemCount2( XATTR_LINESTART );

            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem2( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetLineStartValue() != pLineStartItem->GetLineStartValue() )
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
                nCount = pPool1->GetItemCount2( XATTR_LINEEND );

                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem2( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetLineEndValue() != pLineStartItem->GetLineStartValue() )
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
            nCount = pPool2->GetItemCount2( XATTR_LINESTART );
            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = (const XLineStartItem*)pPool2->GetItem2( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetLineStartValue() != pLineStartItem->GetLineStartValue() )
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
                nCount = pPool2->GetItemCount2( XATTR_LINEEND );
                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool2->GetItem2( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetLineEndValue() != pLineStartItem->GetLineStartValue() )
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
                nCount = pPool1->GetItemCount2( XATTR_LINESTART );
                sal_uInt32 nSurrogate2;

                for( nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++ )
                {
                    const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem2( XATTR_LINESTART, nSurrogate2 );

                    if( pItem && pItem->GetName().Len() )
                    {
                        if( !bForceNew && pItem->GetLineStartValue() == pLineStartItem->GetLineStartValue() )
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

                nCount = pPool1->GetItemCount2( XATTR_LINEEND );
                for( nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem2( XATTR_LINEEND, nSurrogate2 );

                    if( pItem && pItem->GetName().Len() )
                    {
                        if( !bForceNew && pItem->GetLineEndValue() == pLineStartItem->GetLineStartValue() )
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
                return new XLineStartItem( aUniqueName, maPolyPolygon );
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
|*    XLineEndItem::XLineEndItem(sal_Int32 nIndex)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineEndItem::XLineEndItem(sal_Int32 nIndex)
:   NameOrIndex(XATTR_LINEEND, nIndex)
{
}

/*************************************************************************
|*
|*    XLineEndItem::XLineEndItem(const XubString& rName,
|*                               const basegfx::B2DPolyPolygon& rXPolygon)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

XLineEndItem::XLineEndItem(const XubString& rName, const basegfx::B2DPolyPolygon& rPolyPolygon)
:   NameOrIndex(XATTR_LINEEND, rName),
    maPolyPolygon(rPolyPolygon)
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

XLineEndItem::XLineEndItem(const XLineEndItem& rItem)
:   NameOrIndex(rItem),
    maPolyPolygon(rItem.maPolyPolygon)
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
        maPolyPolygon = streamInB2DPolyPolygon(rIn);
    }
}

//*************************************************************************

XLineEndItem::XLineEndItem(SfxItemPool* /*pPool*/, const basegfx::B2DPolyPolygon& rPolyPolygon)
:   NameOrIndex( XATTR_LINEEND, -1 ),
    maPolyPolygon(rPolyPolygon)
{
}

//*************************************************************************

XLineEndItem::XLineEndItem(SfxItemPool* /*pPool*/)
:   NameOrIndex(XATTR_LINEEND, -1 )
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

SfxPoolItem* XLineEndItem::Clone(SfxItemPool* /*pPool*/) const
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
    return ( NameOrIndex::operator==(rItem) && ((const XLineEndItem&) rItem).maPolyPolygon == maPolyPolygon );
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineEndItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SvStream& XLineEndItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        streamOutB2DPolyPolygon(maPolyPolygon, rOut);
    }

    return rOut;
}

/*************************************************************************
|*
|*    const basegfx::B2DPolyPolygon& XLineEndItem::GetValue(const XLineEndTable* pTable) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  18.11.94
|*
*************************************************************************/

basegfx::B2DPolyPolygon XLineEndItem::GetLineEndValue(const XLineEndTable* pTable) const
{
    if (!IsIndex())
    {
        return maPolyPolygon;
    }
    else
    {
        return pTable->GetLineEnd(GetIndex())->GetLineEnd();
    }
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

        if( !maPolyPolygon.count() )
        {
            // if the polygon is empty, check if the name is empty
            if( aUniqueName.Len() == 0 )
                return (XLineEndItem*)this;

            // force empty name for empty polygons
            return new XLineEndItem( String(), maPolyPolygon );
        }

        if( maPolyPolygon.count() > 1L )
        {
            // check if the polygon is closed
            if(!maPolyPolygon.isClosed())
            {
                // force a closed polygon
                basegfx::B2DPolyPolygon aNew(maPolyPolygon);
                aNew.setClosed(true);
                pTempItem = new XLineEndItem( aUniqueName, aNew );
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
            nCount = pPool1->GetItemCount2( XATTR_LINESTART );

            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem2( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetLineStartValue() != pLineEndItem->GetLineEndValue() )
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
                nCount = pPool1->GetItemCount2( XATTR_LINEEND );

                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem2( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetLineEndValue() != pLineEndItem->GetLineEndValue() )
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
            nCount = pPool2->GetItemCount2( XATTR_LINESTART );
            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = (const XLineStartItem*)pPool2->GetItem2( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetLineStartValue() != pLineEndItem->GetLineEndValue() )
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
                nCount = pPool2->GetItemCount2( XATTR_LINEEND );
                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool2->GetItem2( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetLineEndValue() != pLineEndItem->GetLineEndValue() )
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
                nCount = pPool1->GetItemCount2( XATTR_LINESTART );
                sal_uInt32 nSurrogate2;

                for( nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++ )
                {
                    const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem2( XATTR_LINESTART, nSurrogate2 );

                    if( pItem && pItem->GetName().Len() )
                    {
                        if( !bForceNew && pItem->GetLineStartValue() == pLineEndItem->GetLineEndValue() )
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

                nCount = pPool1->GetItemCount2( XATTR_LINEEND );
                for( nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++ )
                {
                    const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem2( XATTR_LINEEND, nSurrogate2 );

                    if( pItem && pItem->GetName().Len() )
                    {
                        if( !bForceNew && pItem->GetLineEndValue() == pLineEndItem->GetLineEndValue() )
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
                return new XLineEndItem( aUniqueName, maPolyPolygon );
            }
        }
    }

    return (XLineEndItem*)this;
}


//------------------------------------------------------------------------

SfxItemPresentation XLineEndItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineEndItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    if( nMemberId == MID_NAME )
    {
        rtl::OUString aApiName;
        SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
        rVal <<= aApiName;
    }
    else
    {
        com::sun::star::drawing::PolyPolygonBezierCoords aBezier;
        SvxConvertB2DPolyPolygonToPolyPolygonBezier( maPolyPolygon, aBezier );
        rVal <<= aBezier;
    }
    return sal_True;
}

sal_Bool XLineEndItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    if( nMemberId == MID_NAME )
    {
        return sal_False;
    }
    else
    {
        maPolyPolygon.clear();

        if( rVal.hasValue() && rVal.getValue() )
        {
            if( rVal.getValueType() != ::getCppuType((const com::sun::star::drawing::PolyPolygonBezierCoords*)0) )
                return sal_False;

            com::sun::star::drawing::PolyPolygonBezierCoords* pCoords = (com::sun::star::drawing::PolyPolygonBezierCoords*)rVal.getValue();
            if( pCoords->Coordinates.getLength() > 0 )
            {
                maPolyPolygon = SvxConvertPolyPolygonBezierToB2DPolyPolygon( pCoords );
                // #i72807# close line start/end polygons hard
                // maPolyPolygon.setClosed(true);
            }
        }
    }

    return sal_True;
}

// ----------------------------
// class XLineStartWidthItem
// ----------------------------
TYPEINIT1_AUTOFACTORY(XLineStartWidthItem, SfxMetricItem);

/*************************************************************************
|*
|*    XLineStartWidthItem::XLineStartWidthItem(sal_Int32 nWidth)
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

SfxPoolItem* XLineStartWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStartWidthItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStartWidthItem::Create(SvStream& rIn, sal_uInt16 nVer)
|*                                              const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartWidthItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineStartWidthItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineStartWidthItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const IntlWrapper * pIntl
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
                                    eCoreUnit, ePresUnit, pIntl);
            rText += SVX_RESSTR( GetMetricId( ePresUnit) );
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineStartWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

sal_Bool XLineStartWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
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

SfxPoolItem* XLineEndWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineEndWidthItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineEndWidthItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndWidthItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineEndWidthItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineEndWidthItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const IntlWrapper *pIntl
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
                                    eCoreUnit, ePresUnit, pIntl);
            rText += SVX_RESSTR( GetMetricId( ePresUnit) );
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineEndWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

sal_Bool XLineEndWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
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
|*    XLineStartCenterItem::XLineStartCenterItem(sal_Bool bStartCenter)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineStartCenterItem::XLineStartCenterItem(sal_Bool bStartCenter) :
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

SfxPoolItem* XLineStartCenterItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStartCenterItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineStartCenterItem::Create(SvStream& rIn, sal_uInt16 nVer)
|*                                              const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineStartCenterItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineStartCenterItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineStartCenterItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineStartCenterItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    sal_Bool bValue = GetValue();
    rVal.setValue( &bValue, ::getCppuBooleanType()  );
    return sal_True;
}

sal_Bool XLineStartCenterItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
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
|*    XLineEndCenterItem::XLineEndCenterItem(sal_Bool bEndCenter)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XLineEndCenterItem::XLineEndCenterItem(sal_Bool bEndCenter) :
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

SfxPoolItem* XLineEndCenterItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineEndCenterItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineEndCenterItem::Create(SvStream& rIn, sal_uInt16 nVer)
|*                                              const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XLineEndCenterItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineEndCenterItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineEndCenterItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

sal_Bool XLineEndCenterItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    sal_Bool bValue = GetValue();
    rVal.setValue( &bValue, ::getCppuBooleanType()  );
    return sal_True;
}

sal_Bool XLineEndCenterItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
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
    SfxEnumItem(XATTR_FILLSTYLE, sal::static_int_cast< sal_uInt16 >(eFillStyle))
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

SfxPoolItem* XFillStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillStyleItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillStyleItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillStyleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillStyleItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XFillStyleItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
            sal_uInt16 nId = 0;

            switch( (sal_uInt16)GetValue() )
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

//------------------------------------------------------------------------

sal_uInt16 XFillStyleItem::GetValueCount() const
{
    return 5;
}

// -----------------------------------------------------------------------
sal_Bool XFillStyleItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    ::com::sun::star::drawing::FillStyle eFS = (::com::sun::star::drawing::FillStyle)GetValue();

    rVal <<= eFS;

    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool XFillStyleItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    ::com::sun::star::drawing::FillStyle eFS;
    if(!(rVal >>= eFS))
    {
        // also try an int (for Basic)
        sal_Int32 nFS = 0;
        if(!(rVal >>= nFS))
            return sal_False;
        eFS = (::com::sun::star::drawing::FillStyle)nFS;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( eFS ) );

    return sal_True;
}


// -------------------
// class XFillColorItem
// -------------------
TYPEINIT1_AUTOFACTORY(XFillColorItem, XColorItem);

/*************************************************************************
|*
|*    XFillColorItem::XFillColorItem(sal_Int32 nIndex, const Color& rTheColor)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillColorItem::XFillColorItem(sal_Int32 nIndex, const Color& rTheColor) :
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

SfxPoolItem* XFillColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillColorItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillColorItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillColorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillColorItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XFillColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

// -----------------------------------------------------------------------

sal_Bool XFillColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetColorValue().GetRGBColor();

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool XFillColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue ))
        return sal_False;

    SetColorValue( nValue );
    return sal_True;
}

// -----------------------------
// class XSecondaryFillColorItem
// -----------------------------
TYPEINIT1_AUTOFACTORY(XSecondaryFillColorItem, XColorItem);

XSecondaryFillColorItem::XSecondaryFillColorItem(sal_Int32 nIndex, const Color& rTheColor) :
    XColorItem(XATTR_SECONDARYFILLCOLOR, nIndex, rTheColor)
{
}

XSecondaryFillColorItem::XSecondaryFillColorItem(const XubString& rName, const Color& rTheColor) :
    XColorItem(XATTR_SECONDARYFILLCOLOR, rName, rTheColor)
{
}

XSecondaryFillColorItem::XSecondaryFillColorItem( SvStream& rIn ) :
    XColorItem(XATTR_SECONDARYFILLCOLOR, rIn)
{
}

SfxPoolItem* XSecondaryFillColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XSecondaryFillColorItem(*this);
}

SfxPoolItem* XSecondaryFillColorItem::Create( SvStream& rIn, sal_uInt16 nVer ) const
{
    if ( nVer >= 2 )
        return new XSecondaryFillColorItem( rIn );
    else
        return new XSecondaryFillColorItem( String(), Color(0,184,255) );
}
sal_uInt16 XSecondaryFillColorItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/ ) const
{
    return 2;
}
SfxItemPresentation XSecondaryFillColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

// ----------------
// class XGradient
// ----------------

/*************************************************************************
|*
|*    XGradient::XGradient()
|*
*************************************************************************/

XGradient::XGradient() :
    eStyle( XGRAD_LINEAR ),
    aStartColor( Color( COL_BLACK ) ),
    aEndColor( Color( COL_WHITE ) ),
    nAngle( 0 ),
    nBorder( 0 ),
    nOfsX( 50 ),
    nOfsY( 50 ),
    nIntensStart( 100 ),
    nIntensEnd( 100 ),
    nStepCount( 0 )
{
}

/*************************************************************************
|*
|*    XGradient::XGradient(XGradientStyle, const Color&, const Color&,
|*                         long, sal_uInt16, sal_uInt16, sal_uInt16)
|*
|*    Beschreibung
|*    Ersterstellung    21.11.94
|*    Letzte Aenderung  21.11.94
|*
*************************************************************************/

XGradient::XGradient(const Color& rStart, const Color& rEnd,
                     XGradientStyle eTheStyle, long nTheAngle, sal_uInt16 nXOfs,
                     sal_uInt16 nYOfs, sal_uInt16 nTheBorder,
                     sal_uInt16 nStartIntens, sal_uInt16 nEndIntens,
                     sal_uInt16 nSteps) :
    eStyle(eTheStyle),
    aStartColor(rStart),
    aEndColor(rEnd),
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

bool XGradient::operator==(const XGradient& rGradient) const
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
|*    XFillGradientItem::XFillGradientItem(sal_Int32 nIndex,
|*                                       const XGradient& rTheGradient)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillGradientItem::XFillGradientItem(sal_Int32 nIndex,
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

XFillGradientItem::XFillGradientItem(SvStream& rIn, sal_uInt16 nVer) :
    NameOrIndex(XATTR_FILLGRADIENT, rIn),
    aGradient(COL_BLACK, COL_WHITE)
{
    if (!IsIndex())
    {
        sal_uInt16 nUSTemp;
        sal_uInt16 nRed;
        sal_uInt16 nGreen;
        sal_uInt16 nBlue;
        sal_Int16  nITemp;
        sal_Int32  nLTemp;

        rIn >> nITemp; aGradient.SetGradientStyle((XGradientStyle)nITemp);
        rIn >> nRed;
        rIn >> nGreen;
        rIn >> nBlue;
        Color aCol;
        aCol = Color( (sal_uInt8)( nRed >> 8 ), (sal_uInt8)( nGreen >> 8 ), (sal_uInt8)( nBlue >> 8 ) );
        aGradient.SetStartColor( aCol );

        rIn >> nRed;
        rIn >> nGreen;
        rIn >> nBlue;
        aCol = Color( (sal_uInt8)( nRed >> 8 ), (sal_uInt8)( nGreen >> 8 ), (sal_uInt8)( nBlue >> 8 ) );
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

XFillGradientItem::XFillGradientItem(SfxItemPool* /*pPool*/, const XGradient& rTheGradient)
:   NameOrIndex( XATTR_FILLGRADIENT, -1 ),
    aGradient(rTheGradient)
{
}

//*************************************************************************

XFillGradientItem::XFillGradientItem(SfxItemPool* /*pPool*/)
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

SfxPoolItem* XFillGradientItem::Clone(SfxItemPool* /*pPool*/) const
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
|*    SfxPoolItem* XFillGradientItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillGradientItem::Create(SvStream& rIn, sal_uInt16 nVer) const
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

SvStream& XFillGradientItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut << (sal_Int16)aGradient.GetGradientStyle();

        sal_uInt16 nTmp;

        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetRed() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetGreen() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetBlue() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetRed() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetGreen() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetBlue() ); rOut << nTmp;

        rOut << (sal_Int32) aGradient.GetAngle();
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

const XGradient& XFillGradientItem::GetGradientValue(const XGradientTable* pTable) const // GetValue -> GetGradientValue
{
    if (!IsIndex())
        return aGradient;
    else
        return pTable->GetGradient(GetIndex())->GetGradient();
}


/*************************************************************************
|*
|*    sal_uInt16 XFillGradientItem::GetVersion() const
|*
|*    Beschreibung
|*    Ersterstellung    01.11.95
|*    Letzte Aenderung  01.11.95
|*
*************************************************************************/

sal_uInt16 XFillGradientItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/) const
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
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

// -----------------------------------------------------------------------
sal_Bool XFillGradientItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    //sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue > aPropSeq( 2 );

            ::com::sun::star::awt::Gradient aGradient2;

            const XGradient& aXGradient = GetGradientValue();
            aGradient2.Style = (::com::sun::star::awt::GradientStyle) aXGradient.GetGradientStyle();
            aGradient2.StartColor = (sal_Int32)aXGradient.GetStartColor().GetColor();
            aGradient2.EndColor = (sal_Int32)aXGradient.GetEndColor().GetColor();
            aGradient2.Angle = (short)aXGradient.GetAngle();
            aGradient2.Border = aXGradient.GetBorder();
            aGradient2.XOffset = aXGradient.GetXOffset();
            aGradient2.YOffset = aXGradient.GetYOffset();
            aGradient2.StartIntensity = aXGradient.GetStartIntens();
            aGradient2.EndIntensity = aXGradient.GetEndIntens();
            aGradient2.StepCount = aXGradient.GetSteps();

            rtl::OUString aApiName;
            SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
            aPropSeq[0].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ));
            aPropSeq[0].Value   = uno::makeAny( aApiName );
            aPropSeq[1].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ));
            aPropSeq[1].Value   = uno::makeAny( aGradient2 );
            rVal = uno::makeAny( aPropSeq );
            break;
        }

        case MID_FILLGRADIENT:
        {
            const XGradient& aXGradient = GetGradientValue();
            ::com::sun::star::awt::Gradient aGradient2;

            aGradient2.Style = (::com::sun::star::awt::GradientStyle) aXGradient.GetGradientStyle();
            aGradient2.StartColor = (sal_Int32)aXGradient.GetStartColor().GetColor();
            aGradient2.EndColor = (sal_Int32)aXGradient.GetEndColor().GetColor();
            aGradient2.Angle = (short)aXGradient.GetAngle();
            aGradient2.Border = aXGradient.GetBorder();
            aGradient2.XOffset = aXGradient.GetXOffset();
            aGradient2.YOffset = aXGradient.GetYOffset();
            aGradient2.StartIntensity = aXGradient.GetStartIntens();
            aGradient2.EndIntensity = aXGradient.GetEndIntens();
            aGradient2.StepCount = aXGradient.GetSteps();

            rVal <<= aGradient2;
            break;
        }

        case MID_NAME:
        {
            rtl::OUString aApiName;
            SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
            rVal <<= aApiName;
            break;
        }

        case MID_GRADIENT_STYLE: rVal <<= (sal_Int16)GetGradientValue().GetGradientStyle(); break;
        case MID_GRADIENT_STARTCOLOR: rVal <<= (sal_Int32)GetGradientValue().GetStartColor().GetColor(); break;
        case MID_GRADIENT_ENDCOLOR: rVal <<= (sal_Int32)GetGradientValue().GetEndColor().GetColor(); break;
        case MID_GRADIENT_ANGLE: rVal <<= (sal_Int16)GetGradientValue().GetAngle(); break;
        case MID_GRADIENT_BORDER: rVal <<= GetGradientValue().GetBorder(); break;
        case MID_GRADIENT_XOFFSET: rVal <<= GetGradientValue().GetXOffset(); break;
        case MID_GRADIENT_YOFFSET: rVal <<= GetGradientValue().GetYOffset(); break;
        case MID_GRADIENT_STARTINTENSITY: rVal <<= GetGradientValue().GetStartIntens(); break;
        case MID_GRADIENT_ENDINTENSITY: rVal <<= GetGradientValue().GetEndIntens(); break;
        case MID_GRADIENT_STEPCOUNT: rVal <<= GetGradientValue().GetSteps(); break;

        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool XFillGradientItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue >   aPropSeq;
            ::com::sun::star::awt::Gradient         aGradient2;
            rtl::OUString                           aName;
            bool                                    bGradient( false );

            if ( rVal >>= aPropSeq )
            {
                for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
                {
                    if ( aPropSeq[n].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Name" )))
                        aPropSeq[n].Value >>= aName;
                    else if ( aPropSeq[n].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FillGradient" )))
                    {
                        if ( aPropSeq[n].Value >>= aGradient2 )
                            bGradient = true;
                    }
                }

                SetName( aName );
                if ( bGradient )
                {
                    XGradient aXGradient;

                    aXGradient.SetGradientStyle( (XGradientStyle) aGradient2.Style );
                    aXGradient.SetStartColor( aGradient2.StartColor );
                    aXGradient.SetEndColor( aGradient2.EndColor );
                    aXGradient.SetAngle( aGradient2.Angle );
                    aXGradient.SetBorder( aGradient2.Border );
                    aXGradient.SetXOffset( aGradient2.XOffset );
                    aXGradient.SetYOffset( aGradient2.YOffset );
                    aXGradient.SetStartIntens( aGradient2.StartIntensity );
                    aXGradient.SetEndIntens( aGradient2.EndIntensity );
                    aXGradient.SetSteps( aGradient2.StepCount );

                    SetGradientValue( aXGradient );
                }

                return sal_True;
            }

            return sal_False;
        }

        case MID_NAME:
        {
            rtl::OUString aName;
            if (!(rVal >>= aName ))
                return sal_False;
            SetName( aName );
            break;
        }

        case MID_FILLGRADIENT:
        {
            ::com::sun::star::awt::Gradient aGradient2;
            if(!(rVal >>= aGradient2))
                return sal_False;

            XGradient aXGradient;

            aXGradient.SetGradientStyle( (XGradientStyle) aGradient2.Style );
            aXGradient.SetStartColor( aGradient2.StartColor );
            aXGradient.SetEndColor( aGradient2.EndColor );
            aXGradient.SetAngle( aGradient2.Angle );
            aXGradient.SetBorder( aGradient2.Border );
            aXGradient.SetXOffset( aGradient2.XOffset );
            aXGradient.SetYOffset( aGradient2.YOffset );
            aXGradient.SetStartIntens( aGradient2.StartIntensity );
            aXGradient.SetEndIntens( aGradient2.EndIntensity );
            aXGradient.SetSteps( aGradient2.StepCount );

            SetGradientValue( aXGradient );
            break;
        }

        case MID_GRADIENT_STARTCOLOR:
        case MID_GRADIENT_ENDCOLOR:
        {
            sal_Int32 nVal = 0;
            if(!(rVal >>= nVal ))
                return sal_False;

            XGradient aXGradient = GetGradientValue();

            if ( nMemberId == MID_GRADIENT_STARTCOLOR )
                aXGradient.SetStartColor( nVal );
            else
                aXGradient.SetEndColor( nVal );
            SetGradientValue( aXGradient );
            break;
        }

        case MID_GRADIENT_STYLE:
        case MID_GRADIENT_ANGLE:
        case MID_GRADIENT_BORDER:
        case MID_GRADIENT_STARTINTENSITY:
        case MID_GRADIENT_ENDINTENSITY:
        case MID_GRADIENT_STEPCOUNT:
        case MID_GRADIENT_XOFFSET:
        case MID_GRADIENT_YOFFSET:
        {
            sal_Int16 nVal = sal_Int16();
            if(!(rVal >>= nVal ))
                return sal_False;

            XGradient aXGradient = GetGradientValue();

            switch ( nMemberId )
            {
                case MID_GRADIENT_STYLE:
                    aXGradient.SetGradientStyle( (XGradientStyle)nVal ); break;
                case MID_GRADIENT_ANGLE:
                    aXGradient.SetAngle( nVal ); break;
                case MID_GRADIENT_BORDER:
                    aXGradient.SetBorder( nVal ); break;
                case MID_GRADIENT_STARTINTENSITY:
                    aXGradient.SetStartIntens( nVal ); break;
                case MID_GRADIENT_ENDINTENSITY:
                    aXGradient.SetEndIntens( nVal ); break;
                case MID_GRADIENT_STEPCOUNT:
                    aXGradient.SetSteps( nVal ); break;
                case MID_GRADIENT_XOFFSET:
                    aXGradient.SetXOffset( nVal ); break;
                case MID_GRADIENT_YOFFSET:
                    aXGradient.SetYOffset( nVal ); break;
            }

            SetGradientValue( aXGradient );
            break;
        }
    }

    return sal_True;
}

sal_Bool XFillGradientItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return ((XFillGradientItem*)p1)->GetGradientValue() == ((XFillGradientItem*)p2)->GetGradientValue();
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
    bEnabled( sal_False )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//------------------------------------------------------------------------

XFillFloatTransparenceItem::XFillFloatTransparenceItem( sal_Int32 nIndex, const XGradient& rGradient, sal_Bool bEnable ) :
    XFillGradientItem   ( nIndex, rGradient ),
    bEnabled            ( bEnable )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//------------------------------------------------------------------------

XFillFloatTransparenceItem::XFillFloatTransparenceItem(const XubString& rName, const XGradient& rGradient, sal_Bool bEnable ) :
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

//XFillFloatTransparenceItem::XFillFloatTransparenceItem( SvStream& rIn, sal_uInt16 nVer ) :
//  XFillGradientItem   ( rIn, nVer )
//{
//  SetWhich( XATTR_FILLFLOATTRANSPARENCE );
//  rIn >> bEnabled;
//}

//*************************************************************************

XFillFloatTransparenceItem::XFillFloatTransparenceItem(SfxItemPool* /*pPool*/, const XGradient& rTheGradient, sal_Bool bEnable )
:   XFillGradientItem   ( -1, rTheGradient ),
    bEnabled            ( bEnable )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//*************************************************************************

XFillFloatTransparenceItem::XFillFloatTransparenceItem(SfxItemPool* /*pPool*/)
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

//------------------------------------------------------------------------

int XFillFloatTransparenceItem::operator==( const SfxPoolItem& rItem ) const
{
    return ( NameOrIndex::operator==(rItem) ) &&
            ( GetGradientValue() == ((const XFillGradientItem&)rItem).GetGradientValue() ) &&
             ( bEnabled == ( (XFillFloatTransparenceItem&) rItem ).bEnabled );
}

//------------------------------------------------------------------------

SfxPoolItem* XFillFloatTransparenceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillFloatTransparenceItem( *this );
}

//------------------------------------------------------------------------

//SfxPoolItem* XFillFloatTransparenceItem::Create( SvStream& rIn, sal_uInt16 nVer ) const
//{
//  return( ( 0 == nVer ) ? Clone( NULL ) : new XFillFloatTransparenceItem( rIn, nVer ) );
//}

//------------------------------------------------------------------------

//SvStream& XFillFloatTransparenceItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
//{
//  XFillGradientItem::Store( rOut, nItemVersion );
//  rOut << bEnabled;
//  return rOut;
//}

//------------------------------------------------------------------------

sal_uInt16 XFillFloatTransparenceItem::GetVersion( sal_uInt16 nFileFormatVersion ) const
{
    // !!! if version number of this object must be increased, please   !!!
    // !!! increase version number of base class XFillGradientItem      !!!
    return XFillGradientItem::GetVersion( nFileFormatVersion );
}

//------------------------------------------------------------------------

sal_Bool XFillFloatTransparenceItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    return XFillGradientItem::QueryValue( rVal, nMemberId );
}

//------------------------------------------------------------------------

sal_Bool XFillFloatTransparenceItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    return XFillGradientItem::PutValue( rVal, nMemberId );
}

//------------------------------------------------------------------------

SfxItemPresentation XFillFloatTransparenceItem::GetPresentation(    SfxItemPresentation ePres,
                                                                    SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
                                                                    XubString& rText,
                                                                    const IntlWrapper * pIntlWrapper ) const
{
    return XFillGradientItem::GetPresentation( ePres, eCoreUnit, ePresUnit, rText, pIntlWrapper );
}

sal_Bool XFillFloatTransparenceItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return  ((XFillFloatTransparenceItem*)p1)->IsEnabled() == ((XFillFloatTransparenceItem*)p2)->IsEnabled() &&
            ((XFillFloatTransparenceItem*)p1)->GetGradientValue()  == ((XFillFloatTransparenceItem*)p2)->GetGradientValue();
}

XFillFloatTransparenceItem* XFillFloatTransparenceItem::checkForUniqueItem( SdrModel* pModel ) const
{
    // #85953# unique name only necessary when enabled
    if(IsEnabled())
    {
        if( pModel )
        {
            const String aUniqueName = NameOrIndex::CheckNamedItem( this,
                                                                    XATTR_FILLFLOATTRANSPARENCE,
                                                                    &pModel->GetItemPool(),
                                                                    pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                                                                    XFillFloatTransparenceItem::CompareValueFunc,
                                                                    RID_SVXSTR_TRASNGR0,
                                                                    NULL );

            // if the given name is not valid, replace it!
            if( aUniqueName != GetName() )
            {
                return new XFillFloatTransparenceItem( aUniqueName, GetGradientValue(), sal_True );
            }
        }
    }
    else
    {
        // #85953# if disabled, force name to empty string
        if(GetName().Len())
        {
            return new XFillFloatTransparenceItem(String(), GetGradientValue(), sal_False);
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
    eStyle(eTheStyle),
    aColor(rCol),
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

bool XHatch::operator==(const XHatch& rHatch) const
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
|*    XFillHatchItem::XFillHatchItem(sal_Int32 nIndex,
|*                                 const XHatch& rTheHatch)
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

XFillHatchItem::XFillHatchItem(sal_Int32 nIndex,
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
        sal_uInt16 nRed;
        sal_uInt16 nGreen;
        sal_uInt16 nBlue;
        sal_Int16  nITemp;
        sal_Int32  nLTemp;

        rIn >> nITemp; aHatch.SetHatchStyle((XHatchStyle)nITemp);
        rIn >> nRed;
        rIn >> nGreen;
        rIn >> nBlue;

        Color aCol;
        aCol = Color( (sal_uInt8)( nRed >> 8 ), (sal_uInt8)( nGreen >> 8 ), (sal_uInt8)( nBlue >> 8 ) );
        aHatch.SetColor(aCol);
        rIn >> nLTemp; aHatch.SetDistance(nLTemp);
        rIn >> nLTemp; aHatch.SetAngle(nLTemp);
    }
}

//*************************************************************************

XFillHatchItem::XFillHatchItem(SfxItemPool* /*pPool*/, const XHatch& rTheHatch)
:   NameOrIndex( XATTR_FILLHATCH, -1 ),
    aHatch(rTheHatch)
{
}

//*************************************************************************

XFillHatchItem::XFillHatchItem(SfxItemPool* /*pPool*/)
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

SfxPoolItem* XFillHatchItem::Clone(SfxItemPool* /*pPool*/) const
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
|*    SfxPoolItem* XFillHatchItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  15.11.94
|*
*************************************************************************/

SfxPoolItem* XFillHatchItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SvStream& XFillHatchItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut << (sal_Int16)aHatch.GetHatchStyle();

        sal_uInt16 nTmp;
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetRed() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetGreen() ); rOut << nTmp;
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetBlue() ); rOut << nTmp;

        rOut << (sal_Int32) aHatch.GetDistance();
        rOut << (sal_Int32) aHatch.GetAngle();
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

const XHatch& XFillHatchItem::GetHatchValue(const XHatchTable* pTable) const // GetValue -> GetHatchValue
{
    if (!IsIndex())
        return aHatch;
    else
        return pTable->GetHatch(GetIndex())->GetHatch();
}

//------------------------------------------------------------------------

SfxItemPresentation XFillHatchItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
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
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

//------------------------------------------------------------------------

FASTBOOL XFillHatchItem::HasMetrics() const
{
    return sal_True;
}

//------------------------------------------------------------------------

FASTBOOL XFillHatchItem::ScaleMetrics(long nMul, long nDiv)
{
    aHatch.SetDistance( ScaleMetricValue( aHatch.GetDistance(), nMul, nDiv ) );
    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool XFillHatchItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue > aPropSeq( 2 );

            ::com::sun::star::drawing::Hatch aUnoHatch;

            aUnoHatch.Style = (::com::sun::star::drawing::HatchStyle)aHatch.GetHatchStyle();
            aUnoHatch.Color = aHatch.GetColor().GetColor();
            aUnoHatch.Distance = aHatch.GetDistance();
            aUnoHatch.Angle = aHatch.GetAngle();

            rtl::OUString aApiName;
            SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
            aPropSeq[0].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ));
            aPropSeq[0].Value   = uno::makeAny( aApiName );
            aPropSeq[1].Name    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillHatch" ));
            aPropSeq[1].Value   = uno::makeAny( aUnoHatch );
            rVal = uno::makeAny( aPropSeq );
            break;
        }

        case MID_FILLHATCH:
        {
            ::com::sun::star::drawing::Hatch aUnoHatch;

            aUnoHatch.Style = (::com::sun::star::drawing::HatchStyle)aHatch.GetHatchStyle();
            aUnoHatch.Color = aHatch.GetColor().GetColor();
            aUnoHatch.Distance = aHatch.GetDistance();
            aUnoHatch.Angle = aHatch.GetAngle();
            rVal <<= aUnoHatch;
            break;
        }

        case MID_NAME:
        {
            rtl::OUString aApiName;
            SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
            rVal <<= aApiName;
            break;
        }

        case MID_HATCH_STYLE:
            rVal <<= (::com::sun::star::drawing::HatchStyle)aHatch.GetHatchStyle(); break;
        case MID_HATCH_COLOR:
            rVal <<= (sal_Int32)aHatch.GetColor().GetColor(); break;
        case MID_HATCH_DISTANCE:
            rVal <<= aHatch.GetDistance(); break;
        case MID_HATCH_ANGLE:
            rVal <<= aHatch.GetAngle(); break;

        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool XFillHatchItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue >   aPropSeq;
            ::com::sun::star::drawing::Hatch        aUnoHatch;
            rtl::OUString                           aName;
            bool                                    bHatch( false );

            if ( rVal >>= aPropSeq )
            {
                for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
                {
                    if ( aPropSeq[n].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Name" )))
                        aPropSeq[n].Value >>= aName;
                    else if ( aPropSeq[n].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FillHatch" )))
                    {
                        if ( aPropSeq[n].Value >>= aUnoHatch )
                            bHatch = true;
                    }
                }

                SetName( aName );
                if ( bHatch )
                {
                    aHatch.SetHatchStyle( (XHatchStyle)aUnoHatch.Style );
                    aHatch.SetColor( aUnoHatch.Color );
                    aHatch.SetDistance( aUnoHatch.Distance );
                    aHatch.SetAngle( aUnoHatch.Angle );
                }

                return sal_True;
            }

            return sal_False;
        }

        case MID_FILLHATCH:
        {
            ::com::sun::star::drawing::Hatch aUnoHatch;
            if(!(rVal >>= aUnoHatch))
                return sal_False;

            aHatch.SetHatchStyle( (XHatchStyle)aUnoHatch.Style );
            aHatch.SetColor( aUnoHatch.Color );
            aHatch.SetDistance( aUnoHatch.Distance );
            aHatch.SetAngle( aUnoHatch.Angle );
            break;
        }

        case MID_NAME:
        {
            rtl::OUString aName;
            if (!(rVal >>= aName ))
                return sal_False;
            SetName( aName );
            break;
        }

        case MID_HATCH_STYLE:
        {
            sal_Int16 nVal = sal_Int16();
            if (!(rVal >>= nVal ))
                return sal_False;
            aHatch.SetHatchStyle( (XHatchStyle)nVal );
            break;
        }

        case MID_HATCH_COLOR:
        case MID_HATCH_DISTANCE:
        case MID_HATCH_ANGLE:
        {
            sal_Int32 nVal = 0;
            if (!(rVal >>= nVal ))
                return sal_False;

            if ( nMemberId == MID_HATCH_COLOR )
                aHatch.SetColor( nVal );
            else if ( nMemberId == MID_HATCH_DISTANCE )
                aHatch.SetDistance( nVal );
            else
                aHatch.SetAngle( nVal );
            break;
        }

        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
}

sal_Bool XFillHatchItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return ((XFillHatchItem*)p1)->GetHatchValue() == ((XFillHatchItem*)p2)->GetHatchValue();
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
    SfxEnumItem(XATTR_FORMTXTSTYLE, sal::static_int_cast< sal_uInt16 >(eTheStyle))
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

SfxPoolItem* XFormTextStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextStyleItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextStyleItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextStyleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextStyleItem(rIn);
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

sal_uInt16 XFormTextStyleItem::GetValueCount() const
{
    return 5;
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
sal_Bool XFormTextStyleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
sal_Bool XFormTextStyleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(sal::static_int_cast< sal_uInt16 >(nValue));

    return sal_True;
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
    SfxEnumItem(XATTR_FORMTXTADJUST, sal::static_int_cast< sal_uInt16 >(eTheAdjust))
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

SfxPoolItem* XFormTextAdjustItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextAdjustItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextAdjustItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextAdjustItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextAdjustItem(rIn);
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

sal_uInt16 XFormTextAdjustItem::GetValueCount() const
{
    return 4;
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
sal_Bool XFormTextAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
sal_Bool XFormTextAdjustItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(sal::static_int_cast< sal_uInt16 >(nValue));

    return sal_True;
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

SfxPoolItem* XFormTextDistanceItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextDistanceItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextDistanceItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextDistanceItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SfxPoolItem* XFormTextStartItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextStartItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextStartItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    02.02.95 ESO
|*    Letzte Aenderung  02.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextStartItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextStartItem(rIn);
}

// -------------------------
// class XFormTextMirrorItem
// -------------------------
TYPEINIT1_AUTOFACTORY(XFormTextMirrorItem, SfxBoolItem);

/*************************************************************************
|*
|*    XFormTextMirrorItem::XFormTextMirrorItem(sal_Bool bMirror)
|*
|*    Ersterstellung    06.02.95 ESO
|*    Letzte Aenderung  06.02.95 ESO
|*
*************************************************************************/

XFormTextMirrorItem::XFormTextMirrorItem(sal_Bool bMirror) :
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

SfxPoolItem* XFormTextMirrorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextMirrorItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextMirrorItem::Create(SvStream& rIn, sal_uInt16 nVer)
|*                                              const
|*
|*    Ersterstellung    06.02.95 ESO
|*    Letzte Aenderung  06.02.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextMirrorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

XFormTextOutlineItem::XFormTextOutlineItem(sal_Bool bOutline) :
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

SfxPoolItem* XFormTextOutlineItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextOutlineItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextOutlineItem::Create(SvStream& rIn, sal_uInt16 nVer)
|*                                              const
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextOutlineItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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
    SfxEnumItem(
        XATTR_FORMTXTSHADOW, sal::static_int_cast< sal_uInt16 >(eFormTextShadow))
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

SfxPoolItem* XFormTextShadowItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextShadowItem(rIn);
}


/*************************************************************************
|*
|*
|*
\*************************************************************************/

sal_uInt16 XFormTextShadowItem::GetValueCount() const
{
    return 3;
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
sal_Bool XFormTextShadowItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
sal_Bool XFormTextShadowItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(sal::static_int_cast< sal_uInt16 >(nValue));

    return sal_True;
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

XFormTextShadowColorItem::XFormTextShadowColorItem(sal_Int32 nIndex,
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

SfxPoolItem* XFormTextShadowColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowColorItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowColorItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowColorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SfxPoolItem* XFormTextShadowXValItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowXValItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowXValItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowXValItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SfxPoolItem* XFormTextShadowYValItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowYValItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowYValItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    28.06.95 ESO
|*    Letzte Aenderung  28.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowYValItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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
    SfxEnumItem(
        XATTR_FORMTXTSTDFORM, sal::static_int_cast< sal_uInt16 >(eFormTextStdForm))
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

SfxPoolItem* XFormTextStdFormItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextStdFormItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextStdFormItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    27.06.95
|*    Letzte Aenderung  27.06.95
|*
*************************************************************************/

SfxPoolItem* XFormTextStdFormItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextStdFormItem(rIn);
}


/*************************************************************************
|*
|*
|*
\*************************************************************************/

sal_uInt16 XFormTextStdFormItem::GetValueCount() const
{
    return 3;
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
sal_Bool XFormTextStdFormItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
sal_Bool XFormTextStdFormItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(sal::static_int_cast< sal_uInt16 >(nValue));

    return sal_True;
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

XFormTextHideFormItem::XFormTextHideFormItem(sal_Bool bHide) :
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

SfxPoolItem* XFormTextHideFormItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextHideFormItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextHideFormItem::Create(SvStream& rIn, sal_uInt16 nVer)
|*                                              const
|*
|*    Ersterstellung    27.06.95 ESO
|*    Letzte Aenderung  27.06.95 ESO
|*
*************************************************************************/

SfxPoolItem* XFormTextHideFormItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
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

SfxPoolItem* XLineAttrSetItem::Create( SvStream& rStream, sal_uInt16 /*nVersion*/) const
{
    SfxItemSet *pSet2 = new SfxItemSet( *GetItemSet().GetPool(),
                                    XATTR_LINE_FIRST, XATTR_LINE_LAST);
    pSet2->Load( rStream );
    return new XLineAttrSetItem( pSet2 );
}

/*************************************************************************
|*
|* SetItem in Stream speichern
|*
\************************************************************************/

SvStream& XLineAttrSetItem::Store( SvStream& rStream, sal_uInt16 nItemVersion ) const
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

SfxPoolItem* XFillAttrSetItem::Create( SvStream& rStream, sal_uInt16 /*nVersion*/) const
{
    SfxItemSet *pSet2 = new SfxItemSet( *GetItemSet().GetPool(),
                                    XATTR_FILL_FIRST, XATTR_FILL_LAST);
    pSet2->Load( rStream );
    return new XFillAttrSetItem( pSet2 );
}

/*************************************************************************
|*
|* SetItem in Stream speichern
|*
\************************************************************************/

SvStream& XFillAttrSetItem::Store( SvStream& rStream, sal_uInt16 nItemVersion ) const
{
    return SfxSetItem::Store( rStream, nItemVersion );
}

// eof

