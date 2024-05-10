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
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/propertyvalue.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/any.hxx>
#include <svl/itempool.hxx>
#include <editeng/memberids.h>
#include <docmodel/uno/UnoGradientTools.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/color/ComplexColorJSON.hxx>
#include <tools/mapunit.hxx>
#include <tools/UnitConversion.hxx>
#include <osl/diagnose.h>

#include <svx/unoapi.hxx>
#include <svl/style.hxx>

#include <tools/bigint.hxx>
#include <svl/itemset.hxx>
#include <svx/strings.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnasit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xsflclit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedcit.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <svx/xdef.hxx>
#include <svx/unomid.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xftdiit.hxx>
#include <svx/xftstit.hxx>
#include <svx/xftmrit.hxx>
#include <svx/xftouit.hxx>
#include <svx/xftshit.hxx>
#include <svx/xftshcit.hxx>
#include <svx/xftshxy.hxx>
#include <svx/xftadit.hxx>
#include <svx/svddef.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

typedef std::map<OUString, OUString> StringMap;

NameOrIndex::NameOrIndex(TypedWhichId<NameOrIndex> _nWhich, sal_Int32 nIndex) :
    SfxStringItem(_nWhich, OUString()),
    m_nPalIndex(nIndex)
{
}

NameOrIndex::NameOrIndex(TypedWhichId<NameOrIndex> _nWhich, const OUString& rName) :
    SfxStringItem(_nWhich, rName),
    m_nPalIndex(-1)
{
}

NameOrIndex::NameOrIndex(const NameOrIndex& rNameOrIndex) :
    SfxStringItem(rNameOrIndex),
    m_nPalIndex(rNameOrIndex.m_nPalIndex)
{
}

bool NameOrIndex::operator==(const SfxPoolItem& rItem) const
{
    return ( SfxStringItem::operator==(rItem) &&
            static_cast<const NameOrIndex&>(rItem).m_nPalIndex == m_nPalIndex );
}

NameOrIndex* NameOrIndex::Clone(SfxItemPool* /*pPool*/) const
{
    return new NameOrIndex(*this);
}

/** this static checks if the given NameOrIndex item has a unique name for its value.
    The returned String is a unique name for an item with this value in both given pools.
    Argument pPool2 can be null.
    If returned string equals NameOrIndex->GetName(), the name was already unique.
*/
OUString NameOrIndex::CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, SvxCompareValueFunc pCompareValueFunc, TranslateId pPrefixResId, const XPropertyListRef &pDefaults )
{
    bool bForceNew = false;

    OUString aUniqueName = SvxUnogetInternalNameForItem(nWhich, pCheckItem->GetName());

    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different line end or start

    if (!aUniqueName.isEmpty() && pPool1)
    {
        ItemSurrogates aSurrogates;
        pPool1->GetItemSurrogates(aSurrogates, nWhich);
        for (const SfxPoolItem* pItem : aSurrogates)
        {
            const NameOrIndex *pNameOrIndex = static_cast<const NameOrIndex*>(pItem);

            if( pNameOrIndex->GetName() == pCheckItem->GetName() )
            {
                // if there is already an item with the same name and the same
                // value it's ok to set it
                if( !pCompareValueFunc( pNameOrIndex, pCheckItem ) )
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

        if( pDefaults )
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
                            sal_Int32 nThisIndex = o3tl::toInt32(aEntryName.subView( aUser.getLength() ));
                            if( nThisIndex >= nUserIndex )
                                nUserIndex = nThisIndex + 1;
                        }
                    }
                }
            }
        }

        if (aUniqueName.isEmpty() && pPool1)
        {
            ItemSurrogates aSurrogates;
            pPool1->GetItemSurrogates(aSurrogates, nWhich);
            for (const SfxPoolItem* pItem : aSurrogates)
            {
                const NameOrIndex *pNameOrIndex = static_cast<const NameOrIndex*>(pItem);

                if( !pNameOrIndex->GetName().isEmpty() )
                {
                    if( !bForceNew && pCompareValueFunc( pNameOrIndex, pCheckItem ) )
                        return pNameOrIndex->GetName();

                    if( pNameOrIndex->GetName().startsWith( aUser ) )
                    {
                        sal_Int32 nThisIndex = o3tl::toInt32(pNameOrIndex->GetName().subView( aUser.getLength() ));
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
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("NameOrIndex"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("isIndex"), BAD_CAST(OString::boolean(IsIndex()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(m_nPalIndex).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

SfxPoolItem* XColorItem::CreateDefault() { return new XColorItem; }

XColorItem::XColorItem(TypedWhichId<XColorItem> _nWhich, sal_Int32 nIndex, const Color& rTheColor) :
    NameOrIndex(_nWhich, nIndex),
    m_aColor(rTheColor)
{
}

XColorItem::XColorItem(TypedWhichId<XColorItem> _nWhich, const OUString& rName, const Color& rTheColor) :
    NameOrIndex(_nWhich, rName),
    m_aColor(rTheColor)
{
}

XColorItem::XColorItem(TypedWhichId<XColorItem> _nWhich, const Color& rTheColor)
    : NameOrIndex(_nWhich, OUString())
    , m_aColor(rTheColor)
{
}

XColorItem::XColorItem(const XColorItem& rItem) :
    NameOrIndex(rItem),
    m_aColor(rItem.m_aColor),
    maComplexColor(rItem.maComplexColor)
{
}

XColorItem* XColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XColorItem(*this);
}

bool XColorItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
            static_cast<const XColorItem&>(rItem).m_aColor == m_aColor ) &&
            static_cast<const XColorItem&>(rItem).maComplexColor == maComplexColor;
}

const Color& XColorItem::GetColorValue() const
{
    assert(!IsIndex());
    return m_aColor;

}

bool XColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_COMPLEX_COLOR:
        {
            auto xComplexColor = model::color::createXComplexColor(getComplexColor());
            rVal <<= xComplexColor;
            break;
        }
        case MID_COMPLEX_COLOR_JSON:
        {
            rVal <<= OStringToOUString(model::color::convertToJSON(getComplexColor()), RTL_TEXTENCODING_UTF8);
            break;
        }
        default:
        {
            rVal <<= GetColorValue().GetRGBColor();
            break;
        }
    }
    return true;
}

bool XColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_COMPLEX_COLOR:
        {
            css::uno::Reference<css::util::XComplexColor> xComplexColor;
            if (!(rVal >>= xComplexColor))
                return false;
            setComplexColor(model::color::getFromXComplexColor(xComplexColor));
        }
        break;
        case MID_COMPLEX_COLOR_JSON:
        {
            OUString sComplexColorJson;
            if (!(rVal >>= sComplexColorJson))
                return false;

            if (sComplexColorJson.isEmpty())
                return false;

            OString aJSON = OUStringToOString(sComplexColorJson, RTL_TEXTENCODING_ASCII_US);
            model::ComplexColor aComplexColor;
            model::color::convertFromJSON(aJSON, aComplexColor);
            setComplexColor(aComplexColor);
        }
        break;
        default:
        {
            Color nValue;
            if(!(rVal >>= nValue ))
                return false;

            SetColorValue( nValue );

        }
        break;
    }
    return true;
}

void XColorItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("XColorItem"));
    if (Which() == SDRATTR_SHADOWCOLOR)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST("SDRATTR_SHADOWCOLOR"));
    }
    else if (Which() == XATTR_FILLCOLOR)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST("XATTR_FILLCOLOR"));
    }
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aColor"),
                                BAD_CAST(m_aColor.AsRGBHexString().toUtf8().getStr()));

    NameOrIndex::dumpAsXml(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("complex-color"));

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("scheme-index"),
                                      BAD_CAST(OString::number(sal_Int16(maComplexColor.getThemeColorType())).getStr()));

    for (auto const& rTransform : maComplexColor.getTransformations())
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("transformation"));
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"),
                                      BAD_CAST(OString::number(sal_Int16(rTransform.meType)).getStr()));
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                      BAD_CAST(OString::number(rTransform.mnValue).getStr()));
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

// --- line attributes ---


SfxPoolItem* XLineStyleItem::CreateDefault() { return new XLineStyleItem; }

XLineStyleItem::XLineStyleItem(css::drawing::LineStyle eTheLineStyle) :
    SfxEnumItem(XATTR_LINESTYLE, eTheLineStyle)
{
}

XLineStyleItem* XLineStyleItem::Clone(SfxItemPool* /*pPool*/) const
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

    TranslateId pId;

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

XDash::XDash(css::drawing::DashStyle eTheDash, sal_uInt16 nTheDots, double nTheDotLen,
             sal_uInt16 nTheDashes, double nTheDashLen, double nTheDistance) :
    m_eDash(eTheDash),
    m_nDots(nTheDots),
    m_nDashes(nTheDashes),
    m_nDotLen(nTheDotLen),
    m_nDashLen(nTheDashLen),
    m_nDistance(nTheDistance)
{
}

bool XDash::operator==(const XDash& rDash) const
{
    return ( m_eDash      == rDash.m_eDash      &&
             m_nDots      == rDash.m_nDots      &&
             m_nDotLen    == rDash.m_nDotLen    &&
             m_nDashes    == rDash.m_nDashes    &&
             m_nDashLen   == rDash.m_nDashLen   &&
             m_nDistance  == rDash.m_nDistance );
}

// XDash is translated into an array of doubles which describe the lengths of the
// dashes, dots and empty passages. It returns the complete length of the full DashDot
// sequence and fills the given vector of doubles accordingly (also resizing, so deleting it).
const double SMALLEST_DASH_WIDTH(26.95);

double XDash::CreateDotDashArray(::std::vector< double >& rDotDashArray, double fLineWidth) const
{
    double fFullDotDashLen(0.0);
    const sal_uInt16 nNumDotDashArray = (GetDots() + GetDashes()) * 2;
    rDotDashArray.resize( nNumDotDashArray, 0.0 );
    sal_uInt16 a;
    sal_uInt16 nIns(0);
    double fDashDotDistance = GetDistance();
    double fSingleDashLen = GetDashLen();
    double fSingleDotLen = GetDotLen();

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

XLineDashItem* XLineDashItem::Clone(SfxItemPool* /*pPool*/) const
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

void XLineDashItem::ScaleMetrics(tools::Long nMul, tools::Long nDiv)
{
    aDash.SetDotLen( BigInt::Scale( aDash.GetDotLen(), nMul, nDiv ) );
    aDash.SetDashLen( BigInt::Scale( aDash.GetDashLen(), nMul, nDiv ) );
    aDash.SetDistance( BigInt::Scale( aDash.GetDistance(), nMul, nDiv ) );
}

bool XLineDashItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            css::drawing::LineDash aLineDash;

            const XDash& rXD = GetDashValue();
            aLineDash.Style = static_cast<css::drawing::DashStyle>(static_cast<sal_uInt16>(rXD.GetDashStyle()));
            aLineDash.Dots = rXD.GetDots();
            aLineDash.DotLen = rXD.GetDotLen();
            aLineDash.Dashes = rXD.GetDashes();
            aLineDash.DashLen = rXD.GetDashLen();
            aLineDash.Distance = rXD.GetDistance();

            uno::Sequence< beans::PropertyValue > aPropSeq{
                comphelper::makePropertyValue(u"Name"_ustr, SvxUnogetApiNameForItem(Which(), GetName())),
                comphelper::makePropertyValue(u"LineDash"_ustr, aLineDash)
            };
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
                for (const auto& rProp : aPropSeq)
                {
                    if ( rProp.Name == "Name" )
                        rProp.Value >>= aName;
                    else if ( rProp.Name == "LineDash" )
                    {
                        if ( rProp.Value >>= aLineDash )
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
                XLineDashItem::CompareValueFunc, RID_SVXSTR_DASH20,
                pModel->GetPropertyList( XPropertyListType::Dash ) );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
            return std::make_unique<XLineDashItem>( aUniqueName, aDash );
    }

    return nullptr;
}

