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

#include <svx/svxids.hrc>
#include <svx/rulritem.hxx>
#include <svx/unomid.hxx>
#include <tools/debug.hxx>
#include <tools/mapunit.hxx>
#include <tools/UnitConversion.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/status/LeftRightMargin.hpp>
#include <com/sun/star/frame/status/UpperLowerMargin.hpp>

SfxPoolItem* SvxPagePosSizeItem::CreateDefault() { return new SvxPagePosSizeItem; }
SfxPoolItem* SvxLongLRSpaceItem::CreateDefault() { return new SvxLongLRSpaceItem; }
SfxPoolItem* SvxLongULSpaceItem::CreateDefault() { return new SvxLongULSpaceItem; }
SfxPoolItem* SvxColumnItem::CreateDefault() { return new SvxColumnItem; }
SfxPoolItem* SvxObjectItem::CreateDefault() { SAL_WARN( "svx", "No SvxObjectItem factory available"); return nullptr; }

/* SvxLongLRSpaceItem */

bool SvxLongLRSpaceItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        mlLeft  == static_cast<const SvxLongLRSpaceItem &>(rCmp).mlLeft &&
        mlRight == static_cast<const SvxLongLRSpaceItem &>(rCmp).mlRight;
}

bool SvxLongLRSpaceItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch( nMemberId )
    {
        case 0:
        {
            css::frame::status::LeftRightMargin aLeftRightMargin;
            aLeftRightMargin.Left = bConvert ? convertTwipToMm100( mlLeft ) : mlLeft;
            aLeftRightMargin.Right = bConvert ? convertTwipToMm100( mlRight ) : mlRight;
            rVal <<= aLeftRightMargin;
            return true;
        }

        case MID_LEFT:
            nVal = mlLeft;
            break;
        case MID_RIGHT:
            nVal = mlRight;
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
            return false;
    }

    if ( bConvert )
        nVal = convertTwipToMm100( nVal );

    rVal <<= nVal;
    return true;
}

bool SvxLongLRSpaceItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        css::frame::status::LeftRightMargin aLeftRightMargin;
        if ( rVal >>= aLeftRightMargin )
        {
            mlLeft  = bConvert ? o3tl::toTwips(aLeftRightMargin.Left, o3tl::Length::mm100) : aLeftRightMargin.Left;
            mlRight = bConvert ? o3tl::toTwips(aLeftRightMargin.Right, o3tl::Length::mm100) : aLeftRightMargin.Right;
            return true;
        }
    }
    else if ( rVal >>= nVal )
    {
        if ( bConvert )
            nVal = o3tl::toTwips(nVal, o3tl::Length::mm100);

        switch( nMemberId )
        {
            case MID_LEFT:
                mlLeft = nVal;
                break;
            case MID_RIGHT:
                mlRight = nVal;
                break;
            default:
                OSL_FAIL("Wrong MemberId!");
                return false;
        }

        return true;
    }

    return false;
}

bool SvxLongLRSpaceItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        MapUnit             /*eCoreUnit*/,
                        MapUnit             /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper&  /*rWrapper*/) const
{
    return false;
}

SvxLongLRSpaceItem* SvxLongLRSpaceItem::Clone(SfxItemPool *) const
{
    return new SvxLongLRSpaceItem(*this);
}

SvxLongLRSpaceItem::SvxLongLRSpaceItem(tools::Long lLeft, tools::Long lRight, TypedWhichId<SvxLongLRSpaceItem> nId) :
    SfxPoolItem (nId),
    mlLeft      (lLeft),
    mlRight     (lRight)
{}

SvxLongLRSpaceItem::SvxLongLRSpaceItem() :
    SfxPoolItem (0),
    mlLeft      (0),
    mlRight     (0)
{}

void SvxLongLRSpaceItem::SetLeft(tools::Long lArgLeft)
{
    mlLeft = lArgLeft;
}

void SvxLongLRSpaceItem::SetRight(tools::Long lArgRight)
{
    mlRight = lArgRight;
}

/* SvxLongULSpaceItem */

bool SvxLongULSpaceItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        mlLeft == static_cast<const SvxLongULSpaceItem&>(rCmp).mlLeft &&
        mlRight == static_cast<const SvxLongULSpaceItem&>(rCmp).mlRight;
}

bool SvxLongULSpaceItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch( nMemberId )
    {
        case 0:
        {
            css::frame::status::UpperLowerMargin aUpperLowerMargin;
            aUpperLowerMargin.Upper = bConvert ? convertTwipToMm100( mlLeft )  : mlLeft;
            aUpperLowerMargin.Lower = bConvert ? convertTwipToMm100( mlRight ) : mlRight;
            rVal <<= aUpperLowerMargin;
            return true;
        }

        case MID_UPPER:
            nVal = mlLeft;
            break;
        case MID_LOWER:
            nVal = mlRight;
            break;
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    if ( bConvert )
        nVal = convertTwipToMm100( nVal );

    rVal <<= nVal;
    return true;
}

