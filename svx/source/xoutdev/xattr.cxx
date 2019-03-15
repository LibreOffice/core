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

#include <sal/config.h>

#include <utility>

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
#include <o3tl/any.hxx>
#include <svl/itempool.hxx>
#include <editeng/memberids.h>
#include <tools/stream.hxx>
#include <tools/mapunit.hxx>
#include <osl/diagnose.h>

#include <svx/unoapi.hxx>
#include <svl/style.hxx>

#include <tools/bigint.hxx>
#include <svl/itemset.hxx>
#include <svx/strings.hrc>
#include <svx/xattr.hxx>
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>
#include <svx/xdef.hxx>
#include <svx/unomid.hxx>
#include <svx/svdmodel.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/gradient.hxx>

#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

static long ScaleMetricValue( long nVal, long nMul, long nDiv )
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

/** this static checks if the given NameOrIndex item has a unique name for its value.
    The returned String is a unique name for an item with this value in both given pools.
    Argument pPool2 can be null.
    If returned string equals NameOrIndex->GetName(), the name was already unique.
*/
OUString NameOrIndex::CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, SvxCompareValueFunc pCompareValueFunc, const char* pPrefixResId, const XPropertyListRef &pDefaults )
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
                // value it's ok to set it
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
        const OUString aUser(SvxResId(pPrefixResId) + " ");

        if( pDefaults.get() )
        {
            const int nCount = pDefaults->Count();
            int nIndex;
            for( nIndex = 0; nIndex < nCount; nIndex++ )
            {
                const XPropertyEntry* pEntry = pDefaults->Get(nIndex);
                if( pEntry )
                {
                    bool bFound = false;

                    switch( nWhich )
                    {
                    case XATTR_FILLBITMAP:
                    {
                        const GraphicObject& rGraphicObjectA(static_cast<const XFillBitmapItem*>(pCheckItem)->GetGraphicObject());
                        const GraphicObject& rGraphicObjectB(static_cast<const XBitmapEntry*>(pEntry)->GetGraphicObject());

                        bFound = (rGraphicObjectA == rGraphicObjectB);
                        break;
                    }
                    case XATTR_LINEDASH:
                        bFound = static_cast<const XLineDashItem*>(pCheckItem)->GetDashValue() == static_cast<const XDashEntry*>(pEntry)->GetDash();
                        break;
                    case XATTR_LINESTART:
                        bFound = static_cast<const XLineStartItem*>(pCheckItem)->GetLineStartValue() == static_cast<const XLineEndEntry*>(pEntry)->GetLineEnd();
                        break;
                    case XATTR_LINEEND:
                        bFound = static_cast<const XLineEndItem*>(pCheckItem)->GetLineEndValue() == static_cast<const XLineEndEntry*>(pEntry)->GetLineEnd();
                        break;
                    case XATTR_FILLGRADIENT:
                        bFound = static_cast<const XFillGradientItem*>(pCheckItem)->GetGradientValue() == static_cast<const XGradientEntry*>(pEntry)->GetGradient();
                        break;
                    case XATTR_FILLHATCH:
                        bFound = static_cast<const XFillHatchItem*>(pCheckItem)->GetHatchValue() == static_cast<const XHatchEntry*>(pEntry)->GetHatch();
                        break;
                    }

                    if( bFound )
                    {
                        aUniqueName = pEntry->GetName();
                        break;
                    }
                    else
                    {
                        const OUString& aEntryName = pEntry->GetName();
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
            aUniqueName = aUser + OUString::number( nUserIndex );
        }
    }

    return aUniqueName;
}

void NameOrIndex::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("NameOrIndex"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("isIndex"), BAD_CAST(OString::boolean(IsIndex()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(nPalIndex).getStr()));
    xmlTextWriterEndElement(pWriter);
}

SfxPoolItem* XColorItem::CreateDefault() { return new XColorItem; }

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

SfxPoolItem* XColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XColorItem(*this);
}

bool XColorItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
            static_cast<const XColorItem&>(rItem).aColor == aColor );
}

const Color& XColorItem::GetColorValue() const
{
    assert(!IsIndex());
    return aColor;

}

bool XColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetColorValue().GetRGBColor();
    return true;
}

bool XColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetColorValue( Color(nValue) );

    return true;
}

// --- line attributes ---


SfxPoolItem* XLineStyleItem::CreateDefault() { return new XLineStyleItem; }

XLineStyleItem::XLineStyleItem(css::drawing::LineStyle eTheLineStyle) :
    SfxEnumItem(XATTR_LINESTYLE, eTheLineStyle)
{
}

SfxPoolItem* XLineStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStyleItem( *this );
}

bool XLineStyleItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();

    const char* pId = nullptr;

    switch( GetValue() )
    {
        case css::drawing::LineStyle_NONE:
            pId = RID_SVXSTR_INVISIBLE;
            break;
        case css::drawing::LineStyle_SOLID:
            pId = RID_SVXSTR_SOLID;
            break;
        default: break;
    }

    if (pId)
        rText = SvxResId(pId);
    return true;
}