SfxPoolItem* XLineWidthItem::CreateDefault() {return new XLineWidthItem;}

XLineWidthItem::XLineWidthItem(tools::Long nWidth) :
    SfxMetricItem(XATTR_LINEWIDTH, nWidth)
{
}

XLineWidthItem* XLineWidthItem::Clone(SfxItemPool* /*pPool*/) const
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
    rText = GetMetricText( static_cast<tools::Long>(GetValue()),
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
        nValue = o3tl::toTwips(nValue, o3tl::Length::mm100);

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

XLineColorItem* XLineColorItem::Clone(SfxItemPool* /*pPool*/) const
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

bool XLineColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_COMPLEX_COLOR:
        {
            auto xComplexColor = model::color::createXComplexColor(getComplexColor());
            rVal <<= xComplexColor;
            break;
        }
        case MID_COMPLEX_COLOR_JSON:
        {
            rVal <<= OStringToOUString(model::color::convertToJSON(getComplexColor()), RTL_TEXTENCODING_UTF8);
            break;
        }
        default:
        {
            rVal <<= GetColorValue().GetRGBColor();
            break;
        }
    }
    return true;
}

bool XLineColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_COMPLEX_COLOR:
        {
            css::uno::Reference<css::util::XComplexColor> xComplexColor;
            if (!(rVal >>= xComplexColor))
                return false;
            setComplexColor(model::color::getFromXComplexColor(xComplexColor));
        }
        break;
        case MID_COMPLEX_COLOR_JSON:
        {
            OUString sComplexColorJson;
            if (!(rVal >>= sComplexColorJson))
                return false;

            if (sComplexColorJson.isEmpty())
                return false;
            model::ComplexColor aComplexColor;
            OString aJSON = OUStringToOString(sComplexColorJson, RTL_TEXTENCODING_ASCII_US);
            model::color::convertFromJSON(aJSON, aComplexColor);
            setComplexColor(aComplexColor);
        }
        break;
        default:
        {
            sal_Int32 nValue;
            if(!(rVal >>= nValue ))
                return false;

            SetColorValue( Color(ColorTransparency, nValue) );
            break;
        }
    }
    return true;
}


SfxPoolItem* XLineStartItem::CreateDefault() {return new XLineStartItem;}

XLineStartItem::XLineStartItem(sal_Int32 nIndex)
:   NameOrIndex(XATTR_LINESTART, nIndex)
{
}

XLineStartItem::XLineStartItem(const OUString& rName, basegfx::B2DPolyPolygon aPolyPolygon)
:   NameOrIndex(XATTR_LINESTART, rName),
    maPolyPolygon(std::move(aPolyPolygon))
{
}

XLineStartItem::XLineStartItem(const XLineStartItem& rItem)
:   NameOrIndex(rItem),
    maPolyPolygon(rItem.maPolyPolygon)
{
}

XLineStartItem::XLineStartItem(basegfx::B2DPolyPolygon aPolyPolygon)
:   NameOrIndex( XATTR_LINESTART, -1 ),
    maPolyPolygon(std::move(aPolyPolygon))
{
}

