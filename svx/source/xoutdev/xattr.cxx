/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <tools/mapunit.hxx>

#include "svx/unoapi.hxx"
#include <svl/style.hxx>

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
#include <basegfx/tools/unotools.hxx>
#include <vcl/gradient.hxx>

#include <stdio.h>
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

#define VCLTOSVCOL( rCol ) (sal_uInt16)((((sal_uInt16)(rCol))<<8)|(rCol))

TYPEINIT1_AUTOFACTORY(NameOrIndex, SfxStringItem);

//SfxPoolItem* NameOrIndex::CreateDefault() { return new NameOrIndex; }

long ScaleMetricValue( long nVal, long nMul, long nDiv )
{
    BigInt aVal( nVal );

    aVal *= nMul;

    if ( aVal.IsNeg() != ( nDiv < 0 ) )
        aVal-=nDiv/2; // for correct rounding
    else
        aVal+=nDiv/2; // for correct rounding

    aVal/=nDiv;

    return long( aVal );
}

NameOrIndex::NameOrIndex(sal_uInt16 _nWhich, sal_Int32 nIndex) :
    SfxStringItem(_nWhich, OUString()),
    nPalIndex(nIndex)
{
}

NameOrIndex::NameOrIndex(sal_uInt16 _nWhich, const OUString& rName) :
    SfxStringItem(_nWhich, rName),
    nPalIndex(-1)
{
}

NameOrIndex::NameOrIndex(sal_uInt16 _nWhich, SvStream& rIn)
    : SfxStringItem(_nWhich, rIn)
    , nPalIndex(-1)
{
    rIn.ReadInt32( nPalIndex );
}

NameOrIndex::NameOrIndex(const NameOrIndex& rNameOrIndex) :
    SfxStringItem(rNameOrIndex),
    nPalIndex(rNameOrIndex.nPalIndex)
{
}

bool NameOrIndex::operator==(const SfxPoolItem& rItem) const
{
    return ( SfxStringItem::operator==(rItem) &&
            static_cast<const NameOrIndex&>(rItem).nPalIndex == nPalIndex );
}

SfxPoolItem* NameOrIndex::Clone(SfxItemPool* /*pPool*/) const
{

    return new NameOrIndex(*this);
}

SfxPoolItem* NameOrIndex::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new NameOrIndex(Which(), rIn);
}

SvStream& NameOrIndex::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    SfxStringItem::Store( rOut, nItemVersion );
    rOut.WriteInt32( nPalIndex );
    return rOut;
}

/** this static checks if the given NameOrIndex item has a unique name for its value.
    The returned String is a unique name for an item with this value in both given pools.
    Argument pPool2 can be null.
    If returned string equals NameOrIndex->GetName(), the name was already unique.
*/
OUString NameOrIndex::CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, const SfxItemPool* /*pPool2*/, SvxCompareValueFunc pCompareValueFunc, sal_uInt16 nPrefixResId, const XPropertyListRef &pDefaults )
{
    bool bForceNew = false;

    OUString aUniqueName = SvxUnogetInternalNameForItem(nWhich, pCheckItem->GetName());

    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different line end or start

    if (!aUniqueName.isEmpty() && pPool1)
    {
        const sal_uInt32 nCount = pPool1->GetItemCount2( nWhich );

        for( sal_uInt32 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            const NameOrIndex *pItem = static_cast<const NameOrIndex*>(pPool1->GetItem2( nWhich, nSurrogate ));

            if( pItem && ( pItem->GetName() == pCheckItem->GetName() ) )
            {
                // if there is already an item with the same name and the same
                // value its ok to set it
                if( !pCompareValueFunc( pItem, pCheckItem ) )
                {
                    // same name but different value, we need a new name for this item
                    aUniqueName.clear();
                    bForceNew = true;
                }
                break;
            }
        }
    }

    // if we have no name yet, find existing item with same content or
    // create a unique name
    if (aUniqueName.isEmpty())
    {
        sal_Int32 nUserIndex = 1;
        const ResId aRes(SVX_RES(nPrefixResId));
        OUString aUser( aRes.toString() );
        aUser += " ";

        if( pDefaults.get() )
        {
            const int nCount = pDefaults->Count();
            int nIndex;
            for( nIndex = 0; nIndex < nCount; nIndex++ )
            {
                XPropertyEntry* pEntry = pDefaults->Get(nIndex);
                if( pEntry )
                {
                    bool bFound = false;

                    switch( nWhich )
                    {
                    case XATTR_FILLBITMAP:
                    {
                        const GraphicObject& rGraphicObjectA(static_cast<const XFillBitmapItem*>(pCheckItem)->GetGraphicObject());
                        const GraphicObject& rGraphicObjectB(static_cast<XBitmapEntry*>(pEntry)->GetGraphicObject());

                        bFound = (rGraphicObjectA == rGraphicObjectB);
                        break;
                    }
                    case XATTR_LINEDASH:
                        bFound = static_cast<const XLineDashItem*>(pCheckItem)->GetDashValue() == static_cast<XDashEntry*>(pEntry)->GetDash();
                        break;
                    case XATTR_LINESTART:
                        bFound = static_cast<const XLineStartItem*>(pCheckItem)->GetLineStartValue() == static_cast<XLineEndEntry*>(pEntry)->GetLineEnd();
                        break;
                    case XATTR_LINEEND:
                        bFound = static_cast<const XLineEndItem*>(pCheckItem)->GetLineEndValue() == static_cast<XLineEndEntry*>(pEntry)->GetLineEnd();
                        break;
                    case XATTR_FILLGRADIENT:
                        bFound = static_cast<const XFillGradientItem*>(pCheckItem)->GetGradientValue() == static_cast<XGradientEntry*>(pEntry)->GetGradient();
                        break;
                    case XATTR_FILLHATCH:
                        bFound = static_cast<const XFillHatchItem*>(pCheckItem)->GetHatchValue() == static_cast<XHatchEntry*>(pEntry)->GetHatch();
                        break;
                    }

                    if( bFound )
                    {
                        aUniqueName = pEntry->GetName();
                        break;
                    }
                    else
                    {
                        OUString aEntryName = pEntry->GetName();
                        if(aEntryName.getLength() >= aUser.getLength())
                        {
                            sal_Int32 nThisIndex = aEntryName.copy( aUser.getLength() ).toInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }
            }
        }

        if (aUniqueName.isEmpty() && pPool1)
        {
            const sal_uInt32 nCount = pPool1->GetItemCount2( nWhich );
            const NameOrIndex *pItem;
            for( sal_uInt32 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = static_cast<const NameOrIndex*>(pPool1->GetItem2( nWhich, nSurrogate ));

                if( pItem && !pItem->GetName().isEmpty() )
                {
                    if( !bForceNew && pCompareValueFunc( pItem, pCheckItem ) )
                        return pItem->GetName();

                    if( pItem->GetName().startsWith( aUser ) )
                    {
                        sal_Int32 nThisIndex = pItem->GetName().copy( aUser.getLength() ).toInt32();
                        if( nThisIndex >= nUserIndex )
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }
            aUniqueName = aUser;
            aUniqueName += OUString::number( nUserIndex );
        }
    }

    return aUniqueName;
}

void NameOrIndex::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("nameOrIndex"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("isIndex"), BAD_CAST(OString::boolean(IsIndex()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(GetIndex()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

TYPEINIT1_AUTOFACTORY(XColorItem, NameOrIndex);

//SfxPoolItem* XColorItem::CreateDefault() { return new XColorItem; }

XColorItem::XColorItem(sal_uInt16 _nWhich, sal_Int32 nIndex, const Color& rTheColor) :
    NameOrIndex(_nWhich, nIndex),
    aColor(rTheColor)
{
}

XColorItem::XColorItem(sal_uInt16 _nWhich, const OUString& rName, const Color& rTheColor) :
    NameOrIndex(_nWhich, rName),
    aColor(rTheColor)
{
}

XColorItem::XColorItem(sal_uInt16 _nWhich, const Color& rTheColor)
    : NameOrIndex(_nWhich, OUString())
    , aColor(rTheColor)
{
}

XColorItem::XColorItem(const XColorItem& rItem) :
    NameOrIndex(rItem),
    aColor(rItem.aColor)
{
}

XColorItem::XColorItem(sal_uInt16 _nWhich, SvStream& rIn) :
    NameOrIndex(_nWhich, rIn)
{
    if (!IsIndex())
    {
        ReadColor( rIn, aColor );
    }
}

SfxPoolItem* XColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XColorItem(*this);
}

bool XColorItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
            static_cast<const XColorItem&>(rItem).aColor == aColor );
}

SfxPoolItem* XColorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XColorItem(Which(), rIn);
}

SvStream& XColorItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if ( !IsIndex() )
    {
        WriteColor( rOut, aColor );
    }

    return rOut;
}

const Color& XColorItem::GetColorValue(const XColorList* pTable) const
{
    if (!IsIndex())
        return aColor;
    else
        return pTable->GetColor(GetIndex())->GetColor();

}

bool XColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetColorValue().GetRGBColor();
    return true;
}

bool XColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetColorValue( nValue );

    return true;
}

// --- line attributes ---

TYPEINIT1_AUTOFACTORY(XLineStyleItem, SfxEnumItem);

SfxPoolItem* XLineStyleItem::CreateDefault() { return new XLineStyleItem; }

XLineStyleItem::XLineStyleItem(css::drawing::LineStyle eTheLineStyle) :
    SfxEnumItem(XATTR_LINESTYLE, sal::static_int_cast< sal_uInt16 >(eTheLineStyle))
{
}

XLineStyleItem::XLineStyleItem(SvStream& rIn) :
    SfxEnumItem(XATTR_LINESTYLE, rIn)
{
}

SfxPoolItem* XLineStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStyleItem( *this );
}

SfxPoolItem* XLineStyleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineStyleItem(rIn);
}

bool XLineStyleItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();

    sal_uInt16 nId = 0;

    switch( (sal_uInt16)GetValue() )
    {
        case css::drawing::LineStyle_NONE:
            nId = RID_SVXSTR_INVISIBLE;
            break;
        case css::drawing::LineStyle_SOLID:
            nId = RID_SVXSTR_SOLID;
            break;
    }

    if ( nId )
        rText = SVX_RESSTR( nId );
    return true;
}