bool XLineStyleItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    css::drawing::LineStyle eLS = GetValue();
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
        eLS = static_cast<css::drawing::LineStyle>(nLS);
    }

    SetValue( eLS );
    return true;
}

sal_uInt16 XLineStyleItem::GetValueCount() const
{
    return 3;
}

XDash::XDash(css::drawing::DashStyle eTheDash, sal_uInt16 nTheDots, sal_uInt32 nTheDotLen,
             sal_uInt16 nTheDashes, sal_uInt32 nTheDashLen, sal_uInt32 nTheDistance) :
    eDash(eTheDash),
    nDotLen(nTheDotLen),
    nDots(nTheDots),
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
static const double SMALLEST_DASH_WIDTH(26.95);

double XDash::CreateDotDashArray(::std::vector< double >& rDotDashArray, double fLineWidth) const
{
    double fFullDotDashLen(0.0);
    const sal_uInt16 nNumDotDashArray = (GetDots() + GetDashes()) * 2;
    rDotDashArray.resize( nNumDotDashArray, 0.0 );
    sal_uInt16 a;
    sal_uInt16 nIns(0);
    double fDashDotDistance = static_cast<double>(GetDistance());
    double fSingleDashLen = static_cast<double>(GetDashLen());
    double fSingleDotLen = static_cast<double>(GetDotLen());

    if (fLineWidth == 0.0)
        fLineWidth = SMALLEST_DASH_WIDTH;

    if(GetDashStyle() == css::drawing::DashStyle_RECTRELATIVE || GetDashStyle() == css::drawing::DashStyle_ROUNDRELATIVE)
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
                // dash as distance
                fDashDotDistance *= fFactor;
            }
            else
            {
                // dot as distance
                fDashDotDistance = fLineWidth;
            }
        }
    }
    else
    {
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
                if(fSingleDashLen < fLineWidth)
                {
                    fSingleDashLen = fLineWidth;
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
                if(fSingleDotLen < fLineWidth)
                {
                    fSingleDotLen = fLineWidth;
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
                if(fDashDotDistance < fLineWidth)
                {
                    fDashDotDistance = fLineWidth;
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

SfxPoolItem* XLineDashItem::CreateDefault() {return new XLineDashItem;}

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

XLineDashItem::XLineDashItem(const XDash& rTheDash)
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

bool XLineDashItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText = GetName();
    return true;
}

bool XLineDashItem::HasMetrics() const
{
    return true;
}

void XLineDashItem::ScaleMetrics(long nMul, long nDiv)
{
    aDash.SetDotLen( ScaleMetricValue( aDash.GetDotLen(), nMul, nDiv ) );
    aDash.SetDashLen( ScaleMetricValue( aDash.GetDashLen(), nMul, nDiv ) );
    aDash.SetDistance( ScaleMetricValue( aDash.GetDistance(), nMul, nDiv ) );
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
            aLineDash.Style = static_cast<css::drawing::DashStyle>(static_cast<sal_uInt16>(rXD.GetDashStyle()));
            aLineDash.Dots = rXD.GetDots();
            aLineDash.DotLen = rXD.GetDotLen();
            aLineDash.Dashes = rXD.GetDashes();
            aLineDash.DashLen = rXD.GetDashLen();
            aLineDash.Distance = rXD.GetDistance();

            aPropSeq[0].Name    = "Name";
            aPropSeq[0].Value   <<= SvxUnogetApiNameForItem(Which(), GetName());
            aPropSeq[1].Name    = "LineDash";
            aPropSeq[1].Value   <<= aLineDash;
            rVal <<= aPropSeq;
            break;
        }

        case MID_NAME:
        {
            rVal <<= SvxUnogetApiNameForItem(Which(), GetName());
            break;
        }

        case MID_LINEDASH:
        {
            const XDash& rXD = GetDashValue();

            css::drawing::LineDash aLineDash;

            aLineDash.Style = static_cast<css::drawing::DashStyle>(static_cast<sal_uInt16>(rXD.GetDashStyle()));
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
            rVal <<= static_cast<css::drawing::DashStyle>(static_cast<sal_Int16>(rXD.GetDashStyle()));
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

                    aXDash.SetDashStyle(static_cast<css::drawing::DashStyle>(static_cast<sal_uInt16>(aLineDash.Style)));
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

            aXDash.SetDashStyle(static_cast<css::drawing::DashStyle>(static_cast<sal_uInt16>(aLineDash.Style)));
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
            aXDash.SetDashStyle(static_cast<css::drawing::DashStyle>(static_cast<sal_uInt16>(nVal)));

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
            sal_uInt32 nVal = 0;
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

std::unique_ptr<XLineDashItem> XLineDashItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const OUString aUniqueName = NameOrIndex::CheckNamedItem(
                this, XATTR_LINEDASH, &pModel->GetItemPool(),
                XLineDashItem::CompareValueFunc, RID_SVXSTR_DASH11,
                pModel->GetPropertyList( XPropertyListType::Dash ) );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
            return std::make_unique<XLineDashItem>( aUniqueName, aDash );
    }

    return nullptr;
}

SfxPoolItem* XLineWidthItem::CreateDefault() {return new XLineWidthItem;}

XLineWidthItem::XLineWidthItem(long nWidth) :
    SfxMetricItem(XATTR_LINEWIDTH, nWidth)
{
}

SfxPoolItem* XLineWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineWidthItem(*this);
}

bool XLineWidthItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    rText = GetMetricText( static_cast<long>(GetValue()),
                            eCoreUnit, ePresUnit, &rIntl) +
            " " + EditResId( GetMetricId( ePresUnit) );
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

SfxPoolItem* XLineColorItem::CreateDefault() { return new XLineColorItem; }

XLineColorItem::XLineColorItem(sal_Int32 nIndex, const Color& rTheColor) :
    XColorItem(XATTR_LINECOLOR, nIndex, rTheColor)
{
}

XLineColorItem::XLineColorItem(const OUString& rName, const Color& rTheColor) :
    XColorItem(XATTR_LINECOLOR, rName, rTheColor)
{
}

SfxPoolItem* XLineColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineColorItem(*this);
}

