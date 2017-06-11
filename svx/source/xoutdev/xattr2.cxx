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

#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <i18nutil/unicode.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/itemtype.hxx>
#include <svx/xdef.hxx>
#include <svx/AffineMatrixItem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <libxml/xmlwriter.h>

XLineTransparenceItem::XLineTransparenceItem(sal_uInt16 nLineTransparence) :
    SfxUInt16Item(XATTR_LINETRANSPARENCE, nLineTransparence)
{
}

XLineTransparenceItem::XLineTransparenceItem(SvStream& rIn) :
    SfxUInt16Item(XATTR_LINETRANSPARENCE, rIn)
{
}

SfxPoolItem* XLineTransparenceItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineTransparenceItem(*this);
}

SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineTransparenceItem(rIn);
}

bool XLineTransparenceItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();

    switch ( ePres )
    {
        case SfxItemPresentation::Complete:
            rText = SvxResId(RID_SVXSTR_TRANSPARENCE) + ": ";
            SAL_FALLTHROUGH;
        case SfxItemPresentation::Nameless:
            rText += unicode::formatPercent(GetValue(),
                Application::GetSettings().GetUILanguageTag());
            return true;
        default:
            return false;
    }
}


SfxPoolItem* XLineJointItem::CreateDefault() { return new XLineJointItem; }

XLineJointItem::XLineJointItem( css::drawing::LineJoint eLineJoint ) :
    SfxEnumItem(XATTR_LINEJOINT, eLineJoint)
{
}

XLineJointItem::XLineJointItem( SvStream& rIn ) :
    SfxEnumItem( XATTR_LINEJOINT, rIn )
{
}

sal_uInt16 XLineJointItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

SfxPoolItem* XLineJointItem::Create( SvStream& rIn, sal_uInt16 nVer ) const
{
    XLineJointItem* pRet = new XLineJointItem( rIn );

    if(nVer < 1)
        pRet->SetValue(css::drawing::LineJoint_ROUND);

    return pRet;
}

SfxPoolItem* XLineJointItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineJointItem( *this );
}

bool XLineJointItem::GetPresentation( SfxItemPresentation /*ePres*/, MapUnit /*eCoreUnit*/,
    MapUnit /*ePresUnit*/, OUString& rText, const IntlWrapper*) const
{
    rText.clear();

    const char* pId = nullptr;

    switch( GetValue() )
    {
        case css::drawing::LineJoint::LineJoint_MAKE_FIXED_SIZE:
        case css::drawing::LineJoint_NONE:
            pId = RID_SVXSTR_NONE;
        break;

        case css::drawing::LineJoint_MIDDLE:
            pId = RID_SVXSTR_LINEJOINT_MIDDLE;
        break;


        case css::drawing::LineJoint_BEVEL:
            pId = RID_SVXSTR_LINEJOINT_BEVEL;
        break;


        case css::drawing::LineJoint_MITER:
            pId = RID_SVXSTR_LINEJOINT_MITER;
        break;


        case css::drawing::LineJoint_ROUND:
            pId = RID_SVXSTR_LINEJOINT_ROUND;
        break;
    }

    if (pId)
        rText = SvxResId(pId);

    return true;
}

bool XLineJointItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    const css::drawing::LineJoint eJoint = GetValue();
    rVal <<= eJoint;
    return true;
}

bool XLineJointItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    css::drawing::LineJoint eUnoJoint;

    if(!(rVal >>= eUnoJoint))
    {
        // also try an int (for Basic)
        sal_Int32 nLJ = 0;
        if(!(rVal >>= nLJ))
            return false;
        eUnoJoint = (css::drawing::LineJoint)nLJ;
    }

    SetValue( eUnoJoint );

    return true;
}

sal_uInt16 XLineJointItem::GetValueCount() const
{
    // don't forget to update the api interface also
    return 5;
}


AffineMatrixItem::AffineMatrixItem(const css::geometry::AffineMatrix2D* pMatrix)
:   SfxPoolItem(SID_ATTR_TRANSFORM_MATRIX)
{
    if(pMatrix)
    {
        maMatrix = *pMatrix;
    }
    else
    {
        maMatrix.m00 = 1.0;
        maMatrix.m01 = 0.0;
        maMatrix.m02 = 0.0;
        maMatrix.m10 = 0.0;
        maMatrix.m11 = 1.0;
        maMatrix.m12 = 0.0;
    }
}

AffineMatrixItem::AffineMatrixItem(SvStream& rIn)
:   SfxPoolItem(SID_ATTR_TRANSFORM_MATRIX)
{
    rIn.ReadDouble( maMatrix.m00 );
    rIn.ReadDouble( maMatrix.m01 );
    rIn.ReadDouble( maMatrix.m02 );
    rIn.ReadDouble( maMatrix.m10 );
    rIn.ReadDouble( maMatrix.m11 );
    rIn.ReadDouble( maMatrix.m12 );
}