bool XLineStyleItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    css::drawing::LineStyle eLS = (css::drawing::LineStyle)GetValue();
    rVal <<= eLS;
    return true;
}

bool XLineStyleItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    css::drawing::LineStyle eLS;
    if(!(rVal >>= eLS ))
    {
        // also try an int (for Basic)
        sal_Int32 nLS = 0;
        if(!(rVal >>= nLS))
            return false;
        eLS = (css::drawing::LineStyle)nLS;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( eLS ) );
    return true;
}

sal_uInt16 XLineStyleItem::GetValueCount() const
{
    return 3;
}

XDash::XDash(css::drawing::DashStyle eTheDash, sal_uInt16 nTheDots, sal_uIntPtr nTheDotLen,
             sal_uInt16 nTheDashes, sal_uIntPtr nTheDashLen, sal_uIntPtr nTheDistance) :
    eDash(eTheDash),
    nDots(nTheDots),
    nDotLen(nTheDotLen),
    nDashes(nTheDashes),
    nDashLen(nTheDashLen),
    nDistance(nTheDistance)
{
}

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

    if(GetDashStyle() == css::drawing::DashStyle_RECTRELATIVE || GetDashStyle() == css::drawing::DashStyle_ROUNDRELATIVE)
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

TYPEINIT1_AUTOFACTORY(XLineDashItem, NameOrIndex);

//SfxPoolItem* XLineDashItem::CreateDefault() { return new XLineDashItem; }

XLineDashItem::XLineDashItem(const OUString& rName, const XDash& rTheDash) :
    NameOrIndex(XATTR_LINEDASH, rName),
    aDash(rTheDash)
{
}

XLineDashItem::XLineDashItem(const XLineDashItem& rItem) :
    NameOrIndex(rItem),
    aDash(rItem.aDash)
{
}

XLineDashItem::XLineDashItem(SvStream& rIn) :
    NameOrIndex(XATTR_LINEDASH, rIn)
{
    if (!IsIndex())
    {
        sal_uInt16  nSTemp;
        sal_uInt32  nLTemp;
        sal_Int32   nITemp;

        rIn.ReadInt32( nITemp ); aDash.SetDashStyle((css::drawing::DashStyle)nITemp);
        rIn.ReadUInt16( nSTemp ); aDash.SetDots(nSTemp);
        rIn.ReadUInt32( nLTemp ); aDash.SetDotLen(nLTemp);
        rIn.ReadUInt16( nSTemp ); aDash.SetDashes(nSTemp);
        rIn.ReadUInt32( nLTemp ); aDash.SetDashLen(nLTemp);
        rIn.ReadUInt32( nLTemp ); aDash.SetDistance(nLTemp);
    }
}

XLineDashItem::XLineDashItem(SfxItemPool* /*pPool*/, const XDash& rTheDash)
:   NameOrIndex( XATTR_LINEDASH, -1 ),
    aDash(rTheDash)
{
}

SfxPoolItem* XLineDashItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineDashItem(*this);
}

bool XLineDashItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             aDash == static_cast<const XLineDashItem&>(rItem).aDash );
}

SfxPoolItem* XLineDashItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineDashItem(rIn);
}

SvStream& XLineDashItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut.WriteInt32( aDash.GetDashStyle() );
        rOut.WriteUInt16( aDash.GetDots() );
        rOut.WriteUInt32( aDash.GetDotLen() );
        rOut.WriteUInt16( aDash.GetDashes() );
        rOut.WriteUInt32( aDash.GetDashLen() );
        rOut.WriteUInt32( aDash.GetDistance() );
    }

    return rOut;
}


bool XLineDashItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetName();
    return true;
}

bool XLineDashItem::HasMetrics() const
{
    return true;
}

bool XLineDashItem::ScaleMetrics(long nMul, long nDiv)
{
    aDash.SetDotLen( ScaleMetricValue( aDash.GetDotLen(), nMul, nDiv ) );
    aDash.SetDashLen( ScaleMetricValue( aDash.GetDashLen(), nMul, nDiv ) );
    aDash.SetDistance( ScaleMetricValue( aDash.GetDistance(), nMul, nDiv ) );
    return true;
}

bool XLineDashItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue > aPropSeq( 2 );

            css::drawing::LineDash aLineDash;

            const XDash& rXD = GetDashValue();
            aLineDash.Style = (css::drawing::DashStyle)((sal_uInt16)rXD.GetDashStyle());
            aLineDash.Dots = rXD.GetDots();
            aLineDash.DotLen = rXD.GetDotLen();
            aLineDash.Dashes = rXD.GetDashes();
            aLineDash.DashLen = rXD.GetDashLen();
            aLineDash.Distance = rXD.GetDistance();

            OUString aApiName = SvxUnogetApiNameForItem(Which(), GetName());
            aPropSeq[0].Name    = "Name";
            aPropSeq[0].Value   = uno::makeAny( aApiName );
            aPropSeq[1].Name    = "LineDash";
            aPropSeq[1].Value   = uno::makeAny( aLineDash );
            rVal = uno::makeAny( aPropSeq );
            break;
        }

        case MID_NAME:
        {
            OUString aApiName = SvxUnogetApiNameForItem(Which(), GetName());
            rVal <<= aApiName;
            break;
        }

        case MID_LINEDASH:
        {
            const XDash& rXD = GetDashValue();

            css::drawing::LineDash aLineDash;

            aLineDash.Style = (css::drawing::DashStyle)((sal_uInt16)rXD.GetDashStyle());
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
            rVal <<= (css::drawing::DashStyle)((sal_Int16)rXD.GetDashStyle());
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

        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}

bool XLineDashItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue >   aPropSeq;

            if ( rVal >>= aPropSeq )
            {
                css::drawing::LineDash aLineDash;
                OUString aName;
                bool bLineDash( false );
                for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
                {
                    if ( aPropSeq[n].Name == "Name" )
                        aPropSeq[n].Value >>= aName;
                    else if ( aPropSeq[n].Name == "LineDash" )
                    {
                        if ( aPropSeq[n].Value >>= aLineDash )
                            bLineDash = true;
                    }
                }

                SetName( aName );
                if ( bLineDash )
                {
                    XDash aXDash;

                    aXDash.SetDashStyle((css::drawing::DashStyle)((sal_uInt16)(aLineDash.Style)));
                    aXDash.SetDots(aLineDash.Dots);
                    aXDash.SetDotLen(aLineDash.DotLen);
                    aXDash.SetDashes(aLineDash.Dashes);
                    aXDash.SetDashLen(aLineDash.DashLen);
                    aXDash.SetDistance(aLineDash.Distance);

                    if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
                        aXDash.SetDots(1);

                    SetDashValue( aXDash );
                }

                return true;
            }

            return false;
        }

        case MID_NAME:
        {
            OUString aName;
            if (!(rVal >>= aName))
                return false;
            SetName( aName );
            break;
        }

        case MID_LINEDASH:
        {
            css::drawing::LineDash aLineDash;
            if(!(rVal >>= aLineDash))
                return false;

            XDash aXDash;

            aXDash.SetDashStyle((css::drawing::DashStyle)((sal_uInt16)(aLineDash.Style)));
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
                return false;

            XDash aXDash = GetDashValue();
            aXDash.SetDashStyle((css::drawing::DashStyle)((sal_uInt16)(nVal)));

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
                return false;

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
                return false;

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

    return true;
}

bool XLineDashItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return static_cast<const XLineDashItem*>(p1)->GetDashValue() == static_cast<const XLineDashItem*>(p2)->GetDashValue();
}

XLineDashItem* XLineDashItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const OUString aUniqueName = NameOrIndex::CheckNamedItem(
                this, XATTR_LINEDASH, &pModel->GetItemPool(),
                pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                XLineDashItem::CompareValueFunc, RID_SVXSTR_DASH11,
                pModel->GetPropertyList( XDASH_LIST ) );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
            return new XLineDashItem( aUniqueName, aDash );
    }

    return NULL;
}

TYPEINIT1_AUTOFACTORY(XLineWidthItem, SfxMetricItem);

//SfxPoolItem* XLineWidthItem::CreateDefault() { return new XLineWidthItem; }

XLineWidthItem::XLineWidthItem(long nWidth) :
    SfxMetricItem(XATTR_LINEWIDTH, nWidth)
{
}

XLineWidthItem::XLineWidthItem(SvStream& rIn) :
    SfxMetricItem(XATTR_LINEWIDTH, rIn)
{
}

SfxPoolItem* XLineWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineWidthItem(*this);
}

SfxPoolItem* XLineWidthItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineWidthItem(rIn);
}

bool XLineWidthItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper * pIntl
)   const
{
    rText = GetMetricText( (long) GetValue(),
                            eCoreUnit, ePresUnit, pIntl) +
            " " + EE_RESSTR( GetMetricId( ePresUnit) );
    return true;
}

bool XLineWidthItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Int32 nValue = GetValue();
    if( 0 != (nMemberId&CONVERT_TWIPS) )
        nValue = convertTwipToMm100(nValue);

    rVal <<= nValue;
    return true;
}

bool XLineWidthItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    if( 0 != (nMemberId&CONVERT_TWIPS) )
        nValue = convertMm100ToTwip(nValue);

    SetValue( nValue );
    return true;
}

TYPEINIT1_AUTOFACTORY(XLineColorItem, XColorItem);

//SfxPoolItem* XLineColorItem::CreateDefault() { return new XLineColorItem; }

XLineColorItem::XLineColorItem(sal_Int32 nIndex, const Color& rTheColor) :
    XColorItem(XATTR_LINECOLOR, nIndex, rTheColor)
{
}

XLineColorItem::XLineColorItem(const OUString& rName, const Color& rTheColor) :
    XColorItem(XATTR_LINECOLOR, rName, rTheColor)
{
}

XLineColorItem::XLineColorItem(SvStream& rIn) :
    XColorItem(XATTR_LINECOLOR, rIn)
{
}