XLineStartItem* XLineStartItem::Clone(SfxItemPool* /*pPool*/) const
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
                pTempItem.reset(new XLineStartItem( aUniqueName, std::move(aNew) ));
                pLineStartItem = pTempItem.get();
            }
        }

        bool bForceNew = false;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        const SfxItemPool& rPool1 = pModel->GetItemPool();
        if (!aUniqueName.isEmpty())
        {
            ItemSurrogates aSurrogates;
            rPool1.GetItemSurrogates(aSurrogates, XATTR_LINESTART);
            for (const SfxPoolItem* p : aSurrogates)
            {
                auto pItem = dynamic_cast<const XLineStartItem*>(p);

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
                rPool1.GetItemSurrogates(aSurrogates, XATTR_LINEEND);
                for (const SfxPoolItem* p : aSurrogates)
                {
                    auto pItem = dynamic_cast<const XLineEndItem*>(p);

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
            ItemSurrogates aSurrogates;
            pPool2->GetItemSurrogates(aSurrogates, XATTR_LINESTART);
            for (const SfxPoolItem* p : aSurrogates)
            {
                auto pItem = dynamic_cast<const XLineStartItem*>(p);

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
                pPool2->GetItemSurrogates(aSurrogates, XATTR_LINEEND);
                for (const SfxPoolItem* p : aSurrogates)
                {
                    auto pItem = dynamic_cast<const XLineEndItem*>(p);

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

            ItemSurrogates aSurrogates;
            rPool1.GetItemSurrogates(aSurrogates, XATTR_LINESTART);
            for (const SfxPoolItem* p : aSurrogates)
            {
                auto pItem = dynamic_cast<const XLineStartItem*>(p);

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
                        sal_Int32 nThisIndex = o3tl::toInt32(pItem->GetName().subView(aUser.getLength()));
                        if (nThisIndex >= nUserIndex)
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }

            rPool1.GetItemSurrogates(aSurrogates, XATTR_LINEEND);
            for (const SfxPoolItem* p : aSurrogates)
            {
                auto pItem = dynamic_cast<const XLineEndItem*>(p);

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
                        sal_Int32 nThisIndex = o3tl::toInt32(pItem->GetName().subView(aUser.getLength()));
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

XLineEndItem::XLineEndItem(const OUString& rName, basegfx::B2DPolyPolygon aPolyPolygon)
:   NameOrIndex(XATTR_LINEEND, rName),
    maPolyPolygon(std::move(aPolyPolygon))
{
}

XLineEndItem::XLineEndItem(const XLineEndItem& rItem)
:   NameOrIndex(rItem),
    maPolyPolygon(rItem.maPolyPolygon)
{
}

XLineEndItem::XLineEndItem(basegfx::B2DPolyPolygon aPolyPolygon)
:   NameOrIndex( XATTR_LINEEND, -1 ),
    maPolyPolygon(std::move(aPolyPolygon))
{
}

XLineEndItem* XLineEndItem::Clone(SfxItemPool* /*pPool*/) const
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
                pTempItem.reset(new XLineEndItem( aUniqueName, std::move(aNew) ));
                pLineEndItem = pTempItem.get();
            }
        }

        bool bForceNew = false;

        // 2. if we have a name check if there is already an item with the
        // same name in the documents pool with a different line end or start

        const SfxItemPool& rPool1 = pModel->GetItemPool();
        if (!aUniqueName.isEmpty())
        {
            ItemSurrogates aSurrogates;
            rPool1.GetItemSurrogates(aSurrogates, XATTR_LINESTART);
            for (const SfxPoolItem* p : aSurrogates)
            {
                auto pItem = dynamic_cast<const XLineStartItem*>(p);

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
                rPool1.GetItemSurrogates(aSurrogates, XATTR_LINEEND);
                for (const SfxPoolItem* p : aSurrogates)
                {
                    auto pItem = dynamic_cast<const XLineEndItem*>(p);

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
            ItemSurrogates aSurrogates;
            pPool2->GetItemSurrogates(aSurrogates, XATTR_LINESTART);
            for (const SfxPoolItem* p : aSurrogates)
            {
                auto pItem = dynamic_cast<const XLineStartItem*>(p);

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
                pPool2->GetItemSurrogates(aSurrogates, XATTR_LINEEND);
                for (const SfxPoolItem* p : aSurrogates)
                {
                    auto pItem = dynamic_cast<const XLineEndItem*>(p);

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

            ItemSurrogates aSurrogates;
            rPool1.GetItemSurrogates(aSurrogates, XATTR_LINESTART);
            for (const SfxPoolItem* p : aSurrogates)
            {
                auto pItem = dynamic_cast<const XLineStartItem*>(p);

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
                        sal_Int32 nThisIndex = o3tl::toInt32(pItem->GetName().subView(aUser.getLength()));
                        if (nThisIndex >= nUserIndex)
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }

            rPool1.GetItemSurrogates(aSurrogates, XATTR_LINEEND);
            for (const SfxPoolItem* p : aSurrogates)
            {
                auto pItem = dynamic_cast<const XLineEndItem*>(p);

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
                        sal_Int32 nThisIndex = o3tl::toInt32(pItem->GetName().subView(aUser.getLength()));
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

XLineStartWidthItem::XLineStartWidthItem(tools::Long nWidth) :
    SfxMetricItem(XATTR_LINESTARTWIDTH, nWidth)
{
}

XLineStartWidthItem* XLineStartWidthItem::Clone(SfxItemPool* /*pPool*/) const
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
    rText = GetMetricText( static_cast<tools::Long>(GetValue()),
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

XLineEndWidthItem::XLineEndWidthItem(tools::Long nWidth) :
   SfxMetricItem(XATTR_LINEENDWIDTH, nWidth)
{
}

XLineEndWidthItem* XLineEndWidthItem::Clone(SfxItemPool* /*pPool*/) const
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
    rText = GetMetricText( static_cast<tools::Long>(GetValue()),
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

XLineStartCenterItem* XLineStartCenterItem::Clone(SfxItemPool* /*pPool*/) const
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
    if( !b.has_value() )
        return false;

    SetValue( *b );
    return true;
}

XLineEndCenterItem::XLineEndCenterItem(bool bEndCenter) :
    SfxBoolItem(XATTR_LINEENDCENTER, bEndCenter)
{
}

XLineEndCenterItem* XLineEndCenterItem::Clone(SfxItemPool* /*pPool*/) const
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
    if( !b.has_value() )
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

XFillStyleItem* XFillStyleItem::Clone(SfxItemPool* /*pPool*/) const
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

    TranslateId pId;

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
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("XFillStyleItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(static_cast<sal_Int16>(GetValue())).getStr()));

    OUString aPresentation;
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    GetPresentation(SfxItemPresentation::Nameless, MapUnit::Map100thMM, MapUnit::Map100thMM, aPresentation, aIntlWrapper);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(aPresentation.toUtf8().getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

boost::property_tree::ptree XFillStyleItem::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = SfxPoolItem::dumpAsJSON();

    if (Which() == XATTR_FILLSTYLE)
        aTree.put("commandName", ".uno:FillStyle");

    OUString sValue;

    switch( GetValue() )
    {
        case drawing::FillStyle_NONE:
            sValue = "NONE";
            break;
        case drawing::FillStyle_SOLID:
            sValue = "SOLID";
            break;
        case drawing::FillStyle_GRADIENT:
            sValue = "GRADIENT";
            break;
        case drawing::FillStyle_HATCH:
            sValue = "HATCH";
            break;
        case drawing::FillStyle_BITMAP:
            sValue = "BITMAP";
            break;
        default: break;
    }

    aTree.put("state", sValue);

    return aTree;
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

XFillColorItem* XFillColorItem::Clone(SfxItemPool* /*pPool*/) const
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

bool XFillColorItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_COLOR_THEME_INDEX:
        {
            rVal <<= sal_Int16(getComplexColor().getThemeColorType());
            break;
        }
        case MID_COLOR_LUM_MOD:
        {
            sal_Int16 nValue = 10000;
            for (auto const& rTransform : getComplexColor().getTransformations())
            {
                if (rTransform.meType == model::TransformationType::LumMod)
                    nValue = rTransform.mnValue;
            }
            rVal <<= nValue;
            break;
        }
        case MID_COLOR_LUM_OFF:
        {
            sal_Int16 nValue = 0;
            for (auto const& rTransform : getComplexColor().getTransformations())
            {
                if (rTransform.meType == model::TransformationType::LumOff)
                    nValue = rTransform.mnValue;
            }
            rVal <<= nValue;
            break;
        }
        case MID_COMPLEX_COLOR:
        {
            auto xComplexColor = model::color::createXComplexColor(getComplexColor());
            rVal <<= xComplexColor;
            break;
        }
        case MID_COMPLEX_COLOR_JSON:
        {
            rVal <<= OStringToOUString(model::color::convertToJSON(getComplexColor()), RTL_TEXTENCODING_UTF8);
            break;
        }
        default:
        {
            rVal <<= GetColorValue().GetRGBColor();
            break;
        }
    }

    return true;
}

bool XFillColorItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_COLOR_THEME_INDEX:
        {
            sal_Int16 nIndex = -1;
            if (!(rVal >>= nIndex))
                return false;
            getComplexColor().setThemeColor(model::convertToThemeColorType(nIndex));
        }
        break;
        case MID_COLOR_LUM_MOD:
        {
            sal_Int16 nLumMod = 10000;
            if (!(rVal >>= nLumMod))
                return false;
            getComplexColor().removeTransformations(model::TransformationType::LumMod);
            getComplexColor().addTransformation({model::TransformationType::LumMod, nLumMod});
        }
        break;
        case MID_COLOR_LUM_OFF:
        {
            sal_Int16 nLumOff = 0;
            if (!(rVal >>= nLumOff))
                return false;
            getComplexColor().removeTransformations(model::TransformationType::LumOff);
            getComplexColor().addTransformation({model::TransformationType::LumOff, nLumOff});
        }
        break;
        case MID_COMPLEX_COLOR:
        {
            css::uno::Reference<css::util::XComplexColor> xComplexColor;
            if (!(rVal >>= xComplexColor))
                return false;
            setComplexColor(model::color::getFromXComplexColor(xComplexColor));
        }
        break;
        case MID_COMPLEX_COLOR_JSON:
        {
            OUString sComplexColorJson;
            if (!(rVal >>= sComplexColorJson))
                return false;

            if (sComplexColorJson.isEmpty())
                return false;

            OString aJSON = OUStringToOString(sComplexColorJson, RTL_TEXTENCODING_ASCII_US);
            model::ComplexColor aComplexColor;
            model::color::convertFromJSON(aJSON, aComplexColor);
            setComplexColor(aComplexColor);
        }
        break;
        default:
        {
            Color nValue;
            if(!(rVal >>= nValue ))
                return false;

            SetColorValue( nValue );

        }
        break;
    }
    return true;
}

void XFillColorItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("XFillColorItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));

    XColorItem::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

boost::property_tree::ptree XFillColorItem::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = SfxPoolItem::dumpAsJSON();

    if (Which() == XATTR_FILLCOLOR)
        aTree.put("commandName", ".uno:FillPageColor");

    aTree.put("state", GetColorValue().AsRGBHexString());

    return aTree;
}

XSecondaryFillColorItem::XSecondaryFillColorItem(const OUString& rName, const Color& rTheColor) :
    XColorItem(XATTR_SECONDARYFILLCOLOR, rName, rTheColor)
{
}

XSecondaryFillColorItem* XSecondaryFillColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XSecondaryFillColorItem(*this);
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

SfxPoolItem* XFillGradientItem::CreateDefault() { return new XFillGradientItem; }

XFillGradientItem::XFillGradientItem(sal_Int32 nIndex,
                                   const basegfx::BGradient& rTheGradient) :
    NameOrIndex(XATTR_FILLGRADIENT, nIndex),
    m_aGradient(rTheGradient)
{
}

XFillGradientItem::XFillGradientItem(const OUString& rName,
                                   const basegfx::BGradient& rTheGradient, TypedWhichId<XFillGradientItem> nWhich)
    : NameOrIndex(nWhich, rName)
    , m_aGradient(rTheGradient)
{
}

XFillGradientItem::XFillGradientItem(const XFillGradientItem& rItem) :
    NameOrIndex(rItem),
    m_aGradient(rItem.m_aGradient)
{
}

XFillGradientItem::XFillGradientItem( const basegfx::BGradient& rTheGradient )
:   NameOrIndex( XATTR_FILLGRADIENT, -1 ),
    m_aGradient(rTheGradient)
{
}

XFillGradientItem* XFillGradientItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillGradientItem(*this);
}

bool XFillGradientItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             m_aGradient == static_cast<const XFillGradientItem&>(rItem).m_aGradient );
}

const basegfx::BGradient& XFillGradientItem::GetGradientValue() const // GetValue -> GetGradientValue
{
    if (!IsIndex())
        return m_aGradient;
    // ToDo: This should fail. We never called this code with a table so this should always
    // have failed. Thus, I'm thinking that XFillGradientItem can't be an Index.
    return m_aGradient;
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
            // fill values
            const css::awt::Gradient2 aGradient2 = model::gradient::createUnoGradient2(GetGradientValue());

            // create sequence
            uno::Sequence< beans::PropertyValue > aPropSeq{
                comphelper::makePropertyValue(u"Name"_ustr, SvxUnogetApiNameForItem(Which(), GetName())),
                comphelper::makePropertyValue(u"FillGradient"_ustr, aGradient2)
            };
            rVal <<= aPropSeq;
            break;
        }

        case MID_FILLGRADIENT:
        {
            // fill values
            const css::awt::Gradient2 aGradient2 = model::gradient::createUnoGradient2(GetGradientValue());

            // create sequence
            rVal <<= aGradient2;
            break;
        }

        case MID_NAME:
        {
            rVal <<= SvxUnogetApiNameForItem(Which(), GetName());
            break;
        }

        case MID_GRADIENT_COLORSTOPSEQUENCE:
        {
            // fill values
            const css::awt::ColorStopSequence aColorStopSequence = model::gradient::createColorStopSequence(GetGradientValue().GetColorStops());

            // create sequence
            rVal <<= aColorStopSequence;
            break;
        }

        case MID_GRADIENT_STYLE: rVal <<= static_cast<sal_Int16>(GetGradientValue().GetGradientStyle()); break;
        case MID_GRADIENT_STARTCOLOR: rVal <<= Color(GetGradientValue().GetColorStops().front().getStopColor()); break;
        case MID_GRADIENT_ENDCOLOR: rVal <<= Color(GetGradientValue().GetColorStops().back().getStopColor()); break;
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
            css::uno::Any aGradientAny;

            if ( rVal >>= aPropSeq )
            {
                OUString aName;

                for (const auto& rProp : aPropSeq)
                {
                    if ( rProp.Name == "Name" )
                        rProp.Value >>= aName;
                    else if ( rProp.Name == "FillGradient" )
                        aGradientAny = rProp.Value;
                }

                SetName( aName );

                if (aGradientAny.hasValue() && (aGradientAny.has<css::awt::Gradient>() || aGradientAny.has<css::awt::Gradient2>()))
                {
                    SetGradientValue(model::gradient::getFromAny(aGradientAny));
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
            if (rVal.hasValue() && (rVal.has<css::awt::Gradient>() || rVal.has<css::awt::Gradient2>()))
            {
                SetGradientValue(model::gradient::getFromAny(rVal));
            }

            break;
        }

        case MID_GRADIENT_COLORSTOPSEQUENCE:
        {
            // check if we have a awt::ColorStopSequence
            if (rVal.hasValue() && rVal.has<css::awt::ColorStopSequence>())
            {

                const basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromAny(rVal);

                if (!aColorStops.empty())
                {
                    basegfx::BGradient aBGradient(GetGradientValue());
                    aBGradient.SetColorStops(aColorStops);
                    SetGradientValue(aBGradient);
                }
            }
            break;
        }

        case MID_GRADIENT_STARTCOLOR:
        case MID_GRADIENT_ENDCOLOR:
        {
            Color nVal;
            if(!(rVal >>= nVal ))
                return false;

            basegfx::BGradient aBGradient(GetGradientValue());
            basegfx::BColorStops aNewColorStops(aBGradient.GetColorStops());

            if ( nMemberId == MID_GRADIENT_STARTCOLOR )
            {
                aNewColorStops.replaceStartColor(nVal.getBColor());
            }
            else
            {
                aNewColorStops.replaceEndColor(nVal.getBColor());
            }

            aBGradient.SetColorStops(aNewColorStops);
            SetGradientValue( aBGradient );
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

            basegfx::BGradient aBGradient = GetGradientValue();

            switch ( nMemberId )
            {
                case MID_GRADIENT_STYLE:
                    aBGradient.SetGradientStyle( static_cast<css::awt::GradientStyle>(nVal) ); break;
                case MID_GRADIENT_ANGLE:
                    aBGradient.SetAngle( Degree10(nVal) ); break;
                case MID_GRADIENT_BORDER:
                    aBGradient.SetBorder( nVal ); break;
                case MID_GRADIENT_STARTINTENSITY:
                    aBGradient.SetStartIntens( nVal ); break;
                case MID_GRADIENT_ENDINTENSITY:
                    aBGradient.SetEndIntens( nVal ); break;
                case MID_GRADIENT_STEPCOUNT:
                    aBGradient.SetSteps( nVal ); break;
                case MID_GRADIENT_XOFFSET:
                    aBGradient.SetXOffset( nVal ); break;
                case MID_GRADIENT_YOFFSET:
                    aBGradient.SetYOffset( nVal ); break;
            }

            SetGradientValue( aBGradient );
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
            return std::make_unique<XFillGradientItem>( aUniqueName, m_aGradient, TypedWhichId<XFillGradientItem>(Which()) );
    }

    return nullptr;
}

boost::property_tree::ptree XFillGradientItem::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = SfxPoolItem::dumpAsJSON();

    if (Which() == XATTR_FILLGRADIENT)
        aTree.put("commandName", ".uno:FillGradient");

    aTree.push_back(std::make_pair("state", GetGradientValue().dumpAsJSON()));

    return aTree;
}


SfxPoolItem* XFillFloatTransparenceItem::CreateDefault() { return new XFillFloatTransparenceItem; }

XFillFloatTransparenceItem::XFillFloatTransparenceItem() :
    bEnabled( false )
{
    SetWhich( XATTR_FILLFLOATTRANSPARENCE );
}

XFillFloatTransparenceItem::XFillFloatTransparenceItem(const OUString& rName, const basegfx::BGradient& rGradient, bool bEnable ) :
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

XFillFloatTransparenceItem::XFillFloatTransparenceItem(const basegfx::BGradient& rTheGradient, bool bEnable )
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

XFillFloatTransparenceItem* XFillFloatTransparenceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillFloatTransparenceItem( *this );
}

bool XFillFloatTransparenceItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    if (MID_GRADIENT_STARTINTENSITY == nMemberId
        || MID_GRADIENT_ENDINTENSITY == nMemberId
        || MID_GRADIENT_STEPCOUNT == nMemberId)
    {
        // tdf#155913 handle attributes not supported by transparency gradient as error
        return false;
    }

    if (!IsEnabled() && nMemberId == MID_NAME)
    {
        // make sure that we return empty string in case of query for
        // "FillTransparenceGradientName" if the item is disabled
        rVal <<= OUString();
        return true;
    }

    return XFillGradientItem::QueryValue( rVal, nMemberId );
}

bool XFillFloatTransparenceItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    if (MID_GRADIENT_STARTINTENSITY == nMemberId
        || MID_GRADIENT_ENDINTENSITY == nMemberId
        || MID_GRADIENT_STEPCOUNT == nMemberId)
    {
        // tdf#155913 handle attributes not supported by transparency gradient as error
        return false;
    }

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

boost::property_tree::ptree XFillFloatTransparenceItem::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = XFillGradientItem::dumpAsJSON();
    aTree.put("commandName", ".uno:FillFloatTransparence");

    if (!bEnabled)
    {
        boost::property_tree::ptree& rState = aTree.get_child("state");
        // When gradient fill is disabled, the intensity fields contain the
        // constant encoded percent-transparency. However we use that here to just
        // distinguish between 'None' and 'Solid' types and correct the 'style'
        // property appropriately.
        if (GetGradientValue().GetStartIntens() == 100)
            rState.put("style", "NONE");
        else
            rState.put("style", "SOLID");
    }

    return aTree;
}

XHatch::XHatch(const Color& rCol, css::drawing::HatchStyle eTheStyle, tools::Long nTheDistance,
               Degree10 nTheAngle) :
    m_eStyle(eTheStyle),
    m_aColor(rCol),
    m_nDistance(nTheDistance),
    m_nAngle(nTheAngle)
{
}

bool XHatch::operator==(const XHatch& rHatch) const
{
    return ( m_eStyle     == rHatch.m_eStyle    &&
             m_aColor     == rHatch.m_aColor    &&
             m_nDistance  == rHatch.m_nDistance &&
             m_nAngle     == rHatch.m_nAngle );
}


SfxPoolItem* XFillHatchItem::CreateDefault() { return new XFillHatchItem; }

XFillHatchItem::XFillHatchItem(const OUString& rName,
                             const XHatch& rTheHatch) :
    NameOrIndex(XATTR_FILLHATCH, rName),
    m_aHatch(rTheHatch)
{
}

XFillHatchItem::XFillHatchItem(const XFillHatchItem& rItem) :
    NameOrIndex(rItem),
    m_aHatch(rItem.m_aHatch)
{
}

XFillHatchItem::XFillHatchItem(const XHatch& rTheHatch)
:   NameOrIndex( XATTR_FILLHATCH, -1 ),
    m_aHatch(rTheHatch)
{
}

XFillHatchItem* XFillHatchItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillHatchItem(*this);
}

bool XFillHatchItem::operator==(const SfxPoolItem& rItem) const
{
    return ( NameOrIndex::operator==(rItem) &&
             m_aHatch == static_cast<const XFillHatchItem&>(rItem).m_aHatch );
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

void XFillHatchItem::ScaleMetrics(tools::Long nMul, tools::Long nDiv)
{
    m_aHatch.SetDistance( BigInt::Scale( m_aHatch.GetDistance(), nMul, nDiv ) );
}

bool XFillHatchItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case 0:
        {
            css::drawing::Hatch aUnoHatch;

            aUnoHatch.Style = m_aHatch.GetHatchStyle();
            aUnoHatch.Color = sal_Int32(m_aHatch.GetColor());
            aUnoHatch.Distance = m_aHatch.GetDistance();
            aUnoHatch.Angle = m_aHatch.GetAngle().get();

            uno::Sequence< beans::PropertyValue > aPropSeq{
                comphelper::makePropertyValue(u"Name"_ustr, SvxUnogetApiNameForItem(Which(), GetName())),
                comphelper::makePropertyValue(u"FillHatch"_ustr, aUnoHatch)
            };
            rVal <<= aPropSeq;
            break;
        }

        case MID_FILLHATCH:
        {
            css::drawing::Hatch aUnoHatch;

            aUnoHatch.Style = m_aHatch.GetHatchStyle();
            aUnoHatch.Color = sal_Int32(m_aHatch.GetColor());
            aUnoHatch.Distance = m_aHatch.GetDistance();
            aUnoHatch.Angle = m_aHatch.GetAngle().get();
            rVal <<= aUnoHatch;
            break;
        }

        case MID_NAME:
        {
            rVal <<= SvxUnogetApiNameForItem(Which(), GetName());
            break;
        }

        case MID_HATCH_STYLE:
            rVal <<= m_aHatch.GetHatchStyle(); break;
        case MID_HATCH_COLOR:
            rVal <<= m_aHatch.GetColor(); break;
        case MID_HATCH_DISTANCE:
            rVal <<= m_aHatch.GetDistance(); break;
        case MID_HATCH_ANGLE:
            rVal <<= m_aHatch.GetAngle().get(); break;

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
                for (const auto& rProp : aPropSeq)
                {
                    if ( rProp.Name == "Name" )
                        rProp.Value >>= aName;
                    else if ( rProp.Name == "FillHatch" )
                    {
                        if ( rProp.Value >>= aUnoHatch )
                            bHatch = true;
                    }
                }

                SetName( aName );
                if ( bHatch )
                {
                    m_aHatch.SetHatchStyle( aUnoHatch.Style );
                    m_aHatch.SetColor( Color(ColorTransparency, aUnoHatch.Color) );
                    m_aHatch.SetDistance( aUnoHatch.Distance );
                    m_aHatch.SetAngle( Degree10(aUnoHatch.Angle) );
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

            m_aHatch.SetHatchStyle( aUnoHatch.Style );
            m_aHatch.SetColor( Color(ColorTransparency, aUnoHatch.Color) );
            m_aHatch.SetDistance( aUnoHatch.Distance );
            m_aHatch.SetAngle( Degree10(aUnoHatch.Angle) );
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
            m_aHatch.SetHatchStyle( static_cast<css::drawing::HatchStyle>(nVal) );
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
                m_aHatch.SetColor( Color(ColorTransparency, nVal) );
            else if ( nMemberId == MID_HATCH_DISTANCE )
                m_aHatch.SetDistance( nVal );
            else
                m_aHatch.SetAngle( Degree10(nVal) );
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
            return std::make_unique<XFillHatchItem>( aUniqueName, m_aHatch );
    }

    return nullptr;
}

// --- form text attributes ---


SfxPoolItem* XFormTextStyleItem::CreateDefault() { return new XFormTextStyleItem; }

XFormTextStyleItem::XFormTextStyleItem(XFormTextStyle eTheStyle) :
    SfxEnumItem(XATTR_FORMTXTSTYLE, eTheStyle)
{
}

XFormTextStyleItem* XFormTextStyleItem::Clone(SfxItemPool* /*pPool*/) const
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

XFormTextAdjustItem* XFormTextAdjustItem::Clone(SfxItemPool* /*pPool*/) const
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

XFormTextDistanceItem::XFormTextDistanceItem(tools::Long nDist) :
    SfxMetricItem(XATTR_FORMTXTDISTANCE, nDist)
{
}

XFormTextDistanceItem* XFormTextDistanceItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextDistanceItem(*this);
}