bool SvxLongULSpaceItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        css::frame::status::UpperLowerMargin aUpperLowerMargin;
        if ( rVal >>= aUpperLowerMargin )
        {
            mlLeft    = bConvert ? o3tl::toTwips(aUpperLowerMargin.Upper, o3tl::Length::mm100) : aUpperLowerMargin.Upper;
            mlRight   = bConvert ? o3tl::toTwips(aUpperLowerMargin.Lower, o3tl::Length::mm100) : aUpperLowerMargin.Lower;
            return true;
        }
    }
    else if ( rVal >>= nVal )
    {
        if ( bConvert )
            nVal = o3tl::toTwips(nVal, o3tl::Length::mm100);

        switch( nMemberId )
        {
            case MID_UPPER:
                mlLeft = nVal;
                break;
            case MID_LOWER:
                mlRight = nVal;
                break;
            default:
                OSL_FAIL("Wrong MemberId!");
                return false;
        }

        return true;
    }

    return false;
}

bool SvxLongULSpaceItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        MapUnit             /*eCoreUnit*/,
                        MapUnit             /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper&  /*rWrapper*/ ) const
{
    return false;
}

SvxLongULSpaceItem* SvxLongULSpaceItem::Clone(SfxItemPool *) const
{
    return new SvxLongULSpaceItem(*this);
}

SvxLongULSpaceItem::SvxLongULSpaceItem(tools::Long lLeft, tools::Long lRight, TypedWhichId<SvxLongULSpaceItem> nId) :
    SfxPoolItem (nId),
    mlLeft       (lLeft),
    mlRight      (lRight)
{}

SvxLongULSpaceItem::SvxLongULSpaceItem() :
    SfxPoolItem (0),
    mlLeft      (0),
    mlRight     (0)
{}


void SvxLongULSpaceItem::SetUpper(tools::Long lArgLeft)
{
    mlLeft = lArgLeft;
}

void SvxLongULSpaceItem::SetLower(tools::Long lArgRight)
{
    mlRight = lArgRight;
}

/* SvxPagePosSizeItem */

bool SvxPagePosSizeItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        aPos == static_cast<const SvxPagePosSizeItem &>(rCmp).aPos &&
            lWidth == static_cast<const SvxPagePosSizeItem &>(rCmp).lWidth  &&
            lHeight == static_cast<const SvxPagePosSizeItem &>(rCmp).lHeight;
}

bool SvxPagePosSizeItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch ( nMemberId )
    {
        case 0 :
        {
            css::awt::Rectangle aPagePosSize;
            aPagePosSize.X = aPos.X();
            aPagePosSize.Y = aPos.Y();
            aPagePosSize.Width = lWidth;
            aPagePosSize.Height = lHeight;
            rVal <<= aPagePosSize;
            return true;
        }

        case MID_X: nVal = aPos.X(); break;
        case MID_Y: nVal = aPos.Y(); break;
        case MID_WIDTH: nVal = lWidth; break;
        case MID_HEIGHT: nVal = lHeight; break;

        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    rVal <<= nVal;
    return true;
}

bool SvxPagePosSizeItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        css::awt::Rectangle aPagePosSize;
        if ( rVal >>= aPagePosSize )
        {
            aPos.setX( aPagePosSize.X );
            aPos.setY( aPagePosSize.Y );
            lWidth   = aPagePosSize.Width;
            lHeight  = aPagePosSize.Height;
            return true;
        }
        else
            return false;
    }
    else if ( rVal >>= nVal )
    {
        switch ( nMemberId )
        {
            case MID_X: aPos.setX( nVal ); break;
            case MID_Y: aPos.setY( nVal ); break;
            case MID_WIDTH: lWidth = nVal; break;
            case MID_HEIGHT: lHeight = nVal; break;

            default: OSL_FAIL("Wrong MemberId!"); return false;
        }

        return true;
    }

    return false;
}

bool SvxPagePosSizeItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        MapUnit             /*eCoreUnit*/,
                        MapUnit             /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper&  /*rWrapper*/ ) const
{
    return false;
}

SvxPagePosSizeItem* SvxPagePosSizeItem::Clone(SfxItemPool *) const
{
    return new SvxPagePosSizeItem(*this);
}