bool XLineColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText = GetName();
    return true;
}

bool XLineColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetColorValue().GetRGBColor();
    return true;
}

bool XLineColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetColorValue( Color(nValue) );
    return true;
}


SfxPoolItem* XLineStartItem::CreateDefault() {return new XLineStartItem;}

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

XLineStartItem::XLineStartItem(const basegfx::B2DPolyPolygon& rPolyPolygon)
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

bool XLineStartItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
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
        rVal <<= SvxUnogetApiNameForItem(Which(), GetName());
    }
    else
    {
        css::drawing::PolyPolygonBezierCoords aBezier;
        basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords( maPolyPolygon, aBezier );
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

        if( rVal.hasValue() )
        {
            auto pCoords = o3tl::tryAccess<css::drawing::PolyPolygonBezierCoords>(
                rVal);
            if( !pCoords )
                return false;

            if( pCoords->Coordinates.getLength() > 0 )
            {
                maPolyPolygon = basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon( *pCoords );
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
std::unique_ptr<XLineStartItem> XLineStartItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        std::unique_ptr<XLineStartItem> pTempItem;
        const XLineStartItem* pLineStartItem = this;

        OUString aUniqueName( GetName() );

        if( !maPolyPolygon.count() )
        {
            // if the polygon is empty, check if the name is empty
            if( aUniqueName.isEmpty() )
                return nullptr;

            // force empty name for empty polygons
            return std::make_unique<XLineStartItem>( "", maPolyPolygon );
        }

        if( maPolyPolygon.count() > 1 )
        {
            // check if the polygon is closed
            if(!maPolyPolygon.isClosed())
            {
                // force a closed polygon
                basegfx::B2DPolyPolygon aNew(maPolyPolygon);
                aNew.setClosed(true);
                pTempItem.reset(new XLineStartItem( aUniqueName, aNew ));
                pLineStartItem = pTempItem.get();
            }
        }

        bool bForceNew = false;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        sal_uInt32 nCount, nSurrogate;

        const SfxItemPool& rPool1 = pModel->GetItemPool();
        if (!aUniqueName.isEmpty())
        {
            nCount = rPool1.GetItemCount2(XATTR_LINESTART);

            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = rPool1.GetItem2(XATTR_LINESTART, nSurrogate);

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value it's ok to set it
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
                nCount = rPool1.GetItemCount2(XATTR_LINEEND);

                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = rPool1.GetItem2(XATTR_LINEEND, nSurrogate);

                    if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value it's ok to set it
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

        const SfxItemPool* pPool2 = pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : nullptr;
        if( !aUniqueName.isEmpty() && pPool2)
        {
            nCount = pPool2->GetItemCount2( XATTR_LINESTART );
            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = pPool2->GetItem2( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value it's ok to set it
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
                    const XLineEndItem* pItem = pPool2->GetItem2( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value it's ok to set it
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
            const OUString aUser(SvxResId(RID_SVXSTR_LINEEND));

            nCount = rPool1.GetItemCount2(XATTR_LINESTART);
            sal_uInt32 nSurrogate2;

            for (nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++)
            {
                const XLineStartItem* pItem = rPool1.GetItem2(XATTR_LINESTART, nSurrogate2);

                if (pItem && !pItem->GetName().isEmpty())
                {
                    if (!bForceNew && pItem->GetLineStartValue() == pLineStartItem->GetLineStartValue())
                    {
                        aUniqueName = pItem->GetName();
                        bFoundExisting = true;
                        break;
                    }

                    if (pItem->GetName().startsWith(aUser))
                    {
                        sal_Int32 nThisIndex = pItem->GetName().copy(aUser.getLength()).toInt32();
                        if (nThisIndex >= nUserIndex)
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }

            nCount = rPool1.GetItemCount2(XATTR_LINEEND);
            for (nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++)
            {
                const XLineEndItem* pItem = rPool1.GetItem2(XATTR_LINEEND, nSurrogate2);

                if (pItem && !pItem->GetName().isEmpty())
                {
                    if (!bForceNew && pItem->GetLineEndValue() == pLineStartItem->GetLineStartValue())
                    {
                        aUniqueName = pItem->GetName();
                        bFoundExisting = true;
                        break;
                    }

                    if (pItem->GetName().startsWith(aUser))
                    {
                        sal_Int32 nThisIndex = pItem->GetName().copy(aUser.getLength()).toInt32();
                        if (nThisIndex >= nUserIndex)
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }

            if( !bFoundExisting )
            {
                aUniqueName = aUser + " " + OUString::number( nUserIndex );
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
                return std::make_unique<XLineStartItem>( aUniqueName, maPolyPolygon );
            }
        }
    }

    return nullptr;
}

SfxPoolItem* XLineEndItem::CreateDefault() {return new XLineEndItem;}

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

XLineEndItem::XLineEndItem(const basegfx::B2DPolyPolygon& rPolyPolygon)
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


/** this function searches in both the models pool and the styles pool for XLineStartItem
    and XLineEndItem with the same value or name and returns an item with the value of
    this item and a unique name for an item with this value. */
std::unique_ptr<XLineEndItem> XLineEndItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        std::unique_ptr<XLineEndItem> pTempItem;
        const XLineEndItem* pLineEndItem = this;

        OUString aUniqueName( GetName() );

        if( !maPolyPolygon.count() )
        {
            // if the polygon is empty, check if the name is empty
            if( aUniqueName.isEmpty() )
                return nullptr;

            // force empty name for empty polygons
            return std::make_unique<XLineEndItem>( "", maPolyPolygon );
        }

        if( maPolyPolygon.count() > 1 )
        {
            // check if the polygon is closed
            if(!maPolyPolygon.isClosed())
            {
                // force a closed polygon
                basegfx::B2DPolyPolygon aNew(maPolyPolygon);
                aNew.setClosed(true);
                pTempItem.reset(new XLineEndItem( aUniqueName, aNew ));
                pLineEndItem = pTempItem.get();
            }
        }

        bool bForceNew = false;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        sal_uInt16 nCount, nSurrogate;

        const SfxItemPool& rPool1 = pModel->GetItemPool();
        if (!aUniqueName.isEmpty())
        {
            nCount = rPool1.GetItemCount2(XATTR_LINESTART);

            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = rPool1.GetItem2(XATTR_LINESTART, nSurrogate);

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value it's ok to set it
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
                nCount = rPool1.GetItemCount2(XATTR_LINEEND);

                for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
                {
                    const XLineEndItem* pItem = rPool1.GetItem2(XATTR_LINEEND, nSurrogate);

                    if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value it's ok to set it
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

        const SfxItemPool* pPool2 = pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : nullptr;
        if( !aUniqueName.isEmpty() && pPool2)
        {
            nCount = pPool2->GetItemCount2( XATTR_LINESTART );
            for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                const XLineStartItem* pItem = pPool2->GetItem2( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value it's ok to set it
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
                    const XLineEndItem* pItem = pPool2->GetItem2( XATTR_LINEEND, nSurrogate );

                    if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                    {
                        // if there is already an item with the same name and the same
                        // value it's ok to set it
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
            const OUString aUser(SvxResId(RID_SVXSTR_LINEEND));

            nCount = rPool1.GetItemCount2(XATTR_LINESTART);
            sal_uInt32 nSurrogate2;

            for (nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++)
            {
                const XLineStartItem* pItem = rPool1.GetItem2(XATTR_LINESTART, nSurrogate2);

                if (pItem && !pItem->GetName().isEmpty())
                {
                    if (!bForceNew && pItem->GetLineStartValue() == pLineEndItem->GetLineEndValue())
                    {
                        aUniqueName = pItem->GetName();
                        bFoundExisting = true;
                        break;
                    }

                    if (pItem->GetName().startsWith(aUser))
                    {
                        sal_Int32 nThisIndex = pItem->GetName().copy(aUser.getLength()).toInt32();
                        if (nThisIndex >= nUserIndex)
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }

            nCount = rPool1.GetItemCount2(XATTR_LINEEND);
            for (nSurrogate2 = 0; nSurrogate2 < nCount; nSurrogate2++)
            {
                const XLineEndItem* pItem = rPool1.GetItem2(XATTR_LINEEND, nSurrogate2);

                if (pItem && !pItem->GetName().isEmpty())
                {
                    if (!bForceNew && pItem->GetLineEndValue() == pLineEndItem->GetLineEndValue())
                    {
                        aUniqueName = pItem->GetName();
                        bFoundExisting = true;
                        break;
                    }

                    if (pItem->GetName().startsWith(aUser))
                    {
                        sal_Int32 nThisIndex = pItem->GetName().copy(aUser.getLength()).toInt32();
                        if (nThisIndex >= nUserIndex)
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }

            if( !bFoundExisting )
            {
                aUniqueName = aUser + " " + OUString::number( nUserIndex );
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
                return std::make_unique<XLineEndItem>( aUniqueName, maPolyPolygon );
            }
        }
    }

    return nullptr;
}

bool XLineEndItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
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
        rVal <<= SvxUnogetApiNameForItem(Which(), GetName());
    }
    else
    {
        css::drawing::PolyPolygonBezierCoords aBezier;
        basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords( maPolyPolygon, aBezier );
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

        if( rVal.hasValue() )
        {
            auto pCoords = o3tl::tryAccess<css::drawing::PolyPolygonBezierCoords>(
                rVal);
            if( !pCoords )
                return false;

            if( pCoords->Coordinates.getLength() > 0 )
            {
                maPolyPolygon = basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon( *pCoords );
                // #i72807# close line start/end polygons hard
                // maPolyPolygon.setClosed(true);
            }
        }
    }

    return true;
}

XLineStartWidthItem::XLineStartWidthItem(long nWidth) :
    SfxMetricItem(XATTR_LINESTARTWIDTH, nWidth)
{
}

SfxPoolItem* XLineStartWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStartWidthItem(*this);
}

bool XLineStartWidthItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    rText = GetMetricText( static_cast<long>(GetValue()),
                            eCoreUnit, ePresUnit, &rIntl) +
            " " + EditResId( GetMetricId( ePresUnit) );
    return true;
}

bool XLineStartWidthItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
    return true;
}

bool XLineStartWidthItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue( nValue );
    return true;
}

XLineEndWidthItem::XLineEndWidthItem(long nWidth) :
   SfxMetricItem(XATTR_LINEENDWIDTH, nWidth)
{
}

SfxPoolItem* XLineEndWidthItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineEndWidthItem(*this);
}

bool XLineEndWidthItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    rText = GetMetricText( static_cast<long>(GetValue()),
                            eCoreUnit, ePresUnit, &rIntl) +
            " " + EditResId( GetMetricId( ePresUnit) );
    return true;
}

bool XLineEndWidthItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
    return true;
}

bool XLineEndWidthItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue( nValue );
    return true;
}

XLineStartCenterItem::XLineStartCenterItem(bool bStartCenter) :
    SfxBoolItem(XATTR_LINESTARTCENTER, bStartCenter)
{
}

SfxPoolItem* XLineStartCenterItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineStartCenterItem(*this);
}

bool XLineStartCenterItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText = SvxResId(GetValue() ? RID_SVXSTR_CENTERED : RID_SVXSTR_NOTCENTERED);
    return true;
}

bool XLineStartCenterItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    bool bValue = GetValue();
    rVal <<= bValue;
    return true;
}

bool XLineStartCenterItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    auto b = o3tl::tryAccess<bool>(rVal);
    if( !b )
        return false;

    SetValue( *b );
    return true;
}

