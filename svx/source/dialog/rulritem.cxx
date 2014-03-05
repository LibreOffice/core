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

#include <svx/dialogs.hrc>
#include "svx/rulritem.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/status/LeftRightMargin.hpp>
#include <com/sun/star/frame/status/UpperLowerMargin.hpp>

TYPEINIT1_AUTOFACTORY(SvxPagePosSizeItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxLongLRSpaceItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxLongULSpaceItem, SfxPoolItem);
TYPEINIT1(SvxColumnItem, SfxPoolItem);
TYPEINIT1(SvxObjectItem, SfxPoolItem);

/* SvxLongLRSpaceItem */

bool SvxLongLRSpaceItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        mlLeft  == ((const SvxLongLRSpaceItem &)rCmp).mlLeft &&
        mlRight == ((const SvxLongLRSpaceItem &)rCmp).mlRight;
}

OUString SvxLongLRSpaceItem::GetValueText() const
{
    return OUString();
}

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

bool SvxLongLRSpaceItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch( nMemberId )
    {
        case 0:
        {
            ::com::sun::star::frame::status::LeftRightMargin aLeftRightMargin;
            aLeftRightMargin.Left = bConvert ? TWIP_TO_MM100( mlLeft ) : mlLeft;
            aLeftRightMargin.Right = bConvert ? TWIP_TO_MM100( mlRight ) : mlRight;
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
        nVal = TWIP_TO_MM100( nVal );

    rVal <<= nVal;
    return true;
}

bool SvxLongLRSpaceItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        ::com::sun::star::frame::status::LeftRightMargin aLeftRightMargin;
        if ( rVal >>= aLeftRightMargin )
        {
            mlLeft  = bConvert ? MM100_TO_TWIP(aLeftRightMargin.Left) : aLeftRightMargin.Left;
            mlRight = bConvert ? MM100_TO_TWIP(aLeftRightMargin.Right) : aLeftRightMargin.Right;
            return true;
        }
    }
    else if ( rVal >>= nVal )
    {
        if ( bConvert )
            nVal = MM100_TO_TWIP( nVal );

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

SfxItemPresentation SvxLongLRSpaceItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        SfxMapUnit          /*eCoreUnit*/,
                        SfxMapUnit          /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper*  /*pWrapper*/) const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxPoolItem* SvxLongLRSpaceItem::Clone(SfxItemPool *) const
{
    return new SvxLongLRSpaceItem(*this);
}

SvxLongLRSpaceItem::SvxLongLRSpaceItem(long lLeft, long lRight, sal_uInt16 nId) :
    SfxPoolItem (nId),
    mlLeft      (lLeft),
    mlRight     (lRight)
{}

SvxLongLRSpaceItem::SvxLongLRSpaceItem() :
    SfxPoolItem (0),
    mlLeft      (0),
    mlRight     (0)
{}

SvxLongLRSpaceItem::SvxLongLRSpaceItem(const SvxLongLRSpaceItem &rCpy) :
    SfxPoolItem (rCpy),
    mlLeft      (rCpy.mlLeft),
    mlRight     (rCpy.mlRight)
{}

long SvxLongLRSpaceItem::GetLeft() const
{
    return mlLeft;
}

long SvxLongLRSpaceItem::GetRight() const
{
    return mlRight;
}

void SvxLongLRSpaceItem::SetLeft(long lArgLeft)
{
    mlLeft = lArgLeft;
}

void SvxLongLRSpaceItem::SetRight(long lArgRight)
{
    mlRight = lArgRight;
}

/* SvxLongULSpaceItem */

bool SvxLongULSpaceItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        mlLeft == ((const SvxLongULSpaceItem&)rCmp).mlLeft &&
        mlRight == ((const SvxLongULSpaceItem&)rCmp).mlRight;
}


OUString SvxLongULSpaceItem::GetValueText() const
{
    return OUString();
}