SfxPoolItem* XLineColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineColorItem(*this);
}

SfxPoolItem* XLineColorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineColorItem(rIn);
}

bool XLineColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetName();
    return true;
}

bool XLineColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetColorValue().GetRGBColor();
    return true;
}

bool XLineColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetColorValue( nValue );
    return true;
}

// --- tooling for simple spooling B2DPolygon to file and back ---

namespace
{
    void streamOutB2DPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, SvStream& rOut)
    {
        const sal_uInt32 nPolygonCount(rPolyPolygon.count());
        rOut.WriteUInt32( nPolygonCount );

        for(sal_uInt32 a(0L); a < nPolygonCount; a++)
        {
            const basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(a));
            const sal_uInt32 nPointCount(aCandidate.count());
            const sal_uInt8 bClosed(aCandidate.isClosed() ? 1 : 0);
            const sal_uInt8 bControlPoints(aCandidate.areControlPointsUsed() ? 1 : 0);
            rOut.WriteUInt32( nPointCount );
            rOut.WriteUChar( bClosed );
            rOut.WriteUChar( bControlPoints );

            for(sal_uInt32 b(0L); b < nPointCount; b++)
            {
                const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(b));
                rOut.WriteDouble( aPoint.getX() );
                rOut.WriteDouble( aPoint.getY() );

                if(bControlPoints)
                {
                    const sal_uInt8 bEdgeIsCurve(aCandidate.isPrevControlPointUsed(b) || aCandidate.isNextControlPointUsed(b) ? 1 : 0);
                    rOut.WriteUChar( bEdgeIsCurve );

                    if(bEdgeIsCurve)
                    {
                        const basegfx::B2DVector aControlVectorA(aCandidate.getPrevControlPoint(b));
                        rOut.WriteDouble( aControlVectorA.getX() );
                        rOut.WriteDouble( aControlVectorA.getY() );

                        const basegfx::B2DVector aControlVectorB(aCandidate.getNextControlPoint(b));
                        rOut.WriteDouble( aControlVectorB.getX() );
                        rOut.WriteDouble( aControlVectorB.getY() );
                    }
                }
            }
        }
    }

    basegfx::B2DPolyPolygon streamInB2DPolyPolygon(SvStream& rIn)
    {
        basegfx::B2DPolyPolygon aRetval;
        sal_uInt32 nPolygonCount;
        rIn.ReadUInt32( nPolygonCount );

        for(sal_uInt32 a(0L); a < nPolygonCount; a++)
        {
            sal_uInt32 nPointCount;
            sal_uInt8 bClosed;
            sal_uInt8 bControlPoints;

            rIn.ReadUInt32( nPointCount );
            rIn.ReadUChar( bClosed );
            rIn.ReadUChar( bControlPoints );

            basegfx::B2DPolygon aCandidate;
            aCandidate.setClosed(0 != bClosed);

            for(sal_uInt32 b(0L); b < nPointCount; b++)
            {
                double fX, fY;
                rIn.ReadDouble( fX );
                rIn.ReadDouble( fY );
                aCandidate.append(basegfx::B2DPoint(fX, fY));

                if(0 != bControlPoints)
                {
                    sal_uInt8 bEdgeIsCurve;
                    rIn.ReadUChar( bEdgeIsCurve );

                    if(0 != bEdgeIsCurve)
                    {
                        rIn.ReadDouble( fX );
                        rIn.ReadDouble( fY );
                        aCandidate.setPrevControlPoint(b, basegfx::B2DVector(fX, fY));

                        rIn.ReadDouble( fX );
                        rIn.ReadDouble( fY );
                        aCandidate.setNextControlPoint(b, basegfx::B2DVector(fX, fY));
                    }
                }
            }

            aRetval.append(aCandidate);
        }

        return aRetval;
    }
}

TYPEINIT1_AUTOFACTORY(XLineStartItem, NameOrIndex);

//SfxPoolItem* XLineStartItem::CreateDefault() { return new XLineStartItem; }

XLineStartItem::XLineStartItem(sal_Int32 nIndex)
:   NameOrIndex(XATTR_LINESTART, nIndex)
{
}

XLineStartItem::XLineStartItem(const OUString& rName, const basegfx::B2DPolyPolygon& rPolyPolygon)
:   NameOrIndex(XATTR_LINESTART, rName),
    maPolyPolygon(rPolyPolygon)
{
}

XLineStartItem::XLineStartItem(const XLineStartItem& rItem)
:   NameOrIndex(rItem),
    maPolyPolygon(rItem.maPolyPolygon)
{
}

XLineStartItem::XLineStartItem(SvStream& rIn) :
    NameOrIndex(XATTR_LINESTART, rIn)
{
    if (!IsIndex())
    {
        maPolyPolygon = streamInB2DPolyPolygon(rIn);
    }
}

XLineStartItem::XLineStartItem(SfxItemPool* /*pPool*/, const basegfx::B2DPolyPolygon& rPolyPolygon)
:   NameOrIndex( XATTR_LINESTART, -1 ),
    maPolyPolygon(rPolyPolygon)
{
}

SfxPoolItem* XLineStartItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStartItem(*this);
}

bool XLineStartItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) && static_cast<const XLineStartItem&>(rItem).maPolyPolygon == maPolyPolygon );
}

SfxPoolItem* XLineStartItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineStartItem(rIn);
}

SvStream& XLineStartItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        streamOutB2DPolyPolygon(maPolyPolygon, rOut);
    }

    return rOut;
}


bool XLineStartItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetName();
    return true;
}

bool XLineStartItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    if( nMemberId == MID_NAME )
    {
        OUString aApiName = SvxUnogetApiNameForItem(Which(), GetName());
        rVal <<= aApiName;
    }
    else
    {
        css::drawing::PolyPolygonBezierCoords aBezier;
        basegfx::unotools::b2DPolyPolygonToPolyPolygonBezier( maPolyPolygon, aBezier );
        rVal <<= aBezier;
    }

    return true;
}

bool XLineStartItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    if( nMemberId == MID_NAME )
    {
        return false;
    }
    else
    {
        maPolyPolygon.clear();

        if( rVal.hasValue() && rVal.getValue() )
        {
            if( rVal.getValueType() != cppu::UnoType<css::drawing::PolyPolygonBezierCoords>::get())
                return false;

            css::drawing::PolyPolygonBezierCoords const * pCoords = static_cast<css::drawing::PolyPolygonBezierCoords const *>(rVal.getValue());
            if( pCoords->Coordinates.getLength() > 0 )
            {
                maPolyPolygon = basegfx::unotools::polyPolygonBezierToB2DPolyPolygon( *pCoords );
                // #i72807# close line start/end polygons hard
                // maPolyPolygon.setClosed(true);
            }
        }
    }

    return true;
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

        OUString aUniqueName( GetName() );

        if( !maPolyPolygon.count() )
        {
            // if the polygon is empty, check if the name is empty
            if( aUniqueName.isEmpty() )
                return NULL;

            // force empty name for empty polygons
            return new XLineStartItem( "", maPolyPolygon );
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

        bool bForceNew = false;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        sal_uInt32 nCount, nSurrogate;

        const SfxItemPool* pPool1 = &pModel->GetItemPool();
        if( !aUniqueName.isEmpty() && pPool1 )
        {
            nCount = pPool1->GetItemCount2( XATTR_LINESTART );

            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = static_cast<const XLineStartItem*>(pPool1->GetItem2( XATTR_LINESTART, nSurrogate ));

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetLineStartValue() != pLineStartItem->GetLineStartValue() )
                    {
                        // same name but different value, we need a new name for this item
                        aUniqueName.clear();
                        bForceNew = true;
                    }
                    break;
                }
            }

            if( !bForceNew )
            {
                nCount = pPool1->GetItemCount2( XATTR_LINEEND );

                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = static_cast<const XLineEndItem*>(pPool1->GetItem2( XATTR_LINEEND, nSurrogate ));

                    if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetLineEndValue() != pLineStartItem->GetLineStartValue() )
                        {
                            // same name but different value, we need a new name for this item
                            aUniqueName.clear();
                            bForceNew = true;
                        }
                        break;
                    }
                }
            }
        }

        const SfxItemPool* pPool2 = pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL;
        if( !aUniqueName.isEmpty() && pPool2)
        {
            nCount = pPool2->GetItemCount2( XATTR_LINESTART );
            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = static_cast<const XLineStartItem*>(pPool2->GetItem2( XATTR_LINESTART, nSurrogate ));

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetLineStartValue() != pLineStartItem->GetLineStartValue() )
                    {
                        // same name but different value, we need a new name for this item
                        aUniqueName.clear();
                        bForceNew = true;
                    }
                    break;
                }
            }

            if( !bForceNew )
            {
                nCount = pPool2->GetItemCount2( XATTR_LINEEND );
                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = static_cast<const XLineEndItem*>(pPool2->GetItem2( XATTR_LINEEND, nSurrogate ));

                    if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetLineEndValue() != pLineStartItem->GetLineStartValue() )
                        {
                            // same name but different value, we need a new name for this item
                            aUniqueName.clear();
                            bForceNew = true;
                        }
                        break;
                    }
                }
            }
        }

        // if we have no name yet, find existing item with same content or
        // create a unique name
        if( aUniqueName.isEmpty() )
        {
            bool bFoundExisting = false;

            sal_Int32 nUserIndex = 1;
            const ResId aRes(SVX_RES(RID_SVXSTR_LINEEND));
            const OUString aUser( aRes.toString() );

            if( pPool1 )
            {
                nCount = pPool1->GetItemCount2( XATTR_LINESTART );
                sal_uInt32 nSurrogate2;

                for( nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++ )
                {
                    const XLineStartItem* pItem = static_cast<const XLineStartItem*>(pPool1->GetItem2( XATTR_LINESTART, nSurrogate2 ));

                    if( pItem && !pItem->GetName().isEmpty() )
                    {
                        if( !bForceNew && pItem->GetLineStartValue() == pLineStartItem->GetLineStartValue() )
                        {
                            aUniqueName = pItem->GetName();
                            bFoundExisting = true;
                            break;
                        }

                        if( pItem->GetName().startsWith( aUser ) )
                        {
                            sal_Int32 nThisIndex = pItem->GetName().copy( aUser.getLength() ).toInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }

                nCount = pPool1->GetItemCount2( XATTR_LINEEND );
                for( nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++ )
                {
                    const XLineEndItem* pItem = static_cast<const XLineEndItem*>(pPool1->GetItem2( XATTR_LINEEND, nSurrogate2 ));

                    if( pItem && !pItem->GetName().isEmpty() )
                    {
                        if( !bForceNew && pItem->GetLineEndValue() == pLineStartItem->GetLineStartValue() )
                        {
                            aUniqueName = pItem->GetName();
                            bFoundExisting = true;
                            break;
                        }

                        if( pItem->GetName().startsWith( aUser ) )
                        {
                            sal_Int32 nThisIndex = pItem->GetName().copy( aUser.getLength() ).toInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }
            }

            if( !bFoundExisting )
            {
                aUniqueName = aUser;
                aUniqueName += " ";
                aUniqueName += OUString::number( nUserIndex );
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

    return NULL;
}

TYPEINIT1_AUTOFACTORY(XLineEndItem, NameOrIndex);

//SfxPoolItem* XLineEndItem::CreateDefault() { return new XLineEndItem; }

XLineEndItem::XLineEndItem(sal_Int32 nIndex)
:   NameOrIndex(XATTR_LINEEND, nIndex)
{
}

XLineEndItem::XLineEndItem(const OUString& rName, const basegfx::B2DPolyPolygon& rPolyPolygon)
:   NameOrIndex(XATTR_LINEEND, rName),
    maPolyPolygon(rPolyPolygon)
{
}

XLineEndItem::XLineEndItem(const XLineEndItem& rItem)
:   NameOrIndex(rItem),
    maPolyPolygon(rItem.maPolyPolygon)
{
}

XLineEndItem::XLineEndItem(SvStream& rIn) :
    NameOrIndex(XATTR_LINEEND, rIn)
{
    if (!IsIndex())
    {
        maPolyPolygon = streamInB2DPolyPolygon(rIn);
    }
}

XLineEndItem::XLineEndItem(SfxItemPool* /*pPool*/, const basegfx::B2DPolyPolygon& rPolyPolygon)
:   NameOrIndex( XATTR_LINEEND, -1 ),
    maPolyPolygon(rPolyPolygon)
{
}

SfxPoolItem* XLineEndItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineEndItem(*this);
}

bool XLineEndItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) && static_cast<const XLineEndItem&>(rItem).maPolyPolygon == maPolyPolygon );
}

SfxPoolItem* XLineEndItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineEndItem(rIn);
}