XLineEndCenterItem::XLineEndCenterItem(bool bEndCenter) :
    SfxBoolItem(XATTR_LINEENDCENTER, bEndCenter)
{
}

SfxPoolItem* XLineEndCenterItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineEndCenterItem(*this);
}

bool XLineEndCenterItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText = SvxResId(GetValue() ? RID_SVXSTR_CENTERED : RID_SVXSTR_NOTCENTERED);
    return true;
}

bool XLineEndCenterItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    bool bValue = GetValue();
    rVal <<= bValue;
    return true;
}

bool XLineEndCenterItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    auto b = o3tl::tryAccess<bool>(rVal);
    if( !b )
        return false;

    SetValue( *b );
    return true;
}

// --- fill attributes ---


SfxPoolItem* XFillStyleItem::CreateDefault() { return new XFillStyleItem; }

XFillStyleItem::XFillStyleItem(drawing::FillStyle eFillStyle) :
    SfxEnumItem(XATTR_FILLSTYLE, eFillStyle)
{
}

SfxPoolItem* XFillStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillStyleItem( *this );
}

bool XFillStyleItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();

    const char* pId = nullptr;

    switch( GetValue() )
    {
        case drawing::FillStyle_NONE:
            pId = RID_SVXSTR_INVISIBLE;
            break;
        case drawing::FillStyle_SOLID:
            pId = RID_SVXSTR_SOLID;
            break;
        case drawing::FillStyle_GRADIENT:
            pId = RID_SVXSTR_GRADIENT;
            break;
        case drawing::FillStyle_HATCH:
            pId = RID_SVXSTR_HATCH;
            break;
        case drawing::FillStyle_BITMAP:
            pId = RID_SVXSTR_BITMAP;
            break;
        default: break;
    }

    if (pId)
        rText = SvxResId(pId);
    return true;
}