bool SvxLongULSpaceItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch( nMemberId )
    {
        case 0:
        {
            ::com::sun::star::frame::status::UpperLowerMargin aUpperLowerMargin;
            aUpperLowerMargin.Upper = bConvert ? TWIP_TO_MM100( mlLeft )  : mlLeft;
            aUpperLowerMargin.Lower = bConvert ? TWIP_TO_MM100( mlRight ) : mlRight;
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
        nVal = TWIP_TO_MM100( nVal );

    rVal <<= nVal;
    return true;
}

bool SvxLongULSpaceItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        ::com::sun::star::frame::status::UpperLowerMargin aUpperLowerMargin;
        if ( rVal >>= aUpperLowerMargin )
        {
            mlLeft    = bConvert ? MM100_TO_TWIP( aUpperLowerMargin.Upper ) : aUpperLowerMargin.Upper;
            mlRight   = bConvert ? MM100_TO_TWIP( aUpperLowerMargin.Lower ) : aUpperLowerMargin.Lower;
            return true;
        }
    }
    else if ( rVal >>= nVal )
    {
        if ( bConvert )
            nVal = MM100_TO_TWIP( nVal );

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

SfxItemPresentation SvxLongULSpaceItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        SfxMapUnit          /*eCoreUnit*/,
                        SfxMapUnit          /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper*  /*pWrapper*/ ) const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxPoolItem* SvxLongULSpaceItem::Clone(SfxItemPool *) const
{
    return new SvxLongULSpaceItem(*this);
}

SvxLongULSpaceItem::SvxLongULSpaceItem(long lLeft, long lRight, sal_uInt16 nId) :
    SfxPoolItem (nId),
    mlLeft       (lLeft),
    mlRight      (lRight)
{}

SvxLongULSpaceItem::SvxLongULSpaceItem(const SvxLongULSpaceItem &rCpy) :
    SfxPoolItem (rCpy),
    mlLeft      (rCpy.mlLeft),
    mlRight     (rCpy.mlRight)
{}

SvxLongULSpaceItem::SvxLongULSpaceItem() :
    SfxPoolItem (0),
    mlLeft      (0),
    mlRight     (0)
{}

long SvxLongULSpaceItem::GetUpper() const
{
    return mlLeft;
}

long SvxLongULSpaceItem::GetLower() const
{
    return mlRight;
}

void SvxLongULSpaceItem::SetUpper(long lArgLeft)
{
    mlLeft = lArgLeft;
}

void SvxLongULSpaceItem::SetLower(long lArgRight)
{
    mlRight = lArgRight;
}

/* SvxPagePosSizeItem */

bool SvxPagePosSizeItem::operator==( const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
        aPos == ((const SvxPagePosSizeItem &)rCmp).aPos &&
            lWidth == ((const SvxPagePosSizeItem &)rCmp).lWidth  &&
            lHeight == ((const SvxPagePosSizeItem &)rCmp).lHeight;
}

bool SvxPagePosSizeItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch ( nMemberId )
    {
        case 0 :
        {
            com::sun::star::awt::Rectangle aPagePosSize;
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

bool SvxPagePosSizeItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        com::sun::star::awt::Rectangle aPagePosSize;
        if ( rVal >>= aPagePosSize )
        {
            aPos.X() = aPagePosSize.X;
            aPos.Y() = aPagePosSize.Y;
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
            case MID_X: aPos.X() = nVal; break;
            case MID_Y: aPos.Y() = nVal; break;
            case MID_WIDTH: lWidth = nVal; break;
            case MID_HEIGHT: lHeight = nVal; break;

            default: OSL_FAIL("Wrong MemberId!"); return false;
        }

        return true;
    }

    return false;
}

OUString SvxPagePosSizeItem::GetValueText() const
{
    return OUString();
}

SfxItemPresentation SvxPagePosSizeItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        SfxMapUnit          /*eCoreUnit*/,
                        SfxMapUnit          /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper*  /*pWrapper*/ ) const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxPoolItem* SvxPagePosSizeItem::Clone(SfxItemPool *) const
{
    return new SvxPagePosSizeItem(*this);
}

SvxPagePosSizeItem::SvxPagePosSizeItem(const Point &rP, long lW, long lH) :
    SfxPoolItem (SID_RULER_PAGE_POS),
    aPos        (rP),
    lWidth      (lW),
    lHeight     (lH)
{}