SvStream& XLineEndItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        streamOutB2DPolyPolygon(maPolyPolygon, rOut);
    }

    return rOut;
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

        OUString aUniqueName( GetName() );

        if( !maPolyPolygon.count() )
        {
            // if the polygon is empty, check if the name is empty
            if( aUniqueName.isEmpty() )
                return NULL;

            // force empty name for empty polygons
            return new XLineEndItem( "", maPolyPolygon );
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

        bool bForceNew = false;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        sal_uInt16 nCount, nSurrogate;

        const SfxItemPool* pPool1 = &pModel->GetItemPool();
        if( !aUniqueName.isEmpty() && pPool1 )
        {
            nCount = pPool1->GetItemCount2( XATTR_LINESTART );

            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = static_cast<const XLineStartItem*>(pPool1->GetItem2( XATTR_LINESTART, nSurrogate ));

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetLineStartValue() != pLineEndItem->GetLineEndValue() )
                    {
                        // same name but different value, we need a new name for this item
                        aUniqueName.clear();
                        bForceNew = true;
                    }
                    break;
                }
            }

            if( !bForceNew )
            {
                nCount = pPool1->GetItemCount2( XATTR_LINEEND );

                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = static_cast<const XLineEndItem*>(pPool1->GetItem2( XATTR_LINEEND, nSurrogate ));

                    if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetLineEndValue() != pLineEndItem->GetLineEndValue() )
                        {
                            // same name but different value, we need a new name for this item
                            aUniqueName.clear();
                            bForceNew = true;
                        }
                        break;
                    }
                }
            }
        }

        const SfxItemPool* pPool2 = pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL;
        if( !aUniqueName.isEmpty() && pPool2)
        {
            nCount = pPool2->GetItemCount2( XATTR_LINESTART );
            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = static_cast<const XLineStartItem*>(pPool2->GetItem2( XATTR_LINESTART, nSurrogate ));

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetLineStartValue() != pLineEndItem->GetLineEndValue() )
                    {
                        // same name but different value, we need a new name for this item
                        aUniqueName.clear();
                        bForceNew = true;
                    }
                    break;
                }
            }

            if( !bForceNew )
            {
                nCount = pPool2->GetItemCount2( XATTR_LINEEND );
                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = static_cast<const XLineEndItem*>(pPool2->GetItem2( XATTR_LINEEND, nSurrogate ));

                    if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value its ok to set it
                        if( pItem->GetLineEndValue() != pLineEndItem->GetLineEndValue() )
                        {
                            // same name but different value, we need a new name for this item
                            aUniqueName.clear();
                            bForceNew = true;
                        }
                        break;
                    }
                }
            }
        }

        // if we have no name yet, find existing item with same content or
        // create a unique name
        if( aUniqueName.isEmpty() )
        {
            bool bFoundExisting = false;

            sal_Int32 nUserIndex = 1;
            const ResId aRes(SVX_RES(RID_SVXSTR_LINEEND));
            const OUString aUser( aRes.toString() );

            if( pPool1 )
            {
                nCount = pPool1->GetItemCount2( XATTR_LINESTART );
                sal_uInt32 nSurrogate2;

                for( nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++ )
                {
                    const XLineStartItem* pItem = static_cast<const XLineStartItem*>(pPool1->GetItem2( XATTR_LINESTART, nSurrogate2 ));

                    if( pItem && !pItem->GetName().isEmpty() )
                    {
                        if( !bForceNew && pItem->GetLineStartValue() == pLineEndItem->GetLineEndValue() )
                        {
                            aUniqueName = pItem->GetName();
                            bFoundExisting = true;
                            break;
                        }

                        if( pItem->GetName().startsWith( aUser ) )
                        {
                            sal_Int32 nThisIndex = pItem->GetName().copy( aUser.getLength() ).toInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }

                nCount = pPool1->GetItemCount2( XATTR_LINEEND );
                for( nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++ )
                {
                    const XLineEndItem* pItem = static_cast<const XLineEndItem*>(pPool1->GetItem2( XATTR_LINEEND, nSurrogate2 ));

                    if( pItem && !pItem->GetName().isEmpty() )
                    {
                        if( !bForceNew && pItem->GetLineEndValue() == pLineEndItem->GetLineEndValue() )
                        {
                            aUniqueName = pItem->GetName();
                            bFoundExisting = true;
                            break;
                        }

                        if( pItem->GetName().startsWith( aUser ) )
                        {
                            sal_Int32 nThisIndex = pItem->GetName().copy( aUser.getLength() ).toInt32();
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }
            }

            if( !bFoundExisting )
            {
                aUniqueName = aUser;
                aUniqueName += " ";
                aUniqueName += OUString::number( nUserIndex );
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

    return NULL;
}

bool XLineEndItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetName();
    return true;
}

bool XLineEndItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    if( nMemberId == MID_NAME )
    {
        OUString aApiName = SvxUnogetApiNameForItem(Which(), GetName());
        rVal <<= aApiName;
    }
    else
    {
        css::drawing::PolyPolygonBezierCoords aBezier;
        basegfx::unotools::b2DPolyPolygonToPolyPolygonBezier( maPolyPolygon, aBezier );
        rVal <<= aBezier;
    }
    return true;
}

bool XLineEndItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    if( nMemberId == MID_NAME )
    {
        return false;
    }
    else
    {
        maPolyPolygon.clear();

        if( rVal.hasValue() && rVal.getValue() )
        {
            if( rVal.getValueType() != cppu::UnoType<css::drawing::PolyPolygonBezierCoords>::get())
                return false;

            css::drawing::PolyPolygonBezierCoords const * pCoords = static_cast<css::drawing::PolyPolygonBezierCoords const *>(rVal.getValue());
            if( pCoords->Coordinates.getLength() > 0 )
            {
                maPolyPolygon = basegfx::unotools::polyPolygonBezierToB2DPolyPolygon( *pCoords );
                // #i72807# close line start/end polygons hard
                // maPolyPolygon.setClosed(true);
            }
        }
    }

    return true;
}

TYPEINIT1_AUTOFACTORY(XLineStartWidthItem, SfxMetricItem);

//SfxPoolItem* XLineStartWidthItem::CreateDefault() { return new XLineStartWidthItem; }

XLineStartWidthItem::XLineStartWidthItem(long nWidth) :
    SfxMetricItem(XATTR_LINESTARTWIDTH, nWidth)
{
}

XLineStartWidthItem::XLineStartWidthItem(SvStream& rIn) :
    SfxMetricItem(XATTR_LINESTARTWIDTH, rIn)
{
}

SfxPoolItem* XLineStartWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStartWidthItem(*this);
}

SfxPoolItem* XLineStartWidthItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineStartWidthItem(rIn);
}

bool XLineStartWidthItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper * pIntl
)   const
{
    rText = GetMetricText( (long) GetValue(),
                            eCoreUnit, ePresUnit, pIntl) +
            " " + EE_RESSTR( GetMetricId( ePresUnit) );
    return true;
}

bool XLineStartWidthItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool XLineStartWidthItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue( nValue );
    return true;
}

TYPEINIT1_AUTOFACTORY(XLineEndWidthItem, SfxMetricItem);

//SfxPoolItem* XLineEndWidthItem::CreateDefault() { return new XLineEndWidthItem; }

XLineEndWidthItem::XLineEndWidthItem(long nWidth) :
   SfxMetricItem(XATTR_LINEENDWIDTH, nWidth)
{
}