sal_uInt16 XFillStyleItem::GetValueCount() const
{
    return 5;
}

bool XFillStyleItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    css::drawing::FillStyle eFS = GetValue();

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
        eFS = static_cast<css::drawing::FillStyle>(nFS);
    }

    SetValue( eFS );

    return true;
}

void XFillStyleItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("XFillStyleItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(static_cast<sal_Int16>(GetValue())).getStr()));

    OUString aPresentation;
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    GetPresentation(SfxItemPresentation::Nameless, MapUnit::Map100thMM, MapUnit::Map100thMM, aPresentation, aIntlWrapper);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    xmlTextWriterEndElement(pWriter);
}


SfxPoolItem* XFillColorItem::CreateDefault() { return new XFillColorItem; }

XFillColorItem::XFillColorItem(sal_Int32 nIndex, const Color& rTheColor) :
    XColorItem(XATTR_FILLCOLOR, nIndex, rTheColor)
{
}

XFillColorItem::XFillColorItem(const OUString& rName, const Color& rTheColor) :
    XColorItem(XATTR_FILLCOLOR, rName, rTheColor)
{
}

SfxPoolItem* XFillColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillColorItem(*this);
}

bool XFillColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText = GetName();
    return true;
}

bool XFillColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetColorValue().GetRGBColor();

    return true;
}