SvxPagePosSizeItem::SvxPagePosSizeItem(const SvxPagePosSizeItem &rCpy) :
    SfxPoolItem (rCpy),
    aPos        (rCpy.aPos),
    lWidth      (rCpy.lWidth),
    lHeight     (rCpy.lHeight)
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
       nActColumn != ((const SvxColumnItem&)rCmp).nActColumn ||
       nLeft != ((const SvxColumnItem&)rCmp).nLeft ||
       nRight != ((const SvxColumnItem&)rCmp).nRight ||
       bTable != ((const SvxColumnItem&)rCmp).bTable ||
       Count() != ((const SvxColumnItem&)rCmp).Count())
        return false;

    const sal_uInt16 nCount = ((const SvxColumnItem&)rCmp).Count();
    for(sal_uInt16 i = 0; i < nCount;++i)
    {
        if( (*this)[i] != ((const SvxColumnItem&)rCmp)[i] )
            return false;
    }
    return true;
}

SvxColumnItem::SvxColumnItem( sal_uInt16 nAct ) :
    SfxPoolItem (SID_RULER_BORDERS),
    nLeft       (0),
    nRight      (0),
    nActColumn  (nAct),
    bTable      (sal_False),
    bOrtho      (sal_True)

{}

SvxColumnItem::SvxColumnItem( sal_uInt16 nActCol, sal_uInt16 left, sal_uInt16 right ) :
    SfxPoolItem (SID_RULER_BORDERS),
    nLeft       (left),
    nRight      (right),
    nActColumn  (nActCol),
    bTable      (sal_True),
    bOrtho      (sal_True)
{}

SvxColumnItem::SvxColumnItem( const SvxColumnItem& rCopy ) :
    SfxPoolItem (rCopy),
    nLeft       (rCopy.nLeft),
    nRight      (rCopy.nRight),
    nActColumn  (rCopy.nActColumn),
    bTable      (rCopy.bTable),
    bOrtho      (rCopy.bOrtho)
{
    aColumns.resize(rCopy.aColumns.size());
    std::copy(rCopy.aColumns.begin(), rCopy.aColumns.end(), aColumns.begin());
}

SvxColumnItem::~SvxColumnItem()
{}

OUString SvxColumnItem::GetValueText() const
{
    return OUString();
}

SfxItemPresentation SvxColumnItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        SfxMapUnit          /*eCoreUnit*/,
                        SfxMapUnit          /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper*  /*pWrapper*/ ) const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxPoolItem* SvxColumnItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxColumnItem(*this);
}

const SvxColumnItem& SvxColumnItem::operator=(const SvxColumnItem& rCopy)
{
    nLeft = rCopy.nLeft;
    nRight = rCopy.nRight;
    bTable = rCopy.bTable;
    nActColumn = rCopy.nActColumn;
    aColumns.resize(rCopy.aColumns.size());

    std::copy(rCopy.aColumns.begin(), rCopy.aColumns.end(), aColumns.begin());

    return *this;
}

bool SvxColumnItem::CalcOrtho() const
{
    const sal_uInt16 nCount = Count();
    DBG_ASSERT(nCount >= 2, "no columns");
    if(nCount < 2)
        return false;

    long nColWidth = (*this)[0].GetWidth();
    for(sal_uInt16 i = 1; i < nCount; ++i) {
        if( (*this)[i].GetWidth() != nColWidth)
            return false;
    }
    //!! Wide divider
    return true;
}

bool SvxColumnItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_COLUMNARRAY:
            return false;
        case MID_RIGHT:
            rVal <<= nRight;
            break;
        case MID_LEFT:
            rVal <<= nLeft;
            break;
        case MID_ORTHO:
            rVal <<= (sal_Bool) bOrtho;
            break;
        case MID_ACTUAL:
            rVal <<= (sal_Int32) nActColumn;
            break;
        case MID_TABLE:
            rVal <<= (sal_Bool) bTable;
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
            return false;
    }

    return true;
}

bool SvxColumnItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
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
            bOrtho = (sal_Bool) nVal;
            break;
        case MID_ACTUAL:
            rVal >>= nVal;
            nActColumn = (sal_uInt16) nVal;
            break;
        case MID_TABLE:
            rVal >>= nVal;
            bTable = (sal_Bool) nVal;
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

void SvxColumnItem::Insert(const SvxColumnDescription &rDesc, sal_uInt16 nPos)
{
    aColumns.insert(aColumns.begin() + nPos, rDesc);
}

void SvxColumnItem::Append(const SvxColumnDescription &rDesc)
{
    Insert(rDesc, Count());
}