AffineMatrixItem::AffineMatrixItem(const AffineMatrixItem& rRef)
:   SfxPoolItem(SID_ATTR_TRANSFORM_MATRIX)
{
    maMatrix = rRef.maMatrix;
}

AffineMatrixItem::~AffineMatrixItem()
{
}

bool AffineMatrixItem::operator==(const SfxPoolItem& rRef) const
{
    if(!SfxPoolItem::operator==(rRef))
    {
        return false;
    }

    const AffineMatrixItem* pRef = dynamic_cast< const AffineMatrixItem* >(&rRef);

    if(!pRef)
    {
        return false;
    }

    return (maMatrix.m00 == pRef->maMatrix.m00
        && maMatrix.m01 == pRef->maMatrix.m01
        && maMatrix.m02 == pRef->maMatrix.m02
        && maMatrix.m10 == pRef->maMatrix.m10
        && maMatrix.m11 == pRef->maMatrix.m11
        && maMatrix.m12 == pRef->maMatrix.m12);
}

SfxPoolItem* AffineMatrixItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new AffineMatrixItem(*this);
}

SfxPoolItem* AffineMatrixItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/ ) const
{
    return new AffineMatrixItem(rIn);
}

SvStream& AffineMatrixItem::Store(SvStream &rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    rStream.WriteDouble( maMatrix.m00 );
    rStream.WriteDouble( maMatrix.m01 );
    rStream.WriteDouble( maMatrix.m02 );
    rStream.WriteDouble( maMatrix.m10 );
    rStream.WriteDouble( maMatrix.m11 );
    rStream.WriteDouble( maMatrix.m12 );
    return rStream;
}

bool AffineMatrixItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= maMatrix;
    return true;
}

bool AffineMatrixItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    if (rVal >>= maMatrix)
    {
        return true;
    }

    OSL_ENSURE(false, "AffineMatrixItem::PutValue - Wrong type!");
    return false;
}


SfxPoolItem* XLineCapItem::CreateDefault() { return new XLineCapItem; }

XLineCapItem::XLineCapItem(css::drawing::LineCap eLineCap)
:   SfxEnumItem(XATTR_LINECAP, eLineCap)
{
}

XLineCapItem::XLineCapItem( SvStream& rIn )
:   SfxEnumItem(XATTR_LINECAP, rIn)
{
}

sal_uInt16 XLineCapItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

SfxPoolItem* XLineCapItem::Create( SvStream& rIn, sal_uInt16 nVer ) const
{
    XLineCapItem* pRet = new XLineCapItem( rIn );

    if(nVer < 1)
        pRet->SetValue(css::drawing::LineCap_BUTT);

    return pRet;
}

SfxPoolItem* XLineCapItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineCapItem( *this );
}

bool XLineCapItem::GetPresentation( SfxItemPresentation /*ePres*/, MapUnit /*eCoreUnit*/,
                                                     MapUnit /*ePresUnit*/, OUString& rText, const IntlWrapper*) const
{
    rText.clear();

    const char* pId = nullptr;

    switch( GetValue() )
    {
        default: /*css::drawing::LineCap_BUTT*/
            pId = RID_SVXSTR_LINECAP_BUTT;
        break;

        case(css::drawing::LineCap_ROUND):
            pId = RID_SVXSTR_LINECAP_ROUND;
        break;

        case(css::drawing::LineCap_SQUARE):
            pId = RID_SVXSTR_LINECAP_SQUARE;
        break;
    }

    if (pId)
        rText = SvxResId(pId);

    return true;
}

bool XLineCapItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    const css::drawing::LineCap eCap(GetValue());
    rVal <<= eCap;
    return true;
}

bool XLineCapItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    css::drawing::LineCap eUnoCap;

    if(!(rVal >>= eUnoCap))
    {
        // also try an int (for Basic)
        sal_Int32 nLJ(0);

        if(!(rVal >>= nLJ))
        {
            return false;
        }

        eUnoCap = (css::drawing::LineCap)nLJ;
    }

    OSL_ENSURE(css::drawing::LineCap_BUTT == eUnoCap
        || css::drawing::LineCap_ROUND == eUnoCap
        || css::drawing::LineCap_SQUARE == eUnoCap, "Unknown enum value in XATTR_LINECAP (!)");

    SetValue(eUnoCap);

    return true;
}

sal_uInt16 XLineCapItem::GetValueCount() const
{
    // don't forget to update the api interface also
    return 3;
}