bool XFillColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue ))
        return false;

    SetColorValue( Color(nValue) );
    return true;
}

void XFillColorItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("XFillColorItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(GetColorValue().AsRGBHexString().toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
}

XSecondaryFillColorItem::XSecondaryFillColorItem(const OUString& rName, const Color& rTheColor) :
    XColorItem(XATTR_SECONDARYFILLCOLOR, rName, rTheColor)
{
}

SfxPoolItem* XSecondaryFillColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XSecondaryFillColorItem(*this);
}

sal_uInt16 XSecondaryFillColorItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/ ) const
{
    return 2;
}

bool XSecondaryFillColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText = GetName();
    return true;
}

XGradient::XGradient() :
    eStyle( css::awt::GradientStyle_LINEAR ),
    aStartColor( COL_BLACK ),
    aEndColor( COL_WHITE ),
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


SfxPoolItem* XFillGradientItem::CreateDefault() { return new XFillGradientItem; }

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
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
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
            aGradient2.Style = aXGradient.GetGradientStyle();
            aGradient2.StartColor = static_cast<sal_Int32>(aXGradient.GetStartColor());
            aGradient2.EndColor = static_cast<sal_Int32>(aXGradient.GetEndColor());
            aGradient2.Angle = static_cast<short>(aXGradient.GetAngle());
            aGradient2.Border = aXGradient.GetBorder();
            aGradient2.XOffset = aXGradient.GetXOffset();
            aGradient2.YOffset = aXGradient.GetYOffset();
            aGradient2.StartIntensity = aXGradient.GetStartIntens();
            aGradient2.EndIntensity = aXGradient.GetEndIntens();
            aGradient2.StepCount = aXGradient.GetSteps();

            aPropSeq[0].Name    = "Name";
            aPropSeq[0].Value   <<= SvxUnogetApiNameForItem(Which(), GetName());
            aPropSeq[1].Name    = "FillGradient";
            aPropSeq[1].Value   <<= aGradient2;
            rVal <<= aPropSeq;
            break;
        }

        case MID_FILLGRADIENT:
        {
            const XGradient& aXGradient = GetGradientValue();
            css::awt::Gradient aGradient2;

            aGradient2.Style = aXGradient.GetGradientStyle();
            aGradient2.StartColor = static_cast<sal_Int32>(aXGradient.GetStartColor());
            aGradient2.EndColor = static_cast<sal_Int32>(aXGradient.GetEndColor());
            aGradient2.Angle = static_cast<short>(aXGradient.GetAngle());
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
            rVal <<= SvxUnogetApiNameForItem(Which(), GetName());
            break;
        }

        case MID_GRADIENT_STYLE: rVal <<= static_cast<sal_Int16>(GetGradientValue().GetGradientStyle()); break;
        case MID_GRADIENT_STARTCOLOR: rVal <<= GetGradientValue().GetStartColor(); break;
        case MID_GRADIENT_ENDCOLOR: rVal <<= GetGradientValue().GetEndColor(); break;
        case MID_GRADIENT_ANGLE: rVal <<= static_cast<sal_Int16>(GetGradientValue().GetAngle()); break;
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

                    aXGradient.SetGradientStyle( aGradient2.Style );
                    aXGradient.SetStartColor( Color(aGradient2.StartColor) );
                    aXGradient.SetEndColor( Color(aGradient2.EndColor) );
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

            aXGradient.SetGradientStyle( aGradient2.Style );
            aXGradient.SetStartColor( Color(aGradient2.StartColor) );
            aXGradient.SetEndColor( Color(aGradient2.EndColor) );
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
                aXGradient.SetStartColor( Color(nVal) );
            else
                aXGradient.SetEndColor( Color(nVal) );
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
                    aXGradient.SetGradientStyle( static_cast<css::awt::GradientStyle>(nVal) ); break;
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

std::unique_ptr<XFillGradientItem> XFillGradientItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const OUString aUniqueName = NameOrIndex::CheckNamedItem(
                this, Which(), &pModel->GetItemPool(),
                XFillGradientItem::CompareValueFunc, RID_SVXSTR_GRADIENT,
                pModel->GetPropertyList( XPropertyListType::Gradient ) );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
            return std::make_unique<XFillGradientItem>( aUniqueName, aGradient, Which() );
    }

    return nullptr;
}


SfxPoolItem* XFillFloatTransparenceItem::CreateDefault() { return new XFillFloatTransparenceItem; }

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

XFillFloatTransparenceItem::XFillFloatTransparenceItem(const XGradient& rTheGradient, bool bEnable )
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
                                                                    MapUnit eCoreUnit, MapUnit ePresUnit,
                                                                    OUString& rText,
                                                                    const IntlWrapper& rIntlWrapper ) const
{
    return XFillGradientItem::GetPresentation( ePres, eCoreUnit, ePresUnit, rText, rIntlWrapper );
}