XLineEndWidthItem::XLineEndWidthItem(SvStream& rIn) :
    SfxMetricItem(XATTR_LINEENDWIDTH, rIn)
{
}

SfxPoolItem* XLineEndWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineEndWidthItem(*this);
}

SfxPoolItem* XLineEndWidthItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineEndWidthItem(rIn);
}

bool XLineEndWidthItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    rText = GetMetricText( (long) GetValue(),
                            eCoreUnit, ePresUnit, pIntl) +
            " " + EE_RESSTR( GetMetricId( ePresUnit) );
    return true;
}

bool XLineEndWidthItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool XLineEndWidthItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue( nValue );
    return true;
}

TYPEINIT1_AUTOFACTORY(XLineStartCenterItem, SfxBoolItem);

//SfxPoolItem* XLineStartCenterItem::CreateDefault() { return new XLineStartCenterItem; }

XLineStartCenterItem::XLineStartCenterItem(bool bStartCenter) :
    SfxBoolItem(XATTR_LINESTARTCENTER, bStartCenter)
{
}

XLineStartCenterItem::XLineStartCenterItem(SvStream& rIn) :
    SfxBoolItem(XATTR_LINESTARTCENTER, rIn)
{
}

SfxPoolItem* XLineStartCenterItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStartCenterItem(*this);
}

SfxPoolItem* XLineStartCenterItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineStartCenterItem(rIn);
}

bool XLineStartCenterItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = OUString( ResId( GetValue() ? RID_SVXSTR_CENTERED :
                    RID_SVXSTR_NOTCENTERED, DIALOG_MGR() ) );
    return true;
}

bool XLineStartCenterItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    sal_Bool bValue = GetValue();
    rVal.setValue( &bValue, cppu::UnoType<bool>::get() );
    return true;
}

bool XLineStartCenterItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    if( !rVal.hasValue() || rVal.getValueType() != cppu::UnoType<bool>::get() )
        return false;

    SetValue( *static_cast<sal_Bool const *>(rVal.getValue()) );
    return true;
}

TYPEINIT1_AUTOFACTORY(XLineEndCenterItem, SfxBoolItem);

//SfxPoolItem* XLineEndCenterItem::CreateDefault() { return new XLineEndCenterItem; }

XLineEndCenterItem::XLineEndCenterItem(bool bEndCenter) :
    SfxBoolItem(XATTR_LINEENDCENTER, bEndCenter)
{
}

XLineEndCenterItem::XLineEndCenterItem(SvStream& rIn) :
    SfxBoolItem(XATTR_LINEENDCENTER, rIn)
{
}

SfxPoolItem* XLineEndCenterItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineEndCenterItem(*this);
}

SfxPoolItem* XLineEndCenterItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineEndCenterItem(rIn);
}

bool XLineEndCenterItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = OUString( ResId( GetValue() ? RID_SVXSTR_CENTERED :
                    RID_SVXSTR_NOTCENTERED, DIALOG_MGR() ) );
    return true;
}

bool XLineEndCenterItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    sal_Bool bValue = GetValue();
    rVal.setValue( &bValue, cppu::UnoType<bool>::get() );
    return true;
}

bool XLineEndCenterItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    if( !rVal.hasValue() || rVal.getValueType() != cppu::UnoType<bool>::get() )
        return false;

    SetValue( *static_cast<sal_Bool const *>(rVal.getValue()) );
    return true;
}

// --- fill attributes ---

TYPEINIT1_AUTOFACTORY(XFillStyleItem, SfxEnumItem);

SfxPoolItem* XFillStyleItem::CreateDefault() { return new XFillStyleItem; }

XFillStyleItem::XFillStyleItem(drawing::FillStyle eFillStyle) :
    SfxEnumItem(XATTR_FILLSTYLE, sal::static_int_cast< sal_uInt16 >(eFillStyle))
{
}

XFillStyleItem::XFillStyleItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FILLSTYLE, rIn)
{
}

SfxPoolItem* XFillStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillStyleItem( *this );
}

SfxPoolItem* XFillStyleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillStyleItem(rIn);
}

bool XFillStyleItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();

    sal_uInt16 nId = 0;

    switch( (sal_uInt16)GetValue() )
    {
        case drawing::FillStyle_NONE:
            nId = RID_SVXSTR_INVISIBLE;
            break;
        case drawing::FillStyle_SOLID:
            nId = RID_SVXSTR_SOLID;
            break;
        case drawing::FillStyle_GRADIENT:
            nId = RID_SVXSTR_GRADIENT;
            break;
        case drawing::FillStyle_HATCH:
            nId = RID_SVXSTR_HATCH;
            break;
        case drawing::FillStyle_BITMAP:
            nId = RID_SVXSTR_BITMAP;
            break;
    }

    if ( nId )
        rText = SVX_RESSTR( nId );
    return true;
}

sal_uInt16 XFillStyleItem::GetValueCount() const
{
    return 5;
}

bool XFillStyleItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    css::drawing::FillStyle eFS = (css::drawing::FillStyle)GetValue();

    rVal <<= eFS;

    return true;
}

bool XFillStyleItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    css::drawing::FillStyle eFS;
    if(!(rVal >>= eFS))
    {
        // also try an int (for Basic)
        sal_Int32 nFS = 0;
        if(!(rVal >>= nFS))
            return false;
        eFS = (css::drawing::FillStyle)nFS;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( eFS ) );

    return true;
}

void XFillStyleItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("xFillStyleItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));

    OUString aPresentation;
    GetPresentation(SFX_ITEM_PRESENTATION_NAMELESS, SFX_MAPUNIT_100TH_MM, SFX_MAPUNIT_100TH_MM, aPresentation);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    xmlTextWriterEndElement(pWriter);
}

TYPEINIT1_AUTOFACTORY(XFillColorItem, XColorItem);

//SfxPoolItem* XFillColorItem::CreateDefault() { return new XFillColorItem; }

XFillColorItem::XFillColorItem(sal_Int32 nIndex, const Color& rTheColor) :
    XColorItem(XATTR_FILLCOLOR, nIndex, rTheColor)
{
}

XFillColorItem::XFillColorItem(const OUString& rName, const Color& rTheColor) :
    XColorItem(XATTR_FILLCOLOR, rName, rTheColor)
{
}

XFillColorItem::XFillColorItem(SvStream& rIn) :
    XColorItem(XATTR_FILLCOLOR, rIn)
{
}

SfxPoolItem* XFillColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillColorItem(*this);
}

SfxPoolItem* XFillColorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillColorItem(rIn);
}

bool XFillColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetName();
    return true;
}

bool XFillColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetColorValue().GetRGBColor();

    return true;
}

bool XFillColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue ))
        return false;

    SetColorValue( nValue );
    return true;
}

void XFillColorItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("xFillColorItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(GetColorValue().AsRGBHexString().toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
}

TYPEINIT1_AUTOFACTORY(XSecondaryFillColorItem, XColorItem);

//SfxPoolItem* XSecondaryFillColorItem::CreateDefault() { return new XSecondaryFillColorItem; }

XSecondaryFillColorItem::XSecondaryFillColorItem(const OUString& rName, const Color& rTheColor) :
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
        return new XSecondaryFillColorItem( "", Color(0,184,255) );
}

sal_uInt16 XSecondaryFillColorItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/ ) const
{
    return 2;
}

bool XSecondaryFillColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetName();
    return true;
}

XGradient::XGradient() :
    eStyle( css::awt::GradientStyle_LINEAR ),
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

XGradient::XGradient(const Color& rStart, const Color& rEnd,
                     css::awt::GradientStyle eTheStyle, long nTheAngle, sal_uInt16 nXOfs,
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

TYPEINIT1_AUTOFACTORY(XFillGradientItem, NameOrIndex);

//SfxPoolItem* XFillGradientItem::CreateDefault() { return new XFillGradientItem; }

XFillGradientItem::XFillGradientItem(sal_Int32 nIndex,
                                   const XGradient& rTheGradient) :
    NameOrIndex(XATTR_FILLGRADIENT, nIndex),
    aGradient(rTheGradient)
{
}

XFillGradientItem::XFillGradientItem(const OUString& rName,
                                   const XGradient& rTheGradient, sal_uInt16 nWhich)
    : NameOrIndex(nWhich, rName)
    , aGradient(rTheGradient)
{
}

XFillGradientItem::XFillGradientItem(const XFillGradientItem& rItem) :
    NameOrIndex(rItem),
    aGradient(rItem.aGradient)
{
}

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

        rIn.ReadInt16( nITemp ); aGradient.SetGradientStyle((css::awt::GradientStyle)nITemp);
        rIn.ReadUInt16( nRed );
        rIn.ReadUInt16( nGreen );
        rIn.ReadUInt16( nBlue );
        Color aCol;
        aCol = Color( (sal_uInt8)( nRed >> 8 ), (sal_uInt8)( nGreen >> 8 ), (sal_uInt8)( nBlue >> 8 ) );
        aGradient.SetStartColor( aCol );

        rIn.ReadUInt16( nRed );
        rIn.ReadUInt16( nGreen );
        rIn.ReadUInt16( nBlue );
        aCol = Color( (sal_uInt8)( nRed >> 8 ), (sal_uInt8)( nGreen >> 8 ), (sal_uInt8)( nBlue >> 8 ) );
        aGradient.SetEndColor(aCol);
        rIn.ReadInt32( nLTemp ); aGradient.SetAngle(nLTemp);
        rIn.ReadUInt16( nUSTemp ); aGradient.SetBorder(nUSTemp);
        rIn.ReadUInt16( nUSTemp ); aGradient.SetXOffset(nUSTemp);
        rIn.ReadUInt16( nUSTemp ); aGradient.SetYOffset(nUSTemp);
        rIn.ReadUInt16( nUSTemp ); aGradient.SetStartIntens(nUSTemp);
        rIn.ReadUInt16( nUSTemp ); aGradient.SetEndIntens(nUSTemp);

        // for newer versions consider the step width as well
        if (nVer >= 1)
        {
            rIn.ReadUInt16( nUSTemp ); aGradient.SetSteps(nUSTemp);
        }
    }
}