css::drawing::LineCap XLineCapItem::GetValue() const
{
    const css::drawing::LineCap eRetval((css::drawing::LineCap)SfxEnumItem::GetValue());
    OSL_ENSURE(css::drawing::LineCap_BUTT == eRetval
        || css::drawing::LineCap_ROUND == eRetval
        || css::drawing::LineCap_SQUARE == eRetval, "Unknown enum value in XATTR_LINECAP (!)");

    return eRetval;
}

XFillTransparenceItem::XFillTransparenceItem(sal_uInt16 nFillTransparence) :
    SfxUInt16Item(XATTR_FILLTRANSPARENCE, nFillTransparence)
{
}

XFillTransparenceItem::XFillTransparenceItem(SvStream& rIn) :
    SfxUInt16Item(XATTR_FILLTRANSPARENCE, rIn)
{
}

SfxPoolItem* XFillTransparenceItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillTransparenceItem(*this);
}

SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillTransparenceItem(rIn);
}

bool XFillTransparenceItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();

    switch ( ePres )
    {
        case SfxItemPresentation::Complete:
            rText = SvxResId(RID_SVXSTR_TRANSPARENCE) + ": ";
            SAL_FALLTHROUGH;
        case SfxItemPresentation::Nameless:
            rText += unicode::formatPercent(GetValue(),
                Application::GetSettings().GetUILanguageTag());
            return true;
        default:
            return false;
    }
}

void XFillTransparenceItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("XFillTransparenceItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}


XFormTextShadowTranspItem::XFormTextShadowTranspItem(sal_uInt16 nShdwTransparence) :
    SfxUInt16Item(XATTR_FORMTXTSHDWTRANSP, nShdwTransparence)
{
}

XFormTextShadowTranspItem::XFormTextShadowTranspItem(SvStream& rIn) :
    SfxUInt16Item(XATTR_FORMTXTSHDWTRANSP, rIn)
{
}

SfxPoolItem* XFormTextShadowTranspItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowTranspItem(*this);
}

SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextShadowTranspItem(rIn);
}

// class XFillGradientStepCountItem

XGradientStepCountItem::XGradientStepCountItem( sal_uInt16 nStepCount ) :
    SfxUInt16Item( XATTR_GRADIENTSTEPCOUNT, nStepCount )
{
}

XGradientStepCountItem::XGradientStepCountItem( SvStream& rIn ) :
    SfxUInt16Item( XATTR_GRADIENTSTEPCOUNT, rIn )
{
}

SfxPoolItem* XGradientStepCountItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XGradientStepCountItem( *this );
}

SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XGradientStepCountItem( rIn );
}

bool XGradientStepCountItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();

    rText += OUString::number(GetValue());
    return true;
}


XFillBmpTileItem::XFillBmpTileItem( bool bTile ) :
            SfxBoolItem( XATTR_FILLBMP_TILE, bTile )
{
}

XFillBmpTileItem::XFillBmpTileItem( SvStream& rIn ) :
            SfxBoolItem( XATTR_FILLBMP_TILE, rIn )
{
}

SfxPoolItem* XFillBmpTileItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpTileItem( *this );
}

SfxPoolItem* XFillBmpTileItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpTileItem( rIn );
}

bool XFillBmpTileItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

void XFillBmpTileItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("XFillBmpTileItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class XFillBmpTilePosItem


XFillBmpPosItem::XFillBmpPosItem( RectPoint eRP ) :
    SfxEnumItem( XATTR_FILLBMP_POS, eRP )
{
}

XFillBmpPosItem::XFillBmpPosItem( SvStream& rIn ) :
            SfxEnumItem( XATTR_FILLBMP_POS, rIn )
{
}

SfxPoolItem* XFillBmpPosItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpPosItem( *this );
}

SfxPoolItem* XFillBmpPosItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpPosItem( rIn );
}

bool XFillBmpPosItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

sal_uInt16 XFillBmpPosItem::GetValueCount() const
{
    return 9;
}

void XFillBmpPosItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("XFillBmpPosItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number((int)GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class XFillBmpTileSizeXItem

XFillBmpSizeXItem::XFillBmpSizeXItem( long nSizeX ) :
            SfxMetricItem( XATTR_FILLBMP_SIZEX, nSizeX )
{
}

XFillBmpSizeXItem::XFillBmpSizeXItem( SvStream& rIn ) :
            SfxMetricItem( XATTR_FILLBMP_SIZEX, rIn )
{
}

SfxPoolItem* XFillBmpSizeXItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpSizeXItem( *this );
}

SfxPoolItem* XFillBmpSizeXItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpSizeXItem( rIn );
}

bool XFillBmpSizeXItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

bool XFillBmpSizeXItem::HasMetrics() const
{
    return GetValue() > 0L;
}

// class XFillBmpTileSizeYItem


XFillBmpSizeYItem::XFillBmpSizeYItem( long nSizeY ) :
            SfxMetricItem( XATTR_FILLBMP_SIZEY, nSizeY )
{
}