bool XFillFloatTransparenceItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    return  static_cast<const XFillFloatTransparenceItem*>(p1)->IsEnabled() == static_cast<const XFillFloatTransparenceItem*>(p2)->IsEnabled() &&
            static_cast<const XFillFloatTransparenceItem*>(p1)->GetGradientValue()  == static_cast<const XFillFloatTransparenceItem*>(p2)->GetGradientValue();
}

std::unique_ptr<XFillFloatTransparenceItem> XFillFloatTransparenceItem::checkForUniqueItem( SdrModel* pModel ) const
{
    // #85953# unique name only necessary when enabled
    if(IsEnabled())
    {
        if( pModel )
        {
            const OUString aUniqueName = NameOrIndex::CheckNamedItem( this,
                                                                    XATTR_FILLFLOATTRANSPARENCE,
                                                                    &pModel->GetItemPool(),
                                                                    XFillFloatTransparenceItem::CompareValueFunc,
                                                                    RID_SVXSTR_TRASNGR0,
                                                                    XPropertyListRef() );

            // if the given name is not valid, replace it!
            if( aUniqueName != GetName() )
            {
                return std::make_unique<XFillFloatTransparenceItem>( aUniqueName, GetGradientValue(), true );
            }
        }
    }
    else
    {
        // #85953# if disabled, force name to empty string
        if( !GetName().isEmpty() )
        {
            return std::make_unique<XFillFloatTransparenceItem>(OUString(), GetGradientValue(), false);
        }
    }

    return nullptr;
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


SfxPoolItem* XFillHatchItem::CreateDefault() { return new XFillHatchItem; }

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

XFillHatchItem::XFillHatchItem(const XHatch& rTheHatch)
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

bool XFillHatchItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText = GetName();
    return true;
}

bool XFillHatchItem::HasMetrics() const
{
    return true;
}

void XFillHatchItem::ScaleMetrics(long nMul, long nDiv)
{
    aHatch.SetDistance( ScaleMetricValue( aHatch.GetDistance(), nMul, nDiv ) );
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

            aUnoHatch.Style = aHatch.GetHatchStyle();
            aUnoHatch.Color = sal_Int32(aHatch.GetColor());
            aUnoHatch.Distance = aHatch.GetDistance();
            aUnoHatch.Angle = aHatch.GetAngle();

            aPropSeq[0].Name    = "Name";
            aPropSeq[0].Value   <<= SvxUnogetApiNameForItem(Which(), GetName());
            aPropSeq[1].Name    = "FillHatch";
            aPropSeq[1].Value   <<= aUnoHatch;
            rVal <<= aPropSeq;
            break;
        }

        case MID_FILLHATCH:
        {
            css::drawing::Hatch aUnoHatch;

            aUnoHatch.Style = aHatch.GetHatchStyle();
            aUnoHatch.Color = sal_Int32(aHatch.GetColor());
            aUnoHatch.Distance = aHatch.GetDistance();
            aUnoHatch.Angle = aHatch.GetAngle();
            rVal <<= aUnoHatch;
            break;
        }

        case MID_NAME:
        {
            rVal <<= SvxUnogetApiNameForItem(Which(), GetName());
            break;
        }

        case MID_HATCH_STYLE:
            rVal <<= aHatch.GetHatchStyle(); break;
        case MID_HATCH_COLOR:
            rVal <<= aHatch.GetColor(); break;
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
                    aHatch.SetHatchStyle( aUnoHatch.Style );
                    aHatch.SetColor( Color(aUnoHatch.Color) );
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

            aHatch.SetHatchStyle( aUnoHatch.Style );
            aHatch.SetColor( Color(aUnoHatch.Color) );
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
            aHatch.SetHatchStyle( static_cast<css::drawing::HatchStyle>(nVal) );
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
                aHatch.SetColor( Color(nVal) );
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

std::unique_ptr<XFillHatchItem> XFillHatchItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const OUString aUniqueName = NameOrIndex::CheckNamedItem(
                this, XATTR_FILLHATCH, &pModel->GetItemPool(),
                XFillHatchItem::CompareValueFunc, RID_SVXSTR_HATCH10,
                pModel->GetPropertyList( XPropertyListType::Hatch ) );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
            return std::make_unique<XFillHatchItem>( aUniqueName, aHatch );
    }

    return nullptr;
}

// --- form text attributes ---


SfxPoolItem* XFormTextStyleItem::CreateDefault() { return new XFormTextStyleItem; }

XFormTextStyleItem::XFormTextStyleItem(XFormTextStyle eTheStyle) :
    SfxEnumItem(XATTR_FORMTXTSTYLE, eTheStyle)
{
}

SfxPoolItem* XFormTextStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextStyleItem( *this );
}

sal_uInt16 XFormTextStyleItem::GetValueCount() const
{
    return 5;
}

bool XFormTextStyleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<sal_Int32>(GetValue());
    return true;
}

bool XFormTextStyleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(static_cast<XFormTextStyle>(nValue));

    return true;
}


SfxPoolItem* XFormTextAdjustItem::CreateDefault() { return new XFormTextAdjustItem; }