XFillGradientItem::XFillGradientItem( const XGradient& rTheGradient )
:   NameOrIndex( XATTR_FILLGRADIENT, -1 ),
    aGradient(rTheGradient)
{
}

SfxPoolItem* XFillGradientItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillGradientItem(*this);
}

bool XFillGradientItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             aGradient == static_cast<const XFillGradientItem&>(rItem).aGradient );
}

SfxPoolItem* XFillGradientItem::Create(SvStream& rIn, sal_uInt16 nVer) const
{
    return new XFillGradientItem(rIn, nVer);
}

SvStream& XFillGradientItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut.WriteInt16( aGradient.GetGradientStyle() );

        sal_uInt16 nTmp;

        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetRed() ); rOut.WriteUInt16( nTmp );
        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetGreen() ); rOut.WriteUInt16( nTmp );
        nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetBlue() ); rOut.WriteUInt16( nTmp );
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetRed() ); rOut.WriteUInt16( nTmp );
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetGreen() ); rOut.WriteUInt16( nTmp );
        nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetBlue() ); rOut.WriteUInt16( nTmp );

        rOut.WriteInt32( aGradient.GetAngle() );
        rOut.WriteUInt16( aGradient.GetBorder() );
        rOut.WriteUInt16( aGradient.GetXOffset() );
        rOut.WriteUInt16( aGradient.GetYOffset() );
        rOut.WriteUInt16( aGradient.GetStartIntens() );
        rOut.WriteUInt16( aGradient.GetEndIntens() );
        rOut.WriteUInt16( aGradient.GetSteps() );
    }

    return rOut;
}

const XGradient& XFillGradientItem::GetGradientValue() const // GetValue -> GetGradientValue
{
    if (!IsIndex())
        return aGradient;
    // ToDo: This should fail. We never called this code with a table so this should always
    // have failed. Thus, I'm thinking that XFillGradientItem can't be an Index.
    return aGradient;
}

sal_uInt16 XFillGradientItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/) const
{
    // !!! this version number also represents the version number of superclasses
    // !!! (e.g. XFillFloatTransparenceItem); if you make any changes here,
    // !!! the superclass is also affected
    return 1;
}

bool XFillGradientItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetName();
    return true;
}

bool XFillGradientItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue > aPropSeq( 2 );

            css::awt::Gradient aGradient2;

            const XGradient& aXGradient = GetGradientValue();
            aGradient2.Style = (css::awt::GradientStyle) aXGradient.GetGradientStyle();
            aGradient2.StartColor = (sal_Int32)aXGradient.GetStartColor().GetColor();
            aGradient2.EndColor = (sal_Int32)aXGradient.GetEndColor().GetColor();
            aGradient2.Angle = (short)aXGradient.GetAngle();
            aGradient2.Border = aXGradient.GetBorder();
            aGradient2.XOffset = aXGradient.GetXOffset();
            aGradient2.YOffset = aXGradient.GetYOffset();
            aGradient2.StartIntensity = aXGradient.GetStartIntens();
            aGradient2.EndIntensity = aXGradient.GetEndIntens();
            aGradient2.StepCount = aXGradient.GetSteps();

            OUString aApiName = SvxUnogetApiNameForItem(Which(), GetName());
            aPropSeq[0].Name    = "Name";
            aPropSeq[0].Value   = uno::makeAny( aApiName );
            aPropSeq[1].Name    = "FillGradient";
            aPropSeq[1].Value   = uno::makeAny( aGradient2 );
            rVal = uno::makeAny( aPropSeq );
            break;
        }

        case MID_FILLGRADIENT:
        {
            const XGradient& aXGradient = GetGradientValue();
            css::awt::Gradient aGradient2;

            aGradient2.Style = (css::awt::GradientStyle) aXGradient.GetGradientStyle();
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
            OUString aApiName = SvxUnogetApiNameForItem(Which(), GetName());
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

        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}

bool XFillGradientItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue >   aPropSeq;

            if ( rVal >>= aPropSeq )
            {
                css::awt::Gradient aGradient2;
                OUString aName;
                bool bGradient( false );
                for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
                {
                    if ( aPropSeq[n].Name == "Name" )
                        aPropSeq[n].Value >>= aName;
                    else if ( aPropSeq[n].Name == "FillGradient" )
                    {
                        if ( aPropSeq[n].Value >>= aGradient2 )
                            bGradient = true;
                    }
                }

                SetName( aName );
                if ( bGradient )
                {
                    XGradient aXGradient;

                    aXGradient.SetGradientStyle( (css::awt::GradientStyle) aGradient2.Style );
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

                return true;
            }

            return false;
        }

        case MID_NAME:
        {
            OUString aName;
            if (!(rVal >>= aName ))
                return false;
            SetName( aName );
            break;
        }

        case MID_FILLGRADIENT:
        {
            css::awt::Gradient aGradient2;
            if(!(rVal >>= aGradient2))
                return false;

            XGradient aXGradient;

            aXGradient.SetGradientStyle( (css::awt::GradientStyle) aGradient2.Style );
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
                return false;

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
                return false;

            XGradient aXGradient = GetGradientValue();

            switch ( nMemberId )
            {
                case MID_GRADIENT_STYLE:
                    aXGradient.SetGradientStyle( (css::awt::GradientStyle)nVal ); break;
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

    return true;
}

bool XFillGradientItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return static_cast<const XFillGradientItem*>(p1)->GetGradientValue() == static_cast<const XFillGradientItem*>(p2)->GetGradientValue();
}

XFillGradientItem* XFillGradientItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const OUString aUniqueName = NameOrIndex::CheckNamedItem(
                this, Which(), &pModel->GetItemPool(),
                pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                XFillGradientItem::CompareValueFunc, RID_SVXSTR_GRADIENT,
                pModel->GetPropertyList( XGRADIENT_LIST ) );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
            return new XFillGradientItem( aUniqueName, aGradient, Which() );
    }

    return NULL;
}

TYPEINIT1_AUTOFACTORY( XFillFloatTransparenceItem, XFillGradientItem );

//SfxPoolItem* XFillFloatTransparenceItem::CreateDefault() { return new XFillFloatTransparenceItem; }

XFillFloatTransparenceItem::XFillFloatTransparenceItem() :
    bEnabled( false )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

XFillFloatTransparenceItem::XFillFloatTransparenceItem(const OUString& rName, const XGradient& rGradient, bool bEnable ) :
    XFillGradientItem   ( rName, rGradient ),
    bEnabled            ( bEnable )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

XFillFloatTransparenceItem::XFillFloatTransparenceItem( const XFillFloatTransparenceItem& rItem ) :
    XFillGradientItem   ( rItem ),
    bEnabled            ( rItem.bEnabled )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

XFillFloatTransparenceItem::XFillFloatTransparenceItem(SfxItemPool* /*pPool*/, const XGradient& rTheGradient, bool bEnable )
:   XFillGradientItem   ( -1, rTheGradient ),
    bEnabled            ( bEnable )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

bool XFillFloatTransparenceItem::operator==( const SfxPoolItem& rItem ) const
{
    return ( NameOrIndex::operator==(rItem) ) &&
           ( GetGradientValue() == static_cast<const XFillGradientItem&>(rItem).GetGradientValue() ) &&
           ( bEnabled == static_cast<const XFillFloatTransparenceItem&>(rItem).bEnabled );
}

SfxPoolItem* XFillFloatTransparenceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillFloatTransparenceItem( *this );
}

sal_uInt16 XFillFloatTransparenceItem::GetVersion( sal_uInt16 nFileFormatVersion ) const
{
    // !!! if version number of this object must be increased, please   !!!
    // !!! increase version number of base class XFillGradientItem      !!!
    return XFillGradientItem::GetVersion( nFileFormatVersion );
}

bool XFillFloatTransparenceItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    return XFillGradientItem::QueryValue( rVal, nMemberId );
}

bool XFillFloatTransparenceItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    return XFillGradientItem::PutValue( rVal, nMemberId );
}

bool XFillFloatTransparenceItem::GetPresentation(    SfxItemPresentation ePres,
                                                                    SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
                                                                    OUString& rText,
                                                                    const IntlWrapper * pIntlWrapper ) const
{
    return XFillGradientItem::GetPresentation( ePres, eCoreUnit, ePresUnit, rText, pIntlWrapper );
}

bool XFillFloatTransparenceItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return  static_cast<const XFillFloatTransparenceItem*>(p1)->IsEnabled() == static_cast<const XFillFloatTransparenceItem*>(p2)->IsEnabled() &&
            static_cast<const XFillFloatTransparenceItem*>(p1)->GetGradientValue()  == static_cast<const XFillFloatTransparenceItem*>(p2)->GetGradientValue();
}

XFillFloatTransparenceItem* XFillFloatTransparenceItem::checkForUniqueItem( SdrModel* pModel ) const
{
    // #85953# unique name only necessary when enabled
    if(IsEnabled())
    {
        if( pModel )
        {
            const OUString aUniqueName = NameOrIndex::CheckNamedItem( this,
                                                                    XATTR_FILLFLOATTRANSPARENCE,
                                                                    &pModel->GetItemPool(),
                                                                    pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                                                                    XFillFloatTransparenceItem::CompareValueFunc,
                                                                    RID_SVXSTR_TRASNGR0,
                                                                    XPropertyListRef() );

            // if the given name is not valid, replace it!
            if( aUniqueName != GetName() )
            {
                return new XFillFloatTransparenceItem( aUniqueName, GetGradientValue(), true );
            }
        }
    }
    else
    {
        // #85953# if disabled, force name to empty string
        if( !GetName().isEmpty() )
        {
            return new XFillFloatTransparenceItem(OUString(), GetGradientValue(), false);
        }
    }

    return NULL;
}

XHatch::XHatch(const Color& rCol, css::drawing::HatchStyle eTheStyle, long nTheDistance,
               long nTheAngle) :
    eStyle(eTheStyle),
    aColor(rCol),
    nDistance(nTheDistance),
    nAngle(nTheAngle)
{
}