void SvxColumnItem::SetLeft(long left)
{
    nLeft = left;
}

void SvxColumnItem::SetRight(long right)
{
    nRight = right;
}

sal_uInt16 SvxColumnItem::GetActColumn() const
{
    return nActColumn;
}

bool SvxColumnItem::IsFirstAct() const
{
    return nActColumn == 0;
}

bool SvxColumnItem::IsLastAct() const
{
    return nActColumn == Count() - 1;
}

long SvxColumnItem::GetLeft()
{
    return nLeft;
}

long SvxColumnItem::GetRight()
{
    return nRight;
}

bool SvxColumnItem::IsTable() const
{
    return bTable;
}

/* SvxColumnDescription */

SvxColumnDescription::SvxColumnDescription() :
    nStart   (0),
    nEnd     (0),
    bVisible (true),
    nEndMin  (0),
    nEndMax  (0)
{}

SvxColumnDescription::SvxColumnDescription(const SvxColumnDescription &rCopy) :
    nStart   (rCopy.nStart),
    nEnd     (rCopy.nEnd),
    bVisible (rCopy.bVisible),
    nEndMin  (rCopy.nEndMin),
    nEndMax  (rCopy.nEndMax)
{}

SvxColumnDescription::SvxColumnDescription(long start, long end, bool bVis) :
    nStart   (start),
    nEnd     (end),
    bVisible (bVis),
    nEndMin  (0),
    nEndMax  (0)
{}

SvxColumnDescription::SvxColumnDescription(long start, long end, long endMin, long endMax, bool bVis) :
    nStart   (start),
    nEnd     (end),
    bVisible (bVis),
    nEndMin  (endMin),
    nEndMax  (endMax)
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

long SvxColumnDescription::GetWidth() const
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
       nStartX == ((const SvxObjectItem&)rCmp).nStartX &&
       nEndX == ((const SvxObjectItem&)rCmp).nEndX &&
       nStartY == ((const SvxObjectItem&)rCmp).nStartY &&
       nEndY == ((const SvxObjectItem&)rCmp).nEndY &&
       bLimits == ((const SvxObjectItem&)rCmp).bLimits;
}

OUString SvxObjectItem::GetValueText() const
{
    return OUString();
}

SfxItemPresentation SvxObjectItem::GetPresentation(
                        SfxItemPresentation /*ePres*/,
                        SfxMapUnit          /*eCoreUnit*/,
                        SfxMapUnit          /*ePresUnit*/,
                        OUString&           /*rText*/,
                        const IntlWrapper*  /*pWrapper*/ ) const
{
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxPoolItem* SvxObjectItem::Clone(SfxItemPool *) const
{
    return new SvxObjectItem(*this);
}

SvxObjectItem::SvxObjectItem( long nSX, long nEX,
                              long nSY, long nEY, bool limits ) :
    SfxPoolItem (SID_RULER_OBJECT),
    nStartX     (nSX),
    nEndX       (nEX),
    nStartY     (nSY),
    nEndY       (nEY),
    bLimits     (limits)
{}

SvxObjectItem::SvxObjectItem( const SvxObjectItem& rCopy ) :
    SfxPoolItem (rCopy),
    nStartX     (rCopy.nStartX),
    nEndX       (rCopy.nEndX),
    nStartY     (rCopy.nStartY),
    nEndY       (rCopy.nEndY),
    bLimits     (rCopy.bLimits)
{}

bool SvxObjectItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
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

bool SvxObjectItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
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

bool SvxObjectItem::HasLimits() const
{
    return bLimits;
}

long SvxObjectItem::GetStartX() const
{
    return nStartX;
}

long SvxObjectItem::GetEndX() const
{
    return nEndX;
}

long SvxObjectItem::GetStartY() const
{
    return nStartY;
}

long SvxObjectItem::GetEndY() const
{
    return nEndY;
}

void SvxObjectItem::SetStartX(long lValue)
{
    nStartX = lValue;
}

void SvxObjectItem::SetEndX(long lValue)
{
    nEndX = lValue;
}

void SvxObjectItem::SetStartY(long lValue)
{
    nStartY = lValue;
}

void SvxObjectItem::SetEndY(long lValue)
{
    nEndY = lValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