XFormTextAdjustItem::XFormTextAdjustItem(XFormTextAdjust eTheAdjust) :
    SfxEnumItem(XATTR_FORMTXTADJUST, eTheAdjust)
{
}

SfxPoolItem* XFormTextAdjustItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextAdjustItem( *this );
}

sal_uInt16 XFormTextAdjustItem::GetValueCount() const
{
    return 4;
}

bool XFormTextAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<sal_Int32>(GetValue());
    return true;
}

bool XFormTextAdjustItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(static_cast<XFormTextAdjust>(nValue));

    return true;
}


SfxPoolItem* XFormTextDistanceItem::CreateDefault() { return new XFormTextDistanceItem; }

XFormTextDistanceItem::XFormTextDistanceItem(long nDist) :
    SfxMetricItem(XATTR_FORMTXTDISTANCE, nDist)
{
}

SfxPoolItem* XFormTextDistanceItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextDistanceItem(*this);
}

SfxPoolItem* XFormTextStartItem::CreateDefault() { return new XFormTextStartItem; }

XFormTextStartItem::XFormTextStartItem(long nStart) :
    SfxMetricItem(XATTR_FORMTXTSTART, nStart)
{
}

SfxPoolItem* XFormTextStartItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextStartItem(*this);
}


SfxPoolItem* XFormTextMirrorItem::CreateDefault() { return new XFormTextMirrorItem; }

XFormTextMirrorItem::XFormTextMirrorItem(bool bMirror) :
    SfxBoolItem(XATTR_FORMTXTMIRROR, bMirror)
{
}

SfxPoolItem* XFormTextMirrorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextMirrorItem(*this);
}


SfxPoolItem* XFormTextOutlineItem::CreateDefault() { return new XFormTextOutlineItem; }

XFormTextOutlineItem::XFormTextOutlineItem(bool bOutline) :
    SfxBoolItem(XATTR_FORMTXTOUTLINE, bOutline)
{
}

SfxPoolItem* XFormTextOutlineItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextOutlineItem(*this);
}


SfxPoolItem* XFormTextShadowItem::CreateDefault() { return new XFormTextShadowItem; }

XFormTextShadowItem::XFormTextShadowItem(XFormTextShadow eFormTextShadow) :
    SfxEnumItem(XATTR_FORMTXTSHADOW, eFormTextShadow)
{
}

SfxPoolItem* XFormTextShadowItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowItem( *this );
}

sal_uInt16 XFormTextShadowItem::GetValueCount() const
{
    return 3;
}

bool XFormTextShadowItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<sal_Int32>(GetValue());
    return true;
}

bool XFormTextShadowItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    rVal >>= nValue;
    SetValue(static_cast<XFormTextShadow>(nValue));

    return true;
}


SfxPoolItem* XFormTextShadowColorItem::CreateDefault() { return new XFormTextShadowColorItem; }

XFormTextShadowColorItem::XFormTextShadowColorItem(const OUString& rName,
                                                     const Color& rTheColor) :
    XColorItem(XATTR_FORMTXTSHDWCOLOR, rName, rTheColor)
{
}

SfxPoolItem* XFormTextShadowColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowColorItem(*this);
}

SfxPoolItem* XFormTextShadowXValItem::CreateDefault() { return new XFormTextShadowXValItem; }

XFormTextShadowXValItem::XFormTextShadowXValItem(long nVal) :
    SfxMetricItem(XATTR_FORMTXTSHDWXVAL, nVal)
{
}

SfxPoolItem* XFormTextShadowXValItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowXValItem(*this);
}


SfxPoolItem* XFormTextShadowYValItem::CreateDefault() { return new XFormTextShadowYValItem; }

XFormTextShadowYValItem::XFormTextShadowYValItem(long nVal) :
    SfxMetricItem(XATTR_FORMTXTSHDWYVAL, nVal)
{
}

SfxPoolItem* XFormTextShadowYValItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowYValItem(*this);
}


SfxPoolItem* XFormTextHideFormItem::CreateDefault() { return new XFormTextHideFormItem; }

XFormTextHideFormItem::XFormTextHideFormItem(bool bHide) :
    SfxBoolItem(XATTR_FORMTXTHIDEFORM, bHide)
{
}

SfxPoolItem* XFormTextHideFormItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextHideFormItem(*this);
}

// --- SetItems ---


/// a line attribute set item
XLineAttrSetItem::XLineAttrSetItem( std::unique_ptr<SfxItemSet>&& pItemSet ) :
    SfxSetItem( XATTRSET_LINE, std::move(pItemSet))
{
}

XLineAttrSetItem::XLineAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_LINE,
        std::make_unique<SfxItemSet>( *pItemPool, svl::Items<XATTR_LINE_FIRST, XATTR_LINE_LAST>{}))
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

/// fill attribute set item
XFillAttrSetItem::XFillAttrSetItem( std::unique_ptr<SfxItemSet>&& pItemSet ) :
    SfxSetItem( XATTRSET_FILL, std::move(pItemSet))
{
}

XFillAttrSetItem::XFillAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_FILL,
        std::make_unique<SfxItemSet>( *pItemPool, svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{}))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