bool XHatch::operator==(const XHatch& rHatch) const
{
    return ( eStyle     == rHatch.eStyle    &&
             aColor     == rHatch.aColor    &&
             nDistance  == rHatch.nDistance &&
             nAngle     == rHatch.nAngle );
}

TYPEINIT1_AUTOFACTORY(XFillHatchItem, NameOrIndex);

//SfxPoolItem* XFillHatchItem::CreateDefault() { return new XFillHatchItem; }

XFillHatchItem::XFillHatchItem(const OUString& rName,
                             const XHatch& rTheHatch) :
    NameOrIndex(XATTR_FILLHATCH, rName),
    aHatch(rTheHatch)
{
}

XFillHatchItem::XFillHatchItem(const XFillHatchItem& rItem) :
    NameOrIndex(rItem),
    aHatch(rItem.aHatch)
{
}

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

        rIn.ReadInt16( nITemp ); aHatch.SetHatchStyle((css::drawing::HatchStyle)nITemp);
        rIn.ReadUInt16( nRed );
        rIn.ReadUInt16( nGreen );
        rIn.ReadUInt16( nBlue );

        Color aCol;
        aCol = Color( (sal_uInt8)( nRed >> 8 ), (sal_uInt8)( nGreen >> 8 ), (sal_uInt8)( nBlue >> 8 ) );
        aHatch.SetColor(aCol);
        rIn.ReadInt32( nLTemp ); aHatch.SetDistance(nLTemp);
        rIn.ReadInt32( nLTemp ); aHatch.SetAngle(nLTemp);
    }
}

XFillHatchItem::XFillHatchItem(SfxItemPool* /*pPool*/, const XHatch& rTheHatch)
:   NameOrIndex( XATTR_FILLHATCH, -1 ),
    aHatch(rTheHatch)
{
}

SfxPoolItem* XFillHatchItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillHatchItem(*this);
}

bool XFillHatchItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             aHatch == static_cast<const XFillHatchItem&>(rItem).aHatch );
}

SfxPoolItem* XFillHatchItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillHatchItem(rIn);
}

SvStream& XFillHatchItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store( rOut, nItemVersion );

    if (!IsIndex())
    {
        rOut.WriteInt16( aHatch.GetHatchStyle() );

        sal_uInt16 nTmp;
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetRed() ); rOut.WriteUInt16( nTmp );
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetGreen() ); rOut.WriteUInt16( nTmp );
        nTmp = VCLTOSVCOL( aHatch.GetColor().GetBlue() ); rOut.WriteUInt16( nTmp );

        rOut.WriteInt32( aHatch.GetDistance() );
        rOut.WriteInt32( aHatch.GetAngle() );
    }

    return rOut;
}


bool XFillHatchItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetName();
    return true;
}

bool XFillHatchItem::HasMetrics() const
{
    return true;
}

bool XFillHatchItem::ScaleMetrics(long nMul, long nDiv)
{
    aHatch.SetDistance( ScaleMetricValue( aHatch.GetDistance(), nMul, nDiv ) );
    return true;
}

bool XFillHatchItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue > aPropSeq( 2 );

            css::drawing::Hatch aUnoHatch;

            aUnoHatch.Style = (css::drawing::HatchStyle)aHatch.GetHatchStyle();
            aUnoHatch.Color = aHatch.GetColor().GetColor();
            aUnoHatch.Distance = aHatch.GetDistance();
            aUnoHatch.Angle = aHatch.GetAngle();

            OUString aApiName = SvxUnogetApiNameForItem(Which(), GetName());
            aPropSeq[0].Name    = "Name";
            aPropSeq[0].Value   = uno::makeAny( aApiName );
            aPropSeq[1].Name    = "FillHatch";
            aPropSeq[1].Value   = uno::makeAny( aUnoHatch );
            rVal = uno::makeAny( aPropSeq );
            break;
        }

        case MID_FILLHATCH:
        {
            css::drawing::Hatch aUnoHatch;

            aUnoHatch.Style = (css::drawing::HatchStyle)aHatch.GetHatchStyle();
            aUnoHatch.Color = aHatch.GetColor().GetColor();
            aUnoHatch.Distance = aHatch.GetDistance();
            aUnoHatch.Angle = aHatch.GetAngle();
            rVal <<= aUnoHatch;
            break;
        }

        case MID_NAME:
        {
            OUString aApiName = SvxUnogetApiNameForItem(Which(), GetName());
            rVal <<= aApiName;
            break;
        }

        case MID_HATCH_STYLE:
            rVal <<= (css::drawing::HatchStyle)aHatch.GetHatchStyle(); break;
        case MID_HATCH_COLOR:
            rVal <<= (sal_Int32)aHatch.GetColor().GetColor(); break;
        case MID_HATCH_DISTANCE:
            rVal <<= aHatch.GetDistance(); break;
        case MID_HATCH_ANGLE:
            rVal <<= aHatch.GetAngle(); break;

        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}

bool XFillHatchItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            uno::Sequence< beans::PropertyValue >   aPropSeq;
            if ( rVal >>= aPropSeq )
            {
                css::drawing::Hatch aUnoHatch;
                OUString aName;
                bool bHatch( false );
                for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
                {
                    if ( aPropSeq[n].Name == "Name" )
                        aPropSeq[n].Value >>= aName;
                    else if ( aPropSeq[n].Name == "FillHatch" )
                    {
                        if ( aPropSeq[n].Value >>= aUnoHatch )
                            bHatch = true;
                    }
                }

                SetName( aName );
                if ( bHatch )
                {
                    aHatch.SetHatchStyle( (css::drawing::HatchStyle)aUnoHatch.Style );
                    aHatch.SetColor( aUnoHatch.Color );
                    aHatch.SetDistance( aUnoHatch.Distance );
                    aHatch.SetAngle( aUnoHatch.Angle );
                }

                return true;
            }

            return false;
        }

        case MID_FILLHATCH:
        {
            css::drawing::Hatch aUnoHatch;
            if(!(rVal >>= aUnoHatch))
                return false;

            aHatch.SetHatchStyle( (css::drawing::HatchStyle)aUnoHatch.Style );
            aHatch.SetColor( aUnoHatch.Color );
            aHatch.SetDistance( aUnoHatch.Distance );
            aHatch.SetAngle( aUnoHatch.Angle );
            break;
        }

        case MID_NAME:
        {
            OUString aName;
            if (!(rVal >>= aName ))
                return false;
            SetName( aName );
            break;
        }

        case MID_HATCH_STYLE:
        {
            sal_Int16 nVal = sal_Int16();
            if (!(rVal >>= nVal ))
                return false;
            aHatch.SetHatchStyle( (css::drawing::HatchStyle)nVal );
            break;
        }

        case MID_HATCH_COLOR:
        case MID_HATCH_DISTANCE:
        case MID_HATCH_ANGLE:
        {
            sal_Int32 nVal = 0;
            if (!(rVal >>= nVal ))
                return false;

            if ( nMemberId == MID_HATCH_COLOR )
                aHatch.SetColor( nVal );
            else if ( nMemberId == MID_HATCH_DISTANCE )
                aHatch.SetDistance( nVal );
            else
                aHatch.SetAngle( nVal );
            break;
        }

        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}

bool XFillHatchItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return static_cast<const XFillHatchItem*>(p1)->GetHatchValue() == static_cast<const XFillHatchItem*>(p2)->GetHatchValue();
}

XFillHatchItem* XFillHatchItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const OUString aUniqueName = NameOrIndex::CheckNamedItem(
                this, XATTR_FILLHATCH, &pModel->GetItemPool(),
                pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                XFillHatchItem::CompareValueFunc, RID_SVXSTR_HATCH10,
                pModel->GetPropertyList( XHATCH_LIST ) );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
            return new XFillHatchItem( aUniqueName, aHatch );
    }

    return NULL;
}

// --- form text attributes ---

TYPEINIT1_AUTOFACTORY(XFormTextStyleItem, SfxEnumItem);

SfxPoolItem* XFormTextStyleItem::CreateDefault() { return new XFormTextStyleItem; }

XFormTextStyleItem::XFormTextStyleItem(XFormTextStyle eTheStyle) :
    SfxEnumItem(XATTR_FORMTXTSTYLE, sal::static_int_cast< sal_uInt16 >(eTheStyle))
{
}

XFormTextStyleItem::XFormTextStyleItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FORMTXTSTYLE, rIn)
{
}

SfxPoolItem* XFormTextStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextStyleItem( *this );
}

SfxPoolItem* XFormTextStyleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextStyleItem(rIn);
}

sal_uInt16 XFormTextStyleItem::GetValueCount() const
{
    return 5;
}

bool XFormTextStyleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool XFormTextStyleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(sal::static_int_cast< sal_uInt16 >(nValue));

    return true;
}

TYPEINIT1_AUTOFACTORY(XFormTextAdjustItem, SfxEnumItem);

SfxPoolItem* XFormTextAdjustItem::CreateDefault() { return new XFormTextAdjustItem; }

XFormTextAdjustItem::XFormTextAdjustItem(XFormTextAdjust eTheAdjust) :
    SfxEnumItem(XATTR_FORMTXTADJUST, sal::static_int_cast< sal_uInt16 >(eTheAdjust))
{
}

XFormTextAdjustItem::XFormTextAdjustItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FORMTXTADJUST, rIn)
{
}

SfxPoolItem* XFormTextAdjustItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextAdjustItem( *this );
}

SfxPoolItem* XFormTextAdjustItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextAdjustItem(rIn);
}

sal_uInt16 XFormTextAdjustItem::GetValueCount() const
{
    return 4;
}

bool XFormTextAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool XFormTextAdjustItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(sal::static_int_cast< sal_uInt16 >(nValue));

    return true;
}

TYPEINIT1_AUTOFACTORY(XFormTextDistanceItem, SfxMetricItem);

SfxPoolItem* XFormTextDistanceItem::CreateDefault() { return new XFormTextDistanceItem; }