SfxPoolItem* XFormTextStartItem::CreateDefault() { return new XFormTextStartItem; }

XFormTextStartItem::XFormTextStartItem(tools::Long nStart) :
    SfxMetricItem(XATTR_FORMTXTSTART, nStart)
{
}

XFormTextStartItem* XFormTextStartItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextStartItem(*this);
}

SfxPoolItem* XFormTextMirrorItem::CreateDefault() { return new XFormTextMirrorItem; }

XFormTextMirrorItem::XFormTextMirrorItem(bool bMirror) :
    SfxBoolItem(XATTR_FORMTXTMIRROR, bMirror)
{
}

XFormTextMirrorItem* XFormTextMirrorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextMirrorItem(*this);
}

SfxPoolItem* XFormTextOutlineItem::CreateDefault() { return new XFormTextOutlineItem; }

XFormTextOutlineItem::XFormTextOutlineItem(bool bOutline) :
    SfxBoolItem(XATTR_FORMTXTOUTLINE, bOutline)
{
}

XFormTextOutlineItem* XFormTextOutlineItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextOutlineItem(*this);
}

SfxPoolItem* XFormTextShadowItem::CreateDefault() { return new XFormTextShadowItem; }

XFormTextShadowItem::XFormTextShadowItem(XFormTextShadow eFormTextShadow) :
    SfxEnumItem(XATTR_FORMTXTSHADOW, eFormTextShadow)
{
}