SvxPagePosSizeItem::SvxPagePosSizeItem(const Point &rP, tools::Long lW, tools::Long lH) :
    SfxPoolItem (SID_RULER_PAGE_POS),
    aPos        (rP),
    lWidth      (lW),
    lHeight     (lH)
{}

SvxPagePosSizeItem::SvxPagePosSizeItem() :
    SfxPoolItem (0),
    aPos        (0, 0),
    lWidth      (0),
    lHeight     (0)
{}

/* SvxColumnItem */

bool SvxColumnItem::operator==(const SfxPoolItem& rCmp) const
{
    if(!SfxPoolItem::operator==(rCmp) ||
       nActColumn != static_cast<const SvxColumnItem&>(rCmp).nActColumn ||
       nLeft != static_cast<const SvxColumnItem&>(rCmp).nLeft ||
       nRight != static_cast<const SvxColumnItem&>(rCmp).nRight ||
       bTable != static_cast<const SvxColumnItem&>(rCmp).bTable ||
       Count() != static_cast<const SvxColumnItem&>(rCmp).Count())
        return false;

    const sal_uInt16 nCount = static_cast<const SvxColumnItem&>(rCmp).Count();
    for(sal_uInt16 i = 0; i < nCount;++i)
    {
        if( (*this)[i] != static_cast<const SvxColumnItem&>(rCmp)[i] )
            return false;
    }
    return true;
}

SvxColumnItem::SvxColumnItem( sal_uInt16 nAct ) :
    SfxPoolItem (SID_RULER_BORDERS ),
    nLeft       (0),
    nRight      (0),
    nActColumn  (nAct),
    bTable      (false),
    bOrtho      (true)

{}

SvxColumnItem::SvxColumnItem( sal_uInt16 nActCol, sal_uInt16 left, sal_uInt16 right ) :
    SfxPoolItem (SID_RULER_BORDERS),
    nLeft       (left),
    nRight      (right),
    nActColumn  (nActCol),
    bTable      (true),
    bOrtho      (true)
{}

bool SvxColumnItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        MapUnit             /*eCoreUnit*/,
                        MapUnit             /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper&  /*rWrapper*/ ) const
{
    return false;
}

SvxColumnItem* SvxColumnItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxColumnItem(*this);
}

bool SvxColumnItem::CalcOrtho() const
{
    const sal_uInt16 nCount = Count();
    DBG_ASSERT(nCount >= 2, "no columns");
    if(nCount < 2)
        return false;

    tools::Long nColWidth = (*this)[0].GetWidth();
    for(sal_uInt16 i = 1; i < nCount; ++i) {
        if( (*this)[i].GetWidth() != nColWidth)
            return false;
    }
    //!! Wide divider
    return true;
}

bool SvxColumnItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
            // SfxDispatchController_Impl::StateChanged calls this with hardcoded 0 triggering this;
            SAL_INFO("svx", "SvxColumnItem::QueryValue with nMemberId of 0");
            return false;
        case MID_COLUMNARRAY:
            return false;
        case MID_RIGHT:
            rVal <<= nRight;
            break;
        case MID_LEFT:
            rVal <<= nLeft;
            break;
        case MID_ORTHO:
            rVal <<= bOrtho;
            break;
        case MID_ACTUAL:
            rVal <<= static_cast<sal_Int32>(nActColumn);
            break;
        case MID_TABLE:
            rVal <<= bTable;
            break;
        default:
            SAL_WARN("svx", "Wrong MemberId!");
            return false;
    }

    return true;
}

bool SvxColumnItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal = 0;
    switch ( nMemberId )
    {
        case MID_COLUMNARRAY:
        {
            return false;
        }
        case MID_RIGHT:
            rVal >>= nRight;
            break;
        case MID_LEFT:
            rVal >>= nLeft;
            break;
        case MID_ORTHO:
            rVal >>= nVal;
            bOrtho = static_cast<bool>(nVal);
            break;
        case MID_ACTUAL:
            rVal >>= nVal;
            nActColumn = static_cast<sal_uInt16>(nVal);
            break;
        case MID_TABLE:
            rVal >>= nVal;
            bTable = static_cast<bool>(nVal);
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
            return false;
    }

    return true;
}

sal_uInt16 SvxColumnItem::Count() const
{
    return aColumns.size();
}

SvxColumnDescription& SvxColumnItem::At(sal_uInt16 index)
{
    return aColumns[index];
}

SvxColumnDescription& SvxColumnItem::GetActiveColumnDescription()
{
    return aColumns[GetActColumn()];
}

SvxColumnDescription& SvxColumnItem::operator[](sal_uInt16 index)
{
    return aColumns[index];
}

const SvxColumnDescription& SvxColumnItem::operator[](sal_uInt16 index) const
{
    return aColumns[index];
}