XFormTextDistanceItem::XFormTextDistanceItem(long nDist) :
    SfxMetricItem(XATTR_FORMTXTDISTANCE, nDist)
{
}

XFormTextDistanceItem::XFormTextDistanceItem(SvStream& rIn) :
    SfxMetricItem(XATTR_FORMTXTDISTANCE, rIn)
{
}

SfxPoolItem* XFormTextDistanceItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextDistanceItem(*this);
}

SfxPoolItem* XFormTextDistanceItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextDistanceItem(rIn);
}

//SfxPoolItem* XFormTextStartItem::CreateDefault() { return new XFormTextStartItem; }

TYPEINIT1_AUTOFACTORY(XFormTextStartItem, SfxMetricItem);

SfxPoolItem* XFormTextStartItem::CreateDefault() { return new XFormTextStartItem; }

XFormTextStartItem::XFormTextStartItem(long nStart) :
    SfxMetricItem(XATTR_FORMTXTSTART, nStart)
{
}

XFormTextStartItem::XFormTextStartItem(SvStream& rIn) :
    SfxMetricItem(XATTR_FORMTXTSTART, rIn)
{
}

SfxPoolItem* XFormTextStartItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextStartItem(*this);
}

SfxPoolItem* XFormTextStartItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextStartItem(rIn);
}

TYPEINIT1_AUTOFACTORY(XFormTextMirrorItem, SfxBoolItem);

//SfxPoolItem* XFormTextMirrorItem::CreateDefault() { return new XFormTextMirrorItem; }

XFormTextMirrorItem::XFormTextMirrorItem(bool bMirror) :
    SfxBoolItem(XATTR_FORMTXTMIRROR, bMirror)
{
}

XFormTextMirrorItem::XFormTextMirrorItem(SvStream& rIn) :
    SfxBoolItem(XATTR_FORMTXTMIRROR, rIn)
{
}

SfxPoolItem* XFormTextMirrorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextMirrorItem(*this);
}

SfxPoolItem* XFormTextMirrorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextMirrorItem(rIn);
}

TYPEINIT1_AUTOFACTORY(XFormTextOutlineItem, SfxBoolItem);

SfxPoolItem* XFormTextOutlineItem::CreateDefault() { return new XFormTextOutlineItem; }

XFormTextOutlineItem::XFormTextOutlineItem(bool bOutline) :
    SfxBoolItem(XATTR_FORMTXTOUTLINE, bOutline)
{
}

XFormTextOutlineItem::XFormTextOutlineItem(SvStream& rIn) :
    SfxBoolItem(XATTR_FORMTXTOUTLINE, rIn)
{
}

SfxPoolItem* XFormTextOutlineItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextOutlineItem(*this);
}

SfxPoolItem* XFormTextOutlineItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextOutlineItem(rIn);
}

TYPEINIT1_AUTOFACTORY(XFormTextShadowItem, SfxEnumItem);

SfxPoolItem* XFormTextShadowItem::CreateDefault() { return new XFormTextShadowItem; }

XFormTextShadowItem::XFormTextShadowItem(XFormTextShadow eFormTextShadow) :
    SfxEnumItem(
        XATTR_FORMTXTSHADOW, sal::static_int_cast< sal_uInt16 >(eFormTextShadow))
{
}

XFormTextShadowItem::XFormTextShadowItem(SvStream& rIn) :
    SfxEnumItem(XATTR_FORMTXTSHADOW, rIn)
{
}

SfxPoolItem* XFormTextShadowItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowItem( *this );
}

SfxPoolItem* XFormTextShadowItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextShadowItem(rIn);
}

sal_uInt16 XFormTextShadowItem::GetValueCount() const
{
    return 3;
}

bool XFormTextShadowItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool XFormTextShadowItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(sal::static_int_cast< sal_uInt16 >(nValue));

    return true;
}

TYPEINIT1_AUTOFACTORY(XFormTextShadowColorItem, XColorItem);

//SfxPoolItem* XFormTextShadowColorItem::CreateDefault() { return new XFormTextShadowColorItem; }

XFormTextShadowColorItem::XFormTextShadowColorItem(const OUString& rName,
                                                     const Color& rTheColor) :
    XColorItem(XATTR_FORMTXTSHDWCOLOR, rName, rTheColor)
{
}

XFormTextShadowColorItem::XFormTextShadowColorItem(SvStream& rIn) :
    XColorItem(XATTR_FORMTXTSHDWCOLOR, rIn)
{
}

SfxPoolItem* XFormTextShadowColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowColorItem(*this);
}

SfxPoolItem* XFormTextShadowColorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextShadowColorItem(rIn);
}

TYPEINIT1_AUTOFACTORY(XFormTextShadowXValItem, SfxMetricItem);

SfxPoolItem* XFormTextShadowXValItem::CreateDefault() { return new XFormTextShadowXValItem; }

XFormTextShadowXValItem::XFormTextShadowXValItem(long nVal) :
    SfxMetricItem(XATTR_FORMTXTSHDWXVAL, nVal)
{
}

XFormTextShadowXValItem::XFormTextShadowXValItem(SvStream& rIn) :
    SfxMetricItem(XATTR_FORMTXTSHDWXVAL, rIn)
{
}

SfxPoolItem* XFormTextShadowXValItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowXValItem(*this);
}

SfxPoolItem* XFormTextShadowXValItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextShadowXValItem(rIn);
}

TYPEINIT1_AUTOFACTORY(XFormTextShadowYValItem, SfxMetricItem);

SfxPoolItem* XFormTextShadowYValItem::CreateDefault() { return new XFormTextShadowYValItem; }

XFormTextShadowYValItem::XFormTextShadowYValItem(long nVal) :
    SfxMetricItem(XATTR_FORMTXTSHDWYVAL, nVal)
{
}

XFormTextShadowYValItem::XFormTextShadowYValItem(SvStream& rIn) :
    SfxMetricItem(XATTR_FORMTXTSHDWYVAL, rIn)
{
}

SfxPoolItem* XFormTextShadowYValItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowYValItem(*this);
}

SfxPoolItem* XFormTextShadowYValItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextShadowYValItem(rIn);
}

TYPEINIT1_AUTOFACTORY(XFormTextHideFormItem, SfxBoolItem);

SfxPoolItem* XFormTextHideFormItem::CreateDefault() { return new XFormTextHideFormItem; }

XFormTextHideFormItem::XFormTextHideFormItem(bool bHide) :
    SfxBoolItem(XATTR_FORMTXTHIDEFORM, bHide)
{
}

XFormTextHideFormItem::XFormTextHideFormItem(SvStream& rIn) :
    SfxBoolItem(XATTR_FORMTXTHIDEFORM, rIn)
{
}

SfxPoolItem* XFormTextHideFormItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextHideFormItem(*this);
}

SfxPoolItem* XFormTextHideFormItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextHideFormItem(rIn);
}

// --- SetItems ---

TYPEINIT1(XLineAttrSetItem, SfxSetItem);

/// a line attribute set item
XLineAttrSetItem::XLineAttrSetItem( SfxItemSet* pItemSet ) :
    SfxSetItem( XATTRSET_LINE, pItemSet)
{
}

XLineAttrSetItem::XLineAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_LINE,
        new SfxItemSet( *pItemPool, XATTR_LINE_FIRST, XATTR_LINE_LAST))
{
}

XLineAttrSetItem::XLineAttrSetItem( const XLineAttrSetItem& rLineAttr ) :
    SfxSetItem( rLineAttr )
{
}

XLineAttrSetItem::XLineAttrSetItem( const XLineAttrSetItem& rLineAttr,
                                    SfxItemPool* pItemPool) :
    SfxSetItem( rLineAttr, pItemPool )
{
}

SfxPoolItem* XLineAttrSetItem::Clone( SfxItemPool* pPool ) const
{
    return new XLineAttrSetItem( *this, pPool );
}

/// create a set item out of a stream
SfxPoolItem* XLineAttrSetItem::Create( SvStream& rStream, sal_uInt16 /*nVersion*/) const
{
    SfxItemSet *pSet2 = new SfxItemSet( *GetItemSet().GetPool(),
                                    XATTR_LINE_FIRST, XATTR_LINE_LAST);
    pSet2->Load( rStream );
    return new XLineAttrSetItem( pSet2 );
}

/// save a set item in a stream
SvStream& XLineAttrSetItem::Store( SvStream& rStream, sal_uInt16 nItemVersion ) const
{
    return SfxSetItem::Store( rStream, nItemVersion );
}


TYPEINIT1(XFillAttrSetItem, SfxSetItem);

/// fill attribute set item
XFillAttrSetItem::XFillAttrSetItem( SfxItemSet* pItemSet ) :
    SfxSetItem( XATTRSET_FILL, pItemSet)
{
}

XFillAttrSetItem::XFillAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_FILL,
        new SfxItemSet( *pItemPool, XATTR_FILL_FIRST, XATTR_FILL_LAST))
{
}

XFillAttrSetItem::XFillAttrSetItem( const XFillAttrSetItem& rFillAttr ) :
    SfxSetItem( rFillAttr )
{
}

XFillAttrSetItem::XFillAttrSetItem( const XFillAttrSetItem& rFillAttr,
                                    SfxItemPool* pItemPool ) :
    SfxSetItem( rFillAttr, pItemPool )
{
}

SfxPoolItem* XFillAttrSetItem::Clone( SfxItemPool* pPool ) const
{
    return new XFillAttrSetItem( *this, pPool );
}

/// create a set item out of a stream
SfxPoolItem* XFillAttrSetItem::Create( SvStream& rStream, sal_uInt16 /*nVersion*/) const
{
    SfxItemSet *pSet2 = new SfxItemSet( *GetItemSet().GetPool(),
                                    XATTR_FILL_FIRST, XATTR_FILL_LAST);
    pSet2->Load( rStream );
    return new XFillAttrSetItem( pSet2 );
}

/// save a set item in a stream
SvStream& XFillAttrSetItem::Store( SvStream& rStream, sal_uInt16 nItemVersion ) const
{
    return SfxSetItem::Store( rStream, nItemVersion );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