XFormTextShadowItem* XFormTextShadowItem::Clone(SfxItemPool* /*pPool*/) const
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

XFormTextShadowColorItem* XFormTextShadowColorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowColorItem(*this);
}

SfxPoolItem* XFormTextShadowXValItem::CreateDefault() { return new XFormTextShadowXValItem; }

XFormTextShadowXValItem::XFormTextShadowXValItem(tools::Long nVal) :
    SfxMetricItem(XATTR_FORMTXTSHDWXVAL, nVal)
{
}

XFormTextShadowXValItem* XFormTextShadowXValItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowXValItem(*this);
}

SfxPoolItem* XFormTextShadowYValItem::CreateDefault() { return new XFormTextShadowYValItem; }

XFormTextShadowYValItem::XFormTextShadowYValItem(tools::Long nVal) :
    SfxMetricItem(XATTR_FORMTXTSHDWYVAL, nVal)
{
}

XFormTextShadowYValItem* XFormTextShadowYValItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowYValItem(*this);
}

SfxPoolItem* XFormTextHideFormItem::CreateDefault() { return new XFormTextHideFormItem; }

XFormTextHideFormItem::XFormTextHideFormItem(bool bHide) :
    SfxBoolItem(XATTR_FORMTXTHIDEFORM, bHide)
{
}

XFormTextHideFormItem* XFormTextHideFormItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextHideFormItem(*this);
}

// --- SetItems ---


/// a line attribute set item
XLineAttrSetItem::XLineAttrSetItem( SfxItemSet&& pItemSet ) :
    SfxSetItem( XATTRSET_LINE, std::move(pItemSet))
{
}

XLineAttrSetItem::XLineAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_LINE,
        SfxItemSetFixed<XATTR_LINE_FIRST, XATTR_LINE_LAST>( *pItemPool ))
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

XLineAttrSetItem* XLineAttrSetItem::Clone( SfxItemPool* pPool ) const
{
    return new XLineAttrSetItem( *this, pPool );
}

/// fill attribute set item
XFillAttrSetItem::XFillAttrSetItem( SfxItemSet&& pItemSet ) :
    SfxSetItem( XATTRSET_FILL, std::move(pItemSet))
{
}

XFillAttrSetItem::XFillAttrSetItem( SfxItemPool* pItemPool ) :
    SfxSetItem( XATTRSET_FILL,
        SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST>( *pItemPool ))
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

XFillAttrSetItem* XFillAttrSetItem::Clone( SfxItemPool* pPool ) const
{
    return new XFillAttrSetItem( *this, pPool );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