XFillBmpSizeYItem::XFillBmpSizeYItem( SvStream& rIn ) :
            SfxMetricItem( XATTR_FILLBMP_SIZEY, rIn )
{
}

SfxPoolItem* XFillBmpSizeYItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpSizeYItem( *this );
}

SfxPoolItem* XFillBmpSizeYItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpSizeYItem( rIn );
}

bool XFillBmpSizeYItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

bool XFillBmpSizeYItem::HasMetrics() const
{
    return GetValue() > 0L;
}

// class XFillBmpTileLogItem

XFillBmpSizeLogItem::XFillBmpSizeLogItem( bool bLog ) :
            SfxBoolItem( XATTR_FILLBMP_SIZELOG, bLog )
{
}

XFillBmpSizeLogItem::XFillBmpSizeLogItem( SvStream& rIn ) :
            SfxBoolItem( XATTR_FILLBMP_SIZELOG, rIn )
{
}

SfxPoolItem* XFillBmpSizeLogItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpSizeLogItem( *this );
}

SfxPoolItem* XFillBmpSizeLogItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpSizeLogItem( rIn );
}

bool XFillBmpSizeLogItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

// class XFillBmpTileOffXItem


XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( sal_uInt16 nOffX ) :
            SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETX, nOffX )
{
}

XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( SvStream& rIn ) :
            SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETX, rIn )
{
}

SfxPoolItem* XFillBmpTileOffsetXItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpTileOffsetXItem( *this );
}

SfxPoolItem* XFillBmpTileOffsetXItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpTileOffsetXItem( rIn );
}

bool XFillBmpTileOffsetXItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

// class XFillBmpTileOffYItem

XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( sal_uInt16 nOffY ) :
            SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETY, nOffY )
{
}

XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( SvStream& rIn ) :
            SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETY, rIn )
{
}

SfxPoolItem* XFillBmpTileOffsetYItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpTileOffsetYItem( *this );
}

SfxPoolItem* XFillBmpTileOffsetYItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpTileOffsetYItem( rIn );
}

bool XFillBmpTileOffsetYItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

XFillBmpStretchItem::XFillBmpStretchItem( bool bStretch ) :
            SfxBoolItem( XATTR_FILLBMP_STRETCH, bStretch )
{
}

XFillBmpStretchItem::XFillBmpStretchItem( SvStream& rIn ) :
            SfxBoolItem( XATTR_FILLBMP_STRETCH, rIn )
{
}

SfxPoolItem* XFillBmpStretchItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpStretchItem( *this );
}

SfxPoolItem* XFillBmpStretchItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpStretchItem( rIn );
}

bool XFillBmpStretchItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

void XFillBmpStretchItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("XFillBmpStretchItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class XFillBmpTileOffPosXItem

XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( sal_uInt16 nOffPosX ) :
            SfxUInt16Item( XATTR_FILLBMP_POSOFFSETX, nOffPosX )
{
}

XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( SvStream& rIn ) :
            SfxUInt16Item( XATTR_FILLBMP_POSOFFSETX, rIn )
{
}

SfxPoolItem* XFillBmpPosOffsetXItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpPosOffsetXItem( *this );
}

SfxPoolItem* XFillBmpPosOffsetXItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpPosOffsetXItem( rIn );
}

bool XFillBmpPosOffsetXItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

// class XFillBmpTileOffPosYItem

XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( sal_uInt16 nOffPosY ) :
            SfxUInt16Item( XATTR_FILLBMP_POSOFFSETY, nOffPosY )
{
}

XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( SvStream& rIn ) :
            SfxUInt16Item( XATTR_FILLBMP_POSOFFSETY, rIn )
{
}

SfxPoolItem* XFillBmpPosOffsetYItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpPosOffsetYItem( *this );
}

SfxPoolItem* XFillBmpPosOffsetYItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpPosOffsetYItem( rIn );
}

bool XFillBmpPosOffsetYItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return true;
}

XFillBackgroundItem::XFillBackgroundItem( bool bFill ) :
    SfxBoolItem( XATTR_FILLBACKGROUND, bFill )
{
}

XFillBackgroundItem::XFillBackgroundItem( SvStream& rIn ) :
    SfxBoolItem( XATTR_FILLBACKGROUND, rIn )
{
}

SfxPoolItem* XFillBackgroundItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBackgroundItem( *this );
}

SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBackgroundItem( rIn );
}

bool XFillBackgroundItem::GetPresentation( SfxItemPresentation /*ePres*/, MapUnit /*eCoreUnit*/,
                                           MapUnit /*ePresUnit*/, OUString& rText, const IntlWrapper*) const
{
    rText.clear();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