void SvxColumnItem::Append(const SvxColumnDescription &rDesc)
{
    aColumns.push_back(rDesc);
}

void SvxColumnItem::SetLeft(tools::Long left)
{
    nLeft = left;
}

void SvxColumnItem::SetRight(tools::Long right)
{
    nRight = right;
}


bool SvxColumnItem::IsFirstAct() const
{
    return nActColumn == 0;
}

bool SvxColumnItem::IsLastAct() const
{
    return nActColumn == Count() - 1;
}

SvxColumnDescription::SvxColumnDescription(tools::Long start, tools::Long end, bool bVis) :
    nStart   (start),
    nEnd     (end),
    bVisible (bVis),
    nEndMin  (0),
    nEndMax  (0)
{}

SvxColumnDescription::SvxColumnDescription(tools::Long start, tools::Long end, tools::Long endMin, tools::Long endMax, bool bVis) :
    nStart   (start),
    nEnd     (end),
    bVisible (bVis),
    // fdo#85858 hack: clamp these to smaller value to prevent overflow
    nEndMin(std::min<tools::Long>(endMin, std::numeric_limits<unsigned short>::max())),
    nEndMax(std::min<tools::Long>(endMax, std::numeric_limits<unsigned short>::max()))
{}

bool SvxColumnDescription::operator==(const SvxColumnDescription& rCmp) const
{
    return nStart   == rCmp.nStart
        && bVisible == rCmp.bVisible
        && nEnd     == rCmp.nEnd
        && nEndMin  == rCmp.nEndMin
        && nEndMax  == rCmp.nEndMax;
}

bool SvxColumnDescription::operator!=(const SvxColumnDescription& rCmp) const
{
    return !operator==(rCmp);
}

tools::Long SvxColumnDescription::GetWidth() const
{
    return nEnd - nStart;
}

/* SvxColumnItem */
void SvxColumnItem::SetOrtho(bool bVal)
{
    bOrtho = bVal;
}

bool SvxColumnItem::IsConsistent() const
{
    return nActColumn < aColumns.size();
}

bool SvxObjectItem::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxPoolItem::operator==(rCmp) &&
       nStartX == static_cast<const SvxObjectItem&>(rCmp).nStartX &&
       nEndX == static_cast<const SvxObjectItem&>(rCmp).nEndX &&
       nStartY == static_cast<const SvxObjectItem&>(rCmp).nStartY &&
       nEndY == static_cast<const SvxObjectItem&>(rCmp).nEndY &&
       bLimits == static_cast<const SvxObjectItem&>(rCmp).bLimits;
}

bool SvxObjectItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        MapUnit             /*eCoreUnit*/,
                        MapUnit             /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper&  /*rWrapper*/ ) const
{
    return false;
}

SvxObjectItem* SvxObjectItem::Clone(SfxItemPool *) const
{
    return new SvxObjectItem(*this);
}

SvxObjectItem::SvxObjectItem( tools::Long nSX, tools::Long nEX,
                              tools::Long nSY, tools::Long nEY ) :
    SfxPoolItem (SID_RULER_OBJECT),
    nStartX     (nSX),
    nEndX       (nEX),
    nStartY     (nSY),
    nEndY       (nEY),
    bLimits     (false)
{}

bool SvxObjectItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case MID_START_X:
            rVal <<= nStartX;
            break;
        case MID_START_Y:
            rVal <<= nStartY;
            break;
        case MID_END_X:
            rVal <<= nEndX;
            break;
        case MID_END_Y:
            rVal <<= nEndY;
            break;
        case MID_LIMIT:
            rVal <<= bLimits;
            break;
        default:
            OSL_FAIL( "Wrong MemberId" );
            return false;
    }

    return true;
}

bool SvxObjectItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = false;
    switch (nMemberId)
    {
        case MID_START_X:
            bRet = (rVal >>= nStartX);
            break;
        case MID_START_Y:
            bRet = (rVal >>= nStartY);
            break;
        case MID_END_X:
            bRet = (rVal >>= nEndX);
            break;
        case MID_END_Y:
            bRet = (rVal >>= nEndY);
            break;
        case MID_LIMIT:
            bRet = (rVal >>= bLimits);
            break;
        default: OSL_FAIL( "Wrong MemberId" );
    }

    return bRet;
}


void SvxObjectItem::SetStartX(tools::Long lValue)
{
    nStartX = lValue;
}

void SvxObjectItem::SetEndX(tools::Long lValue)
{
    nEndX = lValue;
}

void SvxObjectItem::SetStartY(tools::Long lValue)
{
    nStartY = lValue;
}

void SvxObjectItem::SetEndY(tools::Long lValue)
{
    nEndY = lValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
